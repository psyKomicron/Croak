#include "pch.h"
#include "OverlaySettingsPage.xaml.h"
#if __has_include("OverlaySettingsPage.g.cpp")
#include "OverlaySettingsPage.g.cpp"
#endif

#include <math.h>

namespace Foundation = winrt::Windows::Foundation;
namespace Input = winrt::Microsoft::UI::Input;
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

    void OverlaySettingsPage::Page_Loaded(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Windows::Graphics::RectInt32 displayRect = MainWindow::Current().DisplayRect();
        int32_t width = displayRect.Width;
        int32_t height = displayRect.Height;
        auto display = winrt::Microsoft::UI::Windowing::DisplayArea::Primary();
        int32_t displayHeight = display.WorkArea().Height;
        int32_t displayWidth = display.WorkArea().Width;

        double borderHeight = DisplayCanvas().ActualHeight() * (height / static_cast<double>(displayHeight));
        double borderWidth = DisplayCanvas().ActualWidth() * (width / static_cast<double>(displayWidth));
        ScreenBorder().Width(borderWidth);
        ScreenBorder().Height(borderHeight);
    }

    void OverlaySettingsPage::Canvas_PointerPressed(Foundation::IInspectable const& sender, Xaml::PointerRoutedEventArgs const& e)
    {
        if (pointerCaptured)
        {
            DisplayCanvas().ReleasePointerCapture(capturedPointer);
            pointerCaptured = false;

            CanvasBorder().BorderBrush(Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
        }
        else
        {
            capturedPointer = e.Pointer();
            pointerCaptured = DisplayCanvas().CapturePointer(capturedPointer);

            CanvasBorder().BorderBrush(Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::White()));
        }
    }

    void OverlaySettingsPage::DisplayCanvas_PointerMoved(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
        if (pointerCaptured)
        {
            Input::PointerPoint pointerPoint = e.GetCurrentPoint(DisplayCanvas());
            Foundation::Point position = pointerPoint.Position();
            if (position.X > 0)
            {
                Xaml::Controls::Canvas::SetLeft(
                    ScreenBorder(), 
                    position.X < DisplayCanvas().ActualWidth() - ScreenBorder().ActualWidth() ? position.X : DisplayCanvas().ActualWidth() - ScreenBorder().ActualWidth()
                );
            }
            else
            {
                Xaml::Controls::Canvas::SetTop(ScreenBorder(), 0);
            }

            if (position.Y > 0)
            {
                Xaml::Controls::Canvas::SetTop(
                    ScreenBorder(), 
                    position.Y < DisplayCanvas().ActualHeight() - ScreenBorder().ActualHeight() ? position.Y : DisplayCanvas().ActualHeight() - ScreenBorder().ActualHeight()
                );
            }
            else
            {
                Xaml::Controls::Canvas::SetTop(ScreenBorder(), 0);
            }
        }
    }

    void OverlaySettingsPage::Border_PointerPressed(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const&)
    {
    }
}
