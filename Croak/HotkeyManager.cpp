#include "pch.h"
#include "HotKeyManager.h"
#include "DebugOutput.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::System;


namespace Croak::System::HotKeys
{
    HotKeyManager::~HotKeyManager()
    {
        // TODO: Take read-write lock.
        for (auto&& pair : hotKeys)
        {
            HotKey* hotKey = pair.second;
            delete hotKey;
        }
    }

    void HotKeyManager::RegisterHotKey(const VirtualKeyModifiers& modifiers, const uint32_t& virtualKey, const bool& isRepeating, const HotKeyId& agnosticId)
    {
        // TODO: Check arguments validity.
        try
        {
            HotKey* hotKey = new HotKey(modifiers, virtualKey, isRepeating);
            hotKey->Activate(); // throws

            hotKey->Fired([this, id = agnosticId](auto, auto)
            {
                e_hotKeyFired(id, nullptr);
            });

            hotKeys.insert({ agnosticId, hotKey});
        }
        catch (const std::invalid_argument&)
        {
            throw;
        }
    }

    bool HotKeyManager::EditKey(const HotKeyId& hotKeyId, const VirtualKeyModifiers& modifiers, const uint32_t& virtualKey)
    {
        if (hotKeys.find(hotKeyId) != hotKeys.cend())
        {
            //TODO: Don't reallocate memory, make the HotKey able to re-register itself with new parameters.
            HotKey* hotKey = hotKeys[hotKeyId];
            if (hotKey)
            {
                DebugLog(format("Replacing hot key (id: {0}).", hotKey->KeyId()));

                HotKey* newHotKey = new HotKey(modifiers, virtualKey, false);
                newHotKey->Activate();

                delete hotKey;

                newHotKey->Fired([this, id = hotKeyId](auto, auto)
                {
                    e_hotKeyFired(id, nullptr);
                });

                hotKeys[hotKeyId] = newHotKey;
            }
            return true;
        }
        return false;
    }

    void HotKeyManager::DeactivateKey(const::Croak::System::HotKeys::HotKeyId& agnosticId, const bool& state)
    {
        if (hotKeys.find(agnosticId) != hotKeys.cend())
        {
            hotKeys[agnosticId]->Enabled(state);
        }
    }

    void HotKeyManager::ReplaceOrInsertKey(HotKey* /*previousKey*/, HotKey* /*newKey*/)
    {
        // TODO: Implement.
    }

    map<HotKeyId, HotKeyView> HotKeyManager::ViewManagedKeys()
    {
        map<HotKeyId, HotKeyView> view{};
        for (auto&& pair : hotKeys)
        {
            view.insert({ pair.first, HotKeyView(pair.second, pair.first) });
        }
        return view;
    }
}