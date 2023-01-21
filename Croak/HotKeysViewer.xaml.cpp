#include "pch.h"
#include "HotKeysViewer.xaml.h"
#if __has_include("HotKeysViewer.g.cpp")
#include "HotKeysViewer.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Input;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::System;


namespace winrt::Croak::implementation
{
    HotKeysViewer::HotKeysViewer()
    {
        InitializeComponent();
    }

    void HotKeysViewer::ShowMouseMap(const bool& value)
    {
        if (value)
        {
            MouseMapColumn().Width(winrt::Microsoft::UI::Xaml::GridLengthHelper::FromValueAndType(1, winrt::Microsoft::UI::Xaml::GridUnitType::Star));
        }
        else
        {
            MouseMapColumn().Width(winrt::Microsoft::UI::Xaml::GridLengthHelper::FromPixels(0));
        }

        MouseMapViewBox().Visibility(value ? winrt::Microsoft::UI::Xaml::Visibility::Visible : winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
    }


    void HotKeysViewer::SetActiveKeys(const IVector<VirtualKey>& activeKeys)
    {
        for (VirtualKey key : activeKeys)
        {
            switch (key)
            {
                case VirtualKey::LeftButton:
                    LeftMouseButton().Background(GetActiveBrush());
                    break;
                case VirtualKey::RightButton:
                    RightMouseButton().Background(GetActiveBrush());
                    break;
                case VirtualKey::Cancel:
                    break;
                case VirtualKey::MiddleButton:
                    MiddleMouseButton().Background(GetActiveBrush());
                    break;
                case VirtualKey::XButton1:
                    XMouseButton1().Background(GetActiveBrush());
                    break;
                case VirtualKey::XButton2:
                    XMouseButton2().Background(GetActiveBrush());
                    break;
                case VirtualKey::Back:
                    break;
                case VirtualKey::Tab:
                    TabKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Clear:
                    break;
                case VirtualKey::Enter:
                    EnterKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Shift:
                    LeftShiftKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Control:
                    LeftControlKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Menu:
                    LeftAltKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Pause:
                    break;
                case VirtualKey::CapitalLock:
                    CapsLockKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Escape:
                    EscKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Space:
                    SpaceKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::PageUp:
                    break;
                case VirtualKey::PageDown:
                    break;
                case VirtualKey::End:
                    break;
                case VirtualKey::Home:
                    break;
                case VirtualKey::Left:
                    LeftKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Up:
                    UpKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Right:
                    RightKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Down:
                    DownKey().Background(GetActiveBrush());
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
                    ZeroKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number1:
                    OneKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number2:
                    TwoKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number3:
                    ThreeKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number4:
                    FourKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number5:
                    FiveKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number6:
                    SixKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number7:
                    SevenKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number8:
                    EightKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Number9:
                    NineKey().Background(GetActiveBrush());
                    break;

                case VirtualKey::A:
                    AKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::B:
                    BKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::C:
                    CKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::D:
                    DKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::E:
                    EKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::F:
                    FKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::G:
                    GKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::H:
                    HKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::I:
                    IKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::J:
                    JKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::K:
                    KKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::L:
                    LKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::M:
                    MKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::N:
                    NKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::O:
                    OKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::P:
                    PKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Q:
                    QKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::R:
                    RKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::S:
                    SKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::T:
                    TKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::U:
                    UKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::V:
                    VKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::W:
                    WKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::X:
                    XKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Y:
                    YKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::Z:
                    ZKey().Background(GetActiveBrush());
                    break;

                case VirtualKey::LeftWindows:
                    LeftWindowsKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::RightWindows:
                    RightWindowsKey().Background(GetActiveBrush());
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
                    F1Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F2:
                    F2Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F3:
                    F3Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F4:
                    F4Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F5:
                    F5Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F6:
                    F6Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F7:
                    F7Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F8:
                    F8Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F9:
                    F9Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F10:
                    F10Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F11:
                    F11Key().Background(GetActiveBrush());
                    break;
                case VirtualKey::F12:
                    F12Key().Background(GetActiveBrush());
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
                    LeftShiftKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::RightShift:
                    RightShiftKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::LeftControl:
                    LeftControlKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::RightControl:
                    RightControlKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::LeftMenu:
                    LeftAltKey().Background(GetActiveBrush());
                    break;
                case VirtualKey::RightMenu:
                    RightAltKey().Background(GetActiveBrush());
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
            }
        }
    }

    void HotKeysViewer::AddActiveKey(const winrt::Croak::HotkeyViewModel& hotKeyView)
    {
        hotKeys.push_back(hotKeyView);
        DependencyObject tooltipHolder{ nullptr };

        if (static_cast<uint32_t>(hotKeyView.Modifiers() & VirtualKeyModifiers::Control))
        {
            LeftControlKey().Background(GetActiveBrush());
        }
        if (static_cast<uint32_t>(hotKeyView.Modifiers() & VirtualKeyModifiers::Menu))
        {
            LeftAltKey().Background(GetActiveBrush());
        }
        if (static_cast<uint32_t>(hotKeyView.Modifiers() & VirtualKeyModifiers::Shift))
        {
            LeftShiftKey().Background(GetActiveBrush());
        }
        if (static_cast<uint32_t>(hotKeyView.Modifiers() & VirtualKeyModifiers::Windows))
        {
            LeftWindowsKey().Background(GetActiveBrush());
        }

        switch (hotKeyView.Key())
        {
            case VirtualKey::LeftButton:
                LeftMouseButton().Background(GetActiveBrush());
                tooltipHolder = LeftMouseButton();
                break;

            case VirtualKey::RightButton:
                RightMouseButton().Background(GetActiveBrush());
                tooltipHolder = RightMouseButton();
                break;

            case VirtualKey::Cancel:
                break;

            case VirtualKey::MiddleButton:
                MiddleMouseButton().Background(GetActiveBrush());
                tooltipHolder = MiddleMouseButton();
                break;

            case VirtualKey::XButton1:
                XMouseButton1().Background(GetActiveBrush());
                tooltipHolder = XMouseButton1();
                break;

            case VirtualKey::XButton2:
                XMouseButton2().Background(GetActiveBrush());
                tooltipHolder = XMouseButton2();
                break;

            case VirtualKey::Back:
                BackspaceKey().Background(GetActiveBrush());
                tooltipHolder = BackspaceKey();
                break;

            case VirtualKey::Tab:
                TabKey().Background(GetActiveBrush());
                tooltipHolder = TabKey();
                break;

            case VirtualKey::Clear:
                // TODO: Clear key
                break;

            case VirtualKey::Enter:
                EnterKey().Background(GetActiveBrush());
                tooltipHolder = EnterKey();
                break;

            case VirtualKey::Shift:
                LeftShiftKey().Background(GetActiveBrush());
                tooltipHolder = LeftShiftKey();
                break;

            case VirtualKey::Control:
                LeftControlKey().Background(GetActiveBrush());
                tooltipHolder = LeftControlKey();
                break;

            case VirtualKey::Menu:
                LeftAltKey().Background(GetActiveBrush());
                tooltipHolder = LeftAltKey();
                break;

            case VirtualKey::Pause:
                break;

            case VirtualKey::CapitalLock:
                CapsLockKey().Background(GetActiveBrush());
                tooltipHolder = CapsLockKey();
                break;

            case VirtualKey::Escape:
                EscKey().Background(GetActiveBrush());
                tooltipHolder = EscKey();
                break;

            case VirtualKey::Space:
                SpaceKey().Background(GetActiveBrush());
                tooltipHolder = SpaceKey();
                break;

            case VirtualKey::PageUp:
                // TODO: PageUp key
                break;
            case VirtualKey::PageDown:
                // TODO: PageDown key
                break;
            case VirtualKey::End:
                // TODO: End key
                break;
            case VirtualKey::Home:
                // TODO: Home key
                break;

            case VirtualKey::Left:
                LeftKey().Background(GetActiveBrush());
                tooltipHolder = LeftKey();
                break;

            case VirtualKey::Up:
                UpKey().Background(GetActiveBrush());
                tooltipHolder = UpKey();
                break;

            case VirtualKey::Right:
                RightKey().Background(GetActiveBrush());
                tooltipHolder = RightKey();
                break;

            case VirtualKey::Down:
                DownKey().Background(GetActiveBrush());
                tooltipHolder = DownKey();
                break;

            case VirtualKey::Select:
                // TODO: Select key.
                break;
            case VirtualKey::Print:
                // TODO: Print key.
                break;
            case VirtualKey::Execute:
                // TODO: Execute key.
                break;
            case VirtualKey::Snapshot:
                break;
            case VirtualKey::Insert:
                // TODO: Insert key.
                break;
            case VirtualKey::Delete:
                // TODO: Delete key.
                break;
            case VirtualKey::Help:
                // TODO: Help key.
                break;

            case VirtualKey::Number0:
                ZeroKey().Background(GetActiveBrush());
                tooltipHolder = ZeroKey();
                break;
            case VirtualKey::Number1:
                OneKey().Background(GetActiveBrush());
                tooltipHolder = OneKey();
                break;
            case VirtualKey::Number2:
                TwoKey().Background(GetActiveBrush());
                tooltipHolder = TwoKey();
                break;
            case VirtualKey::Number3:
                ThreeKey().Background(GetActiveBrush());
                tooltipHolder = ThreeKey();
                break;
            case VirtualKey::Number4:
                FourKey().Background(GetActiveBrush());
                tooltipHolder = FourKey();
                break;
            case VirtualKey::Number5:
                FiveKey().Background(GetActiveBrush());
                tooltipHolder = FiveKey();
                break;
            case VirtualKey::Number6:
                SixKey().Background(GetActiveBrush());
                tooltipHolder = DownKey();
                break;
            case VirtualKey::Number7:
                SevenKey().Background(GetActiveBrush());
                tooltipHolder = SevenKey();
                break;
            case VirtualKey::Number8:
                EightKey().Background(GetActiveBrush());
                tooltipHolder = EightKey();
                break;
            case VirtualKey::Number9:
                NineKey().Background(GetActiveBrush());
                tooltipHolder = NineKey();
                break;

            case VirtualKey::A:
                AKey().Background(GetActiveBrush());
                tooltipHolder = AKey();
                break;
            case VirtualKey::B:
                BKey().Background(GetActiveBrush());
                tooltipHolder = BKey();
                break;
            case VirtualKey::C:
                CKey().Background(GetActiveBrush());
                tooltipHolder = CKey();
                break;
            case VirtualKey::D:
                DKey().Background(GetActiveBrush());
                tooltipHolder = DKey();
                break;
            case VirtualKey::E:
                EKey().Background(GetActiveBrush());
                tooltipHolder = EKey();
                break;
            case VirtualKey::F:
                FKey().Background(GetActiveBrush());
                tooltipHolder = FKey();
                break;
            case VirtualKey::G:
                GKey().Background(GetActiveBrush());
                tooltipHolder = GKey();
                break;
            case VirtualKey::H:
                HKey().Background(GetActiveBrush());
                tooltipHolder = HKey();
                break;
            case VirtualKey::I:
                IKey().Background(GetActiveBrush());
                tooltipHolder = IKey();
                break;
            case VirtualKey::J:
                JKey().Background(GetActiveBrush());
                tooltipHolder = JKey();
                break;
            case VirtualKey::K:
                KKey().Background(GetActiveBrush());
                tooltipHolder = KKey();
                break;
            case VirtualKey::L:
                LKey().Background(GetActiveBrush());
                tooltipHolder = LKey();
                break;
            case VirtualKey::M:
                MKey().Background(GetActiveBrush());
                tooltipHolder = MKey();
                break;
            case VirtualKey::N:
                NKey().Background(GetActiveBrush());
                tooltipHolder = NKey();
                break;
            case VirtualKey::O:
                OKey().Background(GetActiveBrush());
                tooltipHolder = OKey();
                break;
            case VirtualKey::P:
                PKey().Background(GetActiveBrush());
                tooltipHolder = PKey();
                break;
            case VirtualKey::Q:
                QKey().Background(GetActiveBrush());
                tooltipHolder = QKey();
                break;
            case VirtualKey::R:
                RKey().Background(GetActiveBrush());
                tooltipHolder = RKey();
                break;
            case VirtualKey::S:
                SKey().Background(GetActiveBrush());
                tooltipHolder = SKey();
                break;
            case VirtualKey::T:
                TKey().Background(GetActiveBrush());
                tooltipHolder = TKey();
                break;
            case VirtualKey::U:
                UKey().Background(GetActiveBrush());
                tooltipHolder = UKey();
                break;
            case VirtualKey::V:
                VKey().Background(GetActiveBrush());
                tooltipHolder = VKey();
                break;
            case VirtualKey::W:
                WKey().Background(GetActiveBrush());
                tooltipHolder = WKey();
                break;
            case VirtualKey::X:
                XKey().Background(GetActiveBrush());
                tooltipHolder = XKey();
                break;
            case VirtualKey::Y:
                YKey().Background(GetActiveBrush());
                tooltipHolder = YKey();
                break;
            case VirtualKey::Z:
                ZKey().Background(GetActiveBrush());
                tooltipHolder = ZKey();
                break;

            case VirtualKey::LeftWindows:
                LeftWindowsKey().Background(GetActiveBrush());
                tooltipHolder = LeftWindowsKey();
                break;
            case VirtualKey::RightWindows:
                RightWindowsKey().Background(GetActiveBrush());
                //tooltipHolder = ();
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
                F1Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F2:
                F2Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F3:
                F3Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F4:
                F4Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F5:
                F5Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F6:
                F6Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F7:
                F7Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F8:
                F8Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F9:
                F9Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F10:
                F10Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F11:
                F11Key().Background(GetActiveBrush());
                break;
            case VirtualKey::F12:
                F12Key().Background(GetActiveBrush());
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
                LeftShiftKey().Background(GetActiveBrush());
                break;
            case VirtualKey::RightShift:
                RightShiftKey().Background(GetActiveBrush());
                break;
            case VirtualKey::LeftControl:
                LeftControlKey().Background(GetActiveBrush());
                break;
            case VirtualKey::RightControl:
                RightControlKey().Background(GetActiveBrush());
                break;
            case VirtualKey::LeftMenu:
                LeftAltKey().Background(GetActiveBrush());
                break;
            case VirtualKey::RightMenu:
                RightAltKey().Background(GetActiveBrush());
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
        }

        if (tooltipHolder)
        {
            ToolTipService::SetToolTip(tooltipHolder, box_value(hotKeyView.HotKeyName()));
        }
    }


    Brush HotKeysViewer::GetActiveBrush()
    {
        return Application::Current().Resources().Lookup(box_value(L"AccentFillColorDefaultBrush")).as<Brush>();
    }
}
