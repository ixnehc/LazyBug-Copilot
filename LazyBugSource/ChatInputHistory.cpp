#include "stdh.h"
#include "ChatInputHistory.h"
#include "chatinputtag.h"

#include <algorithm>

#include "stringparser/stringparser.h"

#include "Utils.h"

extern const char* GetOpenedDBFolderPath_utf8();

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


void CChatInputHistory::SaveToFile()
{
	const char* dbFolder = GetOpenedDBFolderPath_utf8();
	if (!dbFolder || dbFolder[0] == '\0')
		return;

	std::string path = std::string(dbFolder) + "\\.inputhistory";

	json j;
	j["version"] = 7;

	// 最多保存最近的60条记录
	const int maxHistory = 60;
	int historyCount = (int)_history.size();
	int startIndex = historyCount > maxHistory ? historyCount - maxHistory : 0;
	int savedCount = historyCount - startIndex;

	json historyArr = json::array();
	for (int i = 0; i < savedCount; i++)
	{
		historyArr.push_back(widechar_to_utf8(_history[startIndex + i].c_str()));
	}
	j["history"] = historyArr;

	// 保存当前内容和索引
	j["currentContent"] = widechar_to_utf8(_currentContent.c_str());
	j["currentIndex"] = _currentIndex;

	Utils::SaveFileContent(path.c_str(), j.dump());
}

void CChatInputHistory::LoadFromFile()
{
	// 清空当前历史记录
	_history.clear();
	_currentContent.clear();
	_currentIndex = -1;

	const char* dbFolder = GetOpenedDBFolderPath_utf8();
	if (!dbFolder || dbFolder[0] == '\0')
		return;

	std::string path = std::string(dbFolder) + "\\.inputhistory";

	std::string content;
	if (!Utils::LoadFileContent(path.c_str(), content) || content.empty())
		return;

	json j;
	try
	{
		j = json::parse(content);
	}
	catch (...)
	{
		return;
	}

	// 检查版本号，版本不匹配则丢弃旧版本数据
	if (!j.contains("version") || !j["version"].is_number() || j["version"].get<int>() != 7)
		return;

	// 读取每条历史记录
	if (j.contains("history") && j["history"].is_array())
	{
		for (const auto& item : j["history"])
		{
			if (!item.is_string())
				continue;

			std::wstring textStr = utf8_to_widechar(item.get<std::string>());

			if (!textStr.empty() && !IsEmptyContent(textStr))
			{
				_history.push_back(textStr);
			}
		}
	}

	// 读取当前内容和索引
	if (j.contains("currentContent") && j["currentContent"].is_string())
		_currentContent = utf8_to_widechar(j["currentContent"].get<std::string>());

	if (j.contains("currentIndex") && j["currentIndex"].is_number())
		_currentIndex = j["currentIndex"].get<int>();

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

	SaveToFile();
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

	SaveToFile();
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

	SaveToFile();

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
		SaveToFile();

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
	SaveToFile();
	return true;
}

