#include "pch.h"
#include "OverlaySettingsPage.xaml.h"
#if __has_include("OverlaySettingsPage.g.cpp")
#include "OverlaySettingsPage.g.cpp"
#endif

#include "DebugOutput.h"

namespace Foundation = winrt::Windows::Foundation;
namespace Input = winrt::Microsoft::UI::Input;
namespace UI = winrt::Windows::UI;
namespace Storage
{
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::ApplicationModel::Resources;
}
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

    void OverlaySettingsPage::OnNavigatedTo(const winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs& args)
    {
        if (args.NavigationMode() == Xaml::Navigation::NavigationMode::New)
        {
            if (SecondWindow::Current().Breadcrumbs().Size() > 0)
            {
                auto atEnd = SecondWindow::Current().Breadcrumbs().GetAt(SecondWindow::Current().Breadcrumbs().Size() - 1);
                if (atEnd.ItemTypeName() != xaml_typename<winrt::Croak::OverlaySettingsPage>())
                {
                    Storage::ResourceLoader resLoader{};
                    SecondWindow::Current().Breadcrumbs().Append(NavigationBreadcrumbBarItem(resLoader.GetString(L"OverlaySettingsPageName"), xaml_typename<winrt::Croak::OverlaySettingsPage>()));
                }
            }
            else
            {
                Storage::ResourceLoader resLoader{};
                SecondWindow::Current().Breadcrumbs().Append(NavigationBreadcrumbBarItem(resLoader.GetString(L"OverlaySettingsPageName"), xaml_typename< winrt::Croak::OverlaySettingsPage >()));
            }
        }
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

        HideWindowToggleSwitch().IsOn(unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"HideWindowInOverlayMode"), false));
    }

    void OverlaySettingsPage::Canvas_PointerPressed(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const&)
    {
        pointerCaptured = true;
        //CanvasBorder().BorderBrush(Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::White()));
    }

    void OverlaySettingsPage::Canvas_PointerReleased(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const&)
    {
        if (pointerCaptured)
        {
            pointerCaptured = false;
            moving = false;

            //CanvasBorder().BorderBrush(Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
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
                    ScreenBorder().Width(newSize > 145 ? newSize : 145);
                }
                else if (cursorShape == Input::InputSystemCursorShape::SizeNorthSouth)
                {
                    double newSize = point.Y - oldPoint.Y;
                    ScreenBorder().Height(newSize > 100 ? newSize : 100);
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

    void OverlaySettingsPage::DisplayCanvas_PointerEntered(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const& /*e*/)
    {
        /*capturedPointer = e.Pointer();
        bool captured = DisplayCanvas().CapturePointer(capturedPointer);*/
    }

    void OverlaySettingsPage::DisplayCanvas_PointerExited(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const&)
    {
        DisplayCanvas().ReleasePointerCapture(capturedPointer);
    }

    void OverlaySettingsPage::ScreenBorder_SizeChanged(Foundation::IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        WidthNumberBlock().Double(ScreenBorder().ActualWidth());
        HeightNumberBlock().Double(ScreenBorder().ActualHeight());
    }

    void OverlaySettingsPage::CenterRightWindowButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        double borderWidth = ScreenBorder().ActualWidth();
        double borderHeight = ScreenBorder().ActualHeight();
        double canvasWidth = DisplayCanvas().ActualWidth();
        double canvasHeight = DisplayCanvas().ActualHeight();
        double x = canvasWidth - 10 - borderWidth;
        double y = (canvasHeight / 2) - (borderHeight / 2);

        Xaml::Canvas::SetLeft(ScreenBorder(), x);
        Xaml::Canvas::SetTop(ScreenBorder(), y);
    }

    void OverlaySettingsPage::CenterLeftWindowButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        double borderHeight = ScreenBorder().ActualHeight();
        double canvasHeight = DisplayCanvas().ActualHeight();
        double x = 10;
        double y = (canvasHeight / 2) - (borderHeight / 2);

        Xaml::Canvas::SetLeft(ScreenBorder(), x);
        Xaml::Canvas::SetTop(ScreenBorder(), y);
    }

    void OverlaySettingsPage::BottomCenterButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        double borderWidth = ScreenBorder().ActualWidth();
        double borderHeight = ScreenBorder().ActualHeight();
        double canvasWidth = DisplayCanvas().ActualWidth();
        double canvasHeight = DisplayCanvas().ActualHeight();
        double x = (canvasWidth / 2) - (borderWidth / 2);
        double y = (canvasHeight - 10) - borderHeight;

        Xaml::Canvas::SetLeft(ScreenBorder(), x);
        Xaml::Canvas::SetTop(ScreenBorder(), y);
    }

    void OverlaySettingsPage::TopCenterWindowButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        double borderWidth = ScreenBorder().ActualWidth();
        double canvasWidth = DisplayCanvas().ActualWidth();
        double x = (canvasWidth / 2) - (borderWidth / 2);
        double y = 10;

        Xaml::Canvas::SetLeft(ScreenBorder(), x);
        Xaml::Canvas::SetTop(ScreenBorder(), y);
    }

    void OverlaySettingsPage::CenterWindowButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        double borderWidth = ScreenBorder().ActualWidth();
        double borderHeight = ScreenBorder().ActualHeight();
        double canvasWidth = DisplayCanvas().ActualWidth();
        double canvasHeight = DisplayCanvas().ActualHeight();
        double x = (canvasWidth / 2) - (borderWidth / 2);
        double y = (canvasHeight / 2) - (borderHeight / 2);

        Xaml::Canvas::SetLeft(ScreenBorder(), x);
        Xaml::Canvas::SetTop(ScreenBorder(), y);
    }

    void OverlaySettingsPage::ApplyChangesButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Storage::ApplicationDataContainer settingsDataContainer = Storage::ApplicationData::Current().LocalSettings();

        int32_t x = static_cast<int32_t>(Xaml::Canvas::GetLeft(ScreenBorder()));
        int32_t y = static_cast<int32_t>(Xaml::Canvas::GetTop(ScreenBorder()));
        int32_t width = static_cast<int32_t>(ScreenBorder().ActualWidth());
        int32_t heigth = static_cast<int32_t>(ScreenBorder().ActualHeight());

        Storage::ApplicationDataCompositeValue composite{};
        composite.Insert(L"X", box_value(x));
        composite.Insert(L"Y", box_value(y));
        composite.Insert(L"Width", box_value(width));
        composite.Insert(L"Height", box_value(heigth));

        settingsDataContainer.Values().Insert(L"OverlayDisplay", composite);
    }

    void OverlaySettingsPage::HideWindowToggleSwitch_Toggled(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Storage::ApplicationData::Current().LocalSettings().Values().Insert(L"HideWindowInOverlayMode", box_value(HideWindowToggleSwitch().IsOn()));
    }


    bool OverlaySettingsPage::InResizingRange(const Input::PointerPoint& pointerPoint, Input::InputSystemCursorShape& cursorShape)
    {
        bool inRange = false;

        Foundation::Point point = pointerPoint.Position();
        double width = ScreenBorder().ActualWidth();
        double height = ScreenBorder().ActualHeight();
        double borderX = Xaml::Canvas::GetLeft(ScreenBorder());
        double borderY = Xaml::Canvas::GetTop(ScreenBorder());
        double x = point.X - borderX;
        double y = point.Y - borderY;

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
