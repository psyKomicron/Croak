#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "HotKey.h"
#include "SecondWindow.xaml.h"
#include <ppl.h>
#include <ppltasks.h>
#include "IconHelper.h"

#define USE_TIMER 1
#define DEACTIVATE_TIMER 0
#define ENABLE_HOTKEYS 1

using namespace ::Croak::Audio;

using namespace std;

using namespace winrt;
using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Composition;
using namespace winrt::Microsoft::UI::Composition::SystemBackdrops;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::Resources;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;


namespace winrt::Croak::implementation
{
    winrt::Croak::MainWindow MainWindow::singleton{ nullptr };

    MainWindow::MainWindow(const winrt::hstring& args)
    {
        singleton = *this;

        InitializeComponent();
        InitializeWindow();
        SettingsButtonTeachingTip().Target(SettingsButton());

#ifdef DEBUG
        Application::Current().UnhandledException([&](IInspectable const&/*sender*/, UnhandledExceptionEventArgs const& e)
        {
            TextBlock block{};
        block.TextWrapping(TextWrapping::Wrap);
        block.Text(e.Message());
        WindowMessageBar().EnqueueString(e.Message());
        });
#endif // DEBUG

        if (unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"ShowSplashScreen"), true))
        {
            winrt::Windows::ApplicationModel::PackageId packageId = winrt::Windows::ApplicationModel::Package::Current().Id();
            ApplicationData::Current().LocalSettings().Values().Insert(L"ShowSplashScreen", IReference(false));

            ApplicationVersionTextBlock().Text(
                to_hstring(packageId.Version().Major) + L"." + to_hstring(packageId.Version().Minor) + L"." + to_hstring(packageId.Version().Build)
            );
        }
        else
        {
            WindowSplashScreen().Visibility(Visibility::Collapsed);
            ContentGrid().Visibility(Visibility::Visible);
        }

        // If the application has been restarted by the user, or started with arguments.
        wstring_view argsView{ args };
        if (argsView.find(L"-secondWindow") != string_view::npos) // -secondWindow -> open the second window/settings window.
        {
            SettingsIconButton_Click(nullptr, nullptr);
        }
    }


#pragma region Event handlers
    void MainWindow::OnLoaded(IInspectable const&, RoutedEventArgs const&)
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
        ApplicationDataContainer teachingTips = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TeachingTips");
        if (!teachingTips)
        {
            teachingTips = ApplicationData::Current().LocalSettings().CreateContainer(L"TeachingTips", ApplicationDataCreateDisposition::Always);
        }
        if (!teachingTips.Values().HasKey(L"ShowSettingsTeachingTip"))
        {
            teachingTips.Values().Insert(L"ShowSettingsTeachingTip", IReference(false));
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
                OutputDebugHString(L"User presence status changed: user present.");

                if (
                    unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true)
                    )
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
            case static_cast<int32_t>(UserPresenceStatus::Absent):
            case 2:
                OutputDebugHString(L"User presence status changed: user absent.");

                if (
                    unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true)
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
        });


        uint16_t lastMajor = 0;
        uint16_t lastMinor = 0;
        uint16_t lastBuild = 0;
        bool packageDifferentVersion = false;

        winrt::Windows::ApplicationModel::PackageId packageId = winrt::Windows::ApplicationModel::Package::Current().Id();

        ApplicationDataContainer lastPackageIdContainer{ nullptr };
        if (ApplicationData::Current().LocalSettings().Containers().HasKey(L"PackageId"))
        {
            lastPackageIdContainer = ApplicationData::Current().LocalSettings().Containers().Lookup(L"PackageId");
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
            lastPackageIdContainer = ApplicationData::Current().LocalSettings().CreateContainer(L"PackageId", ApplicationDataCreateDisposition::Always);
        }

        lastPackageIdContainer.Values().Insert(L"Major", box_value(packageId.Version().Major));
        lastPackageIdContainer.Values().Insert(L"Minor", box_value(packageId.Version().Minor));
        lastPackageIdContainer.Values().Insert(L"Build", box_value(packageId.Version().Build));

        if (packageDifferentVersion)
        {
            StackPanel panel{};
            TextBlock block{};
            block.Text(L"App has been updated");
            HyperlinkButton button{};
            button.Content(box_value(L"read notes here"));
            // I can assume this will not be dereferenced.
            button.Click([this](auto, auto)
            {
                if (!secondWindow)
                {
                    secondWindow = make<SecondWindow>();
                    secondWindow.Closed([this](IInspectable, WindowEventArgs)
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

    void MainWindow::Grid_SizeChanged(IInspectable const&, SizeChangedEventArgs const&)
    {
        if (usingCustomTitleBar)
        {
            SetDragRectangles();
        }

        if (appWindow.Size().Width < 210 && !compact)
        {
            compact = true;

            MuteToggleButtonColumn().Width(GridLengthHelper::FromPixels(0));
            SystemVolumeSliderTextColumn().Width(GridLengthHelper::FromPixels(0));

            MuteToggleButton().Visibility(Visibility::Collapsed);
            SystemVolumeNumberBlock().Visibility(Visibility::Collapsed);
        }
        else if (appWindow.Size().Width > 210 && compact)
        {
            compact = false;

            MuteToggleButtonColumn().Width(GridLengthHelper::Auto());
            SystemVolumeSliderTextColumn().Width(GridLengthHelper::FromPixels(26));

            MuteToggleButton().Visibility(Visibility::Visible);
            SystemVolumeNumberBlock().Visibility(Visibility::Visible);
        }
    }

    void MainWindow::AudioSessionView_VolumeChanged(AudioSessionView const& sender, RangeBaseValueChangedEventArgs const& args)
    {
        unique_lock lock{ audioSessionsMutex };
        audioSessions.at(sender.Id())->Volume(static_cast<float>(args.NewValue() / 100.0));
    }

    void MainWindow::AudioSessionView_VolumeStateChanged(winrt::Croak::AudioSessionView const& sender, bool const& args)
    {
        unique_lock lock{ audioSessionsMutex };
        audioSessions.at(sender.Id())->SetMute(args);
    }

    void MainWindow::AudioSessionsPanel_Loading(FrameworkElement const&, IInspectable const&)
    {
        LoadContent();

        if (unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"LoadLastProfile"), true))
        {
            hstring profileName = unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"AudioProfile"), L"");
            if (!profileName.empty())
            {
                LoadProfile(profileName);
            }
        }

#if ENABLE_HOTKEYS
        // Activate hotkeys.
        try
        {
            volumeUpHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueString(L"Failed to activate system volume up hot key");
        }

        try
        {
            volumeDownHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueString(L"Failed to activate system volume down hot key");
        }

        try
        {
            volumePageUpHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueString(L"Failed to activate system volume up (PageUp) hot key");
        }

        try
        {
            volumePageDownHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueString(L"Failed to activate system volume down (PageDown) hot key");
        }

        try
        {
            muteHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueString(L"Failed to activate mute/unmute hot key");
        }
#endif // ENABLE_HOTKEYS
    }

    void MainWindow::SystemVolumeSlider_ValueChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        if (mainAudioEndpoint)
        {
            mainAudioEndpoint->Volume(static_cast<float>(e.NewValue() / 100.));
        }

        SystemVolumeNumberBlock().Double(e.NewValue());
    }

    void MainWindow::SystemVolumeActivityBorder_SizeChanged(IInspectable const&, SizeChangedEventArgs const&)
    {
        SystemVolumeActivityBorderClippingRight().Rect(
            Rect(0, 0, static_cast<float>(SystemVolumeActivityBorderRight().ActualWidth()), static_cast<float>(SystemVolumeActivityBorderRight().ActualHeight()))
        );
        SystemVolumeActivityBorderClippingLeft().Rect(
            Rect(0, 0, static_cast<float>(SystemVolumeActivityBorderLeft().ActualWidth()), static_cast<float>(SystemVolumeActivityBorderLeft().ActualHeight()))
        );
    }

    void MainWindow::HideMenuFlyoutItem_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        appWindow.Presenter().as<OverlappedPresenter>().Minimize();
    }

    void MainWindow::SwitchStateMenuFlyoutItem_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
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
            unique_lock lock{ audioSessionsMutex };
            for (pair<guid, AudioSession*> iter : audioSessions)
            {
                iter.second->Muted(setMute);
            }
        }

        for (AudioSessionView view : audioSessionViews)
        {
            view.Muted(setMute);
        }
    }

    void MainWindow::HorizontalViewMenuFlyoutItem_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(ScrollBarVisibility::Disabled);
        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewHorizontalLayout")).as<Style>()
        );
        layout = 1;

        // Set DropDownButton to mimic ComboBox selected item behavior.
        FontIcon icon{};
        icon.Glyph(L"\ue8c0");
        LayoutDropDownButton().Content(icon);

        for (auto&& view : audioSessionViews)
        {
            view.Orientation(Orientation::Vertical);
        }
    }

    void MainWindow::VerticalViewMenuFlyoutItem_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewVerticalLayout")).as<Style>()
        );
        layout = 2;

        // Set DropDownButton to mimic ComboBox selected item behavior.
        FontIcon icon{};
        icon.Glyph(L"\ue8c0");
        icon.Translation(::Numerics::float3(16, 0, 0));
        icon.Rotation(90);
        LayoutDropDownButton().Content(icon);

        for (auto&& view : audioSessionViews)
        {
            view.Orientation(Orientation::Horizontal);
        }
    }

    void MainWindow::AutoViewMenuFlyoutItem_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(ScrollBarVisibility::Auto);

        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewHorizontalLayout")).as<Style>()
        );
        layout = 0;

        // Set DropDownButton to mimic ComboBox selected item behavior.
        FontIcon icon{};
        icon.Glyph(L"\uf0e2");
        LayoutDropDownButton().Content(icon);

        for (auto&& view : audioSessionViews)
        {
            view.Orientation(Orientation::Vertical);
        }
    }

    void MainWindow::SettingsIconButton_Click(IconButton const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
        if (!secondWindow)
        {
            secondWindow = make<SecondWindow>();
            secondWindow.Closed([this](IInspectable, WindowEventArgs)
            {
                secondWindow = nullptr;
            });
        }
        secondWindow.Activate();
    }

    void MainWindow::DisableHotKeysIconButton_Click(IconToggleButton const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
#if ENABLE_HOTKEYS
        volumeUpHotKeyPtr.Enabled(!volumeUpHotKeyPtr.Enabled());
        volumeDownHotKeyPtr.Enabled(!volumeDownHotKeyPtr.Enabled());
        volumePageUpHotKeyPtr.Enabled(!volumePageUpHotKeyPtr.Enabled());
        volumePageDownHotKeyPtr.Enabled(!volumePageDownHotKeyPtr.Enabled());
        muteHotKeyPtr.Enabled(!muteHotKeyPtr.Enabled());

        ResourceLoader loader{};
        if (muteHotKeyPtr.Enabled())
        {
            WindowMessageBar().EnqueueString(loader.GetString(L"InfoHotKeysEnabled"));
        }
        else
        {
            WindowMessageBar().EnqueueString(loader.GetString(L"InfoHotKeysDisabled"));
        }
#endif // ENABLE_HOTKEYS
        SettingsButtonFlyout().Hide();
    }

    void MainWindow::ShowAppBarIconButton_Click(IconToggleButton const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
        AppBarGrid().Visibility(AppBarGrid().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);
    }

    void MainWindow::DisableAnimationsIconButton_Click(IconToggleButton const& /*sender*/, RoutedEventArgs const& /*args*/)
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

    void MainWindow::MuteToggleButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());
        MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
    }

    void MainWindow::ViewHotKeysHyperlinkButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void MainWindow::SplashScreen_Dismissed(winrt::Croak::SplashScreen const&, IInspectable const&)
    {
        WindowSplashScreen().Visibility(Visibility::Collapsed);
        ContentGrid().Visibility(Visibility::Visible);
    }

    void MainWindow::ExpandFlyoutButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MoreFlyoutGrid().Visibility(
            MoreFlyoutGrid().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible
        );
    }

    void MainWindow::KeepOnTopToggleButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        OverlappedPresenter presenter = appWindow.Presenter().as<OverlappedPresenter>();
        presenter.IsMaximizable(!presenter.IsMaximizable());
        presenter.IsMinimizable(!presenter.IsMinimizable());

        bool alwaysOnTop = KeepOnTopToggleButton().IsChecked().GetBoolean();
        presenter.IsAlwaysOnTop(alwaysOnTop);
        ApplicationData::Current().LocalSettings().Values().Insert(L"IsAlwaysOnTop", box_value(alwaysOnTop));

        RightPaddingColumn().Width(GridLengthHelper::FromPixels(
            presenter.IsMinimizable() ? 135 : 45
        ));
    }

    void MainWindow::ReloadSessionsIconButton_Click(IconButton const&, RoutedEventArgs const&)
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
            unique_lock lock{ audioSessionsMutex };
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

        ResourceLoader loader{};
        WindowMessageBar().EnqueueString(loader.GetString(L"InfoAudioSessionsReloaded"));
        SettingsButtonFlyout().Hide();
    }

    void MainWindow::RestartIconButton_Click(IconButton const&, RoutedEventArgs const&)
    {
        if (Microsoft::Windows::AppLifecycle::AppInstance::Restart(secondWindow ? L"-secondWindow" : L"") != AppRestartFailureReason::RestartPending)
        {
            ResourceLoader loader{};
            WindowMessageBar().EnqueueString(loader.GetString(L"ErrorAppFailedRestart"));
        }
    }

    void MainWindow::OpenProfilesIconButton_Click(IconButton const&, RoutedEventArgs const&)
    {
        MoreFlyoutStackpanel().Visibility(Visibility::Collapsed);
        ProfilesGrid().Visibility(Visibility::Visible);

        ProfilesStackpanel().Children().Clear();
        ApplicationDataContainer audioProfilesContainer = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"AudioProfiles");
        if (audioProfilesContainer)
        {
            for (auto&& profile : audioProfilesContainer.Containers())
            {
                hstring key = profile.Key();
                ToggleButton item{};
                item.Content(box_value(key));
                item.Tag(box_value(key));
                item.IsChecked(currentAudioProfile && (key == currentAudioProfile.ProfileName()));
                item.HorizontalAlignment(HorizontalAlignment::Stretch);
                item.Background(SolidColorBrush(Colors::Transparent()));
                item.BorderThickness(Thickness(0));

                item.Click([this](const IInspectable& sender, RoutedEventArgs)
                {
                    MoreFlyoutStackpanel().Visibility(Visibility::Visible);
                ProfilesGrid().Visibility(Visibility::Collapsed);
                SettingsButtonFlyout().Hide();
                LoadProfile(sender.as<FrameworkElement>().Tag().as<hstring>());
                });

                ProfilesStackpanel().Children().Append(item);
            }
        }
    }

    void MainWindow::CloseProfilesButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MoreFlyoutStackpanel().Visibility(Visibility::Visible);
        ProfilesGrid().Visibility(Visibility::Collapsed);
    }

    void MainWindow::OpenProfilesButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!secondWindow)
        {
            secondWindow = make<SecondWindow>();
            secondWindow.Closed([this](IInspectable, WindowEventArgs)
            {
                secondWindow = nullptr;
            });
        }
        secondWindow.Activate();
        secondWindow.NavigateTo(xaml_typename<AudioProfilesPage>());
    }

    void MainWindow::RootGrid_ActualThemeChanged(FrameworkElement const&, IInspectable const&)
    {
        WindowMessageBar().EnqueueString(L"Actual application theme has changed. Loading new theme right now, all effects will be applied on restart.");
        // Change title bar buttons background to fit the new theme.
        if (usingCustomTitleBar)
        {
            if (RootGrid().ActualTheme() == ElementTheme::Light)
            {
                appWindow.TitleBar().ButtonForegroundColor(Colors::Black());
                appWindow.TitleBar().ButtonHoverForegroundColor(Colors::Black());
                appWindow.TitleBar().ButtonPressedForegroundColor(Colors::Black());
            }
            else if (RootGrid().ActualTheme() == ElementTheme::Dark)
            {
                appWindow.TitleBar().ButtonForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
            }
        }

        if (systemBackdropConfiguration)
        {
            systemBackdropConfiguration.Theme((SystemBackdropTheme)RootGrid().ActualTheme());
            backdropController.TintColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
            backdropController.FallbackColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
        }

        // HACK: Flyout grids are not auto updating their backgrounds when the theme change.
        Brush brush = SolidColorBrush(
            Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorSecondary")).as<Windows::UI::Color>()
        );
        brush.Opacity(0.4);
        CommandBarGrid().Background(brush);
        SettingsButtonFlyoutGrid().Background(
            SolidColorBrush(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>())
        );
        MoreFlyoutGrid().Background(
            Application::Current().Resources().TryLookup(box_value(L"SubtleFillColorSecondaryBrush")).as<Brush>()
        );
    }
#pragma endregion


    void MainWindow::InitializeWindow()
    {
        auto nativeWindow{ this->try_as<::IWindowNative>() };
        check_bool(nativeWindow);
        HWND handle = nullptr;
        nativeWindow->get_WindowHandle(&handle);
        WindowId windowID = GetWindowIdFromWindow(handle);
        appWindow = AppWindow::GetFromWindowId(windowID);
        if (appWindow != nullptr)
        {
#ifdef DEBUG
            TextBlock timestamp{};
            timestamp.IsHitTestVisible(false);
            timestamp.Opacity(0.6);
            timestamp.HorizontalAlignment(HorizontalAlignment::Right);
            timestamp.VerticalAlignment(VerticalAlignment::Bottom);
            timestamp.Text(to_hstring(__TIME__) + L", " + to_hstring(__DATE__));

            Grid::SetRow(timestamp, WindowGrid().RowDefinitions().GetView().Size() - 1);
            RootGrid().Children().Append(timestamp);
#endif // _DEBUG

            appWindow.Title(Application::Current().Resources().Lookup(box_value(L"AppTitle")).as<hstring>());

            appWindow.Closing({ this, &MainWindow::AppWindow_Closing });
            appWindow.Changed([this](AppWindow, winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs args)
            {
                OverlappedPresenter presenter = nullptr;
            if ((presenter = appWindow.Presenter().try_as<OverlappedPresenter>()))
            {
                if (args.DidSizeChange())
                {
                    int32_t minimum = presenter.IsMinimizable() ? 250 : 130;
                    if (appWindow.Size().Width < minimum)
                    {
                        appWindow.Resize(SizeInt32(minimum, appWindow.Size().Height));
                    }
                }

                if (presenter.State() != OverlappedPresenterState::Maximized)
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

            if (unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"UseCustomTitleBar"), true) &&
                appWindow.TitleBar().IsCustomizationSupported())
            {
                usingCustomTitleBar = true;

                appWindow.TitleBar().ExtendsContentIntoTitleBar(true);
                appWindow.TitleBar().IconShowOptions(IconShowOptions::HideIconAndSystemMenu);

                LeftPaddingColumn().Width(GridLengthHelper::FromPixels(static_cast<double>(appWindow.TitleBar().LeftInset())));

                appWindow.TitleBar().ButtonBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveForegroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"AppTitleBarHoverColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonHoverBackgroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"ButtonHoverBackgroundColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonPressedBackgroundColor(Colors::Transparent());

                if (RootGrid().ActualTheme() == ElementTheme::Light)
                {
                    appWindow.TitleBar().ButtonForegroundColor(Colors::Black());
                    appWindow.TitleBar().ButtonHoverForegroundColor(Colors::Black());
                    appWindow.TitleBar().ButtonPressedForegroundColor(Colors::Black());
                }
                else if (RootGrid().ActualTheme() == ElementTheme::Dark)
                {
                    appWindow.TitleBar().ButtonForegroundColor(Colors::White());
                    appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
                    appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
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

        volumeUpHotKeyPtr.Fired([this](auto, auto)
        {
            try
        {
            constexpr float stepping = 0.02f;
            mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() + stepping < 1.f ? mainAudioEndpoint->Volume() + stepping : 1.f);
        }
        catch (...)
        {
        }
        });

        volumeDownHotKeyPtr.Fired([this](auto, auto)
        {
            try
        {
            constexpr float stepping = 0.02f;
            mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() - stepping > 0.f ? mainAudioEndpoint->Volume() - stepping : 0.f);
        }
        catch (...)
        {
        }
        });

        volumePageUpHotKeyPtr.Fired([this](auto, auto)
        {
            try
        {
            constexpr float stepping = 0.07f;
            mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() + stepping < 1.f ? mainAudioEndpoint->Volume() + stepping : 1.f);
        }
        catch (...)
        {
        }
        });

        volumePageDownHotKeyPtr.Fired([this](auto, auto)
        {
            try
        {
            constexpr float stepping = 0.07f;
            mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() - stepping > 0.f ? mainAudioEndpoint->Volume() - stepping : 0.f);
        }
        catch (...)
        {
        }
        });

        muteHotKeyPtr.Fired([this](auto, auto)
        {
            try
        {
            mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());
            /*DispatcherQueue().TryEnqueue([this]()
            {
                MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
                MuteToggleButton().IsChecked(IReference(mainAudioEndpoint->Muted()));
            });*/
        }
        catch (...)
        {
        }
        });

#pragma warning(pop)  
#endif // ENABLE_HOTKEYS
    }

    void MainWindow::SetBackground()
    {
        if (unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"TransparencyAllowed"), true))
        {
            if (DesktopAcrylicController::IsSupported())
            {
                auto&& supportsBackdrop = try_as<ICompositionSupportsSystemBackdrop>();
                if (supportsBackdrop)
                {
                    WindowGrid().Background(SolidColorBrush(Colors::Transparent()));

                    if (!DispatcherQueue::GetForCurrentThread() && !dispatcherQueueController)
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

                    systemBackdropConfiguration = SystemBackdropConfiguration();
                    systemBackdropConfiguration.IsInputActive(true);
                    systemBackdropConfiguration.Theme((SystemBackdropTheme)RootGrid().ActualTheme());

                    backdropController = BackdropController();
                    backdropController.TintColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                    backdropController.FallbackColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                    backdropController.TintOpacity(static_cast<float>(Application::Current().Resources().TryLookup(box_value(L"BackdropTintOpacity")).as<double>()));
                    backdropController.LuminosityOpacity(static_cast<float>(Application::Current().Resources().TryLookup(box_value(L"BackdropLuminosityOpacity")).as<double>()));
                    backdropController.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                    backdropController.AddSystemBackdropTarget(supportsBackdrop);
                }
            }
        }
        else
        {
            hstring path = unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"BackgroundImageUri"), L"");
            if (!path.empty())
            {
                try
                {
                    BitmapImage image{};
                    image.UriSource(Uri(path));
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
        ResourceLoader loader{};
        GUID appID{};
        if (SUCCEEDED(UuidCreate(&appID)))
        {
            try
            {
                // Create and setup audio interfaces.
                audioController = new AudioController(appID);

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
                auto audioSessionsVector = unique_ptr<vector<AudioSession*>>();
                try
                {
                    audioSessionsVector = unique_ptr<vector<AudioSession*>>(audioController->GetSessions());
                    for (size_t i = 0; i < audioSessionsVector->size(); i++)
                    {
                        audioSessions.insert({ guid(audioSessionsVector->at(i)->Id()), audioSessionsVector->at(i) });

                        // Check if the session is active, if not check if the user asked to show inactive sessions on startup.
                        if (audioSessionsVector->at(i)->State() == ::AudioSessionState::AudioSessionStateActive ||
                            unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"ShowInactiveSessionsOnStartup"), false))
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

                audioSessionsPeakTimer.Interval(TimeSpan(std::chrono::milliseconds(83)));
                audioSessionsPeakTimer.Tick({ this, &MainWindow::UpdatePeakMeters });
                audioSessionsPeakTimer.Stop();

                mainAudioEndpointPeakTimer.Interval(TimeSpan(std::chrono::milliseconds(83)));
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

    AudioSessionView MainWindow::CreateAudioView(AudioSession* audioSession)
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

    AudioSessionView MainWindow::CreateAudioSessionView(AudioSession* audioSession, bool skipDuplicates)
    {
        // Check for duplicates, multiple audio sessions might be grouped under one by the app/system owning the sessions.
        //checkForDuplicates = false;
        if (!skipDuplicates && audioSession->State() != ::AudioSessionState::AudioSessionStateActive)
        {
            unique_lock lock{ audioSessionsMutex };

            uint8_t hitcount = 0u;
            for (auto iter : audioSessions)
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

            FontIcon icon{};
            icon.Glyph(L"\ue770");
            icon.Foreground(
                SolidColorBrush(Application::Current().Resources().TryLookup(box_value(L"SystemAccentColorLight1")).as<Windows::UI::Color>())
            );
            Viewbox iconViewbox{};
            iconViewbox.HorizontalAlignment(HorizontalAlignment::Stretch);
            iconViewbox.VerticalAlignment(VerticalAlignment::Stretch);
            iconViewbox.Child(icon);
            view.Logo().Content(iconViewbox);
        }
        else
        {
            if (!audioSession->ProcessInfo()->Manifest().Logo().empty())
            {
                view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);

                BitmapImage imageSource{};
                imageSource.UriSource(Uri(audioSession->ProcessInfo()->Manifest().Logo()));
                Image image{};
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
        RectInt32 dragRectangle{};

        dragRectangle.X = static_cast<int32_t>(LeftPaddingColumn().ActualWidth() + SettingsButtonColumn().ActualWidth());
        dragRectangle.Y = 0;
        dragRectangle.Width = static_cast<int32_t>(TitleBarGrid().ActualWidth() - (LeftPaddingColumn().ActualWidth() + SettingsButtonColumn().ActualWidth()));
        dragRectangle.Height = static_cast<int32_t>(TitleBarGrid().ActualHeight());

        appWindow.TitleBar().SetDragRectangles(vector<RectInt32>{ dragRectangle });
    }

    void MainWindow::SaveAudioLevels()
    {
        ApplicationDataContainer audioLevels = ApplicationData::Current().LocalSettings().CreateContainer(
            L"AudioLevels",
            ApplicationData::Current().LocalSettings().Containers().HasKey(L"AudioLevels") ? ApplicationDataCreateDisposition::Existing : ApplicationDataCreateDisposition::Always
        );

        for (uint32_t i = 0; i < audioSessionViews.Size(); i++) // Only saving the visible audio sessions levels.
        {
            ApplicationDataCompositeValue compositeValue{};
            compositeValue.Insert(L"Muted", box_value(audioSessionViews.GetAt(i).Muted()));
            compositeValue.Insert(L"Level", box_value(audioSessionViews.GetAt(i).Volume()));
            if (!audioSessionViews.GetAt(i).Header().empty())
            {
                audioLevels.Values().Insert(audioSessionViews.GetAt(i).Header(), compositeValue);
            }
        }
    }

    void MainWindow::LoadSettings()
    {
        IPropertySet settings = ApplicationData::Current().LocalSettings().Values();

        RectInt32 rect{};
        rect.Width = unbox_value_or(
            settings.TryLookup(L"WindowWidth"),
            Application::Current().Resources().Lookup(box_value(L"WindowWidth")).as<int32_t>()
        );
        rect.Height = unbox_value_or(
            settings.TryLookup(L"WindowHeight"),
            Application::Current().Resources().Lookup(box_value(L"WindowHeight")).as<int32_t>()
        );
        rect.X = unbox_value_or(settings.TryLookup(L"WindowPosX"), -1);
        rect.Y = unbox_value_or(settings.TryLookup(L"WindowPosY"), -1);
        DisplayArea display = DisplayArea::GetFromPoint(PointInt32(rect.X, rect.Y), DisplayAreaFallback::None);
        if (!display || rect.X < 0 || rect.Y < 0)
        {
            display = DisplayArea::GetFromPoint(PointInt32(rect.X, rect.Y), DisplayAreaFallback::Primary);
            rect.X = display.WorkArea().X + 10;
            rect.Y = display.WorkArea().X + 10;
        }
        appWindow.MoveAndResize(rect);


        auto&& presenter = appWindow.Presenter().as<OverlappedPresenter>();

        bool alwaysOnTop = unbox_value_or(settings.TryLookup(L"IsAlwaysOnTop"), false);
        OverlappedPresenterState presenterState = static_cast<OverlappedPresenterState>(unbox_value_or(settings.TryLookup(L"PresenterState"), 2));
        switch (presenterState)
        {
            case OverlappedPresenterState::Maximized:
                presenter.Maximize();
                break;
            case OverlappedPresenterState::Minimized:
                presenter.Minimize();
                break;
                /*case OverlappedPresenterState::Restored:
                default:
                    break;*/
        }

        presenter.IsMaximizable(!alwaysOnTop);
        presenter.IsMinimizable(!alwaysOnTop);
        RightPaddingColumn().Width(GridLengthHelper::FromPixels(
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
            AppBarGrid().Visibility(Visibility::Visible);
        }
    }

    void MainWindow::SaveSettings()
    {
        IPropertySet settings = ApplicationData::Current().LocalSettings().Values();
        settings.Insert(L"WindowHeight", box_value(displayRect.Height));
        settings.Insert(L"WindowWidth", box_value(displayRect.Width));
        settings.Insert(L"WindowPosX", box_value(displayRect.X));
        settings.Insert(L"WindowPosY", box_value(displayRect.Y));

        auto presenter = appWindow.Presenter().as<OverlappedPresenter>();
        settings.Insert(L"IsAlwaysOnTop", box_value(presenter.IsAlwaysOnTop()));
        settings.Insert(L"PresenterState", box_value(static_cast<int32_t>(presenter.State())));

        settings.Insert(L"DisableAnimations", box_value(DisableAnimationsIconToggleButton().IsOn()));
        settings.Insert(L"SessionsLayout", IReference<int32_t>(layout));
        settings.Insert(L"ShowAppBar", box_value(ShowAppBarIconToggleButton().IsOn()));

        if (currentAudioProfile)
        {
            settings.Insert(L"AudioProfile", box_value(currentAudioProfile.ProfileName()));

            if (unbox_value_or(settings.TryLookup(L"AllowChangesToLoadedProfile"), false))
            {
                currentAudioProfile.SystemVolume(mainAudioEndpoint->Volume());
                currentAudioProfile.Layout(layout);
                currentAudioProfile.KeepOnTop(
                    appWindow.Presenter().as<OverlappedPresenter>().IsAlwaysOnTop()
                );
                currentAudioProfile.ShowMenu(
                    AppBarGrid().Visibility() == Visibility::Visible
                );
                currentAudioProfile.DisableAnimations(
                    mainAudioEndpointPeakTimer.IsRunning()
                );

                unique_lock lock{ audioSessionsMutex };
                for (auto iter : audioSessions)
                {
                    auto muted = iter.second->Muted();
                    auto volume = iter.second->Volume();
                    auto name = iter.second->Name();

                    currentAudioProfile.AudioLevels().Insert(name, volume);
                    currentAudioProfile.AudioStates().Insert(name, muted);
                }

                // If the user has a profile, the AudioProfile container has to exist, so no TryLookup and container creation.
                currentAudioProfile.Save(ApplicationData::Current().LocalSettings().Containers().Lookup(L"AudioProfiles"));
            }
        }
    }

    void MainWindow::LoadProfile(const hstring& profileName)
    {
        ApplicationDataContainer audioProfilesContainer = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"AudioProfiles");
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
                        currentAudioProfile = move(audioProfile);
                        // Cannot use audioProfile variable below here.

                        bool disableAnimations = currentAudioProfile.DisableAnimations();
                        bool keepOnTop = currentAudioProfile.KeepOnTop();
                        bool showMenu = currentAudioProfile.ShowMenu();
                        uint32_t windowLayout = currentAudioProfile.Layout();

                        if (disableAnimations)
                        {
                            DisableAnimationsIconButton_Click(nullptr, nullptr);
                        }

                        if (OverlappedPresenter presenter = appWindow.Presenter().try_as<OverlappedPresenter>())
                        {
                            presenter.IsAlwaysOnTop(keepOnTop);
                            presenter.IsMaximizable(!keepOnTop);
                            presenter.IsMinimizable(!keepOnTop);
                            KeepOnTopToggleButton().IsChecked(keepOnTop);

                            // 45px  -> Only close button.
                            // 135px -> Minimize + maximize + close buttons.
                            RightPaddingColumn().Width(GridLengthHelper::FromPixels(keepOnTop ? 45 : 135));
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
                        AudioSessionsPanelProgressRing().Visibility(Visibility::Visible);

                        // Finish loading profile in non-UI thread.
                        concurrency::task<void> t = concurrency::task<void>([this]()
                        {
                            try
                            {
                                auto audioLevels = currentAudioProfile.AudioLevels();
                                auto audioStates = currentAudioProfile.AudioStates();

                                // Set audio sessions volume.
                                unique_lock lock{ audioSessionsMutex }; // Taking the lock will also lock sessions from being added to the display.

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

                                vector<AudioSessionView> uniqueSessions{};
                                // HACK: IVector<T>::GetMany does not seem to work. Manual copy.
                                for (uint32_t i = 0; i < audioSessionViews.Size(); i++)
                                {
                                    uniqueSessions.push_back(audioSessionViews.GetAt(i));
                                }

                                auto indexes = currentAudioProfile.SessionsIndexes();
                                auto&& views = vector<AudioSessionView>(indexes.Size(), nullptr);
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
                                                views[i] = move(views[j]);
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
                                AudioSessionsPanelProgressRing().Visibility(Visibility::Collapsed);
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
                    AudioSessionsPanelProgressRing().Visibility(Visibility::Collapsed);
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
            unique_lock lock{ audioSessionsMutex };
            for (auto iter : audioSessions)
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

    map<guid, AudioSession*> MainWindow::MapAudioSessions(vector<AudioSession*>* vect)
    {
        map<guid, AudioSession*> audioSessionsMap{};

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
            unique_lock lock{ audioSessionsMutex };

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

    void MainWindow::UpdatePeakMeters(IInspectable, IInspectable)
    {
        if (!loaded || audioSessions.size() == 0) return;

        for (auto const& view : audioSessionViews)
        {
            auto&& pair = audioSessions.at(view.Id())->GetChannelsPeak();
            view.SetPeak(pair.first, pair.second);
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

            unique_lock lock{ audioSessionsMutex };

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
                            unique_lock lock{ audioSessionsMutex };

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
                    AudioSession* session = audioSessions.at(id);
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

            while (AudioSession* newSession = audioController->NewSession())
            {
                {
                    unique_lock lock{ audioSessionsMutex };
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
