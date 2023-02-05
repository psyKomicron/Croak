// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "IconToggleButton.g.h"

namespace winrt::Croak::implementation
{
    struct IconToggleButton : IconToggleButtonT<IconToggleButton>
    {
        IconToggleButton();

        static Microsoft::UI::Xaml::DependencyProperty ContentProperty()
        {
            return _contentProperty;
        }

        static Microsoft::UI::Xaml::DependencyProperty GlyphProperty()
        {
            return _glyphProperty;
        }

        static Microsoft::UI::Xaml::DependencyProperty OnIconProperty()
        {
            return _onIconProperty;
        }

        static Microsoft::UI::Xaml::DependencyProperty OffIconProperty()
        {
            return _offIconProperty;
        }

        inline winrt::event_token Click(const Windows::Foundation::TypedEventHandler<winrt::Croak::IconToggleButton, Microsoft::UI::Xaml::RoutedEventArgs>& handler)
        {
            return e_click.add(handler);
        }

        inline void Click(const winrt::event_token token)
        {
            e_click.remove(token);
        }

        inline winrt::Windows::Foundation::IInspectable Content();
        inline void Content(const winrt::Windows::Foundation::IInspectable& value);
        inline winrt::hstring Glyph();
        inline void Glyph(const winrt::hstring& value);
        inline bool IsOn();
        inline void IsOn(const bool& value);
        inline winrt::hstring OnIcon();
        inline void OnIcon(const winrt::hstring& value);
        inline winrt::hstring OffIcon();
        inline void OffIcon(const winrt::hstring& value);
        inline bool Compact();
        inline void Compact(const bool& value);

        void OnPointerEntered(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);
        void OnPointerExited(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);
        void OnPointerPressed(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);
        void OnPointerReleased(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);

    private:
        static Microsoft::UI::Xaml::DependencyProperty _contentProperty;
        static Microsoft::UI::Xaml::DependencyProperty _glyphProperty;
        static Microsoft::UI::Xaml::DependencyProperty _onIconProperty;
        static Microsoft::UI::Xaml::DependencyProperty _offIconProperty;

        Loaded_revoker loadedRevoker;
        bool pointerExited = false;
        bool enabled = true;
        bool isOn = false;

        winrt::event<Windows::Foundation::TypedEventHandler<winrt::Croak::IconToggleButton, Microsoft::UI::Xaml::RoutedEventArgs>> e_click {};
    };
}

namespace winrt::Croak::factory_implementation
{
    struct IconToggleButton : IconToggleButtonT<IconToggleButton, implementation::IconToggleButton>
    {
    };
}
