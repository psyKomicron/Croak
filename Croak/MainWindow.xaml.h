#pragma once

#include "MainWindow.g.h"
#include "AudioController.h"
#include "AudioSession.h"
#include "DefaultAudioEndpoint.h"

using namespace winrt::Windows::System;

namespace winrt::Croak::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    public:
        MainWindow(const winrt::hstring& args);

        static winrt::Croak::MainWindow Current()
        {
            return singleton;
        }

        inline winrt::Windows::Graphics::RectInt32 DisplayRect()
        {
            if (appWindow)
            {
                winrt::Windows::Graphics::PointInt32 position = appWindow.Position();
                winrt::Windows::Graphics::SizeInt32 size = appWindow.Size();
                return winrt::Windows::Graphics::RectInt32(position.X, position.Y, size.Width, size.Height);
            }
            else
            {
                return winrt::Windows::Graphics::RectInt32();
            }
        }

#pragma region Events
        void OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OnLoading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void Grid_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void Window_Activated(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs const& args);
        void HideMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SwitchStateMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HorizontalViewMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void VerticalViewMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AutoViewMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void MuteToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ViewHotKeysHyperlinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SplashScreen_Dismissed(winrt::Croak::SplashScreen const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void SettingsIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void DisableHotKeysIconButton_Click(winrt::Croak::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ShowAppBarIconButton_Click(winrt::Croak::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ExpandFlyoutButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DisableAnimationsIconButton_Click(winrt::Croak::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void PipToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ReloadSessionsIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void RestartIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void OpenProfilesIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void CloseProfilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OpenProfilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RootGrid_ActualThemeChanged(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void ShowHiddenSessionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OverlayModeToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
#pragma endregion

    private:
        using BackdropController = winrt::Microsoft::UI::Composition::SystemBackdrops::DesktopAcrylicController;

        // Static fields
        static winrt::Croak::MainWindow singleton;

        // Logic related attributes.
        // UI related attributes.
        bool loaded = false;
        bool compact = false;
        bool usingCustomTitleBar = false;
        winrt::Windows::Graphics::RectInt32 displayRect;
        winrt::Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
#pragma region Backdrop
        BackdropController backdropController = nullptr;
        winrt::Windows::System::DispatcherQueueController dispatcherQueueController = nullptr;
        winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration systemBackdropConfiguration = nullptr;
        winrt::Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker themeChangedRevoker;
#pragma endregion
        winrt::Croak::SecondWindow secondWindow{ nullptr };
        winrt::Croak::UserProfile currentProfile{ nullptr };
        bool hotKeysEnabled = true;
        bool windowActivated = false;
        winrt::Windows::ApplicationModel::Resources::ResourceLoader resourceLoader{};

        void InitializeWindow();
        void SetBackground();
        void LoadContent();
        void SetDragRectangles();
        void LoadHotKeys();
        void LoadSettings(const winrt::Croak::UserProfile& profile);
        void SaveSettings();
        void LoadProfileByName(const hstring& profileName);
        winrt::Croak::UserProfile LoadStartupProfile();
        winrt::Croak::UserProfile CreateDefaultProfile();

        void HotKeyFired(const uint32_t& id, const Windows::Foundation::IInspectable& /*args*/);
        void AppWindow_Closing(winrt::Microsoft::UI::Windowing::AppWindow, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
