#include "pch.h"
#include "OverlaySettingsPage.xaml.h"
#if __has_include("OverlaySettingsPage.g.cpp")
#include "OverlaySettingsPage.g.cpp"
#endif

#include "DebugOutput.h"

namespace Foundation = winrt::Windows::Foundation;
namespace Input = winrt::Microsoft::UI::Input;
namespace UI = winrt::Windows::UI;
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Input;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Media;
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
        pointerCaptured = true;
        CanvasBorder().BorderBrush(Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::White()));
    }

    void OverlaySettingsPage::Canvas_PointerReleased(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
        if (pointerCaptured)
        {
            pointerCaptured = false;
            moving = false;

            CanvasBorder().BorderBrush(Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
        }
    }

    void OverlaySettingsPage::DisplayCanvas_PointerMoved(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
        Input::InputSystemCursorShape cursorShape{};
        bool inResizingRange = InResizingRange(e.GetCurrentPoint(DisplayCanvas()), cursorShape);
        ProtectedCursor(Input::InputSystemCursor::Create(cursorShape));

        if (pointerCaptured)
        {
            if (inResizingRange && !moving)
            {
                auto oldPoint = Foundation::Point(Xaml::Canvas::GetLeft(ScreenBorder()), Xaml::Canvas::GetTop(ScreenBorder()));
                auto point = e.GetCurrentPoint(DisplayCanvas()).Position();
                if (cursorShape == Input::InputSystemCursorShape::SizeWestEast)
                {
                    double newSize = point.X - oldPoint.X;
                    ScreenBorder().Width(newSize > 20 ? newSize : 20);
                }
                else if (cursorShape == Input::InputSystemCursorShape::SizeNorthSouth)
                {
                    double newSize = point.Y - oldPoint.Y;
                    ScreenBorder().Height(newSize > 20 ? newSize : 20);
                }
            }
            else
            {
                moving = true;

                Input::PointerPoint pointerPoint = e.GetCurrentPoint(DisplayCanvas());
                Foundation::Point position = pointerPoint.Position();
                double canvasWidth = DisplayCanvas().ActualWidth();
                double canvasHeight = DisplayCanvas().ActualHeight();
                double borderWidth = ScreenBorder().ActualWidth();
                double borderHeight = ScreenBorder().ActualHeight();

                double posX = position.X - (borderWidth / 2);
                if (posX > 0)
                {
                    Xaml::Controls::Canvas::SetLeft(
                        ScreenBorder(),
                        posX < canvasWidth - borderWidth ? posX : canvasWidth - borderWidth
                    );
                }
                else
                {
                    Xaml::Controls::Canvas::SetTop(ScreenBorder(), 0);
                }

                double posY = position.Y - (borderHeight / 2);
                if (posY > 0)
                {
                    Xaml::Controls::Canvas::SetTop(
                        ScreenBorder(),
                        posY < canvasHeight - borderHeight ? posY : canvasHeight - borderHeight
                    );
                }
                else
                {
                    Xaml::Controls::Canvas::SetTop(ScreenBorder(), 0);
                }
            }
        }
    }

    void OverlaySettingsPage::DisplayCanvas_PointerEntered(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
        capturedPointer = e.Pointer();
        bool captured = DisplayCanvas().CapturePointer(capturedPointer);
    }

    void OverlaySettingsPage::DisplayCanvas_PointerExited(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
        DisplayCanvas().ReleasePointerCapture(capturedPointer);
    }

    void OverlaySettingsPage::Border_PointerPressed(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const&)
    {
    }

    void OverlaySettingsPage::ScreenBorder_PointerEntered(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
        /*if (!pointerCaptured)
        {
            capturedPointer = e.Pointer();
            ScreenBorder().CapturePointer(capturedPointer);
        }*/
    }

    void OverlaySettingsPage::ScreenBorder_PointerExited(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
        /*if (!pointerCaptured)
        {
            ScreenBorder().ReleasePointerCapture(capturedPointer);
        }

        ProtectedCursor(Input::InputSystemCursor::Create(Input::InputSystemCursorShape::Arrow));*/
    }

    void OverlaySettingsPage::ScreenBorder_PointerMoved(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& e)
    {
    }


    bool OverlaySettingsPage::InResizingRange(const Input::PointerPoint& pointerPoint, Input::InputSystemCursorShape& cursorShape)
    {
        bool inRange = false;

        Foundation::Point point = pointerPoint.Position();
        double x = point.X;
        double y = point.Y;
        double width = ScreenBorder().ActualWidth();
        double height = ScreenBorder().ActualHeight();
        double borderX = Xaml::Canvas::GetLeft(ScreenBorder());
        double borderY = Xaml::Canvas::GetTop(ScreenBorder());

        DebugLog(std::format("\n\tx = {0}\n\ty = {1}\n\twidth = {4}\n\theight = {5}\n\tbounds X = {2}\n\tbounds Y = {3}", x, y, (borderX + width), (borderY + height), width, height));

        if (x < 0 || y < 0 || x > (borderX + width) || y > (borderY + height))
        {
            return false;
        }

        if (x <= 20)
        {
            if (y >= 0 && y <= 20)
            {
                // Top left.
                cursorShape = Input::InputSystemCursorShape::SizeNorthwestSoutheast;
                inRange = true;
            }
            else if (y > (height - 20))
            {
                cursorShape = Input::InputSystemCursorShape::SizeNortheastSouthwest;
                inRange = true;
            }
            else
            {
                cursorShape = Input::InputSystemCursorShape::SizeWestEast;
                inRange = true;
            }
        }
        else if (x >= width - 20)
        {
            if (y >= 0 && y <= 20)
            {
                // Top right.
                cursorShape = Input::InputSystemCursorShape::SizeNortheastSouthwest;
                inRange = true;
            }
            else if (y > (height - 20))
            {
                // Bottom right
                cursorShape = Input::InputSystemCursorShape::SizeNorthwestSoutheast;
                inRange = true;
            }
            else
            {
                // Middle right
                cursorShape = Input::InputSystemCursorShape::SizeWestEast;
                inRange = true;
            }
        }
        else
        {
            if ((y >= 0 && y <= 20) || y > (height - 20))
            {
                // Top right.
                cursorShape = Input::InputSystemCursorShape::SizeNorthSouth;
                inRange = true;
            }
        }

        return inRange;
    }
}
