#include "stdh.h"
#include <fstream>
#include <sstream>
#include "Utils.h"
#include "Utils_Context.h"
#include "Utils_Image.h"
#include "Utils_File.h"
#include "stringparser/stringparser.h"
#include <string>


namespace Utils
{
	// 估算宽字符字符串的token数量
	// 参考Python实现：
	//   中文字符 / 1.5
	//   英文/数字 / 3.5
	//   符号/空格 * 1.0
	int EstimateTokenCount(const std::wstring& text)
	{
		if (text.empty())
			return 0;

		int numChinese = 0;
		int numAlnum = 0;
		int numSpaces = 0;
		int numSymbols = 0;

		for (wchar_t ch : text)
		{
			// 判断是否为中文字符 (CJK统一汉字)
			if (ch >= 0x4E00 && ch <= 0x9FA5)
			{
				numChinese++;
			}
			// 英文大写字母
			else if (ch >= L'A' && ch <= L'Z')
			{
				numAlnum++;
			}
			// 英文小写字母
			else if (ch >= L'a' && ch <= L'z')
			{
				numAlnum++;
			}
			// 数字
			else if (ch >= L'0' && ch <= L'9')
			{
				numAlnum++;
			}
			// 空格
			else if (ch == L' ')
			{
				numSpaces++;
			}
			// 其他符号（包括标点、换行等）
			else
			{
				numSymbols++;
			}
		}

		// 根据权重计算token数
		// 中文除以 1.5，英文/数字除以 3.5，符号乘以 1
		double estimatedTokens = (numChinese / 1.5) + (numAlnum / 3.5) + (numSymbols * 1.0);

		return static_cast<int>(estimatedTokens);
	}

	// 估算UTF-8格式字符串的token数量
	// 参考Python实现：
	//   中文字符 / 1.5
	//   英文/数字 / 3.5
	//   符号/空格 * 1.0
	int EstimateTokenCount(const std::string& text)
	{
		if (text.empty())
			return 0;

		int numChinese = 0;
		int numAlnum = 0;
		int numSpaces = 0;
		int numSymbols = 0;

		const unsigned char* p = reinterpret_cast<const unsigned char*>(text.c_str());
		const unsigned char* end = p + text.length();

		while (p < end)
		{
			unsigned char c = *p;

			// 单字节字符 (ASCII)
			if (c < 0x80)
			{
				// 英文大写字母
				if (c >= 'A' && c <= 'Z')
				{
					numAlnum++;
				}
				// 英文小写字母
				else if (c >= 'a' && c <= 'z')
				{
					numAlnum++;
				}
				// 数字
				else if (c >= '0' && c <= '9')
				{
					numAlnum++;
				}
				// 空格
				else if (c == ' ')
				{
					numSpaces++;
				}
				// 其他符号
				else
				{
					numSymbols++;
				}
				p++;
			}
			// 3字节UTF-8字符 (中文等CJK字符: 0xE0xxxx - 0xEFxxxx)
			else if ((c & 0xF0) == 0xE0 && (p + 2) < end)
			{
				// 检查是否是有效的3字节UTF-8序列
				unsigned char c2 = *(p + 1);
				unsigned char c3 = *(p + 2);
				if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80)
				{
					// 计算Unicode码点
					unsigned int codePoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
					// 判断是否为中文字符 (CJK统一汉字 U+4E00 - U+9FA5)
					if (codePoint >= 0x4E00 && codePoint <= 0x9FA5)
					{
						numChinese++;
					}
					else
					{
						numSymbols++;
					}
					p += 3;
				}
				else
				{
					// 无效的UTF-8序列，当作符号处理
					numSymbols++;
					p++;
				}
			}
			// 2字节UTF-8字符 (0xC0xxxx - 0xDFxxxx)
			else if ((c & 0xE0) == 0xC0 && (p + 1) < end)
			{
				unsigned char c2 = *(p + 1);
				if ((c2 & 0xC0) == 0x80)
				{
					numSymbols++;
					p += 2;
				}
				else
				{
					numSymbols++;
					p++;
				}
			}
			// 4字节UTF-8字符 (0xF0xxxxxx - 0xF7xxxxxx)
			else if ((c & 0xF8) == 0xF0 && (p + 3) < end)
			{
				unsigned char c2 = *(p + 1);
				unsigned char c3 = *(p + 2);
				unsigned char c4 = *(p + 3);
				if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80 && (c4 & 0xC0) == 0x80)
				{
					numSymbols++;
					p += 4;
				}
				else
				{
					numSymbols++;
					p++;
				}
			}
			else
			{
				// 无效的UTF-8字节，当作符号处理
				numSymbols++;
				p++;
			}
		}

		// 根据权重计算token数
		double estimatedTokens = (numChinese / 1.5) + (numAlnum / 3.5) + (numSymbols * 1.0);

		return static_cast<int>(estimatedTokens);
	}


	// 估计文件的Token数
	// - 图片文件：根据图片尺寸估算 (粗略 Token 数 ≈ (宽 × 高) ÷ 1500)
	// - 文本文件：使用 EstimateTokenCount 估算
	// 返回值：估计的Token数，失败返回0
	int EstimateFileTokenCount(const char* filePath)
	{
		if (!filePath || !*filePath)
			return 0;

		// 1. 检查是否为图片文件
		if (IsImageFile(filePath))
		{
			int width = 0, height = 0;
			if (GetImageSize(filePath, width, height) && width > 0 && height > 0)
			{
				// 图片Token估算公式：粗略 Token 数 ≈ (宽 × 高) ÷ 1500
				long long totalPixels = static_cast<long long>(width) * height;
				int estimatedTokens = static_cast<int>(totalPixels / 1500);
				
				// 设置合理的上限 (4K图片约为 600万/1500 ≈ 4000 tokens)
				const int MAX_IMAGE_TOKENS = 2048;
				if (estimatedTokens > MAX_IMAGE_TOKENS)
					return MAX_IMAGE_TOKENS;
				
				// 至少返回1个token
				return estimatedTokens > 0 ? estimatedTokens : 1;
			}

			// 如果无法获取尺寸，返回一个默认值
			return 255;
		}

		// 2. 检查是否为二进制文件
		if (CheckFileBinary(filePath))
		{
			// 二进制文件无法估算token，返回0表示不支持
			return 0;
		}

		// 3. 文本文件：读取内容并使用 EstimateTokenCount 估算
		std::string content;
		if (LoadFileContent(filePath, content))
		{
			return EstimateTokenCount(content);
		}

		// 加载失败返回0
		return 0;
	}

}
