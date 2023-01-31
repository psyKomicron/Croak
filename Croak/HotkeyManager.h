#pragma once
#include <concurrent_unordered_map.h>
#include "HotKey.h"
#include "HotKeyView.h"

namespace Croak::System::HotKeys
{
    typedef uint32_t HotKeyId;

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

        inline winrt::event_token HotKeyFired(const winrt::Windows::Foundation::TypedEventHandler<HotKeyId, winrt::Windows::Foundation::IInspectable>& handler)
        {
            return e_hotKeyFired.add(handler);
        }

        inline void HotKeyFired(const winrt::event_token& token)
        {
            e_hotKeyFired.remove(token);
        }

        /**
         * @brief Registers a hot key, allowing it to be managed by the HotKeyManager.
         * @param modifiers Windows, Control, Alt, Shift
         * @param virtualKey Virtual key code (VK_[...])
         * @param isRepeating If set to true, the event will be repeating as long as the user presses the key combo.
         * @param agnosticId Identifier for the caller to recognize that hot key when fired.
        */
        void RegisterHotKey(const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& virtualKey, const bool& isRepeating, const HotKeyId& agnosticId);
        void EditKey(const ::Croak::System::HotKeys::HotKeyId& agnosticId, const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& virtualKey);
        void EditKey(const ::Croak::System::HotKeys::HotKeyId& agnosticId, const bool& state);
        void ReplaceOrInsertKey(::Croak::System::HotKeys::HotKey* previousKey, ::Croak::System::HotKeys::HotKey* newKey);
        std::map<HotKeyId, ::Croak::System::HotKeys::HotKeyView> GetManagedKeys();

        /**
         * @brief Copy operator.
        */
        HotKeyManager& operator=(const HotKeyManager& other) = delete;

    private:
        Concurrency::concurrent_unordered_map<HotKeyId, HotKeys::HotKey*> hotKeys{};
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<HotKeyId, winrt::Windows::Foundation::IInspectable>> e_hotKeyFired{};

        /**
         * @brief Default constructor.
        */
        HotKeyManager() = default;
    };
}

