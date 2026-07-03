#pragma once


class CCurrentUserRegistry;

class CChatInputHistory 
{
public:

	void SaveToRegistry(CCurrentUserRegistry &reg);
	void LoadFromRegistry(CCurrentUserRegistry& reg);

	void Add(const std::wstring& content);
	bool FindLast(const std::wstring& curContent, std::wstring& lastContent);
	bool FindNext(const std::wstring& curContent, std::wstring& nextContent);

	// 当前输入内容管理
	void OnModifyCurrent(const std::wstring& content);
	void OnSendCurrent();
	bool NavigatePrev();
	bool NavigateNext();

	const std::wstring& GetCurrentContent() const { return _currentContent; }
	int GetCurrentIndex() const { return _currentIndex; }
	void SetCurrentContent(const std::wstring& content) { _currentContent = content; }

protected:
	std::vector<std::wstring> _history;
	
	// 当前输入内容
	std::wstring _currentContent;
	// 指向history里的索引，-1表示不是来自历史记录
	int _currentIndex = -1;
};
