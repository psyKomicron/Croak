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

        inline winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::AudioSessionView> AudioSessions() const
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
        void AudioSessionView_VolumeChanged(winrt::Croak::AudioSessionView const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);
        void AudioSessionView_VolumeStateChanged(winrt::Croak::AudioSessionView const& sender, bool const& args);
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
        void SplashScreen_Dismissed(winrt::Croak::SplashScreen const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void SettingsIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void DisableHotKeysIconButton_Click(winrt::Croak::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ShowAppBarIconButton_Click(winrt::Croak::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ExpandFlyoutButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DisableAnimationsIconButton_Click(winrt::Croak::IconToggleButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void KeepOnTopToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ReloadSessionsIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void RestartIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void OpenProfilesIconButton_Click(winrt::Croak::IconButton const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void CloseProfilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OpenProfilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RootGrid_ActualThemeChanged(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void NewContentButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ShowHiddenSessionsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        using BackdropController = winrt::Microsoft::UI::Composition::SystemBackdrops::DesktopAcrylicController;

        // Static fields
        static winrt::Croak::MainWindow singleton;

        // Logic related attributes.
        std::mutex audioSessionsMutex{};
        std::mutex mainAudioEndpointMutex{};
        ::Croak::Audio::DefaultAudioEndpoint* mainAudioEndpoint = nullptr;
        ::Croak::Audio::AudioController* audioController = nullptr;
        std::map<winrt::guid, ::Croak::Audio::AudioSession*> audioSessions{};
        winrt::event_token mainAudioEndpointVolumeChangedToken;
        winrt::event_token mainAudioEndpointStateChangedToken;
        winrt::event_token audioControllerSessionAddedToken;
        winrt::event_token audioControllerEndpointChangedToken;
        std::map<winrt::guid, winrt::event_token> audioSessionVolumeChanged{};
        std::map<winrt::guid, winrt::event_token> audioSessionsStateChanged{};
        // UI related attributes.
        bool loaded = false;
        bool compact = false;
        bool usingCustomTitleBar = false;
        uint16_t layout = 0;
        winrt::Croak::AudioSessionState globalSessionAudioState = winrt::Croak::AudioSessionState::Unmuted;
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
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::AudioSessionView> audioSessionViews
        {
            winrt::multi_threaded_observable_vector<winrt::Croak::AudioSessionView>()
        };
        winrt::Croak::SecondWindow secondWindow{ nullptr };
        winrt::Croak::AudioProfile currentAudioProfile{ nullptr };

        void InitializeWindow();
        void SetBackground();
        void LoadContent();
        winrt::Croak::AudioSessionView CreateAudioView(::Croak::Audio::AudioSession* audioSession);
        winrt::Croak::AudioSessionView CreateAudioSessionView(::Croak::Audio::AudioSession* audioSession, bool skipDuplicates);
        void SetDragRectangles();
        void SaveAudioLevels();
        void LoadHotKeys();
        void LoadSettings();
        void SaveSettings();
        void LoadProfile(const hstring& profileName);
        void ReloadAudioSessions();
        std::map<winrt::guid, ::Croak::Audio::AudioSession*> MapAudioSessions(std::vector<::Croak::Audio::AudioSession*>* vect);
        void CleanUpResources(const bool& forReload);
        void InsertSessionAccordingToProfile(::Croak::Audio::AudioSession* audioSession);
        void SetSizeIndicators();

        void HotKeyFired(const uint32_t& id, const Windows::Foundation::IInspectable& /*args*/);
        void AppWindow_Closing(winrt::Microsoft::UI::Windowing::AppWindow, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs);
        void UpdatePeakMeters(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
        void MainAudioEndpoint_VolumeChanged(winrt::Windows::Foundation::IInspectable /*sender*/, const float& newVolume);
        void AudioSession_VolumeChanged(const winrt::guid& sender, const float& newVolume);
        void AudioSession_StateChanged(const winrt::guid& sender, const uint32_t& state);
        void AudioController_SessionAdded(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
        void AudioController_EndpointChanged(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
