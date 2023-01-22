#pragma once
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include "HotKey.h"

namespace Croak::System::HotKeys
{
	typedef uint32_t HotKeyID;

	/**
	 * @brief Singleton class to manage hotkeys.
	*/
	class HotKeyManager
	{
	public:
		/**
		 * @brief Copy constructor.
		*/
		HotKeyManager(const HotKeyManager& other) = delete;
		~HotKeyManager();

		static HotKeyManager& GetHotKeyManager()
		{
			static HotKeyManager instance{};
			return instance;
		}

		inline winrt::event_token HotKeyFired(const winrt::Windows::Foundation::TypedEventHandler<HotKeyID, winrt::Windows::Foundation::IInspectable>& handler)
		{
			return e_hotKeyFired.add(handler);
		}

		inline void HotKeyFired(const winrt::event_token& token)
		{
			e_hotKeyFired.remove(token);
		}

		::Croak::System::HotKeys::HotKeyID RegisterHotKey(const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& virtualKey, const bool& isRepeating);
		void EditKey(const ::Croak::System::HotKeys::HotKeyID& hotKeyId, const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& virtualKey);
		void ReplaceOrInsertKey(HotKeys::HotKey* previousKey, HotKeys::HotKey* newKey);

		/**
		 * @brief Copy operator.
		*/
		HotKeyManager& operator=(const HotKeyManager& other) = delete;

	private:
		Concurrency::concurrent_unordered_map<HotKeyID, HotKeys::HotKey*> hotKeys{};
		Concurrency::concurrent_unordered_map<HotKeyID, std::function<void()>> callbacks{};
		winrt::event<winrt::Windows::Foundation::TypedEventHandler<HotKeyID, winrt::Windows::Foundation::IInspectable>> e_hotKeyFired{};

		/**
		 * @brief Default constructor.
		*/
		HotKeyManager() = default;

		inline GUID CreateGuid();
	};
}

