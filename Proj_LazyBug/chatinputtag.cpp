#include "stdh.h"
#include "ChatInputTag.h"

#include "stringparser/stringparser.h"
#include "timer/wuid.h"
#include "nlohmann/json.hpp"

std::wstring DecodeJsonString(const std::wstring& encoded)
{
	std::wstring result;
	result.reserve(encoded.length());

	for (size_t i = 0; i < encoded.length(); ++i)
	{
		if (encoded[i] == L'\\' && i + 1 < encoded.length())
		{
			switch (encoded[i + 1])
			{
			case L'"':  result += L'"'; i++; break;
			case L'\\': result += L'\\'; i++; break;
			case L'/':  result += L'/'; i++; break;
			case L'b':  result += L'\b'; i++; break;
			case L'f':  result += L'\f'; i++; break;
			case L'n':  result += L'\n'; i++; break;
			case L'r':  result += L'\r'; i++; break;
			case L't':  result += L'\t'; i++; break;
			default:    result += encoded[i]; break;
			}
		}
		else
		{
			result += encoded[i];
		}
	}

	return result;
}

std::wstring ExtractJsonStringValue(const std::wstring& json, const std::wstring& key)
{
	// 查找 \"key\":\"value\" 模式（转义引号）
	std::wstring searchPattern = L"\\\"" + key + L"\\\":\\\"";
	size_t startPos = json.find(searchPattern);

	if (startPos == std::wstring::npos)
	{
		// 尝试查找 \"key\":value 模式（非字符串值，转义引号）
		searchPattern = L"\\\"" + key + L"\\\":";
		startPos = json.find(searchPattern);
		if (startPos == std::wstring::npos)
		{
			TRACE(L"ExtractJsonStringValue: Key '%s' not found in: %s\n", key.c_str(), json.c_str());
			return L"";
		}

		startPos += searchPattern.length();

		// 跳过空白字符
		while (startPos < json.length() && (json[startPos] == L' ' || json[startPos] == L'\t'))
			startPos++;

		// 查找值的结束位置（逗号或右花括号）
		size_t endPos = startPos;
		while (endPos < json.length() && json[endPos] != L',' && json[endPos] != L'}')
			endPos++;

		if (endPos > startPos)
		{
			std::wstring value = json.substr(startPos, endPos - startPos);
			// 移除前后空白
			size_t firstNonSpace = value.find_first_not_of(L" \t");
			size_t lastNonSpace = value.find_last_not_of(L" \t");
			if (firstNonSpace != std::wstring::npos && lastNonSpace != std::wstring::npos)
			{
				std::wstring result = value.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
				TRACE(L"ExtractJsonStringValue: Found non-string value for key '%s': %s\n", key.c_str(), result.c_str());
				return result;
			}
		}
		return L"";
	}

	startPos += searchPattern.length();

	// 查找字符串结束的转义引号 \"
	// 需要找到下一个未被转义的 \" 序列
	size_t endPos = startPos;

	while (endPos < json.length() - 1)
	{
		// 查找 \" 序列
		if (json[endPos] == L'\\' && json[endPos + 1] == L'"')
		{
			// 检查这个 \" 前面是否还有反斜杠（即是否是 \\\")
			bool isEscaped = false;
			size_t backslashCount = 0;
			size_t checkPos = endPos;

			// 向前数连续的反斜杠
			while (checkPos > startPos && json[checkPos - 1] == L'\\')
			{
				backslashCount++;
				checkPos--;
			}

			// 如果反斜杠数量是奇数，说明这个 \" 是被转义的
			// 如果是偶数（包括0），说明这个 \" 是字符串结束标记
			if (backslashCount % 2 == 0)
			{
				// 找到字符串结束
				break;
			}
		}
		endPos++;
	}

	if (endPos > startPos && endPos < json.length() - 1)
	{
		std::wstring value = json.substr(startPos, endPos - startPos);
		// 解码JSON转义字符
		std::wstring result = DecodeJsonString(value);
		TRACE(L"ExtractJsonStringValue: Found string value for key '%s': %s\n", key.c_str(), result.c_str());
		return result;
	}

	TRACE(L"ExtractJsonStringValue: No value found for key '%s'\n", key.c_str());
	return L"";
}


void ParseSingleTag(const std::wstring& jsonObject, std::vector<ChatInputTag>& tags)
{
    // 检查是否为标签类型，注意JSON中的引号是被转义的
    if (jsonObject.find(L"\\\"type\\\":\\\"tag\\\"") == std::wstring::npos)
    {
        TRACE(L"ParseSingleTag: Not a tag object: %s\n", jsonObject.c_str());
        return;
    }
    
    ChatInputTag tag;
    
    // 解析各个字段
    tag.id = ExtractJsonStringValue(jsonObject, L"id");
    tag.text = ExtractJsonStringValue(jsonObject, L"text");
    tag.type = ExtractJsonStringValue(jsonObject, L"tagType");
    if (tag.type.empty())
        tag.type = ExtractJsonStringValue(jsonObject, L"type");
    tag.path = DecodeJsonString(ExtractJsonStringValue(jsonObject, L"data"));
    tag.color = L""; // 默认颜色
    
    // 解析removable字段
    std::wstring removableStr = ExtractJsonStringValue(jsonObject, L"removable");
    tag.removable = (removableStr != L"false");
    
    // 解析visible字段
    std::wstring visibleStr = ExtractJsonStringValue(jsonObject, L"visible");
    tag.visible = (visibleStr != L"false"); // 默认为可见
    
    // 检查必要字段
    if (!tag.id.empty() && !tag.text.empty())
    {
        tags.push_back(tag);
        TRACE(L"ParseSingleTag: Found tag - id:%s, text:%s, type:%s\n", 
              tag.id.c_str(), tag.text.c_str(), tag.type.c_str());
    }
    else
    {
        TRACE(L"ParseSingleTag: Missing required fields - id:%s, text:%s\n", 
              tag.id.c_str(), tag.text.c_str());
    }
}

void ParseInlineTags(const std::wstring& inputContent, std::vector<ChatInputTag>& tags)
{
	tags.clear();

	if (inputContent.empty())
		return;

	try
	{
		// 将wstring转换为UTF-8 string
		std::string utf8Content = widechar_to_utf8(inputContent.c_str());
		
		// 解析JSON数组
		nlohmann::json contentArray = nlohmann::json::parse(utf8Content);
		
		if (!contentArray.is_array())
		{
			TRACE(L"ParseInlineTags: Content is not a JSON array\n");
			return;
		}
		
		// 遍历数组
		for (const auto& item : contentArray)
		{
			if (item.is_object() && item.contains("type"))
			{
				std::string type = item["type"];
				
				// 只处理tag类型
				if (type == "tag")
				{
					ChatInputTag tag;
					
					// 解析id字段
					if (item.contains("id"))
					{
						tag.id = utf8_to_widechar(item["id"].get<std::string>());
					}
					
					// 解析text字段
					if (item.contains("text"))
					{
						tag.text = utf8_to_widechar(item["text"].get<std::string>());
					}
					
					// 解析tagType字段，如果没有则使用type字段
					if (item.contains("tagType"))
					{
						tag.type = utf8_to_widechar(item["tagType"].get<std::string>());
					}
					else if (item.contains("type"))
					{
						tag.type = utf8_to_widechar(item["type"].get<std::string>());
					}
					
					// 解析data字段（对应path）
					if (item.contains("data"))
					{
						tag.path = utf8_to_widechar(item["data"].get<std::string>());
					}
					
					// 解析removable字段
					if (item.contains("removable"))
					{
						tag.removable = item["removable"].get<bool>();
					}
					else
					{
						tag.removable = true; // 默认可删除
					}
					
					// 解析visible字段
					if (item.contains("visible"))
					{
						tag.visible = item["visible"].get<bool>();
					}
					else
					{
						tag.visible = true; // 默认可见
					}
					
					// color字段保持默认为空
					tag.color = L"";
					
					// 检查必要字段
					if (!tag.text.empty())
						tags.push_back(tag);
				}
			}
		}
	}
	catch (const nlohmann::json::exception& e)
	{
		TRACE(L"ParseInlineTags: JSON parse error: %s\n", utf8_to_widechar(e.what()).c_str());
		return;
	}
}

std::string ExtractPlainTextUtf8(const std::string& inputContent)
{
	std::string plainText;

	if (inputContent.empty())
		return plainText;

	try
	{
		// 解析JSON数组
		nlohmann::json contentArray = nlohmann::json::parse(inputContent);
		
		if (!contentArray.is_array())
		{
			// 不是数组，直接返回原内容
			return inputContent;
		}
		
		// 遍历数组
		for (const auto& item : contentArray)
		{
			if (item.is_object() && item.contains("type"))
			{
				std::string type = item["type"];
				if (type == "text" && item.contains("content"))
				{
					plainText += item["content"].get<std::string>();
				}
				else if (type == "tag")
				{
					std::string tagText;
					// 如果tagType是file，优先使用data字段
					if (item.contains("tagType") && item["tagType"] == "file" && item.contains("data"))
					{
						tagText = item["data"].get<std::string>();
					}
					else if (item.contains("text"))
					{
						tagText = item["text"].get<std::string>();
					}
					
					if (!tagText.empty())
					{
						plainText += "[" + tagText + "]";
					}
				}
			}
		}
	}
	catch (const nlohmann::json::exception&)
	{
		// JSON解析失败，直接返回原内容
		return inputContent;
	}

	return plainText;
}

std::wstring ExtractPlainText(const std::wstring& inputContent)
{
	if (inputContent.empty())
		return std::wstring();

	// 转换为UTF-8并调用UTF-8版本
	std::string utf8Content = widechar_to_utf8(inputContent.c_str());
	std::string utf8Result = ExtractPlainTextUtf8(utf8Content);
	
	// 如果结果与输入相同，说明不是数组或解析失败，返回原内容
	if (utf8Result == utf8Content)
		return inputContent;
	
	return utf8_to_widechar(utf8Result);
}
