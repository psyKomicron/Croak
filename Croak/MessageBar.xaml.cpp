#include "pch.h"
#include "MessageBar.xaml.h"
#if __has_include("MessageBar.g.cpp")
#include "MessageBar.g.cpp"
#endif

#include "DebugOutput.h"

using namespace winrt;
namespace Foundation = winrt::Windows::Foundation;
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Input;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Media;
    using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
    using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
}

namespace winrt::Croak::implementation
{
    MessageBar::MessageBar()
    {
        InitializeComponent();

        timer = DispatcherQueue().CreateTimer();
        auto duration = (Xaml::Application::Current().Resources().Lookup(box_value(L"MessageBarIntervalSeconds")).as<int32_t>() * 1000) + 150;
        timer.Interval(
            std::chrono::milliseconds(duration)
        );
        timer.Tick({ this, &MessageBar::DispatcherQueueTimer_Tick });
    }

    void MessageBar::EnqueueMessage(const winrt::Windows::Foundation::IInspectable& message)
    {
        {
            std::unique_lock<std::mutex> lock{ messageQueueMutex };
            messageQueue.push(message);
        }

        if (!timer.IsRunning())
        {
            DisplayMessage();
            timer.Start();
        }
    }

    void MessageBar::EnqueueString(const winrt::hstring& message)
    {
        EnqueueMessage(box_value(message));
    }

    void MessageBar::CloseButton_Click(winrt::Windows::Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        timer.Stop();
        TimerProgressBarStoryboard().Stop();
    }

    void MessageBar::MessageControl_Loading(Xaml::FrameworkElement const&, Foundation::IInspectable const&)
    {
    }

    void MessageBar::UserControl_Loaded(winrt::Windows::Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        InitializeAnimations();
        TimerProgressBar_SizeChanged(nullptr, nullptr);

        // If the HorizontalAlignment is not stretched, add corner radius.
        if (HorizontalAlignment() != Xaml::HorizontalAlignment::Stretch)
        {
            Xaml::CornerRadius cornerRadius = Xaml::Application::Current().Resources().Lookup(winrt::box_value(L"ControlCornerRadius")).as<Xaml::CornerRadius>();
            CornerRadius(cornerRadius);
        }
    }

    void MessageBar::TimerProgressBar_SizeChanged(winrt::Windows::Foundation::IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        TimerProgressBarClipping().Rect(
            Foundation::Rect(0.f, 0.f, static_cast<float>(BackgroundTimerBorder().ActualWidth()), static_cast<float>(BackgroundTimerBorder().ActualHeight())));
    }


    void MessageBar::InitializeAnimations()
    {
        //auto parent = Xaml::VisualTreeHelper::GetParent(*this).as<Xaml::FrameworkElement>();
        //double parentWidth = parent.ActualWidth();
        //double parentHeight = parent.ActualHeight();
        //double width = ActualWidth();
        double height = ActualHeight();
        //if (height == 0 || isnan(height)) DEBUG_BREAK();

        Xaml::Animation::DiscreteDoubleKeyFrame appearDiscreteDoubleKeyFrame{}, disappearDiscreteDoubleKeyFrame{};
        Xaml::Animation::SplineDoubleKeyFrame appearSplineDoubleKeyFrame{}, disappearSplineDoubleKeyFrame{};
        // Set collapsed->visible animation values
        appearDiscreteDoubleKeyFrame.KeyTime(Xaml::Animation::KeyTime(std::chrono::milliseconds(0)));
        appearSplineDoubleKeyFrame.KeyTime(Xaml::Animation::KeyTime(std::chrono::milliseconds(333)));
        Xaml::Animation::KeySpline keySpline{};
        keySpline.ControlPoint1(Foundation::Point(0.f, 0.f));
        keySpline.ControlPoint2(Foundation::Point(0.f, 1.f));
        appearSplineDoubleKeyFrame.KeySpline(keySpline);
        // Set visible->collapsed animation values
        disappearDiscreteDoubleKeyFrame.KeyTime(Xaml::Animation::KeyTime(std::chrono::milliseconds(0)));
        disappearSplineDoubleKeyFrame.KeyTime(Xaml::Animation::KeyTime(std::chrono::milliseconds(333)));
        keySpline = Xaml::Animation::KeySpline();
        keySpline.ControlPoint1(Foundation::Point(1.f, 1.f));
        keySpline.ControlPoint2(Foundation::Point(0.f, 1.f));

        if (VerticalAlignment() == Xaml::VerticalAlignment::Bottom)
        {
            // Appear animation
            appearDiscreteDoubleKeyFrame.Value(height + Margin().Bottom);
            appearSplineDoubleKeyFrame.Value(0);
            // Disappear animation
            disappearDiscreteDoubleKeyFrame.Value(0);
            disappearSplineDoubleKeyFrame.Value(height + Margin().Bottom);
        }
        else
        {
            // Appear animation
            appearDiscreteDoubleKeyFrame.Value((-1 * height) - Margin().Top);
            appearSplineDoubleKeyFrame.Value(0);
            // Disappear animation
            disappearDiscreteDoubleKeyFrame.Value(0);
            disappearSplineDoubleKeyFrame.Value((-1 * height) - Margin().Top);
        }

        VisibleStateStoryboard().Stop();
        CollapsedStateStoryboard().Stop();
        AppearAnimation().KeyFrames().Append(appearDiscreteDoubleKeyFrame);
        AppearAnimation().KeyFrames().Append(appearSplineDoubleKeyFrame);
        DisappearAnimation().KeyFrames().Append(disappearDiscreteDoubleKeyFrame);
        DisappearAnimation().KeyFrames().Append(disappearSplineDoubleKeyFrame);
        VisibleStateStoryboard().Resume();
        CollapsedStateStoryboard().Resume();
    }

    void MessageBar::DisplayMessage()
    {
        // Dequeue a message, send it to the UI
        winrt::Windows::Foundation::IInspectable message = nullptr;

        {
            std::unique_lock<std::mutex> lock{ messageQueueMutex };

            if (!messageQueue.empty())
            {
                message = messageQueue.front();
                messageQueue.pop();
            }
            else
            {
                timer.Stop();
                // Hide the control.
                Xaml::VisualStateManager::GoToState(*this, L"Collapsed", true);
            }
        }

        if (message)
        {
            if (std::optional<hstring> hs = message.try_as<hstring>())
            {
                std::wstring text{ hs.value() };

                static std::wregex word{ L"(\\w+)", std::regex_constants::optimize };
                auto iterator = std::wsregex_iterator(text.begin(), text.end(), word);
                auto ptrDiff = std::distance(iterator, std::wsregex_iterator());

                if (ptrDiff > 0)
                {
                    int64_t msCount = (ptrDiff / 3.5) * 1000;
                    if (msCount < 1000)
                    {
                        msCount = 1000;
                    }

                    timer.Interval(std::chrono::milliseconds(msCount + 150)); // Add a small delay to smooth out.
                    TimerProgressBarAnimation().Duration(
                        Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(msCount))
                    );
                }
            }
            else
            {
                timer.Interval(
                    std::chrono::milliseconds(4150)
                );
                TimerProgressBarAnimation().Duration(
                    Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(4000))
                );
            }

            if (Visibility() == Xaml::Visibility::Collapsed)
            {
                // Show the control.
                Xaml::VisualStateManager::GoToState(*this, L"Visible", true);
            }

            MainContentPresenter().Content(box_value(message));
            TimerProgressBarStoryboard().Begin();
            timer.Start();
        }
    }

    void MessageBar::DispatcherQueueTimer_Tick(winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer, winrt::Windows::Foundation::IInspectable)
    {
        DisplayMessage();
    }
}
