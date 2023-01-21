// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once

#include "NewContentPage.g.h"

namespace winrt::Croak::implementation
{
    struct NewContentPage : NewContentPageT<NewContentPage>
    {
        NewContentPage();

        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& args);
        winrt::Windows::Foundation::IAsyncAction Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
    };
}

namespace winrt::Croak::factory_implementation
{
    struct NewContentPage : NewContentPageT<NewContentPage, implementation::NewContentPage>
    {
    };
}
