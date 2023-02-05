// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "OverlaySettingsPage.g.h"

namespace winrt::Croak::implementation
{
    struct OverlaySettingsPage : OverlaySettingsPageT<OverlaySettingsPage>
    {
        OverlaySettingsPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void Canvas_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct OverlaySettingsPage : OverlaySettingsPageT<OverlaySettingsPage, implementation::OverlaySettingsPage>
    {
    };
}
