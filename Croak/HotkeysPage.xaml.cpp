#include "pch.h"
#include "HotkeysPage.xaml.h"
#if __has_include("HotkeysPage.g.cpp")
#include "HotkeysPage.g.cpp"
#endif

#include "DebugOutput.h"
#include "HotKeyManager.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System;
using namespace ::Croak::System::HotKeys;


namespace winrt::Croak::implementation
{
    HotkeysPage::HotkeysPage()
    {
        InitializeComponent();

        /*HotKeysViewer().AddActiveKey({ loader.GetString(L"HotKeyName"), true, VirtualKey::Up, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumeDownHotKeyName"), true, VirtualKey::Down, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumePageUpHotKeyName"), true, VirtualKey::PageUp, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumePageDownHotKeyName"), true, VirtualKey::PageDown, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });
        HotKeysViewer().AddActiveKey({ loader.GetString(L"SystemVolumeSwitchStateHotKeyName"), true, VirtualKey::M, VirtualKeyModifiers::Control | VirtualKeyModifiers::Shift });*/
    }

    void HotkeysPage::HotKeysListView_Loading(FrameworkElement const&, IInspectable const&)
    {
        /*winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
        auto&& manager = HotKeyManager::GetHotKeyManager();
        auto&& hotKeys = manager.ViewManagedKeys();

        for (auto&& pair : hotKeys)
        {
            auto name = loader.GetString(L"HotKeyName/" + to_hstring(pair.second.Id()));
            HotkeyViewModel model{
                loader.GetString(L"HotKeyName/" + to_hstring(pair.second.Id())),
                pair.second.IsEnabled(),
                pair.second.VirtualKey(),
                pair.second.Modifiers()
            };

            HotKeysViewer().AddActiveKey(model);

            auto&& view = HotKeyView(model);
            view.VirtualKeyChanged({ this, &HotkeysPage::HotKeyView_VirtualKeyChanged });
            view.VirtualModifiersChanged({ this, &HotkeysPage::HotKeyView_VirtualKeyModifiersChanged });
            view.Toggled({ this, &HotkeysPage::HotKeyView_Toggled });
            HotKeysListView().Items().Append(view);
        }*/
    }

    void HotkeysPage::HotKeyView_VirtualKeyModifiersChanged(const Croak::HotKeyView& sender, const Windows::System::VirtualKeyModifiers& args)
    {
        DebugLog(std::format("{0} virtual key modifiers changed.", winrt::to_string(sender.HotKeyName())));

        /*auto&& manager = HotKeyManager::GetHotKeyManager();
        auto&& keyViews = manager.ViewManagedKeys();
        for (auto&& pair : keyViews)
        {
            uint32_t id = pair.first;
            auto&& keyView = pair.second;

            if (keyView.Modifiers() == GetMods(args) && keyView.VirtualKey() == static_cast<uint32_t>(sender.Key()))
            {
                try
                {
                    manager.EditKey(id, sender.Modifiers(), keyView.VirtualKey());
                    UserMessageBar().EnqueueString(L"Key (" + sender.HotKeyName() + L") edited");
                }
                catch (const std::invalid_argument&)
                {
                    UserMessageBar().EnqueueString(L"Failed to edit key (" + sender.HotKeyName() + L")");
                }

                break;
            }
        }*/
    }

    void HotkeysPage::HotKeyView_VirtualKeyChanged(const Croak::HotKeyView& sender, const Windows::System::VirtualKey& /*args*/)
    {
        DebugLog(std::format("{0} virtual key changed.", winrt::to_string(sender.HotKeyName())));
    }

    void HotkeysPage::HotKeyView_Toggled(const Croak::HotKeyView& sender, const bool& args)
    {
        /*auto&& manager = HotKeyManager::GetHotKeyManager();
        auto&& keyViews = manager.ViewManagedKeys();
        for (auto&& pair : keyViews)
        {
            uint32_t id = pair.first;
            auto&& keyView = pair.second;

            if (keyView.Modifiers() == GetMods(sender.Modifiers()) && keyView.VirtualKey() == static_cast<uint32_t>(sender.Key()))
            {
                try
                {
                    manager.EditKey(id, args);
                    UserMessageBar().EnqueueString(sender.HotKeyName() + (args ? L" enabled" : L" disabled"));
                }
                catch (const std::invalid_argument&)
                {
                    UserMessageBar().EnqueueString(L"Failed to edit key (" + sender.HotKeyName() + L")");
                }
                break;
            }
        }*/
    }

    uint32_t HotkeysPage::GetMods(const Windows::System::VirtualKeyModifiers& virtualKeyModifiers)
    {
        return
            ((virtualKeyModifiers & VirtualKeyModifiers::Control) == VirtualKeyModifiers::Control ? MOD_CONTROL : 0) |
            ((virtualKeyModifiers & VirtualKeyModifiers::Menu)    == VirtualKeyModifiers::Menu    ? MOD_ALT     : 0) |
            ((virtualKeyModifiers & VirtualKeyModifiers::Shift)   == VirtualKeyModifiers::Shift   ? MOD_SHIFT   : 0) |
            ((virtualKeyModifiers & VirtualKeyModifiers::Windows) == VirtualKeyModifiers::Windows ? MOD_WIN     : 0);
    }
}
