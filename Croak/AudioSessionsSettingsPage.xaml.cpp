#include "pch.h"
#include "AudioSessionsSettingsPage.xaml.h"
#if __has_include("AudioSessionsSettingsPage.g.cpp")
#include "AudioSessionsSettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Navigation;
using namespace winrt::Windows::UI::Xaml::Interop;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::ApplicationModel;


namespace winrt::Croak::implementation
{
    AudioSessionsSettingsPage::AudioSessionsSettingsPage()
    {
        InitializeComponent();
    }

    void AudioSessionsSettingsPage::OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& args)
    {
        if (args.NavigationMode() == NavigationMode::New)
        {
            SecondWindow::Current().Breadcrumbs().Append(NavigationBreadcrumbBarItem(L"Audio sessions", xaml_typename<Croak::AudioSessionsSettingsPage>()));
        }
    }

    void AudioSessionsSettingsPage::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        ShowInactiveAudioSessionsToggleSwitch().IsOn(unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"ShowInactiveSessionsOnStartup"), true));
    }

    void AudioSessionsSettingsPage::DeleteInactiveSessionsToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        // TODO: Limit new sessions volume level.
        ApplicationData::Current().LocalSettings().Values().Insert(L"LimitNewSessionsVolumeLevel", box_value(LimitNewSessionsLevelToggleSwitch().IsOn()));
    }

    void AudioSessionsSettingsPage::ShowInactiveAudioSessionsToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"ShowInactiveSessionsOnStartup", box_value(ShowInactiveAudioSessionsToggleSwitch().IsOn()));
    }

    void AudioSessionsSettingsPage::LimitNewSessionsLevelToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"NewSessionsVolumeLimit", box_value(MaxVolumeSlider().Value()));
    }
}
