#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "SecondWindow.xaml.h"
#include "IconHelper.h"
#include "HotKey.h"
#include "HotKeyManager.h"
#include "DebugOutput.h"

#define ENABLE_HOTKEYS

constexpr int MOVE_WINDOW = 6;
constexpr int MOVE_WINDOW_ALT = 7;

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
}


namespace winrt::Croak::implementation
{
    winrt::Croak::MainWindow MainWindow::singleton{ nullptr };

    MainWindow::MainWindow(const winrt::hstring& args)
    {
        singleton = *this;

        InitializeComponent();
        AudioViewer().SetMessageBar(WindowMessageBar());
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
            AudioViewer().Visibility(Xaml::Visibility::Visible);
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
                        DisableAnimationsIconToggleButton().IsOn(false);
                    });
                    break;
                }
            }
        });

        bool packageDifferentVersion = false;
        uint16_t lastMajor = 0;
        uint16_t lastMinor = 0;
        uint16_t lastBuild = 0;
        Storage::PackageId packageId = Storage::Package::Current().Id();
        Storage::ApplicationDataContainer lastPackageIdContainer{ nullptr };
        if (Storage::ApplicationData::Current().LocalSettings().Containers().HasKey(L"PackageId"))
        {
            lastPackageIdContainer = Storage::ApplicationData::Current().LocalSettings().Containers().Lookup(L"PackageId");

            lastMajor = unbox_value<uint16_t>(lastPackageIdContainer.Values().Lookup(L"Major"));
            lastMinor = unbox_value<uint16_t>(lastPackageIdContainer.Values().Lookup(L"Minor"));
            lastBuild = unbox_value<uint16_t>(lastPackageIdContainer.Values().Lookup(L"Build"));

            auto major = packageId.Version().Major;
            auto minor = packageId.Version().Minor;
            auto build = packageId.Version().Build;

            packageDifferentVersion = major != lastMajor || minor != lastMinor || build != lastBuild;
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
            // I18N: App has been updated && read notes here.
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
                    secondWindow.Closed([this](IInspectable, Xaml::WindowEventArgs)
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

    void MainWindow::Window_Activated(IInspectable const&, Xaml::WindowActivatedEventArgs const& e)
    {
        windowActivated = e.WindowActivationState() != Xaml::WindowActivationState::Deactivated;

        if (e.WindowActivationState() == Xaml::WindowActivationState::Deactivated && 
            OverlayModeToggleButton().IsChecked().GetBoolean())
        {
            if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"HideWindowInOverlayMode"), false))
            {
                appWindow.Hide();
            }
            else
            {
                appWindow.Presenter().as<UI::OverlappedPresenter>().Minimize();
            }
        }
    }

    void MainWindow::Grid_SizeChanged(IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        if (usingCustomTitleBar)
        {
            SetDragRectangles();
        }

        /*if (appWindow.Size().Width < 210 && !compact)
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
        }*/
    }

    void MainWindow::OnLoading(FrameworkElement const&, IInspectable const&)
    {
        LoadContent();

        hstring profileName = unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"UserProfile"), L"");
        if (!profileName.empty())
        {
            LoadProfileByName(profileName);
        }
    }

    void MainWindow::HideMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        appWindow.Presenter().as<UI::OverlappedPresenter>().Minimize();
    }

    void MainWindow::SwitchStateMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        // TODO: Switch state.
       AudioViewer().SwitchAudioState();
    }

    void MainWindow::HorizontalViewMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        // Set DropDownButton to mimic ComboBox selected item behavior.
        Xaml::FontIcon icon{};
        icon.Glyph(L"\ue8c0");
        LayoutDropDownButton().Content(icon);

        AudioViewer().SetAudioSessionLayout(AudioSessionLayout::Horizontal);
    }

    void MainWindow::VerticalViewMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        // Set DropDownButton to mimic ComboBox selected item behavior.
        Xaml::FontIcon icon{};
        icon.Glyph(L"\ue8c0");
        icon.Translation(Foundation::Numerics::float3(16, 0, 0));
        icon.Rotation(90);
        LayoutDropDownButton().Content(icon);

        AudioViewer().SetAudioSessionLayout(AudioSessionLayout::Vertical);
    }

    void MainWindow::AutoViewMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        // Set DropDownButton to mimic ComboBox selected item behavior.
        Xaml::FontIcon icon{};
        icon.Glyph(L"\uf0e2");
        LayoutDropDownButton().Content(icon);

        AudioViewer().SetAudioSessionLayout(AudioSessionLayout::Auto);
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
        hotKeysEnabled = !hotKeysEnabled;
        ::Croak::System::HotKeys::HotKeyManager::GetHotKeyManager().EditKey(MOVE_WINDOW, hotKeysEnabled);

        WindowMessageBar().EnqueueString(resourceLoader.GetString(hotKeysEnabled ? L"InfoHotKeysEnabled" : L"InfoHotKeysDisabled"));

        SettingsButtonFlyout().Hide();
    }

    void MainWindow::ShowAppBarIconButton_Click(IconToggleButton const& /*sender*/, Xaml::RoutedEventArgs const& /*args*/)
    {
        //AudioViewer().ShowAppBar();
    }

    void MainWindow::DisableAnimationsIconButton_Click(IconToggleButton const& /*sender*/, Xaml::RoutedEventArgs const& /*args*/)
    {
        
    }

    void MainWindow::MuteToggleButton_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        AudioViewer().MuteEndpoint();
    }

    void MainWindow::ViewHotKeysHyperlinkButton_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
    }

    void MainWindow::SplashScreen_Dismissed(winrt::Croak::SplashScreen const&, Foundation::IInspectable const&)
    {
        WindowSplashScreen().Visibility(Xaml::Visibility::Collapsed);
        AudioViewer().Visibility(Xaml::Visibility::Visible);
    }

    void MainWindow::ExpandFlyoutButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        MoreFlyoutGrid().Visibility(
            MoreFlyoutGrid().Visibility() == Xaml::Visibility::Visible ? Xaml::Visibility::Collapsed : Xaml::Visibility::Visible
        );
    }

    void MainWindow::PipToggleButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        bool alwaysOnTop = PipToggleButton().IsChecked().GetBoolean();
#if FALSE
        auto&& presenter = alwaysOnTop ? UI::OverlappedPresenter::CreateForContextMenu() : UI::OverlappedPresenter::CreateForToolWindow();
        appWindow.SetPresenter(presenter);
#else
        UI::OverlappedPresenter presenter = appWindow.Presenter().as<UI::OverlappedPresenter>();
        presenter.IsMaximizable(!presenter.IsMaximizable());
        presenter.IsMinimizable(!presenter.IsMinimizable());

        presenter.IsAlwaysOnTop(alwaysOnTop);
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"IsAlwaysOnTop", box_value(alwaysOnTop));

        RightPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(
            presenter.IsMinimizable() ? 135 : 45
        ));
#endif // DEBUG

    }

    void MainWindow::ReloadSessionsIconButton_Click(IconButton const&, Xaml::RoutedEventArgs const&)
    {
        AudioViewer().ReloadAudioSessions();
        SettingsButtonFlyout().Hide();
    }

    void MainWindow::RestartIconButton_Click(IconButton const&, Xaml::RoutedEventArgs const&)
    {
        if (Microsoft::Windows::AppLifecycle::AppInstance::Restart(secondWindow ? L"-secondWindow" : L"") != Storage::Core::AppRestartFailureReason::RestartPending)
        {
            WindowMessageBar().EnqueueString(resourceLoader.GetString(L"ErrorAppFailedRestart"));
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
                item.IsChecked(currentProfile && (key == currentProfile.ProfileName()));
                item.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
                item.Background(Xaml::SolidColorBrush(UI::Colors::Transparent()));
                item.BorderThickness(Xaml::Thickness(0));

                item.Click([this](const IInspectable& sender, Xaml::RoutedEventArgs)
                {
                    MoreFlyoutStackpanel().Visibility(Xaml::Visibility::Visible);
                    ProfilesGrid().Visibility(Xaml::Visibility::Collapsed);
                    SettingsButtonFlyout().Hide();
                    LoadProfileByName(sender.as<FrameworkElement>().Tag().as<hstring>());
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
        AudioViewer().ShowHiddenAudioSessions();
    }

    void MainWindow::OverlayModeToggleButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        // TODO: Enable overlay mode.
        if (OverlayModeToggleButton().IsChecked().GetBoolean())
        {
            UI::OverlappedPresenter presenter = UI::OverlappedPresenter::CreateForContextMenu();
            appWindow.SetPresenter(presenter);
            RightPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(0));  

            // Shrink window
            Graphics::RectInt32 rect{};
            Storage::ApplicationDataCompositeValue composite = unbox_value_or<Storage::ApplicationDataCompositeValue>(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"OverlayDisplay"), nullptr);
            if (composite)
            {
                rect.X = unbox_value_or(composite.Lookup(L"X"), appWindow.Position().X);
                rect.Y = unbox_value_or(composite.Lookup(L"Y"), appWindow.Position().Y);
                rect.Width = unbox_value_or(composite.Lookup(L"Width"), appWindow.Size().Width);
                rect.Height = unbox_value_or(composite.Lookup(L"Height"), appWindow.Size().Height);
                appWindow.MoveAndResize(rect);
            }

            WindowMessageBar().EnqueueString(resourceLoader.GetString(L"OverlayModeEnabled")); // I18N
        }
        else 
        {
            auto presenter = UI::OverlappedPresenter::CreateForToolWindow();
            bool pip = PipToggleButton().IsChecked().GetBoolean();
            presenter.IsMinimizable(!pip);
            presenter.IsMaximizable(!pip);
            appWindow.SetPresenter(presenter);

            RightPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(
                 pip ? 45 : 135
            ));
        }
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

                    if (presenter.State() == UI::OverlappedPresenterState::Restored)
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

            if (appWindow.TitleBar().IsCustomizationSupported())
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

            currentProfile = LoadStartupProfile();
            LoadSettings(currentProfile);
            SetBackground();
        }
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
        // UNDONE: Remove
    }

    void MainWindow::SetDragRectangles()
    {
        Graphics::RectInt32 dragRectangle{};

        dragRectangle.X = static_cast<int32_t>(LeftPaddingColumn().ActualWidth() + SettingsButtonColumn().ActualWidth());
#ifdef DEBUG
        dragRectangle.Y = 7;
#else
        dragRectangle.Y = 0;
#endif
        dragRectangle.Width = static_cast<int32_t>(TitleBarGrid().ActualWidth() - (LeftPaddingColumn().ActualWidth() + SettingsButtonColumn().ActualWidth()));
        dragRectangle.Height = static_cast<int32_t>(TitleBarGrid().ActualHeight());

        appWindow.TitleBar().SetDragRectangles(std::vector<Graphics::RectInt32>{ dragRectangle });
    }

    void MainWindow::LoadHotKeys()
    {
        using namespace ::Croak::System::HotKeys;
        HotKeyManager& hotKeyManager = HotKeyManager::GetHotKeyManager();
        hotKeyManager.HotKeyFired({ this, &MainWindow::HotKeyFired });

        try
        {
            hotKeyManager.RegisterHotKey(Windows::System::VirtualKeyModifiers::Menu, VK_SPACE, true, MOVE_WINDOW);
        }
        catch (const std::invalid_argument& ex)
        {
            DebugLog(std::format("Could not register MOVE_WINDOW.\n\t{0}", ex.what()));
            // TODO: I18N
            WindowMessageBar().EnqueueString(
                L"Alt + Space hot key not enabled"
            );
        }

        try
        {
            hotKeyManager.RegisterHotKey(Windows::System::VirtualKeyModifiers::Menu, 'A', true, MOVE_WINDOW_ALT);
        }
        catch (const std::invalid_argument& ex)
        {
            DebugLog(std::format("Could not register MOVE_WINDOW_ALT.\n\t{0}", ex.what()));
            WindowMessageBar().EnqueueString(
                L"Alt + A hot key not enabled"
            );
        }
    }

    void MainWindow::LoadSettings(const UserProfile& profile)
    {
        Graphics::RectInt32 rect = profile.WindowDisplayRect();
        UI::DisplayArea display = UI::DisplayArea::GetFromPoint(Graphics::PointInt32(rect.X, rect.Y), UI::DisplayAreaFallback::None);
        if (!display || rect.X < 0 || rect.Y < 0) //TODO: Check if rect.X < 0 and rect.Y < 0 are right when the current display is on the left of the main display.
        {
            display = UI::DisplayArea::GetFromPoint(Graphics::PointInt32(rect.X, rect.Y), UI::DisplayAreaFallback::Primary);
            // Set window middle of the screen.
            rect.X = (display.WorkArea().Width / 2) - (rect.Width / 2);
            rect.Y = (display.WorkArea().Height / 2) - (rect.Height / 2);
        }
        appWindow.MoveAndResize(rect);

        auto&& presenter = appWindow.Presenter().as<UI::OverlappedPresenter>();
        bool alwaysOnTop = profile.IsAlwaysOnTop();
        UI::OverlappedPresenterState presenterState = profile.PresenterState();
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

        PipToggleButton().IsChecked(alwaysOnTop);
        DisableAnimationsIconToggleButton().IsOn(profile.AudioProfile().DisableAnimations());
        ShowAppBarIconToggleButton().IsOn(profile.AudioProfile().ShowMenu());

#ifdef ENABLE_HOTKEYS
        // Activate hotkeys.
        if (profile.HotKeysEnabled())
        {
            LoadHotKeys();        
        }
#endif // ENABLE_HOTKEYS
    }

    void MainWindow::SaveSettings()
    {
        Collections::IPropertySet settings = Storage::ApplicationData::Current().LocalSettings().Values();

        settings.Insert(L"LastUsedUserProfile", box_value(currentProfile.ProfileName()));

        if (!currentProfile.Frozen())
        {
            // Update current profile
            auto presenter = appWindow.Presenter().as<UI::OverlappedPresenter>();
            currentProfile.PresenterState(presenter.State());
            currentProfile.IsAlwaysOnTop(appWindow.Presenter().as<UI::OverlappedPresenter>().IsAlwaysOnTop());
            currentProfile.WindowDisplayRect(displayRect);

            // If the user has a profile, the AudioProfile container has to exist, so no TryLookup and container creation.
            Storage::ApplicationDataContainer userProfiles = nullptr;
            if (!Storage::ApplicationData::Current().LocalSettings().Containers().HasKey(L"UserProfiles"))
            {
                userProfiles = Storage::ApplicationData::Current().LocalSettings().CreateContainer(L"UserProfiles", Storage::ApplicationDataCreateDisposition::Always);
            }
            else
            {
                userProfiles = Storage::ApplicationData::Current().LocalSettings().Containers().Lookup(L"UserProfiles");
            }
            Storage::ApplicationDataContainer profileContainer = userProfiles.CreateContainer(currentProfile.ProfileName(),
                Storage::ApplicationDataCreateDisposition::Always);
            currentProfile.Save(profileContainer);
        }
    }

    void MainWindow::LoadProfileByName(const hstring& profileName)
    {
        Storage::ApplicationDataContainer audioProfilesContainer = Storage::ApplicationData::Current().LocalSettings().Containers().TryLookup(L"UserProfiles");
        if (audioProfilesContainer)
        {
            for (auto&& profile : audioProfilesContainer.Containers())
            {
                try
                {
                    hstring name = profile.Key();
                    if (name == profileName)
                    {
                        UserProfile userProfile{};
                        userProfile.Restore(profile.Value()); // If the restore fails, the next instructions will not be ran.
                        currentProfile = std::move(userProfile);
                        // Cannot use audioProfile variable below here.

                        bool keepOnTop = currentProfile.IsAlwaysOnTop();

                        if (UI::OverlappedPresenter presenter = appWindow.Presenter().try_as<UI::OverlappedPresenter>())
                        {
                            presenter.IsAlwaysOnTop(keepOnTop);
                            presenter.IsMaximizable(!keepOnTop);
                            presenter.IsMinimizable(!keepOnTop);
                            PipToggleButton().IsChecked(keepOnTop);

                            // 45px  -> Only close button.
                            // 135px -> Minimize + maximize + close buttons.
                            RightPaddingColumn().Width(Xaml::GridLengthHelper::FromPixels(keepOnTop ? 45 : 135));
                        }

#ifdef ENABLE_HOTKEYS
                        if (currentProfile.HotKeysEnabled())
                        {
                            // Activate hotkeys.
                            LoadHotKeys();
                        }
#endif // ENABLE_HOTKEYS
                    }
                }
                catch (const hresult_error& error)
                {
                    // I18N: Failed to load profile [profile name]
                    WindowMessageBar().EnqueueString(L"Couldn't load profile " + profileName);
                    OutputDebugHString(error.message());
                }
            }
        }
    }

    winrt::Croak::UserProfile MainWindow::LoadStartupProfile()  
    {
        // If `profileName` is empty, create a default profile for the application to use until the user creates it's own or uses the default profile as his.
        winrt::hstring profileName = unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"LastUsedUserProfile"), L"");
        UserProfile profile = CreateDefaultProfile();
        if (!profileName.empty())
        {
            Storage::ApplicationDataContainer container = Storage::ApplicationData::Current().LocalSettings().Containers().TryLookup(L"UserProfiles");
            if (container)
            {
                Storage::ApplicationDataContainer profileContainer = container.Containers().TryLookup(profileName);
                if (profileContainer)
                {
                    try
                    {
                        profile.Restore(profileContainer); // If the restore fails, its going to throw.
                    }
                    catch (winrt::hresult_error)
                    {
                        // TODO: Handle error.
                        WindowMessageBar().EnqueueString(resourceLoader.GetString(L"LoadProfileFailed"));
                    }
                }
            }
        }
        return profile;
    }

    winrt::Croak::UserProfile MainWindow::CreateDefaultProfile()
    {
        try
        {
            UserProfile defaultProfile{};
            defaultProfile.Frozen(false);
            winrt::hstring defaultProfileName = resourceLoader.GetString(L"DefaultProfileName");
            defaultProfile.ProfileName(defaultProfileName);
            // Set default window position and size.
            Graphics::RectInt32 windowDisplayRect{};
            windowDisplayRect.Width = Xaml::Application::Current().Resources().Lookup(winrt::box_value(L"WindowWidth")).as<int32_t>();
            windowDisplayRect.Height = Xaml::Application::Current().Resources().Lookup(winrt::box_value(L"WindowHeight")).as<int32_t>();
            auto display = UI::DisplayArea::GetFromPoint(Graphics::PointInt32(0, 0), UI::DisplayAreaFallback::Primary);
            // Set window middle of the screen.
            windowDisplayRect.X = (display.WorkArea().Width / 2) - (windowDisplayRect.Width / 2);
            windowDisplayRect.Y = (display.WorkArea().Height / 2) - (windowDisplayRect.Height / 2);
            defaultProfile.WindowDisplayRect(windowDisplayRect);

            AudioProfile defaultAudioProfile{};
            defaultAudioProfile.ProfileName(defaultProfileName);
            defaultAudioProfile.IsDefaultProfile(true);

            defaultProfile.AudioProfile(defaultAudioProfile);

            return defaultProfile;
        }
        catch (const winrt::hresult_error& ex)
        {
            DebugBreak();
        }
        catch (const std::exception& ex)
        {
            DebugBreak();
        }
    }

    void MainWindow::AppWindow_Closing(UI::AppWindow, UI::AppWindowClosingEventArgs)
    {
        bool failed = false;
        try
        {
            SaveSettings();
        }
        catch (...)
        {
            failed = true;
            AudioViewer().CloseAudioObjects();
            throw;
        }
        
        if (!failed)
        {
            AudioViewer().CloseAudioObjects();
        }
    }

    void MainWindow::HotKeyFired(const uint32_t& id, const Foundation::IInspectable&)
    {
        constexpr float stepping = 0.01f;
        constexpr float largeStepping = 0.07f;

        if (id == MOVE_WINDOW || id == MOVE_WINDOW_ALT)
        {
            if (windowActivated)
            {
                if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"HideWindowInOverlayMode"), false))
                {
                    appWindow.Hide();
                }
                else
                {
                    appWindow.Presenter().as<UI::OverlappedPresenter>().Minimize();
                }
            }
            else
            {
                POINT p{};
                if (GetCursorPos(&p))
                {
                    UI::DisplayArea currentDisplay = UI::DisplayArea::GetFromPoint(Graphics::PointInt32(p.x, p.y), UI::DisplayAreaFallback::None);
                    auto windowPos = Graphics::PointInt32(displayRect.X, displayRect.Y);
                    UI::DisplayArea windowDisplay = UI::DisplayArea::GetFromPoint(windowPos, UI::DisplayAreaFallback::None);

                    Graphics::PointInt32 newPos
                    {
                        currentDisplay.WorkArea().X + (windowPos.X - windowDisplay.WorkArea().X),
                        currentDisplay.WorkArea().Y + (windowPos.Y - windowDisplay.WorkArea().Y)
                    };

                    if (UI::DisplayArea::GetFromPoint(newPos, UI::DisplayAreaFallback::None))
                    {
                        appWindow.Move(newPos);
                        appWindow.Show();

                        auto nativeWindow{ this->try_as<::IWindowNative>() };
                        if (nativeWindow)
                        {
                            HWND handle = nullptr;
                            nativeWindow->get_WindowHandle(&handle);
                            SwitchToThisWindow(handle, true);
                        }
                    }
                }
            }
        }
        else
        {
            DebugLog(std::format("Hot key not recognized. Id: {0}", id));
        }
    }
}
