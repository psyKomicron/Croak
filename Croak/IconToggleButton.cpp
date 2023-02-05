// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "IconToggleButton.h"
#if __has_include("IconToggleButton.g.cpp")
#include "IconToggleButton.g.cpp"
#endif

namespace UI = winrt::Microsoft::UI::Input;
namespace Control = winrt::Microsoft::UI::Xaml::Controls;
namespace Foundation = winrt::Windows::Foundation;
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Input;
}


namespace winrt::Croak::implementation
{
    Xaml::DependencyProperty IconToggleButton::_contentProperty = Xaml::DependencyProperty::Register(
        L"Content",
        xaml_typename<IInspectable>(),
        xaml_typename<Croak::IconToggleButton>(),
        Xaml::PropertyMetadata(nullptr)
    );
    Xaml::DependencyProperty IconToggleButton::_glyphProperty = Xaml::DependencyProperty::Register(
        L"Glyph",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconToggleButton>(),
        Xaml::PropertyMetadata(box_value(L""))
    );
    Xaml::DependencyProperty IconToggleButton::_onIconProperty = Xaml::DependencyProperty::Register(
        L"OnIcon",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconToggleButton>(),
        Xaml::PropertyMetadata(box_value(L""))
    );
    Xaml::DependencyProperty IconToggleButton::_offIconProperty = Xaml::DependencyProperty::Register(
        L"OffIcon",
        xaml_typename<hstring>(),
        xaml_typename<Croak::IconToggleButton>(),
        Xaml::PropertyMetadata(box_value(L""))
    );

    IconToggleButton::IconToggleButton()
    {
        DefaultStyleKey(winrt::box_value(L"Croak.IconToggleButton"));
        Xaml::VisualStateManager::GoToState(*this, L"Off", true);

        loadedRevoker = Loaded(auto_revoke, [this](IInspectable const&, Xaml::RoutedEventArgs const&)
        {
            if (isOn)
            {
                Xaml::VisualStateManager::GoToState(*this, L"On", true);
            }
            else
            {
                Xaml::VisualStateManager::GoToState(*this, L"Off", true);
            }
        });
    }


    inline Foundation::IInspectable IconToggleButton::Content()
    {
        return unbox_value<winrt::Windows::Foundation::IInspectable>(GetValue(_contentProperty));
    }

    inline void IconToggleButton::Content(const Foundation::IInspectable& value)
    {
        SetValue(_contentProperty, box_value(value));
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
                Xaml::VisualStateManager::GoToState(*this, L"On", true);
            }
            else
            {
                Xaml::VisualStateManager::GoToState(*this, L"Off", true);
            }
        }

        isOn = value;
    }

    inline winrt::hstring IconToggleButton::OnIcon()
    {
        return unbox_value<winrt::hstring>(GetValue(_onIconProperty));
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


    void IconToggleButton::OnPointerEntered(const Xaml::PointerRoutedEventArgs& args)
    {
        pointerExited = false;

        bool leftButtonPressed = false;
        if (args.Pointer().PointerDeviceType() == UI::PointerDeviceType::Mouse)
        {
            auto&& point = args.GetCurrentPoint(*this);
            leftButtonPressed = point.Properties().IsLeftButtonPressed();
        }

        Xaml::VisualStateManager::GoToState(*this, L"PointerOver", !leftButtonPressed);
    }

    void IconToggleButton::OnPointerExited(const Xaml::PointerRoutedEventArgs& args)
    {
        pointerExited = true;
        Xaml::VisualStateManager::GoToState(*this, L"Normal", true);
    }

    void IconToggleButton::OnPointerPressed(const Xaml::PointerRoutedEventArgs& args)
    {
        if (enabled)
        {
            Xaml::VisualStateManager::GoToState(*this, L"PointerPressed", true);
            IsOn(!IsOn());
            e_click(*this, Xaml::RoutedEventArgs());
            args.Handled(true);
        }
    }

    void IconToggleButton::OnPointerReleased(const Xaml::PointerRoutedEventArgs& args)
    {
        if (pointerExited)
        {
            Xaml::VisualStateManager::GoToState(*this, L"Normal", true);
        }
        else
        {
            Xaml::VisualStateManager::GoToState(*this, L"PointerOver", true);
        }
    }
}
