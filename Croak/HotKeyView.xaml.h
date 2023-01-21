// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "HotKeyView.g.h"

namespace winrt::Croak::implementation
{
    struct HotKeyView : HotKeyViewT<HotKeyView>
    {
        HotKeyView();

        inline winrt::Windows::System::VirtualKeyModifiers Modifiers() const
        {
            return _modifiers;
        };
        inline void Modifiers(const winrt::Windows::System::VirtualKeyModifiers& value)
        {
            _modifiers = value;
            SetModifiers();
        };

        inline winrt::Windows::System::VirtualKey Key() const
        {
            return _key;
        };
        inline void Key(const winrt::Windows::System::VirtualKey& value)
        {
            _key = value;
#ifdef DEBUG
            IconContentControl().Content(KeyName());
#else
            e_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"KeyName"));
#endif // DEBUG
        };

        inline bool IsHotKeyEnabled() const
        {
            return _isHotKeyEnabled;
        };
        inline void IsHotKeyEnabled(const bool& value)
        {
            _isHotKeyEnabled = value;
            e_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsHotKeyEnabled"));
        };

        inline winrt::hstring HotKeyName() const
        {
            return _hotKeyName;
        };
        inline void HotKeyName(const winrt::hstring& value)
        {
            _hotKeyName = value;
            e_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"HotKeyName"));
        };

        winrt::Windows::Foundation::IInspectable KeyName() const;


        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        };
        void PropertyChanged(winrt::event_token const& token)
        {
            e_propertyChanged.remove(token);
        };

        void OnKeyDown(const winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs& args);

    private:
        winrt::Windows::System::VirtualKeyModifiers _modifiers = winrt::Windows::System::VirtualKeyModifiers::None;
        winrt::Windows::System::VirtualKey _key = winrt::Windows::System::VirtualKey::None;
        bool _isHotKeyEnabled = false;
        winrt::hstring _hotKeyName{};

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;

        void SetModifiers();
    };
}

namespace winrt::Croak::factory_implementation
{
    struct HotKeyView : HotKeyViewT<HotKeyView, implementation::HotKeyView>
    {
    };
}
