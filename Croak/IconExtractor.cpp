#include "pch.h"
#include "IconExtractor.h"

#include <ocidl.h>
#include <libloaderapi.h>

using namespace winrt;
using namespace std;


namespace Croak::Imaging
{
	HICON IconExtractor::LoadIconFromPath(const wstring_view& resourcePath)
	{
		check_bool(!resourcePath.empty());

		wstring modulePath{};
		wstring iconId{};
		for (int i = resourcePath.size() - 1; i >= 0; i--)
		{
			if (resourcePath[i] == L',')
			{
				modulePath = resourcePath.substr(1, i - 1);
				iconId = resourcePath.substr(i + 1);
				break;
			}
		}

		if (!PathIsRelative(modulePath.data())) __debugbreak();

		if (!modulePath.empty())
		{
#if FALSE
			// Get file full path.
			wchar_t fullPath[2048]{ 0 };
			wcscpy_s(fullPath, modulePath.c_str());
			wchar_t* finalComponent = nullptr;

			auto ret = PathResolve(fullPath, nullptr, PRF_REQUIREABSOLUTE);
			auto f = _wfullpath(fullPath, modulePath.c_str(), 2048);
			return nullptr;

			wchar_t* endPtr{};
			int iconResId = wcstol(iconId.data(), &endPtr, 10);
			HICON largeIcons[1];
			if (ExtractIconEx(modulePath.data(), iconResId, largeIcons, nullptr, 1) == 1 && largeIcons[0])
			{
				return largeIcons[0];
			}
			else
			{
				throw_last_error();
			}

			/*HMODULE moduleHandle{};
			if (SUCCEEDED(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, modulePath.data(), &moduleHandle)))
			{
				HANDLE imageHandle = LoadImage(moduleHandle, iconId.data(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
				if (!imageHandle)
				{
					throw_last_error();
					DEBUG_BREAK();
				}

				HICON hIcon = static_cast<HICON>(imageHandle);
				return hIcon;
			}*/
#endif
		}
		else
		{
			// i.e. firefox.exe
			HICON largeIcon[1]{};
			if (ExtractIconEx(resourcePath.data(), 0, largeIcon, nullptr, 1) == 1)
			{
				return largeIcon[0];
			}
		}

		return nullptr;
	}

	IStream* IconExtractor::ExtractStreamFromHICON(const HICON& hIcon)
	{
		// Create factory.
		com_ptr<IWICImagingFactory> wicImagingFactory{};
		check_hresult(
			CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicImagingFactory))
		);

		// Get bitmap and bitmap source from HICON.
		com_ptr<IWICBitmap> wicBitmap{};
		check_hresult(
			wicImagingFactory->CreateBitmapFromHICON(hIcon, wicBitmap.put())
		);
		com_ptr<IWICBitmapSource> wicBitmapSource = wicBitmap;

		IStream* bitmapStream = GetBitmapSourceStream(wicImagingFactory.get(), wicBitmapSource.get(), GUID_ContainerFormatBmp);
		return bitmapStream;
	}

	IWICBitmapSource* IconExtractor::ExtractHICONBitmapSource(const HICON& hIcon)
	{
		// Create factory.
		com_ptr<IWICImagingFactory> wicImagingFactory{};
		check_hresult(
			CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicImagingFactory))
		);

		// Get bitmap and bitmap source from HICON.
		IWICBitmap* wicBitmap = nullptr;
		check_hresult(
			wicImagingFactory->CreateBitmapFromHICON(hIcon, &wicBitmap)
		);

		return wicBitmap;
	}

	void IconExtractor::WriteHICONToFile(const HICON& hIcon, const std::filesystem::path& filePath)
	{
		// Create factory.
		com_ptr<IWICImagingFactory> wicImagingFactory{};
		check_hresult(
			CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicImagingFactory))
		);

		// Get bitmap and bitmap source from HICON.
		com_ptr<IWICBitmap> wicBitmap{};
		check_hresult(
			wicImagingFactory->CreateBitmapFromHICON(hIcon, wicBitmap.put())
		);
		com_ptr<IWICBitmapSource> wicBitmapSource = wicBitmap;

		// Load bitmap into a stream.
		//	Create stream.
		com_ptr<IStream> hIconStream{};
		check_hresult(CreateStreamOnHGlobal(nullptr, TRUE, hIconStream.put()));

		SaveImage(wicImagingFactory.get(), wicBitmapSource.get(), GUID_ContainerFormatPng, hIconStream.get());

		IStreamPtr outFileStream = nullptr;
		auto path = filePath.wstring();
		check_hresult(
			SHCreateStreamOnFileEx(path.data(), STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, FILE_ATTRIBUTE_NORMAL, true, nullptr, &outFileStream)
		);

		STATSTG statstg{};
		check_hresult(hIconStream->Stat(&statstg, STATFLAG_NONAME));
		check_hresult(IStream_Reset(hIconStream.get()));
		check_hresult(outFileStream->SetSize(statstg.cbSize));
		check_hresult(
			hIconStream->CopyTo(outFileStream, statstg.cbSize, nullptr, nullptr)
		);
		check_hresult(outFileStream->Commit(STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE));
	}

	void IconExtractor::SaveImage(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, const GUID& containerFormatGUID, IStream* hIconStream)
	{
		IStreamPtr bitmapSourceStream = GetBitmapSourceStream(imagingFactory, bitmapSource, containerFormatGUID);
		check_hresult(
			hIconStream->Seek(LARGE_INTEGER{ 0 }, STREAM_SEEK_SET, nullptr)
		);
		check_hresult(
			bitmapSourceStream->CopyTo(hIconStream, ULARGE_INTEGER{ UINT_MAX }, nullptr, nullptr)
		);
	}

	IStream* IconExtractor::GetBitmapSourceStream(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, const GUID& containerFormatGUID)
	{
		IStream* bitmapStream = nullptr;
		check_hresult(CreateStreamOnHGlobal(nullptr, true, &bitmapStream));

		com_ptr<IWICBitmapEncoder> wicBitmapEncoder{};
		check_hresult(
			imagingFactory->CreateEncoder(containerFormatGUID, nullptr, wicBitmapEncoder.put())
		);
		check_hresult(
			wicBitmapEncoder->Initialize(bitmapStream, WICBitmapEncoderCacheOption::WICBitmapEncoderNoCache)
		);

		AddFrameToWICBitmap(imagingFactory, bitmapSource, wicBitmapEncoder.get());

		check_hresult(
			wicBitmapEncoder->Commit()
		);

		LARGE_INTEGER largeInt{};
		bitmapStream->Seek(largeInt, STREAM_SEEK_SET, nullptr);
		return bitmapStream;
	}

	void IconExtractor::AddFrameToWICBitmap(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, IWICBitmapEncoder* bitmapEncoder)
	{
		com_ptr<IWICBitmapFrameEncode> bitmapFrameEncode{};
		com_ptr<IPropertyBag2> encoderOptions{};
		check_hresult(bitmapEncoder->CreateNewFrame(bitmapFrameEncode.put(), encoderOptions.put()));

		GUID containerFormat{};
		check_hresult(bitmapEncoder->GetContainerFormat(&containerFormat));
		// TODO: Understand.
		if ((containerFormat == GUID_ContainerFormatBmp))
		{
			// Write the encoder option to the property bag instance.
			VARIANT varValue{};

			varValue.vt = VT_BOOL;
			varValue.boolVal = VARIANT_TRUE;

			// Options to enable the v5 header support for 32bppBGRA.
			PROPBAG2 v5HeaderOption{};

			const std::wstring str = L"EnableV5Header32bppBGRA";
			v5HeaderOption.pstrName = (LPOLESTR)str.c_str();

			check_hresult(encoderOptions->Write(1, &v5HeaderOption, &varValue));
		}

		check_hresult(bitmapFrameEncode->Initialize(encoderOptions.get()));
		uint32_t width = 0, height = 0;
		check_hresult(bitmapSource->GetSize(&width, &height));
		check_hresult(bitmapFrameEncode->SetSize(width, height));

		com_ptr<IWICBitmapSource> bitmapSourceConverted{};
		bitmapSourceConverted.attach(ConvertBitmapPixelFormat(imagingFactory, bitmapSource, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone));

		WICRect rect{ 0, 0, static_cast<int>(width), static_cast<int>(height) };
		check_hresult(
			bitmapFrameEncode->WriteSource(bitmapSourceConverted.get(), &rect)
		);
		check_hresult(bitmapFrameEncode->Commit());
	}

	IWICBitmapSource* IconExtractor::ConvertBitmapPixelFormat(IWICImagingFactory* imagingFactory, IWICBitmapSource* bitmapSource, const WICPixelFormatGUID& sourcePixelFormat, const WICBitmapDitherType& bitmapDitherType)
	{
		com_ptr<IWICFormatConverter> wicFormatConverter{};
		check_hresult(imagingFactory->CreateFormatConverter(wicFormatConverter.put()));
		check_hresult(
			wicFormatConverter->Initialize(bitmapSource, sourcePixelFormat, bitmapDitherType, nullptr, 0., WICBitmapPaletteTypeCustom)
		);

		IWICBitmapSource* formatConverterBitmapSource = nullptr;
		check_hresult(
			wicFormatConverter->QueryInterface(__uuidof(IWICBitmapSource), (void**)(&formatConverterBitmapSource))
		);
		return formatConverterBitmapSource;
	}
}