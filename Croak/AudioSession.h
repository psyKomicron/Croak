#pragma once

#include "IComEventImplementation.h"
#include "ProcessInfo.h"
#include "AudioInterfacesSmartPtrs.h"

namespace Croak::Audio
{
	class AudioSession : private IAudioSessionEvents, public IComEventImplementation
	{
    public:
        AudioSession(IAudioSessionControl2* audioSessionControl, GUID eventContextId);

        /**
         * @brief Grouping parameter for the audio session.
         * @return GUID
        */
        GUID GroupingParam();
        /**
         * @brief True if the audio session is the "System" audio session.
         * @return
        */
        bool IsSystemSoundSession();
        /**
         * @brief Unique ID for this session.
         * @return GUID
        */
        GUID Id();
        /**
         * @brief Checks if the audio session is muted or not.
         * @return True if the session is muted.
        */
        bool Muted();
        /**
         * @brief Checks if the audio session is muted or not.
         * @param isMuted
        */
        void Muted(const bool& isMuted);
        /**
         * @brief Gets the name of the session. Can be the display name or the name of the executable associated with the audio session's PID.
         * @return The name of the session
        */
        std::wstring Name();
        /**
         * @brief State of the session (active, inactive, expired).
         * @return AudioSessionState
        */
        AudioSessionState State() const;
        /**
         * @brief Sets the volume of the session.
         * @return The volume of the session
        */
        float Volume() const;
        /**
         * @brief Gets the volume of the session.
         * @param desiredVolume
        */
        void Volume(float const& desiredVolume);

        inline DWORD PID()
        {
            return processPID;
        }

        inline System::ProcessInfo* ProcessInfo()
        {
            return processInfo.get();
        }

        /**
         * @brief State changed event subscriber.
         * @param handler Event handler
         * @return Event token
        */
        winrt::event_token StateChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::guid, uint32_t> const& handler);
        void StateChanged(winrt::event_token const& token);
        /**
         * @brief Volume changed event subscriber.
         * @param handler Event handler
         * @return Event token
        */
        winrt::event_token VolumeChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::guid, float> const& handler);
        void VolumeChanged(winrt::event_token const& eventToken);

        /**
         * @brief Registers the audio session to system notifications.
         * @return true if successful
        */
        bool Register();
        /**
         * @brief Unregisters the audio session to system notifications.
         * @return true if successful
        */
        bool Unregister();
        /**
         * @brief Sets the session audio level to mute if state == true.
         * @param state
         * @return True if the audio session has been muted.
        */
        bool SetMute(bool const& mute);
        /**
         * @brief Sets the volume of the audio sessions, allowing for the VolumeChanged event to be fired.
         * @param volume Desired volume in absolute percentage (0-1)
        */
        void SetVolume(const float& volume);
        /**
         * @brief Gets the normalized peak PCM value for this audio session.
         * @return float between 0 and 1
        */
        float GetPeak() const;
        /**
         * @brief Gets the normalized peak PCM values for the channels in this audio session.
         * @return pair of float between 0 and 1
        */
        std::pair<float, float> GetChannelsPeak() const;

        // IUnknown
        IFACEMETHODIMP_(ULONG) AddRef();
        IFACEMETHODIMP_(ULONG) Release();
        IFACEMETHODIMP QueryInterface(REFIID riid, VOID** ppvInterface);

    private:
        IAudioSessionControl2Ptr audioSessionControl{ nullptr };
        IAudioMeterInformationPtr audioMeter{ nullptr };
        ISimpleAudioVolumePtr simpleAudioVolume{ nullptr };
        GUID eventContextId;
        GUID groupingParam;
        GUID id;
        bool isRegistered = false;
        bool isSystemSoundSession = false;
        bool muted;
        DWORD processPID = 0;
        ::winrt::impl::atomic_ref_count refCount{ 1 };
        std::wstring sessionName{};
        std::unique_ptr<System::ProcessInfo> processInfo{ nullptr };
        bool isSessionActive = false;

        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::guid, float>> e_volumeChanged {};
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::guid, uint32_t>> e_stateChanged {};

        // IAudioSessionEvents
        STDMETHOD(OnDisplayNameChanged)(LPCWSTR NewDisplayName, LPCGUID EventContext);
        STDMETHOD(OnIconPathChanged)(LPCWSTR NewIconPath, LPCGUID EventContext);
        STDMETHOD(OnSimpleVolumeChanged)(float NewVolume, BOOL NewMute, LPCGUID EventContext);
        STDMETHOD(OnStateChanged)(AudioSessionState NewState);
        STDMETHOD(OnSessionDisconnected)(AudioSessionDisconnectReason DisconnectReason);
        STDMETHOD(OnChannelVolumeChanged)(DWORD /*ChannelCount*/, float /*NewChannelVolumeArray*/[], DWORD /*ChangedChannel*/, LPCGUID /*EventContext*/)
        {
            return S_OK;
        };
        STDMETHOD(OnGroupingParamChanged)(LPCGUID /*NewGroupingParam*/, LPCGUID /*EventContext*/)
        {
            return S_OK;
        };
	};
}