#pragma once
#include "HotKey.h"

namespace Croak::System::HotKeys
{
	class HotKeyView
	{
	public:
		HotKeyView(const bool& enabled, const uint32_t& modifiers, const uint32_t& virtualKey, const uint32_t& agnosticId);
		HotKeyView(const HotKey* hotKey, const uint32_t& agnosticId);

		inline bool IsEnabled()
		{
			return isEnabled;
		}

		inline uint32_t Modifiers()
		{
			return modifiers;
		}

		inline uint32_t VirtualKey()
		{
			return virtualKey;
		}

		inline uint32_t Id()
		{
			return agnosticId;
		}

	private:
		bool isEnabled;
		uint32_t modifiers;
		uint32_t virtualKey;
		uint32_t agnosticId;
	};
}

