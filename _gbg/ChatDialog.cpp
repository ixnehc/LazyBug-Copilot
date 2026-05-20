#include "stdh.h"
#include "ChatDialog.h"
#include "ChatInputTag.h"
//#include "WndBase.h"

#include "stringparser/stringparser.h"

#include "Registry/Registry.h"

#ifndef VS_EXTENSION
#include "LspClient.h"
#endif

#include "Utils.h"

#include "ReplaceChunks.h"

#include "LlmLib.h"

#include "nlohmann/json.hpp"

#include "ChatTask_ResolveSymbolLinks.h"

extern CCurrentUserRegistry g_reg;

#include <windows.h>
#include <richedit.h>

#include "SolutionDBApi.h"

CLlmLib g_llmLib;

extern const char* GetOpenedDBFolderPath_utf8();

#define WM_FORCE_UPDATE_FILECHANGE_ATTACH (WM_USER+100)
#define WM_HANDLE_USER_MESSAGE_RESTORE (WM_USER+101)
#define WM_HANDLE_DISABLED_MESSAGE (WM_USER+102)


// 假设hRichEdit是你的RichEdit控件句柄
void SetUnicodeTextToRichEdit(CRichEditCtrl &edit, const wchar_t* str, bool replaceSel)
{
	// 设置文本格式为Unicode
	SETTEXTEX stex;
	stex.flags = 8;//ST_UNICODE
	if (replaceSel)
		stex.flags |= ST_SELECTION;
	stex.codepage = 1200; // 1200 是 UTF-16LE 的代码页

	// 发送消息设置文本
	SendMessage(edit.GetSafeHwnd(), EM_SETTEXTEX, (WPARAM)&stex, (LPARAM)str);
}

CStringW GetWindowTextAsUnicode(const CRichEditCtrl* pRichEdit)
{
	CStringW strUnicode;

	if (!pRichEdit || !pRichEdit->GetSafeHwnd())
		return strUnicode;

	// 首先获取文本长度（字节数）
	GETTEXTLENGTHEX gtl;
	gtl.flags = GTL_NUMBYTES | GTL_PRECISE | GTL_USECRLF;
	gtl.codepage = 1200;  // UTF-16LE 编码

	LRESULT nBytes = pRichEdit->SendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);

	if (nBytes > 0)
	{
		// 设置获取文本的参数
		GETTEXTEX gt;
		ZeroMemory(&gt, sizeof(GETTEXTEX));
		gt.cb = (DWORD)nBytes + sizeof(WCHAR);  // 加上 null 终止符
		gt.flags = GT_USECRLF;     // 使用 Unicode 和 CR/LF
		gt.codepage = 1200;                     // UTF-16LE 编码

		// 分配足够的缓冲区（字符数 = 字节数 / 2）
		LPWSTR pBuffer = strUnicode.GetBufferSetLength((int)(nBytes / sizeof(WCHAR)) + 1);

		// 获取文本
		pRichEdit->SendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pBuffer);

		// 释放缓冲区
		strUnicode.ReleaseBuffer();
	}

	return strUnicode;
}

//extern CChangelists* GetChangelists();
extern CCheckpoints* GetCheckpoints();
extern ChatRestoreMode GetChatRestoreMode();


//////////////////////////////////////////////////////////////////////////
//CChatDialog
CChatDialog::CChatDialog( CWnd* pParent /* = NULL  */ )
	:CDialog( CChatDialog::IDD, pParent )
{
	_idTimer = 0;
	_workingMode = WorkingMode::None;
	_requestInterrupt = false;
	_requestSaveChatCtrl = false;
#ifdef LAZYBUG_RETAIL
	_isRawHistoryVisible = false;
#else
	_isRawHistoryVisible = true;
#endif
	_chatFileVer = 0;
	_requestSendToolCallResult = false;
	_requestEscapeInputTime = 0;
	_splitterRatio = 0.67; // 默认上部分占67%
	_contextUsagePrompCacheToken = 0;
}

CChatDialog::~CChatDialog()
{
	// 清理AI聊天资源
	_llmChat.Clear();
}

void CChatDialog::Create(CWnd *parent)
{
	CDialog::Create(CChatDialog::IDD,parent);
}

ChatCmd_EditFile g_recentCmd;

BOOL CChatDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
// 	HIDE_CONTROL( this, 1000);
// 	HIDE_CONTROL( this, IDC_TREE );
	//创建窗口
	CRect rc;
	GetClientRect(&rc);

	g_llmLib.Init();
	g_llmTools.Init();

	// 创建聊天历史记录控件
	_editLog.Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | 
		ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, CRect(0, 0, 0, 0), this, 4021);

	_chatCtrl.Create(CRect(0, 0, 200, 200), this, 4022);
	_chatCtrl.ShowWindow(SW_SHOW);

	_chatInput.Create(CRect(0, 0, 200, 200), this, 4023);
	_chatInput.ShowWindow(SW_SHOW);

	// 创建分隔条
	_splitter.Create(CRect(0, 0, 200, 6), this, 4025);
	_splitter.ShowWindow(SW_SHOW);

	// 设置分隔条拖动回调
	_splitter.SetDragCallback([this](int newSplitterY) {
		_OnSplitterDragged(newSplitterY);
	});

	// 创建ChatSettingPage，初始为隐藏状态
	_chatSettingPage.Create(CRect(0, 0, 200, 200), this, 4024);
	_chatSettingPage.ShowWindow(SW_HIDE);

	// 设置ChatSettingPage退出回调
	_chatSettingPage.SetExitCallback([this]() {
		_HandleChatSettingPageClose();
	});

	// 设置编辑扩展模型回调
	_chatSettingPage.SetEditModelsCallback([this]() {
		_chatSettingPage.EditModels();
	});

	if (!_isRawHistoryVisible)
		_editLog.ShowWindow(SW_HIDE);

	// 创建微软雅黑字体并设置给编辑框
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -16;  // 字体大小
	lf.lfWeight = FW_NORMAL;  // 正常粗细
	lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy(lf.lfFaceName, _T("微软雅黑"));
	
	_fontEdit.CreateFontIndirect(&lf);
	_editLog.SetFont(&_fontEdit);

	_inputHistory.LoadFromRegistry(g_reg);
	
	// 初始化AI聊天
	_InitLlmChat();

	// 设置WebView消息接收回调
	_chatCtrl.SetWebMessageReceivedCallback([this](const std::wstring& message) {
		_OnWebViewMessage(message);
	});

	// 设置设置按钮点击回调
	_chatCtrl.SetSettingsButtonClickedCallback([this]() {
		_HandleSettingsButtonClicked();
	});

	// 设置标题栏菜单更新回调
	_chatCtrl.SetTitlebarMenuUpdateCallback([this]() {
		_RefreshTitleMenu();
	});

	// 设置停止按钮点击回调（移动到ChatInput）
	_chatInput.SetStopButtonClickedCallback([this]() {
		_OnStopButtonClicked();
	});

	// 设置FileEdit标题点击回调
	_chatCtrl.SetFileEditTitleClickedCallback([this](const std::wstring& fileEditId) {
		_HandleFileEditTitleClicked(fileEditId);
	});

	// 设置FileSummarize点击回调
	_chatCtrl.SetFileSummarizeClickedCallback([this](const std::wstring& messageId, const std::wstring& filePath) {
		_HandleFileSummarizeClicked(messageId, filePath);
	});

	// 设置目录按钮点击回调
	_chatCtrl.SetTocButtonClickedCallback([this]() {
		_HandleTocButtonClicked();
	});

	// 设置标题栏菜单点击回调
	_chatCtrl.SetTitleMenuItemClickedCallback([this](const std::wstring& menuItemId, 
		const std::wstring& content, const std::wstring& stamp) {
		_HandleTitlebarMenuItemClicked(menuItemId, content, stamp);
	});

	
	
	_chatInput.SetSendCallback([this](const std::wstring& content, const std::wstring& plainText) {
		_OnSendMessage(content);
	});

	_chatInput.SetEscapeCallback([this]() {
		_requestEscapeInputTime = GetAbsTick();
	});

	// 设置ChatInput准备就绪回调，将历史输入内容设置到输入框
	_chatInput.SetReadyCallback([this]() {
		const std::wstring& content = _inputHistory.GetCurrentContent();
		if (!content.empty())
		{
			_chatInput.SetInputContent_(content);
		}
	});

	// 设置Page Up/Page Down按键回调
	_chatInput.SetPageNavigationCallback([this](bool isPrevious, const std::wstring& currentContent) {
		_BrowseInputHistory(isPrevious);
	});

	// 设置内容修改回调
	_chatInput.SetContentChangedCallback([this](const std::wstring& content) {
		_inputHistory.OnModifyCurrent(content);
	});

	// 设置标签点击回调
	_chatInput.SetTagClickedCallback([this](const std::wstring& tagId) {
		_HandleTagClicked(tagId);
	});

	// 设置文件粘贴回调
	_chatInput.SetFilePastedCallback([this](const std::wstring& fileType) {
			_chatInput.HandlePaste();
	});

	// 设置定时器，用于检查AI回答
	_idTimer = (UINT)SetTimer(1, 100, NULL);

	if (TRUE)
	{
		void* data;
		DWORD szData;
		if (g_reg.ReadData("RecendEditFileCmd", "FilePath", data, szData))
			g_recentCmd.filePath = (const char*)data;
		if (g_reg.ReadData("RecendEditFileCmd", "Content", data, szData))
			g_recentCmd.content = (const char*)data;
		if (g_reg.ReadData("RecendEditFileCmd", "Instruction", data, szData))
			g_recentCmd.instruction = (const char*)data;
	}

	if (true)
	{
		ChatTaskContext ctx;
		ctx.fileWriter = &_chatFileWriter;
		ctx.chatCtrl = &_chatCtrl;
		ctx.chatDialog = this;
		_chatTaskMgr.Init(ctx);
		_chatTaskMgrBg.Init(ctx);
	}

	if (g_llmLib.GetWorkingCapability() == CLlmLib::WorkingCapability::CannotWork)
		ShowChatSettingPage();

	return TRUE;
}


void CChatDialog::_InitLlmChat()
{
	_llmChat.Init();
}

void CChatDialog::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	// 不要使用DDX_Control，因为我们是动态创建控件
	// DDX_Control(pDX, 4021, _editLog);
}

BEGIN_MESSAGE_MAP(CChatDialog, CDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_MESSAGE(WM_HANDLE_USER_MESSAGE_RESTORE, &CChatDialog::OnHandleUserMessageRestore)
	ON_MESSAGE(WM_HANDLE_DISABLED_MESSAGE, &CChatDialog::OnHandleDisabledMessage)
END_MESSAGE_MAP()


void CChatDialog::_RecalcLayout()
{
	CRect rc;
	GetClientRect(&rc);

	// 如果ChatSettingPage可见，让它占满整个窗口并覆盖其他控件
	if (_chatSettingPage.GetSafeHwnd() && (_chatSettingPage.GetStyle() & WS_VISIBLE))
	{
		_chatSettingPage.MoveWindow(rc);
		return; // ChatSettingPage可见时，不需要调整其他控件
	}

	// 计算分隔条的位置
	int splitterHeight = _splitter.GetSplitterHeight();
	int minInputHeight = 100; // 输入框最小高度
	int minChatCtrlHeight = 150; // ChatCtrl最小高度
	
	// 设置分隔条的拖动范围
	int minSplitterY = rc.top + minChatCtrlHeight;
	int maxSplitterY = rc.bottom - minInputHeight - splitterHeight;
	_splitter.SetDragRange(minSplitterY, maxSplitterY);

	// 根据比例计算分隔条的Y坐标
	int splitterY = rc.top + (int)(rc.Height() * _splitterRatio);
	
	// 限制在有效范围内
	if (splitterY < minSplitterY)
		splitterY = minSplitterY;
	if (splitterY > maxSplitterY)
		splitterY = maxSplitterY;

	CRect historyRect; // _editLog 的区域
	CRect historyRect2; // _chatCtrl 的区域
	CRect splitterRect; // 分隔条的区域
	CRect inputRect; // 输入框的区域

	if (_isRawHistoryVisible)
	{
		// _editLog 显示时，两者各占一半宽度
		historyRect = rc;
		historyRect.bottom = splitterY;
		historyRect.right = rc.left + rc.Width() / 2;

		historyRect2 = rc;
		historyRect2.bottom = splitterY;
		historyRect2.left = rc.left + rc.Width() / 2;
	}
	else
	{
		// _editLog 隐藏时，_chatCtrl 占据全部宽度
		historyRect.SetRectEmpty(); // _editLog 不显示，区域为空

		historyRect2 = rc;
		historyRect2.bottom = splitterY;
	}

	// 分隔条位置
	splitterRect = rc;
	splitterRect.top = splitterY;
	splitterRect.bottom = splitterY + splitterHeight;

	// 输入框占据分隔条下方的空间
	inputRect = rc;
	inputRect.top = splitterY + splitterHeight;
	
	if (_editLog.GetSafeHwnd())
	{
		_editLog.MoveWindow(historyRect);
	}

	if (_chatCtrl.GetSafeHwnd())
	{
		_chatCtrl.MoveWindow(historyRect2);
	}

	if (_splitter.GetSafeHwnd())
	{
		_splitter.MoveWindow(splitterRect);
	}

	if (_chatInput.GetSafeHwnd())
	{
		_chatInput.MoveWindow(inputRect);
	}

	// ChatSettingPage隐藏时也需要调整位置，以便显示时能正确占满窗口
	if (_chatSettingPage.GetSafeHwnd())
	{
		_chatSettingPage.MoveWindow(rc);
	}
}

void CChatDialog::_OnSplitterDragged(int newSplitterY)
{
	CRect rc;
	GetClientRect(&rc);

	// 更新分隔条位置比例
	if (rc.Height() > 0)
	{
		_splitterRatio = (double)newSplitterY / rc.Height();
	}

	// 重新布局所有控件
	_RecalcLayout();
}


void CChatDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);

	_RecalcLayout();
}


void CChatDialog::OnDestroy()
{
	if (_idTimer)
		KillTimer(_idTimer);
	
	// 清理字体资源
	if (_fontEdit.GetSafeHandle())
		_fontEdit.DeleteObject();

	_chatTaskMgr.Shutdown();
	_chatTaskMgrBg.Shutdown();

	g_llmTools.Clear();
	g_llmLib.Clear();
		
	CDialog::OnDestroy();
}

void CChatDialog::UpdateUI()
{
	if (!m_hWnd)
		return;

}

BOOL CChatDialog::PreTranslateMessage(MSG* pMsg)
{
// 	if (pMsg->message == WM_KEYDOWN)
// 	{
// 		// 检查消息是否来自_editInput控件
// 		if (pMsg->hwnd == _editInput.GetSafeHwnd())
// 		{
// 			if (pMsg->wParam == VK_NEXT)  // PAGEDOWN键
// 			{
// 				_BrowseInputHistory(false);
// 				return TRUE; // 返回TRUE表示消息已处理
// 			}
// 			else if (pMsg->wParam == VK_PRIOR)  // PAGEUP键
// 			{
// 				_BrowseInputHistory(true);
// 				return TRUE; // 返回TRUE表示消息已处理
// 			}
// 		}
// 	}

	
	return CDialog::PreTranslateMessage(pMsg);
}

void CChatDialog::_RemoveDisabledSessions()
{
	std::vector<FilesCheckpointUID> checkpointIds;
	_chatCtrl.RemoveDisabledSessions(checkpointIds);
	CCheckpoints* checkpoints = GetCheckpoints();
	if (checkpoints)
	{
		for (int i = 0;i < checkpointIds.size();i++)
			checkpoints->DiscardCheckpoint(checkpointIds[i]);
	}
}

void CChatDialog::_SendMessageToAI(const LlmSessionRequest& request)
{
	LlmSessionSetting setting;
	if (false == g_llmLib.LoadLlmSetting(setting, g_llmLib.GetMajorChatApi(), ""))
		return;
// 	if (false == g_llmLib.LoadLlmSetting(setting, g_llmLib.GetMajorChatApi(), "chatrules_warmup"))
// 		return;

	bool solutionLoaded = false;
	const char* dbFolder = GetOpenedDBFolderPath_utf8();
	if (dbFolder && dbFolder[0] != 0)
		solutionLoaded = true;

	if (!solutionLoaded)
		setting.api.tools.clear();

	// 检查是否发送成功
	if (!_llmChat.Request(request, setting))
	{
		// 发送失败，显示错误消息
		_AddMessageToLog(L"Fail to send message");
		return;
	}

}

void CChatDialog::_UpdateChat()
{
	if (_requestInterrupt)
		_chatTaskMgr.Interrupt();
	_chatTaskMgr.Update();

	if (_llmChat.HasActiveSession())
	{
		// 处理当前会话
		LlmSessionOutput output;
		if (_llmChat.Process(output, _requestInterrupt))
		{
			if (!output.reasoning.empty())
			{
				std::wstring str = utf8_to_widechar(output.reasoning);
				_AddMessageToLog(str.c_str());

				_AddMessageToChatCtrl_Thinking(str);
			}
			if (!output.content.empty())
			{
				_AddMessageToLog(utf8_to_widechar(output.content).c_str());

				std::wstring str = utf8_to_widechar(output.content);
				_AddMessageToChatCtrl(str, false);
			}

			//如果_requestInterrupt为true,调用_chatTaskMgr.UpdateToolCalls()会开启新的task(因为_chatTaskMgr目前已经清除了之前的task)
			if(!_requestInterrupt)
				_chatTaskMgr.UpdateToolCalls(output.updatedToolCalls);

			// 如果会话完成
			if (output.isCompleted)
			{
				_chatUsage.Accumulate(output.usage);
				_chatCtrl.NotifyPromptCache(output.usage);
				// 如果有错误
				if (output.hasError && !output.errorMessage.empty())
				{
					// 显示错误消息
					std::wstring msg = L"Error: ";
					msg += utf8_to_widechar(output.errorMessage.c_str());
					_AddMessageToLog(msg.c_str());

					_chatCtrl.AddSystemMessage(msg);
				}
			}
		}
	}

	if (!_llmChat.HasActiveSession())
	{
		if (!_chatTaskMgr.IsRunning())
		{
			if (_requestSendToolCallResult)
			{
				_OnSendToolCallResult();

				_requestSendToolCallResult = false;
				return;
			}
		}
	}


	if (!_llmChat.HasActiveSession())
	{
		if (!_chatTaskMgr.IsRunning())
		{
			if (true)
			{
				std::vector<std::wstring> pathes;
				_chatCtrl.GetFileEditFilePathesByMessageId(_aiMessageId, pathes);

				for (int i = 0;i < pathes.size();i++)
					_chatCtrl.AddFileSummarizeToAIMessage(_aiMessageId, Utils::GetActualFilePath(pathes[i].c_str()));
			}

// 			_chatCtrl.AddStreamingAIMessage(_aiMessageId, std::wstring(L"\n \n"));

			_chatCtrl.CompleteStreamingAIMessage(_aiMessageId);
			if (!_aiMessageId.empty())
				_chatCtrl.AddSessionCost(_chatUsage,_aiMessageId);
			_chatCtrl.EndSession();
			_aiMessageId = L"";
			// 隐藏stop按钮
			_chatInput.HideStopButton();
			// 重置状态
			_workingMode = WorkingMode::None;
		}
	}
}


void CChatDialog::_UpdateReapplyFileEdit()
{
	if (_requestInterrupt)
		_chatTaskMgr.Interrupt();
	_chatTaskMgr.Update();

	if (!_chatTaskMgr.IsRunning())
	{
		_chatInput.HideStopButton();

		_workingMode = WorkingMode::None;
	}
}

extern std::wstring utf8_to_widechar(const std::string& utf8_str);
void CChatDialog::OnTimer(UINT_PTR nIDEvent)
{
	_checkpointsFileChange.SetCheckpoints(GetCheckpoints());

	switch (_workingMode)
	{
		case WorkingMode::Chat:
			_UpdateChat();
			break;
		case WorkingMode::ReapplyFileEdit:
			_UpdateReapplyFileEdit();
			break;
		default:
			_chatTaskMgr.Update();//This is for debug tasks 
			break;
	}

	_requestInterrupt = false;

	_chatTaskMgrBg.Update();

	g_llmLib.UpdateReload();

	_chatInput.Update();

//	_UpdateSyncImageTags();

	_chatSettingPage.Update();

	_UpdateSaveChatCtrl();
	_UpdateLoadChatCtrl();

	_UpdateSwitchChat();

	_UpdateContextUsage();

	_chatBrief.Update(*this);


	CDialog::OnTimer(nIDEvent);
}

void CChatDialog::_AddMessageToLog(const wchar_t* str)
{
	// 获取当前文本长度
	int nLength = _editLog.GetWindowTextLength();
	
	// 定位到文本末尾
	_editLog.SetSel(nLength, nLength);

	SetUnicodeTextToRichEdit(_editLog, str,true);
	
	// 滚动到底部
	_editLog.LineScroll(_editLog.GetLineCount());

}

void CChatDialog::_AddMessageToChatCtrl_Thinking(const std::wstring& str)
{
	if (_aiMessageId.empty())
		_aiMessageId = _chatCtrl.StartStreamingAIMessage();
	_chatCtrl.AddStreamingAIMessage_Thinking(_aiMessageId, str);
}

void CChatDialog::AddToolCallMessageToChatCtrl(const std::wstring& message)
{
	if (_aiMessageId.empty())
		_aiMessageId = _chatCtrl.StartStreamingAIMessage();

	_chatCtrl.AddToolCallMessage(_aiMessageId,message);
}


void CChatDialog::_AddMessageToChatCtrl(const std::wstring& str, bool isUser)
{
	if (isUser)
		_chatCtrl.AddUserMessage(str);
	else
	{
		if (_aiMessageId.empty())
			_aiMessageId=_chatCtrl.StartStreamingAIMessage();
		_chatCtrl.AddStreamingAIMessage(_aiMessageId, str);
	}
}


void CChatDialog::_BrowseInputHistory(bool isPrev)
{
	bool isFound = false;
	if (isPrev)
		isFound = _inputHistory.NavigatePrev();
	else
		isFound = _inputHistory.NavigateNext();

	if (isFound)
	{
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
	}
}

void CChatDialog::_RefreshTitleMenu()
{
	_chatCtrl.ClearTitlebarMenuItems();

	_chatCtrl.AddTitlebarMenuItem(L"newchat", L"New Chat", L"");

	std::wstring curfileName = local_to_widechar(_chatFileName.c_str());
	std::vector<CChatHistory::MenuItemInfo> menuItems;
	_chatHistory.GetRecentMenuItems(100, menuItems);
	for (const auto& item : menuItems)
	{
		if (item.uid != curfileName)
			_chatCtrl.AddTitlebarMenuItem(item.uid.c_str(), item.content.c_str(), item.stamp.c_str());
	}

}


void CChatDialog::_UpdateSaveChatCtrl()
{
	bool needSave = false;
	if (!_IsWorking())
		needSave = true;
	else
	{
		if (_requestSaveChatCtrl)
			needSave = true;
	}
	_requestSaveChatCtrl = false;
	if (!needSave)
		return;

	std::string dbFolderPath = GetOpenedDBFolderPath_utf8();
	if (dbFolderPath.empty())
		return;

	std::string chatsFolder = dbFolderPath;
	chatsFolder += "\\_chats";

	if(chatsFolder!=_chatHistory.GetFolderPath())
		return;

	if (_chatCtrl.GetVer() <= _chatFileVer)
		return;

	if (_chatFileName.empty())
		_chatFileName = MakeDateFileName("chat");

	std::string filePath=chatsFolder+"\\"+ _chatFileName;

	if (_chatCtrl.Save(filePath.c_str()))
		_chatFileVer = _chatCtrl.GetVer();

	_chatHistory.Add(_chatFileName.c_str());

}

void CChatDialog::_UpdateLoadChatCtrl()
{
	std::string dbFolderPath = GetOpenedDBFolderPath_utf8();

	if (!_chatCtrl.IsReady())
		return;

	std::string chatsFolder = dbFolderPath;
	if (!dbFolderPath.empty())
		chatsFolder += "\\_chats";

	if (chatsFolder != _chatHistory.GetFolderPath())
	{
		if (_IsWorking())
		{
			_OnStopButtonClicked();
			return;
		}
		if (_requestSaveChatCtrl)
			return;
	}

	if (chatsFolder != _chatHistory.GetFolderPath())
	{
		_chatHistory.Clear();
		if (!chatsFolder.empty())
			_chatHistory.Init(chatsFolder.c_str());

		_chatCtrl.ClearChat(); 
		_chatFileVer = _chatCtrl.GetVer();

		_chatInput.ClearTags();

		_chatFileName = _chatHistory.GetRecentFileName();
		if (!_chatFileName.empty())
		{
			if (!chatsFolder.empty())
			{
				std::string path = chatsFolder + "\\" + _chatFileName;
				if (_chatCtrl.Load(path.c_str()))
				{
					_chatFileVer = _chatCtrl.GetVer();

					_LoadChatInputTagsFromChatCtrl();
				}
			}
		}
	}
}

//====================== WebView消息处理实现 ======================

void CChatDialog::_OnWebViewMessage(const std::wstring& message)
{
	// 将宽字符串转换为UTF-8字符串用于JSON解析
	std::string utf8Message = widechar_to_utf8(message.c_str());
		
	// 解析JSON消息
	nlohmann::json jsonMsg = nlohmann::json::parse(utf8Message);
		
	// 检查是否有action字段
	if (!jsonMsg.contains("action") || !jsonMsg["action"].is_string())
	{
		return; // 无效消息，忽略
	}
		
	std::string action = jsonMsg["action"];
		
	// 处理不同类型的消息
	if (action == "titlebarMenuItemClicked")
	{
		// 处理标题栏菜单项点击事件
		if (jsonMsg.contains("menuItemId") && jsonMsg.contains("content") && jsonMsg.contains("stamp"))
		{
			std::wstring menuItemId = utf8_to_widechar(jsonMsg["menuItemId"].get<std::string>());
			std::wstring content = utf8_to_widechar(jsonMsg["content"].get<std::string>());
			std::wstring stamp = utf8_to_widechar(jsonMsg["stamp"].get<std::string>());
				
			_HandleTitlebarMenuItemClicked(menuItemId, content, stamp);
		}
	}
	else if (action == "userMessageRestoreClicked")
	{
		// 处理用户消息restore按钮点击事件
		if (jsonMsg.contains("messageId"))
		{
			std::wstring messageId = utf8_to_widechar(jsonMsg["messageId"].get<std::string>());
			// 使用PostMessage异步处理，避免阻塞WebView2
			_pendingUserMessageId = messageId;
			PostMessage(WM_HANDLE_USER_MESSAGE_RESTORE, 0, 0);
		}
	}
	else if (action == "disabledMessageClicked")
	{
		// 处理disabled消息点击事件
		if (jsonMsg.contains("messageId"))
		{
			std::wstring messageId = utf8_to_widechar(jsonMsg["messageId"].get<std::string>());
			// 使用PostMessage异步处理，避免阻塞WebView2
			_pendingDisabledMessageId = messageId;
			PostMessage(WM_HANDLE_DISABLED_MESSAGE, 0, 0);
		}
	}
	else if (action == "settingsButtonClicked")
	{
		// 处理设置按钮点击事件
		_HandleSettingsButtonClicked();
	}
	else if (action == "querySymbolLocations")
	{
		// 处理 symbol 查询请求
		if (jsonMsg.contains("messageId") && jsonMsg.contains("symbols"))
		{
			std::wstring messageId = utf8_to_widechar(jsonMsg["messageId"].get<std::string>());
			
			std::vector<std::wstring> symbols;
			if (jsonMsg["symbols"].is_array())
			{
				for (const auto& symbol : jsonMsg["symbols"])
				{
					symbols.push_back(utf8_to_widechar(symbol.get<std::string>()));
				}
			}
			
			_HandleQuerySymbolLocations(messageId, symbols);
		}
	}
	else if (action == "symbolLinkClicked")
	{
		// 处理 symbol 链接点击事件
		if (jsonMsg.contains("symbol"))
		{
			std::wstring symbol = utf8_to_widechar(jsonMsg["symbol"].get<std::string>());
			
			// 解析 results 数组
			std::vector<std::pair<std::wstring, int>> results;
			if (jsonMsg.contains("results") && jsonMsg["results"].is_array())
			{
				for (const auto& result : jsonMsg["results"])
				{
					if (result.contains("filePath") && result.contains("lineNumber"))
					{
						std::wstring filePath = utf8_to_widechar(result["filePath"].get<std::string>());
						int lineNumber = result["lineNumber"].get<int>();
						results.push_back(std::make_pair(filePath, lineNumber));
					}
				}
			}
			
			_HandleSymbolLinkClicked(symbol, results);
		}
	}
	// 可以在这里添加其他消息类型的处理
	// else if (action == "otherAction") { ... }
}

void CChatDialog::_LoadChatInputTagsFromChatCtrl()
{
	std::vector<ChatInputTag> tags;
	_chatCtrl.GetLastNotDisabledSessionTags(tags);

	_chatInput.ClearTags();
	for (int i = 0;i < tags.size();i++)
	{
		_chatInput.AddFilePathTag(tags[i].path.c_str(), tags[i].visible);
// 		_chatInput.AddTag(tags[i].text, tags[i].type, tags[i].path, tags[i].color, tags[i].removable, tags[i].visible);
	}
}

void CChatDialog::_UpdateSyncImageTags()
{
	// 获取当前输入内容
	const std::wstring& currentContent = _inputHistory.GetCurrentContent();
	
	// 解析输入内容中的标签
	std::vector<ChatInputTag> contentTags;
	ParseInlineTags(currentContent, contentTags);
	
	// 获取当前chat input中的所有标签
	const std::vector<ChatInputTag>& existingTags = _chatInput.GetTags();
	
	// 找出所有image类型的标签
	std::vector<std::wstring> contentImagePaths;  // 输入内容中的image标签路径列表
	std::vector<std::wstring> existingImagePaths; // chat input中现有的image标签路径列表
	
	// 收集输入内容中的image标签路径
	for (const auto& tag : contentTags)
	{
		if (tag.type == L"image")
		{
			contentImagePaths.push_back(tag.path);
		}
	}
	
	// 收集chat input中现有的image标签路径
	for (const auto& tag : existingTags)
	{
		if (tag.type == L"image")
		{
			existingImagePaths.push_back(tag.path);
		}
	}
	
	// 找出需要添加的标签（在输入内容中存在但在chat input中不存在的）
	for (const auto& contentTag : contentTags)
	{
		if (contentTag.type != L"image")
			continue;
			
		bool found = false;
		for (const auto& existingTag : existingTags)
		{
			if (existingTag.type == L"image" && existingTag.path == contentTag.path)
			{
				found = true;
				_chatInput.SetTagVisible(existingTag.id, true);
				break;
			}
		}
		
		if (!found)
		{
			// 添加标签
			_chatInput.AddTag(contentTag.text, contentTag.type, contentTag.path, 
			                  contentTag.color, true, true);
		}
	}
	
// 	// 找出需要移除的标签（在chat input中存在但在输入内容中不存在的）
// 	for (const auto& existingTag : existingTags)
// 	{
// 		if (existingTag.type != L"image")
// 			continue;
// 			
// 		bool found = false;
// 		for (const auto& contentTag : contentTags)
// 		{
// 			if (contentTag.type == L"image" && contentTag.path == existingTag.path)
// 			{
// 				found = true;
// 				break;
// 			}
// 		}
// 		
// 		if (!found)
// 		{
// 			// 移除标签
// 			_chatInput.RemoveTag(existingTag.id);
// 		}
// 	}
}


void CChatDialog::_UpdateSwitchChat()
{
	if (_requestSwitchChat.IsValid())
	{
		if (_IsWorking())
		{
			_requestInterrupt = true;
			return;
		}
	}

	if (_requestSwitchChat.t + 200 < GetAbsTick())//太久的Request
	{
		_requestSwitchChat.Reset();
		return;
	}

	// 处理"newchat"菜单命令
	if (_requestSwitchChat.chatFileName == L"newchat")
	{
		// 创建新的聊天会话
		_chatCtrl.ClearChat();

		// 清空输入框
		_inputHistory.OnSendCurrent();
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
		_chatInput.ClearTags();

		// 清空聊天历史记录显示
		_editLog.SetWindowText(_T(""));

		// 重置AI聊天状态
		_llmChat.Clear();
		_aiMessageId = L"";

		_chatFileName = "";
		_chatFileVer = _chatCtrl.GetVer();

		_chatBrief.Activate();

		_requestSwitchChat.Reset();
		return;
	}

	// 处理其他聊天历史文件的加载
	// menuItemId就是文件名，可以用来加载对应的聊天记录
	std::string fileName = widechar_to_utf8(_requestSwitchChat.chatFileName.c_str());

	std::string dbFolderPath = GetOpenedDBFolderPath_utf8();
	if (!dbFolderPath.empty())
	{
		std::string chatsFolder = dbFolderPath;
		chatsFolder += "\\_chats";
		std::string fullPath = chatsFolder + "\\" + fileName;


		if (_chatCtrl.Load(fullPath.c_str()))
		{
			_LoadChatInputTagsFromChatCtrl();

			// 设置标题为聊天内容
			_chatCtrl.SetTitle(_requestSwitchChat.chatTitle);

			_chatFileName = fileName;
			_chatFileVer = _chatCtrl.GetVer();
		}
	}
	_chatBrief.Activate();
	_requestSwitchChat.Reset();

}


void CChatDialog::_HandleTitlebarMenuItemClicked(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp)
{
	_requestSwitchChat.chatFileName = menuItemId;
	_requestSwitchChat.chatTitle = content;
	_requestSwitchChat.t = GetAbsTick();

	_UpdateSwitchChat();
}

void CChatDialog::_HandleUserMessageRestoreClicked(const std::wstring& messageId)
{
	if (_IsWorking())
		return;

	_checkpointsFileChange.Deactivate();

	if (GetChatRestoreMode() == ChatRestoreMode::UsingChangelists)
	{
		// 		if (CChangelists *changelists=GetChangelists())
		// 		{
		// 			FileChangeListUID changelistId;
		// 			if (_chatCtrl.GetSessionBeginChangelistsFromUserMessageId(messageId, changelistId))
		// 			{
		// 				if (changelistId != FilesCheckpointUID_Invalid)
		// 				{
		// 					changelists->SwitchTo(changelistId);
		// 				}
		// 			}
		// 		}
	}
	if (GetChatRestoreMode() == ChatRestoreMode::UsingCheckpoints)
	{
		if (CCheckpoints* checkpoints = GetCheckpoints())
		{
			std::vector<FilesCheckpointUID> checkpointIds;
			if (_chatCtrl.GetRestoreCheckpoints(messageId, checkpointIds))//注意返回的是未disable部分的session的checkpoints
			{
				if (checkpointIds.size() > 0)
				{
					// 检查checkpoint链中的文件是否被修改过
					std::vector<std::string> modifiedFiles;
					if (checkpoints->CheckCheckpointsFilesModified(checkpointIds, &modifiedFiles))
					{
						// 有文件被修改，弹出确认对话框
						CString message = "The following files have been modified after checkpoint was recorded:\n\n";
						for (const auto& filePath : modifiedFiles)
						{
							message += local_to_widechar(filePath.c_str()).c_str();
							message += "\n";
						}
						message += "\nContinuing the restore operation will overwrite these modifications. Do you want to continue?";

						int result = MessageBox(message, _T("Confirm Restore"), MB_YESNO | MB_ICONWARNING);
						if (result != IDYES)
						{
							return; // 用户选择取消，不执行restore操作
						}
					}

					FilesCheckpointUID restoredCheckpoint;
					FilesCheckpointUID undoCheckpoint = _chatCtrl.GetUndoCheckpoint(restoredCheckpoint);
					undoCheckpoint = checkpoints->Restore(checkpointIds, undoCheckpoint);
					_chatCtrl.SetUndoCheckpoint(undoCheckpoint);
				}
			}
		}
	}

	_chatCtrl.DisableMessagesAfter(messageId);

	// 将恢复的用户消息内容设置到ChatInput中
	_RestoreUserMessageToInput(messageId);
}

void CChatDialog::_RestoreUserMessageToInput(const std::wstring& messageId)
{
	// 获取用户消息的内容
	std::wstring messageContent;
	if (!_chatCtrl.GetUserMessageContent(messageId, messageContent))
	{
		// 没有找到用户消息，清空输入框
		_inputHistory.OnSendCurrent();
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
		_chatInput.ClearTags();
		return;
	}

	// 获取用户消息所在session的tags
	std::vector<ChatInputTag> sessionTags;
	_chatCtrl.GetUserMessageSessionTags(messageId, sessionTags);

	// 清空现有的tags
	_chatInput.ClearTags();

	// 添加session的tags到输入框
	for (const auto& tag : sessionTags)
	{
		_chatInput.AddTag(tag.text, tag.type, tag.path, tag.color, tag.removable, tag.visible);
	}

	// 设置用户消息内容到输入框
	_inputHistory.OnSendCurrent();
	_inputHistory.OnModifyCurrent(messageContent);
	_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
}


void CChatDialog::_HandleDisabledMessageClicked(const std::wstring& messageId)
{
	if (_IsWorking())
		return;

	if (GetChatRestoreMode() == ChatRestoreMode::UsingChangelists)
	{
		// 		if (CChangelists* changelists = GetChangelists())
		// 		{
		// 			FileChangeListUID changelistId;
		// 			if (_chatCtrl.GetDisabledSessionEndChangelist(changelistId))
		// 			{
		// 				if (changelistId != FilesCheckpointUID_Invalid)
		// 					changelists->SwitchTo(changelistId);
		// 			}
		// 		}
	}
	if (GetChatRestoreMode() == ChatRestoreMode::UsingCheckpoints)
	{
		if (CCheckpoints* checkpoints = GetCheckpoints())
		{
			FilesCheckpointUID restoredCheckpoint;
			FilesCheckpointUID checkpointId = _chatCtrl.GetUndoCheckpoint(restoredCheckpoint);
			if (checkpointId != FilesCheckpointUID_Invalid)
			{
				// 检查restoredCheckpoint中的文件是否被修改过
				std::vector<std::string> modifiedFiles;
				std::vector<FilesCheckpointUID> checkpointChain = { restoredCheckpoint };
				if (checkpoints->CheckCheckpointsFilesModified(checkpointChain, &modifiedFiles))
				{
					// 有文件被修改，弹出确认对话框
					CString message = "The following files have been modified after restore:\n\n";
					for (const auto& filePath : modifiedFiles)
					{
						message += local_to_widechar(filePath.c_str()).c_str();
						message += "\n";
					}
					message += "\nContinuing the operation will lose these modifications. Do you want to continue?";

					int result = MessageBox(message, _T("Confirm Undo"), MB_YESNO | MB_ICONWARNING);
					if (result != IDYES)
					{
						return; // 用户选择取消，不执行undo操作
					}
				}

				checkpoints->UndoRestore(checkpointId);
				_chatCtrl.SetUndoCheckpoint(FilesCheckpointUID_Invalid);
			}
		}
	}

	_chatCtrl.EnableAllDisabledMessages();

}

void CChatDialog::_HandleFileEditTitleClicked(const std::wstring& fileEditId)
{

	FilesCheckpointUID oldCheckpointId, newCheckpointId;
	if (!_chatCtrl.GetFileEditCheckpoint(fileEditId, newCheckpointId))
		return;

	bool existSummarize = _chatCtrl.ExistSummarizeInSession(fileEditId);
	if (existSummarize)
	{
		bool isHead;
		if (!_chatCtrl.GetFileEditPrevCheckpointInSession(fileEditId, oldCheckpointId, isHead))
			return;
	}
	else
	{
		if (!_chatCtrl.GetFileEditCheckpointInSessionBegin(fileEditId, oldCheckpointId))
			return;
	}

	if ((oldCheckpointId == FilesCheckpointUID_Invalid) || (newCheckpointId == FilesCheckpointUID_Invalid))
		return;

	std::wstring filePath;
	if (!_chatCtrl.GetFileEditFullPath(fileEditId, filePath))
		return;

	_checkpointsFileChange.Activate(oldCheckpointId, newCheckpointId, widechar_to_utf8(filePath.c_str()).c_str());
}

void CChatDialog::_HandleFileSummarizeClicked(const std::wstring& messageId, const std::wstring& filePath)
{
	FilesCheckpointUID oldCheckpointId, newCheckpointId;
	std::wstring fileEditId = _chatCtrl.GetLastFileEditCheckpointFromFilePath(messageId, filePath);
	if (fileEditId.empty())
		return;

	if (!_chatCtrl.GetFileEditCheckpoint(fileEditId, newCheckpointId))
		return;

	if (!_chatCtrl.GetFileEditCheckpointInSessionBegin(fileEditId, oldCheckpointId))
		return;

	if ((oldCheckpointId == FilesCheckpointUID_Invalid) || (newCheckpointId == FilesCheckpointUID_Invalid))
		return;

	std::string lowerCasedFilePath = widechar_to_utf8(filePath.c_str());
	StringLower(lowerCasedFilePath);

	_checkpointsFileChange.Activate(oldCheckpointId, newCheckpointId, lowerCasedFilePath.c_str());
}


void CChatDialog::_HandleTagClicked(const std::wstring& tagId)
{
	// 查找对应的标签
	const std::vector<ChatInputTag>& tags = _chatInput.GetTags();
	auto it = std::find_if(tags.begin(), tags.end(),
		[&tagId](const ChatInputTag& tag) { return tag.id == tagId; });
	
	if (it == tags.end())
		return;
	
	const ChatInputTag& tag = *it;
	
	// 根据标签类型处理点击事件
	if (tag.type == L"file")
	{
		// 文件标签：打开对应的文件
		if (!tag.path.empty())
		{
			std::string filePath = widechar_to_utf8(tag.path.c_str());
			FileLocation loc;
			GetFileLocator().Request(filePath.c_str(), loc);
		}
	}
// 	else if (tag.type == L"symbol")
// 	{
// 		// 符号标签：跳转到符号定义
// 		if (!tag.path.empty() && !tag.data.empty())
// 		{
// 			std::string filePath = widechar_to_utf8(tag.path.c_str());
// 			std::string symbolName = widechar_to_utf8(tag.data.c_str());
// 			
// 			// 使用VS的DTE接口跳转到符号
// 			extern bool NavigateToSymbolInVS(const char* filePath, const char* symbolName);
// 			NavigateToSymbolInVS(filePath.c_str(), symbolName.c_str());
// 		}
// 	}
// 	else
// 	{
// 		// 其他类型的标签：显示标签信息
// 		CStringW message;
// 		message.Format(L"标签: %s\n类型: %s\n路径: %s", 
// 			tag.text.c_str(), tag.type.c_str(), tag.path.c_str());
// 		
// 		AfxMessageBox(message, MB_OK | MB_ICONINFORMATION);
// 	}
}

void CChatDialog::_HandleSettingsButtonClicked()
{
	// 显示设置页面
	ShowChatSettingPage();
}

void CChatDialog::_HandleTocButtonClicked()
{
	const char* folderPath = GetOpenedDBFolderPath_utf8();
	if (folderPath && folderPath[0] != '\0')
		ShellExecute(NULL, _T("open"), _T("explorer.exe"), fromMBCS(folderPath), NULL, SW_SHOWNORMAL);
}

void CChatDialog::_HandleQuerySymbolLocations(const std::wstring& messageId, const std::vector<std::wstring>& symbols)
{
	std::vector<SymbolLinkItem> symbolLinks;
	symbolLinks.reserve(symbols.size());
	for (const auto& symbol : symbols)
	{
		SymbolLinkItem item;
		item.messageId = messageId;
		item.symbol = symbol;
		symbolLinks.push_back(item);
	}
	_chatTaskMgrBg.AddTask_ResolveSymbolLinks(symbolLinks);
}

void CChatDialog::_HandleSymbolLinkClicked(const std::wstring& symbol, const std::vector<std::pair<std::wstring, int>>& results)
{
	if (results.empty())
	{
		// 没有找到定义位置
		return;
	}
	
	// 如果点击的是不同的symbol，重置索引为0
	if (_currentSymbolLink != symbol)
	{
		_currentSymbolLink = symbol;
		_symbolLinkClickIndex = 0;
	}
	
	// 确保索引在有效范围内
	if (_symbolLinkClickIndex < 0 || _symbolLinkClickIndex >= (int)results.size())
	{
		_symbolLinkClickIndex = 0;
	}
	
	// 打开当前位置
	const std::wstring& filePath = results[_symbolLinkClickIndex].first;
	int lineNumber = results[_symbolLinkClickIndex].second;
	
	std::string filePathA = widechar_to_utf8(filePath.c_str());
	FileLocation loc;
	if (lineNumber >= 0)
	{
		loc.lineLoc.line = (WORD)lineNumber; // lineNumber 已经是 0-base
	}
	GetFileLocator().Request(filePathA.c_str(), loc);
	
	// 递增索引，下次点击时使用下一个结果
	_symbolLinkClickIndex++;
	
	// 如果超过最大索引，重置为0，实现循环
	if (_symbolLinkClickIndex >= (int)results.size())
	{
		_symbolLinkClickIndex = 0;
	}
}

void CChatDialog::_OnStopButtonClicked()
{
	if (_IsWorking())
		_requestInterrupt = true;
}

void CChatDialog::_TryInheritOldFileAttaches(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx)
{
	//检测我们是否可以沿用oldCtx里的fileAttaches(这样可以利用llm的token cache机制节省开销)
	newCtx.fileAttaches = -1;
	if (oldCtx.fileAttaches >= 0)
	{
		if ((oldCtx.chatFileName == newCtx.chatFileName) &&
			(oldCtx.apiName == newCtx.apiName))
		{
			const LlmApi* api = g_llmLib.GetApi(newCtx.apiName);
			LlmApiCacheControlType cacheControlType= g_llmLib.GetApiCacheControlType(newCtx.apiName);
			if (api)
			{
				if (cacheControlType == LlmApiCacheControlType::Anthropic_)
				{
					if (newCtx.t < oldCtx.t + (5 * 60 - 10) * 1000)//目前Anthropic和google的cache的时效都是5min
					{
						if (!newCtx.visibleFileTags.empty())
						{
							if (_chatCtrl.CheckValidFileAttachesCache(oldCtx.fileAttaches, newCtx.visibleFileTags))
								newCtx.fileAttaches = oldCtx.fileAttaches;
						}
					}
				}
// 				else
// 				{
// 					if (!newCtx.visibleFileTags.empty())
// 					{
// 						if (_chatCtrl.CheckValidFileAttachesCache(oldCtx.fileAttaches, newCtx.visibleFileTags))
// 							newCtx.fileAttaches = oldCtx.fileAttaches;
// 					}
// 				}
			}
		}
	}
}


void CChatDialog::_PrepareLlmSessionContext_SessionBegin(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx)
{
	newCtx.chatFileName = _chatFileName;
	newCtx.apiName = widechar_to_utf8(_chatInput.GetCurrentMajorChatApi().c_str());
	newCtx.visibleFileTags = _chatInput.GetVisibleFileTags();

	newCtx.t = GetAbsTick();

	_TryInheritOldFileAttaches(newCtx, oldCtx);
}

void CChatDialog::_PrepareLlmSessionContext_InSession(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx)
{
	newCtx.chatFileName = oldCtx.chatFileName;
	newCtx.apiName = oldCtx.apiName;
	newCtx.visibleFileTags = oldCtx.visibleFileTags;

	newCtx.t = GetAbsTick();

	if(true)
		newCtx.fileAttaches = oldCtx.fileAttaches;
	else
	{
		newCtx.fileAttaches = -1;
		const LlmApi* api = g_llmLib.GetApi(newCtx.apiName);
		if (api)
		{
			if (g_llmLib.GetApiCacheControlType(newCtx.apiName) == LlmApiCacheControlType::Anthropic_)
				newCtx.fileAttaches = oldCtx.fileAttaches;
		}
	}
//	_TryInheritOldFileAttaches(newCtx, oldCtx);
}


FilesCheckpointUID CChatDialog::_CreateCheckpointForFileAttaches(const LlmSessionContext& ctx)
{
	if (ctx.visibleFileTags.empty())
		return FilesCheckpointUID_Invalid;

	bool needAddToCheckpoint = true;
// 	if (true)
// 	{
// 		const LlmApi* api = g_llmLib.GetApi(ctx.apiName);
// 		if (api->cacheControlType != LlmApiCacheControlType::None)
// 			needAddToCheckpoint = true;
// 	}

	FilesCheckpointUID checkpointId = FilesCheckpointUID_Invalid;
	if (needAddToCheckpoint)
	{
		CCheckpoints* checkpoints = GetCheckpoints();
		if (checkpoints)
		{
			checkpointId = checkpoints->CreateEmptyCheckpoint();
			for (int i = 0;i < ctx.visibleFileTags.size();i++)
			{
				const ChatInputTag& tag = ctx.visibleFileTags[i];
				checkpoints->AddFileToCheckpoint(checkpointId, widechar_to_utf8(tag.path.c_str()).c_str());
			}
		}
	}
	return checkpointId;
}

int CChatDialog::_AddAttachesToChatCtrl(FilesCheckpointUID checkpointId, const LlmSessionContext& ctx)
{
	std::wstring filePathList;
	for (int i = 0;i < ctx.visibleFileTags.size();i++)
	{
		const ChatInputTag& tag = ctx.visibleFileTags[i];
		if (!filePathList.empty())
			filePathList += L"|";
		filePathList += tag.path;
	}
	return _chatCtrl.AddFileAttaches(filePathList, checkpointId);
}

void CChatDialog::_AddTagsToChatCtrl()
{
	const std::vector<ChatInputTag>& tags = _chatInput.GetTags();
	for (int i = 0;i < tags.size();i++)
		_chatCtrl.AddSessionTag(tags[i]);
}


void CChatDialog::_OnSendMessage(const std::wstring& content)
{
	if ((content.empty())|| content == L"[]")
		return;

	if (false)
	{
		extern const char* GetOpenedDBFolderPath_utf8();
		std::string dbFolderPath = GetOpenedDBFolderPath_utf8();

		SolutionDBMsg_FindInFilesResults result;
		extern void SolutionDB_FindInFiles(const char* dbFolderPath, const char* keyword, int maxResults, SolutionDBMsg_FindInFilesResults & result);
		SolutionDB_FindInFiles(dbFolderPath.c_str(), "selectTagCombination", 100, result);

		std::string s;
		Utils::DumpFindInFileResult("CLevel", result.results, s, 20);

		return;
	}

	// 从完整内容中提取纯文本
	std::wstring plainText = ExtractPlainText(content);

	if (false)
	{
		std::string keyword = widechar_to_utf8(plainText.c_str());
		extern const char* GetOpenedDBFolderPath_utf8();
		std::string dbFolderPath = GetOpenedDBFolderPath_utf8();

		SolutionDBMsg_SearchFileResult result;
		extern void SolutionDB_SearchFile(const char* dbFolderPath, const char* keyword, int maxResults, SolutionDBMsg_SearchFileResult & result);
		SolutionDB_SearchFile(dbFolderPath.c_str(), keyword.c_str(), 100, result);
		
		return;
	}

	if (true)
	{
		std::vector<ChatInputTag> inlineTags;
		ParseInlineTags(content, inlineTags);

		// 将 inlineTags 中的 image 类型 tag 添加到 visibleFileTags
		for (const auto& tag : inlineTags)
		{
			if (_wcsicmp(tag.type.c_str(), L"image") == 0)
			{
				_chatInput.AddFilePathTag(tag.path.c_str(), true);
			}
		}
	}
	
	_inputHistory.OnSendCurrent();

	// 检查是否为特殊命令 "_toggle_raw"
	if (plainText == L"_toggle_raw")
	{
		_isRawHistoryVisible = !_isRawHistoryVisible;
		if (_isRawHistoryVisible)
			_editLog.ShowWindow(SW_SHOW);
		else
			_editLog.ShowWindow(SW_HIDE);

		_inputHistory.OnSendCurrent();
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());

		_RecalcLayout(); // 重新计算布局
		return; // 处理完毕，不再继续
	}

	// 如果已经有活动会话，不处理用户输入
	if (_llmChat.HasActiveSession())
	{
		return;
	}


	if (!g_llmLib.IsApiAvailable(g_llmLib.GetMajorChatApi()))
		return;

	// 如果有内容则发送
	if (!plainText.empty())
	{
		if (true)
		{
			extern std::deque<std::string> g_requests;
			extern std::deque<std::string> g_receives;
			g_requests.clear();
			g_receives.clear();
		}

		_RemoveDisabledSessions();

		_AddMessageToLog(plainText.c_str());

		LlmSessionContext newCtx;
		_PrepareLlmSessionContext_SessionBegin(newCtx,_lastLlmSessionCtx);

		FilesCheckpointUID fileAttachesCheckpointId = FilesCheckpointUID_Invalid;
		if (newCtx.fileAttaches < 0)//如果无法利用之前的attaches,或者之前根本就没有attaches,我们需要新添加一个attaches,这里先为之创建一个checkpoint
			fileAttachesCheckpointId = _CreateCheckpointForFileAttaches(newCtx);

		//Begin Session
		if (true)
		{
//			FilesCheckpointUID sessionHeadCheckpointId = fileAttachesCheckpointId;//如果fileAttachesCheckpointId有效就直接用它
			FilesCheckpointUID sessionHeadCheckpointId = FileChangeListUID_Invalid;
			if (sessionHeadCheckpointId == FileChangeListUID_Invalid)
			{
				CCheckpoints* checkpoints = GetCheckpoints();
				if (checkpoints)
					sessionHeadCheckpointId = checkpoints->CreateEmptyCheckpoint();
			}

			_chatCtrl.BeginSession(sessionHeadCheckpointId);
		}

		_AddTagsToChatCtrl();

		//如果无法利用之前的attaches,或者之前根本就没有attaches,我们需要新添加一个attaches
		if (newCtx.fileAttaches < 0)
		{
			newCtx.fileAttaches = _AddAttachesToChatCtrl(fileAttachesCheckpointId, newCtx);

			if(newCtx.fileAttaches>=0)
				_AddMessageToLog(L"\n\n************new attaches***************\n\n");
		}

		_AddMessageToChatCtrl(content, true);

		// 显示stop按钮
		_chatInput.ShowStopButton();

		// 发送消息到AI
		LlmSessionRequest request;
		_chatCtrl.MakeSessionRequest(request, newCtx.fileAttaches);
		_SendMessageToAI(request);
		_workingMode = WorkingMode::Chat;
		_lastLlmSessionCtx = newCtx;
		_chatUsage.Zero();

		// 清空编辑框
		_inputHistory.OnSendCurrent();
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
	}
}

void CChatDialog::_OnSendToolCallResult()
{

	LlmSessionContext newCtx;
	_PrepareLlmSessionContext_InSession(newCtx, _lastLlmSessionCtx);

	if (newCtx.fileAttaches < 0)//如果无法利用之前的attaches,或者之前根本就没有attaches,我们需要新添加一个attaches,这里先为之创建一个checkpoint
		newCtx.fileAttaches = _AddAttachesToChatCtrl(_CreateCheckpointForFileAttaches(newCtx), newCtx);

	LlmSessionRequest request;
	_chatCtrl.MakeSessionRequest(request, newCtx.fileAttaches);
	_SendMessageToAI(request);

}



//====================== ChatSettingPage控制方法实现 ======================

void CChatDialog::ShowChatSettingPage()
{
	if (_chatSettingPage.GetSafeHwnd())
	{
		_chatSettingPage.ShowWindow(SW_SHOW);
		_chatSettingPage.BringWindowToTop();
		_RecalcLayout(); // 重新计算布局
	}
}

void CChatDialog::HideChatSettingPage()
{
	if (_chatSettingPage.GetSafeHwnd())
	{
		_chatSettingPage.ShowWindow(SW_HIDE);
		_RecalcLayout(); // 重新计算布局
	}
}

bool CChatDialog::IsChatSettingPageVisible() const
{
	if (_chatSettingPage.GetSafeHwnd())
	{
		return (_chatSettingPage.GetStyle() & WS_VISIBLE) != 0;
	}
	return false;
}

void CChatDialog::_HandleChatSettingPageClose()
{
	if (_chatSettingPage.IsValidatingProvider())
		return;
	HideChatSettingPage();
	_chatInput.UpdateMajorChatApiMenu();
}

void CChatDialog::ShowFileEditProgressLabel(const std::wstring& fileName)
{
	std::wstring aiMessageId = GetCurAIMessageID();
	if (aiMessageId.empty())
	{
		AddAIMessageToChatCtrl(L"\n");
		aiMessageId = GetCurAIMessageID();
	}
	_chatCtrl.ShowFileEditProgressLabel(aiMessageId, fileName.c_str());
}

void CChatDialog::ActivateCheckpointFileChange(const std::wstring& fileEditId)
{
	FilesCheckpointUID oldCheckpointId, newCheckpointId;
	if (!_chatCtrl.GetFileEditCheckpoint(fileEditId, newCheckpointId))
		return;
	bool isHead;
	if (!_chatCtrl.GetFileEditPrevCheckpointInSession(fileEditId, oldCheckpointId,isHead))
		return;

	if ((oldCheckpointId == FilesCheckpointUID_Invalid) || (newCheckpointId == FilesCheckpointUID_Invalid))
		return;

	std::wstring filePath;
	if (!_chatCtrl.GetFileEditFullPath(fileEditId, filePath))
		return;

	_checkpointsFileChange.Activate(oldCheckpointId, newCheckpointId, widechar_to_utf8(filePath.c_str()).c_str());

//	GetParent()->SendMessage(WM_FORCE_UPDATE_FILECHANGE_ATTACH, 0, 0);
}

// 自定义消息处理函数实现
LRESULT CChatDialog::OnHandleUserMessageRestore(WPARAM wParam, LPARAM lParam)
{
	if (!_pendingUserMessageId.empty())
	{
		_HandleUserMessageRestoreClicked(_pendingUserMessageId);
		_pendingUserMessageId.clear();
	}
	return 0;
}

LRESULT CChatDialog::OnHandleDisabledMessage(WPARAM wParam, LPARAM lParam)
{
	if (!_pendingDisabledMessageId.empty())
	{
		_HandleDisabledMessageClicked(_pendingDisabledMessageId);
		_pendingDisabledMessageId.clear();
	}
	return 0;
}

void CChatDialog::SetFocusToChatInput()
{
	SetFocus();
	if (_chatInput.GetSafeHwnd())
	{
		_chatInput.RequestOccupyFocus();
	}
}

void CChatDialog::_UpdateContextUsage()
{
	// 获取最近的prompt token数
	int recentPromptToken = _chatCtrl.GetRecentPromptToken();

	// 获取当前使用的API名称
	std::wstring currentApiNameW = _chatInput.GetCurrentMajorChatApi();

	if ((_contextUsagePrompCacheToken == recentPromptToken) && (_contextUsageCacheApi == currentApiNameW))
		return;

	std::string currentApiName = widechar_to_utf8(currentApiNameW.c_str());
	
	// 获取API的context capacity
	int contextCapacity = 0;
	const LlmApi* api = g_llmLib.GetApi(currentApiName);
	if (api)
	{
		contextCapacity = api->contextCapacity;
	}
	
	// 计算使用率进度 (0.0 - 1.0)
	float progress = 0.0f;
	if (contextCapacity > 0)
	{
		progress = static_cast<float>(recentPromptToken) / static_cast<float>(contextCapacity);
		if (progress > 1.0f)
			progress = 1.0f;
	}
	
	// 格式化tooltip文本
	char tooltip[256];
	if (contextCapacity > 0)
	{
		snprintf(tooltip, sizeof(tooltip), "( %.1f%% )", progress * 100.0f);
	}
	else
	{
		snprintf(tooltip, sizeof(tooltip), "( %.1f%% )", 100.0f);
	}
	
	// 设置上下文使用率到chat input
	if (!_chatInput.SetContextUsage(progress, tooltip))
		return;

	_contextUsagePrompCacheToken = recentPromptToken;
	_contextUsageCacheApi = currentApiNameW;

}
