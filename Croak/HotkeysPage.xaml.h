#pragma once

#include "HotkeysPage.g.h"

namespace winrt::Croak::implementation
{
    struct HotkeysPage : HotkeysPageT<HotkeysPage>
    {
        HotkeysPage();

        void HotKeysListView_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void HotKeyView_VirtualKeyModifiersChanged(const Croak::HotKeyView& sender, const Windows::System::VirtualKeyModifiers& args);
        void HotKeyView_VirtualKeyChanged(const Croak::HotKeyView& sender, const Windows::System::VirtualKey& args);
        void HotKeyView_Toggled(const Croak::HotKeyView& sender, const bool& args);

    private:
        uint32_t GetMods(const Windows::System::VirtualKeyModifiers& virtualKeyModifiers);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct HotkeysPage : HotkeysPageT<HotkeysPage, implementation::HotkeysPage>
    {
    };
}
