#pragma once

#include <string>

namespace Utils
{

	extern bool GetFileContentIntoBase64(const char* path, std::string& content);

	extern bool LoadImageThumbnailIntoBase64(const char* path, int maxWidth, int maxHeight, std::string& content);

	extern std::string GenTempImageFromClipboard();

	// 获取图片尺寸（仅读取文件头，不加载整个文件）
	// 支持格式：jpg, jpeg, png, webp
	bool GetImageSize(const char* path, int& width, int& height);

}
