#include "pch.h"
#include "AudioViewer.xaml.h"
#if __has_include("AudioViewer.g.cpp")
#include "AudioViewer.g.cpp"
#endif

#include <regex>
#include <string>
#include "DebugOutput.h"
#include "Hotkey.h"
#include "HotkeyManager.h"

#define USE_TIMER
//#define DEACTIVATE_TIMER
#define ENABLE_HOTKEYS

constexpr int VOLUME_DOWN = 1;
constexpr int VOLUME_UP = 2;
constexpr int VOLUME_UP_ALT = 3;
constexpr int VOLUME_DOWN_ALT = 4;
constexpr int MUTE_SYSTEM = 5;

namespace CAudio = ::Croak::Audio;
namespace Microsoft = winrt::Microsoft;
namespace Windows = winrt::Windows;
namespace Foundation = winrt::Windows::Foundation;
namespace System = winrt::Windows::System;
namespace Graphics = winrt::Windows::Graphics;
namespace Collections = winrt::Windows::Foundation::Collections;
namespace Power = Microsoft::Windows::System::Power;
/**
 * @brief Microsoft::UI, Microsoft::UI::Composition, Microsoft::UI::Composition::SystemBackdrops, Microsoft::UI::Windowing
*/
namespace UI
{
    using namespace winrt::Microsoft::UI;
    using namespace winrt::Microsoft::UI::Windowing;
}
/**
 * @brief Microsoft::UI::Xaml, Microsoft::UI::Xaml::Input, Microsoft::UI::Xaml::Controls, Microsoft::UI::Xaml::Media, Microsoft::UI::Xaml::Media::Imaging, Microsoft::UI::Xaml::Controls::Primitives
*/
namespace Xaml
{
    using namespace winrt::Microsoft::UI::Xaml;
    using namespace winrt::Microsoft::UI::Xaml::Input;
    using namespace winrt::Microsoft::UI::Xaml::Controls;
    using namespace winrt::Microsoft::UI::Xaml::Media;
    using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
    using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
}
/**
 * @brief Windows::Storage, Windows::ApplicationModel, Windows::ApplicationModel::Core, Windows::ApplicationModel::Resources
*/
namespace Storage
{
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::ApplicationModel::Core;
    using namespace winrt::Windows::ApplicationModel::Resources;
}


namespace winrt::Croak::implementation
{
    AudioViewer::AudioViewer()
    {
        InitializeComponent();
    }

    inline winrt::hstring AudioViewer::DefaultAudioEndpointName()
    {
        return defaultAudioEndpointName;
    }

    inline winrt::event_token AudioViewer::PropertyChanged(const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& value)
    {
        return e_propertyChanged.add(value);
    }

    inline void AudioViewer::PropertyChanged(const winrt::event_token& token)
    {
        e_propertyChanged.remove(token);
    }

    void AudioViewer::SetMessageBar(const winrt::Croak::MessageBar& messageBar)
    {
        this->messageBar = messageBar;
    }

    void AudioViewer::CloseAudioObjects()
    {
        CleanUpResources(false);
    }

    void AudioViewer::MuteEndpoint()
    {
        mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());
        MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
    }

    void AudioViewer::LoadAudioProfile(const AudioProfile& audioProfile)
    {
        try
        {
#pragma region Basic profile stuff
            // Set system volume.
            mainAudioEndpoint->SetVolume(audioProfile.DefaultAudioEndpointSettings().AudioLevel());

            if (audioProfile.DisableAnimations())
            {
                StopAnimations();
            }

            Croak::AudioSessionLayout windowLayout = audioProfile.Layout();
            if (windowLayout != Croak::AudioSessionLayout::Auto)
            {
                SetAudioSessionLayout(windowLayout);
            }
#pragma endregion

            AudioSessionsGridView().ItemsSource(nullptr);

            // Set audio sessions volume.
            std::unique_lock lock{ audioSessionsMutex }; // Taking the lock will also lock sessions from being added to the display.
            auto audioSessionsSettings = audioProfile.AudioSessionsSettings();
            for (auto&& audioSessionSettings : audioSessionsSettings)
            {
                for (std::pair<winrt::guid, CAudio::AudioSession*> iter : audioSessions)
                {
                    if (iter.second->Name() == audioSessionSettings.Name())
                    {
                        iter.second->SetVolume(audioSessionSettings.AudioLevel());
                        iter.second->SetMute(audioSessionSettings.Muted());
                    }
                }
            }

            std::vector<AudioSessionView> uniqueSessions{};
            // HACK: IVector<T>::GetMany does not seem to work. Manual copy.
            for (uint32_t i = 0; i < audioSessionViews.Size(); i++)
            {
                uniqueSessions.push_back(audioSessionViews.GetAt(i));
            }

            auto indexes = audioProfile.SessionsIndexes();
            auto&& views = std::vector<AudioSessionView>(indexes.Size(), nullptr);
            for (size_t i = 0; i < uniqueSessions.size(); i++)
            {
                auto&& view = uniqueSessions[i];
                auto opt = indexes.TryLookup(view.Header());
                if (opt.has_value())
                {
                    size_t index = opt.value();
                    if (index < views.size())
                    {
                        // The index might already be populated by the else if/else statements when the index is over the collection size.
                        if (views[index] == nullptr)
                        {
                            views[index] = view;
                        }
                        else
                        {
                            views.push_back(views[index]);
                            views[index] = view;
                        }
                    }
                    else if (views[views.size() - 1] == nullptr) // Check if the last index of the collection is free.
                    {
                        views[views.size() - 1] = view;
                    }
                    else // The index is over the collection size and the last value of the collection is already populated.
                    {
                        views.push_back(view);
                    }

                    uniqueSessions[i] = nullptr; // Set to null to tell the session has been added.
                }
            }

            // Add the sessions that were not in the profile.
            // Try to add the session where there is still space. If not possible, append it.
            bool hasTrailingNullptrs = true;
            for (size_t i = 0; i < uniqueSessions.size(); i++)
            {
                if (uniqueSessions[i])
                {
                    int j = static_cast<int>(views.size()) - 1;
                    for (; j >= 0; j--)
                    {
                        if (views[j] == nullptr)
                        {
                            views[j] = uniqueSessions[i];
                            break;
                        }
                    }

                    if (j < 0)
                    {
                        views.push_back(uniqueSessions[i]);
                        hasTrailingNullptrs = false;
                    }
                }
            }

            if (hasTrailingNullptrs)
            {
                //HACK: Clear null values by passing another time through the view array.
                size_t finalSize = views.size();
                for (size_t i = 0; i < views.size(); i++)
                {
                    if (views[i] == nullptr)
                    {
                        size_t j = i + 1;
                        while (j < views.size() && views[j] == nullptr)
                        {
                            j++;
                        }

                        if (j < views.size())
                        {
                            views[i] = std::move(views[j]);
                        }
                        else
                        {
                            finalSize--;
                        }
                    }
                }
                views.resize(finalSize);
            }

            audioSessionViews = winrt::multi_threaded_observable_vector<AudioSessionView>(std::move(views));

            // HACK: Can we use INotifyPropertyChanged to raise that the vector has changed ?
            AudioSessionsGridView().ItemsSource(audioSessionViews);
            // I18N: Loaded profile [profile name]
            EnqueueString(L"Loaded profile " + audioProfile.ProfileName());
        }
        catch (const hresult_error& error)
        {
            // I18N: Failed to load profile [profile name]
            EnqueueString(L"Couldn't load profile " + audioProfile.ProfileName());
            OutputDebugHString(error.message());
        }
        catch (const std::out_of_range& ex)
        {
            // TODO: Log exception to EventViewer to enable app analysis.
            OutputDebugHString(to_hstring(ex.what()));
            EnqueueString(L"Couldn't load profile " + audioProfile.ProfileName());
        }
    }

    void AudioViewer::PauseAnimations()
    {
        if (audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }

        if (mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }
    }

    void AudioViewer::SetAudioSessionLayout(const winrt::Croak::AudioSessionLayout& audioSessionLayout)
    {
        if (audioSessionLayout == AudioSessionLayout::Vertical)
        {
            // Vertical layout
            AudioSessionsScrollViewer().HorizontalScrollBarVisibility(Xaml::ScrollBarVisibility::Disabled);
            AudioSessionsScrollViewer().VerticalScrollBarVisibility(Xaml::ScrollBarVisibility::Auto);
            AudioSessionsGridView().Style(
                RootGrid().Resources().Lookup(box_value(L"GridViewVerticalLayout")).as<Xaml::Style>());
        }
        else if (audioSessionLayout == AudioSessionLayout::Horizontal)
        {
            // Horizontal layout
            AudioSessionsScrollViewer().HorizontalScrollBarVisibility(Xaml::ScrollBarVisibility::Auto);
            AudioSessionsScrollViewer().VerticalScrollBarVisibility(Xaml::ScrollBarVisibility::Disabled);

            AudioSessionsGridView().Style(
                RootGrid().Resources().Lookup(box_value(L"GridViewHorizontalLayout")).as<Xaml::Style>()
            );
        }
        else
        {
            // Auto layout
            AudioSessionsScrollViewer().HorizontalScrollBarVisibility(Xaml::ScrollBarVisibility::Disabled);
            AudioSessionsScrollViewer().VerticalScrollBarVisibility(Xaml::ScrollBarVisibility::Auto);

            AudioSessionsGridView().Style(
                RootGrid().Resources().Lookup(box_value(L"GridViewHorizontalLayout")).as<Xaml::Style>()
            );  
        }

        // Update audio sessions.
        for (auto&& audioSession : audioSessionViews)
        {
            audioSession.Orientation(audioSessionLayout == AudioSessionLayout::Vertical ? Xaml::Orientation::Horizontal : Xaml::Orientation::Vertical);
        }

        layout = audioSessionLayout;
    }

    void AudioViewer::StopAnimations()
    {
        if (audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }
        else
        {
            audioSessionsPeakTimer.Start();
        }

        if (mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }
        else
        {
            mainAudioEndpointPeakTimer.Start();
        }

        for (auto&& view : audioSessionViews)
        {
            view.SetPeak(0, 0);
        }

        LeftVolumeAnimation().To(0.);
        RightVolumeAnimation().To(0.);
        VolumeStoryboard().Begin();
    }

    void AudioViewer::SwitchAudioState()
    {
        bool setMute = false;

        uint32_t count = 0;
        if (globalSessionAudioState == winrt::Croak::AudioSessionState::Unmuted)
        {
            for (auto&& view : audioSessionViews)
            {
                if (view.Muted())
                {
                    count++;
                }
            }

            if (count != audioSessionViews.Size())
            {
                setMute = true;
                globalSessionAudioState = AudioSessionState::Muted;
            }
        }
        else if (globalSessionAudioState == winrt::Croak::AudioSessionState::Muted)
        {
            for (auto&& view : audioSessionViews)
            {
                if (!view.Muted())
                {
                    count++;
                }
            }

            if (count == audioSessionViews.Size())
            {
                setMute = true;
                globalSessionAudioState = AudioSessionState::Muted;
            }
            else
            {
                setMute = false;
                globalSessionAudioState = AudioSessionState::Unmuted;
            }
        }

        {
            std::unique_lock lock{ audioSessionsMutex };
            for (std::pair<guid, CAudio::AudioSession*> iter : audioSessions)
            {
                iter.second->Muted(setMute);
            }
        }

        for (AudioSessionView view : audioSessionViews)
        {
            view.Muted(setMute);
        }
    }

    void AudioViewer::ReloadAudioSessions()
    {
        Storage::ResourceLoader resLoader{};
        // Stop peak timers to avoid race conditions for nullptrs.
        if (audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }
        /*if (mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }*/

        audioSessionViews.Clear();
        VolumeStoryboard().Stop();

        // Unregister VolumeChanged event handler & unregister audio sessions from audio events and release com ptrs.
        {
            std::unique_lock lock{ audioSessionsMutex };
            for (auto& pair : audioSessions)
            {
                CAudio::AudioSession* audioSession = pair.second;
                audioSession->VolumeChanged(audioSessionVolumeChanged[pair.first]);
                audioSession->StateChanged(audioSessionsStateChanged[pair.first]);
                audioSession->Unregister();
                audioSession->Release();
            }
            audioSessions.clear();
            // The lock can be released since no interactions will be made with audioSessions && audioSessionViews
        }

        // Reload audio sessions.
        try
        {
            auto audioSessionsVect = audioController->GetSessions();
            for (size_t i = 0; i < audioSessionsVect->size(); i++)
            {
                audioSessions.insert({ guid(audioSessionsVect->at(i)->Id()), audioSessionsVect->at(i) });

                AudioSessionView view = CreateRegisterView(audioSessionsVect->at(i));
                if (view)
                {
                    audioSessionViews.Append(view);
                }
            }

            if (!animationsDisabled)
            {
                audioSessionsPeakTimer.Start();
            }

            EnqueueString(resLoader.GetString(L"InfoAudioSessionsReloaded"));
        }
        catch (const winrt::hresult_error& err)
        {
            OutputDebugHString(L"Failed to reload audio sessions: " + err.message());
        }
    }

    void AudioViewer::ShowHiddenAudioSessions()
    {
        static bool showingSessions = false;

        /*
        * - Show hidden sessions, enable item selection.
        * - Selected audio sessions will be hidden the next time 'show hidden buttons' is clicked.
        * - Unselected audio sessions will be kept the next time 'show hidden buttons' is clicked.
        */
        // TODO: How do I insert the new sessions: lookup in the profile (if loaded) or insert at the end to not disrupt the UX too much.
        if (showingSessions)
        {
            AudioSessionsGridView().SelectionMode(Xaml::ListViewSelectionMode::None);
            auto&& selectedItems = AudioSessionsGridView().SelectedItems();
            auto ranges = AudioSessionsGridView().SelectedRanges();
            for (auto&& range : ranges)
            {
                DebugLog(std::format("[{0:d}, {1:d}] length={2:d}", range.FirstIndex(), range.LastIndex(), range.Length()));
            }
        }
        else
        {
            for (auto&& pair : audioSessions)
            {
                bool hidden = true;
                for (auto&& view : audioSessionViews)
                {
                    if (pair.first == view.Id())
                    {
                        // Audio session is hidden
                        hidden = false;
                        break;
                    }
                }

                if (hidden)
                {
                    CAudio::AudioSession* audioSession = pair.second;
                    AudioSessionView view = CreateView(audioSession, false);
                    if (view)
                    {
                        audioSessionViews.Append(view);
                        view.IsEnabled(false);
                    }
                }
            }

            AudioSessionsGridView().SelectionMode(Xaml::ListViewSelectionMode::Multiple);
        }

        showingSessions = !showingSessions;
    }


    void AudioViewer::MuteToggleButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());
        MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
    }

    void AudioViewer::SystemVolumeActivityBorder_SizeChanged(Foundation::IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        SystemVolumeActivityBorderClippingRight().Rect(
            Foundation::Rect(0, 0, static_cast<float>(SystemVolumeActivityBorderRight().ActualWidth()), static_cast<float>(SystemVolumeActivityBorderRight().ActualHeight()))
        );
        SystemVolumeActivityBorderClippingLeft().Rect(
            Foundation::Rect(0, 0, static_cast<float>(SystemVolumeActivityBorderLeft().ActualWidth()), static_cast<float>(SystemVolumeActivityBorderLeft().ActualHeight()))
        );
    }

    void AudioViewer::SystemVolumeSlider_ValueChanged(Foundation::IInspectable const&, Xaml::RangeBaseValueChangedEventArgs const& e)
    {
        if (mainAudioEndpoint)
        {
            mainAudioEndpoint->Volume(static_cast<float>(e.NewValue() / 100.));
        }

        SystemVolumeNumberBlock().Double(e.NewValue());
    }

    void AudioViewer::AudioSessionsPanel_Loading(Xaml::FrameworkElement const&, Foundation::IInspectable const& args)
    {
        LoadContent();

#ifdef ENABLE_HOTKEYS
        LoadHotKeys();
#endif // ENABLE_HOTKEYS
    }

    void AudioViewer::OnLoaded(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        atomicLoaded.exchange(true);

        // Generate size changed event to get correct clipping rectangle size
        SystemVolumeActivityBorder_SizeChanged(nullptr, nullptr);
        Grid_SizeChanged(nullptr, nullptr);

        audioSessionsPeakTimer.Start();
        mainAudioEndpointPeakTimer.Start();

        /*if ((!currentAudioProfile || !currentAudioProfile.DisableAnimations()) && 
            (!audioSessionsPeakTimer.IsRunning() || !mainAudioEndpointPeakTimer.IsRunning()))
        {
            DEBUG_BREAK();
        }*/

        Power::PowerManager::UserPresenceStatusChanged([this](IInspectable, IInspectable)
        {
            auto userStatus = static_cast<int32_t>(Power::PowerManager::UserPresenceStatus());

            switch (userStatus)
            {
                case static_cast<int32_t>(Power::UserPresenceStatus::Present):
                {
                    OutputDebugHString(L"User presence status changed: user present.");

                    if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true))
                    {
                        DispatcherQueue().TryEnqueue([this]()
                        {
                            if (!mainAudioEndpointPeakTimer.IsRunning())
                            {
                                mainAudioEndpointPeakTimer.Start();
                            }

                            if (!audioSessionsPeakTimer.IsRunning())
                            {
                                audioSessionsPeakTimer.Start();
                            }
                        });
                    }

                    break;
                }
                case static_cast<int32_t>(Power::UserPresenceStatus::Absent):
                case 2:
                {
                    OutputDebugHString(L"User presence status changed: user absent.");

                    if (unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"PowerEfficiencyEnabled"), true))
                    {
                        DispatcherQueue().TryEnqueue([this]()
                        {
                            if (mainAudioEndpointPeakTimer.IsRunning())
                            {
                                mainAudioEndpointPeakTimer.Stop();
                                LeftVolumeAnimation().To(0.);
                                RightVolumeAnimation().To(0.);
                                VolumeStoryboard().Begin();
                            }

                            if (audioSessionsPeakTimer.IsRunning())
                            {
                                audioSessionsPeakTimer.Stop();
                                for (auto&& view : audioSessionViews)
                                {
                                    view.SetPeak(0, 0);
                                }
                            }
                        });
                    }
                    break;
                }
            }
        });
    }

    void AudioViewer::Grid_SizeChanged(Foundation::IInspectable const&, Xaml::SizeChangedEventArgs const&)
    {
        DrawSizeIndicators();
    }

    void AudioViewer::AudioSessionView_VolumeChanged(winrt::Croak::AudioSessionView const& sender, Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& args)
    {
        if (audioSessions.find(sender.Id()) != audioSessions.end())
        {
            audioSessions[sender.Id()]->SetVolume(args.NewValue() / 100.f);
        }
    }

    void AudioViewer::AudioSessionView_VolumeStateChanged(winrt::Croak::AudioSessionView const& sender, bool const& args)
    {
        //TODO: 
    }

    void AudioViewer::UserControl_Unloaded(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        //CleanUpResources(false);
    }

    void AudioViewer::AppBarButton_Click(Foundation::IInspectable const& sender, Xaml::RoutedEventArgs const& e)
    {
        ShowHiddenAudioSessions();
    }


    /**
     * @brief Registers CAudio::AudioSession events and creates a view.
     * @param audioSession 
     * @return 
    */
    winrt::Croak::AudioSessionView AudioViewer::CreateRegisterView(CAudio::AudioSession* audioSession)
    {
        if (audioSession->Register())
        {
            audioSessionVolumeChanged.insert({ audioSession->Id(), audioSession->VolumeChanged({ this, &AudioViewer::AudioSession_VolumeChanged }) });
            audioSessionsStateChanged.insert({ audioSession->Id(), audioSession->StateChanged({ this, &AudioViewer::AudioSession_StateChanged }) });
        }
        else
        {
            OutputDebugWString(L"Failed to register audio session '" + audioSession->Name() + L"'.");
            EnqueueString(hstring(audioSession->Name() + L" notifications off"));
        }

        return CreateView(audioSession, true);
    }

    winrt::Croak::AudioSessionView AudioViewer::CreateView(CAudio::AudioSession* audioSession, bool checkDuplicates)
    {
        // Check for duplicates, multiple audio sessions might be grouped under one by the app/system owning the sessions.
        if (checkDuplicates && audioSession->State() != ::AudioSessionState::AudioSessionStateActive)
        {
            std::unique_lock lock{ audioSessionsMutex };

            uint8_t hitcount = 0u;
            for (auto& iter : audioSessions)
            {
                if (iter.second->GroupingParam() == audioSession->GroupingParam())
                {
                    if (hitcount > 0)
                    {
                        return nullptr;
                    }
                    hitcount++;
                }
            }
        }

        AudioSessionView view = nullptr;
        if (audioSession->IsSystemSoundSession())
        {
            view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);

            Xaml::FontIcon icon{};
            icon.Glyph(L"\ue977");
            // #5FDFFF
            icon.Foreground(
                Xaml::SolidColorBrush(Xaml::Application::Current().Resources().TryLookup(box_value(L"WindowsLogoColor")).as<Windows::UI::Color>())
            );
            Xaml::Viewbox iconViewbox{};
            iconViewbox.HorizontalAlignment(Xaml::HorizontalAlignment::Stretch);
            iconViewbox.VerticalAlignment(Xaml::VerticalAlignment::Stretch);
            iconViewbox.Child(icon);
            view.Logo().Content(iconViewbox);
        }
        else
        {
            if (!audioSession->ProcessInfo()->Manifest().Logo().empty())
            {
                view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);

                Xaml::BitmapImage imageSource{};
                imageSource.UriSource(Foundation::Uri(audioSession->ProcessInfo()->Manifest().Logo()));
                Xaml::Image image{};
                image.Source(imageSource);

                view.Logo().Content(image);
            }
            else if (!audioSession->ProcessInfo()->ExecutablePath().empty())
            {
                view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0, audioSession->ProcessInfo()->ExecutablePath());
            }
            else
            {
                view = AudioSessionView(audioSession->Name(), audioSession->Volume() * 100.0);
            }
        }

        view.Id(guid(audioSession->Id()));
        view.Muted(audioSession->Muted());
        view.SetState((AudioSessionState)audioSession->State());
        view.Orientation(layout == AudioSessionLayout::Horizontal ? Xaml::Orientation::Horizontal : Xaml::Orientation::Vertical);

        view.VolumeChanged({ this, &AudioViewer::AudioSessionView_VolumeChanged });
        view.VolumeStateChanged({ this, &AudioViewer::AudioSessionView_VolumeStateChanged });
        view.Hidden([this](AudioSessionView sender, auto)
        {
#ifdef DEBUG

#else
            for (auto const& view : audioSessionViews)
            {
                if (view.Id() == sender.Id())
                {
                    uint32_t indexOf = 0;
                    if (audioSessionViews.IndexOf(view, indexOf))
                    {
                        audioSessionViews.RemoveAt(indexOf);
                    }

                    return;
                }
            }
#endif // DEBUG

        });

        return view;
    }

    void AudioViewer::LoadContent()
    {
        Storage::ResourceLoader loader{};
        GUID appID{};
        if (SUCCEEDED(UuidCreate(&appID)))
        {
            try
            {
                // Create and setup audio interfaces.
                audioController = new CAudio::AudioController(appID);

                if (audioController->Register())
                {
                    audioController->SessionAdded({ this, &AudioViewer::AudioController_SessionAdded });
                    audioController->EndpointChanged({ this, &AudioViewer::AudioController_EndpointChanged });
                }
                else
                {
                    EnqueueString(loader.GetString(L"ErrorAudioSessionsUnavailable"));
                }

                mainAudioEndpoint = audioController->GetMainAudioEndpoint();
                DefaultAudioEndpointName(mainAudioEndpoint->Name());
                if (mainAudioEndpoint->Register())
                {
                    mainAudioEndpointVolumeChangedToken = mainAudioEndpoint->VolumeChanged({ this, &AudioViewer::MainAudioEndpoint_VolumeChanged });
                    mainAudioEndpointStateChangedToken = mainAudioEndpoint->StateChanged([this](IInspectable, bool muted)
                    {
                        DispatcherQueue().TryEnqueue([this, muted]()
                        {
                            MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());
                            MuteToggleButtonFontIcon().Glyph(muted ? L"\ue74f" : L"\ue767");
                        });
                    });
                }

                // TODO: Check code validity, no memory leaks, performance.
                auto audioSessionsVector = std::unique_ptr<std::vector<CAudio::AudioSession*>>();
                try
                {
                    audioSessionsVector = std::unique_ptr<std::vector<CAudio::AudioSession*>>(audioController->GetSessions());
                    for (size_t i = 0; i < audioSessionsVector->size(); i++)
                    {
                        audioSessions.insert({ guid(audioSessionsVector->at(i)->Id()), audioSessionsVector->at(i) });

                        // Check if the session is active, if not check if the user asked to show inactive sessions on startup.
                        if (audioSessionsVector->at(i)->State() == ::AudioSessionState::AudioSessionStateActive ||
                            unbox_value_or(Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"ShowInactiveSessionsOnStartup"), true))
                        {
                            if (AudioSessionView view = CreateRegisterView(audioSessionsVector->at(i)))
                            {
                                audioSessionViews.Append(view);
                            }
                        }
                        else // Register to events since we are not adding/creating the view.
                        {
                            if (audioSessionsVector->at(i)->Register())
                            {
                                audioSessionVolumeChanged.insert({ audioSessionsVector->at(i)->Id(), audioSessionsVector->at(i)->VolumeChanged({ this, &AudioViewer::AudioSession_VolumeChanged }) });
                                audioSessionsStateChanged.insert({ audioSessionsVector->at(i)->Id(), audioSessionsVector->at(i)->StateChanged({ this, &AudioViewer::AudioSession_StateChanged }) });
                            }
                            else
                            {
                                OutputDebugWString(L"Failed to register audio session '" + audioSessionsVector->at(i)->Name() + L"'. This session will never be shown.");
                                EnqueueString(hstring(audioSessionsVector->at(i)->Name() + L" : notifications off"));
                            }
                        }
                    }
                }
                catch (const hresult_error&)
                {
                    // Clean audioSessionsVector and throw the error back.
                    if (audioSessionsVector.get())
                    {
                        for (size_t j = 0; j < audioSessionsVector->size(); j++)
                        {
                            audioSessionsVector->at(j)->Release();
                        }

                        auto vect = audioSessionsVector.release();
                        delete vect;
                    }
                    throw;
                }


                // Create and setup peak meters timers
                audioSessionsPeakTimer = DispatcherQueue().CreateTimer();
                mainAudioEndpointPeakTimer = DispatcherQueue().CreateTimer();

                audioSessionsPeakTimer.Interval(Foundation::TimeSpan(std::chrono::milliseconds(83)));
                audioSessionsPeakTimer.Tick({ this, &AudioViewer::UpdatePeakMeters });
                audioSessionsPeakTimer.Stop();

                mainAudioEndpointPeakTimer.Interval(Foundation::TimeSpan(std::chrono::milliseconds(83)));
                mainAudioEndpointPeakTimer.Tick([&](auto, auto)
                {
                    if (!atomicLoaded.load())
                    {
                        return;
                    }

                    try
                    {
                        std::pair<float, float> peakValues = mainAudioEndpoint->GetPeaks();
                        LeftVolumeAnimation().To(static_cast<double>(peakValues.first));
                        RightVolumeAnimation().To(static_cast<double>(peakValues.second));
                        VolumeStoryboard().Begin();
                    }
                    catch (const hresult_error&)
                    {
                        // TODO: Handle error.
                    }
                });

                //MainEndpointNameTextBlock().Text(mainAudioEndpoint->Name());
                SystemVolumeSlider().Value(static_cast<double>(mainAudioEndpoint->Volume()) * 100.);
                MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());
                MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
            }
            catch (const hresult_error& ex)
            {
                EnqueueString(loader.GetString(L"ErrorFatalFailure"));
                OutputDebugHString(ex.message());
            }
        }
        else
        {
            EnqueueString(loader.GetString(L"ErrorFatalFailure"));
        }
    }

    void AudioViewer::SaveAudioLevels()
    {
        Storage::ApplicationDataContainer audioLevels = Storage::ApplicationData::Current().LocalSettings().CreateContainer(
            L"AudioLevels",
            Storage::ApplicationData::Current().LocalSettings().Containers().HasKey(L"AudioLevels") ? Storage::ApplicationDataCreateDisposition::Existing : Storage::ApplicationDataCreateDisposition::Always
        );

        for (uint32_t i = 0; i < audioSessionViews.Size(); i++) // Only saving the visible audio sessions levels.
        {
            Storage::ApplicationDataCompositeValue compositeValue{};
            compositeValue.Insert(L"Muted", box_value(audioSessionViews.GetAt(i).Muted()));
            compositeValue.Insert(L"Level", box_value(audioSessionViews.GetAt(i).Volume()));
            if (!audioSessionViews.GetAt(i).Header().empty())
            {
                audioLevels.Values().Insert(audioSessionViews.GetAt(i).Header(), compositeValue);
            }
        }
    }

    void AudioViewer::LoadHotKeys()
    {
        ::Croak::System::HotKeys::HotKeyManager& hotKeyManager = ::Croak::System::HotKeys::HotKeyManager::GetHotKeyManager();
        hotKeyManager.HotKeyFired({ this, &AudioViewer::HotKeyFired });

        try
        {
            hotKeyManager.RegisterHotKey(Windows::System::VirtualKeyModifiers::Windows | Windows::System::VirtualKeyModifiers::Control, VK_DOWN, true, VOLUME_DOWN);
            hotKeyManager.RegisterHotKey(Windows::System::VirtualKeyModifiers::Windows | Windows::System::VirtualKeyModifiers::Control, VK_UP, true, VOLUME_UP);
            hotKeyManager.RegisterHotKey(Windows::System::VirtualKeyModifiers::Control | Windows::System::VirtualKeyModifiers::Menu, VK_UP, true, VOLUME_UP_ALT);
            hotKeyManager.RegisterHotKey(Windows::System::VirtualKeyModifiers::Control | Windows::System::VirtualKeyModifiers::Menu, VK_DOWN, true, VOLUME_DOWN_ALT);
            hotKeyManager.RegisterHotKey(Windows::System::VirtualKeyModifiers::Control | Windows::System::VirtualKeyModifiers::Shift, 'M', true, MUTE_SYSTEM);
        }
        catch (const std::invalid_argument& ex)
        {
            DebugLog(std::format("Could not register hot key.\n\t > {0}", ex.what()));
            EnqueueMessage(box_value(L"Failed to enable some hot keys."));
        }
    }

    void AudioViewer::LoadSettings()
    {
        //TODO: Load non audio related settings.
    }

    void AudioViewer::SaveSettings()
    {
        if (currentAudioProfile)
        {
            Collections::IPropertySet settings = Storage::ApplicationData::Current().LocalSettings().Values();
            settings.Insert(L"AudioProfile", box_value(currentAudioProfile.ProfileName()));

            if (unbox_value_or(settings.TryLookup(L"AllowChangesToLoadedProfile"), false))
            {
                //currentAudioProfile.SystemVolume(mainAudioEndpoint->Volume());
                //currentAudioProfile.Layout(layout);
                currentAudioProfile.ShowMenu(AppBarGrid().Visibility() == Xaml::Visibility::Visible);
                currentAudioProfile.DisableAnimations(mainAudioEndpointPeakTimer.IsRunning());

                std::unique_lock lock{ audioSessionsMutex };
                for (std::pair<winrt::guid, CAudio::AudioSession*> iter : audioSessions)
                {
                    auto name = iter.second->Name();
                    auto muted = iter.second->Muted();
                    auto volume = iter.second->Volume();

                    currentAudioProfile.AudioSessionsSettings().Append(AudioSessionSettings(name, muted, volume));
                }

                // If the user has a profile, the AudioProfile container has to exist, so no TryLookup and container creation.
                currentAudioProfile.Save(Storage::ApplicationData::Current().LocalSettings().Containers().Lookup(L"AudioProfiles"));
            }
        }
    }

    void AudioViewer::CleanUpResources(const bool& forReload)
    {
        if (audioSessionsPeakTimer && audioSessionsPeakTimer.IsRunning())
        {
            audioSessionsPeakTimer.Stop();
        }

        if (!forReload && mainAudioEndpointPeakTimer && mainAudioEndpointPeakTimer.IsRunning())
        {
            mainAudioEndpointPeakTimer.Stop();
        }

        VolumeStoryboard().Stop();

        // Unregister VolumeChanged event handler & unregister audio sessions from audio events and release com ptrs.
        if (audioSessions.size() > 0)
        {
            std::unique_lock lock{ audioSessionsMutex };

            for (auto& iter : audioSessions)
            {
                iter.second->VolumeChanged(audioSessionVolumeChanged[iter.first]);
                iter.second->StateChanged(audioSessionsStateChanged[iter.first]);

                assert(iter.second->Unregister());
                assert(iter.second->Release() == 0);
            }
        }

        // Clean up ComPtr/IUnknown objects
        if (audioController)
        {
            if (mainAudioEndpoint)
            {
                mainAudioEndpoint->VolumeChanged(mainAudioEndpointVolumeChangedToken);
                mainAudioEndpoint->Unregister();
                mainAudioEndpoint->Release();
            }

            audioController->EndpointChanged(audioControllerEndpointChangedToken);
            audioController->SessionAdded(audioControllerSessionAddedToken);

            assert(audioController->Unregister());
            assert(audioController->Release() == 0);
        }

        if (forReload)
        {
            audioSessionViews.Clear();
        }
        else
        {
            SaveAudioLevels();
        }
    }

    void AudioViewer::InsertSessionAccordingToProfile(::Croak::Audio::AudioSession* audioSession)
    {
    }

    void AudioViewer::DrawSizeIndicators()
    {
        if (layout != AudioSessionLayout::Vertical)
        {
            double gridHeight = AudioSessionsGrid().ActualHeight();
            double gridWidth = AudioSessionsGrid().ActualWidth();

            IndicatorsCanvas().Children().Clear();

            int32_t position = 1;
            const int32_t sessionHeight = 290 + 2;
            while ((position * sessionHeight) < gridHeight)
            {
                winrt::Microsoft::UI::Xaml::Shapes::Rectangle rect{};
                rect.Width(6);
                rect.Height(1);
                auto&& brush = Xaml::SolidColorBrush(UI::Colors::DimGray());
                brush.Opacity(0.5);
                rect.Fill(brush);

                Xaml::Canvas::SetLeft(rect, 0);
                Xaml::Canvas::SetTop(rect, position * sessionHeight);
                IndicatorsCanvas().Children().Append(rect);

                position++;
            }

            position = 1;
            const int32_t sessionWidth = 80 + 3;
            while ((position * sessionWidth) < gridWidth)
            {
                winrt::Microsoft::UI::Xaml::Shapes::Rectangle rect{};
                rect.Width(1);
                rect.Height(6);
                auto&& brush = Xaml::SolidColorBrush(UI::Colors::DimGray());
                brush.Opacity(0.5);
                rect.Fill(brush);

                Xaml::Canvas::SetLeft(rect, position * sessionWidth);
                Xaml::Canvas::SetTop(rect, 0);
                IndicatorsCanvas().Children().Append(rect);

                position++;
            }
        }
    }

    void AudioViewer::HotKeyFired(const uint32_t& id, const Foundation::IInspectable&)
    {
        constexpr float stepping = 0.01f;
        constexpr float largeStepping = 0.07f;
        try
        {
            switch (id)
            {
                case VOLUME_DOWN:
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() - stepping > 0.f ? mainAudioEndpoint->Volume() - stepping : 0.f);
                    break;
                }
                case VOLUME_UP:
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() + stepping < 1.f ? mainAudioEndpoint->Volume() + stepping : 1.f);
                    break;
                }
                case VOLUME_UP_ALT:
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() + largeStepping < 1.f ? mainAudioEndpoint->Volume() + largeStepping : 1.f);
                    break;
                }
                case VOLUME_DOWN_ALT:
                {
                    mainAudioEndpoint->SetVolume(mainAudioEndpoint->Volume() - largeStepping > 0.f ? mainAudioEndpoint->Volume() - largeStepping : 0.f);
                    break;
                }
                case MUTE_SYSTEM:
                {
                    mainAudioEndpoint->SetMute(!mainAudioEndpoint->Muted());

                    DispatcherQueue().TryEnqueue([this]()
                    {
                        MuteToggleButtonFontIcon().Glyph(mainAudioEndpoint->Muted() ? L"\ue74f" : L"\ue767");
                        MuteToggleButton().IsChecked(Foundation::IReference(mainAudioEndpoint->Muted()));
                    });
                    break;
                }
                default:
                {
                    DebugLog(std::format("Hot key not recognized. Id: {0}", id));
                }
            }
        }
        catch (...)
        {
        }
    }

    void AudioViewer::UpdatePeakMeters(Foundation::IInspectable, Foundation::IInspectable)
    {
        if (!atomicLoaded.load() || audioSessions.size() == 0) return;

        for (auto const& view : audioSessionViews)
        {
            auto&& pair = audioSessions.at(view.Id())->GetChannelsPeak();
            view.SetPeak(pair.first, pair.second);
        }
    }

    void AudioViewer::MainAudioEndpoint_VolumeChanged(Foundation::IInspectable, const float& newVolume)
    {
        if (!atomicLoaded.load()) return;

        DispatcherQueue().TryEnqueue([this, newVolume]()
        {
            SystemVolumeSlider().Value(newVolume * 100.);
        });
    }

    void AudioViewer::AudioSession_VolumeChanged(const winrt::guid& id, const float& newVolume)
    {
        if (!atomicLoaded.load()) return;

        DispatcherQueue().TryEnqueue([this, id, newVolume]()
        {
            for (auto const& view : audioSessionViews)
            {
                if (view.Id() == id)
                {
                    view.Volume(static_cast<double>(newVolume) * 100.0);
                }
            }
        });
    }

    void AudioViewer::AudioSession_StateChanged(const winrt::guid& id, const uint32_t& state)
    {
        if (!atomicLoaded.load()) return;

        // Cast state to AudioSessionState, uint32_t is only used to cross ABI
        AudioSessionState audioState = (AudioSessionState)state;
        if (audioState == AudioSessionState::Expired || audioState == AudioSessionState::Active)
        {
            audioSessionsPeakTimer.Stop();

            std::unique_lock lock{ audioSessionsMutex };
            if (audioSessions.contains(id))
            {
                if (audioState == AudioSessionState::Active)
                {
                    // Check if the newly active session is added to the UI, if not I need to add it as it might 
                    // have been skipped because of grouping params and the session being inactive at the time.
                    bool added = false;
                    for (auto const& view : audioSessionViews)
                    {
                        if (view.Id() == id)
                        {
                            added = true;
                            break;
                        }
                    }

                    if (!added)
                    {
                        lock.unlock();

                        DispatcherQueue().TryEnqueue([this, id]()
                        {
                            std::unique_lock lock{ audioSessionsMutex };
                            if (AudioSessionView view = CreateView(audioSessions.at(id), true))
                            {
                                audioSessionViews.InsertAt(0, view);
                            }
                        });

                        // Return here since the code in DispatcherQueue().TryEnqueue([this, id, audioState]...) below relies on finding an AudioSessionView 
                        // with the matching id. Or I found that the AudioSessionView with the given id does not exists.
                        audioSessionsPeakTimer.Start();
                        return;
                    }
                }
                else
                {
                    // The audio session is expired 
                    CAudio::AudioSession* session = audioSessions.at(id);
                    session->VolumeChanged(audioSessionVolumeChanged[session->Id()]);
                    session->StateChanged(audioSessionsStateChanged[session->Id()]);
                    assert(session->Unregister());
                    session->Release();

                    audioSessions.erase(id);
                }
            }

            audioSessionsPeakTimer.Start();
        }

        DispatcherQueue().TryEnqueue([this, id, audioState]()
        {
            for (auto const& view : audioSessionViews)
            {
                if (view.Id() == id)
                {
                    switch (audioState)
                    {
                        case AudioSessionState::Muted:
                            view.Muted(true);
                            break;

                        case AudioSessionState::Unmuted:
                            view.Muted(false);
                            break;

                        case AudioSessionState::Active:
                        case AudioSessionState::Inactive:
                            view.SetState(audioState);
                            break;

                        case AudioSessionState::Expired:
                            uint32_t indexOf = 0;
                            if (audioSessionViews.IndexOf(view, indexOf))
                            {
                                audioSessionViews.RemoveAt(indexOf);

                                if (audioSessionViews.Size() == 0)
                                {
                                    EnqueueString(L"All sessions expired.");// I18N: Translate "All sessions expired"
                                }
                            }
                            break;
                    }
                    return;
                }
            }
        });
    }

    void AudioViewer::AudioController_SessionAdded(Foundation::IInspectable, Foundation::IInspectable)
    {
        // TODO: Reorder audio sessions according to the currently loaded audio profile (if any).
        DispatcherQueue().TryEnqueue([this]()
        {
            audioSessionsPeakTimer.Stop();

            while (CAudio::AudioSession* newSession = audioController->NewSession())
            {
                {
                    std::unique_lock lock{ audioSessionsMutex };
                    audioSessions.insert({ newSession->Id(), newSession });
                }

                if (AudioSessionView view = CreateRegisterView(newSession))
                {
                    // TODO: If a profile is loaded, find the right index for the audio session to be added to.
                    audioSessionViews.InsertAt(0, view);
                }
            }

            audioSessionsPeakTimer.Start();
        });
    }

    void AudioViewer::AudioController_EndpointChanged(Foundation::IInspectable, Foundation::IInspectable)
    {
        // Stop animation while getting the new audio endpoint.
        mainAudioEndpointPeakTimer.Stop();

        mainAudioEndpoint->VolumeChanged(mainAudioEndpointVolumeChangedToken);
        mainAudioEndpoint->StateChanged(mainAudioEndpointStateChangedToken);
        mainAudioEndpoint->Unregister();
        mainAudioEndpoint->Release();

        mainAudioEndpoint = audioController->GetMainAudioEndpoint();
        if (mainAudioEndpoint->Register())
        {
            // Register to events.
            mainAudioEndpointVolumeChangedToken = mainAudioEndpoint->VolumeChanged({ this, &AudioViewer::MainAudioEndpoint_VolumeChanged });
            mainAudioEndpointStateChangedToken = mainAudioEndpoint->StateChanged([this](IInspectable, bool muted)
            {
                DispatcherQueue().TryEnqueue([this, muted]()
                {
                    MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());
                    MuteToggleButtonFontIcon().Glyph(muted ? L"\ue74f" : L"\ue767");
                });
            });
        }


        DispatcherQueue().TryEnqueue([&]()
        {
            mainAudioEndpointPeakTimer.Start();
                                                                     
            SystemVolumeSlider().Value(static_cast<double>(mainAudioEndpoint->Volume()) * 100.);
            MuteToggleButton().IsChecked(mainAudioEndpoint->Muted());

            ReloadAudioSessions();

            DefaultAudioEndpointName(mainAudioEndpoint->Name());
        });
    }

    void AudioViewer::DefaultAudioEndpointName(const winrt::hstring& value)
    {
        defaultAudioEndpointName = value;
        e_propertyChanged(*this, Xaml::Data::PropertyChangedEventArgs(L"DefaultAudioEndpointName"));
    }

    void AudioViewer::EnqueueString(const winrt::hstring& str)
    {
        if (messageBar)
        {
            messageBar.EnqueueString(str);
        }
    }

    void AudioViewer::EnqueueMessage(const winrt::Windows::Foundation::IInspectable& inspectable)
    {
        if (messageBar)
        {
            messageBar.EnqueueMessage(inspectable);
        }
    }
}
