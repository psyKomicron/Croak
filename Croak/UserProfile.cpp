#include "pch.h"
#include "UserProfile.h"
#if __has_include("UserProfile.g.cpp")
#include "UserProfile.g.cpp"
#endif

namespace Storage = winrt::Windows::Storage;
namespace Windowing = winrt::Microsoft::UI::Windowing;

namespace winrt::Croak::implementation
{
    void UserProfile::Restore(const Storage::ApplicationDataContainer& container)
    {
        hotKeysEnabled = unbox_value<bool>(container.Values().Lookup(L"HotKeysEnabled"));
        isAlwaysOnTop = unbox_value<bool>(container.Values().Lookup(L"IsAlwaysOnTop"));
        restoreWindowState = unbox_value<bool>(container.Values().Lookup(L"RestoreWindowState"));
        presenterState = static_cast<Windowing::OverlappedPresenterState>(
                unbox_value<int32_t>(container.Values().Lookup(L"PresenterState"))
            );

        Storage::ApplicationDataCompositeValue composite = unbox_value<Storage::ApplicationDataCompositeValue>(container.Values().Lookup(L"WindowDisplayRect"));
        int32_t x = unbox_value<int32_t>(composite.Lookup(L"X"));
        int32_t y = unbox_value<int32_t>(composite.Lookup(L"Y"));
        int32_t width = unbox_value<int32_t>(composite.Lookup(L"Width"));
        int32_t height = unbox_value<int32_t>(composite.Lookup(L"Height"));
        windowDisplayRect = winrt::Windows::Graphics::RectInt32(x, y, width, height);
    }

    void UserProfile::Save(const Storage::ApplicationDataContainer& container)
    {
        container.Values().Insert(L"HotKeysEnabled", box_value(hotKeysEnabled));
        container.Values().Insert(L"IsAlwaysOnTop", box_value(isAlwaysOnTop));
        container.Values().Insert(L"RestoreWindowState", box_value(restoreWindowState));
        container.Values().Insert(L"PresenterState", box_value(static_cast<int32_t>(presenterState)));

        Storage::ApplicationDataCompositeValue composite{};
        composite.Insert(L"X", winrt::box_value(windowDisplayRect.X));
        composite.Insert(L"Y", winrt::box_value(windowDisplayRect.Y));
        composite.Insert(L"Width", winrt::box_value(windowDisplayRect.Width));
        composite.Insert(L"Height", winrt::box_value(windowDisplayRect.Height));
        container.Values().Insert(L"WindowDisplayRect", composite);
    }
}
