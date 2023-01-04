#pragma once
#include "MainWindow.g.h"

#include <vector>
#include <map>
#include "AudioSession.h"
#include "LegacyAudioController.h"
#include "MainAudioEndpoint.h"
#include "HotKey.h"

using namespace winrt::Windows::System;


namespace winrt::SND_Vol::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    public:
        MainWindow();
        ~MainWindow();

        static winrt::SND_Vol::MainWindow Current()
        {
            return singleton;
        }

        inline winrt::Windows::Foundation::Collections::IObservableVector<winrt::SND_Vol::AudioSessionView> AudioSessions() const
        {
            return audioSessionViews;
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


        void OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AudioSessionView_VolumeChanged(winrt::SND_Vol::AudioSessionView const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);
        void AudioSessionView_VolumeStateChanged(winrt::SND_Vol::AudioSessionView const& sender, bool const& args);
        void AudioSessionsPanel_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void Grid_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void SystemVolumeSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void Window_Activated(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs const& args);
        void SystemVolumeActivityBorder_SizeChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const&);
        void HideMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SwitchStateMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HorizontalViewMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void VerticalViewMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AutoViewMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void MuteToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ViewHotKeysHyperlinkButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SplashScreen_Dismissed(winrt::SND_Vol::SplashScreen const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void MenuFlyout_Opening(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void SettingsIconButton_Click(winrt::SND_Vol::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void DisableHotKeysIconButton_Click(winrt::SND_Vol::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ShowAppBarIconButton_Click(winrt::SND_Vol::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ExpandFlyoutButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DisableAnimationsIconButton_Click(winrt::SND_Vol::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void KeepOnTopToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ReloadSessionsIconButton_Click(winrt::SND_Vol::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void RestartIconButton_Click(winrt::SND_Vol::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ProfilesButton_Click(winrt::SND_Vol::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void SettingsButtonFlyout_Closed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void CloseProfilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        using BackdropController = winrt::Microsoft::UI::Composition::SystemBackdrops::DesktopAcrylicController;

        // Static fields
        static winrt::SND_Vol::MainWindow singleton;

        // Logic related attributes.
        std::mutex audioSessionsMutex{};
        Audio::MainAudioEndpoint* mainAudioEndpoint = nullptr;
        Audio::LegacyAudioController* audioController = nullptr;
        std::unique_ptr<std::vector<Audio::AudioSession*>> audioSessions{ nullptr };
        winrt::event_token mainAudioEndpointVolumeChangedToken;
        winrt::event_token mainAudioEndpointStateChangedToken;
        winrt::event_token audioControllerSessionAddedToken;
        winrt::event_token audioControllerEndpointChangedToken;
        std::map<winrt::guid, winrt::event_token> audioSessionVolumeChanged{};
        std::map<winrt::guid, winrt::event_token> audioSessionsStateChanged{};
        // Hot keys.
        System::HotKey volumeUpHotKeyPtr{ VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift, VK_UP };
        System::HotKey volumeDownHotKeyPtr{ VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift, VK_DOWN };
        System::HotKey volumePageUpHotKeyPtr{ VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift, VK_PRIOR };
        System::HotKey volumePageDownHotKeyPtr{ VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift, VK_NEXT };
        System::HotKey muteHotKeyPtr{ VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift, static_cast<uint32_t>('M') };
        // UI related attributes.
        bool loaded = false;
        bool compact = false;
        bool usingCustomTitleBar = false;
        uint16_t layout = 0;
        winrt::SND_Vol::AudioSessionState globalSessionAudioState = winrt::SND_Vol::AudioSessionState::Unmuted;
        winrt::Windows::Graphics::RectInt32 displayRect;
        winrt::Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        #pragma region Backdrop
        BackdropController backdropController = nullptr;
        winrt::Windows::System::DispatcherQueueController dispatcherQueueController = nullptr;
        winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration systemBackdropConfiguration = nullptr;
        winrt::Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker themeChangedRevoker;
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer audioSessionsPeakTimer = nullptr;
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer mainAudioEndpointPeakTimer = nullptr;
        #pragma endregion
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::SND_Vol::AudioSessionView> audioSessionViews
        {
            winrt::multi_threaded_observable_vector<winrt::SND_Vol::AudioSessionView>()
        };
        winrt::Microsoft::UI::Xaml::Window secondWindow{ nullptr };
        winrt::SND_Vol::AudioProfile currentAudioProfile{ nullptr };
        bool checkForDuplicates = true;

        void InitializeWindow();
        void SetBackground();
        void LoadContent();
        winrt::SND_Vol::AudioSessionView CreateAudioView(Audio::AudioSession* audioSession, bool enabled);
        void SetDragRectangles();
        void SaveAudioLevels();
        void LoadHotKeys();
        void LoadSettings();
        void SaveSettings();
        void LoadProfile(const hstring& profileName);

        void AppWindow_Closing(winrt::Microsoft::UI::Windowing::AppWindow, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs);
        void UpdatePeakMeters(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
        void MainAudioEndpoint_VolumeChanged(winrt::Windows::Foundation::IInspectable /*sender*/, const float& newVolume);
        void AudioSession_VolumeChanged(const winrt::guid& sender, const float& newVolume);
        void AudioSession_StateChanged(const winrt::guid& sender, const uint32_t& state);
        void AudioController_SessionAdded(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
        void AudioController_EndpointChanged(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);        
    };
}

namespace winrt::SND_Vol::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
