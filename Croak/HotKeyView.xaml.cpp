// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "HotKeyView.xaml.h"
#if __has_include("HotKeyView.g.cpp")
#include "HotKeyView.g.cpp"
#endif

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

    void HotKeyView::OnKeyDown(const winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs&)
    {
        OutputDebugHString(L"Key pressed.");
    }


    void HotKeyView::SetModifiers()
    {
        ControlToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Control) == VirtualKeyModifiers::Control);
        AltToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Menu) == VirtualKeyModifiers::Menu);
        ShiftToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Shift) == VirtualKeyModifiers::Shift);
        WindowsToggleButton().IsChecked((_modifiers & VirtualKeyModifiers::Windows) == VirtualKeyModifiers::Windows);
    }
}
