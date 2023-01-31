#include "pch.h"
#include "HotKeyView.h"

namespace Croak::System::HotKeys
{
	HotKeyView::HotKeyView(const bool& enabled, const uint32_t& modifiers, const uint32_t& virtualKey, const uint32_t& agnosticId) :
		isEnabled{ enabled },
		modifiers{ modifiers },
		virtualKey{ virtualKey },
		agnosticId{ agnosticId }
	{
	}

	HotKeyView::HotKeyView(const HotKey* hotKey, const uint32_t& agnosticId) :
		HotKeyView(hotKey->Enabled(), hotKey->Modifiers(), hotKey->Key(), agnosticId)
	{
	}
}
