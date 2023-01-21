#pragma once

#include <stack>
#include "AudioSession.h"
#include "DefaultAudioEndpoint.h"
#include "AudioInterfacesSmartPtrs.h"
#include "IComEventImplementation.h"

namespace Croak::Audio
{
	class AudioController : public IComEventImplementation, private IAudioSessionNotification, private IMMNotificationClient
	{
    public:
        AudioController(GUID const& guid);

        inline winrt::event_token EndpointChanged(const winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::Windows::Foundation::IInspectable>& handler)
        {
            return e_endpointChanged.add(handler);
        }

        inline void EndpointChanged(const winrt::event_token& token)
        {
            return e_endpointChanged.remove(token);
        }

        winrt::event_token SessionAdded(const winrt::Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable>& handler)
        {
            return e_sessionAdded.add(handler);
        }

        void SessionAdded(const ::winrt::event_token& token)
        {
            e_sessionAdded.remove(token);
        }

        // IUnknown
        IFACEMETHODIMP_(ULONG) AddRef();
        IFACEMETHODIMP_(ULONG) Release();
        IFACEMETHODIMP QueryInterface(REFIID riid, VOID** ppvInterface);

        bool Register();
        bool Unregister();

        /**
         * @brief Enumerates current audio sessions.
         * @return Audio sessions currently active
        */
        std::vector<AudioSession*>* GetSessions();
        /**
         * @brief Newly created sessions. Call in loop to get all the new sessions.
         * @return
        */
        AudioSession* NewSession();
        /**
         * @brief
         * @return
        */
        Croak::Audio::DefaultAudioEndpoint* GetMainAudioEndpoint();

    private:
        ::winrt::impl::atomic_ref_count refCount{ 1 };
        GUID audioSessionID;
        IAudioSessionManager2Ptr audioSessionManager{ nullptr };
        IMMDeviceEnumeratorPtr deviceEnumerator{ nullptr };
        IAudioSessionEnumeratorPtr audioSessionEnumerator{ nullptr };
        std::stack<AudioSession*> newSessions{};
        bool isRegistered = false;
        std::wstring currentPwstrDefaultDeviceId{};

        winrt::event<winrt::Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable>> e_sessionAdded {};
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::Windows::Foundation::IInspectable>> e_endpointChanged {};

        STDMETHOD(OnSessionCreated)(IAudioSessionControl* NewSession);
#pragma region IMMNotificationClient
        STDMETHODIMP OnDeviceStateChanged(__in LPCWSTR /*pwstrDeviceId*/, __in DWORD /*dwNewState*/) noexcept;
        STDMETHODIMP OnDefaultDeviceChanged(__in EDataFlow flow, __in  ERole /*role*/, __in_opt LPCWSTR pwstrDefaultDeviceId) noexcept;
        // Not implemented.
        STDMETHOD(OnPropertyValueChanged)(__in LPCWSTR /*pwstrDeviceId*/, __in const PROPERTYKEY /*key*/) noexcept { return S_OK; };
        STDMETHOD(OnDeviceAdded)(__in LPCWSTR /*pwstrDeviceId*/) noexcept { return S_OK; };
        STDMETHOD(OnDeviceRemoved)(__in LPCWSTR /*pwstrDeviceId*/) noexcept { return S_OK; };
#pragma endregion
	};
}

