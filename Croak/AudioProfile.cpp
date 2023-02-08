#include "pch.h"
#include "AudioProfile.h"
#if __has_include("AudioProfile.g.cpp")
#include "AudioProfile.g.cpp"
#endif

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;


namespace winrt::Croak::implementation
{
    void AudioProfile::Save(const ApplicationDataContainer& container)
    {
        ApplicationDataContainer props = container.Containers().TryLookup(profileName);
        if (!props)
        {
            props = container.CreateContainer(profileName, ApplicationDataCreateDisposition::Always);
        }

        props.Values().Insert(L"IsDefaultProfile", IReference(isDefaultProfile));
        props.Values().Insert(L"DisableAnimations", IReference(disableAnimations));
        props.Values().Insert(L"KeepOnTop", IReference(keepOnTop));
        props.Values().Insert(L"ShowMenu", IReference(showMenu));
        props.Values().Insert(L"SystemVolume", IReference(systemVolume));
        props.Values().Insert(L"Layout", IReference(layout));

        ApplicationDataContainer audioSettingsContainer = props.CreateContainer(L"AudioSessionsSettings", ApplicationDataCreateDisposition::Always);
        for (auto&& audioSessionSettings : audioSessionsSettings)
        {
            audioSettingsContainer.Values().Insert(audioSessionSettings.Name(), audioSessionSettings.Save());
        }

        ApplicationDataCompositeValue indexes{};
        for (auto&& pair : sessionsIndexes)
        {
            hstring name = pair.Key();
            uint32_t index = pair.Value();
            indexes.Insert(name, IReference(index));
        }
        props.Values().Insert(L"SessionsIndexes", indexes);
    }

    void AudioProfile::Restore(const ApplicationDataContainer& container)
    {
        profileName = container.Name();
        isDefaultProfile = unbox_value<bool>(container.Values().Lookup(L"IsDefaultProfile"));
        disableAnimations = unbox_value<bool>(container.Values().Lookup(L"DisableAnimations"));
        keepOnTop = unbox_value<bool>(container.Values().Lookup(L"KeepOnTop"));
        showMenu = unbox_value<bool>(container.Values().Lookup(L"ShowMenu"));
        systemVolume = unbox_value<float>(container.Values().Lookup(L"SystemVolume"));
        layout = unbox_value<uint32_t>(container.Values().Lookup(L"Layout"));

        auto audioSessionsSettings = container.Containers().Lookup(L"AudioSessionsSettings").Values();
        auto sessionsIndexesContainer = container.Values().Lookup(L"SessionsIndexes").as<ApplicationDataCompositeValue>();

        for (auto&& pair : audioSessionsSettings)
        {
            hstring name = pair.Key();
            winrt::Croak::AudioSessionSettings audioSessionSettings{};
            audioSessionSettings.Name(name);
            audioSessionSettings.Restore(unbox_value<ApplicationDataCompositeValue>(pair.Value()));
        }

        for (auto&& pair : sessionsIndexesContainer)
        {
            hstring name = pair.Key();
            uint32_t index = pair.Value().as<uint32_t>();
            sessionsIndexes.Insert(name, index);
        }
    }
}
