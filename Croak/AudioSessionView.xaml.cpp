#include "pch.h"
#include "AudioSessionView.xaml.h"
#if __has_include("AudioSessionView.g.cpp")
#include "AudioSessionView.g.cpp"
#endif

#include <math.h>
#include <limits>
#include "IconHelper.h"
#include "DebugOutput.h"

namespace Foundation = winrt::Windows::Foundation;
namespace UI = winrt::Windows::UI;
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
    using namespace winrt::Microsoft::UI::Xaml::Input;
    using namespace winrt::Microsoft::UI::Xaml::Media;
    using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
}


namespace winrt::Croak::implementation
{
    AudioSessionView::AudioSessionView()
    {
        InitializeComponent();
    }

    AudioSessionView::AudioSessionView(winrt::hstring const& header, double const& volume) : AudioSessionView()
    {
        _header = header;
        _volume = round(volume);
        SetGlyph();
    }

    AudioSessionView::AudioSessionView(winrt::hstring const& header, double const& volume, const winrt::hstring& logoPath) :
        AudioSessionView(header, volume)
    {
        audioSessionProcessPath = hstring(logoPath);
    }


#pragma region Properties
    winrt::hstring AudioSessionView::VolumeGlyph()
    {
        return _volumeGlyph;
    }

    Xaml::Orientation AudioSessionView::Orientation()
    {
        return isVertical ? Xaml::Orientation::Vertical : Xaml::Orientation::Horizontal;
    }

    void AudioSessionView::Orientation(const Xaml::Orientation& value)
    {
        isVertical = value == Xaml::Orientation::Vertical;
        Xaml::VisualStateManager::GoToState(*this, isVertical ? L"VerticalLayout" : L"HorizontalLayout", false);
    }

    winrt::Microsoft::UI::Xaml::Controls::ContentPresenter AudioSessionView::Logo()
    {
        return AudioSessionAppLogo();
    }

    double AudioSessionView::Volume()
    {
        return _volume;
    }

    void AudioSessionView::Volume(double const& value)
    {
        if (value >= 0.0 && value <= 100.0)
        {
            _volume = value;
            e_propertyChanged(*this, PropertyChangedEventArgs(L"Volume"));
        }
    }

    hstring AudioSessionView::Header()
    {
        return _header;
    }

    void AudioSessionView::Header(hstring const& value)
    {
        _header = value;
        e_propertyChanged(*this, PropertyChangedEventArgs(L"Header"));
    }

    guid AudioSessionView::Id()
    {
        return _id;
    }

    void AudioSessionView::Id(guid const& value)
    {
        _id = value;
        //e_propertyChanged(*this, PropertyChangedEventArgs(L"Id"));
    }

    bool AudioSessionView::Muted()
    {
        return _muted;
    }

    void AudioSessionView::Muted(bool const& value)
    {
        _muted = value;
        e_propertyChanged(*this, PropertyChangedEventArgs(L"Muted"));

        if (_muted)
        {
            _volumeGlyph = L"\ue74f";
        }
        else
        {
            SetGlyph();
        }

        e_propertyChanged(*this, PropertyChangedEventArgs(L"VolumeGlyph"));
    }
#pragma endregion


#pragma region Events
    winrt::event_token AudioSessionView::PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
    {
        return e_propertyChanged.add(value);
    }

    void AudioSessionView::PropertyChanged(winrt::event_token const& token)
    {
        e_propertyChanged.remove(token);
    }

    winrt::event_token AudioSessionView::VolumeChanged(Foundation::TypedEventHandler<winrt::Croak::AudioSessionView, Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs> const& handler)
    {
        return e_volumeChanged.add(handler);
    }

    void AudioSessionView::VolumeChanged(winrt::event_token const& token)
    {
        e_volumeChanged.remove(token);
    }

    winrt::event_token AudioSessionView::VolumeStateChanged(Foundation::TypedEventHandler<winrt::Croak::AudioSessionView, bool> const& handler)
    {
        return e_volumeStateChanged.add(handler);
    }

    void AudioSessionView::VolumeStateChanged(winrt::event_token const& token)
    {
        e_volumeStateChanged.remove(token);
    }

    winrt::event_token AudioSessionView::Hidden(Foundation::TypedEventHandler<winrt::Croak::AudioSessionView, Foundation::IInspectable> const& handler)
    {
        return e_hidden.add(handler);
    }

    void AudioSessionView::Hidden(winrt::event_token const& token)
    {
        e_hidden.remove(token);
    }
#pragma endregion


    void AudioSessionView::SetState(const AudioSessionState& state)
    {
        auto resources = Xaml::Application::Current().Resources();
        switch (state)
        {
            case AudioSessionState::Active:
                active = true;
                VolumeFontIcon().Foreground(Xaml::SolidColorBrush(Windows::UI::Colors::LimeGreen()));
                VolumeFontIcon().Opacity(1);
                break;

            case AudioSessionState::Expired:
            case AudioSessionState::Inactive:
            default:
                active = false;
                /*VolumeFontIcon().Foreground(
                     RootGrid().ActualTheme() == ElementTheme::Dark ?
                        ::Media::SolidColorBrush(Windows::UI::Colors::WhiteSmoke()) :
                        ::Media::SolidColorBrush(Windows::UI::Colors::DarkGray())
                );*/

                VolumeFontIcon().Foreground(
                    Xaml::Application::Current().Resources().Lookup(box_value(L"TextFillColorPrimaryBrush")).as<Xaml::Brush>()
                );
                VolumeFontIcon().Opacity(0.6);
                break;
        }
    }

    void AudioSessionView::SetPeak(float peak)
    {
        SetPeak(peak, peak);
    }

    void AudioSessionView::SetPeak(const float& left, const float& right)
    {
        float isVerticalFloat = static_cast<float>(isVertical);
        LeftPeakAnimation().To(static_cast<double>(left * isVerticalFloat));
        RightPeakAnimation().To(static_cast<double>(right * isVerticalFloat));

        TopVolumeAnimation().To(static_cast<double>(left * !isVerticalFloat));
        BottomVolumeAnimation().To(static_cast<double>(right * !isVerticalFloat));

        if (isVertical)
        {
            VerticalPeakStoryboard().Begin();
        }
        else
        {
            HorizontalPeakStoryboard().Begin();
        }
    }

    Windows::Foundation::IAsyncAction AudioSessionView::SetImageSource(IStream* stream)
    {
        co_return;
    }


    void AudioSessionView::UserControl_Loaded(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        Grid_SizeChanged(nullptr, nullptr);

        if (AudioSessionAppLogo().Content() != nullptr)
        {
            Xaml::VisualStateManager::GoToState(*this, L"UsingLogo", true);
        }
    }

    void AudioSessionView::Slider_ValueChanged(IInspectable const&, Xaml::RangeBaseValueChangedEventArgs const& e)
    {
        _volume = e.NewValue();

        if (!_muted)
        {
            SetGlyph();
            e_propertyChanged(*this, PropertyChangedEventArgs(L"VolumeGlyph"));
        }
        e_propertyChanged(*this, PropertyChangedEventArgs(L"Volume"));
        e_volumeChanged(*this, e);
    }

    void AudioSessionView::MuteToggleButton_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        _muted = !_muted;
        if (_muted)
        {
            _volumeGlyph = L"\ue74f";
        }
        else
        {
            SetGlyph();
        }

        e_propertyChanged(*this, PropertyChangedEventArgs(L"VolumeGlyph"));
        e_volumeStateChanged(*this, _muted);
    }

    void AudioSessionView::Grid_SizeChanged(IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        // For vertical layout.
        Foundation::Rect borderClippingLeftRect = Foundation::Rect(0, 0, VolumePeakBorderLeft().ActualWidth(), VolumePeakBorderLeft().ActualHeight());
        BorderClippingLeft().Rect(borderClippingLeftRect);
        Foundation::Rect borderClippingRightRect = Foundation::Rect(0, 0, VolumePeakBorderRight().ActualWidth(), VolumePeakBorderRight().ActualHeight());
        BorderClippingRight().Rect(borderClippingRightRect);
        double yTranslate = VolumePeakBorderLeft().ActualHeight();
        double yTranslate2 = VolumePeakBorderRight().ActualHeight();
        BorderClippingLeftCompositeTransform().TranslateY(yTranslate);
        BorderClippingRightCompositeTransform().TranslateY(yTranslate2);

        // For horizontal layout.
        VolumePeakBorderClippingTop().Rect(Foundation::Rect(0, 0, VolumePeakBorderTop().ActualWidth(), VolumePeakBorderTop().ActualHeight()));
        VolumePeakBorderClippingBottom().Rect(
            Foundation::Rect(0, 0, VolumePeakBorderBottom().ActualWidth(), VolumePeakBorderBottom().ActualHeight()));
    }

    void AudioSessionView::RootGrid_PointerEntered(Foundation::IInspectable const&, Xaml::PointerRoutedEventArgs const&)
    {
        Xaml::VisualStateManager::GoToState(*this, L"PointerOver", true);
    }

    void AudioSessionView::RootGrid_PointerExited(IInspectable const&, Xaml::PointerRoutedEventArgs const&)
    {
        Xaml::VisualStateManager::GoToState(*this, L"Normal", true);
    }

    void AudioSessionView::RootGrid_RightTapped(IInspectable const&, Xaml::RightTappedRoutedEventArgs const&)
    {
    }

    void AudioSessionView::LockMenuFlyoutItem_Click(IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        if (isLocked)
        {
            Xaml::VisualStateManager::GoToState(*this, L"Unlocked", true);
        }
        else
        {
            Xaml::VisualStateManager::GoToState(*this, L"Locked", true);
        }

        isLocked = !isLocked;
    }

    void AudioSessionView::HideMenuFlyoutItem_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        e_hidden(*this, IInspectable());
    }

    void AudioSessionView::RootGrid_ActualThemeChanged(Xaml::FrameworkElement const&, Foundation::IInspectable const&)
    {
        if (!active)
        {
            VolumeFontIcon().Foreground(
                Xaml::Application::Current().Resources().Lookup(box_value(L"TextFillColorPrimaryBrush")).as<Xaml::Brush>()
            );
        }
    }

    Foundation::IAsyncAction AudioSessionView::UserControl_Loading(Xaml::FrameworkElement const&, Foundation::IInspectable const&)
    {
        if (audioSessionProcessPath.empty())
        {
            co_return;
        }

        ::Croak::Imaging::IconHelper iconHelper{};

        HICON hIcon = iconHelper.LoadIconFromPath(audioSessionProcessPath);
        IStreamPtr hIconStream = iconHelper.ExtractStreamFromHICON(hIcon);

        using namespace Windows::Storage::Streams;
        InMemoryRandomAccessStream inMemoryRAS{};
        DataWriter dataWriter{ inMemoryRAS };
        dataWriter.UnicodeEncoding(UnicodeEncoding::Utf8);

        unsigned char buffer[1024]{};
        void* pv = buffer;
        ULONG cb = 1024;
        ULONG cbRead = 0;
        while (SUCCEEDED(hIconStream->Read(pv, cb, &cbRead)))
        {
            dataWriter.WriteBytes(buffer);

            // Check of EOF.
            if (cbRead < cb)
            {
                break;
            }
        }

        co_await dataWriter.StoreAsync();
        co_await dataWriter.FlushAsync();
        dataWriter.DetachStream();
        inMemoryRAS.Seek(0);

        Xaml::Image logo{};
        Xaml::BitmapImage image{};
        logo.Source(image);
        AudioSessionAppLogo().Content(logo);

        try
        {
            co_await image.SetSourceAsync(inMemoryRAS);
        }
        catch (const hresult_error& e)
        {
            OutputDebugHString(e.message());
        }

        inMemoryRAS.Close();
        dataWriter.Close();
        DestroyIcon(hIcon);
    }


    void AudioSessionView::SetGlyph()
    {
        if (_volume > 75.0)
        {
            _volumeGlyph = L"\ue995";// 🔊
        }
        else if (_volume > 50.0)
        {
            _volumeGlyph = L"\ue994";
        }
        else if (_volume > 5.0)
        {
            _volumeGlyph = L"\ue993";// 🔉
        }
        else if (_volume > 0)
        {
            _volumeGlyph = L"\ue992";// 🔈
        }
        else
        {
            _volumeGlyph = L"\ue74f";// 🔇
        }
    }
}
