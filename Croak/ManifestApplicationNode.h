#pragma once
#include "ComSmartPtrTypeDefs.h"

namespace Croak::System::AppX
{
	class ManifestApplicationNode
	{
	public:
		ManifestApplicationNode() = default;
		ManifestApplicationNode(IAppxManifestApplicationPtr& appxManifestApplicationPtr, PWSTR packagePath);

		inline std::wstring Description() const
		{
			return description;
		};

		inline std::wstring DisplayName() const
		{
			return displayName;
		};

		inline std::wstring Logo() const
		{
			return logo;
		}

		inline void Logo(const std::wstring& path)
		{
			logo = path;
		}

		inline std::wstring BackgroundColor() const
		{
			return backgroundColor;
		};

		inline std::wstring ForegroundText() const
		{
			return foregroundText;
		};

		inline std::wstring ShortName() const
		{
			return shortName;
		};

		inline std::wstring SmallLogo() const
		{
			return smallLogo;
		};

		inline std::wstring Square150Logo() const
		{
			return square150x150Logo;
		};

		inline std::wstring Square70Logo() const
		{
			return square70x70Logo;
		};

		inline std::wstring Square30Logo() const
		{
			return square30x30Logo;
		};

	private:
		std::wstring description{};
		std::wstring displayName{};
		std::wstring backgroundColor{};
		std::wstring foregroundText{};
		std::wstring shortName{};
		std::wstring logo{};
		std::wstring smallLogo{};
		std::wstring square30x30Logo{};
		std::wstring square70x70Logo{};
		std::wstring square150x150Logo{};
		std::wstring square44x44Logo{};

		std::wstring GetScale(std::wstring path);
	};
}