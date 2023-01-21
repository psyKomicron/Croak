#include "pch.h"
#include "DefaultAudioEndpoint.h"

#include <Functiondiscoverykeys_devpkey.h>
#include "AudioController.h"
#include "AudioInterfacesSmartPtrs.h"

using namespace winrt;


namespace Croak::Audio
{
	DefaultAudioEndpoint::DefaultAudioEndpoint(IMMDevice* pDevice, GUID eventContextId) :
		eventContextId{ eventContextId },
		device{ pDevice }
	{
		check_hresult(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&audioEndpointVolume));
		check_hresult(device->GetId(&deviceId));


		if (FAILED(device->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&audioMeterInfo)))
		{
			OutputDebugHString(L"Main audio endpoint failed to get audio meter information, peak values will be blank.");
		}
	}

	DefaultAudioEndpoint::~DefaultAudioEndpoint()
	{
		if (deviceId)
		{
			CoTaskMemFree(deviceId);
		}
	}


	winrt::hstring DefaultAudioEndpoint::Name() const
	{
		IPropertyStore* pProps = nullptr;
		if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &pProps)))
		{
			PROPVARIANT deviceFriendlyName{};
			PropVariantInit(&deviceFriendlyName);
			if (SUCCEEDED(pProps->GetValue(PKEY_Device_FriendlyName, &deviceFriendlyName)))
			{
				return hstring(deviceFriendlyName.pwszVal);
			}
		}
		return hstring();
	}

	void DefaultAudioEndpoint::Volume(const float& value)
	{
		if (value < 0.) return;

		winrt::check_hresult(audioEndpointVolume->SetMasterVolumeLevelScalar(value, &eventContextId));
	}

	float DefaultAudioEndpoint::Volume() const
	{
		float volumeLevel = 0.0;
		winrt::check_hresult(audioEndpointVolume->GetMasterVolumeLevelScalar(&volumeLevel));
		return volumeLevel;
	}

	uint32_t DefaultAudioEndpoint::Channels() const
	{
		uint32_t channelCount = 0;
		check_hresult(audioEndpointVolume->GetChannelCount(&channelCount));
		return channelCount;
	}

	bool DefaultAudioEndpoint::Muted() const
	{
		BOOL mute = false;
		check_hresult(audioEndpointVolume->GetMute(&mute));
		return mute & 1;
	}


	float DefaultAudioEndpoint::GetPeak() const
	{
		float peak = 0.f;
		audioMeterInfo->GetPeakValue(&peak);
		return peak;
	}

	std::pair<float, float> DefaultAudioEndpoint::GetPeaks()
	{
		float channelsPeakValues[2]{ 0 };
		// TODO: Make sure we are giving the correct amount of channels.
		check_hresult(audioMeterInfo->GetChannelsPeakValues(2, channelsPeakValues));
		return std::pair<float, float>(channelsPeakValues[0], channelsPeakValues[1]);
	}

	bool DefaultAudioEndpoint::Register()
	{
		return SUCCEEDED(audioEndpointVolume->RegisterControlChangeNotify(this));
	}

	bool DefaultAudioEndpoint::Unregister()
	{
		return SUCCEEDED(audioEndpointVolume->UnregisterControlChangeNotify(this));
	}

	void DefaultAudioEndpoint::SetMute(const bool& mute)
	{
		winrt::check_hresult(audioEndpointVolume->SetMute(mute, &eventContextId));
	}

	void DefaultAudioEndpoint::SetVolume(const float& newVolume)
	{
		check_hresult(audioEndpointVolume->SetMasterVolumeLevelScalar(newVolume, nullptr));
	}

#pragma region  IUnknown
	IFACEMETHODIMP_(ULONG) DefaultAudioEndpoint::AddRef()
	{
		return ++refCount;
	}

	IFACEMETHODIMP_(ULONG) DefaultAudioEndpoint::Release()
	{
		const uint32_t remaining = --refCount;

		if (remaining == 0)
		{
			delete(static_cast<void*>(this));
		}

		return remaining;
	}

	IFACEMETHODIMP DefaultAudioEndpoint::QueryInterface(REFIID riid, VOID** ppvInterface)
	{
		if (riid == IID_IUnknown)
		{
			*ppvInterface = static_cast<IUnknown*>(static_cast<IAudioEndpointVolumeCallback*>(this));
			AddRef();
		}
		else if (riid == __uuidof(IAudioEndpointVolumeCallback))
		{
			*ppvInterface = static_cast<IAudioEndpointVolumeCallback*>(this);
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


	STDMETHODIMP DefaultAudioEndpoint::OnNotify(__in PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
	{
		if (pNotify->guidEventContext != eventContextId)
		{
			// Handle notifications
			e_volumeChanged(winrt::Windows::Foundation::IInspectable(), pNotify->fMasterVolume);
			e_stateChanged(winrt::Windows::Foundation::IInspectable(), pNotify->bMuted & 1);
		}

		return S_OK;
	}

	void DefaultAudioEndpoint::CheckIfAvailable()
	{
		DWORD state = 0;
		if (SUCCEEDED(device->GetState(&state)) && state != DEVICE_STATE_ACTIVE)
		{
			/*switch (state)
			{
				case DEVICE_STATE_DISABLED:
					break;
				case DEVICE_STATE_NOTPRESENT:
					break;
				case DEVICE_STATE_UNPLUGGED:
					break;
			}*/
			throw std::out_of_range("Device is not active.");
		}
	}
}