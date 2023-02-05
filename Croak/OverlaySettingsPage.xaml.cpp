#include "pch.h"
#include "OverlaySettingsPage.xaml.h"
#if __has_include("OverlaySettingsPage.g.cpp")
#include "OverlaySettingsPage.g.cpp"
#endif

namespace Foundation = winrt::Windows::Foundation;
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Input;
}


namespace winrt::Croak::implementation
{
    OverlaySettingsPage::OverlaySettingsPage()
    {
        InitializeComponent();
    }

    void OverlaySettingsPage::Canvas_PointerPressed(Foundation::IInspectable const& sender, Xaml::PointerRoutedEventArgs const& e)
    {
        if (DisplayCanvas().CapturePointer(e.Pointer()))
        {

        }
    }
}
