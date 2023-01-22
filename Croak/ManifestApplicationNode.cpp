#include "pch.h"
#include "ManifestApplicationNode.h"

#include "DebugOutput.h"

using namespace std;


namespace Croak::System::AppX
{
	ManifestApplicationNode::ManifestApplicationNode(IAppxManifestApplicationPtr& application, PWSTR _packagePath)
	{
        wstring packagePath = wstring(_packagePath) + L"\\";

        wchar_t* value = nullptr;
        application->GetStringValue(L"Description", &value);
        if (value)
        {
            description = wstring(value);
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"DisplayName", &value);
        if (value)
        {
            displayName = wstring(value);
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"BackgroundColor", &value);
        if (value)
        {
            backgroundColor = wstring(value);
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"ForegroundText", &value);
        if (value)
        {
            foregroundText = wstring(value);
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"ShortName", &value);
        if (value)
        {
            shortName = wstring(value);
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"Logo", &value);
        if (value)
        {
            logo = GetScale(packagePath + wstring(value));
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"SmallLogo", &value);
        if (value)
        {
            smallLogo = GetScale(packagePath + wstring(value));
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"Square150x150Logo", &value);
        if (value)
        {
            square150x150Logo = GetScale(packagePath + wstring(value));
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"Square70x70Logo", &value);
        if (value)
        {
            square70x70Logo = GetScale(packagePath + wstring(value));
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"Square30x30Logo", &value);
        if (value)
        {
            square30x30Logo = GetScale(packagePath + wstring(value));
            CoTaskMemFree(value);
        }

        application->GetStringValue(L"Square44x44Logo", &value);
        if (value)
        {
            wstring logoPath = packagePath + wstring(value);
            wstring assetsPath{};
            CoTaskMemFree(value);

            // Extract the file name and extension to input target size.
            wstring filePathWithoutExt{};
            wstring ext{};
            for (int i = logoPath.size() - 1; i >= 0; i--)
            {
                if (logoPath[i] == '.')
                {
                    filePathWithoutExt = logoPath.substr(0, i);
                    ext = logoPath.substr(i);

                    // Extract the logos folder path.
                    for (int j = i; j >= 0; j--)
                    {
                        // Windows path delimiter.
                        if (logoPath[j] == '\\')
                        {
                            assetsPath = logoPath.substr(0, j + 1);
                            break;
                        }
                    }

                    break;
                }
            }

            wregex targetScaleRegx{ L"(.+targetsize-)(\\d+)(.*-unplated.*)" };
            WIN32_FIND_DATA findData{};
            HANDLE findHandle = FindFirstFile((filePathWithoutExt + L"*").c_str(), &findData);
            if (findHandle != INVALID_HANDLE_VALUE)
            {
                wstring maxRes{};
                int max = 0;
                do
                {
                    wstring fileName = wstring(findData.cFileName);
                    wcmatch matches{};
                    if (regex_match(fileName.c_str(), matches, targetScaleRegx) && matches.size() >= 3u)
                    {
                        int value = stoi(matches[2]);
                        if (value > max)
                        {
                            max = value;
                            maxRes = assetsPath + fileName;
                        }
                    }
                }
                while (FindNextFile(findHandle, &findData));
                FindClose(findHandle);

                OutputDebugHString(L"Best logo found target size : " + winrt::to_hstring(max) + L". File path : " + maxRes);
                square44x44Logo = maxRes;
                logo = maxRes;
            }

        }
	}
    std::wstring ManifestApplicationNode::GetScale(std::wstring path)
    {
        wstring filePathWithoutExt{};
        wstring ext{};
        for (int i = path.size() - 1; i >= 0; i--)
        {
            if (path[i] == '.')
            {
                filePathWithoutExt = path.substr(0, i);
                ext = path.substr(i);
                break;
            }
        }

        if (!filePathWithoutExt.empty() && !ext.empty())
        {
            wstring filePath = filePathWithoutExt + L".scale-100" + ext;
            if (PathFileExists(filePath.c_str()))
            {
                return filePath;
            }
        }

        return wstring();
    }
}
