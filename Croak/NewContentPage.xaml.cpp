// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "NewContentPage.xaml.h"
#if __has_include("NewContentPage.g.cpp")
#include "NewContentPage.g.cpp"
#endif

#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.System.UserProfile.h>
#include <winrt/Windows.Storage.Streams.h>
#include "DebugOutput.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Data::Json;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;


namespace winrt::Croak::implementation
{
    NewContentPage::NewContentPage()
    {
        InitializeComponent();
    }

    void NewContentPage::OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const&)
    {
        SecondWindow::Current().Breadcrumbs().Append(NavigationBreadcrumbBarItem(L"What's new", xaml_typename<winrt::Croak::NewContentPage>()));
    }

    IAsyncAction NewContentPage::Page_Loading(FrameworkElement const&, IInspectable const&)
    {
        // Fetch update notes, localized.
        hstring text;
        Uri appUri{ L"ms-appx:///update_notes.json" };
        StorageFile file = co_await StorageFile::GetFileFromApplicationUriAsync(appUri);
        try
        {
            text = co_await FileIO::ReadTextAsync(file);
        }
        catch (const std::exception& ex)
        {
            OutputDebugHString(to_hstring(ex.what()));
        }
        catch (const hresult_error& err)
        {
            OutputDebugHString(err.message());
        }

        if (!text.empty())
        {
            JsonObject jsonObject{};
            if (JsonObject::TryParse(text, jsonObject))
            {
                // It would be strange that the user does not have languages installed on his machine, so we can assume that the collection will be at least of size 1.
                hstring languageTag = winrt::Windows::System::UserProfile::GlobalizationPreferences::Languages().GetAt(0);
                // Get the json I18N notes for the language.
                if (!jsonObject.HasKey(languageTag))
                {
                    // If the current Windows language is not supported, fall back to en-US.
                    languageTag = L"en-US";
                }

                if (jsonObject.HasKey(languageTag))
                {
                    JsonObject notes = jsonObject.GetNamedObject(languageTag);
                    auto changes = notes.GetNamedArray(L"changes");
                    auto bugs = notes.GetNamedArray(L"bugs");
                    auto news = notes.GetNamedArray(L"news");

                    for (auto&& item : news)
                    {
                        hstring title = item.GetObjectW().GetNamedString(L"title");
                        hstring content = item.GetObjectW().GetNamedString(L"text");

                        TextBlock titleBlock{};
                        titleBlock.Style(
                            Application::Current().Resources().Lookup(box_value(L"SubtitleTextBlockStyle")).as<::Style>()
                        );
                        titleBlock.FontSize(24);
                        titleBlock.Text(title);
                        TextBlock textBlock{};
                        textBlock.Text(content);
                        StackPanel panel{};
                        panel.Spacing(7);
                        panel.Margin(Thickness(0, 7, 0, 7));
                        panel.Children().Append(titleBlock);
                        panel.Children().Append(textBlock);
                        NewContentListView().Items().Append(panel);
                    }

                    for (auto&& item : changes)
                    {
                        TextBlock textBlock{};
                        textBlock.Text(item.GetString());
                        ChangesListView().Items().Append(textBlock);
                    }

                    for (auto&& item : bugs)
                    {
                        TextBlock textBlock{};
                        textBlock.Text(item.GetString());
                        BugsListView().Items().Append(textBlock);
                    }
                }
            }
        }
    }
}
