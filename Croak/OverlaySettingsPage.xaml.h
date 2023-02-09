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
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ScreenBorder_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void ScreenBorder_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void ScreenBorder_PointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Canvas_PointerReleased(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void DisplayCanvas_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void DisplayCanvas_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

    private:
        bool pointerCaptured = false;
        bool moving = false;
        winrt::Microsoft::UI::Xaml::Input::Pointer capturedPointer = nullptr;

        bool InResizingRange(const Microsoft::UI::Input::PointerPoint& pointerPoint, winrt::Microsoft::UI::Input::InputSystemCursorShape& cursorShape);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct OverlaySettingsPage : OverlaySettingsPageT<OverlaySettingsPage, implementation::OverlaySettingsPage>
    {
    };
}
