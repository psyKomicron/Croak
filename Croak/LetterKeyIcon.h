// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "LetterKeyIcon.g.h"

namespace winrt::Croak::implementation
{
    struct LetterKeyIcon : LetterKeyIconT<LetterKeyIcon>
    {
    public:
        LetterKeyIcon();
        LetterKeyIcon(const winrt::hstring& key);

        winrt::hstring Key() const;
        void Key(const winrt::hstring& value);

    private:
        static winrt::Microsoft::UI::Xaml::DependencyProperty _keyProperty;
    };
}

namespace winrt::Croak::factory_implementation
{
    struct LetterKeyIcon : LetterKeyIconT<LetterKeyIcon, implementation::LetterKeyIcon>
    {
    };
}
