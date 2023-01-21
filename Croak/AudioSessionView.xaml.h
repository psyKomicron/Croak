#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "AudioSessionView.g.h"

namespace winrt::Croak::implementation
{
    struct AudioSessionView : AudioSessionViewT<AudioSessionView>
    {
    public:
        AudioSessionView();
        AudioSessionView(hstring const& header, double const& volume);
        AudioSessionView(hstring const& header, double const& volume, const hstring& logoPath);

        hstring Header();
        void Header(hstring const& value);
        guid Id();
        void Id(guid const& value);
        bool Muted();
        void Muted(bool const& value);
        double Volume();
        void Volume(double const& value);
        hstring VolumeGlyph();
        Microsoft::UI::Xaml::Controls::Orientation Orientation();
        void Orientation(const Microsoft::UI::Xaml::Controls::Orientation& value);
        Microsoft::UI::Xaml::Controls::ContentPresenter Logo();

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value);
        void PropertyChanged(event_token const& token);
        event_token VolumeChanged(Windows::Foundation::TypedEventHandler<winrt::Croak::AudioSessionView, Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs> const& handler);
        void VolumeChanged(event_token const& token);
        event_token VolumeStateChanged(Windows::Foundation::TypedEventHandler<winrt::Croak::AudioSessionView, bool> const& handler);
        void VolumeStateChanged(event_token const& token);
        event_token Hidden(Windows::Foundation::TypedEventHandler<winrt::Croak::AudioSessionView, Windows::Foundation::IInspectable> const& handler);
        void Hidden(event_token const& token);

        void SetState(const winrt::Croak::AudioSessionState& state);
        void SetPeak(float peak);
        void SetPeak(const float& peak1, const float& peak2);
        Windows::Foundation::IAsyncAction SetImageSource(IStream* stream);

        void Slider_ValueChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void MuteToggleButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void Grid_SizeChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void UserControl_Loaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void RootGrid_PointerEntered(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
        void RootGrid_PointerExited(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const&);
        void RootGrid_RightTapped(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e);
        void LockMenuFlyoutItem_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void HideMenuFlyoutItem_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RootGrid_ActualThemeChanged(Microsoft::UI::Xaml::FrameworkElement const& sender, Windows::Foundation::IInspectable const& args);
        Windows::Foundation::IAsyncAction UserControl_Loading(Microsoft::UI::Xaml::FrameworkElement const& sender, Windows::Foundation::IInspectable const& args);

    private:
        double _volume = 50.0;
        hstring _volumeGlyph = L"\ue994";
        hstring _header = L"";
        hstring audioSessionProcessPath = L"";
        guid _id{};
        bool _muted = false;
        float lastPeak = 0;
        bool active = false;
        bool isLocked = false;
        bool isVertical = true;

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;
        event<Windows::Foundation::TypedEventHandler<Croak::AudioSessionView, Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs>>
            e_volumeChanged;
        event< Windows::Foundation::TypedEventHandler<Croak::AudioSessionView, bool> > e_volumeStateChanged;
        event<Windows::Foundation::TypedEventHandler<Croak::AudioSessionView, Windows::Foundation::IInspectable>> e_hidden;

        void SetGlyph();
    };
}

namespace winrt::Croak::factory_implementation
{
    struct AudioSessionView : AudioSessionViewT<AudioSessionView, implementation::AudioSessionView>
    {
    };
}
