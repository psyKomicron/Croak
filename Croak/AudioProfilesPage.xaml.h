#pragma once

#include "AudioProfilesPage.g.h"

namespace winrt::Croak::implementation
{
    struct AudioProfilesPage : AudioProfilesPageT<AudioProfilesPage>
    {
        AudioProfilesPage();

        inline winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::AudioProfile> AudioProfiles()
        {
            return audioProfiles;
        };

        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& args);
        void Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AddProfileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DeleteProfileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction EditProfileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DuplicateProfileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AllowChangesToLoadedProfileToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::AudioProfile> audioProfiles
        {
            single_threaded_observable_vector<winrt::Croak::AudioProfile>()
        };
        winrt::Windows::Foundation::Collections::IVectorView<winrt::Croak::AudioSessionView> audioSessions;
    };
}

namespace winrt::Croak::factory_implementation
{
    struct AudioProfilesPage : AudioProfilesPageT<AudioProfilesPage, implementation::AudioProfilesPage>
    {
    };
}
