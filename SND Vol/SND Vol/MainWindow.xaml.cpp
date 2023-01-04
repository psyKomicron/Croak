#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "HotKey.h"
#include "SecondWindow.xaml.h"

#define USE_TIMER 1
#define DEACTIVATE_TIMER 0
#define ENABLE_HOTKEYS 1
#define FORCE_SHOW_SPLASH_SCREEN 0

using namespace Audio;

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


namespace winrt::SND_Vol::implementation
{
    winrt::SND_Vol::MainWindow MainWindow::singleton{ nullptr };

    MainWindow::MainWindow()
    {
        singleton = *this;

        InitializeComponent();
        InitializeWindow();

    #ifdef DEBUG
        Application::Current().UnhandledException([&](IInspectable const&/*sender*/, UnhandledExceptionEventArgs const& e)
        {
            TextBlock block{};
            block.TextWrapping(TextWrapping::Wrap);
            block.Text(e.Message());
            WindowMessageBar().EnqueueMessage(e.Message());
        });
    #endif // DEBUG

    #if FORCE_SHOW_SPLASH_SCREEN & defined DEBUG
        ApplicationData::Current().LocalSettings().Values().Remove(L"ShowSplashScreen");
    #endif
        if (unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"ShowSplashScreen"), true))
        {
            ApplicationData::Current().LocalSettings().Values().Insert(L"ShowSplashScreen", IReference(false));

            winrt::Windows::ApplicationModel::PackageId packageId = winrt::Windows::ApplicationModel::Package::Current().Id();
            ApplicationVersionTextBlock().Text(
                to_hstring(packageId.Version().Major) + L"." + to_hstring(packageId.Version().Minor) + L"." + to_hstring(packageId.Version().Build)
            );
        }
        else
        {
            WindowSplashScreen().Visibility(Visibility::Collapsed);
            ContentGrid().Visibility(Visibility::Visible);
        }
    }

    MainWindow::~MainWindow()
    {
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

            if (audioSessions.get())
            {
                audioSessionsPeakTimer.Start();
            }
        }
        #endif // USE_TIMER

        // Generate size changed event to get correct clipping rectangle size
        SystemVolumeActivityBorder_SizeChanged(nullptr, nullptr);
        Grid_SizeChanged(nullptr, nullptr);

        // Teaching tips
        SettingsButtonTeachingTip().Target(SettingsButton());
        ApplicationDataContainer teachingTips = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TeachingTips");
        if (!teachingTips)
        {
            teachingTips = ApplicationData::Current().LocalSettings().CreateContainer(L"TeachingTips", ApplicationDataCreateDisposition::Always);
        }
        if (!teachingTips.Values().HasKey(L"ShowSettingsTeachingTip"))
        {
            teachingTips.Values().Insert(L"ShowSettingsTeachingTip", IReference(false));
            SettingsButtonTeachingTip().IsOpen(true);
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
            SystemVolumeSliderText().Visibility(Visibility::Collapsed);
        }
        else if (appWindow.Size().Width > 210 && compact)
        {
            compact = false;

            MuteToggleButtonColumn().Width(GridLengthHelper::Auto());
            SystemVolumeSliderTextColumn().Width(GridLengthHelper::FromPixels(26));

            MuteToggleButton().Visibility(Visibility::Visible);
            SystemVolumeSliderText().Visibility(Visibility::Visible);
        }
    }

    void MainWindow::AudioSessionView_VolumeChanged(AudioSessionView const& sender, RangeBaseValueChangedEventArgs const& args)
    {
        unique_lock lock{ audioSessionsMutex };

        for (size_t i = 0; i < audioSessions->size(); i++)
        {
            guid id = audioSessions->at(i)->Id();
            if (id == sender.Id())
            {
                audioSessions->at(i)->Volume(static_cast<float>(args.NewValue() / 100.0));
            }
        }
    }

    void MainWindow::AudioSessionView_VolumeStateChanged(winrt::SND_Vol::AudioSessionView const& sender, bool const& args)
    {
        unique_lock lock{ audioSessionsMutex };

        for (size_t i = 0; i < audioSessions->size(); i++)
        {
            guid id = audioSessions->at(i)->Id();
            if (id == sender.Id())
            {
                audioSessions->at(i)->SetMute(args);
            }
        }
    }

    void MainWindow::AudioSessionsPanel_Loading(FrameworkElement const&, IInspectable const&)
    {
        LoadContent();

        /*hstring profileName = unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"AudioProfile"), L"");
        if (!profileName.empty())
        {
            LoadProfile(profileName);
        }*/

#if ENABLE_HOTKEYS
        // Activate hotkeys.
        try
        {
            volumeUpHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueMessage(L"Failed to activate system volume up hot key");
        }

        try
        {
            volumeDownHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueMessage(L"Failed to activate system volume down hot key");
        }

        try
        {
            volumePageUpHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueMessage(L"Failed to activate system volume up (PageUp) hot key");
        }

        try
        {
            volumePageDownHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueMessage(L"Failed to activate system volume down (PageDown) hot key");
        }

        try
        {
            muteHotKeyPtr.Activate();
        }
        catch (const std::invalid_argument&)
        {
            WindowMessageBar().EnqueueMessage(L"Failed to activate mute/unmute hot key");
        }
#endif // ENABLE_HOTKEYS
    }

    void MainWindow::SystemVolumeSlider_ValueChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        if (mainAudioEndpoint)
        {
            mainAudioEndpoint->Volume(static_cast<float>(e.NewValue() / 100.));
        }

        SystemVolumeSliderText().Text(to_hstring(round(e.NewValue())));
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
        if (globalSessionAudioState == winrt::SND_Vol::AudioSessionState::Unmuted)
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
        else if (globalSessionAudioState == winrt::SND_Vol::AudioSessionState::Muted)
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
            for (AudioSession* session : *audioSessions)
            {
                session->Muted(setMute);
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

        // Setting DropDownButton.Content to mimic ComboBox selected item behavior.
        FontIcon icon{};
        icon.Glyph(L"\ue8c0");
        LayoutDropDownButton().Content(icon);
    }

    void MainWindow::VerticalViewMenuFlyoutItem_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewVerticalLayout")).as<Style>()
        );
        layout = 2;

        // Setting DropDownButton.Content to mimic ComboBox selected item behavior.
        FontIcon icon{};
        icon.Rotation(90);
        icon.Translation(::Numerics::float3(16, 0, 0));
        icon.Glyph(L"\ue8c0");
        LayoutDropDownButton().Content(icon);
    }

    void MainWindow::AutoViewMenuFlyoutItem_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        AudioSessionsScrollViewer().HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
        AudioSessionsScrollViewer().VerticalScrollBarVisibility(ScrollBarVisibility::Auto);

        AudioSessionsPanel().Style(
            RootGrid().Resources().Lookup(box_value(L"GridViewHorizontalLayout")).as<Style>()
        );
        layout = 0;

        // Setting DropDownButton.Content to mimic ComboBox selected item behavior.
        FontIcon icon{};
        icon.Glyph(L"\uf0e2");
        LayoutDropDownButton().Content(icon);
    }

    void MainWindow::SettingsIconButton_Click(IconButton const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
        if (!secondWindow)
        {
            secondWindow = make<SecondWindow>();
            secondWindow.Closed([this](auto, auto)
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
            WindowMessageBar().EnqueueMessage(loader.GetString(L"InfoHotKeysEnabled"));
        }
        else
        {
            WindowMessageBar().EnqueueMessage(loader.GetString(L"InfoHotKeysDisabled"));
        }
#endif // ENABLE_HOTKEYS
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
    }

    void MainWindow::MuteToggleButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());
        MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
    }

    void MainWindow::ViewHotKeysHyperlinkButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void MainWindow::SplashScreen_Dismissed(winrt::SND_Vol::SplashScreen const&, IInspectable const&)
    {
        WindowSplashScreen().Visibility(Visibility::Collapsed);
        ContentGrid().Visibility(Visibility::Visible);
    }
        
    void MainWindow::MenuFlyout_Opening(IInspectable const&, IInspectable const&)
    {
        
    }

    void MainWindow::ExpandFlyoutButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MoreFlyoutStackpanel().Visibility(MoreFlyoutStackpanel().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);
    }

    void MainWindow::KeepOnTopToggleButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        OverlappedPresenter presenter = appWindow.Presenter().as<OverlappedPresenter>();
        presenter.IsMaximizable(!presenter.IsMaximizable());
        presenter.IsMinimizable(!presenter.IsMinimizable());

        RightPaddingColumn().Width(GridLengthHelper::FromPixels(
            presenter.IsMinimizable() ? 135 : 45
        ));

        bool alwaysOnTop = KeepOnTopToggleButton().IsChecked().GetBoolean();
        appWindow.Presenter().as<OverlappedPresenter>().IsAlwaysOnTop(alwaysOnTop);
        ApplicationData::Current().LocalSettings().Values().Insert(L"IsAlwaysOnTop", box_value(alwaysOnTop));
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
            for (size_t i = 0; i < audioSessions->size(); i++)
            {
                audioSessions->at(i)->VolumeChanged(audioSessionVolumeChanged[audioSessions->at(i)->Id()]);
                audioSessions->at(i)->StateChanged(audioSessionsStateChanged[audioSessions->at(i)->Id()]);
                audioSessions->at(i)->Unregister();
                audioSessions->at(i)->Release();
            }
            audioSessions->clear();
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

            if (audioSessions.get())
            {
                audioSessionsPeakTimer.Start();
            }
        }

        ResourceLoader loader{};
        WindowMessageBar().EnqueueMessage(loader.GetString(L"InfoAudioSessionsReloaded"));
    }

    void MainWindow::RestartIconButton_Click(IconButton const&, RoutedEventArgs const&)
    {
        if (Microsoft::Windows::AppLifecycle::AppInstance::Restart(L"") != AppRestartFailureReason::RestartPending)
        {
            ResourceLoader loader{};
            WindowMessageBar().EnqueueMessage(loader.GetString(L"ErrorAppFailedRestart"));
        }
    }

    void MainWindow::ProfilesButton_Click(IconButton const&, RoutedEventArgs const&)
    {
        MoreFlyoutStackpanel().Visibility(MoreFlyoutStackpanel().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);
        ProfilesFlyoutGrid().Visibility(ProfilesFlyoutGrid().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);

        // Load profiles
        ProfilesFlyoutStackpanel().Children().Clear();
        ApplicationDataContainer audioProfilesContainer = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"AudioProfiles");
        if (audioProfilesContainer)
        {
            for (auto&& profile : audioProfilesContainer.Containers())
            {
                hstring key = profile.Key();
                Button item{};
                item.Content(box_value(key));
                item.Tag(box_value(key));
                item.Click([this](const IInspectable& sender, RoutedEventArgs)
                {
                    LoadProfile(sender.as<FrameworkElement>().Tag().as<hstring>());
                });

                ProfilesFlyoutStackpanel().Children().Append(item);
            }
        }
    }

    void MainWindow::SettingsButtonFlyout_Closed(IInspectable const&, IInspectable const&)
    {
        MoreFlyoutStackpanel().Visibility(Visibility::Visible);
        ProfilesFlyoutGrid().Visibility(Visibility::Collapsed);
    }

    void MainWindow::CloseProfilesButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MoreFlyoutStackpanel().Visibility(MoreFlyoutStackpanel().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);
        ProfilesFlyoutGrid().Visibility(ProfilesFlyoutGrid().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);
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
            //timestamp.Margin(Thickness(0, 0, 5, 0));

            Grid::SetRow(timestamp, WindowGrid().RowDefinitions().GetView().Size() - 1);
            RootGrid().Children().Append(timestamp);
        #endif // _DEBUG

            appWindow.Title(Application::Current().Resources().Lookup(box_value(L"AppTitle")).as<hstring>());

            appWindow.Closing({ this, &MainWindow::AppWindow_Closing });
            appWindow.Changed([this](auto&&, winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs args)
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
                appWindow.TitleBar().ButtonForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonInactiveBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveForegroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"AppTitleBarHoverColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonHoverBackgroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"ButtonHoverBackgroundColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonPressedBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
            }

            LoadSettings();
        }

        SetBackground();

        LoadHotKeys();
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
                    WindowMessageBar().EnqueueMessage(L"Failed to load background image.");
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
                audioController = new LegacyAudioController(appID);

                if (audioController->Register())
                {
                    audioController->SessionAdded({ this, &MainWindow::AudioController_SessionAdded });
                    audioController->EndpointChanged({ this, &MainWindow::AudioController_EndpointChanged });
                }
                else
                {
                    WindowMessageBar().EnqueueMessage(loader.GetString(L"ErrorAudioSessionsUnavailable"));
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

                audioSessions = unique_ptr<vector<AudioSession*>>(audioController->GetSessions());
                for (size_t i = 0; i < audioSessions->size(); i++)
                {
                    AudioSessionView view = CreateAudioView(audioSessions->at(i), true);
                    if (view)
                    {
                        audioSessionViews.Append(view);
                    }
                }


                // Create and setup peak meters timers
                audioSessionsPeakTimer = DispatcherQueue().CreateTimer();
                mainAudioEndpointPeakTimer = DispatcherQueue().CreateTimer();

                audioSessionsPeakTimer.Interval(TimeSpan(std::chrono::milliseconds(100)));
                audioSessionsPeakTimer.Tick({ this, &MainWindow::UpdatePeakMeters });

                mainAudioEndpointPeakTimer.Interval(TimeSpan(std::chrono::milliseconds(100)));
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
                WindowMessageBar().EnqueueMessage(loader.GetString(L"ErrorFatalFailure"));
            }
        }
        else
        {
            WindowMessageBar().EnqueueMessage(loader.GetString(L"ErrorFatalFailure"));
        }
    }

    AudioSessionView MainWindow::CreateAudioView(AudioSession* audioSession, bool enabled)
    {
        if (!audioSession->Register())
        {
            OutputDebugHString(L"Failed to register audio session '" + audioSession->Name() + L"'.");
            enabled = false;
        }

        // Check for duplicates, multiple audio sessions might be grouped under one by the app/system owning the sessions.
        //checkForDuplicates = false;
        if (audioSession->State() != ::AudioSessionState::AudioSessionStateActive)
        {
            unique_lock lock{ audioSessionsMutex };

            char hitcount = 0;
            for (size_t j = 0; j < audioSessions->size(); j++)
            {
                if (audioSessions->at(j)->GroupingParam() == audioSession->GroupingParam())
                {
                    if (hitcount > 1)
                    {
                        return nullptr;
                    }
                    hitcount++;
                }
            }
        }

        audioSessionVolumeChanged.insert({ audioSession->Id(), audioSession->VolumeChanged({ this, &MainWindow::AudioSession_VolumeChanged }) });
        audioSessionsStateChanged.insert({ audioSession->Id(), audioSession->StateChanged({ this, &MainWindow::AudioSession_StateChanged }) });

        AudioSessionView view = nullptr;
        if (audioSession->LogoPath().empty())
        {
            view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);
        }
        else
        {
            view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0, audioSession->LogoPath());
        }
        view.Id(guid(audioSession->Id()));
        view.Muted(audioSession->Muted());
        view.SetState((AudioSessionState)audioSession->State());
        view.IsEnabled(enabled);

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

        //if (currentAudioProfile)
        //{
        //    auto optional = currentAudioProfile.SessionsIndexes().TryLookup(audioSession->Name());
        //    if (optional.has_value())
        //    {
        //        if (optional.value() < audioSessionViews.Size())
        //        {
        //            audioSessionViews.InsertAt(optional.value(), view);
        //        }
        //        else
        //        {
        //            // I can either append at the end forever, or go through the hassle of checking everytime a sesion is added 
        //            // the sessions with indexes saved are in their right spot, including if the user has repositioned them
        //            // in-between 2 passes of this function. It could be functional but I don't think it would be that useful
        //            // for all the work done, and it could very well confuse the user with the sessions moving around for no
        //            // particular reason.
        //            audioSessionViews.Append(view);
        //        }
        //        return;
        //    }
        //}
        
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
            audioLevels.Values().Insert(audioSessionViews.GetAt(i).Header(), compositeValue);
        }
    }

    void MainWindow::LoadHotKeys()
    {
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
                mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() + .02f < 1.f ? mainAudioEndpoint->Volume() + 0.02f : 1.f);
            }
            catch (...)
            {
            }
        });

        volumeDownHotKeyPtr.Fired([this](auto, auto)
        {
            try
            {
                mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() - 0.02f > 0.f ? mainAudioEndpoint->Volume() - 0.02f : 0.f);
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
                DispatcherQueue().TryEnqueue([this]()
                {
                    MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
                MuteToggleButton().IsChecked(IReference(mainAudioEndpoint->Muted()));
                });
            }
            catch (...)
            {
            }
        });

        #pragma warning(pop)  
        #endif // DEBUG

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

        bool alwaysOnTop = unbox_value_or(settings.TryLookup(L"IsAlwaysOnTop"), true);
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
        switch (layout)
        {
            case 1:
                HorizontalViewMenuFlyoutItem_Click(nullptr, nullptr);
                break;
            case 2:
                VerticalViewMenuFlyoutItem_Click(nullptr, nullptr);
                break;
            /*case 0:
            default:
                break;*/
        }

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

                        float systemVolume = currentAudioProfile.SystemVolume();
                        auto audioLevels = currentAudioProfile.AudioLevels();
                        auto audioStates = currentAudioProfile.AudioStates();
                        bool disableAnimations = currentAudioProfile.DisableAnimations();
                        bool keepOnTop = currentAudioProfile.KeepOnTop();
                        bool showMenu = currentAudioProfile.ShowMenu();
                        uint32_t windowLayout = currentAudioProfile.Layout();

                        // Set system volume.
                        mainAudioEndpoint->SetVolume(systemVolume);

                        // Set audio sessions volume.
                        {
                            unique_lock lock{ audioSessionsMutex }; // Taking the lock will also lock sessions from being added to the display.

                            for (auto pair : audioLevels)
                            {
                                for (size_t i = 0; i < audioSessions->size(); i++)
                                {
                                    if (audioSessions->at(i)->Name() == pair.Key())
                                    {
                                        audioSessions->at(i)->Volume(pair.Value());
                                    }
                                }
                            }

                            for (auto pair : audioStates)
                            {
                                for (size_t i = 0; i < audioSessions->size(); i++)
                                {
                                    if (audioSessions->at(i)->Name() == pair.Key())
                                    {
                                        audioSessions->at(i)->Muted(pair.Value());
                                    }
                                }
                            }
                        }

                        if (disableAnimations)
                        {
                            DisableAnimationsIconButton_Click(nullptr, nullptr);
                        }

                        if (OverlappedPresenter presenter = appWindow.Presenter().try_as<OverlappedPresenter>())
                        {
                            presenter.IsAlwaysOnTop(keepOnTop);
                            presenter.IsMinimizable(!keepOnTop);
                            presenter.IsMinimizable(!keepOnTop);

                            RightPaddingColumn().Width(GridLengthHelper::FromPixels(!keepOnTop ? 135 : 45));
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
                        audioSessionViews.Clear();

                        // Remove duplicates.
                        vector<AudioSessionView> uniqueSessions{};
                        {
                            unique_lock lock{ audioSessionsMutex };

                            for (size_t i = 0; i < audioSessions->size(); i++)
                            {
                                lock.unlock();
                                auto&& view = CreateAudioView(audioSessions->at(i), true);
                                lock.lock();
                                if (view)
                                {
                                    uniqueSessions.push_back(view);
                                }
                            }
                        }

                        auto indexes = currentAudioProfile.SessionsIndexes();
                        auto&& views = vector<AudioSessionView>(indexes.Size(), nullptr);
                        for (size_t i = 0; i < uniqueSessions.size(); i++)
                        {
                            auto&& view = uniqueSessions[i];
                            auto opt = indexes.TryLookup(view.Header());
                            if (opt.has_value())
                            {
                                if (opt.value() < views.size())
                                {
                                    if (views[opt.value()] == nullptr) // The index might already be populated by the else if/else statements when the index is over the collection size.
                                    {
                                        views[opt.value()] = view;
                                    }
                                    else
                                    {
                                        views.push_back(views[opt.value()]);
                                        views[opt.value()] = view;
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

                        for (size_t i = 0; i < uniqueSessions.size(); i++)
                        {
                            if (uniqueSessions[i])
                            {
                                int j = views.size() - 1;
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
                                }
                            }
                        }
                        
                        audioSessionViews = multi_threaded_observable_vector<AudioSessionView>(move(views));
                        // HACK: Can we use INotifyPropertyChanged to raise that the vector has changed ?
                        AudioSessionsPanel().ItemsSource(audioSessionViews);

                        // I18N: Loaded profile [profile name]
                        WindowMessageBar().EnqueueMessage(L"Loaded profile " + profileName);
                    }
                }
                catch (const hresult_error& error)
                {
                    // I18N: Failed to load profile [profile name]
                    WindowMessageBar().EnqueueMessage(L"Couldn't load profile " + profileName);
                    OutputDebugHString(error.message());
                }
                catch (const std::out_of_range& error)
                {
                    OutputDebugHString(to_hstring(error.what()));
                }
            }
        }
    }

    void MainWindow::UpdatePeakMeters(winrt::Windows::Foundation::IInspectable, winrt::Windows::Foundation::IInspectable)
    {
        if (!loaded) return;

        // TODO: Try to see if I can update the code to make it faster. Multithreading is not an option since it's a single threaded appartement.
        for (size_t i = 0; i < audioSessions->size(); i++)
        {
            if (audioSessions->at(i))
            {
                guid id = audioSessions->at(i)->Id();

                for (auto const& view : audioSessionViews)
                {
                    if (view.Id() == id)
                    {
                        auto&& pair = audioSessions->at(i)->GetChannelsPeak();
                        view.SetPeak(pair.first, pair.second);
                    }
                }
            }
        }
    }

    void MainWindow::AppWindow_Closing(winrt::Microsoft::UI::Windowing::AppWindow, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs)
    {
        if (audioSessionsPeakTimer && audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }
        if (mainAudioEndpointPeakTimer && mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }

        VolumeStoryboard().Stop();

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

            audioController->Unregister();
            audioController->Release();
        }

        // Unregister VolumeChanged event handler & unregister audio sessions from audio events and release com ptrs.
        if (audioSessions.get())
        {
            unique_lock lock{ audioSessionsMutex };

            SaveAudioLevels();

            for (size_t i = 0; i < audioSessions->size(); i++)
            {
                audioSessions->at(i)->VolumeChanged(audioSessionVolumeChanged[audioSessions->at(i)->Id()]);
                audioSessions->at(i)->StateChanged(audioSessionsStateChanged[audioSessions->at(i)->Id()]);
                audioSessions->at(i)->Unregister();
                audioSessions->at(i)->Release();
            }
        }

        SaveSettings();
    }

    void MainWindow::MainAudioEndpoint_VolumeChanged(winrt::Windows::Foundation::IInspectable, const float& newVolume)
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

        if (audioState == AudioSessionState::Expired)
        {
            unique_lock lock{ audioSessionsMutex };

            audioSessionsPeakTimer.Stop();

            GUID sessionID = id;
            for (size_t i = 0; i < audioSessions->size(); i++)
            {
                if (audioSessions->at(i)->Id() == sessionID)
                {
                    AudioSession* session = audioSessions->at(i);
                    session->Unregister();
                    session->Release();
                    audioSessions->erase(audioSessions->begin() + i);
                    break;
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
                                    WindowMessageBar().EnqueueMessage(L"All sessions expired.");
                                }
                            }
                            break;
                    }

                    return;
                }
            }
        });
    }

    void MainWindow::AudioController_SessionAdded(winrt::Windows::Foundation::IInspectable, winrt::Windows::Foundation::IInspectable)
    {
        // TODO: Reorder audio sessions according to the currently loaded audio profile (if any).
        DispatcherQueue().TryEnqueue([this]()
        {
            audioSessionsPeakTimer.Stop();
            while (AudioSession* newSession = audioController->NewSession())
            {
                {
                    unique_lock lock{ audioSessionsMutex };
                    audioSessions->push_back(newSession);
                }
                AudioSessionView view = CreateAudioView(newSession, true);
                if (view)
                {
                    audioSessionViews.InsertAt(0, view);
                }
            }
            audioSessionsPeakTimer.Start();
        });
    }

    void MainWindow::AudioController_EndpointChanged(winrt::Windows::Foundation::IInspectable, winrt::Windows::Foundation::IInspectable)
    {
        mainAudioEndpoint->VolumeChanged(mainAudioEndpointVolumeChangedToken);
        mainAudioEndpoint->StateChanged(mainAudioEndpointStateChangedToken);
        mainAudioEndpoint->Unregister();
        mainAudioEndpoint->Release();

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

        DispatcherQueue().TryEnqueue([&]()
        {
            MainEndpointNameTextBlock().Text(mainAudioEndpoint->Name());
            SystemVolumeSlider().Value(static_cast<double>(mainAudioEndpoint->Volume()) * 100.);
            MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());

            unique_lock lock{ audioSessionsMutex };

            for (auto const& item : AudioSessionsPanel().Items())
            {
                AudioSessionView view = item.as<AudioSessionView>();

                for (size_t i = 0; i < audioSessions->size(); i++)
                {
                    guid id = audioSessions->at(i)->Id();

                    if (view.Id() == id)
                    {
                        audioSessions->at(i)->Volume(view.Volume() / 100.f);
                    }
                }
            }
        });
    }
}
