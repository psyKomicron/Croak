#include "pch.h"
#include "HotkeysPage.xaml.h"
#if __has_include("HotkeysPage.g.cpp")
#include "HotkeysPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System;


namespace winrt::Croak::implementation
{
    HotkeysPage::HotkeysPage()
    {
        InitializeComponent();

        winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumeUpHotKeyName"), true, VirtualKey::Up, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumeDownHotKeyName"), true, VirtualKey::Down, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumePageUpHotKeyName"), true, VirtualKey::PageUp, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumePageDownHotKeyName"), true, VirtualKey::PageDown, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumeSwitchStateHotKeyName"), true, VirtualKey::M, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
    }

    void HotkeysPage::OnKeyDown(const winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs&)
    {
        OutputDebugHString(L"Key pressed.");
    }

    void HotkeysPage::Button_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }
}
