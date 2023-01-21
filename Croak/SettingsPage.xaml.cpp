#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Navigation;
using namespace winrt::Windows::UI::Xaml::Interop;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Storage::Pickers;


namespace winrt::Croak::implementation
{
    SettingsPage::SettingsPage()
    {
        InitializeComponent();

        winrt::Windows::ApplicationModel::PackageId packageId = winrt::Windows::ApplicationModel::Package::Current().Id();
        AppVersionTextBlock().Text(
            to_hstring(packageId.Version().Major) + L"." + to_hstring(packageId.Version().Minor) + L"." + to_hstring(packageId.Version().Build)
        );
    }

    IAsyncAction SettingsPage::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        TransparencyEffectsToggleButton().IsOn(unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"TransparencyAllowed"), true));
        CustomTitleBarToggleButton().IsOn(unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"UseCustomTitleBar"), true));
        StartupTask startupTask = co_await StartupTask::GetAsync(L"CroakStartupTaskId");
        AddToStartupToggleSwitch().IsOn(startupTask.State() == StartupTaskState::Enabled);
        PowerEfficiencyToggleButton().IsOn(unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true));
        StartupProfileToggleSwitch().IsOn(unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"LoadLastProfile"), true));
    }

    void SettingsPage::OnNavigatedTo(NavigationEventArgs const& args)
    {
        if (args.NavigationMode() == NavigationMode::New)
        {
            winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
            SecondWindow::Current().Breadcrumbs().Append(
                NavigationBreadcrumbBarItem{ loader.GetString(L"SettingsPageDisplayName"), xaml_typename<winrt::Croak::SettingsPage>() }
            );
        }
    }

    void SettingsPage::AudioProfilesButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<AudioProfilesPage>());
    }

    void SettingsPage::HotKeysButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<HotkeysPage>());

        winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
        SecondWindow::Current().Breadcrumbs().Append(
            winrt::Croak::NavigationBreadcrumbBarItem{ loader.GetString(L"HotKeysPageDisplayName"), xaml_typename<winrt::Croak::HotkeysPage>() }
        );
    }

    void SettingsPage::NewContentButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<NewContentPage>());
    }

    void SettingsPage::TransparencyEffectsToggleButton_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"TransparencyAllowed", IReference(TransparencyEffectsToggleButton().IsOn()));
    }

    void SettingsPage::CustomTitleBarToggleButton_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"UseCustomTitleBar", IReference(CustomTitleBarToggleButton().IsOn()));
    }

    IAsyncAction SettingsPage::AddToStartupToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        AddToStartupProgressRing().Visibility(Visibility::Visible);
        StartupTask startupTask = co_await StartupTask::GetAsync(L"CroakStartupTaskId");
        if (AddToStartupToggleSwitch().IsOn())
        {
            // Add app to startup.
            if (startupTask.State() == StartupTaskState::Disabled)
            {
                StartupTaskState newState = co_await startupTask.RequestEnableAsync();
                if (newState == StartupTaskState::Enabled)
                {
                    UserMessageBar().EnqueueString(L"App added to startup");
                }
            }
        }
        else
        {
            // Remove app from startup.
            if (startupTask.State() == StartupTaskState::Enabled)
            {
                // Disable task
                startupTask.Disable();
                UserMessageBar().EnqueueString(L"App removed from startup");
            }
        }

        AddToStartupProgressRing().Visibility(Visibility::Collapsed);
    }

    IAsyncAction SettingsPage::BrowseButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        FileOpenPicker picker{};

        // HACK: https://github.com/microsoft/WindowsAppSDK/issues/1063
        HWND mainWindowHandle = GetWindowFromWindowId(SecondWindow::Current().Id());
        picker.as<IInitializeWithWindow>()->Initialize(mainWindowHandle);

        picker.FileTypeFilter().ReplaceAll({ L"*" });
        picker.SettingsIdentifier(L"Spotlight Importer");
        picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
        picker.ViewMode(PickerViewMode::List);

        StorageFile chosenFile = co_await picker.PickSingleFileAsync();
        if (chosenFile)
        {
            ApplicationData::Current().LocalSettings().Values().Insert(L"BackgroundImageUri", box_value(chosenFile.Path()));
        }
    }

    void SettingsPage::PowerEfficiencyToggleButton_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"PowerEfficiencyEnabled", box_value(PowerEfficiencyToggleButton().IsOn()));
    }

    void SettingsPage::StartupProfileToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"LoadLastProfile", box_value(StartupProfileToggleSwitch().IsOn()));
    }

    void SettingsPage::AudioSessionsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<AudioSessionsSettingsPage>());
    }
}
