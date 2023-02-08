#include "pch.h"
#include "AudioSessionSettings.h"
#if __has_include("AudioSessionSettings.g.cpp")
#include "AudioSessionSettings.g.cpp"
#endif

namespace winrt::Croak::implementation
{
    AudioSessionSettings::AudioSessionSettings(const winrt::hstring& name, const bool& muted, const float& audioLevel) :
        name{ name }, muted{ muted }, audioLevel{ audioLevel }
    {
    }

    winrt::hstring AudioSessionSettings::Name() const
    {
        return name;
    }

    void AudioSessionSettings::Name(const winrt::hstring& value)
    {
        name = value;
    }

    bool AudioSessionSettings::Muted() const
    {
        return false;
    }

    void AudioSessionSettings::Muted(const bool& value)
    {
        muted = value;
    }

    float AudioSessionSettings::AudioLevel() const
    {
        return audioLevel;
    }

    void AudioSessionSettings::AudioLevel(const float& value)
    {
        audioLevel = value;
    }

    winrt::Windows::Storage::ApplicationDataCompositeValue AudioSessionSettings::Save()
    {
        winrt::Windows::Storage::ApplicationDataCompositeValue storage = winrt::Windows::Storage::ApplicationDataCompositeValue();
        storage.Insert(L"Muted", box_value(muted));
        storage.Insert(L"AudioLevel", box_value(audioLevel));
        return storage;
    }

    void AudioSessionSettings::Restore(const winrt::Windows::Storage::ApplicationDataCompositeValue& container)
    {
        muted = unbox_value_or(container.Lookup(L"Muted"), false);
        audioLevel = unbox_value_or(container.Lookup(L"AudioLevel"), 0.0);
    }
}
