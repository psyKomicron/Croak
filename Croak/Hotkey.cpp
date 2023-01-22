#include "pch.h"
#include "HotKey.h"
#include "DebugOutput.h"

using namespace winrt::Windows::System;


namespace Croak::System::HotKeys
{
	std::atomic_int32_t HotKey::globalIds = 1;

	HotKey::HotKey(const VirtualKeyModifiers& modifiers, const uint32_t& key, const bool& isRepeating) :
		key{ key },
		repeating{ isRepeating }
	{
		hotKeyId = globalIds.fetch_add(1);

		if (static_cast<uint32_t>(modifiers & VirtualKeyModifiers::Control))
		{
			this->modifiers |= MOD_CONTROL;
		}
		if (static_cast<uint32_t>(modifiers & VirtualKeyModifiers::Menu))
		{
			this->modifiers |= MOD_ALT;

		}
		if (static_cast<uint32_t>(modifiers & VirtualKeyModifiers::Shift))
		{
			this->modifiers |= MOD_SHIFT;
		}
		if (static_cast<uint32_t>(modifiers & VirtualKeyModifiers::Windows))
		{
			this->modifiers |= MOD_WIN;
		}
	}

	HotKey::~HotKey()
	{
		if (notificationThread != nullptr)
		{
			PostThreadMessage(threadId, WM_QUIT, 0, 0); // Post quit message to the HotKey thread (GetMessage will return 0).
			notificationThread->join(); // Wait for the thread to exit.
			delete notificationThread; // Free the memory.

			DebugLog(std::format("Successfully free resources (key id: {0})", std::to_string(hotKeyId)));
		}
	}

	void HotKey::Activate()
	{
		std::atomic_flag startFlag{};

		notificationThread = new std::thread(&HotKey::ThreadFunction, this, &startFlag);

		startFlag.wait(false); // Wait to check if the HotKey was registered successfully or not.
		if (!startFlag.test()) // If the test returns false it means the HotKey registration has failed, throw exception.
		{
			throw std::invalid_argument("Hot key is not valid (rejected by the system).");
		}
	}


	void HotKey::ThreadFunction(std::atomic_flag* startFlag)
	{
		threadId = GetCurrentThreadId();

		if (!::RegisterHotKey(nullptr, hotKeyId, modifiers, key))
		{
			DebugLog(std::format("Hotkey (id: {0}) > Failed to register hot key.\n", std::to_string(hotKeyId)));
			//threadFlag.test_and_set();
			startFlag->notify_one();
		}
		else
		{
			DebugLog(std::format("Hotkey (id: {0}) > Registered.\n", std::to_string(hotKeyId)));
			startFlag->test_and_set();
			startFlag->notify_one();

			MSG message{};
			while (1)
			{
				//PeekMessage()
				if (GetMessage(&message, (HWND)(-1), 0, 0) & 1)
				{
					if (message.message == WM_HOTKEY)
					{
						DebugLog(std::format("Hotkey (id: {0}) > Hotkey pressed.\n", std::to_string(hotKeyId)));

						if (keyEnabled.load())
						{
							e_fired(winrt::Windows::Foundation::IInspectable(), winrt::guid());
						}
					}
				}
				else
				{
					DebugLog(std::format("Hotkey (id: {0}) > GetMessage returned 0.\n", std::to_string(hotKeyId)));
					break;
				}
			}

			// Unregister the HotKey when exiting the thread.
			UnregisterHotKey(nullptr, hotKeyId);

			DebugLog(std::format("Hotkey (id: {0}) > thread exiting.\n", std::to_string(hotKeyId)));
		}
	}
}