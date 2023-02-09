// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "AudioViewer.g.h"

namespace winrt::Croak::implementation
{
    struct AudioViewer : AudioViewerT<AudioViewer>
    {
        AudioViewer();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct AudioViewer : AudioViewerT<AudioViewer, implementation::AudioViewer>
    {
    };
}
