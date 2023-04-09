#include "pch.h"
#include "AudioSession.h"

#include "AudioInterfacesSmartPtrs.h"
#include <appmodel.h>
#include "AudioSessionStates.h"
#include "ManifestApplicationNode.h"
#include "IconExtractor.h"
#include "DebugOutput.h"

using namespace winrt;
using namespace std;


namespace Croak::Audio
{
    AudioSession::AudioSession(IAudioSessionControl2* audioSessionControl, GUID eventContextId) :
        eventContextId{ eventContextId },
        sessionName{ L"" },
        audioSessionControl{ audioSessionControl }
    {
        check_bool(UuidCreate(&id) == 0);
        check_hresult(audioSessionControl->GetGroupingParam(&groupingParam));
        check_hresult(audioSessionControl->GetProcessId(&processPID));

        if (audioSessionControl->IsSystemSoundsSession() == S_OK)
        {
            winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
            sessionName = loader.GetString(L"SystemAudioSessionName");
            isSystemSoundSession = true;
        }
        else
        {
            LPWSTR wStr = nullptr;
            if (SUCCEEDED(audioSessionControl->GetDisplayName(&wStr)))
            {
                sessionName = to_hstring(wStr);
                // Free memory allocated by audioSessionControl->GetDisplayName()
                CoTaskMemFree(wStr);
            }
        }

        if (processPID > 0 && !isSystemSoundSession)
        {
            processInfo = unique_ptr<System::ProcessInfo>(new System::ProcessInfo(processPID));
            sessionName = !processInfo->Name().empty() ? std::wstring(processInfo->Name()) : processInfo->Manifest().DisplayName();
        }

        AudioSessionState state{};
        if (SUCCEEDED(audioSessionControl->GetState(&state)))
        {
            isActive = (state == AudioSessionState::AudioSessionStateActive);
        }

        check_hresult(audioSessionControl->QueryInterface(_uuidof(ISimpleAudioVolume), (void**)&simpleAudioVolume));
        if (FAILED(simpleAudioVolume->GetMute((BOOL*)&muted)))
        {
            // TODO: LOGS
            OutputDebugWString(std::format(L"Audio session '{}' > Failed to get session state. Default (unmuted) assumed.", sessionName));
        }

        if (FAILED(audioSessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&audioInformationMeter)))
        {
            // TODO: LOGS
            OutputDebugWString(std::format(L"Audio session '{}' > Failed to get audio meter info. Peak values will be blank.", sessionName));
        }
    }


#pragma region Properties
    GUID AudioSession::GroupingParam()
    {
        return groupingParam;
    }

    bool AudioSession::IsSystemSoundSession()
    {
        return isSystemSoundSession;
    }

    GUID AudioSession::Id()
    {
        return id;
    }

    bool AudioSession::Muted()
    {
        return muted;
    }

    void AudioSession::Muted(const bool& isMuted)
    {
        simpleAudioVolume->SetMute(isMuted, &eventContextId);
    }

    wstring AudioSession::Name()
    {
        return sessionName;
    }

    void AudioSession::Volume(float const& desiredVolume)
    {
        check_hresult(simpleAudioVolume->SetMasterVolume(desiredVolume, &eventContextId));
    }

    float AudioSession::Volume() const
    {
        float volume = 0.0f;
        check_hresult(simpleAudioVolume->GetMasterVolume(&volume));
        return volume;
    }

    AudioSessionState AudioSession::State() const
    {
        AudioSessionState state{};
        check_hresult(audioSessionControl->GetState(&state));
        return state;
    }
#pragma endregion


#pragma region Events
    winrt::event_token AudioSession::StateChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::guid, uint32_t> const& handler)
    {
        return e_stateChanged.add(handler);
    }

    void AudioSession::StateChanged(winrt::event_token const& token)
    {
        e_stateChanged.remove(token);
    }

    winrt::event_token AudioSession::VolumeChanged(winrt::Windows::Foundation::TypedEventHandler<winrt::guid, float> const& handler)
    {
        return e_volumeChanged.add(handler);
    }

    void AudioSession::VolumeChanged(winrt::event_token const& eventToken)
    {
        e_volumeChanged.remove(eventToken);
    }
#pragma endregion


    bool AudioSession::SetMute(bool const& state)
    {
        return SUCCEEDED(simpleAudioVolume->SetMute(state, nullptr));
    }

    void AudioSession::SetVolume(const float& volume)
    {
        check_hresult(simpleAudioVolume->SetMasterVolume(volume, nullptr));
    }

    bool AudioSession::Register()
    {
        if (!isRegistered)
        {
            isRegistered = SUCCEEDED(audioSessionControl->RegisterAudioSessionNotification(this));
        }
        return isRegistered;
    }

    bool AudioSession::Unregister()
    {
        if (isRegistered)
        {
            return SUCCEEDED(audioSessionControl->UnregisterAudioSessionNotification(this));
        }
        return false;
    }

#pragma region IUnknown
    IFACEMETHODIMP_(ULONG)AudioSession::AddRef()
    {
        return ++refCount;
    }

    IFACEMETHODIMP_(ULONG) AudioSession::Release()
    {
        const uint32_t remaining = --refCount;

        if (remaining == 0)
        {
            delete this;
        }

        return remaining;
    }

    IFACEMETHODIMP AudioSession::QueryInterface(REFIID riid, VOID** ppvInterface)
    {
        if (riid == IID_IUnknown)
        {
            *ppvInterface = static_cast<IUnknown*>(static_cast<IAudioSessionEvents*>(this));
            AddRef();
        }
        else if (riid == __uuidof(IAudioSessionEvents))
        {
            *ppvInterface = static_cast<IAudioSessionEvents*>(this);
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


    STDMETHODIMP AudioSession::OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID)
    {
        OutputDebugHString(sessionName + L" > Display name changed : " + to_hstring(NewDisplayName));
        e_stateChanged(id, static_cast<uint32_t>(AudioSessionStates::DisplayNameChanged));
        return S_OK;
    }

    STDMETHODIMP AudioSession::OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID)
    {
        OutputDebugHString(sessionName + L" > Icon path changed : " + to_hstring(NewIconPath));
        e_stateChanged(id, static_cast<uint32_t>(AudioSessionStates::IconChanged));
        return S_OK;
    }

    STDMETHODIMP AudioSession::OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext)
    {
        if (*EventContext != eventContextId)
        {
            muted = NewMute;
            e_volumeChanged(id, NewVolume);
            e_stateChanged(id, muted ? static_cast<uint32_t>(AudioSessionStates::Muted) : static_cast<uint32_t>(AudioSessionStates::Unmuted));
        }
        return S_OK;
    }

    STDMETHODIMP AudioSession::OnStateChanged(::AudioSessionState NewState)
    {
        isActive = NewState == AudioSessionState::AudioSessionStateActive;
        e_stateChanged(id, static_cast<uint32_t>(NewState)); // Cast to uint32_t to cross ABI without more code.
        return S_OK;
    }

    STDMETHODIMP AudioSession::OnSessionDisconnected(AudioSessionDisconnectReason /*DisconnectReason*/)
    {
        e_stateChanged(id, static_cast<uint32_t>(AudioSessionStates::Expired));
        return S_OK;
    }
}