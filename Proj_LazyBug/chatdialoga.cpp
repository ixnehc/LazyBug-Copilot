#include "stdh.h"
#include "ChatDialogA.h"
#include "ChatInputTag.h"
//#include "WndBase.h"

#include "stringparser/stringparser.h"

#include "Registry/Registry.h"

#include <fstream>

#ifndef VS_EXTENSION
#include "LspClient.h"
#endif

#include "Utils.h"
#include "Utils_Skill.h"
#include "Utils_CliWhitelist.h"

#include "CliWhitelist.h"

#include "ReplaceChunks.h"

#include "LlmLib.h"

#include "ChatOpsCompress.h"

#include "nlohmann/json.hpp"

#include "ChatTask_ResolveSymbolLinks.h"

extern CCurrentUserRegistry g_reg;

#include <windows.h>
#include <richedit.h>

#include "SolutionDBApi.h"

extern CLlmLib g_llmLib;

extern const char* GetOpenedDBFolderPath_utf8();
extern const char* GetCurModuleFolderPath_utf8();

#define WM_FORCE_UPDATE_FILECHANGE_ATTACH (WM_USER+100)
#define WM_HANDLE_USER_MESSAGE_RESTORE (WM_USER+101)
#define WM_HANDLE_DISABLED_MESSAGE (WM_USER+102)


//extern CChangelists* GetChangelists();
extern CCheckpoints* GetCheckpoints();
extern ChatRestoreMode GetChatRestoreMode();


//////////////////////////////////////////////////////////////////////////
//CChatDialogA
CChatDialogA::CChatDialogA( CWnd* pParent /* = NULL  */ )
	:CDialog( CChatDialogA::IDD, pParent )
{
	_idTimer = 0;

	_splitterRatio = 0.67; // 默认上部分占67%

	_chatFileVer = 0;
	_compressLevelOfContextUsage = ChatOpCompressIntensity::None;
}

CChatDialogA::~CChatDialogA()
{
	_agent.Shutdown();
}

void CChatDialogA::Create(CWnd *parent)
{
	CDialog::Create(CChatDialogA::IDD,parent);
}


void CChatDialogA::_ShutdownAgent()
{
	_agent.Shutdown();
	_chatFileVer = _agent.GetFileVer();
}

void CChatDialogA::_InitAgent(const char* fileName)
{
	ChatAgentContext ctx;
	ctx.checkpoints = GetCheckpoints();
	ctx.dbFolderPath = GetOpenedDBFolderPath_utf8();

	if (true)
	{
		std::string path = Utils::GetGlobalRulesFilePath();
		if (!path.empty())
			ctx.rulesFiles.push_back(path);
		path = Utils::GetProjectRulesFilePath();
		if (!path.empty())
			ctx.rulesFiles.push_back(path);
	}

	// 设置 CChatNotify 的回调
	_notify.SetBeforeSendToLlmCallback([this](bool isUserMessage) {
		return _OnBeforeSendToLlm(isUserMessage);
	});
	_notify.SetAfterReceiveFromLlmCallback([this]() {
		_OnAfterReceiveFromLlm();
	});
	_notify.SetCheckCompressCallback([this]() {
		return _OnCheckCompress();
	});

	_agent.Init(fileName, ctx, &_ui, &_notify);
	_chatFileVer = _agent.GetFileVer();

	// 初始化 TokenStats
	ChatTokenStatsContext tokenCtx;
	tokenCtx.opsCtrl = &_agent.GetOpsCtrl();
	tokenCtx.chatInput = &_chatInput;
	tokenCtx.llmSkills = &g_llmSkills;
	tokenCtx.chatDialog = this;
	_tokenStats.Initialize(tokenCtx);
}


BOOL CChatDialogA::OnInitDialog()
{
	CDialog::OnInitDialog();
	
// 	HIDE_CONTROL( this, 1000);
// 	HIDE_CONTROL( this, IDC_TREE );
	//创建窗口
	CRect rc;
	GetClientRect(&rc);

	g_llmLib.Init();
	g_llmTools.Init();

	_ui.Create(CRect(0, 0, 200, 200), this, 4022);
	_ui.ShowWindow(SW_SHOW);

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

	// 创建微软雅黑字体并设置给编辑框
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = -16;  // 字体大小
	lf.lfWeight = FW_NORMAL;  // 正常粗细
	lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy(lf.lfFaceName, _T("微软雅黑"));
	  
	_inputHistory.LoadFromRegistry(g_reg);
	
	// 初始化AI聊天

	// 设置WebView消息接收回调
	_ui.SetWebMessageReceivedCallback([this](const std::wstring& message) {
		_OnWebViewMessage(message);
	});

	// 设置标题栏菜单更新回调
	_ui.SetTitlebarMenuUpdateCallback([this]() {
		_RefreshTitleMenu();
	});

	// 设置停止按钮点击回调（移动到ChatInput）
	_chatInput.SetStopButtonClickedCallback([this]() {
		_OnStopButtonClicked();
	});

	// 设置FileEdit标题点击回调
	_ui.SetFileEditTitleClickedCallback([this](const std::wstring& fileEditId) {
		_HandleFileEditTitleClicked(fileEditId);
	});

	// 设置FileSummarize点击回调
	_ui.SetFileSummarizeClickedCallback([this](const std::wstring& messageId, const std::wstring& filePath) {
		_HandleFileSummarizeClicked(messageId, filePath);
	});

	// 设置标题栏菜单点击回调
	_ui.SetTitleMenuItemClickedCallback([this](const std::wstring& menuItemId,
		const std::wstring& content, const std::wstring& stamp) {
		_HandleTitlebarMenuItemClicked(menuItemId, content, stamp);
	});

	// 设置WebView导航完成回调
	_ui.SetNavigationCompletedCallback([this](bool success) {
		UpdateSettingMenuButton();
	});

	_chatInput.SetSendCallback([this](const std::wstring& content, const std::wstring& plainText) {
		_OnSendMessage(content);
	});

	_chatInput.SetEscapeCallback([this]() {
		_HandleEscape();
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
		_OnInputContentChanged(content);
	});

	// 设置标签点击回调
	_chatInput.SetTagClickedCallback([this](const std::wstring& tagId) {
		_HandleTagClicked(tagId);
	});

	// 设置文件粘贴回调
	_chatInput.SetFilePastedCallback([this](const std::wstring& fileType) {
			_chatInput.HandlePaste();
	});

	// 设置Skill按钮点击回调
	_chatInput.SetSkillButtonClickedCallback([this](const RECT& btnRect) {
		_HandleSkillButtonClicked(btnRect);
	});

	// 设置压缩强度改变回调
	_chatInput.SetCompressIntensityChangedCallback([this](int intensity) {
		CChatOpsCompress::SaveIntensityForCurrentApi(static_cast<ChatOpCompressIntensity>(intensity));
	});

	// 创建Skills弹出窗口
	_chatSkillsTree.CreateSkillsTreeWindow(this);
	_chatSkillsTree.SetSkillSelectedCallback([this](const std::wstring& skillPath) {

		std::string filePathA = widechar_to_utf8(skillPath.c_str());
		filePathA += "\\skill.md";
		FileLocation loc;
		GetFileLocator().Request(filePathA.c_str(), loc);

	});
	_chatSkillsTree.SetSkillEnableChangedCallback([this](const std::wstring& skillPath, bool enable) {
		// Skill启用状态变化处理：创建或删除 .enable 文件
		std::wstring wEnableFilePath = skillPath + L"\\.enable";
		std::string skillPathUtf8 = widechar_to_utf8(skillPath.c_str());
		if (enable)
		{
			// 创建空的 .enable 文件（使用宽字符版本）
			std::wofstream file(wEnableFilePath);
			file.close();
		}
		else
		{
			// 删除 .enable 文件（使用宽字符版本）
			DeleteFileW(wEnableFilePath.c_str());
		}

		// 更新 g_llmSkills 中对应 skill 的 enable 状态
		for (auto& skill : g_llmSkills._skills)
		{
			if (skill.folderPath == skillPathUtf8)
			{
				skill.enable = enable;
				break;
			}
		}
	});

	// 创建设置菜单窗口
	_settingMenuWindow.CreateSettingMenuWindow(this);
	_settingMenuWindow.SetItemSelectedCallback([this](const std::wstring& itemName) {
		_HandleSettingMenuItemClicked(itemName);
	});

	// 设置定时器，用于检查AI回答
	_idTimer = (UINT)SetTimer(1, 100, NULL);

	if (true)
	{
		ChatTaskContext ctx;
		ctx.fileWriter = &_chatFileWriter;
		ctx.chatUi = &_ui;
 		_chatTaskMgrBg.Init(ctx);
	}

	Utils::SyncBuiltInSkills();

	Utils::LoadLlmSkills(g_llmSkills, "");

	_InitAgent("");


	if (g_llmLib.GetWorkingCapability() == CLlmLib::WorkingCapability::CannotWork)
		ShowChatSettingPage();

	return TRUE;
}


void CChatDialogA::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	// 不要使用DDX_Control，因为我们是动态创建控件
}

BEGIN_MESSAGE_MAP(CChatDialogA, CDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_MESSAGE(WM_HANDLE_USER_MESSAGE_RESTORE, &CChatDialogA::OnHandleUserMessageRestore)
	ON_MESSAGE(WM_HANDLE_DISABLED_MESSAGE, &CChatDialogA::OnHandleDisabledMessage)
END_MESSAGE_MAP()


void CChatDialogA::_RecalcLayout()
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

	CRect historyRect2; // _chatCtrl 的区域
	CRect splitterRect; // 分隔条的区域
	CRect inputRect; // 输入框的区域

	{
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
	
	if (_ui.GetSafeHwnd())
	{
		_ui.MoveWindow(historyRect2);
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

void CChatDialogA::_OnSplitterDragged(int newSplitterY)
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


void CChatDialogA::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);

	_RecalcLayout();
}


void CChatDialogA::OnDestroy()
{
	if (_idTimer)
		KillTimer(_idTimer);

	_ShutdownAgent();
	_chatTaskMgrBg.Shutdown();

	g_llmTools.Clear();
	g_llmLib.Clear();
		
	CDialog::OnDestroy();
}


extern std::wstring utf8_to_widechar(const std::string& utf8_str);
void CChatDialogA::OnTimer(UINT_PTR nIDEvent)
{
	_checkpointsFileChange.SetCheckpoints(GetCheckpoints());

	if (true)
	{
		bool wasWorking = _agent.IsWorking();
		_agent.Update();
		if (wasWorking && (!_agent.IsWorking()))
			_chatInput.HideStopButton();
	}

	_chatTaskMgrBg.Update();

	g_llmLib.UpdateReload();

	extern CCliWhitelist g_cliWhitelist;
	g_cliWhitelist.UpdateReload();

	_chatInput.Update();

	_chatSkillsTree.Update();

	_settingMenuWindow.Update();

//	_UpdateSyncImageTags();

	_chatSettingPage.Update();

	_UpdateSaveChatCtrl();
	_UpdateLoadChatCtrl();

	_UpdateSwitchChat();

	_UpdateContextUsage();

	_chatBrief.Update(*this);


	CDialog::OnTimer(nIDEvent);
}


void CChatDialogA::_BrowseInputHistory(bool isPrev)
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

void CChatDialogA::_RefreshTitleMenu()
{
	_ui.ClearTitlebarMenuItems();

	_ui.AddTitlebarMenuItem(L"newchat", L"New Chat", L"");

	std::wstring curfileName = utf8_to_widechar(_agent.GetFileName());
	std::vector<CChatHistory::MenuItemInfo> menuItems;
	_chatHistory.GetRecentMenuItems(100, menuItems);
	for (const auto& item : menuItems)
	{
		if (item.uid != curfileName)
			_ui.AddTitlebarMenuItem(item.uid.c_str(), item.content.c_str(), item.stamp.c_str());
	}

}


void CChatDialogA::_UpdateSaveChatCtrl()
{
	if (_agent.GetFileVer() > _chatFileVer)
	{
		_chatHistory.Add(_agent.GetFileName());
		_chatFileVer = _agent.GetFileVer();
	}
}

void CChatDialogA::_UpdateLoadChatCtrl()
{
	std::string dbFolderPath = GetOpenedDBFolderPath_utf8();

	if (!_ui.IsReady())
		return;

	std::string chatsFolder = dbFolderPath;
	if (!dbFolderPath.empty())
		chatsFolder += "\\_chats";

	if (chatsFolder != _chatHistory.GetFolderPath())
	{
		if (_agent.IsWorking())
		{
			_OnStopButtonClicked();
			return;
		}
		if (_agent.IsRequestSave())
			return;
	}

	if (chatsFolder != _chatHistory.GetFolderPath())
	{
		_chatHistory.Clear();
		if (!chatsFolder.empty())
			_chatHistory.Init(chatsFolder.c_str());

		Utils::LoadLlmSkills(g_llmSkills, GetOpenedDBFolderPath_utf8());

		_ShutdownAgent();

		_chatInput.ClearTags();

		std::string fileName = _chatHistory.GetRecentFileName();

		_InitAgent(fileName.c_str());

		UpdateSettingMenuButton();

		_LoadChatInputTagsFromChatCtrl();
	}
}

//====================== WebView消息处理实现 ======================

void CChatDialogA::_OnWebViewMessage(const std::wstring& message)
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
	else if (action == "cliStatusChange")
	{
		// 处理 CLI 状态变化（accept/reject/stop）
		if (jsonMsg.contains("cliId") && jsonMsg.contains("cliStatus"))
		{
			std::wstring cliId = utf8_to_widechar(jsonMsg["cliId"].get<std::string>());
			std::wstring cliStatus = utf8_to_widechar(jsonMsg["cliStatus"].get<std::string>());
			
			CliStatus status = CliStatus::None;
			if (cliStatus == L"accept")
				status = CliStatus::Accept;
			else if (cliStatus == L"reject")
				status = CliStatus::Reject;
			else if (cliStatus == L"stop")
				status = CliStatus::Stop;
			
			_ui.SetCliStatus(cliId, status);
		}
	}
	else if (action == "cliWhitelist")
	{
		// 处理 CLI 白名单按钮点击事件
		if (jsonMsg.contains("cliId"))
		{
			std::wstring cliId = utf8_to_widechar(jsonMsg["cliId"].get<std::string>());
			
			_HandleCliWhitelist(cliId);
		}
	}
	// 可以在这里添加其他消息类型的处理
	// else if (action == "otherAction") { ... }
}

void CChatDialogA::_LoadChatInputTagsFromChatCtrl()
{
	std::vector<ChatInputTag> tags;
	_agent.GetLastNotDisabledSessionTags(tags);

	_chatInput.ClearTags();
	for (int i = 0;i < tags.size();i++)
	{
		_chatInput.AddFilePathTag(tags[i].path.c_str(), tags[i].visible);
// 		_chatInput.AddTag(tags[i].text, tags[i].type, tags[i].path, tags[i].color, tags[i].removable, tags[i].visible);
	}
}

void CChatDialogA::_UpdateSwitchChat()
{
	if (_requestSwitchChat.IsValid())
	{
		if (_agent.IsWorking())
		{
			_agent.RequestInterrupt();
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
		_ShutdownAgent();

		// 清空输入框
		_inputHistory.OnSendCurrent();
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
		_chatInput.ClearTags();

		_InitAgent("");

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
		_ShutdownAgent();
		_InitAgent(fileName.c_str());

		_LoadChatInputTagsFromChatCtrl();
	}
	_chatBrief.Activate();
	_requestSwitchChat.Reset();

}

void CChatDialogA::_HandleTitlebarMenuItemClicked(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp)
{
	_requestSwitchChat.chatFileName = menuItemId;
	_requestSwitchChat.t = GetAbsTick();

	_UpdateSwitchChat();
}

void CChatDialogA::_HandleUserMessageRestoreClicked(const std::wstring& messageId)
{
	if (_agent.IsWorking())
		return;

	_checkpointsFileChange.Deactivate();

	// 定义确认回调
	auto confirmCallback = [this](const std::vector<std::string>& modifiedFiles) -> bool
	{
		// 有文件被修改，弹出确认对话框
		CString message = _T("The following files have been modified after checkpoint was recorded:\r\n\r\n");
		for (const auto& filePath : modifiedFiles)
		{
			message += utf8_to_widechar(filePath.c_str()).c_str();
			message += _T("\r\n");
		}
		message += _T("\r\nContinuing the restore operation will overwrite these modifications. Do you want to continue?");

		int result = MessageBox(message, _T("Confirm Restore"), MB_YESNO | MB_ICONWARNING);
		return result == IDYES;
	};

	// 调用 CChatAgent::RestoreUserMessage
	if (_agent.RestoreUserMessage(messageId, confirmCallback))
	{
		// 将恢复的用户消息内容设置到ChatInput中
		_RestoreUserMessageToInput(messageId);
	}
}

void CChatDialogA::_RestoreUserMessageToInput(const std::wstring& messageId)
{
	// 获取用户消息的内容
	std::wstring messageContent;
	if (!_agent.GetUserMessageContent(messageId, messageContent))
	{
		// 没有找到用户消息，清空输入框
		_inputHistory.OnSendCurrent();
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
		_chatInput.ClearTags();
		return;
	}

	// 获取用户消息所在session的tags
	std::vector<ChatInputTag> sessionTags;
	_agent.GetUserMessageSessionTags(messageId, sessionTags);

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

void CChatDialogA::_HandleDisabledMessageClicked(const std::wstring& messageId)
{
	// 定义确认回调
	auto confirmCallback = [this](const std::vector<std::string>& modifiedFiles) -> bool
	{
		// 有文件被修改，弹出确认对话框
		CString message = _T("The following files have been modified after restore:\r\n\r\n");
		for (const auto& filePath : modifiedFiles)
		{
			message += utf8_to_widechar(filePath.c_str()).c_str();
			message += _T("\r\n");
		}
		message += _T("\r\nContinuing the operation will lose these modifications. Do you want to continue?");

		int result = MessageBox(message, _T("Confirm Undo"), MB_YESNO | MB_ICONWARNING);
		return result == IDYES;
	};

	// 调用 CChatAgent::RestoreDisabledMessage
	_agent.RestoreDisabledMessage(confirmCallback);
} 

void CChatDialogA::_HandleFileEditTitleClicked(const std::wstring& fileEditId)
{
	std::string filePath;
	FilesCheckpointUID oldCheckpointId, newCheckpointId;
	if (!_agent.GetFileEditDiff(fileEditId, filePath, oldCheckpointId, newCheckpointId))
		return;

	_checkpointsFileChange.Activate(oldCheckpointId, newCheckpointId, filePath.c_str());
}

void CChatDialogA::_HandleFileSummarizeClicked(const std::wstring& messageId, const std::wstring& filePath)
{
	FilesCheckpointUID oldCheckpointId, newCheckpointId;
	if (!_agent.GetFileSummarizeDiff(messageId, filePath, oldCheckpointId, newCheckpointId))
		return;

	std::string localFilePath = widechar_to_utf8(filePath.c_str());
	StringLower(localFilePath);
	_checkpointsFileChange.Activate(oldCheckpointId, newCheckpointId, localFilePath.c_str());
}

bool CChatDialogA::_OnBeforeSendToLlm(bool isUserMessage)
{
	_tokenStats.Update();
	_tokenStats.BeginCalibration();
	return true;
}

void CChatDialogA::_OnAfterReceiveFromLlm()
{
	_tokenStats.ApplyCalibration(_agent.GetOpsCtrl().GetRecentPromptToken());
}

void CChatDialogA::_HandleTagClicked(const std::wstring& tagId)
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

void CChatDialogA::_HandleSettingsButtonClicked()
{
	// 获取设置按钮的位置
	CRect rect;
	_ui.GetWindowRect(&rect);
	CRect btnRect;
	// 设置按钮在右上角，距离右边约40像素，标题栏高度约35像素
	btnRect.left = rect.right - 45;  // 距离右边45像素（按钮宽度约30+边距）
	btnRect.top = rect.top + 4;      // 标题栏顶部偏移约4像素
	btnRect.right = btnRect.left + 30; // 按钮宽度
	btnRect.bottom = btnRect.top + 28; // 按钮高度
	_settingMenuWindow.ShowWindow(btnRect);
}

void CChatDialogA::_HandleSettingMenuItemClicked(const std::wstring& itemName)
{
	if (itemName == L"setting")
	{
		// 显示设置页面
		ShowChatSettingPage();
	}
	else if (itemName == L"database_folder")
	{
		// 打开 database folder
		const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
		if (dbFolderPath && dbFolderPath[0] != '\0')
		{
			ShellExecuteA(NULL, "open", "explorer.exe", dbFolderPath, NULL, SW_SHOWNORMAL);
		}
	}
	else if (itemName == L"cli_whitelist.ini")
	{
		// 打开 cli_whitelist.ini
		Utils::EnsureCliWhitelists();
		std::string filePath = std::string(Utils::GetDBRootFolder_utf8()) + "\\" + LAZYBUG_CLI_WHITELIST_FILENAME;
		FileLocation loc;
		GetFileLocator().Request(filePath.c_str(), loc);
	}
	else if (itemName == L"global_rules.md")
	{
		// 打开 global_rules.md
		Utils::EnsureGlobalRulesFile();
		FileLocation loc;
		GetFileLocator().Request(Utils::GetGlobalRulesFilePath().c_str(), loc);
	}
	else if (itemName == L"project_rules.md")
	{
		// 打开 project_rules.md
		Utils::EnsureProjectRulesFile();
		FileLocation loc;
		GetFileLocator().Request(Utils::GetProjectRulesFilePath().c_str(), loc);
	}
}

void CChatDialogA::_HandleQuerySymbolLocations(const std::wstring& messageId, const std::vector<std::wstring>& symbols)
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

void CChatDialogA::_HandleSymbolLinkClicked(const std::wstring& symbol, const std::vector<std::pair<std::wstring, int>>& results)
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

void CChatDialogA::_OnStopButtonClicked()
{
	if (_agent.IsWorking())
	{
		_agent.RequestInterrupt();
	}
}

void CChatDialogA::_OnSendMessage(const std::wstring& content)
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

	// 如果已经有活动会话，不处理用户输入
	if (_agent.IsWorking())
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

		_agent.RemoveDisabledSessions();

		// dump skills 并通过 StartSession 传入，working 周期内复用
		extern CLlmSkills g_llmSkills;
		std::string skillsDump;
		Utils::LoadLlmSkills(g_llmSkills, GetOpenedDBFolderPath_utf8());
		g_llmSkills.Dump(skillsDump);
		_agent.StartSession(content, g_llmLib.GetMajorChatApi(), _chatInput.GetTags(), skillsDump.c_str());

		// 显示stop按钮
		_chatInput.ShowStopButton();

		// 清空编辑框
		_inputHistory.OnSendCurrent();
		_chatInput.SetInputContent_(_inputHistory.GetCurrentContent());
	}
}


//====================== ChatSettingPage控制方法实现 ======================

void CChatDialogA::ShowChatSettingPage()
{
	if (_chatSettingPage.GetSafeHwnd())
	{
		_chatSettingPage.ShowWindow(SW_SHOW);
		_chatSettingPage.BringWindowToTop();
		_RecalcLayout(); // 重新计算布局
	}
}

void CChatDialogA::HideChatSettingPage()
{
	if (_chatSettingPage.GetSafeHwnd())
	{
		_chatSettingPage.ShowWindow(SW_HIDE);
		_RecalcLayout(); // 重新计算布局
	}
}

bool CChatDialogA::IsChatSettingPageVisible() const
{
	if (_chatSettingPage.GetSafeHwnd())
	{
		return (_chatSettingPage.GetStyle() & WS_VISIBLE) != 0;
	}
	return false;
}

void CChatDialogA::_HandleChatSettingPageClose()
{
	if (_chatSettingPage.IsValidatingProvider())
		return;
	HideChatSettingPage();
	_chatInput.UpdateMajorChatApiMenu();
}

void CChatDialogA::ActivateCheckpointFileChange(const std::wstring& fileEditId)
{
	std::string filePath;
	FilesCheckpointUID oldCheckpointId, newCheckpointId;
	if (!_agent.GetFileEditDiff(fileEditId, filePath, oldCheckpointId, newCheckpointId))
		return;

	_checkpointsFileChange.Activate(oldCheckpointId, newCheckpointId, filePath.c_str());

}

// 自定义消息处理函数实现
LRESULT CChatDialogA::OnHandleUserMessageRestore(WPARAM wParam, LPARAM lParam)
{
	if (!_pendingUserMessageId.empty())
	{
		_HandleUserMessageRestoreClicked(_pendingUserMessageId);
		_pendingUserMessageId.clear();
	}
	return 0;
}

LRESULT CChatDialogA::OnHandleDisabledMessage(WPARAM wParam, LPARAM lParam)
{
	if (!_pendingDisabledMessageId.empty())
	{
		_HandleDisabledMessageClicked(_pendingDisabledMessageId);
		_pendingDisabledMessageId.clear();
	}
	return 0;
}

void CChatDialogA::SetFocusToChatInput()
{
	SetFocus();
	if (_chatInput.GetSafeHwnd())
	{
		_chatInput.RequestOccupyFocus();
	}
}

void CChatDialogA::_UpdateContextUsage()
{
	// 更新统计（检测变化）
	_tokenStats.Update();

	ChatOpCompressIntensity intensity = CChatOpsCompress::LoadIntensityForCurrentApi();

	// 获取当前使用的API名称
	std::string currentApiName = g_llmLib.GetMajorChatApi();

	// 没有变化则直接返回
	if ((!_tokenStats.HasAnyChanged()) && (currentApiName == _apiNameOfContextUsage))
		return;

	if (!_chatInput.IsReady())
		return;

	// 更新压缩强度
	if (intensity!= _compressLevelOfContextUsage)
	{
		// 映射: None->0, Low->1, Medium->2, High->3, Extreme->4
		int jsIntensity = 0;
		switch (intensity)
		{
		case ChatOpCompressIntensity::None:
			jsIntensity = 0;
			break;
		case ChatOpCompressIntensity::Low:
			jsIntensity = 1;
			break;
		case ChatOpCompressIntensity::Medium:
			jsIntensity = 2;
			break;
		case ChatOpCompressIntensity::High:
			jsIntensity = 3;
			break;
		case ChatOpCompressIntensity::Extreme:
			jsIntensity = 4;
			break;
		}
		
		// 生成 tooltip
		std::wstring tooltip;
		if (jsIntensity == 0)
		{
			tooltip = L"Context Level Max\nMax context usage will be controlled below 500k tokens";
		}
		else
		{
			int displayLevel = 5 - jsIntensity;
			int maxTokens = 0;
			switch (jsIntensity)
			{
			case 1: maxTokens = 160; break;  // Low
			case 2: maxTokens = 80; break;   // Medium
			case 3: maxTokens = 40; break;   // High
			case 4: maxTokens = 20; break;   // Extreme
			}
			tooltip = L"Context Level " + std::to_wstring(displayLevel) + 
				L"\nMax context usage will be controlled around " + std::to_wstring(maxTokens) + L"k tokens";
		}
		
		_chatInput.SetCompressIntensity(jsIntensity, tooltip);
	}

	if (true)
	{
		// 获取总 Token 数
		int totalTokens = _tokenStats.GetCalibratedTokens();

		// 格式化 token 数量显示
		std::wstring sizeText;
		const int K = 1024;
		const int M = K*K;

		if (totalTokens < K) {
			// < 1k: xxxB
			sizeText = std::to_wstring(totalTokens) + L" tokens";
		}
		else if (totalTokens < M) {
			// < 1m: xx.xxK
			double kValue = static_cast<double>(totalTokens) / K;
			wchar_t buf[32];
			swprintf_s(buf, L"%.2fk tokens", kValue);
			sizeText = buf;
		}
		else {
			// >= 1m: x.xxM
			double mValue = static_cast<double>(totalTokens) / M;
			wchar_t buf[32];
			swprintf_s(buf, L"%.2fm tokens", mValue);
			sizeText = buf;
		}
		
		std::wstring sizeTooltip = L"Current context usage: " + sizeText;
		
		// 如果有压缩，显示原始 token 数
		int uncompressedTokens = _tokenStats.GetUncompressedCalibratedTokens();
		if (uncompressedTokens > totalTokens)
		{
			std::wstring uncompressedText;
			if (uncompressedTokens < K) {
				uncompressedText = std::to_wstring(uncompressedTokens) + L" tokens";
			}
			else if (uncompressedTokens < M) {
				double kValue = static_cast<double>(uncompressedTokens) / K;
				wchar_t buf[32];
				swprintf_s(buf, L"%.2fk tokens", kValue);
				uncompressedText = buf;
			}
			else {
				double mValue = static_cast<double>(uncompressedTokens) / M;
				wchar_t buf[32];
				swprintf_s(buf, L"%.2fm tokens", mValue);
				uncompressedText = buf;
			}
			sizeTooltip += L"\n(uncompressed: " + uncompressedText + L")";
		}
		
		_chatInput.SetCompressedSize(sizeText, sizeTooltip);
	}

	_apiNameOfContextUsage = currentApiName;
	_compressLevelOfContextUsage = intensity;
	_tokenStats.ClearAllChanged();

	if (true)
	{
		if (_agent.GetCompressor().IsSummarizing())
			_chatInput.StartContextLevelFlowing();
		else
			_chatInput.StopContextLevelFlowing();
	}
}


void CChatDialogA::_OnInputContentChanged(const std::wstring& content)
{
	_inputHistory.OnModifyCurrent(content);

// 	if (_agent.IsWorking())
// 		_agent.Pause();

}

void CChatDialogA::_HandleEscape()
{
	_requestEscapeInputTime = GetAbsTick();

}

void CChatDialogA::_HandleSkillButtonClicked(const RECT& btnRect)
{
	// btnRect 包含屏幕上按钮的绝对坐标 (left, top, right, bottom)
	// 弹出Skills选择窗口
	Utils::LoadLlmSkills(g_llmSkills, GetOpenedDBFolderPath_utf8());
	_chatSkillsTree.ShowWindow(btnRect);
}

void CChatDialogA::_HandleCliWhitelist(const std::wstring& cliId)
{
	Utils::EnsureCliWhitelists();

    std::string filePath = std::string(Utils::GetDBRootFolder_utf8()) + "\\" + LAZYBUG_CLI_WHITELIST_FILENAME;
    FileLocation loc;
    GetFileLocator().Request(filePath.c_str(), loc);
}

// 更新设置菜单按钮状态（根据是否有打开的数据库文件夹）
void CChatDialogA::UpdateSettingMenuButton()
{
	const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
	bool hasDbFolder = (dbFolderPath && dbFolderPath[0] != '\0');
	
	// 如果没有打开的数据库文件夹，禁用 database_folder 和 project_rules.md 菜单项
	_settingMenuWindow.SetItemEnabled(L"database_folder", hasDbFolder);
	_settingMenuWindow.SetItemEnabled(L"project_rules.md", hasDbFolder);
}


int CChatDialogA::_OnCheckCompress()
{

	int balance = 100;
	float compressRatio = 1.7f;

	if (balance <= 0 || compressRatio <= 1.0f)
		return 0;

	int currentTokens = _tokenStats.GetCalibratedTokens();
	int threshold = static_cast<int>(balance * compressRatio);

	// 当前 token 未超过阈值，不需要压缩
	if (currentTokens <= threshold)
		return 0;

	// 计算目标 token 数（balance / ratio 的倒数，即压缩到 balance）
	int targetTokens = (int)(((float)balance)/ compressRatio);
	int reduceTokens = currentTokens - targetTokens;

	return reduceTokens;
}
