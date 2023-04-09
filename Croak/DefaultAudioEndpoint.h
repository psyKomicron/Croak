#pragma once

#include "IComEventImplementation.h"
#include "AudioInterfacesSmartPtrs.h"
#include "AudioMeter.h"

namespace Croak::Audio
{
    class DefaultAudioEndpoint : public IComEventImplementation, public AudioMeter, private IAudioEndpointVolumeCallback
    {
    public:
        /**
         * @brief Default constructor.
         * @param devicePtr IMMDevice pointer.
         * @param eventContextId GUID used by this app to generate events.
        */
        DefaultAudioEndpoint(IMMDevice* devicePtr, GUID eventContextId);
        ~DefaultAudioEndpoint();

        /**
         * @brief Gets the volume of the session.
         * @param desiredVolume
         * @return True if the fonction succeeded
        */
        float Volume() const;
        /**
         * @brief Sets the volume of the session.
         * @return The volume of the session
        */
        void Volume(const float& volume);
        /**
         * @brief Gets the friendly name of the audio endpoint.
         * @return Friendly name of the audio endpoint
        */
        winrt::hstring Name() const;
        /**
         * @brief Gets the number of channels for the endpoint.
         * @return Number of channels
        */
        uint32_t Channels() const;
        /**
         * @brief Gets if the endpoint is muted.
         * @return True if the endpoint is muted
        */
        bool Muted() const;

        inline winrt::event_token VolumeChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, float> const& handler)
        {
            return e_volumeChanged.add(handler);
        }
        void VolumeChanged(winrt::event_token const& eventToken)
        {
            e_volumeChanged.remove(eventToken);
        }
        inline winrt::event_token StateChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, bool> const& handler)
        {
            return e_stateChanged.add(handler);
        }
        inline void StateChanged(const winrt::event_token& token)
        {
            e_stateChanged.remove(token);
        }

        IFACEMETHODIMP_(ULONG) AddRef();
        IFACEMETHODIMP_(ULONG) Release();
        IFACEMETHODIMP QueryInterface(REFIID riid, VOID** ppvInterface);

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
         * @brief Sets the audio endpoint muted or unmuted.
         * @param mute true to mute, false to unmute
        */
        void SetMute(const bool& mute);
        /**
         * @brief Sets the new volume for the audio endpoint without using the "ignore notifications" GUID.
         * @param newVolume new volume ∈ [0, 1]
        */
        void SetVolume(const float& newVolume);

    private:
        ::winrt::impl::atomic_ref_count refCount{ 1 };
        IAudioEndpointVolumePtr audioEndpointVolume{ nullptr };
        IMMDevicePtr device{ nullptr };
        LPWSTR deviceId = nullptr;
        GUID eventContextId;

        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, float>> e_volumeChanged {};
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, bool>> e_stateChanged {};

#pragma region IAudioEndpointVolumeCallback
        STDMETHODIMP OnNotify(__in PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
#pragma endregion
        void CheckIfAvailable();
    };
}
