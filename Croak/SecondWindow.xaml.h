#pragma once

#include "SecondWindow.g.h"

namespace winrt::Croak::implementation
{
    struct SecondWindow : SecondWindowT<SecondWindow>
    {
    public:
        SecondWindow();

        static inline winrt::Croak::SecondWindow Current()
        {
            return singleton;
        }

        inline winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::NavigationBreadcrumbBarItem> Breadcrumbs()
        {
            return breadcrumbs;
        }

        inline winrt::Windows::Graphics::RectInt32 DisplayRect()
        {
            if (appWindow)
            {
                winrt::Windows::Graphics::PointInt32 position = appWindow.Position();
                winrt::Windows::Graphics::SizeInt32 size = appWindow.Size();
                return winrt::Windows::Graphics::RectInt32(position.X, position.Y, size.Width, size.Height);
            }
            else
            {
                return winrt::Windows::Graphics::RectInt32();
            }
        }

        inline winrt::Microsoft::UI::WindowId Id() const
        {
            return appWindow.Id();
        }

        bool NavigateTo(const winrt::Windows::UI::Xaml::Interop::TypeName& typeName);

        void Grid_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NavigationFrame_Navigated(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
        void NavigationBreadcrumbBar_ItemClicked(winrt::Microsoft::UI::Xaml::Controls::BreadcrumbBar const& sender, winrt::Microsoft::UI::Xaml::Controls::BreadcrumbBarItemClickedEventArgs const& args);
        void NavigationFrame_NavigationFailed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const& e);
        void RootGrid_ActualThemeChanged(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void NavigationBackButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        static winrt::Croak::SecondWindow singleton;

        winrt::Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        winrt::event_token appWindowClosingEventToken;
        winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController backdropController = nullptr;
        winrt::Windows::System::DispatcherQueueController dispatcherQueueController = nullptr;
        winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration systemBackdropConfiguration = nullptr;
        winrt::Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker themeChangedRevoker;
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::Croak::NavigationBreadcrumbBarItem> breadcrumbs
        {
            single_threaded_observable_vector<winrt::Croak::NavigationBreadcrumbBarItem>()
        };
        bool usingCustomTitleBar = false;

        void InitializeWindow();
        void SetBackground();
        void AppWindow_Closing(winrt::Microsoft::UI::Windowing::AppWindow, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct SecondWindow : SecondWindowT<SecondWindow, implementation::SecondWindow>
    {
    };
}
