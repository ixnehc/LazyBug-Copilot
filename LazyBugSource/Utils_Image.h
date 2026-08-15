#pragma once

#include <string>

namespace Gdiplus { class Image; }

namespace Utils
{

	extern bool GetFileContentIntoBase64(const char* path, std::string& content);

	extern bool LoadImageThumbnailIntoBase64(const char* path, int maxWidth, int maxHeight, std::string& content);

	// 使用 DirectXTex 解码 DDS/TGA，返回 Gdiplus::Bitmap（以 Image 基类指针返回）。
	// 调用者负责 delete 返回的指针。失败返回 nullptr。
	Gdiplus::Image* LoadImageWithDirectXTex(const std::wstring& imagePath);

	// 加载图片并转为 base64，供 LLM 消费。
	// - 原生格式（jpg/jpeg/png/webp/gif）且无需缩放：直接返回原始字节和对应 mime。
	// - 其它格式（bmp/tiff/ico/dds/tga 等）或需要缩放：解码后重编码（jpg 源→JPEG，其余→PNG）。
	// - DDS/TGA 使用 DirectXTex 解码。
	// - 格式不可解码时返回 false。
	// maxWidth/maxHeight 传 0 表示不缩放。
	// 成功时返回 true，并填充 content 和 mimeType。
	bool LoadImageIntoBase64(const char* path, int maxWidth, int maxHeight, std::string& content, std::string& mimeType);

	extern std::string GenTempImageFromClipboard();

	// 获取图片尺寸（仅读取文件头，不加载整个文件）
	// 支持格式：jpg, jpeg, png, webp, dds, tga；其余格式通过 CImage 回退获取
	bool GetImageSize(const char* path, int& width, int& height);

}
