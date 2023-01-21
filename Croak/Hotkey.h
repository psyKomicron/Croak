#pragma once
#include <atomic>

namespace Croak::System::Hotkeys
{
	class Hotkey
	{
	public:
		Hotkey() = delete;
		Hotkey(const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& key);
		~Hotkey();

		/**
		 * @brief Key modifiers (control, alt, etc...).
		 * @return Virtual key modifiers
		*/
		inline winrt::Windows::System::VirtualKeyModifiers KeyModifiers()
		{
			return TranslateModifiers(modifiers);
		};

		/**
		 * @brief Key.
		 * @return The VK_[key name] associated with this hotkey
		*/
		inline uint32_t Key()
		{
			return key;
		};

		/**
		 * @brief If the key is not enabled, Fired event will not be fired when the hotkey is pressed.
		 * @return true if the hotkey is enabled
		*/
		inline bool Enabled()
		{
			return keyEnabled.load();
		};

		/**
		 * @brief If the key is not enabled, Fired event will not be fired when the hotkey is pressed.
		 * @param enabled set to true to disable events
		*/
		inline void Enabled(const bool& enabled)
		{
			keyEnabled.exchange(enabled);
		};

		inline winrt::event_token Fired(const winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::guid>& handler)
		{
			return e_fired.add(handler);
		};
		inline void Fired(const winrt::event_token& token)
		{
			e_fired.remove(token);
		};

		/**
		 * @brief Activates the key to fire events.
		*/
		void Activate();

	private:
		static std::atomic_int32_t id;

		uint32_t modifiers = 0u;
		uint32_t key = 0u;
		std::atomic_flag threadFlag{};
		int32_t hotkeyId = 0;
		std::thread* notificationThread = nullptr;
		std::atomic_bool threadRunning = false;
		std::atomic_bool keyEnabled = true;
		DWORD threadId = 0ul;

		winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::guid>> e_fired {};

		winrt::Windows::System::VirtualKeyModifiers TranslateModifiers(const uint32_t& win32Mods) const;
		void ThreadFunction();
	};
}

