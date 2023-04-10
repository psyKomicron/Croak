#pragma once
#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "AudioViewer.g.h"

#include "AudioController.h"
#include "AudioSession.h"
#include "DefaultAudioEndpoint.h"
#include "AudioSessionStates.h"
#include "HotkeyManager.h"

namespace winrt::Croak::implementation
{
    struct AudioViewer : AudioViewerT<AudioViewer>
    {
    public:
        AudioViewer();

        // Properties
        inline winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::AudioSessionView> AudioSessions() const
        {
            return audioSessionViews;
        }
        inline winrt::hstring DefaultAudioEndpointName();

        // Events
        inline winrt::event_token PropertyChanged(const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& value);
        inline void PropertyChanged(const winrt::event_token& token);

        // Methods
        inline void SetMessageBar(const winrt::Croak::MessageBar& messageBar);
        void CloseAudioObjects();
        void MuteEndpoint();
        void LoadAudioProfile(const winrt::Croak::AudioProfile& audioProfile);
        void PauseAnimations();
        void SetAudioSessionLayout(const winrt::Croak::AudioSessionLayout& layout);
        void StopAnimations();
        void SwitchAudioState();
        void ReloadAudioSessions();
        void ShowHiddenAudioSessions();
        //void LoadColorProfile();

        // Event handlers
        void OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void MuteToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SystemVolumeActivityBorder_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void SystemVolumeSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void AudioSessionsPanel_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void Grid_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void AudioSessionView_VolumeChanged(winrt::Croak::AudioSessionView const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args);
        void AudioSessionView_VolumeStateChanged(winrt::Croak::AudioSessionView const& sender, bool const& args);
        void UserControl_Unloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AudioSessionsGridView_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e);
        void AudioSessionsGridView_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);

    private:
        bool animationsDisabled = false;
        winrt::Croak::AudioSessionLayout layout = winrt::Croak::AudioSessionLayout::Auto;
        std::timed_mutex audioSessionsMutex{};
        std::mutex mainAudioEndpointMutex{};
        std::timed_mutex selectedIndicesMutex{};
        std::vector<winrt::hstring> selectedAudioSessionsIndices{};
        ::Croak::Audio::DefaultAudioEndpoint* mainAudioEndpoint = nullptr;
        ::Croak::Audio::AudioController* audioController = nullptr;
        std::map<winrt::guid, ::Croak::Audio::AudioSession*> audioSessions{};
        winrt::Croak::AudioProfile currentAudioProfile = nullptr;
        ::Croak::System::HotKeys::HotKeyManager hotKeyManager{};
        // UI.
        std::atomic_bool atomicLoaded{};
        winrt::hstring defaultAudioEndpointName = L"";
        winrt::Croak::MessageBar messageBar = nullptr;
        winrt::event_token mainAudioEndpointVolumeChangedToken;
        winrt::event_token mainAudioEndpointStateChangedToken;
        winrt::event_token audioControllerSessionAddedToken;
        winrt::event_token audioControllerEndpointChangedToken;
        std::map<winrt::guid, winrt::event_token> audioSessionVolumeChanged{};
        std::map<winrt::guid, winrt::event_token> audioSessionsStateChanged{};
        winrt::Croak::AudioSessionState globalSessionAudioState = winrt::Croak::AudioSessionState::Unmuted;
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer audioSessionsPeakTimer = nullptr;
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer mainAudioEndpointPeakTimer = nullptr;
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::AudioSessionView> audioSessionViews
        {
            winrt::multi_threaded_observable_vector<winrt::Croak::AudioSessionView>()
        };
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;

        winrt::Croak::AudioSessionView CreateRegisterView(::Croak::Audio::AudioSession* audioSession);
        winrt::Croak::AudioSessionView CreateView(::Croak::Audio::AudioSession* audioSession, bool skipDuplicates);
        void LoadContent();
        void SaveAudioLevels();
        void LoadHotKeys();
        void LoadSettings();
        void SaveSettings();
        void CleanUpResources(const bool& forReload);
        void InsertSessionAccordingToProfile(::Croak::Audio::AudioSession* audioSession);
        void DrawSizeIndicators();
        void HotKeyFired(const uint32_t& id, const Windows::Foundation::IInspectable& /*args*/);
        void UpdatePeakMeters(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
        void MainAudioEndpoint_VolumeChanged(winrt::Windows::Foundation::IInspectable /*sender*/, const float& newVolume);
        void AudioSession_VolumeChanged(const winrt::guid& sender, const float& newVolume);
        void AudioSession_StateChanged(const winrt::guid& sender, const uint32_t& state);
        void AudioController_SessionAdded(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
        void AudioController_EndpointChanged(winrt::Windows::Foundation::IInspectable /*sender*/, winrt::Windows::Foundation::IInspectable /*args*/);
        void DefaultAudioEndpointName(const winrt::hstring& value);
        void EnqueueString(const winrt::hstring& str);
        void EnqueueMessage(const winrt::Windows::Foundation::IInspectable& inspectable);
    public:
        void AudioSessionsGridView_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct AudioViewer : AudioViewerT<AudioViewer, implementation::AudioViewer>
    {
    };
}
