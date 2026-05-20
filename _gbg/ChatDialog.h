#pragma once


#include "resource.h"

#include "LlmChat.h"
#include "ChatProcessors.h"

#include "ChatInputHistory.h"
#include "ChatHistory.h"

#include "ChatCtrl.h"
#include "ChatInput.h"

#include "ChatBrief.h"

#include "ChatFileWriter.h"

#include "ChatTaskMgr.h"

#include "CheckpointsFileChange.h"
#include "FileLocator.h"

#include "ChatSettingPage.h"
#include "ChatDialogSplitter.h"

struct RequestSwitchChat
{
	RequestSwitchChat()
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
		chatTitle.clear();
		t = 0;
	}
	AbsTick t;
	std::wstring chatFileName;
	std::wstring chatTitle;
};

//这个对象记录一次发送Llm request的上下文环境
struct LlmSessionContext
{
	LlmSessionContext()
	{
		t = 0;
		fileAttaches = -1;
	}
	std::string chatFileName;//当前是哪个chat
	std::string apiName;//使用哪个api
	std::vector<ChatInputTag> visibleFileTags;
	AbsTick t;//发送的时间
	int fileAttaches;//一个_chatCtrl里的Op_FileAttaches的索引(里面包含的文件会被attach到llm request里去)
};

class CChatDialog : public CDialog
{
public:

	CChatDialog( CWnd* pParent = NULL );
	virtual ~CChatDialog();
	// dialog template
#ifdef VS_EXTENTION
	enum { IDD = IDD_CHAT_DLG_VSIX };
#else
	enum { IDD = IDD_CHAT_DLG};
#endif


	void Create(CWnd *parent);

	void UpdateUI();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	const std::wstring& GetCurAIMessageID()	{		return _aiMessageId;	}
	CChatCtrl& GetChatCtrl()	{		return _chatCtrl;	}
	CChatInput& GetChatInput() { return _chatInput; }
	const char* GetChatFileName()	{		return _chatFileName.c_str();	}
	CChatFileWriter& GetChatFileWriter()	{	return _chatFileWriter;	}
	CChatTaskMgr& GetChatTaskMgr()	{		return _chatTaskMgr;	}
	CCheckpointsFileChange& GetCheckpointsFileChange() { return _checkpointsFileChange; }
	CFileLacator& GetFileLocator()	{	return _fileLocator;	}
	AbsTick FetchEscapeInputRequestTime() {		AbsTick ret = _requestEscapeInputTime; _requestEscapeInputTime = 0;		return ret;	}

	void AddAIMessageToChatCtrl(const std::wstring& str)	{		_AddMessageToChatCtrl(str, false);	}
	void AddToolCallMessageToChatCtrl(const std::wstring& message);

	void ShowFileEditProgressLabel(const std::wstring &fileName);

	// ChatSettingPage控制方法
	void ShowChatSettingPage();
	void HideChatSettingPage();
	bool IsChatSettingPageVisible() const;
	CChatSettingPage& GetChatSettingPage() { return _chatSettingPage; }

	void RequestSaveChatCtrl()	{		_requestSaveChatCtrl = true;	}
	void RequestSendToolCallResult()	{		_requestSendToolCallResult = true;	}

	void ActivateCheckpointFileChange(const std::wstring& fileEditId);

	// 设置焦点到ChatInput
	void SetFocusToChatInput();

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
	CRichEditCtrl _editLog; // 聊天历史控件
	CChatCtrl _chatCtrl;
	CChatInput _chatInput;
	CChatSettingPage _chatSettingPage;
	CChatBrief _chatBrief;
	CChatDialogSplitter _splitter; // 分隔条控件
	double _splitterRatio; // 分隔条位置比例（0.0-1.0），表示上部分占总高度的比例

	CChatFileWriter _chatFileWriter;

	CChatTaskMgr _chatTaskMgr;
	CChatTaskMgr _chatTaskMgrBg;//background

	CCheckpointsFileChange _checkpointsFileChange;
	CFileLacator _fileLocator;
	AbsTick _requestEscapeInputTime;

	CFont _fontEdit;        // 编辑框字体

	CChatInputHistory _inputHistory;
	CChatHistory _chatHistory;

	void _RecalcLayout();
	void _OnSplitterDragged(int newSplitterY);

	void _CollectTagPathes(std::vector<std::string>& filePathes);
	void _UpdateSyncImageTags();

	UINT _idTimer;

	bool _requestInterrupt;
	bool _requestSaveChatCtrl;
	bool _requestSendToolCallResult;

	bool _isRawHistoryVisible; // 新增成员变量，用于追踪 _editLog 的可见性


	void _UpdateSwitchChat();
	RequestSwitchChat _requestSwitchChat;

	enum class WorkingMode
	{
		None,
		Chat,
		ReapplyFileEdit,
	};
	bool _IsWorking()
	{
		return _workingMode != WorkingMode::None;
	}


	WorkingMode _workingMode;

	// WorkingMode::Chat
	void _InitLlmChat();
	void _UpdateChat();
	void _SendMessageToAI(const LlmSessionRequest& request);
	void _AddMessageToLog(const wchar_t* str);
	void _AddMessageToChatCtrl(const std::wstring& str, bool isUser);
	void _AddMessageToChatCtrl_Thinking(const std::wstring& str);
	void _AddTagsToChatCtrl();
	FilesCheckpointUID _CreateCheckpointForFileAttaches(const LlmSessionContext& ctx);
	int _AddAttachesToChatCtrl(FilesCheckpointUID checkpointId, const LlmSessionContext& ctx);
	bool _IsInChat()	{		return _workingMode == WorkingMode::Chat;	}
	CLlmChat _llmChat;
	LlmSessionUsage _chatUsage;
	std::wstring _aiMessageId;
	LlmSessionContext _lastLlmSessionCtx;

	//WorkingMode::ReapplyFileEdit
	void _UpdateReapplyFileEdit();

	void _BrowseInputHistory(bool isPrev);

	void _UpdateContextUsage();
	std::wstring _contextUsageCacheApi;
	int _contextUsagePrompCacheToken;

	void _UpdateLoadChatCtrl();
	void _UpdateSaveChatCtrl();
	std::string _chatFileName;
	DWORD _chatFileVer;

	void _LoadChatInputTagsFromChatCtrl();
	void _RefreshTitleMenu();

	void _RemoveDisabledSessions();

	// CChatCtrl WebView消息处理
	void _OnWebViewMessage(const std::wstring& message);
	void _HandleTitlebarMenuItemClicked(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp);
	void _HandleUserMessageRestoreClicked(const std::wstring& messageId);
	void _RestoreUserMessageToInput(const std::wstring& messageId);
	void _HandleDisabledMessageClicked(const std::wstring& messageId);
	void _HandleFileEditTitleClicked(const std::wstring& fileEditId);
	void _HandleFileSummarizeClicked(const std::wstring& messageId, const std::wstring& filePath);
	void _HandleSettingsButtonClicked();
	void _HandleTocButtonClicked();
	void _HandleChatSettingPageClose();
	void _HandleTagClicked(const std::wstring& tagId);

	// Symbol 链接相关处理
	void _HandleQuerySymbolLocations(const std::wstring& messageId, const std::vector<std::wstring>& symbols);
	void _HandleSymbolLinkClicked(const std::wstring& symbol, const std::vector<std::pair<std::wstring, int>>& results);
	std::wstring _currentSymbolLink; // 当前点击的symbol
	int _symbolLinkClickIndex; // 当前symbol的点击索引，用于循环多个结果

	// 自定义消息处理函数
	afx_msg LRESULT OnHandleUserMessageRestore(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHandleDisabledMessage(WPARAM wParam, LPARAM lParam);

	// 临时存储消息ID的成员变量
	std::wstring _pendingUserMessageId;
	std::wstring _pendingDisabledMessageId;

	void _OnStopButtonClicked();

	void _OnSendMessage(const std::wstring& content);
	void _OnSendToolCallResult();
	void _PrepareLlmSessionContext_SessionBegin(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx);
	void _PrepareLlmSessionContext_InSession(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx);
	void _TryInheritOldFileAttaches(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx);


};



