#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <ppl.h>
#include <ppltasks.h>
#include "SecondWindow.xaml.h"
#include "IconHelper.h"
#include "HotKey.h"
#include "HotKeyManager.h"
#include "DebugOutput.h"

#define USE_TIMER 1
#define DEACTIVATE_TIMER 0
#define ENABLE_HOTKEYS 1

constexpr int VOLUME_DOWN = 1;
constexpr int VOLUME_UP = 2;
constexpr int VOLUME_UP_ALT = 3;
constexpr int VOLUME_DOWN_ALT = 4;
constexpr int MUTE_SYSTEM = 5;

namespace CAudio = ::Croak::Audio;
namespace Microsoft = winrt::Microsoft;
namespace Windows = winrt::Windows;
namespace Foundation = winrt::Windows::Foundation;
namespace System = winrt::Windows::System;
namespace Graphics = winrt::Windows::Graphics;
namespace Collections = winrt::Windows::Foundation::Collections;

/**
 * @brief Microsoft::UI, Microsoft::UI::Composition, Microsoft::UI::Composition::SystemBackdrops, Microsoft::UI::Windowing
*/
namespace UI
{
    using namespace winrt::Microsoft::UI;
    using namespace winrt::Microsoft::UI::Composition;
    using namespace winrt::Microsoft::UI::Composition::SystemBackdrops;
    using namespace winrt::Microsoft::UI::Windowing;
}

/**
 * @brief Microsoft::UI::Xaml, Microsoft::UI::Xaml::Input, Microsoft::UI::Xaml::Controls, Microsoft::UI::Xaml::Media, Microsoft::UI::Xaml::Media::Imaging, Microsoft::UI::Xaml::Controls::Primitives
*/
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Input;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Media;
    using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
    using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
}

/**
 * @brief Windows::Storage, Windows::ApplicationModel, Windows::ApplicationModel::Core, Windows::ApplicationModel::Resources
*/
namespace Storage
{
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::ApplicationModel::Core;
    using namespace winrt::Windows::ApplicationModel::Resources;
}


namespace winrt::Croak::implementation
{
    winrt::Croak::MainWindow MainWindow::singleton{ nullptr };

    MainWindow::MainWindow(const winrt::hstring& args)
    {
        singleton = *this;

        InitializeComponent();
        InitializeWindow();
        SettingsButtonTeachingTip().Target(SettingsButton());

        if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"ShowSplashScreen"), true))
        {
            Storage::PackageId packageId = Storage::Package::Current().Id();
            Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"ShowSplashScreen", Foundation::IReference(false));

            ApplicationVersionTextBlock().Text(
                to_hstring(packageId.Version().Major) + L"." + to_hstring(packageId.Version().Minor) + L"." + to_hstring(packageId.Version().Build)
            );
        }
        else
        {
            WindowSplashScreen().Visibility(Xaml::Visibility::Collapsed);
            ContentGrid().Visibility(Xaml::Visibility::Visible);
        }

        // If the application has been restarted by the user, or started with arguments.
        std::wstring_view argsView{ args };
        if (argsView.find(L"-secondWindow") != std::string_view::npos) // -secondWindow -> open the second window/settings window.
        {
            SettingsIconButton_Click(nullptr, nullptr);
        }
    }


#pragma region Event handlers
    void MainWindow::OnLoaded(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        loaded = true;

#if USE_TIMER
        if (!DisableAnimationsIconToggleButton().IsOn())
        {
            if (mainAudioEndpoint)
            {
                mainAudioEndpointPeakTimer.Start();
                VolumeStoryboard().Begin();
            }

            if (audioSessions.size() > 0)
            {
                audioSessionsPeakTimer.Start();
            }
        }
#endif // USE_TIMER

        // Generate size changed event to get correct clipping rectangle size
        SystemVolumeActivityBorder_SizeChanged(nullptr, nullptr);
        Grid_SizeChanged(nullptr, nullptr);

        // Teaching tips
        Storage::ApplicationDataContainer teachingTips = Storage::ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TeachingTips");
        if (!teachingTips)
        {
            teachingTips = Storage::ApplicationData::Current().LocalSettings().CreateContainer(L"TeachingTips", Storage::ApplicationDataCreateDisposition::Always);
        }
        if (!teachingTips.Values().HasKey(L"ShowSettingsTeachingTip"))
        {
            teachingTips.Values().Insert(L"ShowSettingsTeachingTip", Foundation::IReference(false));
        }

        switch (layout)
        {
            case 1:
                HorizontalViewMenuFlyoutItem_Click(nullptr, nullptr);
                break;
            case 2:
                VerticalViewMenuFlyoutItem_Click(nullptr, nullptr);
                break;
            case 0:
            default:
                AutoViewMenuFlyoutItem_Click(nullptr, nullptr);
                break;
        }

        using namespace Microsoft::Windows::System::Power;
        PowerManager::EffectivePowerModeChanged([this](IInspectable, IInspectable)
        {
            OutputDebugHString(L"Effective power mode changed.");

            auto powerMode = PowerManager::EffectivePowerMode2();
            switch (powerMode)
            {
                case EffectivePowerMode::BatterySaver:
                case EffectivePowerMode::BetterBattery:
                {
                    // Turn off animations.
                    DispatcherQueue().TryEnqueue([this]()
                    {
                        if (!DisableAnimationsIconToggleButton().IsOn())
                        {
                            OutputDebugHString(L"Battery saver enabled, disabling animations.");

                            if (mainAudioEndpointPeakTimer.IsRunning())
                            {
                                mainAudioEndpointPeakTimer.Stop();
                                LeftVolumeAnimation().To(0.);
                                RightVolumeAnimation().To(0.);
                                VolumeStoryboard().Begin();
                            }

                            if (audioSessionsPeakTimer.IsRunning())
                            {
                                audioSessionsPeakTimer.Stop();
                                for (auto&& view : audioSessionViews)
                                {
                                    view.SetPeak(0, 0);
                                }
                            }

                            // I18N: Translate battery saver messages.
                            WindowMessageBar().EnqueueString(L"Battery saver enabled, disabling animations.");
                            DisableAnimationsIconToggleButton().IsOn(true);
                        }
                    });
                    break;
                }
                case EffectivePowerMode::Balanced:
                case EffectivePowerMode::HighPerformance:
                case EffectivePowerMode::MaxPerformance:
                case EffectivePowerMode::GameMode:
                case EffectivePowerMode::MixedReality:
                default:
                {
                    DispatcherQueue().TryEnqueue([this]()
                    {
                        if (DisableAnimationsIconToggleButton().IsOn())
                        {
                            if (!mainAudioEndpointPeakTimer.IsRunning())
                            {
                                mainAudioEndpointPeakTimer.Start();
                            }

                            if (!audioSessionsPeakTimer.IsRunning())
                            {
                                audioSessionsPeakTimer.Start();
                            }

                            // I18N: Translate battery saver messages.
                            WindowMessageBar().EnqueueString(L"Battery saver disabled, re-enabling animations.");
                            DisableAnimationsIconToggleButton().IsOn(false);
                        }
                    });
                    break;
                }
            }
        });

        PowerManager::UserPresenceStatusChanged([this](IInspectable, IInspectable)
        {
            auto userStatus = static_cast<int32_t>(PowerManager::UserPresenceStatus());

            switch (userStatus)
            {
                case static_cast<int32_t>(UserPresenceStatus::Present):
                {
                    OutputDebugHString(L"User presence status changed: user present.");

                    if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true))
                    {
                        DispatcherQueue().TryEnqueue([this]()
                        {
                            if (!mainAudioEndpointPeakTimer.IsRunning())
                            {
                                mainAudioEndpointPeakTimer.Start();
                            }

                            if (!audioSessionsPeakTimer.IsRunning())
                            {
                                audioSessionsPeakTimer.Start();
                            }
                        });
                    }

                    break;
                }
                case static_cast<int32_t>(UserPresenceStatus::Absent):
                case 2:
                {
                    OutputDebugHString(L"User presence status changed: user absent.");

                    if (
                        unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true)
                        )
                    {
                        DispatcherQueue().TryEnqueue([this]()
                        {
                            if (mainAudioEndpointPeakTimer.IsRunning())
                            {
                                mainAudioEndpointPeakTimer.Stop();
                                LeftVolumeAnimation().To(0.);
                                RightVolumeAnimation().To(0.);
                                VolumeStoryboard().Begin();
                            }

                            if (audioSessionsPeakTimer.IsRunning())
                            {
                                audioSessionsPeakTimer.Stop();
                                for (auto&& view : audioSessionViews)
                                {
                                    view.SetPeak(0, 0);
                                }
                            }
                        });
                    }
                    break;
                }
            }
        });


        uint16_t lastMajor = 0;
        uint16_t lastMinor = 0;
        uint16_t lastBuild = 0;
        bool packageDifferentVersion = false;

        Storage::PackageId packageId = Storage::Package::Current().Id();

        Storage::ApplicationDataContainer lastPackageIdContainer{ nullptr };
        if (Storage::ApplicationData::Current().LocalSettings().Containers().HasKey(L"PackageId"))
        {
            lastPackageIdContainer = Storage::ApplicationData::Current().LocalSettings().Containers().Lookup(L"PackageId");
            lastMajor = unbox_value<uint16_t>(lastPackageIdContainer.Values().Lookup(L"Major"));
            lastMinor = unbox_value<uint16_t>(lastPackageIdContainer.Values().Lookup(L"Minor"));
            lastBuild = unbox_value<uint16_t>(lastPackageIdContainer.Values().Lookup(L"Build"));

            auto Major = packageId.Version().Major;
            auto Minor = packageId.Version().Minor;
            auto Build = packageId.Version().Build;

            packageDifferentVersion = Major != lastMajor || Minor != lastMinor || Build != lastBuild;
        }
        else
        {
            lastPackageIdContainer = Storage::ApplicationData::Current().LocalSettings().CreateContainer(L"PackageId", Storage::ApplicationDataCreateDisposition::Always);
        }

        lastPackageIdContainer.Values().Insert(L"Major", box_value(packageId.Version().Major));
        lastPackageIdContainer.Values().Insert(L"Minor", box_value(packageId.Version().Minor));
        lastPackageIdContainer.Values().Insert(L"Build", box_value(packageId.Version().Build));

        if (packageDifferentVersion)
        {
            Xaml::StackPanel panel{};
            Xaml::TextBlock block{};
            block.Text(L"App has been updated");
            Xaml::HyperlinkButton button{};
            button.Content(box_value(L"read notes here"));
            // I can assume *this* will not be dereferenced.
            button.Click([this](auto, auto)
            {
                if (!secondWindow)
                {
                    secondWindow = make<SecondWindow>();
                    secondWindow.Closed([this](IInspectable, UI::WindowEventArgs)
                    {
                        secondWindow = nullptr;
                    });
                }
                secondWindow.Activate();
                secondWindow.NavigateTo(xaml_typename<NewContentPage>());
            });

            panel.Children().Append(block);
            panel.Children().Append(button);

            WindowMessageBar().EnqueueMessage(panel);
        }
    }

    void MainWindow::Window_Activated(IInspectable const&, WindowActivatedEventArgs const&)
    {
#if DEACTIVATE_TIMER
        if (args.WindowActivationState() == WindowActivationState::Deactivated && audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }
        else if (!audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Start();
        }
#endif
    }

    void MainWindow::Grid_SizeChanged(IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        if (usingCustomTitleBar)
        {
            SetDragRectangles();
        }

        if (appWindow.Size().Width < 210 && !compact)
        {
            compact = true;

            MuteToggleButtonColumn().Width(Xaml::GridLengthHelper::FromPixels(0));
            SystemVolumeSliderTextColumn().Width(Xaml::GridLengthHelper::FromPixels(0));

            MuteToggleButton().Visibility(Xaml::Visibility::Collapsed);
            SystemVolumeNumberBlock().Visibility(Xaml::Visibility::Collapsed);
        }
        else if (appWindow.Size().Width > 210 && compact)
        {
            compact = false;

            MuteToggleButtonColumn().Width(Xaml::GridLengthHelper::Auto());
            SystemVolumeSliderTextColumn().Width(Xaml::GridLengthHelper::FromPixels(26));

            MuteToggleButton().Visibility(Xaml::Visibility::Visible);
            SystemVolumeNumberBlock().Visibility(Xaml::Visibility::Visible);
        }

        SetSizeIndicators();
    }

    void MainWindow::AudioSessionView_VolumeChanged(AudioSessionView const& sender, Xaml::RangeBaseValueChangedEventArgs const& args)
    {
        std::unique_lock lock{ audioSessionsMutex };
        audioSessions.at(sender.Id())->Volume(static_cast<float>(args.NewValue() / 100.0));
    }

    void MainWindow::AudioSessionView_VolumeStateChanged(winrt::Croak::AudioSessionView const& sender, bool const& args)
    {
        std::unique_lock lock{ audioSessionsMutex };
        audioSessions.at(sender.Id())->SetMute(args);
    }

    void MainWindow::AudioSessionsPanel_Loading(FrameworkElement const&, IInspectable const&)
    {
        LoadContent();

        if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"LoadLastProfile"), true))
        {
            hstring profileName = unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"AudioProfile"), L"");
            if (!profileName.empty())
            {
                LoadProfile(profileName);
            }
        }

#if ENABLE_HOTKEYS
        // Activate hotkeys.
        LoadHotKeys();
#endif // ENABLE_HOTKEYS
    }

    void MainWindow::SystemVolumeSlider_ValueChanged(IInspectable const&, Xaml::RangeBaseValueChangedEventArgs const& e)
    {
        if (mainAudioEndpoint)
        {
            mainAudioEndpoint->Volume(static_cast<float>(e.NewValue() / 100.));
        }

        SystemVolumeNumberBlock().Double(e.NewValue());
    }

    void MainWindow::SystemVolumeActivityBorder_SizeChanged(IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        SystemVolumeActivityBorderClippingRight().Rect(
            Foundation::Rect(0, 0, static_cast<float>(SystemVolumeActivityBorderRight().ActualWidth()), static_cast<float>(SystemVolumeActivityBorderRight().ActualHeight()))
        );
        SystemVolumeActivityBorderClippingLeft().Rect(
            Foundation::Rect(0, 0, static_cast<float>(SystemVolumeActivityBorderLeft().ActualWidth()), static_cast<float>(SystemVolumeActivityBorderLeft().ActualHeight()))
        );
    }

    void MainWindow::HideMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        appWindow.Presenter().as<UI::OverlappedPresenter>().Minimize();
    }

    void MainWindow::SwitchStateMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        bool setMute = false;

        uint32_t count = 0;
        if (globalSessionAudioState == winrt::Croak::AudioSessionState::Unmuted)
        {
            for (auto&& view : audioSessionViews)
            {
                if (view.Muted())
                {
                    count++;
                }
            }

            if (count != audioSessionViews.Size())
            {
                setMute = true;
                globalSessionAudioState = AudioSessionState::Muted;
            }
        }
        else if (globalSessionAudioState == winrt::Croak::AudioSessionState::Muted)
        {
            for (auto&& view : audioSessionViews)
            {
                if (!view.Muted())
                {
                    count++;
                }
            }

            if (count == audioSessionViews.Size())
            {
                setMute = true;
                globalSessionAudioState = AudioSessionState::Muted;
            }
            else
            {
                setMute = false;
                globalSessionAudioState = AudioSessionState::Unmuted;
            }
        }

        {
            std::unique_lock lock{ audioSessionsMutex };
            for (std::pair<guid, CAudio::AudioSession*> iter : audioSessions)
            {
                iter.second->Muted(setMute);
            }
        }

        for (AudioSessionView view : audioSessionViews)
        {
            view.Muted(setMute);
        }
    }

    void MainWindow::HorizontalViewMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(Xaml::ScrollBarVisibility::Auto);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(Xaml::ScrollBarVisibility::Disabled);
        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewHorizontalLayout")).as < Xaml::Style > ()
        );
        layout = 1;

        // Set DropDownButton to mimic ComboBox selected item behavior.
        Xaml::FontIcon icon{};
        icon.Glyph(L"\ue8c0");
        LayoutDropDownButton().Content(icon);

        for (auto&& view : audioSessionViews)
        {
            view.Orientation(Xaml::Orientation::Vertical);
        }
    }

    void MainWindow::VerticalViewMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(Xaml::ScrollBarVisibility::Disabled);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(Xaml::ScrollBarVisibility::Auto);
        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewVerticalLayout")).as<Xaml::Style>()
        );
        layout = 2;

        // Set DropDownButton to mimic ComboBox selected item behavior.
        Xaml::FontIcon icon{};
        icon.Glyph(L"\ue8c0");
        icon.Translation(Foundation::Numerics::float3(16, 0, 0));
        icon.Rotation(90);
        LayoutDropDownButton().Content(icon);

        for (auto&& view : audioSessionViews)
        {
            view.Orientation(Xaml::Orientation::Horizontal);
        }
    }

    void MainWindow::AutoViewMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(Xaml::ScrollBarVisibility::Disabled);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(Xaml::ScrollBarVisibility::Auto);

        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewHorizontalLayout")).as<Xaml::Style>()
        );
        layout = 0;

        // Set DropDownButton to mimic ComboBox selected item behavior.
        Xaml::FontIcon icon{};
        icon.Glyph(L"\uf0e2");
        LayoutDropDownButton().Content(icon);

        for (auto&& view : audioSessionViews)
        {
            view.Orientation(Xaml::Orientation::Vertical);
        }
    }

    void MainWindow::SettingsIconButton_Click(IconButton const& /*sender*/, Xaml::RoutedEventArgs const& /*args*/)
    {
        if (!secondWindow)
        {
            secondWindow = make<SecondWindow>();
            secondWindow.Closed([this](IInspectable, Xaml::WindowEventArgs)
            {
                secondWindow = nullptr;
            });
        }
        secondWindow.Activate();
    }

    void MainWindow::DisableHotKeysIconButton_Click(IconToggleButton const& /*sender*/, Xaml::RoutedEventArgs const& /*args*/)
    {
#if ENABLE_HOTKEYS
        /*ResourceLoader loader{};
        if (muteHotKeyPtr.Enabled())
        {
            WindowMessageBar().EnqueueString(loader.GetString(L"InfoHotKeysEnabled"));
        }
        else
        {
            WindowMessageBar().EnqueueString(loader.GetString(L"InfoHotKeysDisabled"));
        }*/
#endif // ENABLE_HOTKEYS
        SettingsButtonFlyout().Hide();
    }

    void MainWindow::ShowAppBarIconButton_Click(IconToggleButton const& /*sender*/, Xaml::RoutedEventArgs const& /*args*/)
    {
        AppBarGrid().Visibility(AppBarGrid().Visibility() == Xaml::Visibility::Visible ? Xaml::Visibility::Collapsed : Xaml::Visibility::Visible);
    }

    void MainWindow::DisableAnimationsIconButton_Click(IconToggleButton const& /*sender*/, Xaml::RoutedEventArgs const& /*args*/)
    {
        if (audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }
        else
        {
            audioSessionsPeakTimer.Start();
        }

        if (mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }
        else
        {
            mainAudioEndpointPeakTimer.Start();
        }


        LeftVolumeAnimation().To(0.);
        RightVolumeAnimation().To(0.);
        VolumeStoryboard().Begin();

        for (auto&& view : audioSessionViews)
        {
            view.SetPeak(0, 0);
        }
        SettingsButtonFlyout().Hide();
    }

    void MainWindow::MuteToggleButton_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());
        MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
    }

    void MainWindow::ViewHotKeysHyperlinkButton_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
    }

    void MainWindow::SplashScreen_Dismissed(winrt::Croak::SplashScreen const&, Foundation::IInspectable const&)
    {
        WindowSplashScreen().Visibility(Xaml::Visibility::Collapsed);
        ContentGrid().Visibility(Xaml::Visibility::Visible);
    }

    void MainWindow::ExpandFlyoutButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        MoreFlyoutGrid().Visibility(
            MoreFlyoutGrid().Visibility() == Xaml::Visibility::Visible ? Xaml::Visibility::Collapsed : Xaml::Visibility::Visible
        );
    }

    void MainWindow::KeepOnTopToggleButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        UI::OverlappedPresenter presenter = appWindow.Presenter().as<UI::OverlappedPresenter>();
        presenter.IsMaximizable(!presenter.IsMaximizable());
        presenter.IsMinimizable(!presenter.IsMinimizable());

        bool alwaysOnTop = KeepOnTopToggleButton().IsChecked().GetBoolean();
        presenter.IsAlwaysOnTop(alwaysOnTop);
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"IsAlwaysOnTop", box_value(alwaysOnTop));

        RightPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(
            presenter.IsMinimizable() ? 135 : 45
        ));
    }

    void MainWindow::ReloadSessionsIconButton_Click(IconButton const&, Xaml::RoutedEventArgs const&)
    {
        if (audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }
        if (mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }

        audioSessionViews.Clear();
        VolumeStoryboard().Stop();

        // Clean up ComPtr/IUnknown objects
        if (audioController)
        {
            if (mainAudioEndpoint)
            {
                mainAudioEndpoint->StateChanged(mainAudioEndpointStateChangedToken);
                mainAudioEndpoint->VolumeChanged(mainAudioEndpointVolumeChangedToken);
                mainAudioEndpoint->Unregister();
                mainAudioEndpoint->Release();
            }

            audioController->EndpointChanged(audioControllerEndpointChangedToken);
            audioController->SessionAdded(audioControllerSessionAddedToken);

            audioController->Unregister();
            audioController->Release();
        }

        // Unregister VolumeChanged event handler & unregister audio sessions from audio events and release com ptrs.
        {
            std::unique_lock lock{ audioSessionsMutex };
            for (auto it : audioSessions)
            {
                it.second->VolumeChanged(audioSessionVolumeChanged[it.first]);
                it.second->StateChanged(audioSessionsStateChanged[it.first]);
                it.second->Unregister();
                it.second->Release();
            }
            audioSessions.clear();
        }


        // Reload content
        LoadContent();
        if (!DisableAnimationsIconToggleButton().IsOn())
        {
            if (mainAudioEndpoint)
            {
                mainAudioEndpointPeakTimer.Start();
                VolumeStoryboard().Begin();
            }

            if (audioSessions.size() > 0u)
            {
                audioSessionsPeakTimer.Start();
            }
        }

        Storage::ResourceLoader loader{};
        WindowMessageBar().EnqueueString(loader.GetString(L"InfoAudioSessionsReloaded"));
        SettingsButtonFlyout().Hide();
    }

    void MainWindow::RestartIconButton_Click(IconButton const&, Xaml::RoutedEventArgs const&)
    {
        if (Microsoft::Windows::AppLifecycle::AppInstance::Restart(secondWindow ? L"-secondWindow" : L"") != Storage::Core::AppRestartFailureReason::RestartPending)
        {
            Storage::ResourceLoader loader{};
            WindowMessageBar().EnqueueString(loader.GetString(L"ErrorAppFailedRestart"));
        }
    }

    void MainWindow::OpenProfilesIconButton_Click(IconButton const&, Xaml::RoutedEventArgs const&)
    {
        MoreFlyoutStackpanel().Visibility(Xaml::Visibility::Collapsed);
        ProfilesGrid().Visibility(Xaml::Visibility::Visible);

        ProfilesStackpanel().Children().Clear();
        Storage::ApplicationDataContainer audioProfilesContainer = Storage::ApplicationData::Current().LocalSettings().Containers().TryLookup(L"AudioProfiles");
        if (audioProfilesContainer)
        {
            for (auto&& profile : audioProfilesContainer.Containers())
            {
                hstring key = profile.Key();
                Xaml::ToggleButton item{};
                item.Content(box_value(key));
                item.Tag(box_value(key));
                item.IsChecked(currentAudioProfile && (key == currentAudioProfile.ProfileName()));
                item.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
                item.Background(Xaml::SolidColorBrush(UI::Colors::Transparent()));
                item.BorderThickness(Xaml::Thickness(0));

                item.Click([this](const IInspectable& sender, Xaml::RoutedEventArgs)
                {
                    MoreFlyoutStackpanel().Visibility(Xaml::Visibility::Visible);
                    ProfilesGrid().Visibility(Xaml::Visibility::Collapsed);
                    SettingsButtonFlyout().Hide();
                    LoadProfile(sender.as<FrameworkElement>().Tag().as<hstring>());
                });

                ProfilesStackpanel().Children().Append(item);
            }
        }
    }

    void MainWindow::CloseProfilesButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        MoreFlyoutStackpanel().Visibility(Xaml::Visibility::Visible);
        ProfilesGrid().Visibility(Xaml::Visibility::Collapsed);
    }

    void MainWindow::OpenProfilesButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        if (!secondWindow)
        {
            secondWindow = make<SecondWindow>();
            secondWindow.Closed([this](IInspectable, Xaml::WindowEventArgs)
            {
                secondWindow = nullptr;
            });
        }
        secondWindow.Activate();
        secondWindow.NavigateTo(xaml_typename<AudioProfilesPage>());
    }

    void MainWindow::RootGrid_ActualThemeChanged(Xaml::FrameworkElement const&, IInspectable const&)
    {
        WindowMessageBar().EnqueueString(L"Actual application theme has changed. Loading new theme right now, all effects will be applied on restart.");
        // Change title bar buttons background to fit the new theme.
        if (usingCustomTitleBar)
        {
            if (RootGrid().ActualTheme() == Xaml::ElementTheme::Light)
            {
                appWindow.TitleBar().ButtonForegroundColor(UI::Colors::Black());
                appWindow.TitleBar().ButtonHoverForegroundColor(UI::Colors::Black());
                appWindow.TitleBar().ButtonPressedForegroundColor(UI::Colors::Black());
            }
            else if (RootGrid().ActualTheme() == Xaml::ElementTheme::Dark)
            {
                appWindow.TitleBar().ButtonForegroundColor(UI::Colors::White());
                appWindow.TitleBar().ButtonHoverForegroundColor(UI::Colors::White());
                appWindow.TitleBar().ButtonPressedForegroundColor(UI::Colors::White());
            }
        }

        if (systemBackdropConfiguration)
        {
            systemBackdropConfiguration.Theme((UI::SystemBackdropTheme)RootGrid().ActualTheme());
            backdropController.TintColor(Xaml::Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
            backdropController.FallbackColor(Xaml::Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
        }

        // HACK: Flyout grids are not auto updating their backgrounds when the theme change.
        Xaml::Brush brush = Xaml::SolidColorBrush(
            Xaml::Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorSecondary")).as<Windows::UI::Color>()
        );
        brush.Opacity(0.4);
        CommandBarGrid().Background(brush);
        SettingsButtonFlyoutGrid().Background(
            Xaml::SolidColorBrush(Xaml::Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>())
        );
        MoreFlyoutGrid().Background(
            Xaml::Application::Current().Resources().TryLookup(box_value(L"SubtleFillColorSecondaryBrush")).as<Xaml::Brush>()
        );
    }

    void MainWindow::ShowHiddenSessionsButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        /* 
        * - Show hidden sessions, enable item selection.
        * - Selected audio sessions will be hidden the next time 'show hidden buttons' is clicked.
        * - Unselected audio sessions will be kept the next time 'show hidden buttons' is clicked.
        */
        // TODO: How do I insert the new sessions: lookup in the profile (if loaded) or insert at the end to not disrupt the UX too much.
    }
#pragma endregion


    void MainWindow::InitializeWindow()
    {
        auto nativeWindow{ this->try_as<::IWindowNative>() };
        check_bool(nativeWindow);
        HWND handle = nullptr;
        nativeWindow->get_WindowHandle(&handle);
        UI::WindowId windowID = UI::GetWindowIdFromWindow(handle);
        appWindow = UI::AppWindow::GetFromWindowId(windowID);
        if (appWindow != nullptr)
        {
#ifdef DEBUG
            Xaml::TextBlock timestamp{};
            timestamp.IsHitTestVisible(false);
            timestamp.Opacity(0.6);
            timestamp.HorizontalAlignment(Xaml::HorizontalAlignment::Right);
            timestamp.VerticalAlignment(Xaml::VerticalAlignment::Bottom);
            timestamp.Text(to_hstring(__TIME__) + L", " + to_hstring(__DATE__));

            Xaml::Grid::SetRow(timestamp, WindowGrid().RowDefinitions().GetView().Size() - 1);
            RootGrid().Children().Append(timestamp);
#endif // _DEBUG

            appWindow.Title(Xaml::Application::Current().Resources().Lookup(box_value(L"AppTitle")).as<hstring>());

            appWindow.Closing({ this, &MainWindow::AppWindow_Closing });
            appWindow.Changed([this](UI::AppWindow, winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs args)
            {
                UI::OverlappedPresenter presenter = nullptr;
                if ((presenter = appWindow.Presenter().try_as<UI::OverlappedPresenter>()))
                {
                    if (args.DidSizeChange())
                    {
                        int32_t minimum = presenter.IsMinimizable() ? 250 : 130;
                        if (appWindow.Size().Width < minimum)
                        {
                            appWindow.Resize(Graphics::SizeInt32(minimum, appWindow.Size().Height));
                        }
                    }

                    if (presenter.State() != UI::OverlappedPresenterState::Maximized)
                    {
                        if (args.DidPositionChange())
                        {
                            displayRect.X = appWindow.Position().X;
                            displayRect.Y = appWindow.Position().Y;
                        }

                        if (args.DidSizeChange())
                        {
                            displayRect.Width = appWindow.Size().Width;
                            displayRect.Height = appWindow.Size().Height;
                        }
                    }
                }
            });

            if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"UseCustomTitleBar"), true) &&
                appWindow.TitleBar().IsCustomizationSupported())
            {
                usingCustomTitleBar = true;

                appWindow.TitleBar().ExtendsContentIntoTitleBar(true);
                appWindow.TitleBar().IconShowOptions(UI::IconShowOptions::HideIconAndSystemMenu);

                LeftPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(static_cast<double>(appWindow.TitleBar().LeftInset())));

                appWindow.TitleBar().ButtonBackgroundColor(UI::Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveBackgroundColor(UI::Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveForegroundColor(
                    Xaml::Application::Current().Resources().TryLookup(box_value(L"AppTitleBarHoverColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonHoverBackgroundColor(
                    Xaml::Application::Current().Resources().TryLookup(box_value(L"ButtonHoverBackgroundColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonPressedBackgroundColor(UI::Colors::Transparent());

                if (RootGrid().ActualTheme() == Xaml::ElementTheme::Light)
                {
                    appWindow.TitleBar().ButtonForegroundColor(UI::Colors::Black());
                    appWindow.TitleBar().ButtonHoverForegroundColor(UI::Colors::Black());
                    appWindow.TitleBar().ButtonPressedForegroundColor(UI::Colors::Black());
                }
                else if (RootGrid().ActualTheme() == Xaml::ElementTheme::Dark)
                {
                    appWindow.TitleBar().ButtonForegroundColor(UI::Colors::White());
                    appWindow.TitleBar().ButtonHoverForegroundColor(UI::Colors::White());
                    appWindow.TitleBar().ButtonPressedForegroundColor(UI::Colors::White());
                }
            }

            LoadSettings();
        }

        SetBackground();

#if ENABLE_HOTKEYS
#pragma warning(push)
#pragma warning(disable:4305)
#pragma warning(disable:4244)
        /*
        * Currently used hotkeys:
        *  - Control + Shift + Up : system volume up
        *  - Control + Shift + Down : system volume down
        *  - Control + Shift + PageUp : system volume big up
        *  - Control + Shift + PageDown : system volume big down
        *  - Alt/Menu + Shift + M : system volume mute/unmute
        */


#pragma warning(pop)  
#endif // ENABLE_HOTKEYS
    }

    void MainWindow::SetBackground()
    {
        if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"TransparencyAllowed"), true))
        {
            if (UI::DesktopAcrylicController::IsSupported())
            {
                auto&& supportsBackdrop = try_as<UI::ICompositionSupportsSystemBackdrop>();
                if (supportsBackdrop)
                {
                    WindowGrid().Background(Xaml::SolidColorBrush(UI::Colors::Transparent()));

                    if (!System::DispatcherQueue::GetForCurrentThread() && !dispatcherQueueController)
                    {
                        DispatcherQueueOptions options
                        {
                            sizeof(DispatcherQueueOptions),
                            DQTYPE_THREAD_CURRENT,
                            DQTAT_COM_NONE
                        };

                        ABI::Windows::System::IDispatcherQueueController* ptr{ nullptr };
                        check_hresult(CreateDispatcherQueueController(options, &ptr));
                        dispatcherQueueController = DispatcherQueueController(ptr, take_ownership_from_abi);
                    }

                    systemBackdropConfiguration = UI::SystemBackdropConfiguration();
                    systemBackdropConfiguration.IsInputActive(true);
                    systemBackdropConfiguration.Theme((UI::SystemBackdropTheme)RootGrid().ActualTheme());

                    backdropController = BackdropController();
                    backdropController.TintColor(Xaml::Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                    backdropController.FallbackColor(Xaml::Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                    backdropController.TintOpacity(static_cast<float>(Xaml::Application::Current().Resources().TryLookup(box_value(L"BackdropTintOpacity")).as<double>()));
                    backdropController.LuminosityOpacity(static_cast<float>(Xaml::Application::Current().Resources().TryLookup(box_value(L"BackdropLuminosityOpacity")).as<double>()));
                    backdropController.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                    backdropController.AddSystemBackdropTarget(supportsBackdrop);
                }
            }
        }
        else
        {
            hstring path = unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"BackgroundImageUri"), L"");
            if (!path.empty())
            {
                try
                {
                    Xaml::BitmapImage image{};
                    image.UriSource(Foundation::Uri(path));
                    BackgroundImage().Source(image);
                }
                catch (const hresult_access_denied&)
                {
                    WindowMessageBar().EnqueueString(L"Failed to load background image.");
                }
            }
        }
    }

    void MainWindow::LoadContent()
    {
        Storage::ResourceLoader loader{};
        GUID appID{};
        if (SUCCEEDED(UuidCreate(&appID)))
        {
            try
            {
                // Create and setup audio interfaces.
                audioController = new CAudio::AudioController(appID);

                if (audioController->Register())
                {
                    audioController->SessionAdded({ this, &MainWindow::AudioController_SessionAdded });
                    audioController->EndpointChanged({ this, &MainWindow::AudioController_EndpointChanged });
                }
                else
                {
                    WindowMessageBar().EnqueueString(loader.GetString(L"ErrorAudioSessionsUnavailable"));
                }

                mainAudioEndpoint = audioController->GetMainAudioEndpoint();
                if (mainAudioEndpoint->Register())
                {
                    mainAudioEndpointVolumeChangedToken = mainAudioEndpoint->VolumeChanged({ this, &MainWindow::MainAudioEndpoint_VolumeChanged });
                    mainAudioEndpointStateChangedToken = mainAudioEndpoint->StateChanged([this](IInspectable, bool muted)
                    {
                        DispatcherQueue().TryEnqueue([this, muted]()
                    {
                        MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());
                    MuteToggleButtonFontIcon().Glyph(muted ? L"\ue74f" : L"\ue767");
                    });
                    });
                }

                // TODO: Check code validity, no memory leaks, performance.
                auto audioSessionsVector = std::unique_ptr<std::vector<CAudio::AudioSession*>>();
                try
                {
                    audioSessionsVector = std::unique_ptr<std::vector<CAudio::AudioSession*>>(audioController->GetSessions());
                    for (size_t i = 0; i < audioSessionsVector->size(); i++)
                    {
                        audioSessions.insert({ guid(audioSessionsVector->at(i)->Id()), audioSessionsVector->at(i) });

                        // Check if the session is active, if not check if the user asked to show inactive sessions on startup.
                        if (audioSessionsVector->at(i)->State() == ::AudioSessionState::AudioSessionStateActive ||
                            unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"ShowInactiveSessionsOnStartup"), false))
                        {
                            if (AudioSessionView view = CreateAudioView(audioSessionsVector->at(i)))
                            {
                                audioSessionViews.Append(view);
                            }
                        }
                        else // Register to events since we are not adding/creating the view.
                        {
                            if (audioSessionsVector->at(i)->Register())
                            {
                                audioSessionVolumeChanged.insert({ audioSessionsVector->at(i)->Id(), audioSessionsVector->at(i)->VolumeChanged({ this, &MainWindow::AudioSession_VolumeChanged }) });
                                audioSessionsStateChanged.insert({ audioSessionsVector->at(i)->Id(), audioSessionsVector->at(i)->StateChanged({ this, &MainWindow::AudioSession_StateChanged }) });
                            }
                            else
                            {
                                OutputDebugWString(L"Failed to register audio session '" + audioSessionsVector->at(i)->Name() + L"'. This session will never be shown.");
                                WindowMessageBar().EnqueueString(audioSessionsVector->at(i)->Name() + L" : notifications off");
                            }
                        }
                    }
                }
                catch (const hresult_error&)
                {
                    if (audioSessionsVector.get())
                    {
                        for (size_t j = 0; j < audioSessionsVector->size(); j++)
                        {
                            audioSessionsVector->at(j)->Release();
                        }

                        auto vect = audioSessionsVector.release();
                        delete vect;
                    }

                    throw;
                }


                // Create and setup peak meters timers
                audioSessionsPeakTimer = DispatcherQueue().CreateTimer();
                mainAudioEndpointPeakTimer = DispatcherQueue().CreateTimer();

                audioSessionsPeakTimer.Interval(Foundation::TimeSpan(std::chrono::milliseconds(83)));
                audioSessionsPeakTimer.Tick({ this, &MainWindow::UpdatePeakMeters });
                audioSessionsPeakTimer.Stop();

                mainAudioEndpointPeakTimer.Interval(Foundation::TimeSpan(std::chrono::milliseconds(83)));
                mainAudioEndpointPeakTimer.Tick([&](auto, auto)
                {
                    if (!loaded)
                    {
                        return;
                    }

                try
                {
                    pair<float, float> peakValues = mainAudioEndpoint->GetPeaks();
                    LeftVolumeAnimation().To(static_cast<double>(peakValues.first));
                    RightVolumeAnimation().To(static_cast<double>(peakValues.second));
                    VolumeStoryboard().Begin();
                }
                catch (const hresult_error&)
                {
                    // TODO: Handle error.
                }
                });

                MainEndpointNameTextBlock().Text(mainAudioEndpoint->Name());
                SystemVolumeSlider().Value(static_cast<double>(mainAudioEndpoint->Volume()) * 100.);
                MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());
                MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
            }
            catch (const hresult_error& ex)
            {
                WindowMessageBar().EnqueueString(loader.GetString(L"ErrorFatalFailure"));
                OutputDebugHString(ex.message());
            }
        }
        else
        {
            WindowMessageBar().EnqueueString(loader.GetString(L"ErrorFatalFailure"));
        }
    }

    AudioSessionView MainWindow::CreateAudioView(CAudio::AudioSession* audioSession)
    {
        if (audioSession->Register())
        {
            audioSessionVolumeChanged.insert({ audioSession->Id(), audioSession->VolumeChanged({ this, &MainWindow::AudioSession_VolumeChanged }) });
            audioSessionsStateChanged.insert({ audioSession->Id(), audioSession->StateChanged({ this, &MainWindow::AudioSession_StateChanged }) });
        }
        else
        {
            OutputDebugWString(L"Failed to register audio session '" + audioSession->Name() + L"'.");
            WindowMessageBar().EnqueueString(audioSession->Name() + L" notifications off");
        }

        return CreateAudioSessionView(audioSession, false);
    }

    AudioSessionView MainWindow::CreateAudioSessionView(CAudio::AudioSession* audioSession, bool skipDuplicates)
    {
        // Check for duplicates, multiple audio sessions might be grouped under one by the app/system owning the sessions.
        //checkForDuplicates = false;
        if (!skipDuplicates && audioSession->State() != ::AudioSessionState::AudioSessionStateActive)
        {
            std::unique_lock lock{ audioSessionsMutex };

            uint8_t hitcount = 0u;
            for (auto& iter : audioSessions)
            {
                if (iter.second->GroupingParam() == audioSession->GroupingParam())
                {
                    if (hitcount > 1)
                    {
                        return nullptr;
                    }
                    hitcount++;
                }
            }
        }

        AudioSessionView view = nullptr;
        if (audioSession->IsSystemSoundSession())
        {
            view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);

            Xaml::FontIcon icon{};
            icon.Glyph(L"\ue977");
            icon.Foreground(
                Xaml::SolidColorBrush(Xaml::Application::Current().Resources().TryLookup(box_value(L"SystemAccentColorLight1")).as<Windows::UI::Color>())
            );
            Xaml::Viewbox iconViewbox{};
            iconViewbox.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
            iconViewbox.VerticalAlignment(Xaml::VerticalAlignment::Stretch);
            iconViewbox.Child(icon);
            view.Logo().Content(iconViewbox);
        }
        else
        {
            if (!audioSession->ProcessInfo()->Manifest().Logo().empty())
            {
                view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);

                Xaml::BitmapImage imageSource{};
                imageSource.UriSource(Foundation::Uri(audioSession->ProcessInfo()->Manifest().Logo()));
                Xaml::Image image{};
                image.Source(imageSource);

                view.Logo().Content(image);
            }
            else if (!audioSession->ProcessInfo()->ExecutablePath().empty())
            {
                view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0, audioSession->ProcessInfo()->ExecutablePath());
            }
            else
            {
                view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);
            }
        }

        view.Id(guid(audioSession->Id()));
        view.Muted(audioSession->Muted());
        view.SetState((AudioSessionState)audioSession->State());

        view.VolumeChanged({ this, &MainWindow::AudioSessionView_VolumeChanged });
        view.VolumeStateChanged({ this, &MainWindow::AudioSessionView_VolumeStateChanged });
        view.Hidden([this](AudioSessionView sender, auto)
        {
#ifdef DEBUG

#else
            for (auto const& view : audioSessionViews)
            {
                if (view.Id() == sender.Id())
                {
                    uint32_t indexOf = 0;
                    if (audioSessionViews.IndexOf(view, indexOf))
                    {
                        audioSessionViews.RemoveAt(indexOf);
                    }

                    return;
                }
            }
#endif // DEBUG

        });

        return view;
    }

    void MainWindow::SetDragRectangles()
    {
        Graphics::RectInt32 dragRectangle{};

        dragRectangle.X = static_cast<int32_t>(LeftPaddingColumn().ActualWidth() + SettingsButtonColumn().ActualWidth());
        dragRectangle.Y = 0;
        dragRectangle.Width = static_cast<int32_t>(TitleBarGrid().ActualWidth() - (LeftPaddingColumn().ActualWidth() + SettingsButtonColumn().ActualWidth()));
        dragRectangle.Height = static_cast<int32_t>(TitleBarGrid().ActualHeight());

        appWindow.TitleBar().SetDragRectangles(std::vector<Graphics::RectInt32>{ dragRectangle });
    }

    void MainWindow::SaveAudioLevels()
    {
        Storage::ApplicationDataContainer audioLevels = Storage::ApplicationData::Current().LocalSettings().CreateContainer(
            L"AudioLevels",
            Storage::ApplicationData::Current().LocalSettings().Containers().HasKey(L"AudioLevels") ? Storage::ApplicationDataCreateDisposition::Existing : Storage::ApplicationDataCreateDisposition::Always
        );

        for (uint32_t i = 0; i < audioSessionViews.Size(); i++) // Only saving the visible audio sessions levels.
        {
            Storage::ApplicationDataCompositeValue compositeValue{};
            compositeValue.Insert(L"Muted", box_value(audioSessionViews.GetAt(i).Muted()));
            compositeValue.Insert(L"Level", box_value(audioSessionViews.GetAt(i).Volume()));
            if (!audioSessionViews.GetAt(i).Header().empty())
            {
                audioLevels.Values().Insert(audioSessionViews.GetAt(i).Header(), compositeValue);
            }
        }
    }

    void MainWindow::LoadHotKeys()
    {
        ::Croak::System::HotKeys::HotKeyManager& hotKeyManager = ::Croak::System::HotKeys::HotKeyManager::GetHotKeyManager();
        hotKeyManager.HotKeyFired({ this, &MainWindow::HotKeyFired });

        try
        {
            hotKeyManager.RegisterHotKey(VirtualKeyModifiers::Windows | VirtualKeyModifiers::Control, VK_DOWN, true, VOLUME_DOWN);
            hotKeyManager.RegisterHotKey(VirtualKeyModifiers::Windows | VirtualKeyModifiers::Control, VK_UP, true, VOLUME_UP);
            hotKeyManager.RegisterHotKey(VirtualKeyModifiers::Control | VirtualKeyModifiers::Menu, VK_UP, true, VOLUME_UP_ALT);
            hotKeyManager.RegisterHotKey(VirtualKeyModifiers::Control | VirtualKeyModifiers::Menu, VK_DOWN, true, VOLUME_DOWN_ALT);
            hotKeyManager.RegisterHotKey(VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift, 'M', true, MUTE_SYSTEM);
        }
        catch (const std::invalid_argument& ex)
        {
            DebugLog(std::format("Could not register hot key.\n\t{0}", ex.what()));
            WindowMessageBar().EnqueueMessage(box_value(L"Failed to enable some hot keys."));
        }
    }

    void MainWindow::LoadSettings()
    {
        Collections::IPropertySet settings = Storage::ApplicationData::Current().LocalSettings().Values();

        Graphics::RectInt32 rect{};
        rect.Width = unbox_value_or(
            settings.TryLookup(L"WindowWidth"),
            Xaml::Application::Current().Resources().Lookup(box_value(L"WindowWidth")).as<int32_t>()
        );
        rect.Height = unbox_value_or(
            settings.TryLookup(L"WindowHeight"),
            Xaml::Application::Current().Resources().Lookup(box_value(L"WindowHeight")).as<int32_t>()
        );
        rect.X = unbox_value_or(settings.TryLookup(L"WindowPosX"), -1);
        rect.Y = unbox_value_or(settings.TryLookup(L"WindowPosY"), -1);
        UI::DisplayArea display = UI::DisplayArea::GetFromPoint(Graphics::PointInt32(rect.X, rect.Y), UI::DisplayAreaFallback::None);
        if (!display || rect.X < 0 || rect.Y < 0)
        {
            display = UI::DisplayArea::GetFromPoint(Graphics::PointInt32(rect.X, rect.Y), UI::DisplayAreaFallback::Primary);
            rect.X = display.WorkArea().X + 10;
            rect.Y = display.WorkArea().X + 10;
        }
        appWindow.MoveAndResize(rect);


        auto&& presenter = appWindow.Presenter().as<UI::OverlappedPresenter>();

        bool alwaysOnTop = unbox_value_or(settings.TryLookup(L"IsAlwaysOnTop"), false);
        UI::OverlappedPresenterState presenterState = static_cast<UI::OverlappedPresenterState>(unbox_value_or(settings.TryLookup(L"PresenterState"), 2));
        switch (presenterState)
        {
            case UI::OverlappedPresenterState::Maximized:
                presenter.Maximize();
                break;
            case UI::OverlappedPresenterState::Minimized:
                presenter.Minimize();
                break;
                /*case OverlappedPresenterState::Restored:
                default:
                    break;*/
        }

        presenter.IsMaximizable(!alwaysOnTop);
        presenter.IsMinimizable(!alwaysOnTop);
        RightPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(
            alwaysOnTop ? 45 : 135
        ));
        presenter.IsAlwaysOnTop(alwaysOnTop);

        layout = unbox_value_or(settings.TryLookup(L"SessionsLayout"), 0);

        KeepOnTopToggleButton().IsChecked(alwaysOnTop);
        DisableAnimationsIconToggleButton().IsOn(unbox_value_or(settings.TryLookup(L"DisableAnimations"), false));
        bool showMenu = unbox_value_or(settings.TryLookup(L"ShowAppBar"), false);
        ShowAppBarIconToggleButton().IsOn(showMenu);
        if (showMenu)
        {
            AppBarGrid().Visibility(Xaml::Visibility::Visible);
        }
    }

    void MainWindow::SaveSettings()
    {
        Collections::IPropertySet settings = Storage::ApplicationData::Current().LocalSettings().Values();
        settings.Insert(L"WindowHeight", box_value(displayRect.Height));
        settings.Insert(L"WindowWidth", box_value(displayRect.Width));
        settings.Insert(L"WindowPosX", box_value(displayRect.X));
        settings.Insert(L"WindowPosY", box_value(displayRect.Y));

        auto presenter = appWindow.Presenter().as<UI::OverlappedPresenter>();
        settings.Insert(L"IsAlwaysOnTop", box_value(presenter.IsAlwaysOnTop()));
        settings.Insert(L"PresenterState", box_value(static_cast<int32_t>(presenter.State())));

        settings.Insert(L"DisableAnimations", box_value(DisableAnimationsIconToggleButton().IsOn()));
        settings.Insert(L"SessionsLayout", Foundation::IReference<int32_t>(layout));
        settings.Insert(L"ShowAppBar", box_value(ShowAppBarIconToggleButton().IsOn()));

        if (currentAudioProfile)
        {
            settings.Insert(L"AudioProfile", box_value(currentAudioProfile.ProfileName()));

            if (unbox_value_or(settings.TryLookup(L"AllowChangesToLoadedProfile"), false))
            {
                currentAudioProfile.SystemVolume(mainAudioEndpoint->Volume());
                currentAudioProfile.Layout(layout);
                currentAudioProfile.KeepOnTop(
                    appWindow.Presenter().as<UI::OverlappedPresenter>().IsAlwaysOnTop()
                );
                currentAudioProfile.ShowMenu(
                    AppBarGrid().Visibility() == Xaml::Visibility::Visible
                );
                currentAudioProfile.DisableAnimations(
                    mainAudioEndpointPeakTimer.IsRunning()
                );

                std::unique_lock lock{ audioSessionsMutex };
                for (auto iter : audioSessions)
                {
                    auto muted = iter.second->Muted();
                    auto volume = iter.second->Volume();
                    auto name = iter.second->Name();

                    currentAudioProfile.AudioLevels().Insert(name, volume);
                    currentAudioProfile.AudioStates().Insert(name, muted);
                }

                // If the user has a profile, the AudioProfile container has to exist, so no TryLookup and container creation.
                currentAudioProfile.Save(Storage::ApplicationData::Current().LocalSettings().Containers().Lookup(L"AudioProfiles"));
            }
        }
    }

    void MainWindow::LoadProfile(const hstring& profileName)
    {
        Storage::ApplicationDataContainer audioProfilesContainer = Storage::ApplicationData::Current().LocalSettings().Containers().TryLookup(L"AudioProfiles");
        if (audioProfilesContainer)
        {
            for (auto&& profile : audioProfilesContainer.Containers())
            {
                try
                {
                    hstring name = profile.Key();
                    if (name == profileName)
                    {
#pragma region Basic profile stuff
                        AudioProfile audioProfile{};
                        audioProfile.Restore(profile.Value()); // If the restore fails, the next instructions will not be ran.
                        currentAudioProfile = std::move(audioProfile);
                        // Cannot use audioProfile variable below here.

                        bool disableAnimations = currentAudioProfile.DisableAnimations();
                        bool keepOnTop = currentAudioProfile.KeepOnTop();
                        bool showMenu = currentAudioProfile.ShowMenu();
                        uint32_t windowLayout = currentAudioProfile.Layout();

                        if (disableAnimations)
                        {
                            DisableAnimationsIconButton_Click(nullptr, nullptr);
                        }

                        if (UI::OverlappedPresenter presenter = appWindow.Presenter().try_as<UI::OverlappedPresenter>())
                        {
                            presenter.IsAlwaysOnTop(keepOnTop);
                            presenter.IsMaximizable(!keepOnTop);
                            presenter.IsMinimizable(!keepOnTop);
                            KeepOnTopToggleButton().IsChecked(keepOnTop);

                            // 45px  -> Only close button.
                            // 135px -> Minimize + maximize + close buttons.
                            RightPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(keepOnTop ? 45 : 135));
                        }

                        if (showMenu)
                        {
                            ShowAppBarIconButton_Click(nullptr, nullptr);
                        }

                        switch (windowLayout)
                        {
                            case 1:
                                HorizontalViewMenuFlyoutItem_Click(nullptr, nullptr);
                                break;
                            case 2:
                                VerticalViewMenuFlyoutItem_Click(nullptr, nullptr);
                                break;
                            case 0:
                            default:
                                AutoViewMenuFlyoutItem_Click(nullptr, nullptr);
                                break;
                        }
#pragma endregion

                        AudioSessionsPanel().ItemsSource(nullptr);
                        AudioSessionsPanelProgressRing().Visibility(Xaml::Visibility::Visible);

                        // Finish loading profile in non-UI thread.
                        concurrency::task<void> t = concurrency::task<void>([this]()
                        {
                            try
                            {
                                auto audioLevels = currentAudioProfile.AudioLevels();
                                auto audioStates = currentAudioProfile.AudioStates();

                                // Set audio sessions volume.
                                std::unique_lock lock{ audioSessionsMutex }; // Taking the lock will also lock sessions from being added to the display.

                                // Set system volume.
                                float systemVolume = currentAudioProfile.SystemVolume();
                                mainAudioEndpoint->SetVolume(systemVolume);

                                for (auto pair : audioLevels)
                                {
                                    for (auto iter : audioSessions)
                                    {
                                        if (iter.second->Name() == pair.Key())
                                        {
                                            iter.second->SetVolume(pair.Value());
                                        }
                                    }
                                }

                                for (auto pair : audioStates)
                                {
                                    for (auto iter : audioSessions)
                                    {
                                        if (iter.second->Name() == pair.Key())
                                        {
                                            iter.second->Muted(pair.Value());
                                        }
                                    }
                                }

                                std::vector<AudioSessionView> uniqueSessions{};
                                // HACK: IVector<T>::GetMany does not seem to work. Manual copy.
                                for (uint32_t i = 0; i < audioSessionViews.Size(); i++)
                                {
                                    uniqueSessions.push_back(audioSessionViews.GetAt(i));
                                }

                                auto indexes = currentAudioProfile.SessionsIndexes();
                                auto&& views = std::vector<AudioSessionView>(indexes.Size(), nullptr);
                                for (size_t i = 0; i < uniqueSessions.size(); i++)
                                {
                                    auto&& view = uniqueSessions[i];

                                    auto opt = indexes.TryLookup(view.Header());
                                    if (opt.has_value())
                                    {
                                        size_t index = opt.value();
                                        if (index < views.size())
                                        {
                                            // The index might already be populated by the else if/else statements when the index is over the collection size.
                                            if (views[index] == nullptr)
                                            {
                                                views[index] = view;
                                            }
                                            else
                                            {
                                                views.push_back(views[index]);
                                                views[index] = view;
                                            }
                                        }
                                        else if (views[views.size() - 1] == nullptr) // Check if the last index of the collection is free.
                                        {
                                            views[views.size() - 1] = view;
                                        }
                                        else // The index is over the collection size and the last value of the collection is already populated.
                                        {
                                            views.push_back(view);
                                        }

                                        uniqueSessions[i] = nullptr; // Set to null to tell the session has been added.
                                    }
                                }

                                // Add the sessions that were not in the profile.
                                // Try to add the session where there is still space. If not possible, append it.
                                bool hasTrailingNullptrs = true;
                                for (size_t i = 0; i < uniqueSessions.size(); i++)
                                {
                                    if (uniqueSessions[i])
                                    {
                                        int j = static_cast<int>(views.size()) - 1;
                                        for (; j >= 0; j--)
                                        {
                                            if (views[j] == nullptr)
                                            {
                                                views[j] = uniqueSessions[i];
                                                break;
                                            }
                                        }

                                        if (j < 0)
                                        {
                                            views.push_back(uniqueSessions[i]);
                                            hasTrailingNullptrs = false;
                                        }
                                    }
                                }

                                if (hasTrailingNullptrs)
                                {
                                    //HACK: Clear null values by passing another time through the view array.
                                    size_t finalSize = views.size();
                                    for (size_t i = 0; i < views.size(); i++)
                                    {
                                        if (views[i] == nullptr)
                                        {
                                            size_t j = i + 1;
                                            while (j < views.size() && views[j] == nullptr)
                                            {
                                                j++;
                                            }

                                            if (j < views.size())
                                            {
                                                views[i] = std::move(views[j]);
                                            }
                                            else
                                            {
                                                finalSize--;
                                            }
                                        }
                                    }
                                    views.resize(finalSize);
                                }

                                audioSessionViews = multi_threaded_observable_vector<AudioSessionView>(move(views));
                                DispatcherQueue().TryEnqueue([this]()
                                {
                                    // HACK: Can we use INotifyPropertyChanged to raise that the vector has changed ?
                                    AudioSessionsPanel().ItemsSource(audioSessionViews);

                                // I18N: Loaded profile [profile name]
                                WindowMessageBar().EnqueueString(L"Loaded profile " + currentAudioProfile.ProfileName());
                                AudioSessionsPanelProgressRing().Visibility(Xaml::Visibility::Collapsed);
                            });
                        }
                        catch (const std::out_of_range& ex)
                        {
                            // TODO: Log exception to EventViewer to enable app analysis.
                            OutputDebugHString(to_hstring(ex.what()));
                            DispatcherQueue().TryEnqueue([this]()
                            {
                                WindowMessageBar().EnqueueString(L"Couldn't load profile " + currentAudioProfile.ProfileName());
                            });
                        }
                        catch (const hresult_error& err)
                        {
                            // I18N: Failed to load profile [profile name]
                            OutputDebugHString(err.message());
                            DispatcherQueue().TryEnqueue([this]()
                            {
                                WindowMessageBar().EnqueueString(L"Couldn't load profile " + currentAudioProfile.ProfileName());
                            });
                        }
                        });
                    }
                }
                catch (const hresult_error& error)
                {
                    // I18N: Failed to load profile [profile name]
                    WindowMessageBar().EnqueueString(L"Couldn't load profile " + profileName);
                    OutputDebugHString(error.message());
                    AudioSessionsPanelProgressRing().Visibility(Xaml::Visibility::Collapsed);
                }
            }
        }
    }

    void MainWindow::ReloadAudioSessions()
    {
        if (audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }

        audioSessionViews.Clear();
        VolumeStoryboard().Stop();

        // Unregister VolumeChanged event handler & unregister audio sessions from audio events and release com ptrs.
        {
            std::unique_lock lock{ audioSessionsMutex };
            for (auto& iter : audioSessions)
            {
                iter.second->VolumeChanged(audioSessionVolumeChanged[iter.first]);
                iter.second->StateChanged(audioSessionsStateChanged[iter.first]);
                iter.second->Unregister();
                iter.second->Release();
            }
            audioSessions.clear();
            // The lock can be realeased since no interactions will be made with audioSessions && audioSessionViews
        }

        try
        {
            // Reload audio sessions.
            auto audioSessionsVect = audioController->GetSessions();
            audioSessions = MapAudioSessions(audioSessionsVect);
            for (size_t i = 0; i < audioSessionsVect->size(); i++)
            {
                if (AudioSessionView view = CreateAudioView(audioSessionsVect->at(i)))
                {
                    audioSessionViews.Append(view);
                }
            }

            if (!DisableAnimationsIconToggleButton().IsOn())
            {
                audioSessionsPeakTimer.Start();
            }
        }
        catch (const winrt::hresult_error& err)
        {
            OutputDebugHString(L"Failed to reload audio sessions: " + err.message());
        }
    }

    std::map<guid, CAudio::AudioSession*> MainWindow::MapAudioSessions(std::vector<CAudio::AudioSession*>* vect)
    {
        std::map<guid, CAudio::AudioSession*> audioSessionsMap{};

        for (size_t i = 0; i < vect->size(); i++)
        {
            audioSessions.insert({ guid(vect->at(i)->Id()), vect->at(i) });
        }

        return move(audioSessionsMap);
    }

    void MainWindow::CleanUpResources(const bool& forReload)
    {
        if (audioSessionsPeakTimer && audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }

        if (!forReload && mainAudioEndpointPeakTimer && mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }

        VolumeStoryboard().Stop();

        // Unregister VolumeChanged event handler & unregister audio sessions from audio events and release com ptrs.
        if (audioSessions.size() > 0)
        {
            std::unique_lock lock{ audioSessionsMutex };

            for (auto iter : audioSessions)
            {
                iter.second->VolumeChanged(audioSessionVolumeChanged[iter.first]);
                iter.second->StateChanged(audioSessionsStateChanged[iter.first]);
                assert(iter.second->Unregister());
                assert(iter.second->Release() == 0);
            }
        }

        // Clean up ComPtr/IUnknown objects
        if (audioController)
        {
            if (mainAudioEndpoint)
            {
                mainAudioEndpoint->VolumeChanged(mainAudioEndpointVolumeChangedToken);
                mainAudioEndpoint->Unregister();
                mainAudioEndpoint->Release();
            }

            audioController->EndpointChanged(audioControllerEndpointChangedToken);
            audioController->SessionAdded(audioControllerSessionAddedToken);

            assert(audioController->Unregister());
            assert(audioController->Release() == 0);
        }

        if (forReload)
        {
            audioSessionViews.Clear();
        }
        else
        {
            SaveAudioLevels();
        }
    }

    void MainWindow::SetSizeIndicators()
    {
        double gridHeight = ContentGrid().ActualHeight();
        double gridWidth = ContentGrid().ActualWidth();

        RootCanvas().Children().Clear();

        int32_t position = 1;
        const int32_t sessionHeight = 290 + 2;
        while ((position * sessionHeight) < gridHeight)
        {
            winrt::Microsoft::UI::Xaml::Shapes::Rectangle rect{};
            rect.Width(10);
            rect.Height(1);
            auto&& brush = Xaml::SolidColorBrush(UI::Colors::DimGray());
            brush.Opacity(0.5);
            rect.Fill(brush);

            Xaml::Canvas::SetLeft(rect, 0);
            Xaml::Canvas::SetTop(rect, position * sessionHeight);
            RootCanvas().Children().Append(rect);

            position++;
        }

        position = 1;
        const int32_t sessionWidth = 80 + 3;
        while ((position * sessionWidth) < gridWidth)
        {
            winrt::Microsoft::UI::Xaml::Shapes::Rectangle rect{};
            rect.Width(1);
            rect.Height(10);
            auto&& brush = Xaml::SolidColorBrush(UI::Colors::DimGray());
            brush.Opacity(0.5);
            rect.Fill(brush);

            Xaml::Canvas::SetLeft(rect, position * sessionWidth);
            Xaml::Canvas::SetTop(rect, 0);
            RootCanvas().Children().Append(rect);

            position++;
        }
    }

    void MainWindow::UpdatePeakMeters(IInspectable, IInspectable)
    {
        if (!loaded || audioSessions.size() == 0) return;

        for (auto const& view : audioSessionViews)
        {
            auto&& pair = audioSessions.at(view.Id())->GetChannelsPeak();
            view.SetPeak(pair.first, pair.second);
        }
    }

    void MainWindow::HotKeyFired(const uint32_t& id, const Windows::Foundation::IInspectable&)
    {
        constexpr float stepping = 0.01f;
        constexpr float largeStepping = 0.07f;

        switch (id)
        {
            case VOLUME_DOWN:
            {
                try
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() - stepping > 0.f ? mainAudioEndpoint->Volume() - stepping : 0.f);
                }
                catch (...)
                {
                }

                break;
            }
            case VOLUME_UP:
            {
                try
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() + stepping < 1.f ? mainAudioEndpoint->Volume() + stepping : 1.f);
                }
                catch (...)
                {
                }

                break;
            }
            case VOLUME_UP_ALT:
            {
                try
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() + largeStepping < 1.f ? mainAudioEndpoint->Volume() + largeStepping : 1.f);
                }
                catch (...)
                {
                }

                break;
            }
            case VOLUME_DOWN_ALT:
            {
                try
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() - largeStepping > 0.f ? mainAudioEndpoint->Volume() - largeStepping : 0.f);
                }
                catch (...)
                {
                }

                break;
            }
            case MUTE_SYSTEM:
            {
                try
                {
                    mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());

                    DispatcherQueue().TryEnqueue([this]()
                    {
                        MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
                        MuteToggleButton().IsChecked(Foundation::IReference(mainAudioEndpoint->Muted()));
                    });
                }
                catch (...)
                {
                }

                break;
            }
            default:
            {
                DebugLog(std::format("Hot key not recognized. Id: {0}", id));
            }
        }
    }

    void MainWindow::AppWindow_Closing(winrt::Microsoft::UI::Windowing::AppWindow, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs)
    {
        SaveSettings();
        CleanUpResources(false);
    }

    void MainWindow::MainAudioEndpoint_VolumeChanged(IInspectable, const float& newVolume)
    {
        if (!loaded) return;

        DispatcherQueue().TryEnqueue([this, newVolume]()
        {
            SystemVolumeSlider().Value(newVolume * 100.);
        });
    }

    void MainWindow::AudioSession_VolumeChanged(const winrt::guid& id, const float& newVolume)
    {
        if (!loaded) return;

        DispatcherQueue().TryEnqueue([this, id, newVolume]()
        {
            for (auto const& view : audioSessionViews)
            {
                if (view.Id() == id)
                {
                    view.Volume(static_cast<double>(newVolume) * 100.0);
                }
            }
        });
    }

    void MainWindow::AudioSession_StateChanged(const winrt::guid& id, const uint32_t& state)
    {
        if (!loaded) return;

        // Cast state to AudioSessionState, uint32_t is only used to cross ABI
        AudioSessionState audioState = (AudioSessionState)state;
        if (audioState == AudioSessionState::Expired || audioState == AudioSessionState::Active)
        {
            audioSessionsPeakTimer.Stop();

            std::unique_lock lock{ audioSessionsMutex };

            if (audioSessions.contains(id))
            {
                if (audioState == AudioSessionState::Active)
                {
                    // Check if the newly active session is added to the UI, if not I need to add it as it might been skipped because of grouping params and the session being inactive at the time.
                    bool added = false;
                    for (auto const& view : audioSessionViews)
                    {
                        if (view.Id() == id)
                        {
                            added = true;
                            break;
                        }
                    }

                    if (!added)
                    {
                        lock.unlock();

                        DispatcherQueue().TryEnqueue([this, id]()
                        {
                            std::unique_lock lock{ audioSessionsMutex };

                            if (AudioSessionView view = CreateAudioSessionView(audioSessions.at(id), true))
                            {
                                audioSessionViews.InsertAt(0, view);
                            }
                        });

                        // The function exists here since the code in DispatcherQueue().TryEnqueue() relies on finding an AudioSessionView with the matching id. Or I found that the AudioSessionView with the given id does not exists.
                        audioSessionsPeakTimer.Start();
                        return;
                    }
                }
                else
                {
                    // The audio session is expired 
                    CAudio::AudioSession* session = audioSessions.at(id);
                    session->VolumeChanged(audioSessionVolumeChanged[session->Id()]);
                    session->StateChanged(audioSessionsStateChanged[session->Id()]);
                    assert(session->Unregister());
                    session->Release();

                    audioSessions.erase(id);
                }
            }

            audioSessionsPeakTimer.Start();
        }

        DispatcherQueue().TryEnqueue([this, id, audioState]()
        {
            for (auto const& view : audioSessionViews)
            {
                if (view.Id() == id)
                {
                    switch (audioState)
                    {
                        case AudioSessionState::Muted:
                            view.Muted(true);
                            break;

                        case AudioSessionState::Unmuted:
                            view.Muted(false);
                            break;

                        case AudioSessionState::Active:
                        case AudioSessionState::Inactive:
                            view.SetState(audioState);
                            break;

                        case AudioSessionState::Expired:
                            uint32_t indexOf = 0;
                            if (audioSessionViews.IndexOf(view, indexOf))
                            {
                                audioSessionViews.RemoveAt(indexOf);

                                if (audioSessionViews.Size() == 0)
                                {
                                    WindowMessageBar().EnqueueString(L"All sessions expired.");
                                }
                            }
                            break;
                    }

                    return;
                }
            }
        });
    }

    void MainWindow::AudioController_SessionAdded(IInspectable, IInspectable)
    {
        // TODO: Reorder audio sessions according to the currently loaded audio profile (if any).
        DispatcherQueue().TryEnqueue([this]()
        {
            audioSessionsPeakTimer.Stop();

            while (CAudio::AudioSession* newSession = audioController->NewSession())
            {
                {
                    std::unique_lock lock{ audioSessionsMutex };
                    audioSessions.insert({ newSession->Id(), newSession });
                }

                if (AudioSessionView view = CreateAudioView(newSession))
                {
                    // TODO: If a profile is loaded, find the right index for the audio session to be added to.
                    audioSessionViews.InsertAt(0, view);
                }
            }

            audioSessionsPeakTimer.Start();
        });
    }

    void MainWindow::AudioController_EndpointChanged(IInspectable, IInspectable)
    {
        // Stop animation while getting the new audio endpoint.
        mainAudioEndpointPeakTimer.Stop();

        mainAudioEndpoint->VolumeChanged(mainAudioEndpointVolumeChangedToken);
        mainAudioEndpoint->StateChanged(mainAudioEndpointStateChangedToken);
        mainAudioEndpoint->Unregister();
        mainAudioEndpoint->Release();

        mainAudioEndpoint = audioController->GetMainAudioEndpoint();
        if (mainAudioEndpoint->Register())
        {
            // Register to events.
            mainAudioEndpointVolumeChangedToken = mainAudioEndpoint->VolumeChanged({ this, &MainWindow::MainAudioEndpoint_VolumeChanged });
            mainAudioEndpointStateChangedToken = mainAudioEndpoint->StateChanged([this](IInspectable, bool muted)
            {
                DispatcherQueue().TryEnqueue([this, muted]()
                {
                    MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());
                    MuteToggleButtonFontIcon().Glyph(muted ? L"\ue74f" : L"\ue767");
                });
            });
        }


        DispatcherQueue().TryEnqueue([&]()
        {
            mainAudioEndpointPeakTimer.Start();

            MainEndpointNameTextBlock().Text(mainAudioEndpoint->Name());
            SystemVolumeSlider().Value(static_cast<double>(mainAudioEndpoint->Volume()) * 100.);
            MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());

            ReloadAudioSessions();
        });
    }
}