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
    public:
        HotKeyView();
        HotKeyView(const Croak::HotkeyViewModel& model);


        inline winrt::Windows::System::VirtualKeyModifiers Modifiers() const
        {
            return _modifiers;
        }

        inline void Modifiers(const winrt::Windows::System::VirtualKeyModifiers& value)
        {
            _modifiers = value;
            SetModifiers();
        }

        inline winrt::Windows::System::VirtualKey Key() const
        {
            return _key;
        }

        inline void Key(const winrt::Windows::System::VirtualKey& value)
        {
            _key = value;
            e_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"KeyName"));
        }

        inline bool IsHotKeyEnabled() const
        {
            return _isHotKeyEnabled;
        }

        inline void IsHotKeyEnabled(const bool& value)
        {
            _isHotKeyEnabled = value;
            e_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsHotKeyEnabled"));
        }

        inline winrt::hstring HotKeyName() const
        {
            return _hotKeyName;
        }

        inline void HotKeyName(const winrt::hstring& value)
        {
            _hotKeyName = value;
            e_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"HotKeyName"));
        }

        winrt::Windows::Foundation::IInspectable KeyName() const;

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value);
        void PropertyChanged(winrt::event_token const& token);
        winrt::event_token VirtualModifiersChanged(const Windows::Foundation::TypedEventHandler<Croak::HotKeyView, Windows::System::VirtualKeyModifiers>& handler);
        void VirtualModifiersChanged(const winrt::event_token& token);
        winrt::event_token VirtualKeyChanged(const Windows::Foundation::TypedEventHandler<Croak::HotKeyView, Windows::System::VirtualKey>& handler);
        void VirtualKeyChanged(const winrt::event_token& token);
        winrt::event_token Toggled(const Windows::Foundation::TypedEventHandler<Croak::HotKeyView, bool>& handler);
        void Toggled(const winrt::event_token& token);

        void OnKeyDown(const winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs& args);
        void ControlToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        std::atomic_bool loaded = false;
        winrt::Windows::System::VirtualKeyModifiers _modifiers = winrt::Windows::System::VirtualKeyModifiers::None;
        winrt::Windows::System::VirtualKey _key = winrt::Windows::System::VirtualKey::None;
        bool _isHotKeyEnabled = false;
        winrt::hstring _hotKeyName{};

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged{};
        winrt::event<Windows::Foundation::TypedEventHandler<Croak::HotKeyView, Windows::System::VirtualKeyModifiers>> e_virtualKeyModifiersChanged{};
        winrt::event<Windows::Foundation::TypedEventHandler<Croak::HotKeyView, Windows::System::VirtualKey>> e_virtualKeyChanged{};
        winrt::event<Windows::Foundation::TypedEventHandler<Croak::HotKeyView, bool>> e_toggled{};

        void SetModifiers();
        Windows::System::VirtualKeyModifiers GetModifiers();
    };
}

namespace winrt::Croak::factory_implementation
{
    struct HotKeyView : HotKeyViewT<HotKeyView, implementation::HotKeyView>
    {
    };
}
