#pragma once
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include "HotKey.h"

namespace Croak::System::Hotkeys
{
	/**
	 * @brief Singleton class to manage hotkeys.
	*/
	class HotkeyManager
	{
	public:
		/**
		 * @brief Copy constructor.
		*/
		HotkeyManager(const HotkeyManager& other) = delete;
		~HotkeyManager();

		static HotkeyManager& GetHotKeyManager()
		{
			static HotkeyManager instance{};
			return instance;
		};

		winrt::guid RegisterHotKey(const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& virtualKey);
		void EditKey(const winrt::guid& hotKeyId, const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& virtualKey);
		void ReplaceOrInsertKey(Croak::System::Hotkeys::Hotkey* previousKey, Croak::System::Hotkeys::Hotkey* newKey);

		/**
		 * @brief Copy operator.
		*/
		HotkeyManager& operator=(const HotkeyManager& other) = delete;

	private:
		Concurrency::concurrent_unordered_map<winrt::guid, Croak::System::Hotkeys::Hotkey*> hotKeys{};

		winrt::Windows::Foundation::TypedEventHandler<winrt::guid, winrt::Windows::Foundation::IInspectable> e_hotKeyFired{};

		/**
		 * @brief Default constructor.
		*/
		HotkeyManager() = default;
	};
}

