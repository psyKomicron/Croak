#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "MessageBar.g.h"

#include <queue>

namespace winrt::Croak::implementation
{
    struct MessageBar : MessageBarT<MessageBar>
    {
    public:
        MessageBar();

        void EnqueueMessage(const winrt::Windows::Foundation::IInspectable& message);
        void EnqueueString(const winrt::hstring& message);

        void CloseButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TimerProgressBar_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void MessageControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        std::mutex messageQueueMutex{};
        std::queue<winrt::Windows::Foundation::IInspectable> messageQueue{};
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer timer = nullptr;

        void InitializeAnimations();
        void DisplayMessage();

        void DispatcherQueueTimer_Tick(winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer, winrt::Windows::Foundation::IInspectable);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct MessageBar : MessageBarT<MessageBar, implementation::MessageBar>
    {
    };
}
