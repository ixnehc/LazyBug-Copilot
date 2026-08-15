#include "stdh.h"
#include "Utils.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <atlimage.h>
#include <gdiplus.h>
#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <DirectXTex.h>

#pragma comment(lib, "gdiplus.lib")

// 引入字符串转换函数声明
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern std::string widechar_to_utf8(const wchar_t* str);

namespace Utils
{

	// 辅助函数：确保GDI+已初始化
static class GdiplusInitializer
{
public:
	GdiplusInitializer()
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	}
	~GdiplusInitializer()
	{
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
	}
private:
	ULONG_PTR m_gdiplusToken;
} s_gdiplusInitializer;



// Base64 编码字符映射表
static const char* BASE64_CHARS = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

// Base64 编码函数（内部使用）
// 直接在传入的 output 字符串上进行编码，避免额外的拷贝
static void Base64Encode(std::string& output, const BYTE* bytes_to_encode, size_t in_len)
{
	output.clear();
	output.reserve(in_len * 4 / 3 + 4); // 预分配空间以提高性能

	int i = 0;
	int j = 0;
	BYTE char_array_3[3];
	BYTE char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			// 将 3 个 8位字节 (24位) 转换为 4 个 6位字节
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; i < 4; i++)
				output += BASE64_CHARS[char_array_4[i]];
			i = 0;
		}
	}

	// 处理末尾不足 3 字节的情况 (补 '=' 号)
	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; j < i + 1; j++)
			output += BASE64_CHARS[char_array_4[j]];

		while (i++ < 3)
			output += '=';
	}
}

bool GetFileContentIntoBase64(const char* path, std::string& content)
{
	content.clear();

	if (!path || *path == '\0')
		return false;

	// 使用现有的 LoadFileContent 函数读取文件内容到 vector<BYTE>
	std::vector<BYTE> fileContent;
	if (!LoadFileContent(path, fileContent))
		return false;

	// 如果文件为空，返回空字符串（对于空图片文件这种情况也是合理的）
	if (fileContent.empty())
	{
		content.clear();
		return true;
	}

	// 将文件内容进行 Base64 编码（直接在 content 上编码）
	Base64Encode(content, fileContent.data(), fileContent.size());
	return true;
}

bool LoadImageThumbnailIntoBase64(const char* path, int maxWidth, int maxHeight, std::string& content)
{
	content.clear();

	if (!path || *path == '\0')
		return false;

	if (maxWidth <= 0 || maxHeight <= 0)
		return false;

	// 获取文件扩展名以确定原始格式
	std::string ext = GetFileSuffix(std::string(path));
	StringLower(ext);

	// 使用 CImage 加载图片
	CImage srcImage;
	HRESULT hr = E_FAIL;

	hr = srcImage.Load(path);

	if (FAILED(hr))
		return false;

	int srcWidth = srcImage.GetWidth();
	int srcHeight = srcImage.GetHeight();

	if (srcWidth <= 0 || srcHeight <= 0)
		return false;

	// 计算保持宽高比的缩放比例
	double scaleX = (double)maxWidth / srcWidth;
	double scaleY = (double)maxHeight / srcHeight;
	double scale = (scaleX < scaleY) ? scaleX : scaleY;

	// 如果图片已经小于最大尺寸，不需要缩放，直接读取原始文件内容
	if (scale >= 1.0)
	{
		return GetFileContentIntoBase64(path, content);
	}

	int destWidth = (int)(srcWidth * scale);
	int destHeight = (int)(srcHeight * scale);

	// 确保最小尺寸为1
	if (destWidth < 1) destWidth = 1;
	if (destHeight < 1) destHeight = 1;

	// 创建目标图片
	CImage destImage;
	if (!destImage.Create(destWidth, destHeight, 32, 0))
		return false;

	// 使用高质量缩放
	HDC hDestDC = destImage.GetDC();
	HDC hSrcDC = srcImage.GetDC();

	// 设置拉伸模式为高质量
	int oldStretchMode = SetStretchBltMode(hDestDC, HALFTONE);

	// 执行缩放
	BOOL bltResult = srcImage.StretchBlt(hDestDC, 0, 0, destWidth, destHeight, SRCCOPY);

	// 恢复原来的拉伸模式
	SetStretchBltMode(hDestDC, oldStretchMode);

	// 释放DC
	srcImage.ReleaseDC();
	destImage.ReleaseDC();

	if (!bltResult)
		return false;

	// 将缩放后的图片保存到内存流
	IStream* pStream = nullptr;
	HRESULT hrStream = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	if (FAILED(hrStream) || pStream == nullptr)
		return false;

	// 根据原始格式保存
	if (ext == "jpg" || ext == "jpeg")
	{
		// JPEG格式
		hr = destImage.Save(pStream, Gdiplus::ImageFormatJPEG);
	}
	else
	{
		// PNG格式（默认）和其他格式
		hr = destImage.Save(pStream, Gdiplus::ImageFormatPNG);
	}

	if (FAILED(hr))
	{
		pStream->Release();
		return false;
	}

	// 获取流的大小
	STATSTG statstg;
	hr = pStream->Stat(&statstg, STATFLAG_DEFAULT);
	if (FAILED(hr))
	{
		pStream->Release();
		return false;
	}

	// 回到流的开始位置
	LARGE_INTEGER li = { 0 };
	hr = pStream->Seek(li, STREAM_SEEK_SET, NULL);
	if (FAILED(hr))
	{
		pStream->Release();
		return false;
	}

	// 读取流中的数据
	size_t dataSize = (size_t)statstg.cbSize.QuadPart;
	std::vector<BYTE> imageData(dataSize);
	ULONG bytesRead = 0;
	hr = pStream->Read(imageData.data(), (ULONG)dataSize, &bytesRead);
	pStream->Release();

	if (FAILED(hr) || bytesRead != dataSize)
		return false;

	// 将图片数据进行 Base64 编码
	Base64Encode(content, imageData.data(), dataSize);

	return true;
}

// ---- 图片编码辅助函数 ----

// 获取 GDI+ 编码器 CLSID
static int GetEncoderClsid(const wchar_t* format, CLSID* pClsid)
{
	UINT num = 0, size = 0;
	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	std::vector<BYTE> buffer(size);
	Gdiplus::ImageCodecInfo* pEncoders = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
	Gdiplus::GetImageEncoders(num, size, pEncoders);

	for (UINT i = 0; i < num; i++)
	{
		if (wcscmp(pEncoders[i].MimeType, format) == 0)
		{
			*pClsid = pEncoders[i].Clsid;
			return (int)i;
		}
	}
	return -1;
}

// 将 IStream 内容读取并编码为 base64
static bool StreamToBase64(IStream* pStream, std::string& outContent)
{
	STATSTG statstg;
	if (FAILED(pStream->Stat(&statstg, STATFLAG_DEFAULT)))
	{
		pStream->Release();
		return false;
	}

	LARGE_INTEGER li = { 0 };
	if (FAILED(pStream->Seek(li, STREAM_SEEK_SET, NULL)))
	{
		pStream->Release();
		return false;
	}

	size_t dataSize = (size_t)statstg.cbSize.QuadPart;
	std::vector<BYTE> imageData(dataSize);
	ULONG bytesRead = 0;
	HRESULT hrRead = pStream->Read(imageData.data(), (ULONG)dataSize, &bytesRead);
	pStream->Release();

	if (FAILED(hrRead) || bytesRead != dataSize)
		return false;

	Base64Encode(outContent, imageData.data(), dataSize);
	return true;
}

// 将 Gdiplus::Bitmap 保存为 base64（JPEG 或 PNG）
static bool SaveGdiplusBitmapToBase64(Gdiplus::Bitmap* pBitmap, bool asJpeg, std::string& outContent)
{
	if (!pBitmap)
		return false;

	CLSID clsid;
	const wchar_t* mime = asJpeg ? L"image/jpeg" : L"image/png";
	if (GetEncoderClsid(mime, &clsid) < 0)
		return false;

	IStream* pStream = nullptr;
	if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &pStream)) || !pStream)
		return false;

	if (pBitmap->Save(pStream, &clsid, nullptr) != Gdiplus::Ok)
	{
		pStream->Release();
		return false;
	}

	return StreamToBase64(pStream, outContent);
}

// 使用 DirectXTex 解码 DDS/TGA，返回 Gdiplus::Bitmap（以 Image 基类指针返回）
Gdiplus::Image* LoadImageWithDirectXTex(const std::wstring& imagePath)
{
	DirectX::TexMetadata metadata = {};
	DirectX::ScratchImage sourceImage;
	std::string utf8Path = widechar_to_utf8(imagePath.c_str());
	std::string ext = GetFileSuffix(utf8Path);
	StringLower(ext);

	HRESULT hr = E_FAIL;
	if (ext == "dds")
	{
		hr = DirectX::LoadFromDDSFile(imagePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, sourceImage);
	}
	else if (ext == "tga")
	{
		hr = DirectX::LoadFromTGAFile(imagePath.c_str(), &metadata, sourceImage);
	}
	if (FAILED(hr) || sourceImage.GetImageCount() == 0)
		return nullptr;

	// DDS 常见格式分两类：
	// - BC1/BC2/BC3/BC4/BC5/BC6H/BC7 等压缩格式：必须先用 Decompress 解压。
	// - R8G8B8A8/B8G8R8A8 等普通像素格式：用 Convert 转换格式即可。
	DirectX::ScratchImage workingImage;
	if (DirectX::IsCompressed(metadata.format))
	{
		hr = DirectX::Decompress(
			sourceImage.GetImages(),
			sourceImage.GetImageCount(),
			metadata,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			workingImage);
	}
	else
	{
		hr = DirectX::Convert(
			sourceImage.GetImages(),
			sourceImage.GetImageCount(),
			metadata,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DirectX::TEX_FILTER_DEFAULT,
			0.0f,
			workingImage);
	}
	if (FAILED(hr) || workingImage.GetImageCount() == 0)
		return nullptr;

	const DirectX::Image* image = workingImage.GetImages();
	if (!image || image->width == 0 || image->height == 0 || !image->pixels)
		return nullptr;

	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(
		static_cast<INT>(image->width),
		static_cast<INT>(image->height),
		PixelFormat32bppARGB);
	if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok)
	{
		delete bitmap;
		return nullptr;
	}

	Gdiplus::BitmapData bitmapData = {};
	Gdiplus::Rect rect(0, 0, static_cast<INT>(image->width), static_cast<INT>(image->height));
	if (bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData) != Gdiplus::Ok)
	{
		delete bitmap;
		return nullptr;
	}

	const size_t rowBytes = static_cast<size_t>(image->width) * 4;
	for (size_t y = 0; y < image->height; ++y)
	{
		memcpy(
			static_cast<BYTE*>(bitmapData.Scan0) + y * bitmapData.Stride,
			image->pixels + y * image->rowPitch,
			rowBytes);
	}
	bitmap->UnlockBits(&bitmapData);
	return bitmap;
}

// ---- LoadImageIntoBase64 ----

bool LoadImageIntoBase64(const char* path, int maxWidth, int maxHeight, std::string& content, std::string& mimeType)
{
	content.clear();
	mimeType.clear();

	if (!path || *path == '\0')
		return false;

	std::string ext = GetFileSuffix(std::string(path));
	StringLower(ext);

	// 判断是否为 LLM 原生支持的格式（可直接传原始字节）
	auto isNativelySupported = [](const std::string& ext) -> bool {
		return ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "webp" || ext == "gif";
	};

	// 仅对原生格式返回对应 mime，其余格式转换后统一为 PNG/JPEG
	auto getNativeMime = [](const std::string& ext) -> const char* {
		if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
		if (ext == "png")  return "image/png";
		if (ext == "webp") return "image/webp";
		if (ext == "gif")  return "image/gif";
		return nullptr;
	};

	bool needResize = (maxWidth > 0 && maxHeight > 0);

	// 原生支持且不需要缩放：直接返回原始字节
	if (isNativelySupported(ext) && !needResize)
	{
		if (GetFileContentIntoBase64(path, content))
		{
			mimeType = getNativeMime(ext);
			return true;
		}
		return false;
	}

	// 需要解码（用于缩放或格式转换）
	CImage srcImage;
	std::wstring wPath = utf8_to_widechar(path);
	HRESULT hr = srcImage.Load(path);
	if (FAILED(hr))
	{
		// CImage 解码失败时，使用 DirectXTex 处理 DDS/TGA。
		Gdiplus::Image* pDirectXTexImage = nullptr;
		if (ext == "dds" || ext == "tga")
			pDirectXTexImage = LoadImageWithDirectXTex(wPath);

		if (pDirectXTexImage)
		{
			Gdiplus::Bitmap* bitmap = static_cast<Gdiplus::Bitmap*>(pDirectXTexImage);
			int imageWidth = bitmap->GetWidth();
			int imageHeight = bitmap->GetHeight();
			bool actuallyResize = false;
			double scale = 1.0;
			if (needResize)
			{
				double scaleX = (double)maxWidth / imageWidth;
				double scaleY = (double)maxHeight / imageHeight;
				scale = (scaleX < scaleY) ? scaleX : scaleY;
				actuallyResize = (scale < 1.0);
			}

			bool outputJpeg = (ext == "jpg" || ext == "jpeg");
			bool converted = false;
			if (actuallyResize)
			{
				int destWidth = (int)(imageWidth * scale);
				int destHeight = (int)(imageHeight * scale);
				if (destWidth < 1) destWidth = 1;
				if (destHeight < 1) destHeight = 1;

				Gdiplus::Bitmap destBitmap(destWidth, destHeight, PixelFormat32bppPARGB);
				Gdiplus::Graphics graphics(&destBitmap);
				graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
				if (graphics.DrawImage(bitmap, 0, 0, destWidth, destHeight) == Gdiplus::Ok)
					converted = SaveGdiplusBitmapToBase64(&destBitmap, outputJpeg, content);
			}
			else
			{
				converted = SaveGdiplusBitmapToBase64(bitmap, outputJpeg, content);
			}

			delete pDirectXTexImage;
			if (converted)
			{
				mimeType = outputJpeg ? "image/jpeg" : "image/png";
				return true;
			}
		}

		// 解码失败时，如果是原生格式，仍退回直接返回原始字节。
		if (isNativelySupported(ext))
		{
			if (GetFileContentIntoBase64(path, content))
			{
				mimeType = getNativeMime(ext);
				return true;
			}
		}
		return false;
	}

	int srcWidth = srcImage.GetWidth();
	int srcHeight = srcImage.GetHeight();
	if (srcWidth <= 0 || srcHeight <= 0)
		return false;

	// 判断是否真的需要缩放
	bool actuallyResize = false;
	double scale = 1.0;
	if (needResize)
	{
		double scaleX = (double)maxWidth / srcWidth;
		double scaleY = (double)maxHeight / srcHeight;
		scale = (scaleX < scaleY) ? scaleX : scaleY;
		actuallyResize = (scale < 1.0);
	}

	// 不需要缩放且原生支持：直接返回原始字节
	if (!actuallyResize && isNativelySupported(ext))
	{
		if (GetFileContentIntoBase64(path, content))
		{
			mimeType = getNativeMime(ext);
			return true;
		}
		return false;
	}

	// 需要重新编码（缩放或格式转换）
	// jpg/jpeg 源 → 输出 JPEG；其余 → 输出 PNG
	bool outputJpeg = (ext == "jpg" || ext == "jpeg");

	// 辅助 lambda：将 CImage 保存到 base64
	auto saveImageToBase64 = [](CImage& img, bool asJpeg, std::string& outContent) -> bool
	{
		IStream* pStream = nullptr;
		HRESULT hrStream = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
		if (FAILED(hrStream) || pStream == nullptr)
			return false;

		HRESULT hrSave = img.Save(pStream, asJpeg ? Gdiplus::ImageFormatJPEG : Gdiplus::ImageFormatPNG);
		if (FAILED(hrSave))
		{
			pStream->Release();
			return false;
		}

		STATSTG statstg;
		if (FAILED(pStream->Stat(&statstg, STATFLAG_DEFAULT)))
		{
			pStream->Release();
			return false;
		}

		LARGE_INTEGER li = { 0 };
		if (FAILED(pStream->Seek(li, STREAM_SEEK_SET, NULL)))
		{
			pStream->Release();
			return false;
		}

		size_t dataSize = (size_t)statstg.cbSize.QuadPart;
		std::vector<BYTE> imageData(dataSize);
		ULONG bytesRead = 0;
		HRESULT hrRead = pStream->Read(imageData.data(), (ULONG)dataSize, &bytesRead);
		pStream->Release();

		if (FAILED(hrRead) || bytesRead != dataSize)
			return false;

		Base64Encode(outContent, imageData.data(), dataSize);
		return true;
	};

	if (actuallyResize)
	{
		int destWidth = (int)(srcWidth * scale);
		int destHeight = (int)(srcHeight * scale);
		if (destWidth < 1) destWidth = 1;
		if (destHeight < 1) destHeight = 1;

		CImage destImage;
		if (!destImage.Create(destWidth, destHeight, 32, 0))
			return false;

		HDC hDestDC = destImage.GetDC();
		HDC hSrcDC = srcImage.GetDC();
		int oldStretchMode = SetStretchBltMode(hDestDC, HALFTONE);
		BOOL bltResult = srcImage.StretchBlt(hDestDC, 0, 0, destWidth, destHeight, SRCCOPY);
		SetStretchBltMode(hDestDC, oldStretchMode);
		srcImage.ReleaseDC();
		destImage.ReleaseDC();

		if (!bltResult)
			return false;

		if (!saveImageToBase64(destImage, outputJpeg, content))
			return false;
	}
	else
	{
		if (!saveImageToBase64(srcImage, outputJpeg, content))
			return false;
	}

	mimeType = outputJpeg ? "image/jpeg" : "image/png";
	return true;
}
static std::string CalculateBitmapHash(HBITMAP hBitmap)
{
	// 获取位图信息
	BITMAP bmp;
	if (!GetObject(hBitmap, sizeof(BITMAP), &bmp))
		return std::string();

	// 计算位图数据大小
	int width = bmp.bmWidth;
	int height = bmp.bmHeight;
	int bpp = bmp.bmBitsPixel;
	int pitch = ((width * bpp + 31) / 32) * 4; // 每行字节数（4字节对齐）
	size_t dataSize = pitch * height;

	// 分配缓冲区读取像素数据
	std::vector<BYTE> buffer(dataSize);
	if (!GetBitmapBits(hBitmap, (LONG)dataSize, buffer.data()))
		return std::string();

	// 使用FNV-1a哈希算法计算哈希值
	const uint32_t FNV_PRIME = 16777619u;
	const uint32_t FNV_OFFSET_BASIS = 2166136261u;
	uint32_t hash = FNV_OFFSET_BASIS;

	for (size_t i = 0; i < dataSize; ++i)
	{
		hash ^= buffer[i];
		hash *= FNV_PRIME;
	}

	// 将哈希值转换为16进制字符串
	char hashStr[16];
	snprintf(hashStr, sizeof(hashStr), "%08X", hash);
	return std::string(hashStr);
}

// 辅助函数：读取哈希文件内容
static std::string ReadHashFile(const std::string& hashFilePath)
{
	std::wstring wHashFilePath = utf8_to_widechar(hashFilePath);
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, wHashFilePath.c_str(), L"r") != 0 || fp == nullptr)
		return std::string();

	char hashStr[32] = {0};
	if (fgets(hashStr, sizeof(hashStr), fp))
	{
		// 去掉可能的换行符
		size_t len = strlen(hashStr);
		if (len > 0 && (hashStr[len-1] == '\n' || hashStr[len-1] == '\r'))
			hashStr[len-1] = '\0';
		if (len > 1 && (hashStr[len-2] == '\r'))
			hashStr[len-2] = '\0';
	}
	fclose(fp);
	return std::string(hashStr);
}
 
// 辅助函数：写入哈希文件
static void WriteHashFile(const std::string& hashFilePath, const std::string& hash)
{
	std::wstring wHashFilePath = utf8_to_widechar(hashFilePath);
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, wHashFilePath.c_str(), L"w") == 0 && fp != nullptr)
	{
		fprintf(fp, "%s\n", hash.c_str());
		fclose(fp);
	}
}

bool GetImageSize(const char* path, int& width, int& height) 
{
	width = height = 0;
	if (!path || *path == '\0')
		return false;

	FILE* f = nullptr;
	std::wstring wPath = utf8_to_widechar(path);
	if (_wfopen_s(&f, wPath.c_str(), L"rb") != 0 || !f)
		return false;

	uint8_t buf[30];
	size_t bytes_read = fread(buf, 1, sizeof(buf), f);

	if (bytes_read >= 24) 
	{
		// 1. 检查 PNG (Magic: 89 50 4E 47 ...)
		if (buf[0] == 0x89 && buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G') 
		{
			// PNG 的宽高信息固定在偏移 16 和 20 的位置，大端序
			width = (buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19];
			height = (buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | buf[23];
			fclose(f);
			return true;
		}

		// 2. 检查 WebP (RIFF .... WEBP)
		if (buf[0] == 'R' && buf[1] == 'I' && buf[2] == 'F' && buf[3] == 'F' &&
			buf[8] == 'W' && buf[9] == 'E' && buf[10] == 'B' && buf[11] == 'P') 
		{

			if (buf[12] == 'V' && buf[13] == 'P' && buf[14] == '8') 
			{
				if (buf[15] == ' ' && bytes_read >= 30) 
				{
					// 有损 WebP (VP8 )
					if (buf[23] == 0x9D && buf[24] == 0x01 && buf[25] == 0x2A) 
					{
						width = (buf[26] | (buf[27] << 8)) & 0x3FFF;
						height = (buf[28] | (buf[29] << 8)) & 0x3FFF;
						fclose(f);
						return true;
					}
				}
				else if (buf[15] == 'L' && bytes_read >= 25) 
				{
					// 无损 WebP (VP8L)
					if (buf[20] == 0x2F) 
					{
						uint32_t val = buf[21] | (buf[22] << 8) | (buf[23] << 16) | (buf[24] << 24);
						width = 1 + (val & 0x3FFF);
						height = 1 + ((val >> 14) & 0x3FFF);
						fclose(f);
						return true;
					}
				}
				else if (buf[15] == 'X' && bytes_read >= 30) 
				{
					// 扩展 WebP (VP8X)
					width = 1 + (buf[24] | (buf[25] << 8) | (buf[26] << 16));
					height = 1 + (buf[27] | (buf[28] << 8) | (buf[29] << 16));
					fclose(f);
					return true;
				}
			}
		}
	}

	// 3. 检查 JPEG (Magic: FF D8)
	if (bytes_read >= 2 && buf[0] == 0xFF && buf[1] == 0xD8) 
	{
		fseek(f, 2, SEEK_SET);
		uint8_t marker;
		while (fread(&marker, 1, 1, f) == 1) 
		{
			if (marker != 0xFF) continue;

			// 跳过连续的 0xFF 填充
			while (fread(&marker, 1, 1, f) == 1 && marker == 0xFF);

			if (marker == 0xDA) break; // SOS 标记说明头信息已结束，开始真正压缩数据了
			if ((marker >= 0xD0 && marker <= 0xD9) || marker == 0x01) continue; // 没有长度数据的 marker

			// 常见的 SOF0 - SOF3 (包含图片宽高)
			if (marker >= 0xC0 && marker <= 0xC3) 
			{
				uint8_t info[7];
				if (fread(info, 1, 7, f) == 7) 
				{
					height = (info[3] << 8) | info[4]; // JPEG 是大端序
					width = (info[5] << 8) | info[6];
					fclose(f);
					return true;
				}
			}
			else 
			{
				// 读取段长度并直接跳过，快速往后寻找SOF标记
				uint8_t len_buf[2];
				if (fread(len_buf, 1, 2, f) != 2) break;
				int len = (len_buf[0] << 8) | len_buf[1];
				if (len < 2) break; // 防止死循环
				fseek(f, len - 2, SEEK_CUR);
			}
		}
	}

	// 4. 检查 DDS (Magic: "DDS ")
	if (bytes_read >= 20 && buf[0] == 'D' && buf[1] == 'D' && buf[2] == 'S' && buf[3] == ' ')
	{
		// DDS 头：偏移 12 = dwHeight, 偏移 16 = dwWidth（小端 uint32）
		height = buf[12] | (buf[13] << 8) | (buf[14] << 16) | (buf[15] << 24);
		width  = buf[16] | (buf[17] << 8) | (buf[18] << 16) | (buf[19] << 24);
		fclose(f);
		return true;
	}

	fclose(f);

	// 对于 TGA，使用 DirectXTex 获取尺寸；CImage/GDI+ 通常不支持 TGA。
	std::string imageExt = GetFileSuffix(std::string(path));
	StringLower(imageExt);
	if (imageExt == "tga")
	{
		DirectX::TexMetadata metadata = {};
		DirectX::ScratchImage image;
		std::wstring wPath = utf8_to_widechar(path);
		if (SUCCEEDED(DirectX::LoadFromTGAFile(wPath.c_str(), &metadata, image)))
		{
			width = static_cast<int>(metadata.width);
			height = static_cast<int>(metadata.height);
			if (width > 0 && height > 0)
				return true;
		}
	}

	// 对于未通过文件头解析的格式（bmp, gif, tiff, ico 等），使用 CImage 解码获取尺寸
	{
		CImage img;
		if (SUCCEEDED(img.Load(path)))
		{
			width = img.GetWidth();
			height = img.GetHeight();
			if (width > 0 && height > 0)
				return true;
		}
	}

	return false;
}



std::string GenTempImageFromClipboard()
{
	// 从剪贴板获取图片数据
	if (!OpenClipboard(nullptr))
		return std::string();

	// 检查是否有位图格式
	BOOL hasBitmap = IsClipboardFormatAvailable(CF_BITMAP) || IsClipboardFormatAvailable(CF_DIB);
	if (!hasBitmap)
	{
		CloseClipboard();
		return std::string();
	}

	// 获取位图数据
	HBITMAP hBitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
	if (!hBitmap)
	{
		CloseClipboard();
		return std::string();
	}

	// 计算当前剪贴板图片的哈希值
	std::string currentHash = CalculateBitmapHash(hBitmap);
	if (currentHash.empty())
	{
		CloseClipboard();
		return std::string();
	}

	// 准备临时图片目录路径（UTF-8格式）
	std::string dbRoot = GetDBRootFolder_utf8();
	std::string tempImageFolder = dbRoot + "\\_tempimage";

	// 转换为宽字符路径进行文件操作
	std::wstring wTempImageFolder = utf8_to_widechar(tempImageFolder);

	// 确保目录存在（使用宽字符版本）
	EnsureFolder(tempImageFolder.c_str());

	// 序号文件路径（只记录当前最大序号）
	std::string indexFilePath = tempImageFolder + "\\_index.txt";
	std::wstring wIndexFilePath = utf8_to_widechar(indexFilePath);

	// 读取当前序号
	unsigned int currentIndex = 0;
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, wIndexFilePath.c_str(), L"r") == 0 && fp != nullptr)
	{
		fwscanf_s(fp, L"%u", &currentIndex);
		fclose(fp);
	}

	// 只检查最近10个文件（避免重复检查过多）
	const int CHECK_RECENT_COUNT = 10;
	int checkStart = (currentIndex > CHECK_RECENT_COUNT) ? (currentIndex - CHECK_RECENT_COUNT) : 0;

	for (int idx = currentIndex - 1; idx >= checkStart; --idx)
	{
		char hashFileName[64];
		snprintf(hashFileName, sizeof(hashFileName), "#%06d#.hash", (idx % 100000));
		std::string hashFilePath = tempImageFolder + "\\" + hashFileName;
		std::wstring wHashFilePath = utf8_to_widechar(hashFilePath);

		std::string existingHash = ReadHashFile(hashFilePath);
		if (!existingHash.empty() && existingHash == currentHash)
		{
			// 找到匹配的图片，检查PNG和JPG文件是否存在
			
			char pngFileName[64];
			snprintf(pngFileName, sizeof(pngFileName), "#%06d#.png", (idx % 100000));
			std::string pngPath = tempImageFolder + "\\" + pngFileName;
			std::wstring wPngPath = utf8_to_widechar(pngPath);
			
			char jpgFileName[64];
			snprintf(jpgFileName, sizeof(jpgFileName), "#%06d#.jpg", (idx % 100000));
			std::string jpgPath = tempImageFolder + "\\" + jpgFileName;
			std::wstring wJpgPath = utf8_to_widechar(jpgPath);
			
			// 检查哪个文件存在（优先返回PNG）
			FILE* testFp = nullptr;
			if (_wfopen_s(&testFp, wPngPath.c_str(), L"r") == 0 && testFp != nullptr)
			{
				fclose(testFp);
				CloseClipboard();
				return pngPath;
			}
			else if (_wfopen_s(&testFp, wJpgPath.c_str(), L"r") == 0 && testFp != nullptr)
			{
				fclose(testFp);
				CloseClipboard();
				return jpgPath;
			}
		}
	}

	// 没有找到匹配，需要创建新文件
	// 使用 GDI+ Bitmap::Save 保存，直接接受宽字符路径，避免 MBCS 下路径乱码问题

	// 辅助 lambda：根据 MIME 类型获取 GDI+ 编码器 CLSID
	auto GetEncoderClsid = [](const WCHAR* mimeType, CLSID* pClsid) -> bool
	{
		UINT numEncoders = 0, size = 0;
		Gdiplus::GetImageEncodersSize(&numEncoders, &size);
		if (size == 0) return false;
		std::vector<BYTE> buf(size);
		Gdiplus::ImageCodecInfo* pCodecInfo = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buf.data());
		Gdiplus::GetImageEncoders(numEncoders, size, pCodecInfo);
		for (UINT i = 0; i < numEncoders; ++i)
		{
			if (wcscmp(pCodecInfo[i].MimeType, mimeType) == 0)
			{
				*pClsid = pCodecInfo[i].Clsid;
				return true;
			}
		}
		return false;
	};

	// 生成临时文件名用于比较
	char tempPngName[64];
	snprintf(tempPngName, sizeof(tempPngName), "#%06d#_temp.png", (currentIndex % 100000));
	std::string tempPngPath = tempImageFolder + "\\" + tempPngName;
	std::wstring wTempPngPath = utf8_to_widechar(tempPngPath);

	char tempJpgName[64];
	snprintf(tempJpgName, sizeof(tempJpgName), "#%06d#_temp.jpg", (currentIndex % 100000));
	std::string tempJpgPath = tempImageFolder + "\\" + tempJpgName;
	std::wstring wTempJpgPath = utf8_to_widechar(tempJpgPath);

	// 用 GDI+ 直接从 HBITMAP 构造 Bitmap，再以宽字符路径保存
	CLSID clsidPng, clsidJpg;
	bool hasPngEncoder = GetEncoderClsid(L"image/png",  &clsidPng);
	bool hasJpgEncoder = GetEncoderClsid(L"image/jpeg", &clsidJpg);

	Gdiplus::Status stPng = Gdiplus::GenericError;
	Gdiplus::Status stJpg = Gdiplus::GenericError;
	{
		Gdiplus::Bitmap bmp(hBitmap, NULL);
		if (hasPngEncoder)
			stPng = bmp.Save(wTempPngPath.c_str(), &clsidPng, NULL);
		if (hasJpgEncoder)
			stJpg = bmp.Save(wTempJpgPath.c_str(), &clsidJpg, NULL);
	}

	HRESULT hrPng = (stPng == Gdiplus::Ok) ? S_OK : E_FAIL;
	HRESULT hrJpg = (stJpg == Gdiplus::Ok) ? S_OK : E_FAIL;

	// 关闭剪贴板
	CloseClipboard();

	// 检查保存结果
	if (FAILED(hrPng) && FAILED(hrJpg))
		return std::string();

	// 比较文件大小，选择较小的格式
	std::string resultPath;
	char finalFileName[64]; 

	// 获取文件大小
	long pngSize = 0, jpgSize = 0;
	FILE* fpPng = nullptr;
	FILE* fpJpg = nullptr;

	if (SUCCEEDED(hrPng) && _wfopen_s(&fpPng, wTempPngPath.c_str(), L"rb") == 0 && fpPng != nullptr)
	{
		fseek(fpPng, 0, SEEK_END);
		pngSize = ftell(fpPng);
		fclose(fpPng);
	} 

	if (SUCCEEDED(hrJpg) && _wfopen_s(&fpJpg, wTempJpgPath.c_str(), L"rb") == 0 && fpJpg != nullptr)
	{
		fseek(fpJpg, 0, SEEK_END);
		jpgSize = ftell(fpJpg);
		fclose(fpJpg);
	}

	// 选择较小的文件
	bool usePng = true;
	if (SUCCEEDED(hrPng) && SUCCEEDED(hrJpg))
	{
		// 两种格式都成功，比较大小
		usePng = (pngSize <= jpgSize);
	}
	else if (SUCCEEDED(hrJpg))
	{
		// 只有JPG成功
		usePng = false;
	}
	// else 只有PNG成功或都失败，保持usePng为true

	// 生成最终文件名并重命名
	// 同时尝试删除同一序号的另一种格式文件，确保只有一个文件存在
	char otherFormatFileName[64];
	if (usePng)
	{
		snprintf(finalFileName, sizeof(finalFileName), "#%06d#.png", (currentIndex % 100000));
		resultPath = tempImageFolder + "\\" + finalFileName;
		std::wstring wResultPath = utf8_to_widechar(resultPath);
		
		// 删除临时JPG文件（如果存在）
		if (SUCCEEDED(hrJpg))
			_wremove(wTempJpgPath.c_str());
		
		// 删除同一序号的JPG文件（如果存在），确保只有一个文件
		snprintf(otherFormatFileName, sizeof(otherFormatFileName), "#%06d#.jpg", (currentIndex % 100000));
		std::string otherPath = tempImageFolder + "\\" + otherFormatFileName;
		std::wstring wOtherPath = utf8_to_widechar(otherPath);
		_wremove(wOtherPath.c_str());
		
		// 重命名PNG文件
		if (resultPath != tempPngPath)
			_wrename(wTempPngPath.c_str(), wResultPath.c_str());
	}
	else
	{
		snprintf(finalFileName, sizeof(finalFileName), "#%06d#.jpg", (currentIndex % 100000));
		resultPath = tempImageFolder + "\\" + finalFileName;
		std::wstring wResultPath = utf8_to_widechar(resultPath);
		
		// 删除临时PNG文件（如果存在）
		if (SUCCEEDED(hrPng))
			_wremove(wTempPngPath.c_str());
		
		// 删除同一序号的PNG文件（如果存在），确保只有一个文件
		snprintf(otherFormatFileName, sizeof(otherFormatFileName), "#%06d#.png", (currentIndex % 100000));
		std::string otherPath = tempImageFolder + "\\" + otherFormatFileName;
		std::wstring wOtherPath = utf8_to_widechar(otherPath);
		_wremove(wOtherPath.c_str());
		
		// 重命名JPG文件
		if (resultPath != tempJpgPath)
			_wrename(wTempJpgPath.c_str(), wResultPath.c_str());
	}

	if (resultPath.empty())
		return std::string();

	// 保存哈希文件（与图片同名，后缀为.hash）
	char hashFileName[64];
	snprintf(hashFileName, sizeof(hashFileName), "#%06d#.hash", (currentIndex % 100000));
	std::string hashFilePath = tempImageFolder + "\\" + hashFileName;
	WriteHashFile(hashFilePath, currentHash);

	// 最大文件数限制 - 删除超出限制的旧文件
	const unsigned int MAX_TEMP_FILES = 1000;
	if (currentIndex >= MAX_TEMP_FILES)
	{
		unsigned int oldIndex = (currentIndex - MAX_TEMP_FILES) % 100000;
		char oldFileName[64];
		
		// 尝试删除旧PNG文件
		snprintf(oldFileName, sizeof(oldFileName), "#%06d#.png", oldIndex );
		std::string oldPath = tempImageFolder + "\\" + oldFileName;
		std::wstring wOldPath = utf8_to_widechar(oldPath);
		_wremove(wOldPath.c_str());
		
		// 尝试删除旧JPG文件
		snprintf(oldFileName, sizeof(oldFileName), "#%06d#.jpg", oldIndex );
		oldPath = tempImageFolder + "\\" + oldFileName;
		wOldPath = utf8_to_widechar(oldPath);
		_wremove(wOldPath.c_str());
		
		// 尝试删除旧哈希文件
		snprintf(oldFileName, sizeof(oldFileName), "#%06d#.hash", oldIndex );
		oldPath = tempImageFolder + "\\" + oldFileName;
		wOldPath = utf8_to_widechar(oldPath);
		_wremove(wOldPath.c_str());
	}

	// 更新序号文件
	unsigned int nextIndex = currentIndex + 1;
	if (_wfopen_s(&fp, wIndexFilePath.c_str(), L"w") == 0 && fp != nullptr)
	{
		fwprintf(fp, L"%u", nextIndex);
		fclose(fp);
	}

	return resultPath;
}


}
