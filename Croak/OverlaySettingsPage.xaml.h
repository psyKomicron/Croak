#pragma once

#include "OverlaySettingsPage.g.h"

namespace winrt::Croak::implementation
{
    struct OverlaySettingsPage : OverlaySettingsPageT<OverlaySettingsPage>
    {
    public:
        OverlaySettingsPage();

        void Canvas_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Border_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void DisplayCanvas_PointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        bool pointerCaptured = false;
        winrt::Microsoft::UI::Xaml::Input::Pointer capturedPointer = nullptr;
    public:
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct OverlaySettingsPage : OverlaySettingsPageT<OverlaySettingsPage, implementation::OverlaySettingsPage>
    {
    };
}
