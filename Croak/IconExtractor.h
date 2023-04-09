#pragma once
#include <wincodec.h>

namespace Croak::Imaging
{
	class IconExtractor
	{
	public:
		IconExtractor() = default;

		HICON LoadIconFromPath(const std::wstring_view& resourcePath);
		IStream* ExtractStreamFromHICON(const HICON& hIcon);
		IWICBitmapSource* ExtractHICONBitmapSource(const HICON& hIcon);
		winrt::com_ptr<HICON> LoadIconFromPath2(const std::wstring_view& resPath);

	private:
		/**
		 * @brief Writes the given HICON to a file, specified by the filePath parameter.
		 * @param hIcon HICON representing the bitmap/image
		 * @param filePath Path of the file. The programm must have r+w permissions in that folder.
		*/
		void WriteHICONToFile(const HICON& hIcon, const std::filesystem::path& filePath);
		void SaveImage(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, const GUID& containerFormatGUID, IStream* hIconStream);
		IStream* GetBitmapSourceStream(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, const GUID& containerFormatGUID);
		void AddFrameToWICBitmap(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, IWICBitmapEncoder* bitmapEncoder);
		IWICBitmapSource* ConvertBitmapPixelFormat(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, const WICPixelFormatGUID& sourcePixelFormat, const WICBitmapDitherType& bitmapDitherType);
	};
}

