#pragma once

#include "IComEventImplementation.h"
#include "AudioInterfacesSmartPtrs.h"

namespace Croak::Audio
{
    class AudioMeter
    {
    public:
        AudioMeter() = default;

        /**
        * @brief Gets the normalized peak PCM values for the channels in this audio session.
        * @return pair of float between 0 and 1
        */
        std::pair<float, float> GetStereoPeaks();
        /**
        * @brief Gets the normalized peak PCM value for this audio session.
        * @return float between 0 and 1
        */
        float GetMonoPeaks();
        float GetMultiplesPeak();

    protected:
        IAudioMeterInformationPtr audioInformationMeter{ nullptr };
        bool isActive = false;
    };
}

