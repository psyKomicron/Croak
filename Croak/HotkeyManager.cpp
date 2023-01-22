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
			delete pair.second;
		}
	}

	HotKeyID HotKeyManager::RegisterHotKey(const VirtualKeyModifiers& modifiers, const uint32_t& virtualKey, const bool& isRepeating)
	{
		// TODO:
		// - Check arguments validity
		// - Activate and throw properly if the key has failed to activate.

		HotKey* hotKey = new HotKey(modifiers, virtualKey, isRepeating);

		hotKey->Activate();

		hotKey->Fired([this, id = hotKey->KeyId()](auto, auto)
		{
			e_hotKeyFired(id, nullptr);
		});

		hotKeys.insert({ hotKey->KeyId(), hotKey});
		return hotKey->KeyId();
	}

	void HotKeyManager::EditKey(const HotKeyID& hotKeyId, const VirtualKeyModifiers& modifiers, const uint32_t& virtualKey)
	{
		if (hotKeys.find(hotKeyId) != hotKeys.cend())
		{
			//TODO: Don't reallocate memory, make the HotKey able to re-register itself with new parameters.
			unique_ptr<HotKey> hotKeyPtr{ hotKeys[hotKeyId] };
			if (hotKeyPtr.get())
			{
				//DebugLog(format(L"Replacing hot key (id: {0}).", hotKey->Id()));
				HotKey* newHotKey = new HotKey(modifiers, virtualKey, false);
				newHotKey->Activate();

				newHotKey->Fired([this, id = newHotKey->KeyId()](auto, auto)
				{
					e_hotKeyFired(id, nullptr);
				});

				hotKeys[hotKeyId] = newHotKey;
			}
		}
	}

	void HotKeyManager::ReplaceOrInsertKey(HotKey* previousKey, HotKey* newKey)
	{
	}
}