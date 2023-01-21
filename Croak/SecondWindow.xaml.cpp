#include "pch.h"
#include "SecondWindow.xaml.h"
#if __has_include("SecondWindow.g.cpp")
#include "SecondWindow.g.cpp"
#endif

#include <winrt/Windows.UI.Core.h>
#include "HotKeyViewModel.h"

using namespace winrt;
using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Composition;
using namespace winrt::Microsoft::UI::Composition::SystemBackdrops;
using namespace winrt::Microsoft::UI::Input;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Navigation;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::ApplicationModel::Resources;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;


namespace winrt::Croak::implementation
{
    winrt::Croak::SecondWindow SecondWindow::singleton{ nullptr };

    SecondWindow::SecondWindow()
    {
        singleton = *this;

        InitializeComponent();
        InitializeWindow();
        TitleTextBlock().Text(appWindow.Title());
    }


    bool SecondWindow::NavigateTo(const winrt::Windows::UI::Xaml::Interop::TypeName& typeName)
    {
        return NavigationFrame().Navigate(typeName);
    }

    void SecondWindow::Grid_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        auto currentSourceTypePage = NavigationFrame().CurrentSourcePageType();
        if (currentSourceTypePage.Kind == winrt::Windows::UI::Xaml::Interop::TypeKind::Primitive)
        {
            NavigationFrame().Navigate(xaml_typename<SettingsPage>(), box_value(L"SettingsPage"));
        }
    }

    void SecondWindow::NavigationFrame_Navigated(IInspectable const&, NavigationEventArgs const& e)
    {
    }

    void SecondWindow::NavigationBreadcrumbBar_ItemClicked(BreadcrumbBar const&, BreadcrumbBarItemClickedEventArgs const& e)
    {
        winrt::Croak::NavigationBreadcrumbBarItem item = breadcrumbs.GetAt(e.Index());

        for (int i = breadcrumbs.Size() - 1; i >= e.Index(); i--)
        {
            breadcrumbs.RemoveAt(i);
        }

        NavigationFrame().Navigate(item.ItemTypeName());
    }

    void SecondWindow::NavigationFrame_NavigationFailed(IInspectable const&, NavigationFailedEventArgs const& e)
    {
        try
        {
            auto exception = e.Exception();
            e.Handled(true);
            ErrorMessageBar().EnqueueString(L"Navigation failed");
        }
        catch (const hresult_error& err)
        {
            ErrorMessageBar().EnqueueString(L"Navigation failed");
        }
    }

    void SecondWindow::RootGrid_ActualThemeChanged(FrameworkElement const&, IInspectable const&)
    {
        // Change title bar buttons background to fit the new theme.
        if (usingCustomTitleBar)
        {
            if (RootGrid().ActualTheme() == ElementTheme::Light)
            {
                appWindow.TitleBar().ButtonForegroundColor(Colors::Black());
                appWindow.TitleBar().ButtonHoverForegroundColor(Colors::Black());
                appWindow.TitleBar().ButtonPressedForegroundColor(Colors::Black());
            }
            else if (RootGrid().ActualTheme() == ElementTheme::Dark)
            {
                appWindow.TitleBar().ButtonForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
            }
        }

        if (systemBackdropConfiguration)
        {
            systemBackdropConfiguration.Theme((SystemBackdropTheme)RootGrid().ActualTheme());
            backdropController.TintColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
            backdropController.FallbackColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
        }
    }

    void SecondWindow::NavigationBackButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (breadcrumbs.Size() > 0)
        {
            breadcrumbs.RemoveAtEnd();
        }

        if (NavigationFrame().CanGoBack())
        {
            NavigationFrame().GoBack();
        }
        else if (NavigationFrame().BackStackDepth() == 0)
        {
            NavigateTo(xaml_typename<SettingsPage>());
            NavigationFrame().BackStack().Clear();
        }
    }


    void SecondWindow::InitializeWindow()
    {
        auto nativeWindow{ this->try_as<::IWindowNative>() };
        check_bool(nativeWindow);

        HWND handle = nullptr;
        nativeWindow->get_WindowHandle(&handle);
        WindowId windowID = GetWindowIdFromWindow(handle);
        appWindow = AppWindow::GetFromWindowId(windowID);
        if (appWindow != nullptr)
        {
            appWindow.MoveInZOrderAtTop();

            ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().CreateContainer(
                L"SettingsWindow",
                ApplicationData::Current().LocalSettings().Containers().HasKey(L"SettingsWindow") ? ApplicationDataCreateDisposition::Existing : ApplicationDataCreateDisposition::Always
            );

            int32_t width = unbox_value_or(
                settings.Values().TryLookup(L"WindowWidth"),
                Application::Current().Resources().Lookup(box_value(L"SecondWindowWidth")).as<int32_t>()
            );
            int32_t height = unbox_value_or(
                settings.Values().TryLookup(L"WindowHeight"),
                Application::Current().Resources().Lookup(box_value(L"SecondWindowHeight")).as<int32_t>()
            );
            PointInt32 lastPosition{};
            lastPosition.X = unbox_value_or(settings.Values().TryLookup(L"WindowPosX"), -1);
            lastPosition.Y = unbox_value_or(settings.Values().TryLookup(L"WindowPosY"), -1);

            if (lastPosition.X > 0 && lastPosition.Y > 0)
            {
                // Check if the current window position is out of bounds
                DisplayArea display = DisplayArea::GetFromPoint(lastPosition, DisplayAreaFallback::None);
                if (!display)
                {
                    display = DisplayArea::GetFromPoint(PointInt32(0), DisplayAreaFallback::Primary);
                    lastPosition.X = (display.WorkArea().Width - width) / 2;
                    lastPosition.Y = (display.WorkArea().Height - height) / 2;
                }
            }
            else // Center window on current screen (where the main window is)
            {
                winrt::Windows::Graphics::RectInt32 mainWindowDisplayRect = MainWindow::Current().DisplayRect();
                DisplayArea display = DisplayArea::GetFromPoint(PointInt32(mainWindowDisplayRect.X, mainWindowDisplayRect.Y), DisplayAreaFallback::None);

                // There should always be a display, but in the case that for some reason the main window is not inside screen bounds.
                if (!display)
                {
                    display = DisplayArea::GetFromPoint(PointInt32(0), DisplayAreaFallback::Primary);
                }

                lastPosition.X = (display.WorkArea().Width - width) / 2;
                lastPosition.Y = (display.WorkArea().Height - height) / 2;
            }

            appWindow.MoveAndResize(RectInt32(lastPosition.X, lastPosition.Y, width, height));
            appWindow.Title(ResourceLoader().GetString(L"SecondWindowTitle"));
            appWindowClosingEventToken = appWindow.Closing({ this, &SecondWindow::AppWindow_Closing });

            if (appWindow.TitleBar().IsCustomizationSupported())
            {
                usingCustomTitleBar = true;
                appWindow.TitleBar().ExtendsContentIntoTitleBar(true);
                appWindow.TitleBar().IconShowOptions(IconShowOptions::HideIconAndSystemMenu);

                LeftPaddingColumn().Width(GridLengthHelper::FromPixels(static_cast<double>(appWindow.TitleBar().LeftInset())));

                appWindow.TitleBar().ButtonBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveForegroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"AppTitleBarHoverColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonHoverBackgroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"ButtonHoverBackgroundColor")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonPressedBackgroundColor(Colors::Transparent());

                if (RootGrid().ActualTheme() == ElementTheme::Light)
                {
                    appWindow.TitleBar().ButtonForegroundColor(Colors::Black());
                    appWindow.TitleBar().ButtonHoverForegroundColor(Colors::Black());
                    appWindow.TitleBar().ButtonPressedForegroundColor(Colors::Black());
                }
                else if (RootGrid().ActualTheme() == ElementTheme::Dark)
                {
                    appWindow.TitleBar().ButtonForegroundColor(Colors::White());
                    appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
                    appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
                }
            }
        }

        SetBackground();
    }

    void SecondWindow::SetBackground()
    {
        if (DesktopAcrylicController::IsSupported())
        {
            auto&& supportsBackdrop = try_as<ICompositionSupportsSystemBackdrop>();
            if (supportsBackdrop)
            {
                RootGrid().Background(SolidColorBrush(Colors::Transparent()));
                if (!DispatcherQueue::GetForCurrentThread() && !dispatcherQueueController)
                {
                    DispatcherQueueOptions options
                    {
                        sizeof(DispatcherQueueOptions),
                        DQTYPE_THREAD_CURRENT,
                        DQTAT_COM_NONE
                    };

                    ABI::Windows::System::IDispatcherQueueController* ptr{ nullptr };
                    check_hresult(CreateDispatcherQueueController(options, &ptr));
                    dispatcherQueueController = DispatcherQueueController(ptr, take_ownership_from_abi);
                }

                systemBackdropConfiguration = SystemBackdropConfiguration();
                systemBackdropConfiguration.IsInputActive(true);
                systemBackdropConfiguration.Theme((SystemBackdropTheme)RootGrid().ActualTheme());

                backdropController = MicaController();
                backdropController.TintColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorSecondary")).as<Windows::UI::Color>());
                backdropController.FallbackColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorSecondary")).as<Windows::UI::Color>());
                backdropController.TintOpacity(
                    static_cast<float>(Application::Current().Resources().TryLookup(box_value(L"BackdropSecondaryTintOpacity")).as<double>()));
                backdropController.LuminosityOpacity(
                    static_cast<float>(Application::Current().Resources().TryLookup(box_value(L"BackdropSecondaryLuminosityOpacity")).as<double>()));
                backdropController.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                backdropController.AddSystemBackdropTarget(supportsBackdrop);
            }
        }
    }

    void SecondWindow::AppWindow_Closing(AppWindow, AppWindowClosingEventArgs)
    {
        // Save settings
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().CreateContainer(
            L"SettingsWindow",
            ApplicationData::Current().LocalSettings().Containers().HasKey(L"SettingsWindow") ? ApplicationDataCreateDisposition::Existing : ApplicationDataCreateDisposition::Always
        );

        settings.Values().Insert(L"WindowHeight", box_value(appWindow.Size().Height));
        settings.Values().Insert(L"WindowWidth", box_value(appWindow.Size().Width));
        settings.Values().Insert(L"WindowPosX", box_value(appWindow.Position().X));
        settings.Values().Insert(L"WindowPosY", box_value(appWindow.Position().Y));
    }
}
