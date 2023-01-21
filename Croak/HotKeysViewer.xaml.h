// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "HotKeysViewer.g.h"

namespace winrt::Croak::implementation
{
    struct HotKeysViewer : HotKeysViewerT<HotKeysViewer>
    {
        HotKeysViewer();

        bool ShowMouseMap()
        {
            return MouseMapViewBox().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Visible;
        };

        void ShowMouseMap(const bool& value);

        void SetActiveKeys(const winrt::Windows::Foundation::Collections::IVector<winrt::Windows::System::VirtualKey>& activeKeys);
        void AddActiveKey(const winrt::Croak::HotkeyViewModel& hotKeyView);

    private:
        std::vector<winrt::Croak::HotkeyViewModel> hotKeys{};

        winrt::Microsoft::UI::Xaml::Media::Brush GetActiveBrush();
    };
}

namespace winrt::Croak::factory_implementation
{
    struct HotKeysViewer : HotKeysViewerT<HotKeysViewer, implementation::HotKeysViewer>
    {
    };
}
