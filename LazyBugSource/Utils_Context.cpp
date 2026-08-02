#include "stdh.h"
#include <fstream>
#include <sstream>
#include "Utils.h"
#include "Utils_Context.h"
#include "Utils_Image.h"
#include "Utils_File.h"
#include "stringparser/stringparser.h"
#include <string>
#include "llmlib.h"


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
		double estimatedTokens = (numChinese / 2) + (numAlnum / 2.5) + (numSymbols * 1.0);

		return static_cast<int>(estimatedTokens);
	}

	// 估算 Op_AddToolCallResult 的 JSON 字符串的 token 数
	// jsonString 格式为 MakeToolCallResultString 产生的 [assistant_msg, tool_result_msg] 数组
	// 对 tool result 的 content 为数组（含 image_url block）的情况做正确处理
	int EstimateTokenCountForToolCallResult(const std::string& jsonString)
	{
		try
		{
			json parsed = json::parse(jsonString);
			if (!parsed.is_array())
				return EstimateTokenCount(jsonString);

			int totalTokens = 0;
			int imgWidth = 0, imgHeight = 0; // 从 text block 中提取的 WxH，供 image_url block 使用

			for (const auto& msg : parsed)
			{
				if (!msg.is_object())
				{
					totalTokens += EstimateTokenCount(msg.dump());
					continue;
				}

				const std::string& role = msg.value("role", "");
				if (role == "tool" && msg.contains("content"))
				{
					const auto& content = msg["content"];
					if (content.is_string())
					{
						totalTokens += EstimateTokenCount(content.get<std::string>());
					}
					else if (content.is_array())
					{
						for (const auto& block : content)
						{
							if (!block.is_object())
							{
								totalTokens += EstimateTokenCount(block.dump());
								continue;
							}

							const std::string& type = block.value("type", "");
							if (type == "text" && block.contains("text"))
							{
								const std::string& text = block["text"].get<std::string>();
								totalTokens += EstimateTokenCount(text);

								// 尝试从文本中提取宽高，格式 "(WxH)"，如 "Image read: xxx.png (1920x1080)"
								// 正则: (数字)x(数字)
								size_t pos = text.find('(');
								if (pos != std::string::npos)
								{
									size_t xPos = text.find('x', pos + 1);
									size_t closePos = text.find(')', xPos + 1);
									if (xPos != std::string::npos && closePos != std::string::npos && closePos > xPos + 1)
									{
										std::string wStr = text.substr(pos + 1, xPos - pos - 1);
										std::string hStr = text.substr(xPos + 1, closePos - xPos - 1);
										int w = atoi(wStr.c_str());
										int h = atoi(hStr.c_str());
										if (w > 0 && h > 0)
										{
											imgWidth = w;
											imgHeight = h;
										}
									}
								}
							}
							else if (type == "image_url" && block.contains("image_url"))
							{
								// 使用从 text block 中提取的宽高计算图片 token
								if (imgWidth > 0 && imgHeight > 0)
								{
									long long totalPixels = static_cast<long long>(imgWidth) * imgHeight;
									int imageTokens = static_cast<int>(totalPixels / 1500);
									const int MAX_IMAGE_TOKENS = 2048;
									if (imageTokens > MAX_IMAGE_TOKENS)
										imageTokens = MAX_IMAGE_TOKENS;
									if (imageTokens < 1)
										imageTokens = 1;
									totalTokens += imageTokens;
								}
								else
								{
									// 无法获取尺寸，使用默认值
									totalTokens += 255;
								}
							}
							else
							{
								totalTokens += EstimateTokenCount(block.dump());
							}
						}
					}
					else
					{
						totalTokens += EstimateTokenCount(content.dump());
					}
				}
				else
				{
					// 非 tool 消息（如 assistant），按整段 JSON 文本估算
					totalTokens += EstimateTokenCount(msg.dump());
				}
			}

			return totalTokens;
		}
		catch (const json::parse_error&)
		{
			return EstimateTokenCount(jsonString);
		}
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

	void TokenizeModelString(const std::string& str, std::vector<std::string>& outTokens)
	{
		outTokens.clear();
		std::string current;
		bool inAlpha = false;
		bool inDigit = false;

		for (char c : str)
		{
			bool isAlpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
			bool isDigit = (c >= '0' && c <= '9');

			if (isAlpha)
			{
				if (c >= 'A' && c <= 'Z')
					c = c - 'A' + 'a'; // 转小写
				if (!inAlpha)
				{
					if (!current.empty())
						outTokens.push_back(current);
					current.clear();
					inDigit = false;
				}
				inAlpha = true;
				current.push_back(c);
			}
			else if (isDigit)
			{
				if (!inDigit)
				{
					if (!current.empty())
						outTokens.push_back(current);
					current.clear();
					inAlpha = false;
				}
				inDigit = true;
				current.push_back(c);
			}
			else
			{
				// 分隔符，结束当前 token
				inAlpha = false;
				inDigit = false;
				if (!current.empty())
				{
					outTokens.push_back(current);
					current.clear();
				}
			}
		}

		if (!current.empty())
			outTokens.push_back(current);
	}

	const char* ResolveAutoSummarizeApi()
	{
		// 优先级列表（从高到低）
		static const char* priorityPatterns[] = {
			"kimi 2.5",
			"glm 4.7",
			"glm 5",
			"deepseek 4 pro",
			"gpt 5 mini",
			"haiku 4",
		};

		const auto& apis = g_llmLib.GetApis();

		for (const char* pattern : priorityPatterns)
		{
			// 将 pattern 拆分成 tokens
			std::vector<std::string> tokens;
			TokenizeModelString(pattern, tokens);

			if (tokens.empty())
				continue;

			// 在所有可用 API 中查找匹配的 model
			for (const auto& api : apis)
			{
				if (!api.enable || !g_llmLib.IsApiAvailable(api.name))
					continue;

				// 将 model 字段 token 化
				std::vector<std::string> modelTokens;
				TokenizeModelString(api.model, modelTokens);

				// 在 modelTokens 中按顺序查找 patternTokens
				bool match = true;
				size_t modelIdx = 0;
				for (const auto& t : tokens)
				{
					bool found = false;
					while (modelIdx < modelTokens.size())
					{
						if (modelTokens[modelIdx] == t)
						{
							found = true;
							modelIdx++;
							break;
						}
						modelIdx++;
					}
					if (!found)
					{
						match = false;
						break;
					}
				}

				if (match)
					return api.name.c_str();
			}
		}

		// 回退到 MajorChatApi
		return g_llmLib.GetMajorChatApi().c_str();
	}


}
