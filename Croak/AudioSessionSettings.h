#pragma once

#include "AudioSessionSettings.g.h"

namespace winrt::Croak::implementation
{
    struct AudioSessionSettings : AudioSessionSettingsT<AudioSessionSettings>
    {
    public:
        AudioSessionSettings() = default;
        AudioSessionSettings(const winrt::hstring& name, const bool& muted, const float& audioLevel);

        winrt::hstring Name() const;
        void Name(const winrt::hstring& value);
        bool Muted() const;
        void Muted(const bool& value);
        float AudioLevel() const;
        void AudioLevel(const float& value);

        winrt::Windows::Storage::ApplicationDataCompositeValue Save();
        void Restore(const winrt::Windows::Storage::ApplicationDataCompositeValue& container);

    private:
        winrt::hstring name{};
        bool muted = false;
        float audioLevel = 0.0;
    };
}

namespace winrt::Croak::factory_implementation
{
    struct AudioSessionSettings : AudioSessionSettingsT<AudioSessionSettings, implementation::AudioSessionSettings>
    {
    };
}
