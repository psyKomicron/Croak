#pragma once

#include "AudioSessionsSettingsPage.g.h"

namespace winrt::Croak::implementation
{
    struct AudioSessionsSettingsPage : AudioSessionsSettingsPageT<AudioSessionsSettingsPage>
    {
        AudioSessionsSettingsPage();

        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& args);

        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DeleteInactiveSessionsToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ShowInactiveAudioSessionsToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LimitNewSessionsLevelToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct AudioSessionsSettingsPage : AudioSessionsSettingsPageT<AudioSessionsSettingsPage, implementation::AudioSessionsSettingsPage>
    {
    };
}
