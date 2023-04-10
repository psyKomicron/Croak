#include "pch.h"
#include "AudioProfileEditPage.xaml.h"
#if __has_include("AudioProfileEditPage.g.cpp")
#include "AudioProfileEditPage.g.cpp"
#endif

#include "AudioController.h"
#include "AudioSession.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;


namespace winrt::Croak::implementation
{
    AudioProfileEditPage::AudioProfileEditPage()
    {
        InitializeComponent();

        timer = DispatcherQueue().CreateTimer();
        timer.Interval(ProfileSavedAnimation().Duration().TimeSpan + std::chrono::milliseconds(200));
        timer.IsRepeating(false);
        timer.Tick([this](IInspectable, IInspectable)
        {
            Frame().GoBack();
        });
    }

    void AudioProfileEditPage::OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& args)
    {
        audioProfile = args.Parameter().try_as<AudioProfile>();
        if (audioProfile == nullptr)
        {
            // Audio profile creation. The audio profiles page should send a unique temporary name for this audio profile.
            std::optional<hstring> tempName = args.Parameter().try_as<hstring>();
            if (!tempName)
            {
                navigationOutAllowed = true;
                Frame().GoBack();
            }
            else
            {
                audioProfile = AudioProfile();
                audioProfile.ProfileName(tempName.value());

                //winrt::Windows::ApplicationModel::Resources::ResourceLoader loader{};
                SecondWindow::Current().Breadcrumbs().Append(
                    NavigationBreadcrumbBarItem{ L"Audio profile creation", xaml_typename<winrt::Croak::AudioProfileEditPage>() }
                );
            }
        }
        else
        {
            SecondWindow::Current().Breadcrumbs().Append(
                NavigationBreadcrumbBarItem{ audioProfile.ProfileName(), xaml_typename<winrt::Croak::AudioProfileEditPage>() }
            );
        }
    }

    IAsyncAction AudioProfileEditPage::OnNavigatingFrom(winrt::Microsoft::UI::Xaml::Navigation::NavigatingCancelEventArgs const& args)
    {
        // If either the audio profile is null or if we can navigate out of the page, we ignore the save instructions.
        if (!audioProfile || navigationOutAllowed)
        {
            co_return;
        }

        // Check if changes have been made
        // 1. Check audio changes.
        double systemVolume = 0.0;
        hstring profileName;
        ::Collections::IMap<hstring, float> audioLevels;
        bool audioLevelsEqual = false;
        ::Collections::IMap<hstring, bool> audioStates;
        bool audioStatesEqual = false;

        // 2. Check settings changes.
        bool disableAnimations = DisableAnimationsCheckBox().IsChecked().GetBoolean();
        bool keepOnTop = KeepOnTopCheckBox().IsChecked().GetBoolean();
        bool showMenu = ShowMenuCheckBox().IsChecked().GetBoolean();
        /*
           Layout combo box items should be arranged so that :
             -First item is Auto
             -Second item is Horizontal
             -Third item is Vertical
        */
        uint32_t layout = LayoutComboBox().SelectedIndex();

        // Check equality.
#if 0
        if (
            systemVolume != audioProfile.SystemVolume() || isDefaultProfile != audioProfile.IsDefaultProfile() || profileName != audioProfile.ProfileName() ||
            !audioLevelsEqual || !audioStatesEqual ||
            disableAnimations != audioProfile.DisableAnimations() || keepOnTop != audioProfile.KeepOnTop() || showMenu != audioProfile.ShowMenu() || layout != audioProfile.Layout()
            )
        {
            args.Cancel(true);
            winrt::Windows::UI::Xaml::Interop::TypeName sourcePageType = args.SourcePageType();
            IInspectable parameter = args.Parameter();
            auto navigationTransitionInfo = args.NavigationTransitionInfo();

            navigationOutAllowed = true;
            // Primary button is save.
            if (co_await ConfirmDialog().ShowAsync() == winrt::Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary)
            {
                SaveProfile();
            }
            else // Secondary button is ignore/continue navigation
            {
                Frame().Navigate(sourcePageType, parameter, navigationTransitionInfo);
            }
        }
#endif // 0

    }

    void AudioProfileEditPage::Page_Loading(FrameworkElement const&, IInspectable const&)
    {
        //KeepOnTopCheckBox().IsChecked(audioProfile.KeepOnTop());
        ProfileNameEditTextBox().Text(audioProfile.ProfileName());
        DisableAnimationsCheckBox().IsChecked(audioProfile.DisableAnimations());
        ShowMenuCheckBox().IsChecked(audioProfile.ShowMenu());
        LayoutComboBox().SelectedIndex(static_cast<int32_t>(audioProfile.Layout()));

        ::Croak::Audio::AudioController* controllerPtr = new ::Croak::Audio::AudioController(GUID());
        if (audioProfile.DefaultAudioEndpointSettings().AudioLevel() < 0)
        {
            winrt::com_ptr<::Croak::Audio::DefaultAudioEndpoint> endpoint{};
            endpoint.attach(controllerPtr->GetMainAudioEndpoint());
            float value = endpoint->Volume();
            audioProfile.DefaultAudioEndpointSettings().AudioLevel(value);
        }
        SystemVolumeSlider().Value(static_cast<double>(audioProfile.DefaultAudioEndpointSettings().AudioLevel()) * 100.);

        if (audioProfile.AudioSessionsSettings().Size() > 0)
        {
            std::vector<AudioSessionView> views{};
            views.resize(audioProfile.SessionsIndexes().Size());

            for (auto&& settings : audioProfile.AudioSessionsSettings())
            {
                hstring sessionName = settings.Name();
                double volume = settings.AudioLevel() * 100.;
                bool muted = settings.Muted();
                uint32_t index = audioProfile.SessionsIndexes().Lookup(settings.Name());

                views[index] = CreateAudioSessionView(sessionName, volume, muted);
            }

            audioSessions = single_threaded_observable_vector<AudioSessionView>(std::move(views));
        }
        else
        {
            // Get audio sessions.
            std::vector<::Croak::Audio::AudioSession*>* audioSessionsPtr = controllerPtr->GetSessions();
            for (size_t i = 0; i < audioSessionsPtr->size(); i++)
            {
                winrt::com_ptr<::Croak::Audio::AudioSession> audioSession{};
                audioSession.attach(audioSessionsPtr->at(i));
                // Create view.
                audioSessions.Append(
                    CreateAudioSessionView(hstring(audioSession->Name()), audioSession->Volume() * 100., audioSession->Muted())
                );
            }
            delete audioSessionsPtr;
        }

        controllerPtr->Release(); // Release audio controller and associated resources.

        // Load audio profiles names to warn user from overwriting another profile.
        ApplicationDataContainer audioProfilesContainer = ApplicationData::Current().LocalSettings().Containers().Lookup(L"AudioProfiles");
        for (auto&& container : audioProfilesContainer.Containers())
        {
            if (container.Key() != audioProfile.ProfileName())
            {
                existingProfileNames.push_back(container.Key());
            }
        }
    }

    void AudioProfileEditPage::NextButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        navigationOutAllowed = true;

        SecondWindow::Current().Breadcrumbs().RemoveAtEnd();

        SaveProfile();
    }

    void AudioProfileEditPage::CancelButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        navigationOutAllowed = true;

        SecondWindow::Current().Breadcrumbs().RemoveAtEnd();

        Frame().GoBack();
    }

    void AudioProfileEditPage::ProfileNameEditTextBox_TextChanged(IInspectable const&, ::Controls::TextChangedEventArgs const&)
    {
        // TODO: Check if the profile name is already in use.
        hstring newName = ProfileNameEditTextBox().Text();
        for (hstring name : existingProfileNames)
        {
            if (name == newName)
            {
                ProfileNameEditTextBox().Foreground(
                    Application::Current().Resources().Lookup(box_value(L"SystemErrorTextColor")).as<::Media::Brush>()
                );
                return;
            }
        }

        ProfileNameEditTextBox().Foreground(
            Application::Current().Resources().Lookup(box_value(L"ApplicationForegroundThemeBrush")).as<::Media::Brush>()
        );
    }

    IAsyncAction AudioProfileEditPage::CreateAudioSessionButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (co_await AudioProfileCreationDialog().ShowAsync() == ContentDialogResult::Primary)
        {
            hstring sessionName = ProfileCreationTextBox().Text();
            double volume = ProfileCreationSlider().Value();
            bool muted = ProfileCreationCheckBox().IsChecked().GetBoolean();

            AudioSessionView view = CreateAudioSessionView(sessionName, volume, muted);
            if (view)
            {
                audioSessions.Append(view);
            }
        }
    }

    IAsyncAction AudioProfileEditPage::AddAudioSessionButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // Get audio sessions.
        ::Croak::Audio::AudioController* controllerPtr = new ::Croak::Audio::AudioController(GUID());
        std::vector<::Croak::Audio::AudioSession*>* audioSessionsPtr = controllerPtr->GetSessions();
        for (size_t i = 0; i < audioSessionsPtr->size(); i++)
        {
            // Create view.
            AudioSessionView view = CreateAudioSessionView(
                hstring(audioSessionsPtr->at(i)->Name()),
                audioSessionsPtr->at(i)->Volume() * 100.,
                audioSessionsPtr->at(i)->Muted()
            );

            if (view)
            {
                ProfileAddGridView().Items().Append(view);
            }

            audioSessionsPtr->at(i)->Release(); // Directly release the AudioSession and release COM resources.
        }
        delete audioSessionsPtr;
        controllerPtr->Release(); // Release audio controller and associated resources.

        co_await AudioProfileAddDialog().ShowAsync();
        //TODO: Add the user selected audio sessions to audioSessions.
        auto&& selectedItems = ProfileAddGridView().SelectedItems();
        for (auto&& selectedItem : selectedItems)
        {
            if (AudioSessionView view = selectedItem.try_as<AudioSessionView>())
            {
                audioSessions.Append(AudioSessionView(view));
            }
        }
    }

    void AudioProfileEditPage::LockSessionAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring header = sender.as<FrameworkElement>().Tag().as<hstring>();
        for (uint32_t i = 0; i < audioSessions.Size(); i++)
        {
            if (audioSessions.GetAt(i).Header() == header)
            {
                VisualStateManager::GoToState(audioSessions.GetAt(i), L"Locked", true);
                break;
            }
        }
    }

    void AudioProfileEditPage::RemoveSessionAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        hstring header = sender.as<FrameworkElement>().Tag().as<hstring>();
        for (uint32_t i = 0; i < audioSessions.Size(); i++)
        {
            if (audioSessions.GetAt(i).Header() == header)
            {
                audioSessions.RemoveAt(i);
                break;
            }
        }
    }

    void AudioProfileEditPage::ProfileCreationTextBox_TextChanged(IInspectable const&, TextChangedEventArgs const&)
    {
        hstring newName = ProfileCreationTextBox().Text();
        for (AudioSessionView&& view : audioSessions)
        {
            if (view.Header() == newName)
            {
                ProfileCreationTextBox().Foreground(
                    Application::Current().Resources().Lookup(box_value(L"SystemErrorTextColor")).as<::Media::Brush>()
                );
                return;
            }
        }

        ProfileCreationTextBox().Foreground(
            Application::Current().Resources().Lookup(box_value(L"ApplicationForegroundThemeBrush")).as<::Media::Brush>()
        );
    }


    void AudioProfileEditPage::SaveProfile()
    {
        VisualStateManager::GoToState(*this, L"ProfileSaved", true);

        audioProfile.ProfileName(ProfileNameEditTextBox().Text());
        audioProfile.DefaultAudioEndpointSettings().AudioLevel(static_cast<float>(SystemVolumeSlider().Value()) / 100.f);

        audioProfile.AudioSessionsSettings().Clear();
        for (auto&& view : AudioSessions())
        {
            hstring header = view.Header();
            bool isMuted = view.Muted();
            float volume = static_cast<float>(view.Volume()) / 100.f;
            audioProfile.AudioSessionsSettings().Append(AudioSessionSettings(header, isMuted, volume));

            uint32_t index = 0;
            AudioSessions().IndexOf(view, index);
            audioProfile.SessionsIndexes().Insert(header, index);
        }

        //audioProfile.KeepOnTop(KeepOnTopCheckBox().IsChecked().GetBoolean());
        audioProfile.DisableAnimations(DisableAnimationsCheckBox().IsChecked().GetBoolean());
        audioProfile.ShowMenu(ShowMenuCheckBox().IsChecked().GetBoolean());
        audioProfile.Layout(static_cast<AudioSessionLayout>(LayoutComboBox().SelectedIndex()));

        timer.Start();

        // Save the profile.
        ApplicationDataContainer audioProfilesContainer = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"AudioProfiles");
        if (!audioProfilesContainer)
        {
            audioProfilesContainer = ApplicationData::Current().LocalSettings().CreateContainer(L"AudioProfiles", ApplicationDataCreateDisposition::Always);
        }
        audioProfile.Save(audioProfilesContainer);
    }

    AudioSessionView AudioProfileEditPage::CreateAudioSessionView(hstring header, float volume, bool muted)
    {
        for (auto&& audioSession : audioSessions)
        {
            if (audioSession.Header() == header)
            {
                return nullptr;
            }
        }

        winrt::Croak::AudioSessionView view{};
        view.Header(header);
        view.Volume(volume);
        view.Muted(muted);

        CommandBarFlyout flyout{};

        TextBlock text{};
        FontIcon icon{};
        AppBarButton button{};
        text.Text(L"Lock");
        icon.Glyph(L"\ue72e");
        button.Content(std::move(text));
        button.Icon(std::move(icon));
        button.Click({ this, &AudioProfileEditPage::LockSessionAppBarButton_Click });
        button.Tag(box_value(header));
        flyout.PrimaryCommands().Append(std::move(button));

        text = TextBlock();
        icon = FontIcon();
        button = AppBarButton();
        text.Text(L"Remove");
        icon.Glyph(L"\ue74d");
        button.Content(std::move(text));
        button.Icon(std::move(icon));
        button.Click({ this, &AudioProfileEditPage::RemoveSessionAppBarButton_Click });
        button.Tag(box_value(header));
        flyout.PrimaryCommands().Append(button);

        flyout.SecondaryCommands().Append(AppBarButton());

        view.ContextFlyout(flyout);

        return view;
    }
}
