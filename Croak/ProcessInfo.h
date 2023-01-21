#pragma once
#include "ManifestApplicationNode.h"

namespace Croak::System
{
	typedef unsigned long PID;

	class ProcessInfo
	{
	public:
		ProcessInfo(const PID& pid);

		inline std::wstring_view Name()
		{
			return name;
		}

		inline System::AppX::ManifestApplicationNode Manifest()
		{
			return manifest;
		}

		inline std::wstring_view ExecutablePath()
		{
			return exePath;
		}

	private:
		Croak::System::AppX::ManifestApplicationNode manifest;
		std::wstring name{};
		std::wstring exePath{};

		void GetProcessInfo(const PID& pid);
		bool GetProcessInfoWin32(const HANDLE& processHandle);
		bool GetProcessInfoUWP(const HANDLE& processHandle);
		bool GetProcessPackageInfo(const HANDLE& processHandle);
		winrt::Windows::Foundation::IAsyncAction FindProcessIcon(std::wstring processIconPath);
	};
}

