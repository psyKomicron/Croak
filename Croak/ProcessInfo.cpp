#include "pch.h"
#include "ProcessInfo.h"

#include <appmodel.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include "IconHelper.h"
#include "DebugOutput.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::Data::Xml;
using namespace winrt::Windows::Data::Xml::Dom;


namespace Croak::System
{
    ProcessInfo::ProcessInfo(const PID& pid)
    {
        GetProcessInfo(pid);
    }


    void ProcessInfo::GetProcessInfo(const PID& pid)
    {
        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        check_pointer(processHandle);

        if (!GetProcessInfoUWP(processHandle))
        {
            OutputDebugHString(L"Failed to get process info from package (UWP-like) for PID " + to_hstring((uint64_t)pid));

            if (!GetProcessInfoWin32(processHandle))
            {
                OutputDebugHString(L"Failed to get process info from process handle for PID " + to_hstring((uint64_t)pid));

                DEBUG_BREAK(); // Failed to retreive infos for this process by searching through the app's manifest or using classic Win32 methods.

            }
        }

        CloseHandle(processHandle);
    }

    bool ProcessInfo::GetProcessInfoWin32(const HANDLE& processHandle)
    {
        bool success = false;

        wchar_t executableName[MAX_PATH]{};
        DWORD executableNameLength = MAX_PATH;
        if (QueryFullProcessImageName(processHandle, 0, executableName, &executableNameLength))
        {
            DWORD fileVersionInfoSize = GetFileVersionInfoSize(executableName, NULL);
            if (fileVersionInfoSize > 0)
            {
                void* lpData = malloc(fileVersionInfoSize);
                if (lpData)
                {
                    ZeroMemory(lpData, fileVersionInfoSize);

                    if (GetFileVersionInfo(executableName, 0UL, fileVersionInfoSize, lpData))
                    {
                        uint32_t cbTranslate = 0;
                        struct LANGANDCODEPAGE
                        {
                            WORD wLanguage;
                            WORD wCodePage;
                        } *lpTranslate = nullptr;

                        wchar_t strSubBlock[MAX_PATH]{ 0 };
                        if (VerQueryValue(lpData, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate))
                        {
                            // Check if the process only has 1 translation code file.
                            if ((cbTranslate / sizeof(LANGANDCODEPAGE)) == 1)
                            {
                                // Format the string to query the value later.
                                HRESULT hr = StringCchPrintf(
                                    strSubBlock,
                                    50,
                                    L"\\StringFileInfo\\%04x%04x\\FileDescription",
                                    lpTranslate[0].wLanguage,
                                    lpTranslate[0].wCodePage
                                );

                                if (SUCCEEDED(hr))
                                {
                                    wchar_t* lpBuffer = nullptr;
                                    uint32_t lpBufferSize = 0;
                                    if (VerQueryValue(lpData, strSubBlock, (void**)&lpBuffer, &lpBufferSize) && lpBufferSize > 0)
                                    {
                                        name = to_hstring(lpBuffer);
                                        success = true;
                                    }
                                }
                            }
                            else if ((cbTranslate / sizeof(LANGANDCODEPAGE)) > 1)
                            {
                                DEBUG_BREAK();
                            }
                        }
                    }

                    free(lpData);
                }
            }

            exePath = executableName;
        }

        return success;
    }

    bool ProcessInfo::GetProcessInfoUWP(const HANDLE& processHandle)
    {
        uint32_t applicationUserModelIdLength = 0;
        unique_ptr<wchar_t> applicationUserModelId = nullptr;

        bool success = GetProcessPackageInfo(processHandle);

        if (!success && (GetApplicationUserModelId(processHandle, &applicationUserModelIdLength, applicationUserModelId.get()) == ERROR_INSUFFICIENT_BUFFER))
        {
            applicationUserModelId = unique_ptr<wchar_t>(new wchar_t[applicationUserModelIdLength] { 0 });
            if (SUCCEEDED(GetApplicationUserModelId(processHandle, &applicationUserModelIdLength, applicationUserModelId.get())))
            {
                hstring shellPath = L"shell:appsfolder\\" + to_hstring(applicationUserModelId.get());

                com_ptr<IShellItem> shellItem = nullptr;
                if (SUCCEEDED(SHCreateItemFromParsingName(shellPath.c_str(), nullptr, IID_PPV_ARGS(&shellItem))))
                {
                    LPWSTR wStr = nullptr;
                    if (SUCCEEDED(shellItem->GetDisplayName(SIGDN::SIGDN_NORMALDISPLAY, &wStr)))
                    {
                        name = to_hstring(wStr);
                        CoTaskMemFree(wStr);
                    }

                    success = !name.empty();
                }
            }
        }

        return success;
    }

    bool ProcessInfo::GetProcessPackageInfo(const HANDLE& processHandle)
    {
        bool success = false;
        uint32_t packageFullNameLength = 0;
        long result = GetPackageFullName(processHandle, &packageFullNameLength, nullptr);
        if (result != ERROR_INSUFFICIENT_BUFFER)
        {
            return false;
        }

        unique_ptr<wchar_t> packageFullNameWstr{ new WCHAR[packageFullNameLength] { 0 } };
        if (SUCCEEDED(GetPackageFullName(processHandle, &packageFullNameLength, packageFullNameWstr.get())))
        {
            PACKAGE_INFO_REFERENCE packageInfoReference{};
            if (SUCCEEDED(OpenPackageInfoByFullName(packageFullNameWstr.get(), 0, &packageInfoReference)))
            {
                uint32_t bufferLength = 0;
                uint32_t count = 0;
                if (GetPackageInfo(packageInfoReference, PACKAGE_FILTER_HEAD, &bufferLength, nullptr, &count) == ERROR_INSUFFICIENT_BUFFER)
                {
                    BYTE* bytes = (BYTE*)malloc(bufferLength * sizeof(BYTE));
                    if (SUCCEEDED(GetPackageInfo(packageInfoReference, PACKAGE_FILTER_HEAD, &bufferLength, bytes, &count)))
                    {
                        try
                        {
                            PACKAGE_INFO* packageInfos = reinterpret_cast<PACKAGE_INFO*>(bytes);
                            if (packageInfos)
                            {
                                for (uint32_t i = 0; i < count; i++)
                                {
                                    PACKAGE_INFO packageInfo = packageInfos[i];

                                    // Create AppX factory.
                                    IAppxFactoryPtr factory = nullptr;
                                    check_hresult(CoCreateInstance(__uuidof(AppxFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));

                                    // Get the AppXManifest.xml path for the package.
                                    wchar_t appxManifest[2048](0); // 32 768 bits -- Supported by UTF-8 APIs
                                    PathCombine(appxManifest, packageInfo.path, L"AppXManifest.xml");
                                    if (lstrlen(appxManifest) > 0) // Check if PathCombine succeeded.
                                    {
                                        // Create stream and package reader to read package data.
                                        IStreamPtr streamPtr = nullptr;
                                        check_hresult(SHCreateStreamOnFileEx(appxManifest, STGM_SHARE_DENY_NONE, 0, false, nullptr, &streamPtr));

                                        IAppxManifestReaderPtr manifestReaderPtr = nullptr;
                                        check_hresult(factory->CreateManifestReader(streamPtr, &manifestReaderPtr));

                                        // Get the manifest data for the package.
                                        //  Get package display name through manifest properties.
                                        IAppxManifestPropertiesPtr properties = nullptr;
                                        manifestReaderPtr->GetProperties(&properties);

                                        LPWSTR packageDisplayNameProperty = nullptr;
                                        if (SUCCEEDED(properties->GetStringValue(L"DisplayName", &packageDisplayNameProperty)))
                                        {
                                            name = to_hstring(packageDisplayNameProperty);
                                            CoTaskMemFree(packageDisplayNameProperty);
                                        }

                                        //  Get manifest Applications node to get package data.
                                        IAppxManifestApplicationsEnumeratorPtr manifestResEnumeratorPtr = nullptr;
                                        manifestReaderPtr->GetApplications(&manifestResEnumeratorPtr);
                                        BOOL getHasCurrent = false;
                                        if (SUCCEEDED(manifestResEnumeratorPtr->GetHasCurrent(&getHasCurrent)) && getHasCurrent)
                                        {
                                            IAppxManifestApplicationPtr application = nullptr;
                                            if (FAILED(manifestResEnumeratorPtr->GetCurrent(&application)))
                                            {
                                                continue;
                                            }

                                            manifest = System::AppX::ManifestApplicationNode(application, packageInfo.path);
                                        }

                                        success = true;
                                    }
                                }
                            }
                        }
                        catch (const hresult_error& err)
                        {
                            OutputDebugHString(err.message());
                        }
                        catch (const std::exception& ex)
                        {
                            OutputDebugHString(to_hstring(ex.what()));
                        }
                    }

                    free(bytes);
                }
                else
                {
                    OutputDebugString(L"Failed to get package info.\n");
                }
            }
            else
            {
                OutputDebugString(L"Failed to open package.\n");
            }

            ClosePackageInfo(packageInfoReference);
        }
        else
        {
            OutputDebugString(L"Failed to get package full name.\n");
        }

        return success;
    }

    winrt::Windows::Foundation::IAsyncAction ProcessInfo::FindProcessIcon(std::wstring processIconPath)
    {
        for (int i = processIconPath.size() - 1; i >= 0; i--)
        {
            if (processIconPath[i] == L'\\')
            {
                wstring parentPath = processIconPath.substr(0, i + 1);

                wregex manifestRegex{ L".*(Manifest.xml)$" };

                vector<wstring> pathes{};

                WIN32_FIND_DATA findData{};
                HANDLE findHandle = FindFirstFile((parentPath + L"*").c_str(), &findData);
                if (findHandle != INVALID_HANDLE_VALUE)
                {
                    do
                    {
                        wstring filePath{ findData.cFileName };

                        if (regex_match(filePath, manifestRegex))
                        {
                            // Read the content of the file.
                            try
                            {
                                XmlDocument doc{};
                                co_await doc.LoadFromUriAsync(winrt::Windows::Foundation::Uri(parentPath + filePath));
                                auto node = doc.SelectSingleNode(L".\\Application\\VisualElements");
                            }
                            catch (hresult_error err)
                            {
                                OutputDebugHString(L"Could not load xml " + err.message());
                            }
                        }

                        if (filePath != L"." && filePath != L"..")
                        {
                            pathes.push_back(parentPath + filePath);
                        }
                    }
                    while (FindNextFile(findHandle, &findData));
                    FindClose(findHandle);
                }


            }
        }
    }
}