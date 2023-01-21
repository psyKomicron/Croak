#include "pch.h"
#include "HotkeyManager.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::System;


namespace Croak::System::Hotkeys
{
	HotkeyManager::~HotkeyManager()
	{
		// TODO: Take read-write lock.
		for (auto&& pair : hotKeys)
		{
			delete pair.second;
		}
	}

	guid HotkeyManager::RegisterHotKey(const VirtualKeyModifiers& modifiers, const uint32_t& virtualKey)
	{
		// TODO:
		// - Check arguments validity
		// - Activate and throw properly if the key has failed to activate.
		// - Return an ID so that the caller can identify witch key has been fired.
		// - Handle Fired events and send dispatch them.
		UUID hotKeyId{};
		if (SUCCEEDED((UuidCreate(&hotKeyId))))
		{
			Hotkey* hotKey = new Hotkey(modifiers, virtualKey);
			hotKey->Fired([this, id = guid(hotKeyId)](auto, auto)
			{
				e_hotKeyFired(id, nullptr);
			});

			hotKeys.insert({ guid(hotKeyId), hotKey });
			return guid(move(hotKeyId));
		}
		else
		{
			// TODO: Throw exception.
		}
	}

	void HotkeyManager::EditKey(const winrt::guid& hotKeyId, const VirtualKeyModifiers& modifiers, const uint32_t& virtualKey)
	{
		Hotkey* hotKeyPtr = hotKeys.at(hotKeyId);
		if (!hotKeyPtr)
		{
			throw exception("Hot key was null.");
		}
		delete hotKeyPtr;

		hotKeyPtr = new Hotkey(modifiers, virtualKey);
		hotKeyPtr->Fired([this, id = guid(hotKeyId)](auto, auto)
		{
			e_hotKeyFired(id, nullptr);
		});

		hotKeys.insert({ guid(hotKeyId), hotKeyPtr });
	}

	void HotkeyManager::ReplaceOrInsertKey(Hotkey* previousKey, Hotkey* newKey)
	{
		for (auto&& pair : hotKeys)
		{
			//VirtualKeyModifiers modifiers = previousKey->KeyModifiers();
			//VirtualKeyModifiers currentModifiers = pair.second->KeyModifiers();

			if ((static_cast<uint32_t>(previousKey->KeyModifiers()) == static_cast<uint32_t>(pair.second->KeyModifiers())) &&
				(pair.second->Key() == previousKey->Key()))
			{
				hotKeys.insert({ pair.first, newKey }); // Could break iterator.
				break;
			}
		}
	}
}