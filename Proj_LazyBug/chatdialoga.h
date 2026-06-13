#pragma once


#include "resource.h"

#include "LlmChat.h"

#include "ChatInputHistory.h"
#include "ChatHistory.h"

#include "ChatInput.h"

#include "ChatBriefA.h"

#include "ChatFileWriter.h"

#include "ChatTaskMgr.h"

#include "CheckpointsFileChange.h"
#include "FileLocator.h"

#include "ChatSettingPage.h"
#include "ChatDialogSplitter.h"

#include "ChatAgent.h"

#include "ChatUi.h"

#include "ChatTokenStats.h"

#include "ChatSkillsTree.h"
#include "ChatSettingMenu.h"

#include "ChatNotify.h"


struct RequestSwitchChatA
{
	RequestSwitchChatA()
	{
		t = 0;
	}
	bool IsValid()
	{
		return !chatFileName.empty();
	}
	void Reset()
	{
		chatFileName.clear();
		t = 0;
	}
	AbsTick t;
	std::wstring chatFileName;
};


class CChatDialogA : public CDialog
{
public:

	CChatDialogA( CWnd* pParent = NULL );
	virtual ~CChatDialogA();
	// dialog template
#ifdef VS_EXTENTION
	enum { IDD = IDD_CHAT_DLG_VSIX };
#else
	enum { IDD = IDD_CHAT_DLG};
#endif


	void Create(CWnd *parent);

	CChatInput& GetChatInput() { return _chatInput; }
	CChatInputHistory& GetInputHistory() { return _inputHistory; }
	const char* GetChatFileName()	{		return _agent.GetFileName();	}
	bool HasTitle() { return _agent.HasTitle(); }
	void SetTitle(const std::wstring& title) { _agent.SetTitle(title); }
	const std::vector<ChatOp>& GetOps() const { return _agent.GetOps(); }
	CChatOpsCtrl& GetOpsCtrl() { return _agent.GetOpsCtrl(); }
	CChatFileWriter& GetChatFileWriter()	{	return _chatFileWriter;	}
	CCheckpointsFileChange& GetCheckpointsFileChange() { return _checkpointsFileChange; }
	CFileLacator& GetFileLocator()	{	return _fileLocator;	}
	AbsTick FetchEscapeInputRequestTime() {		AbsTick ret = _requestEscapeInputTime; _requestEscapeInputTime = 0;		return ret;	}


	// ChatSettingPage控制方法
	void ShowChatSettingPage();
	void HideChatSettingPage();
	bool IsChatSettingPageVisible() const;
	CChatSettingPage& GetChatSettingPage() { return _chatSettingPage; }

	void ActivateCheckpointFileChange(const std::wstring& fileEditId);

	// 设置焦点到ChatInput
	void SetFocusToChatInput();

	// 更新设置菜单按钮状态（根据是否有打开的数据库文件夹）
	void UpdateSettingMenuButton();

protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void OnOK()	{}
	virtual void OnCancel(){}
public:
	DECLARE_MESSAGE_MAP()

	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:

protected:
	CChatInput _chatInput;
	CChatSettingPage _chatSettingPage;
	CChatBriefA _chatBrief;
	CChatDialogSplitter _splitter; // 分隔条控件
	double _splitterRatio; // 分隔条位置比例（0.0-1.0），表示上部分占总高度的比例

	CChatSkillsTree _chatSkillsTree; // Skills弹出窗口
	CChatSettingMenu _settingMenuWindow; // 设置菜单弹出窗口

	void _ShutdownAgent();
	void _InitAgent(const char* fileName);
	void _UpdateFavoriteStatus();
	CChatAgent _agent;
	CChatUi _ui;
	CChatNotify _notify;

	CChatFileWriter _chatFileWriter;
	CChatTaskMgr _chatTaskMgrBg;//background

	CCheckpointsFileChange _checkpointsFileChange;
	CFileLacator _fileLocator;
	AbsTick _requestEscapeInputTime;

	DWORD _chatFileVer;
	CChatInputHistory _inputHistory;
	CChatHistory _chatHistory;

	void _RecalcLayout();
	void _OnSplitterDragged(int newSplitterY);

	void _CollectTagPathes(std::vector<std::string>& filePathes);
	void _UpdateSyncImageTags();

	UINT _idTimer;

	void _UpdateSwitchChat();
	RequestSwitchChatA _requestSwitchChat;

	void _BrowseInputHistory(bool isPrev);

	void _UpdateContextUsage();
	CChatTokenStats _tokenStats;
	std::string _apiNameOfContextUsage;
	ChatOpCompressIntensity _compressLevelOfContextUsage;

	void _UpdateSaveChatCtrl();
	void _UpdateLoadChatCtrl();

	void _LoadChatInputTagsFromChatCtrl();
	void _RefreshTitleMenu();

	void _OnSendMessage(const std::wstring& content);

	void _OnInputContentChanged(const std::wstring& content);

	// WebView消息处理
	void _OnWebViewMessage(const std::wstring& message);
	void _HandleTitlebarMenuItemClicked(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp);
	void _HandleFavoriteClicked(const std::wstring& menuItemId, bool isFavorite);
	void _HandleFavoriteListButtonClicked();
	void _HandleUserMessageRestoreClicked(const std::wstring& messageId);
	void _RestoreUserMessageToInput(const std::wstring& messageId);
	void _HandleDisabledMessageClicked(const std::wstring& messageId);
	void _HandleFileEditTitleClicked(const std::wstring& fileEditId);
	void _HandleFileSummarizeClicked(const std::wstring& messageId, const std::wstring& filePath);
	void _HandleSettingsButtonClicked();
	void _HandleSettingMenuItemClicked(const std::wstring& itemName);
	void _HandleChatSettingPageClose();
	void _HandleTagClicked(const std::wstring& tagId);
	void _HandleEscape();
	void _HandleSkillButtonClicked(const RECT& btnRect);

	// CLI 白名单处理
	void _HandleCliWhitelist(const std::wstring& cliId);

	// Symbol 链接相关处理
	void _HandleQuerySymbolLocations(const std::wstring& messageId, const std::vector<std::wstring>& symbols);
	void _HandleSymbolLinkClicked(const std::wstring& symbol, const std::vector<std::pair<std::wstring, int>>& results);
	void _HandleOpenFile(const std::wstring& filePath);
	std::wstring _currentSymbolLink; // 当前点击的symbol
	int _symbolLinkClickIndex; // 当前symbol的点击索引，用于循环多个结果

	//Notify
	bool _OnBeforeSendToLlm(bool isUserMessage);
	void _OnAfterReceiveFromLlm();
	int _OnCheckCompress();

	// 自定义消息处理函数
	afx_msg LRESULT OnHandleUserMessageRestore(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHandleDisabledMessage(WPARAM wParam, LPARAM lParam);

	// 临时存储消息ID的成员变量
	std::wstring _pendingUserMessageId;
	std::wstring _pendingDisabledMessageId;

	void _OnStopButtonClicked();

	// 压缩结果提示相关
	void _UpdateCompressSummarizeTip();
	int _compressSummarizeTipVersion;  // 当前已显示的提示版本号

};






