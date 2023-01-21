#pragma once

#include "AudioProfile.g.h"

namespace winrt::Croak::implementation
{
    struct AudioProfile : AudioProfileT<AudioProfile>
    {
        AudioProfile() = default;

        inline winrt::hstring ProfileName()
        {
            return profileName;
        }

        inline void ProfileName(const winrt::hstring& value)
        {
            profileName = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"ProfileName"));
        }

        inline winrt::Windows::Foundation::Collections::IMap<hstring, float> AudioLevels()
        {
            return audioLevels;
        }

        inline void AudioLevels(const winrt::Windows::Foundation::Collections::IMap<hstring, float>& value)
        {
            audioLevels = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"AudioLevels"));
        }

        inline winrt::Windows::Foundation::Collections::IMap<hstring, bool> AudioStates()
        {
            return audioStates;
        }

        inline void AudioStates(const winrt::Windows::Foundation::Collections::IMap<hstring, bool>& value)
        {
            audioStates = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"AudioStates"));
        }

        inline winrt::Windows::Foundation::Collections::IMap<hstring, uint32_t> SessionsIndexes() const
        {
            return sessionsIndexes;
        }

        inline void SessionsIndexes(const winrt::Windows::Foundation::Collections::IMap<hstring, uint32_t>& value)
        {
            sessionsIndexes = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"SessionsIndexes"));
        }

        inline bool IsDefaultProfile()
        {
            return isDefaultProfile;
        }

        inline void IsDefaultProfile(const bool& value)
        {
            isDefaultProfile = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsDefaultProfile"));
        }

        inline float SystemVolume() const
        {
            return systemVolume;
        }

        inline void SystemVolume(const float& value)
        {
            systemVolume = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"SystemVolume"));
        }

        inline bool DisableAnimations() const
        {
            return disableAnimations;
        }

        inline void DisableAnimations(const bool& value)
        {
            disableAnimations = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"DisableAnimations"));
        }

        inline bool KeepOnTop() const
        {
            return keepOnTop;
        }

        inline void KeepOnTop(const bool& value)
        {
            keepOnTop = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"KeepOnTop"));
        }

        inline bool ShowMenu() const
        {
            return showMenu;
        }

        inline void ShowMenu(const bool& value)
        {
            showMenu = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"ShowMenu"));
        }

        inline uint32_t Layout() const
        {
            return layout;
        }

        inline void Layout(const uint32_t& value)
        {
            layout = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"Layout"));
        }

        inline bool RestoreWindowState() const
        {
            return false;
        }

        void RestoreWindowState(const bool& value)
        {

        }

        inline winrt::Windows::Graphics::RectInt32 WindowDisplayRect() const
        {
            return {};
        }

        inline void WindowDisplayRect(const winrt::Windows::Graphics::RectInt32& value)
        {
        }


        inline winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return e_propertyChanged.add(handler);
        }

        inline void PropertyChanged(winrt::event_token const& token)
        {
            e_propertyChanged.remove(token);
        }


        void Save(const winrt::Windows::Storage::ApplicationDataContainer& container);
        void Restore(const winrt::Windows::Storage::ApplicationDataContainer& container);

    private:
        winrt::hstring profileName{};
        winrt::Windows::Foundation::Collections::IMap<hstring, float> audioLevels{ winrt::single_threaded_map<hstring, float>() };
        winrt::Windows::Foundation::Collections::IMap<hstring, bool> audioStates{ winrt::single_threaded_map<hstring, bool>() };
        winrt::Windows::Foundation::Collections::IMap<hstring, uint32_t> sessionsIndexes{ winrt::single_threaded_map<hstring, uint32_t>() };
        bool isDefaultProfile = false;
        float systemVolume = 0.0;
        bool disableAnimations = false;
        bool keepOnTop = false;
        bool showMenu = false;
        uint32_t layout = 0u;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;
    };
}

namespace winrt::Croak::factory_implementation
{
    struct AudioProfile : AudioProfileT<AudioProfile, implementation::AudioProfile>
    {
    };
}
