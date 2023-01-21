#include "pch.h"
#include "IconButton.h"
#if __has_include("IconButton.g.cpp")
#include "IconButton.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Input;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Controls;


namespace winrt::Croak::implementation
{
    DependencyProperty IconButton::_glyphProperty = DependencyProperty::Register(
        L"Glyph",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconButton>(),
        PropertyMetadata(box_value(L""))
    );
    DependencyProperty IconButton::_textNameProperty = DependencyProperty::Register(
        L"Text",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconButton>(),
        PropertyMetadata(box_value(L""))
    );
    DependencyProperty IconButton::_contentProperty = DependencyProperty::Register(
        L"Content",
        xaml_typename<winrt::Windows::Foundation::IInspectable>(),
        xaml_typename<Croak::IconButton>(),
        PropertyMetadata(nullptr)
    );

    IconButton::IconButton()
    {
        DefaultStyleKey(winrt::box_value(L"Croak.IconButton"));

        loadedRevoker = Loaded(auto_revoke, [this](auto&&, auto&&)
        {
            // If the control has content, hide the text control.
            VisualStateManager::GoToState(*this, Content() != nullptr ? L"UsingPresenter" : L"UsingText", false);
        });
    }

    inline winrt::hstring IconButton::Glyph() const
    {
        return unbox_value<hstring>(GetValue(_glyphProperty));
    }

    inline void IconButton::Glyph(const winrt::hstring& value)
    {
        SetValue(_glyphProperty, box_value(value));
    }

    inline winrt::hstring IconButton::Text() const
    {
        return unbox_value<hstring>(GetValue(_textNameProperty));
    }

    inline void IconButton::Text(const winrt::hstring& value)
    {
        SetValue(_textNameProperty, box_value(value));
    }

    inline winrt::Windows::Foundation::IInspectable IconButton::Content()
    {
        return GetValue(_contentProperty);
    }

    inline void IconButton::Content(const winrt::Windows::Foundation::IInspectable& value)
    {
        SetValue(_contentProperty, value);

        VisualStateManager::GoToState(*this, value != nullptr ? L"UsingPresenter" : L"UsingText", false);
    }

    void IconButton::OnPointerPressed(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args)
    {
        if (enabled)
        {
            VisualStateManager::GoToState(*this, L"PointerPressed", true);
            e_click(*this, RoutedEventArgs());
            args.Handled(true);
        }
    }

    void IconButton::OnPointerReleased(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args)
    {
        if (pointerExited)
        {
            VisualStateManager::GoToState(*this, L"Normal", true);
        }
        else
        {
            VisualStateManager::GoToState(*this, L"PointerOver", true);
        }
    }

    void IconButton::OnPointerEntered(const PointerRoutedEventArgs& args)
    {
        pointerExited = false;

        bool leftButtonPressed = false;
        if (args.Pointer().PointerDeviceType() == PointerDeviceType::Mouse)
        {
            auto&& point = args.GetCurrentPoint(*this);
            leftButtonPressed = point.Properties().IsLeftButtonPressed();
        }

        if (!leftButtonPressed)
        {
            VisualStateManager::GoToState(*this, L"PointerOver", true);
        }
    }

    void IconButton::OnPointerExited(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args)
    {
        pointerExited = true;
        VisualStateManager::GoToState(*this, L"Normal", true);
    }
}
