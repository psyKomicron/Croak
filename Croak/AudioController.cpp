#include "pch.h"
#include "AudioController.h"

using namespace winrt;
using namespace std;

namespace Croak::Audio
{
    AudioController::AudioController(GUID const& guid) :
        audioSessionID(guid)
    {
        IMMDevicePtr pDevice;

        // Create the device enumerator.
        check_hresult(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&deviceEnumerator)));
        // Get the default audio device.
        check_hresult(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice));
        // Get the session manager.
        check_hresult(pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&audioSessionManager));
        // Get session enumerator
        check_hresult(audioSessionManager->GetSessionEnumerator(&audioSessionEnumerator));
    }


    bool AudioController::Register()
    {
        if (!isRegistered)
        {
            isRegistered =
                SUCCEEDED(audioSessionManager->RegisterSessionNotification(this)) &&
                SUCCEEDED(deviceEnumerator->RegisterEndpointNotificationCallback(this));
        }
        return isRegistered;
    }

    bool AudioController::Unregister()
    {
        return SUCCEEDED(audioSessionManager->UnregisterSessionNotification(this)) && SUCCEEDED(deviceEnumerator->UnregisterEndpointNotificationCallback(this));
    }

#pragma region IUnknown
    IFACEMETHODIMP_(ULONG) AudioController::AddRef()
    {
        return ++refCount;
    }

    IFACEMETHODIMP_(ULONG) AudioController::Release()
    {
        const uint32_t remaining = --refCount;

        if (remaining == 0)
        {
            delete this;
        }

        return remaining;
    }

    IFACEMETHODIMP AudioController::QueryInterface(REFIID riid, VOID** ppvInterface)
    {
        if (riid == IID_IUnknown)
        {
            *ppvInterface = static_cast<IUnknown*>(static_cast<IAudioSessionNotification*>(this));
            AddRef();
        }
        else if (riid == __uuidof(IAudioSessionNotification))
        {
            *ppvInterface = static_cast<IAudioSessionNotification*>(this);
            AddRef();
        }
        else if (riid == __uuidof(IMMNotificationClient))
        {
            *ppvInterface = static_cast<IMMNotificationClient*>(this);
            AddRef();
        }
        else
        {
            *ppvInterface = NULL;
            return E_POINTER;
        }
        return S_OK;
    }
#pragma endregion

    vector<AudioSession*>* AudioController::GetSessions()
    {
        vector<AudioSession*>* sessions = new vector<AudioSession*>();
        int sessionCount;
        if (SUCCEEDED(audioSessionEnumerator->GetCount(&sessionCount)))
        {
            for (int i = 0; i < sessionCount; i++)
            {
                IAudioSessionControlPtr control;
                IAudioSessionControl2* control2 = nullptr;

                if (SUCCEEDED(audioSessionEnumerator->GetSession(i, &control)) &&
                    SUCCEEDED(control->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&control2)))
                {
                    sessions->push_back(new AudioSession(control2, audioSessionID));
                }
            }
        }

        return sessions;
    }

    AudioSession* AudioController::NewSession()
    {
        if (newSessions.empty())
        {
            return nullptr;
        }
        else
        {
            AudioSession* ptr = newSessions.top();
            newSessions.pop();
            return ptr;
        }
    }

    DefaultAudioEndpoint* AudioController::GetMainAudioEndpoint()
    {
        IMMDevice* pDevice = nullptr;
        // Get the default audio device.
        check_hresult(deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eMultimedia, &pDevice));
        return new DefaultAudioEndpoint(pDevice, audioSessionID);
    }


    STDMETHODIMP AudioController::OnSessionCreated(IAudioSessionControl* NewSession)
    {
        // HACK: Audio session creation notifications are sent in double. Once we receive one, we will ignore the next. This can cause non doubled events to be ignored.
        static bool ignoreNotification = false;

        if (!ignoreNotification)
        {
            IAudioSessionControl2* control2 = nullptr;
            if (SUCCEEDED(NewSession->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&control2)))
            {
                // TODO: Insure thread safety.
                try
                {
                    // Windows sends OnSessionCreated event when disabling audio enhancements. The IAudioSessionControl received is not usable, making any calls to it's functions or properties fail.
                    auto newAudioSession = new AudioSession(control2, audioSessionID);
                    OutputDebugWString(L"New session created " + newAudioSession->Name());
                    newSessions.push(newAudioSession);
                    e_sessionAdded(nullptr, nullptr);
                }
                catch (const hresult_error& ex)
                {
                    OutputDebugWString(L"AudioController::OnSessionCreated failed to create audio session : " + ex.message());
                }
            }
        }
        else
        {
            OutputDebugString(L"Ignoring OnSessionCreated event.");
        }

        ignoreNotification = !ignoreNotification;
        return S_OK;
    }

    STDMETHODIMP AudioController::OnDefaultDeviceChanged(__in EDataFlow flow, __in ERole /*role*/, __in_opt LPCWSTR pwstrDefaultDeviceId) noexcept
    {
        static bool notified = false;

        assert(pwstrDefaultDeviceId != nullptr);

        wstring_view defaultDeviceId{ pwstrDefaultDeviceId };
        if (!notified && flow == EDataFlow::eRender &&
            (currentPwstrDefaultDeviceId.empty() || defaultDeviceId != currentPwstrDefaultDeviceId)
            )
        {
            OutputDebugHString(L"Default device changed (id: " + defaultDeviceId + L").");
            currentPwstrDefaultDeviceId = defaultDeviceId;
            e_endpointChanged(nullptr, nullptr);
        }
        else
        {
            OutputDebugHString(L"Ignored notification : Default device changed (id: " + defaultDeviceId + L").");
        }

        notified = !notified;
        return S_OK;
    }

    STDMETHODIMP AudioController::OnDeviceStateChanged(__in LPCWSTR /*pwstrDeviceId*/, __in DWORD /*dwNewState*/) noexcept
    {
        return S_OK;
    }
}