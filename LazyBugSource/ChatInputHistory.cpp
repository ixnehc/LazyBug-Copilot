#include "stdh.h"
#include "ChatInputHistory.h"
#include "chatinputtag.h"

#include <algorithm>

#include "stringparser/stringparser.h"

#include "Registry/Registry.h"

extern CCurrentUserRegistry g_reg;

namespace {
	// 检查字符串是否只包含不可见字符（空格、回车、tab等）
	bool IsAllInvisibleChars(const std::wstring& str)
	{
		for (wchar_t c : str)
		{
			// 如果不是不可见字符（空格、\t、\n、\r等），返回false
			if (c != L' ' && c != L'\t' && c != L'\n' && c != L'\r' && c != L'\v' && c != L'\f')
			{
				return false;
			}
		}
		return true;
	}

	// 检查 JSON content 数组是否为空（没有可见内容）
	// content 格式如: "[]" 或 "[{\"content\":\"...\",\"type\":\"text\"}]"
	// 返回 true 表示内容为空（应该清空）
	bool IsEmptyContent(const std::wstring& content)
	{
		// 空串检查
		if (content.empty())
		{
			return true;
		}

		// 空数组 "[]" 检查
		if (content == L"[]")
		{
			return true;
		}

		std::wstring plainText = ExtractPlainText(content);
		return IsAllInvisibleChars(plainText);
	}
}


void CChatInputHistory::SaveToRegistry(CCurrentUserRegistry &reg)
{
	const char* sectionName = LAZYBUG_CHATINPUT_HISTORY;

	// 保存版本号
	const int currentVersion = 7;
	reg.WriteInt(sectionName, "Version", currentVersion);
	
	// 最多保存最近的20条记录
	const int maxHistory = 60;
	int historyCount = (int)_history.size();
	int startIndex = historyCount > maxHistory ? historyCount - maxHistory : 0;
	int savedCount = historyCount - startIndex;

	// 保存历史记录的数量
	reg.WriteInt(sectionName, "Count", savedCount);
	
	// 保存每条历史记录
	for (int i = 0; i < savedCount; i++)
	{
		char valueName[64];
		sprintf(valueName, "Item%d", i);
		
		// 保存文本内容
		reg.WriteWString(sectionName, valueName, _history[startIndex + i]);
	}

	// 保存当前内容和索引
	reg.WriteWString(sectionName, "CurrentContent", _currentContent);
	reg.WriteInt(sectionName, "CurrentIndex", _currentIndex);
}

void CChatInputHistory::LoadFromRegistry(CCurrentUserRegistry& reg)
{
	const char* sectionName = LAZYBUG_CHATINPUT_HISTORY;

	// 清空当前历史记录
	_history.clear();
	
	// 检查版本号
	const int currentVersion = 7;
	int savedVersion = reg.ReadInt(sectionName, "Version", 0);
	
	// 如果版本不匹配，直接返回（丢弃旧版本数据）
	if (savedVersion != currentVersion)
	{
		return;
	}
	
	// 读取历史记录数量
	int historyCount = reg.ReadInt(sectionName, "Count", 0);
	
	// 读取每条历史记录
	for (int i = 0; i < historyCount; i++)
	{
		char valueName[64];
		sprintf(valueName, "Item%d", i);
		
		// 读取文本内容
		std::wstring textStr = reg.ReadWString(sectionName, valueName);
		
		// 添加到历史记录中
		if (!textStr.empty()&&(!IsEmptyContent(textStr)))
		{
			_history.push_back(textStr);
		}
	}

	// 读取当前内容和索引
	_currentContent = reg.ReadWString(sectionName, "CurrentContent");
	_currentIndex = reg.ReadInt(sectionName, "CurrentIndex", -1);
	
	// 验证索引的有效性
	if (_currentIndex >= (int)_history.size())
	{
		_currentIndex = -1;
	}
	if (_currentIndex >= 0)
	{
		if (_currentContent != _history[_currentIndex])
		{
			_currentContent = L"";
			_currentIndex = -1;
		}
	}
}

void CChatInputHistory::Add(const std::wstring& content)
{
	// 检查是否为空内容
	if (content.empty())
	{
		return;
	}
	
	// 查找历史记录中是否已存在相同内容
	auto it = std::find(_history.begin(), _history.end(), content);
	
	// 如果找到相同内容，则从原位置删除
	if (it != _history.end())
	{
		_history.erase(it);
	}
	
	// 添加到历史记录末尾
	_history.push_back(content);
	
	// 限制历史记录数量，最多保存20条
	const size_t maxHistory = 20;
	if (_history.size() > maxHistory)
	{
		_history.erase(_history.begin());
	}
}

bool CChatInputHistory::FindLast(const std::wstring& curContent, std::wstring& lastContent)
{
	if (_history.empty())
	{
		return false;
	}
	
	// 查找当前内容在历史记录中的位置
	auto it = std::find(_history.begin(), _history.end(), curContent);
	
	if (it == _history.end())
	{
		// 当前内容不在历史记录中，返回最后一条记录
		lastContent = _history.back();
		return true;
	}
	else if (it == _history.begin())
	{
		// 当前内容是第一条记录，循环到最后一条
		return false;
	}
	else
	{
		// 返回当前内容之前的一条记录
		lastContent = *(--it);
		return true;
	}
}

bool CChatInputHistory::FindNext(const std::wstring& curContent, std::wstring& nextContent)
{
	if (_history.empty())
	{
		return false;
	}
	
	// 查找当前内容在历史记录中的位置
	auto it = std::find(_history.begin(), _history.end(), curContent);
	
	if (it == _history.end())
	{
		// 当前内容不在历史记录中，直接返回
		return false;
	}
	else if (it == _history.end() - 1)
	{
		return false;
	}
	else
	{
		// 返回当前内容之后的一条记录
		nextContent = *(++it);
		return true;
	}
}

void CChatInputHistory::OnModifyCurrent(const std::wstring& content)
{
	// 检查内容是否为空，如果是则清空
	std::wstring trimmedContent = content;
	if (IsEmptyContent(trimmedContent))
	{
		trimmedContent.clear();
	}
	
	_currentContent = trimmedContent;
	// 清除索引，表示这是新的编辑内容
	_currentIndex = -1;

	SaveToRegistry(g_reg);
}

void CChatInputHistory::OnSendCurrent()
{
	// 将当前内容添加到历史中
	if (!_currentContent.empty())
	{
		Add(_currentContent);
	}
	// 清空当前内容及索引
	_currentContent.clear();
	_currentIndex = -1;

	SaveToRegistry(g_reg);
}

bool CChatInputHistory::NavigatePrev()
{
	if (_history.empty())
	{
		return false;
	}

	// 如果当前没有指向历史记录的某一条(-1)，从最后一条开始
	if (_currentIndex < 0)
	{
		if (_currentContent.empty())
			_currentIndex = (int)_history.size() - 1;
		else
		{
			OnSendCurrent();
			if (_history.size()>=2)
				_currentIndex = (int)_history.size() - 2;
		}
	}
	else if (_currentIndex > 0)
	{
		// 向前导航
		_currentIndex--;
	}
	else
	{
		// 已经到第一条了
		return false;
	}

	_currentContent = _history[_currentIndex];

	SaveToRegistry(g_reg);

	return true;
}

bool CChatInputHistory::NavigateNext()
{

	// 如果当前没有指向历史记录的某一条(-1)
	if (_currentIndex < 0)
	{
		// 如果当前内容不为空，则把当前内容加到历史记录中
		if (!_currentContent.empty())
		{
			Add(_currentContent);
			_currentContent.clear();
			_currentIndex = -1;
		}
		SaveToRegistry(g_reg);

		return true;
	}

	// 向后导航
	if (_currentIndex < (int)_history.size() - 1)
	{
		_currentIndex++;
		_currentContent = _history[_currentIndex];
	}
	else
	{
		// 已经到最后一条了，回到编辑状态
		_currentIndex = -1;
		_currentContent.clear();
	}
	SaveToRegistry(g_reg);
	return true;
}

