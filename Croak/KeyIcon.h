// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "KeyIcon.g.h"

namespace winrt::Croak::implementation
{
    struct KeyIcon : KeyIconT<KeyIcon>
    {
    public:
        KeyIcon();
        KeyIcon(const winrt::Windows::System::VirtualKey& key);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct KeyIcon : KeyIconT<KeyIcon, implementation::KeyIcon>
    {
    };
}
