#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>

// 输入标签结构
struct ChatInputTag
{
	ChatInputTag()
	{
		removable = true;
		visible = true;
	}
    std::wstring id;          // 标签唯一ID
	std::wstring path;        // 路径
	std::wstring text;        // 标签显示文字
    std::wstring type;        // 标签类型 (file, info, etc.)
    std::wstring color;       // 标签颜色
    bool removable;           // 是否可删除
    bool visible;             // 是否可见
};
    // 解析输入内容中的标签信息
extern void ParseInlineTags(const std::wstring& inputContent, std::vector<ChatInputTag>& tags);

// 从完整内容中提取纯文本
extern std::wstring ExtractPlainText(const std::wstring& inputContent);

// 从完整内容中提取纯文本 (UTF-8版本)
extern std::string ExtractPlainTextUtf8(const std::string& inputContent);
    
    
