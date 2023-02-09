// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "AudioViewer.xaml.h"
#if __has_include("AudioViewer.g.cpp")
#include "AudioViewer.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Croak::implementation
{
    AudioViewer::AudioViewer()
    {
        InitializeComponent();
    }

    int32_t AudioViewer::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void AudioViewer::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void AudioViewer::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}
