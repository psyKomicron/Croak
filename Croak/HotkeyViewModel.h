#pragma once

#include "HotkeyViewModel.g.h"

namespace winrt::Croak::implementation
{
    struct HotkeyViewModel : HotkeyViewModelT<HotkeyViewModel>
    {
    public:
        HotkeyViewModel() = default;
        HotkeyViewModel(const winrt::hstring& hotKeyName, const bool& enabled, const uint32_t& key, const uint32_t& modifiers) :
            _hotKeyName(hotKeyName),
            _isHotKeyEnabled(enabled),
            key(key),
            modifiers(modifiers)
        {
        }

        inline winrt::Windows::System::VirtualKeyModifiers Modifiers()
        {
            return GetVirtualModifiers(modifiers);
        }

        inline void Modifiers(const winrt::Windows::System::VirtualKeyModifiers& value)
        {
            modifiers = GetModifiers(value);
        }

        inline winrt::Windows::System::VirtualKey Key()
        {
            return static_cast<winrt::Windows::System::VirtualKey>(key);
        }

        inline void Key(const winrt::Windows::System::VirtualKey& value)
        {
            key = static_cast<uint32_t>(value);
        }

        inline bool IsHotKeyEnabled() const
        {
            return _isHotKeyEnabled;
        }

        inline void IsHotKeyEnabled(const bool& value)
        {
            _isHotKeyEnabled = value;
        }

        inline winrt::hstring HotKeyName() const
        {
            return _hotKeyName;
        }

        inline void HotKeyName(const winrt::hstring& value)
        {
            _hotKeyName = value;
        }

    private:
        uint32_t modifiers = 0u;
        uint32_t key = 0u;
        bool _isHotKeyEnabled = false;
        winrt::hstring _hotKeyName{};

        Windows::System::VirtualKeyModifiers GetVirtualModifiers(const uint32_t& value);
        uint32_t GetModifiers(const Windows::System::VirtualKeyModifiers& modifier);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct HotkeyViewModel : HotkeyViewModelT<HotkeyViewModel, implementation::HotkeyViewModel>
    {
    };
}
