#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "IconButton.g.h"

namespace winrt::Croak::implementation
{
    struct IconButton : IconButtonT<IconButton>
    {
    public:
        IconButton();

        static Microsoft::UI::Xaml::DependencyProperty GlyphProperty()
        {
            return _glyphProperty;
        }

        static Microsoft::UI::Xaml::DependencyProperty TextProperty()
        {
            return _textNameProperty;
        }

        static Microsoft::UI::Xaml::DependencyProperty ContentProperty()
        {
            return _contentProperty;
        }

        inline winrt::hstring Glyph() const;
        inline void Glyph(const winrt::hstring& value);
        inline winrt::hstring Text() const;
        inline void Text(const winrt::hstring& value);
        inline winrt::Windows::Foundation::IInspectable Content();
        inline void Content(const winrt::Windows::Foundation::IInspectable& value);

        inline winrt::event_token Click(const Windows::Foundation::TypedEventHandler<winrt::Croak::IconButton, Microsoft::UI::Xaml::RoutedEventArgs>& handler)
        {
            return e_click.add(handler);
        };

        inline void Click(const winrt::event_token token)
        {
            e_click.remove(token);
        };

        void OnPointerEntered(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);
        void OnPointerExited(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);
        void OnPointerPressed(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);
        void OnPointerReleased(const Microsoft::UI::Xaml::Input::PointerRoutedEventArgs& args);

    private:
        static Microsoft::UI::Xaml::DependencyProperty _glyphProperty;
        static Microsoft::UI::Xaml::DependencyProperty _textNameProperty;
        static Microsoft::UI::Xaml::DependencyProperty _contentProperty;

        bool pointerExited = false;
        bool enabled = true;
        Loaded_revoker loadedRevoker;

        winrt::event<Windows::Foundation::TypedEventHandler<winrt::Croak::IconButton, Microsoft::UI::Xaml::RoutedEventArgs>> e_click {};
    };
}

namespace winrt::Croak::factory_implementation
{
    struct IconButton : IconButtonT<IconButton, implementation::IconButton>
    {
    };
}
