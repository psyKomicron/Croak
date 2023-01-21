#pragma once

#include "HotkeysPage.g.h"

namespace winrt::Croak::implementation
{
    struct HotkeysPage : HotkeysPageT<HotkeysPage>
    {
        HotkeysPage();

        void OnKeyDown(const winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs& args);
        void Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct HotkeysPage : HotkeysPageT<HotkeysPage, implementation::HotkeysPage>
    {
    };
}
