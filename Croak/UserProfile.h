#pragma once

#include "UserProfile.g.h"

namespace winrt::Croak::implementation
{
    struct UserProfile : UserProfileT<UserProfile>
    {
    public:
        UserProfile() = default;

        winrt::Croak::AudioProfile AudioProfile() const
        {
            return audioProfile;
        }
        void AudioProfile(const winrt::Croak::AudioProfile& value)
        {
            audioProfile = value;
        }

        bool Frozen() const
        {
            return frozen;
        }
        void Frozen(const bool& value)
        {
            frozen = value;
        }

        bool HotKeysEnabled() const
        {
            return hotKeysEnabled;
        }
        void HotKeysEnabled(const bool& value)
        {
            hotKeysEnabled = value;
        }

        bool IsAlwaysOnTop() const
        {
            return isAlwaysOnTop;
        }
        void IsAlwaysOnTop(const bool& value)
        {
            isAlwaysOnTop = value;
        }

        winrt::hstring ProfileName() const
        {
            return profileName;
        }
        void ProfileName(const winrt::hstring& value)
        {
            profileName = value;
        }

        winrt::Microsoft::UI::Windowing::OverlappedPresenterState PresenterState() const
        {
            return presenterState;
        }
        void PresenterState(const winrt::Microsoft::UI::Windowing::OverlappedPresenterState& value)
        {
            presenterState = value;
        }

        bool RestoreWindowState() const
        {
            return restoreWindowState;
        }
        void RestoreWindowState(const bool& value)
        {
            restoreWindowState = value;
        }

        winrt::Windows::Graphics::RectInt32 WindowDisplayRect() const
        {
            return windowDisplayRect;
        }
        void WindowDisplayRect(const winrt::Windows::Graphics::RectInt32& value)
        {
            windowDisplayRect = value;
        }

        void Restore(const winrt::Windows::Storage::ApplicationDataContainer& container);
        void Save(const winrt::Windows::Storage::ApplicationDataContainer& containter);

    private:
        winrt::Croak::AudioProfile audioProfile{};
        bool frozen = false;
        bool hotKeysEnabled = true;
        bool isAlwaysOnTop = false;
        bool restoreWindowState = false;
        winrt::Windows::Graphics::RectInt32 windowDisplayRect{};
        winrt::hstring profileName{};
        winrt::Microsoft::UI::Windowing::OverlappedPresenterState presenterState = winrt::Microsoft::UI::Windowing::OverlappedPresenterState::Restored;
    };
}

namespace winrt::Croak::factory_implementation
{
    struct UserProfile : UserProfileT<UserProfile, implementation::UserProfile>
    {
    };
}
