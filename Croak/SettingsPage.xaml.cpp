#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

namespace Foundation  = winrt::Windows::Foundation;
namespace Storage = winrt::Windows::Storage;
namespace ApplicationModel = winrt::Windows::ApplicationModel;
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Navigation;
    using namespace winrt::Windows::UI::Xaml::Interop;
}


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

    Foundation::IAsyncAction SettingsPage::Page_Loaded(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        ApplicationModel::StartupTask startupTask = co_await ApplicationModel::StartupTask::GetAsync(L"CroakStartupTaskId");
        AddToStartupToggleSwitch().IsOn(startupTask.State() == ApplicationModel::StartupTaskState::Enabled);

        TransparencyEffectsToggleButton().IsOn(unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"TransparencyAllowed"), true));
        CustomTitleBarToggleButton().IsOn(unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"UseCustomTitleBar"), true));
        PowerEfficiencyToggleButton().IsOn(unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true));
        StartupProfileToggleSwitch().IsOn(unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"LoadLastProfile"), true));
        HideWindowToggleSwitch().IsOn(unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"HideWindowOnCompactMode"), false));
    }

    void SettingsPage::OnNavigatedTo(Xaml::NavigationEventArgs const& args)
    {
        if (args.NavigationMode() == Xaml::NavigationMode::New)
        {
            winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
            SecondWindow::Current().Breadcrumbs().Append(
                NavigationBreadcrumbBarItem{ loader.GetString(L"SettingsPageDisplayName"), xaml_typename<winrt::Croak::SettingsPage>() }
            );
        }
    }

    void SettingsPage::AudioProfilesButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<AudioProfilesPage>());
    }

    void SettingsPage::HotKeysButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<HotkeysPage>());

        ApplicationModel::Resources::ResourceLoader loader{};
        SecondWindow::Current().Breadcrumbs().Append(
            winrt::Croak::NavigationBreadcrumbBarItem{ loader.GetString(L"HotKeysPageDisplayName"), xaml_typename<winrt::Croak::HotkeysPage>() }
        );
    }

    void SettingsPage::NewContentButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<NewContentPage>());
    }

    void SettingsPage::TransparencyEffectsToggleButton_Toggled(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"TransparencyAllowed", Foundation::IReference(TransparencyEffectsToggleButton().IsOn()));
    }

    void SettingsPage::CustomTitleBarToggleButton_Toggled(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"UseCustomTitleBar", Foundation::IReference(CustomTitleBarToggleButton().IsOn()));
    }

    Foundation::IAsyncAction SettingsPage::AddToStartupToggleSwitch_Toggled(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        AddToStartupProgressRing().Visibility(Xaml::Visibility::Visible);
        ApplicationModel::StartupTask startupTask = co_await ApplicationModel::StartupTask::GetAsync(L"CroakStartupTaskId");
        if (AddToStartupToggleSwitch().IsOn())
        {
            // Add app to startup.
            if (startupTask.State() == ApplicationModel::StartupTaskState::Disabled)
            {
                ApplicationModel::StartupTaskState newState = co_await startupTask.RequestEnableAsync();
                if (newState == ApplicationModel::StartupTaskState::Enabled)
                {
                    UserMessageBar().EnqueueString(L"App added to startup"); // I18N:
                }
            }
        }
        else
        {
            // Remove app from startup.
            if (startupTask.State() == ApplicationModel::StartupTaskState::Enabled)
            {
                // Disable task
                startupTask.Disable();
                UserMessageBar().EnqueueString(L"App removed from startup");
            }
        }

        AddToStartupProgressRing().Visibility(Xaml::Visibility::Collapsed);
    }

    Foundation::IAsyncAction SettingsPage::BrowseButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Storage::Pickers::FileOpenPicker picker{};

        // HACK: https://github.com/microsoft/WindowsAppSDK/issues/1063
        HWND mainWindowHandle = GetWindowFromWindowId(SecondWindow::Current().Id());
        picker.as<IInitializeWithWindow>()->Initialize(mainWindowHandle);

        picker.FileTypeFilter().ReplaceAll({ L"*" });
        picker.SettingsIdentifier(L"Spotlight Importer");
        picker.SuggestedStartLocation(Storage::Pickers::PickerLocationId::PicturesLibrary);
        picker.ViewMode(Storage::Pickers::PickerViewMode::List);

        Storage::StorageFile chosenFile = co_await picker.PickSingleFileAsync();
        if (chosenFile)
        {
            Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"BackgroundImageUri", box_value(chosenFile.Path()));
        }
    }

    void SettingsPage::PowerEfficiencyToggleButton_Toggled(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"PowerEfficiencyEnabled", box_value(PowerEfficiencyToggleButton().IsOn()));
    }

    void SettingsPage::StartupProfileToggleSwitch_Toggled(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"LoadLastProfile", box_value(StartupProfileToggleSwitch().IsOn()));
    }

    void SettingsPage::AudioSessionsButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<AudioSessionsSettingsPage>());
    }

    void SettingsPage::HideWindowToggleSwitch_Toggled(Foundation::IInspectable const&, Xaml::RoutedEventArgs const& e)
    {
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"HideWindowOnCompactMode", box_value(HideWindowToggleSwitch().IsOn()));
    }

    void SettingsPage::OverlayButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Frame().Navigate(xaml_typename<OverlaySettingsPage>());
    }
}


