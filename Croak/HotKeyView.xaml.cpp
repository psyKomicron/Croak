#include "pch.h"
#include "HotKeyView.xaml.h"
#if __has_include("HotKeyView.g.cpp")
#include "HotKeyView.g.cpp"
#endif

#include "DebugOutput.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::System;


namespace winrt::Croak::implementation
{
    HotKeyView::HotKeyView()
    {
        InitializeComponent();
    }

    HotKeyView::HotKeyView(const HotkeyViewModel& model) :
        _key{ model.Key() },
        _modifiers{ model.Modifiers() },
        _isHotKeyEnabled{ model.IsHotKeyEnabled() },
        _hotKeyName{ model.HotKeyName() }
    {
        InitializeComponent();
    }

    IInspectable HotKeyView::KeyName() const
    {
        int32_t keyValue = static_cast<int32_t>(_key);

        if (keyValue >= static_cast<int32_t>(VirtualKey::A) && keyValue <= static_cast<int32_t>(VirtualKey::Z))
        {
            return LetterKeyIcon(to_hstring(static_cast<char16_t>(keyValue)));
        }
        else
        {
            return KeyIcon(_key);
        }
    }


    winrt::event_token HotKeyView::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
    {
        return e_propertyChanged.add(value);
    }

    void HotKeyView::PropertyChanged(winrt::event_token const& token)
    {
        e_propertyChanged.remove(token);
    }

    winrt::event_token HotKeyView::VirtualModifiersChanged(const Windows::Foundation::TypedEventHandler<Croak::HotKeyView, Windows::System::VirtualKeyModifiers>& handler)
    {
        return e_virtualKeyModifiersChanged.add(handler);
    }

    void HotKeyView::VirtualModifiersChanged(const winrt::event_token& token)
    {
        e_virtualKeyModifiersChanged.remove(token);
    }

    winrt::event_token HotKeyView::VirtualKeyChanged(const Windows::Foundation::TypedEventHandler<Croak::HotKeyView, Windows::System::VirtualKey>& handler)
    {
        return e_virtualKeyChanged.add(handler);
    }

    void HotKeyView::VirtualKeyChanged(const winrt::event_token& token)
    {
        e_virtualKeyChanged.remove(token);
    }

    winrt::event_token HotKeyView::Toggled(const Windows::Foundation::TypedEventHandler<Croak::HotKeyView, bool>& handler)
    {
        return e_toggled.add(handler);
    }

    void HotKeyView::Toggled(const winrt::event_token& token)
    {
        e_toggled.remove(token);
    }


    void HotKeyView::OnKeyDown(const winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs&)
    {
    }

    void HotKeyView::ControlToggleButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        VirtualKeyModifiers oldModifiers = _modifiers;
        _modifiers = GetModifiers();

        if (oldModifiers != _modifiers)
        {
            e_virtualKeyModifiersChanged(*this, oldModifiers);
        }
    }

    void HotKeyView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        SetModifiers();
        loaded.store(true);
    }

    void HotKeyView::ToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        if (loaded.load())
        {
            e_toggled(*this, EnabledToggleSwitch().IsOn());
        }
    }


    void HotKeyView::SetModifiers()
    {
        ControlToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Control) == VirtualKeyModifiers::Control);
        AltToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Menu) == VirtualKeyModifiers::Menu);
        ShiftToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Shift) == VirtualKeyModifiers::Shift);
        WindowsToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Windows) == VirtualKeyModifiers::Windows);
    }

    VirtualKeyModifiers HotKeyView::GetModifiers()
    {
        VirtualKeyModifiers modifiers = VirtualKeyModifiers::None;
        modifiers |= ControlToggleButton().IsChecked().GetBoolean() ? VirtualKeyModifiers::Control : VirtualKeyModifiers::None;
        modifiers |= AltToggleButton().IsChecked().GetBoolean()     ? VirtualKeyModifiers::Menu    : VirtualKeyModifiers::None;
        modifiers |= ShiftToggleButton().IsChecked().GetBoolean()   ? VirtualKeyModifiers::Shift   : VirtualKeyModifiers::None;
        modifiers |= WindowsToggleButton().IsChecked().GetBoolean() ? VirtualKeyModifiers::Windows : VirtualKeyModifiers::None;

        return modifiers;
    }
}
