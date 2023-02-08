#include "pch.h"
#include "AudioProfilesPage.xaml.h"
#if __has_include("AudioProfilesPage.g.cpp")
#include "AudioProfilesPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;


namespace winrt::Croak::implementation
{
    AudioProfilesPage::AudioProfilesPage()
    {
        InitializeComponent();

        ProfilesListView().ItemsSource(audioProfiles);
    }


    void AudioProfilesPage::Page_Loading(FrameworkElement const&, IInspectable const&)
    {
        AllowChangesToLoadedProfileToggleSwitch().IsOn(
            unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"AllowChangesToLoadedProfile"), false)
        );
    }

    void AudioProfilesPage::OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& args)
    {
        int32_t size = SecondWindow::Current().Breadcrumbs().Size();
        int32_t index = size - 1;
        if (args.NavigationMode() != ::Navigation::NavigationMode::Back &&
            (index < size ||SecondWindow::Current().Breadcrumbs().GetAt(index).ItemTypeName().Name != xaml_typename<winrt::Croak::AudioProfilesPage>().Name)
            )
        {
            winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
            SecondWindow::Current().Breadcrumbs().Append(
                NavigationBreadcrumbBarItem{ loader.GetString(L"AudioProfilesPageDisplayName"), xaml_typename<winrt::Croak::AudioProfilesPage>() }
            );
        }
    }

    void AudioProfilesPage::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        // Load profiles into the page
        ApplicationDataContainer audioProfilesContainer = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"AudioProfiles");
        if (!audioProfilesContainer)
        {
            audioProfilesContainer = ApplicationData::Current().LocalSettings().CreateContainer(L"AudioProfiles", ApplicationDataCreateDisposition::Always);
        }
        else
        {
            for (auto&& profile : audioProfilesContainer.Containers())
            {
                try
                {
                    hstring key = profile.Key();
                    AudioProfile audioProfile{};
                    audioProfile.Restore(profile.Value());
                    audioProfiles.Append(audioProfile);
                }
                catch (const hresult_error&)
                {
                }
            }
        }
    }

    void AudioProfilesPage::AddProfileButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // I18N: Audio profile.
        hstring profileName = L"Audio profile";
        uint32_t count = 1u;
        for (uint32_t j = 0; j < audioProfiles.Size(); j++)
        {
            hstring test = audioProfiles.GetAt(j).ProfileName();
            if (test == profileName)
            {
                profileName = L"Audio profile (" + to_hstring(count++) + L")";
            }
        }

        Frame().Navigate(xaml_typename<AudioProfileEditPage>(), box_value(profileName), ::Media::Animation::DrillInNavigationTransitionInfo());
    }

    void AudioProfilesPage::DeleteProfileButton_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = sender.as<Button>().Tag().as<hstring>();
        for (uint32_t i = 0; i < audioProfiles.Size(); i++)
        {
            if (audioProfiles.GetAt(i).ProfileName() == tag)
            {
                ApplicationDataContainer audioProfilesContainer = ApplicationData::Current().LocalSettings().Containers().Lookup(L"AudioProfiles");
                audioProfilesContainer.DeleteContainer(audioProfiles.GetAt(i).ProfileName());

                audioProfiles.RemoveAt(i);
                break;
            }
        }
    }

    IAsyncAction AudioProfilesPage::EditProfileButton_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = sender.as<Button>().Tag().as<hstring>();
        for (uint32_t i = 0; i < audioProfiles.Size(); i++)
        {
            if (audioProfiles.GetAt(i).ProfileName() == tag)
            {
                AudioProfile editedProfile = audioProfiles.GetAt(i);
                Frame().Navigate(xaml_typename<AudioProfileEditPage>(), editedProfile, ::Media::Animation::DrillInNavigationTransitionInfo());
                break;
            }
        }

        co_return;
    }

    void AudioProfilesPage::DuplicateProfileButton_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring tag = sender.as<Button>().Tag().as<hstring>();
        for (uint32_t i = 0; i < audioProfiles.Size(); i++)
        {
            if (audioProfiles.GetAt(i).ProfileName() == tag)
            {
                // TODO: Copy the profile values to the new one.

                AudioProfile profile{};
                hstring profileName = audioProfiles.GetAt(i).ProfileName();

                uint32_t count = 1u;
                for (uint32_t j = 0; j < audioProfiles.Size(); j++)
                {
                    hstring test = audioProfiles.GetAt(j).ProfileName();
                    if (test == profileName)
                    {
                        profileName = audioProfiles.GetAt(i).ProfileName() + L" (" + to_hstring(count++) + L")";
                    }
                }

                profile.ProfileName(profileName);
                profile.IsDefaultProfile(false);

                audioProfiles.Append(profile);

                break;
            }
        }
    }

    void AudioProfilesPage::AllowChangesToLoadedProfileToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(
            L"AllowChangesToLoadedProfile",
            box_value(AllowChangesToLoadedProfileToggleSwitch().IsOn())
        );
    }
}
