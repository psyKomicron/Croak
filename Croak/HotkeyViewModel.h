#pragma once

#include "HotkeyViewModel.g.h"

namespace winrt::Croak::implementation
{
    struct HotkeyViewModel : HotkeyViewModelT<HotkeyViewModel>
    {
    public:
        HotkeyViewModel() = default;
        HotkeyViewModel(const winrt::hstring& hotKeyName, const bool& enabled, const winrt::Windows::System::VirtualKey& key, const winrt::Windows::System::VirtualKeyModifiers& modifiers) :
            _hotKeyName(hotKeyName),
            _isHotKeyEnabled(enabled),
            _key(key),
            _modifiers(modifiers)
        {
        };

        inline winrt::Windows::System::VirtualKeyModifiers Modifiers() const
        {
            return _modifiers;
        };
        inline void Modifiers(const winrt::Windows::System::VirtualKeyModifiers& value)
        {
            _modifiers = value;
        };

        inline winrt::Windows::System::VirtualKey Key() const
        {
            return _key;
        };
        inline void Key(const winrt::Windows::System::VirtualKey& value)
        {
            _key = value;
        };

        inline bool IsHotKeyEnabled() const
        {
            return _isHotKeyEnabled;
        };
        inline void IsHotKeyEnabled(const bool& value)
        {
            _isHotKeyEnabled = value;
        };

        inline winrt::hstring HotKeyName() const
        {
            return _hotKeyName;
        };
        inline void HotKeyName(const winrt::hstring& value)
        {
            _hotKeyName = value;
        };

    private:
        winrt::Windows::System::VirtualKeyModifiers _modifiers = winrt::Windows::System::VirtualKeyModifiers::None;
        winrt::Windows::System::VirtualKey _key = winrt::Windows::System::VirtualKey::None;
        bool _isHotKeyEnabled = false;
        winrt::hstring _hotKeyName{};
    };
}

namespace winrt::Croak::factory_implementation
{
    struct HotkeyViewModel : HotkeyViewModelT<HotkeyViewModel, implementation::HotkeyViewModel>
    {
    };
}
