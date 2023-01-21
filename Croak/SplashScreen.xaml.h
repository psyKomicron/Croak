#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "SplashScreen.g.h"

namespace winrt::Croak::implementation
{
    struct SplashScreen : SplashScreenT<SplashScreen>
    {
    public:
        SplashScreen();

        winrt::Windows::Foundation::IInspectable TopContent()
        {
            return _topContent;
        }

        void TopContent(winrt::Windows::Foundation::IInspectable value)
        {
            _topContent = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"TopContent"));
        }

        winrt::Windows::Foundation::IInspectable BottomContent()
        {
            return _bottomContent;
        }

        void BottomContent(winrt::Windows::Foundation::IInspectable value)
        {
            _bottomContent = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"BottomContent"));
        }

        winrt::Windows::Foundation::IInspectable MainContent()
        {
            return _mainContent;
        }

        void MainContent(winrt::Windows::Foundation::IInspectable value)
        {
            _mainContent = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"MainContent"));
        };
        winrt::Windows::Foundation::TimeSpan OnScreenTimeSpan()
        {
            return _onScreenTimeSpan;
        }

        void OnScreenTimeSpan(winrt::Windows::Foundation::TimeSpan value);

        inline winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return e_propertyChanged.add(handler);
        }

        inline void PropertyChanged(winrt::event_token const& token)
        {
            e_propertyChanged.remove(token);
        }

        inline winrt::event_token Dismissed(const winrt::Windows::Foundation::TypedEventHandler<winrt::Croak::SplashScreen, winrt::Windows::Foundation::IInspectable>& handler)
        {
            return e_dismissed.add(handler);
        }

        inline void Dismissed(const winrt::event_token& token)
        {
            e_dismissed.remove(token);
        }

        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TimerProgressBar_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void TimerProgressBarAnimation_Completed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void NextButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        winrt::Windows::Foundation::IInspectable _topContent;
        winrt::Windows::Foundation::IInspectable _bottomContent;
        winrt::Windows::Foundation::IInspectable _mainContent;
        winrt::Windows::Foundation::TimeSpan _onScreenTimeSpan;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged {};
        winrt::event<Windows::Foundation::TypedEventHandler<winrt::Croak::SplashScreen, winrt::Windows::Foundation::IInspectable>> e_dismissed {};
    };
}

namespace winrt::Croak::factory_implementation
{
    struct SplashScreen : SplashScreenT<SplashScreen, implementation::SplashScreen>
    {
    };
}
