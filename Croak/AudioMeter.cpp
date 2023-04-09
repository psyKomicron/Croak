#include "pch.h"
#include "AudioMeter.h"

namespace Croak::Audio
{
    std::pair<float, float> AudioMeter::GetStereoPeaks()
    {
        if (isActive)
        {
            std::pair<float, float> peaks{};
            UINT meteringChannelCount = 0;
            if (SUCCEEDED(audioInformationMeter->GetMeteringChannelCount(&meteringChannelCount)) && meteringChannelCount >= 2)
            {
                std::unique_ptr<float> channelsPeak{ new float[meteringChannelCount]{ 0.0 } };
                winrt::check_hresult(audioInformationMeter->GetChannelsPeakValues(meteringChannelCount, channelsPeak.get()));
                peaks.first = channelsPeak.get()[0];
                peaks.second = channelsPeak.get()[1];
            }
            return peaks;
        }
        else
        {
            return { 0.f, 0.f };
        }
    }

    float AudioMeter::GetMonoPeaks()
    {
        float peak = 0.f;
        audioInformationMeter->GetPeakValue(&peak);
        return peak;
    }

    float AudioMeter::GetMultiplesPeak()
    {
        return 0.0f;
    }
}
