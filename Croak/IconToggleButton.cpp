// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "IconToggleButton.h"
#if __has_include("IconToggleButton.g.cpp")
#include "IconToggleButton.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Input;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;


namespace winrt::Croak::implementation
{
    DependencyProperty IconToggleButton::_onContentProperty = DependencyProperty::Register(
        L"OnContent",
        xaml_typename<IInspectable>(),
        xaml_typename<Croak::IconToggleButton>(),
        PropertyMetadata(nullptr)
    );
    DependencyProperty IconToggleButton::_offContentProperty = DependencyProperty::Register(
        L"OffContent",
        xaml_typename<IInspectable>(),
        xaml_typename<Croak::IconToggleButton>(),
        PropertyMetadata(nullptr)
    );
    DependencyProperty IconToggleButton::_glyphProperty = DependencyProperty::Register(
        L"Glyph",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconToggleButton>(),
        PropertyMetadata(box_value(L""))
    );
    DependencyProperty IconToggleButton::_onIconProperty = DependencyProperty::Register(
        L"OnIcon",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconToggleButton>(),
        PropertyMetadata(box_value(L""))
    );
    DependencyProperty IconToggleButton::_offIconProperty = DependencyProperty::Register(
        L"OffIcon",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconToggleButton>(),
        PropertyMetadata(box_value(L""))
    );

    IconToggleButton::IconToggleButton()
    {
        DefaultStyleKey(winrt::box_value(L"Croak.IconToggleButton"));
        VisualStateManager::GoToState(*this, L"Off", true);

        loadedRevoker = Loaded(auto_revoke, [this](IInspectable const&, RoutedEventArgs const&)
        {
            if (isOn)
            {
                VisualStateManager::GoToState(*this, L"On", true);
            }
            else
            {
                VisualStateManager::GoToState(*this, L"Off", true);
            }
        });
    }


    inline winrt::Windows::Foundation::IInspectable IconToggleButton::OnContent()
    {
        return unbox_value<winrt::Windows::Foundation::IInspectable>(GetValue(_onContentProperty));
    }

    inline void IconToggleButton::OnContent(const winrt::Windows::Foundation::IInspectable& value)
    {
        SetValue(_onContentProperty, box_value(value));
    }

    inline winrt::Windows::Foundation::IInspectable IconToggleButton::OffContent()
    {
        return unbox_value<winrt::Windows::Foundation::IInspectable>(GetValue(_offContentProperty));
    }

    inline void IconToggleButton::OffContent(const winrt::Windows::Foundation::IInspectable& value)
    {
        SetValue(_offContentProperty, box_value(value));
    }

    inline winrt::hstring IconToggleButton::Glyph()
    {
        return unbox_value<hstring>(GetValue(_glyphProperty));
    }

    inline void IconToggleButton::Glyph(const winrt::hstring& value)
    {
        SetValue(_glyphProperty, box_value(value));
    }

    inline bool IconToggleButton::IsOn()
    {
        return isOn;
    }

    inline void IconToggleButton::IsOn(const bool& value)
    {
        if (isOn != value)
        {
            if (value)
            {
                VisualStateManager::GoToState(*this, L"On", true);
            }
            else
            {
                VisualStateManager::GoToState(*this, L"Off", true);
            }
        }

        isOn = value;
    }

    inline winrt::hstring IconToggleButton::OnIcon()
    {
        return unbox_value<hstring>(GetValue(_onIconProperty));
    }

    inline void IconToggleButton::OnIcon(const winrt::hstring& value)
    {
        SetValue(_onIconProperty, box_value(value));
    }

    inline winrt::hstring IconToggleButton::OffIcon()
    {
        return unbox_value<hstring>(GetValue(_offIconProperty));
    }

    inline void IconToggleButton::OffIcon(const winrt::hstring& value)
    {
        SetValue(_offIconProperty, box_value(value));
    }

    inline bool IconToggleButton::Compact()
    {
        return false;
    }

    inline void IconToggleButton::Compact(const bool& value)
    {
    }


    void IconToggleButton::OnPointerEntered(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args)
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

    void IconToggleButton::OnPointerExited(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args)
    {
        pointerExited = true;
        VisualStateManager::GoToState(*this, L"Normal", true);
    }

    void IconToggleButton::OnPointerPressed(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args)
    {
        if (enabled)
        {
            VisualStateManager::GoToState(*this, L"PointerPressed", true);
            IsOn(!IsOn());
            e_click(*this, RoutedEventArgs());
            args.Handled(true);
        }
    }

    void IconToggleButton::OnPointerReleased(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args)
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
}
