#pragma once
#include <atomic>

namespace Croak::System::HotKeys
{
    class HotKey
    {
    public:
        HotKey() = delete;
        HotKey(const winrt::Windows::System::VirtualKeyModifiers& modifiers, const uint32_t& key, const bool& isRepeating);
        ~HotKey();

        inline uint32_t Modifiers() const
        {
            return modifiers;
        }

        /**
         * @brief Key.
         * @return The VK_[key name] associated with this hotkey
        */
        inline uint32_t Key() const
        {
            return key;
        }

        /**
         * @brief If the key is not enabled, Fired event will not be fired when the hotkey is pressed.
         * @return true if the hotkey is enabled
        */
        inline bool Enabled() const
        {
            return keyEnabled.load();
        }

        /**
         * @brief If the key is not enabled, Fired event will not be fired when the hotkey is pressed.
         * @param enabled set to true to disable events
        */
        inline void Enabled(const bool& enabled)
        {
            keyEnabled.exchange(enabled);
        }

        /**
         * @brief Unique identifier for that key.
         * @return 
        */
        inline uint32_t KeyId() const
        {
            return hotKeyId;
        }


        /**
         * @brief Fired event subscriber.
         * @param handler callback
         * @return Event token
        */
        inline winrt::event_token Fired(const winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::guid>& handler)
        {
            return e_fired.add(handler);
        }

        /**
         * @brief Fired event revoker.
         * @param token 
        */
        inline void Fired(const winrt::event_token& token)
        {
            e_fired.remove(token);
        }

        /**
         * @brief Activates the key to fire events.
        */
        void Activate();

    private:
        static std::atomic_int32_t globalIds;

        uint32_t key;
        bool repeating;
        uint32_t modifiers = 0;
        uint32_t hotKeyId = 0;
        std::thread* notificationThread = nullptr;
        std::atomic_bool threadRunning = false;
        std::atomic_bool keyEnabled = true;
        DWORD threadId = 0ul;

        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::guid>> e_fired {};

        bool CheckParametersValidity(const DWORD& modifiers, const DWORD& key);
        void ThreadFunction(std::atomic_flag* startFlag, bool* failed);
    };
}

