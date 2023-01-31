#include "pch.h"
#include "HotkeyViewModel.h"
#if __has_include("HotkeyViewModel.g.cpp")
#include "HotkeyViewModel.g.cpp"
#endif

using namespace winrt::Windows::System;


namespace winrt::Croak::implementation
{
    VirtualKeyModifiers HotkeyViewModel::GetVirtualModifiers(const uint32_t& value)
    {
        return 
            ((value & MOD_CONTROL) ? VirtualKeyModifiers::Control : VirtualKeyModifiers::None) |
            ((value & MOD_ALT)     ? VirtualKeyModifiers::Menu    : VirtualKeyModifiers::None) |
            ((value & MOD_SHIFT)   ? VirtualKeyModifiers::Shift   : VirtualKeyModifiers::None) |
            ((value & MOD_WIN)     ? VirtualKeyModifiers::Windows : VirtualKeyModifiers::None);
    }

    uint32_t HotkeyViewModel::GetModifiers(const VirtualKeyModifiers& virtualModifiers)
    {
        return
            ((virtualModifiers & VirtualKeyModifiers::Control) == VirtualKeyModifiers::Control ? MOD_CONTROL : 0) |
            ((virtualModifiers & VirtualKeyModifiers::Menu)    == VirtualKeyModifiers::Menu    ? MOD_ALT     : 0) |
            ((virtualModifiers & VirtualKeyModifiers::Shift)   == VirtualKeyModifiers::Shift   ? MOD_SHIFT   : 0) |
            ((virtualModifiers & VirtualKeyModifiers::Windows) == VirtualKeyModifiers::Windows ? MOD_WIN     : 0);
    }
}
