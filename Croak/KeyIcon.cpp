#include "pch.h"
#include "KeyIcon.h"
#if __has_include("KeyIcon.g.cpp")
#include "KeyIcon.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Windows::System;


namespace winrt::Croak::implementation
{
    KeyIcon::KeyIcon()
    {
        DefaultStyleKey(winrt::box_value(L"Croak.KeyIcon"));
    }

    KeyIcon::KeyIcon(const winrt::Windows::System::VirtualKey& key)
    {
        hstring defaultStyleKey{};
        switch (key)
        {
            case VirtualKey::None:
                break;
            case VirtualKey::LeftButton:
                defaultStyleKey = L"LeftMouseButtonIcon";
                break;
            case VirtualKey::RightButton:
                defaultStyleKey = L"RightButtonIcon";
                break;
            case VirtualKey::Cancel:
                defaultStyleKey = L"CancelKeyIcon";
                break;
            case VirtualKey::MiddleButton:
                defaultStyleKey = L"MiddleButtonIcon";
                break;
            case VirtualKey::XButton1:
                defaultStyleKey = L"XButton1Icon";
                break;
            case VirtualKey::XButton2:
                defaultStyleKey = L"XButton2Icon";
                break;
            case VirtualKey::Back:
                defaultStyleKey = L"BackKeyIcon";
                break;
            case VirtualKey::Tab:
                defaultStyleKey = L"TabKeyIcon";
                break;
            case VirtualKey::Clear:
                defaultStyleKey = L"ClearKeyIcon";
                break;
            case VirtualKey::Enter:
                defaultStyleKey = L"EnterKeyIcon";
                break;
            case VirtualKey::Shift:
                defaultStyleKey = L"ShiftKeyIcon";
                break;
            case VirtualKey::Control:
                defaultStyleKey = L"ControlKeyIcon";
                break;
            case VirtualKey::Menu:
                defaultStyleKey = L"MenuKeyIcon";
                break;
            case VirtualKey::Pause:
                defaultStyleKey = L"PauseKeyIcon";
                break;
            case VirtualKey::CapitalLock:
                defaultStyleKey = L"CapsLockKeyIcon";
                break;
            case VirtualKey::Escape:
                defaultStyleKey = L"EscapeKeyIcon";
                break;
            case VirtualKey::Space:
                defaultStyleKey = L"SpaceKeyIcon";
                break;
            case VirtualKey::PageUp:
                defaultStyleKey = L"PageUpKeyIcon";
                break;
            case VirtualKey::PageDown:
                defaultStyleKey = L"PageDownKeyIcon";
                break;
            case VirtualKey::End:
                defaultStyleKey = L"EndKeyIcon";
                break;
            case VirtualKey::Home:
                defaultStyleKey = L"HomeKeyIcon";
                break;
            case VirtualKey::Left:
                defaultStyleKey = L"LeftKeyIcon";
                break;
            case VirtualKey::Up:
                defaultStyleKey = L"UpKeyIcon";
                break;
            case VirtualKey::Right:
                defaultStyleKey = L"RightKeyIcon";
                break;
            case VirtualKey::Down:
                defaultStyleKey = L"DownKeyIcon";
                break;
            case VirtualKey::Select:
                break;
            case VirtualKey::Print:
                break;
            case VirtualKey::Execute:
                break;
            case VirtualKey::Snapshot:
                break;
            case VirtualKey::Insert:
                break;
            case VirtualKey::Delete:
                break;
            case VirtualKey::Help:
                break;
            case VirtualKey::Number0:
                break;
            case VirtualKey::Number1:
                break;
            case VirtualKey::Number2:
                break;
            case VirtualKey::Number3:
                break;
            case VirtualKey::Number4:
                break;
            case VirtualKey::Number5:
                break;
            case VirtualKey::Number6:
                break;
            case VirtualKey::Number7:
                break;
            case VirtualKey::Number8:
                break;
            case VirtualKey::Number9:
                break;

            case VirtualKey::A:
            case VirtualKey::B:
            case VirtualKey::C:
            case VirtualKey::D:
            case VirtualKey::E:
            case VirtualKey::F:
            case VirtualKey::G:
            case VirtualKey::H:
            case VirtualKey::I:
            case VirtualKey::J:
            case VirtualKey::K:
            case VirtualKey::L:
            case VirtualKey::M:
            case VirtualKey::N:
            case VirtualKey::O:
            case VirtualKey::P:
            case VirtualKey::Q:
            case VirtualKey::R:
            case VirtualKey::S:
            case VirtualKey::T:
            case VirtualKey::U:
            case VirtualKey::V:
            case VirtualKey::W:
            case VirtualKey::X:
            case VirtualKey::Y:
            case VirtualKey::Z:
                break;

            case VirtualKey::LeftWindows:
                break;
            case VirtualKey::RightWindows:
                break;
            case VirtualKey::Application:
                break;
            case VirtualKey::Sleep:
                break;
            case VirtualKey::NumberPad0:
                break;
            case VirtualKey::NumberPad1:
                break;
            case VirtualKey::NumberPad2:
                break;
            case VirtualKey::NumberPad3:
                break;
            case VirtualKey::NumberPad4:
                break;
            case VirtualKey::NumberPad5:
                break;
            case VirtualKey::NumberPad6:
                break;
            case VirtualKey::NumberPad7:
                break;
            case VirtualKey::NumberPad8:
                break;
            case VirtualKey::NumberPad9:
                break;
            case VirtualKey::Multiply:
                break;
            case VirtualKey::Add:
                break;
            case VirtualKey::Separator:
                break;
            case VirtualKey::Subtract:
                break;
            case VirtualKey::Decimal:
                break;
            case VirtualKey::Divide:
                break;
            case VirtualKey::F1:
                break;
            case VirtualKey::F2:
                break;
            case VirtualKey::F3:
                break;
            case VirtualKey::F4:
                break;
            case VirtualKey::F5:
                break;
            case VirtualKey::F6:
                break;
            case VirtualKey::F7:
                break;
            case VirtualKey::F8:
                break;
            case VirtualKey::F9:
                break;
            case VirtualKey::F10:
                break;
            case VirtualKey::F11:
                break;
            case VirtualKey::F12:
                break;
            case VirtualKey::F13:
                break;
            case VirtualKey::F14:
                break;
            case VirtualKey::F15:
                break;
            case VirtualKey::F16:
                break;
            case VirtualKey::F17:
                break;
            case VirtualKey::F18:
                break;
            case VirtualKey::F19:
                break;
            case VirtualKey::F20:
                break;
            case VirtualKey::F21:
                break;
            case VirtualKey::F22:
                break;
            case VirtualKey::F23:
                break;
            case VirtualKey::F24:
                break;
            case VirtualKey::NavigationView:
                break;
            case VirtualKey::NavigationMenu:
                break;
            case VirtualKey::NavigationUp:
                break;
            case VirtualKey::NavigationDown:
                break;
            case VirtualKey::NavigationLeft:
                break;
            case VirtualKey::NavigationRight:
                break;
            case VirtualKey::NavigationAccept:
                break;
            case VirtualKey::NavigationCancel:
                break;
            case VirtualKey::NumberKeyLock:
                break;
            case VirtualKey::Scroll:
                break;
            case VirtualKey::LeftShift:
                break;
            case VirtualKey::RightShift:
                break;
            case VirtualKey::LeftControl:
                break;
            case VirtualKey::RightControl:
                break;
            case VirtualKey::LeftMenu:
                break;
            case VirtualKey::RightMenu:
                break;
            case VirtualKey::GoBack:
                break;
            case VirtualKey::GoForward:
                break;
            case VirtualKey::Refresh:
                break;
            case VirtualKey::Stop:
                break;
            case VirtualKey::Search:
                break;
            case VirtualKey::Favorites:
                break;
            case VirtualKey::GoHome:
                break;
            case VirtualKey::GamepadA:
                break;
            case VirtualKey::GamepadB:
                break;
            case VirtualKey::GamepadX:
                break;
            case VirtualKey::GamepadY:
                break;
            case VirtualKey::GamepadRightShoulder:
                break;
            case VirtualKey::GamepadLeftShoulder:
                break;
            case VirtualKey::GamepadLeftTrigger:
                break;
            case VirtualKey::GamepadRightTrigger:
                break;
            case VirtualKey::GamepadDPadUp:
                break;
            case VirtualKey::GamepadDPadDown:
                break;
            case VirtualKey::GamepadDPadLeft:
                break;
            case VirtualKey::GamepadDPadRight:
                break;
            case VirtualKey::GamepadMenu:
                break;
            case VirtualKey::GamepadView:
                break;
            case VirtualKey::GamepadLeftThumbstickButton:
                break;
            case VirtualKey::GamepadRightThumbstickButton:
                break;
            case VirtualKey::GamepadLeftThumbstickUp:
                break;
            case VirtualKey::GamepadLeftThumbstickDown:
                break;
            case VirtualKey::GamepadLeftThumbstickRight:
                break;
            case VirtualKey::GamepadLeftThumbstickLeft:
                break;
            case VirtualKey::GamepadRightThumbstickUp:
                break;
            case VirtualKey::GamepadRightThumbstickDown:
                break;
            case VirtualKey::GamepadRightThumbstickRight:
                break;
            case VirtualKey::GamepadRightThumbstickLeft:
                break;
            default:
                break;
        }


        if (!defaultStyleKey.empty())
        {
            Style(
                Application::Current().Resources().Lookup(box_value(defaultStyleKey)).as<winrt::Microsoft::UI::Xaml::Style>()
            );
        }
    }
}
