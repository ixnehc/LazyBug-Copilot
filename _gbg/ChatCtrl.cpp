#include "stdh.h"
#include "ChatCtrl.h"
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <pathcch.h> // For PathCchRemoveFileSpec - Link with Pathcch.lib
#include "timer/wuid.h"
#include "datapacket/DataPacket.h"
#include "LlmSession.h"
#include "stringparser/stringparser.h"

#include "Utils.h"
#include "utils_image.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const char* str);

//////////////////////////////////////////////////////////////////////////
//ChatCtrlOp

// ChatCtrlOp版本定义已删除

void ChatCtrlOp::Save(CDataPacket& dp)
{
    // 写入操作类型
    DWORD opType = static_cast<DWORD>(type);
    dp.Data_WriteSimple(opType);
    
    // 写入通用参数
    dp.Data_WriteWString(messageId);
    dp.Data_WriteWString(content);
    
    // 写入FileEdit相关参数
    dp.Data_WriteWString(fileEditId);
    dp.Data_WriteWString(title);
    dp.Data_WriteWString(fullPath);
    dp.Data_WriteWString(diffContent);
    
    // 写入ChatSession相关参数
    dp.Data_WriteSimple(checkpointId);
}

void ChatCtrlOp::Load(CDataPacket& dp,DWORD ver)
{
    // 读取操作类型
    DWORD opType = dp.Data_ReadSimple<DWORD>();
    type = static_cast<Type>(opType);
    
    // 读取通用参数
    dp.Data_ReadWString(messageId);
    dp.Data_ReadWString(content);
    
    // 读取FileEdit相关参数
    dp.Data_ReadWString(fileEditId);
    dp.Data_ReadWString(title);
    dp.Data_ReadWString(fullPath);
    dp.Data_ReadWString(diffContent);
    
    // 读取ChatSession相关参数
	if (ver <= CHATCTRL_OPERATIONS_VERSION_1_2)
	{
		FileChangeListUID changelistId = dp.Data_ReadSimple<FileChangeListUID>();
	}
    checkpointId = dp.Data_ReadSimple<FilesCheckpointUID>();
}

void ChatCtrlOp::Load(std::ifstream& file,DWORD ver)
{
    // 辅助函数：读取wstring
    auto ReadWString = [&file]() -> std::wstring {
        int len;
        file.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (len <= 0) return L"";
        
        std::wstring str((len / sizeof(wchar_t)) - 1, L'\0');
        file.read(reinterpret_cast<char*>(&str[0]), len);
        return str;
    };
    
    // 读取操作类型
    DWORD opType;
    file.read(reinterpret_cast<char*>(&opType), sizeof(opType));
    type = static_cast<Type>(opType);
    
    // 读取通用参数
    messageId = ReadWString();
    content = ReadWString();
    
    // 读取FileEdit相关参数
    fileEditId = ReadWString();
    title = ReadWString();
    fullPath = ReadWString();
    diffContent = ReadWString();
    
    // 读取ChatSession相关参数
	if (ver <= CHATCTRL_OPERATIONS_VERSION_1_2)
	{
		FileChangeListUID changelistId;
		file.read(reinterpret_cast<char*>(&changelistId), sizeof(changelistId));
	}
    file.read(reinterpret_cast<char*>(&checkpointId), sizeof(checkpointId));
}


//////////////////////////////////////////////////////////////////////////
//CChatCtrl


BEGIN_MESSAGE_MAP(CChatCtrl, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

// 构造函数
CChatCtrl::CChatCtrl()
    : _isWebViewCreated(false)
    , _isChatInitialized(false)
    , _callbackId(0)
    , _title(DEFAULT_CHAT_TITLE)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
	, _ver(0)
	, _undoCheckpointId(FilesCheckpointUID_Invalid)
	, _restoredCheckpointId(FilesCheckpointUID_Invalid)
	, _recentPrompToken(0)
{
	// 创建标题栏菜单窗口
	_titleMenuWindow.CreateTitleMenuWindow(this);
}

// 析构函数
CChatCtrl::~CChatCtrl()
{
    // 确保COM对象在析构时正确释放
    if (_webView != nullptr && _navigationCompletedToken.value != 0)
    {
        _webView->remove_NavigationCompleted(_navigationCompletedToken);
        _navigationCompletedToken.value = 0;
    }

    if (_webView != nullptr && _webMessageReceivedToken.value != 0)
    {
        _webView->remove_WebMessageReceived(_webMessageReceivedToken);
        _webMessageReceivedToken.value = 0;
    }

    // 释放COM对象
    SAFE_RELEASE(_webView);
    SAFE_RELEASE(_controller);
    SAFE_RELEASE(_webViewEnvironment);

	// 销毁标题栏菜单窗口
	if (_titleMenuWindow.GetSafeHwnd())
	{
		_titleMenuWindow.DestroyWindow();
	}
}

// 创建WebView2控件
BOOL CChatCtrl::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)::GetStockObject(WHITE_BRUSH),
        ::LoadIcon(NULL, IDI_APPLICATION));

    // 创建窗口
    BOOL result = CWnd::CreateEx(0, className, _T("WebView Control"),
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN|WS_BORDER, rect, pParentWnd, nID);

    if (result)
    {
        // 初始化WebView2环境
        HRESULT hr = InitializeWebView();
        if (FAILED(hr))
        {
            TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
            return FALSE;
        }
    }

    return result;
}

// 初始化WebView2环境
HRESULT CChatCtrl::InitializeWebView()
{
    // 创建WebView2环境
	extern const wchar_t* GetWebViewUserFolder();
	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, Utils::GetWebViewUserFolder(), nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
                if (SUCCEEDED(result))
                {
                    // 保存环境
                    _webViewEnvironment = environment;
                    if (_webViewEnvironment) _webViewEnvironment->AddRef();

                    // 创建WebView2控制器
                    return _webViewEnvironment->CreateCoreWebView2Controller(GetSafeHwnd(),
                        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                                if (SUCCEEDED(result))
                                {
                                    // 保存控制器
                                    _controller = controller;
                                    if (_controller) _controller->AddRef();

                                    _controller->put_IsVisible(TRUE);

                                    // 获取WebView
                                    _controller->get_CoreWebView2(&_webView);
                                    // get_CoreWebView2 已经AddRef，无需再次AddRef

                                    // 设置边界
                                    RECT bounds;
                                    GetClientRect(&bounds);
                                    _controller->put_Bounds(bounds);

#ifdef DISABLE_WEBVIEW_CONTEXTMENU
                                    // 禁用WebView2默认右键菜单
                                    ICoreWebView2Settings* settings = nullptr;
                                    if (SUCCEEDED(_webView->get_Settings(&settings)) && settings)
                                    {
                                        settings->put_AreDefaultContextMenusEnabled(FALSE);
                                        settings->Release();
                                    }
#endif

                                    // 设置WebView通信处理
                                    // 导航完成事件
                                    _webView->add_NavigationCompleted(
                                        Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                            [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                                BOOL success = FALSE;
                                                args->get_IsSuccess(&success);
                                                if (_navigationCompletedCallback)
                                                {
                                                    _navigationCompletedCallback(success == TRUE);
                                                }
                                                
                                                // 如果导航成功且还没初始化聊天界面，则初始化
                                                if (success && !_isChatInitialized)
                                                {
                                                    InitializeChatUI();
                                                }

												sender->add_ProcessFailed(
                                                    Microsoft::WRL::Callback<ICoreWebView2ProcessFailedEventHandler>(
														[this](ICoreWebView2* sender, ICoreWebView2ProcessFailedEventArgs* args) -> HRESULT {
													COREWEBVIEW2_PROCESS_FAILED_KIND kind;
													args->get_ProcessFailedKind(&kind);
													// 可以尝试获取更多信息，如 Reason, ExitCode, ProcessDescription
//                                                     HRESULT exitCode;
//                                                     args->get_ExitCode(&exitCode);
//                                                     LPCWSTR processDescription = nullptr;
//                                                     args->get_ProcessDescription(&processDescription);

// 													CString errMsg;
// 													errMsg.Format(L"WebView2 process failed. Kind: %d", static_cast<int>(kind));
// 													OutputDebugString(errMsg);
// 													AfxMessageBox(errMsg); // 仅用于调试
													return S_OK;
												})
												.Get(),
												&_processFailedToken);
                                                
                                                return S_OK;
                                            }).Get(),
                                        &_navigationCompletedToken);

                                    // Web消息接收事件
                                    _webView->add_WebMessageReceived(
                                        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                            [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                                LPWSTR message;
                                                args->get_WebMessageAsJson(&message);
                                                
                                                // 处理标题栏点击消息
                                                std::wstring msgStr(message);
                                                if (msgStr.find(L"\"action\":\"titlebarClicked\"") != std::wstring::npos)
                                                {
                                                    // 调用外部回调更新菜单
                                                    if (_titlebarMenuUpdateCallback)
                                                    {
                                                        _titlebarMenuUpdateCallback();
                                                    }
                                                    
                                                    // 更新完成后显示菜单
                                                    ShowTitlebarMenu();
                                                }

                                                // 处理FileEdit标题点击消息
                                                else if (msgStr.find(L"\"action\":\"fileEditTitleClicked\"") != std::wstring::npos)
                                                {
                                                    // 解析fileEditId
                                                    size_t idStart = msgStr.find(L"\"fileEditId\":\"");
                                                    if (idStart != std::wstring::npos)
                                                    {
                                                        idStart += 14; // 跳过 "fileEditId":"
                                                        size_t idEnd = msgStr.find(L"\"", idStart);
                                                        if (idEnd != std::wstring::npos)
                                                        {
                                                            std::wstring fileEditId = msgStr.substr(idStart, idEnd - idStart);
                                                            
                                                            // 检查该FileEdit是否被disabled
                                                            bool isDisabled = false;
                                                            
                                                            // 查找FileEdit对应的操作记录
                                                            int fileEditIndex = _FindFileEditOpIndex(fileEditId);
                                                            
                                                            if (fileEditIndex != -1)
                                                            {
                                                                // 获取disable边界
                                                                int disableAfterIndex = _GetDisableAfterIndex();
                                                                
                                                                // 如果FileEdit操作在disable边界之后，则认为是disabled
                                                                if (fileEditIndex >= disableAfterIndex)
                                                                {
                                                                    isDisabled = true;
                                                                }
                                                            }
                                                            
                                                            // 只有在未被disabled时才调用回调
                                                            if (!isDisabled && _fileEditTitleClickedCallback)
                                                            {
                                                                _fileEditTitleClickedCallback(fileEditId);
                                                            }
                                                        }
                                                    }
                                                }
                                                // 处理设置按钮点击消息
                                                else if (msgStr.find(L"\"action\":\"settingsButtonClicked\"") != std::wstring::npos)
                                                {
                                                    // 调用设置按钮点击回调
                                                    if (_settingsButtonClickedCallback)
                                                    {
                                                        _settingsButtonClickedCallback();
                                                    }
                                                }
                                                // 处理目录按钮点击消息
                                                else if (msgStr.find(L"\"action\":\"tocButtonClicked\"") != std::wstring::npos)
                                                {
                                                    // 调用目录按钮点击回调
                                                    if (_tocButtonClickedCallback)
                                                    {
                                                        _tocButtonClickedCallback();
                                                    }
                                                }
                                                // 处理FileSummarize点击消息
                                                else if (msgStr.find(L"\"action\":\"fileSummarizeClicked\"") != std::wstring::npos)
                                                {
                                                    // 解析messageId
                                                    size_t messageIdStart = msgStr.find(L"\"messageId\":\"");
                                                    if (messageIdStart != std::wstring::npos)
                                                    {
                                                        messageIdStart += 13; // 跳过 "messageId":"
                                                        size_t messageIdEnd = msgStr.find(L"\"", messageIdStart);
                                                        if (messageIdEnd != std::wstring::npos)
                                                        {
                                                            std::wstring messageId = msgStr.substr(messageIdStart, messageIdEnd - messageIdStart);
                                                            
                                                            // 解析filePath
                                                            size_t filePathStart = msgStr.find(L"\"filePath\":\"");
                                                            if (filePathStart != std::wstring::npos)
                                                            {
                                                                filePathStart += 12; // 跳过 "filePath":"
                                                                size_t filePathEnd = msgStr.find(L"\"", filePathStart);
                                                                if (filePathEnd != std::wstring::npos)
                                                                {
                                                                    std::wstring filePath = msgStr.substr(filePathStart, filePathEnd - filePathStart);
                                                                    
                                                                    // 反转义JSON字符串
                                                                    filePath = UnescapeJsonString(filePath);
                                                                    
                                                                    // 调用FileSummarize点击回调
                                                                    if (_fileSummarizeClickedCallback)
                                                                    {
                                                                        _fileSummarizeClickedCallback(messageId, filePath);
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                // 处理打开外部URL消息
                                                else if (msgStr.find(L"\"action\":\"openExternalUrl\"") != std::wstring::npos)
                                                {
                                                    // 解析URL
                                                    size_t urlStart = msgStr.find(L"\"url\":\"");
                                                    if (urlStart != std::wstring::npos)
                                                    {
                                                        urlStart += 7; // 跳过 "url":"
                                                        size_t urlEnd = msgStr.find(L"\"", urlStart);
                                                        if (urlEnd != std::wstring::npos)
                                                        {
                                                            std::wstring url = msgStr.substr(urlStart, urlEnd - urlStart);
                                                            
                                                            // 使用ShellExecuteW在系统默认浏览器中打开URL
                                                            ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
                                                        }
                                                    }
                                                }
                                                else if (_webMessageReceivedCallback)
                                                {
                                                    _webMessageReceivedCallback(message);
                                                }
                                                CoTaskMemFree(message);
                                                return S_OK;
                                            }).Get(),
                                        &_webMessageReceivedToken);

                                    // 启用Web消息 - 修正API调用，使用正确的方法
                                    // WebView2 API变更，不再使用SetIsWebMessageEnabled
                                    // 而是使用add_WebMessageReceived后自动启用
                                    
									extern const char* GetCurModuleFolderPath_utf8();
									std::string htmlPath = GetCurModuleFolderPath_utf8();
//									htmlPath += "\\ChatCtrl.html";
									htmlPath += "\\ChatCtrlHtml\\index.html";
                                    Navigate(utf8_to_widechar(htmlPath.c_str()));
                                    
                                    // 标记WebView创建完成
                                    _isWebViewCreated = true;
                                }
                                return S_OK;
                            }).Get());
                }
                return S_OK;
            }).Get());

    return hr;
}

// 导航到指定URL
void CChatCtrl::Navigate(const std::wstring& url)
{
    if (_webView != nullptr)
    {
        _webView->Navigate(url.c_str());
    }
}

// 导航到HTML字符串
void CChatCtrl::NavigateToString(const std::wstring& htmlContent)
{
    if (_webView != nullptr)
    {
        _webView->NavigateToString(htmlContent.c_str());
    }
}

// 重新加载当前页面
void CChatCtrl::Reload()
{
    if (_webView != nullptr)
    {
        _webView->Reload();
    }
}

// 执行JavaScript脚本
void CChatCtrl::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
{
    if (_webView != nullptr)
    {
        int callbackId = -1;
        if (callback)
        {
            callbackId = _callbackId++;
            _scriptCallbacks[callbackId] = callback;
        }

        _webView->ExecuteScript(script.c_str(),
            Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                [this, callbackId](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                    if (callbackId >= 0 && _scriptCallbacks.find(callbackId) != _scriptCallbacks.end())
                    {
                        _scriptCallbacks[callbackId](resultObjectAsJson ? resultObjectAsJson : L"");
                        _scriptCallbacks.erase(callbackId);
                    }
                    return S_OK;
                }).Get());
    }
}

// 设置导航完成回调
void CChatCtrl::SetNavigationCompletedCallback(WebViewNavigationCompletedCallback callback)
{
    _navigationCompletedCallback = callback;
}

// 设置Web消息接收回调
void CChatCtrl::SetWebMessageReceivedCallback(WebViewMessageReceivedCallback callback)
{
    _webMessageReceivedCallback = callback;
}

// 设置标题栏菜单更新回调
void CChatCtrl::SetTitlebarMenuUpdateCallback(TitlebarMenuUpdateCallback callback)
{
    _titlebarMenuUpdateCallback = callback;
}



// 设置FileEdit标题点击回调
void CChatCtrl::SetFileEditTitleClickedCallback(FileEditTitleClickedCallback callback)
{
    _fileEditTitleClickedCallback = callback;
}

// 设置设置按钮点击回调
void CChatCtrl::SetSettingsButtonClickedCallback(SettingsButtonClickedCallback callback)
{
    _settingsButtonClickedCallback = callback;
}

// 设置目录按钮点击回调
void CChatCtrl::SetTocButtonClickedCallback(TocButtonClickedCallback callback)
{
    _tocButtonClickedCallback = callback;
}

// 设置标题栏菜单项点击回调
void CChatCtrl::SetTitleMenuItemClickedCallback(TitleMenuItemClickedCallback callback)
{
	_titleMenuItemClickedCallback = callback;
}

// 发送消息到WebView
void CChatCtrl::PostWebMessageAsJson(const std::wstring& message)
{
    if (_webView != nullptr)
    {
        _webView->PostWebMessageAsJson(message.c_str());
    }
}

// 调整WebView大小
void CChatCtrl::ResizeWebView()
{
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

// 消息处理：大小变化
void CChatCtrl::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    ResizeWebView();
}

// 消息处理：创建
int CChatCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

// 消息处理：销毁
void CChatCtrl::OnDestroy()
{
    CWnd::OnDestroy();

	// 隐藏标题栏菜单窗口
	_titleMenuWindow.HideMenu();

    // 关闭WebView
    if (_controller != nullptr)
    {
        _controller->Close();
    }
}

//====================== 聊天功能相关实现 ======================

// 生成唯一消息ID
std::wstring CChatCtrl::_GenMsgId()
{
    WUID wuid = GenWUID();
    return L"msg_" + std::to_wstring(wuid);
}


// 初始化聊天界面
void CChatCtrl::InitializeChatUI()
{

    if (_isChatInitialized)
        return;

    _isChatInitialized = true;

    // 初始化FileEdit相关数据
    _fileEdits.clear();

	// 设置标题栏菜单回调
	_titleMenuWindow.SetMenuItemClickedCallback([this](const std::wstring& menuItemId,
		const std::wstring& content, const std::wstring& stamp) {
		if (_titleMenuItemClickedCallback)
		{
			_titleMenuItemClickedCallback(menuItemId, content, stamp);
		}
	});
}

// 添加用户消息
void CChatCtrl::AddUserMessage(const std::wstring& message, const std::wstring& overrideMessageId)
{
    if (!_IsReady())
        return;
    
    // 生成或使用指定的消息ID
    std::wstring messageId;
    if (overrideMessageId.empty())
    {
        messageId = _GenMsgId();
    }
    else
    {
        messageId = overrideMessageId;
    }
    
    // 尝试解析JSON判断是否为full content
    bool isFullContent = false;
    try
    {
        std::string utf8Message = widechar_to_utf8(message.c_str());
        nlohmann::json parsed = nlohmann::json::parse(utf8Message);
        if (parsed.is_array())
        {
            isFullContent = true;
        }
    }
    catch (const nlohmann::json::exception&)
    {
        // 解析失败，不是JSON，按纯文本处理
        isFullContent = false;
    }
    
    // 构造JSON消息
    std::wstring jsonMessage;
    if (isFullContent)
    {
        // 完整内容，直接传递JSON数组
        jsonMessage = L"{\"action\":\"addUserMessage\",\"content\":" + message + L",\"id\":\"" + messageId + L"\",\"isFullContent\":true}";
    }
    else
    {
        // 纯文本，转义后传递
        std::wstring safeMessage = EscapeJsonString(message);
        jsonMessage = L"{\"action\":\"addUserMessage\",\"content\":\"" + safeMessage + L"\",\"id\":\"" + messageId + L"\",\"isFullContent\":false}";
    }
    PostWebMessageAsJson(jsonMessage);

    // 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_AddUserMessage);
        op.content = message;
        op.messageId = messageId;
        _AddOp(op);
    }
}

// 获取用户消息内容
bool CChatCtrl::GetUserMessageContent(const std::wstring& messageId, std::wstring& content) const
{
    content.clear();
    
    // 遍历操作记录，查找指定的用户消息
    for (const auto& op : _ops)
    {
        if (op.type == ChatCtrlOp::Op_AddUserMessage && op.messageId == messageId)
        {
            content = op.content;
            return true;
        }
    }
    
    return false;
}

// 开始新的AI流式消息
std::wstring CChatCtrl::StartStreamingAIMessage(const std::wstring& overrideMessageId)
{
    if (!_IsReady())
        return L"";
    
    // 生成或使用指定的消息ID
    std::wstring messageId;
    if (overrideMessageId.empty())
    {
		if (!_currentStreamingMessageId.empty())
			return _currentStreamingMessageId;
        messageId = _GenMsgId();
    }
    else
    {
        messageId = overrideMessageId;
    }
   
    _currentStreamingMessageId = messageId;
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"startAIMessage\",\"id\":\"" + messageId + L"\"}";
    
    // 发送消息到WebView
    PostWebMessageAsJson(jsonMessage);

	// 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_StartStreamingAIMessage);
        op.messageId = messageId;
        _AddOp(op);
    }

    return messageId;
}


// 添加AI流式消息增量内容
void CChatCtrl::AddStreamingAIMessage(const std::wstring& messageId, const std::wstring& incrementalContent)
{
    if (!_IsReady())
        return;
	HideFileEditProgressLabel(messageId);
    
    // 转义增量内容中的特殊字符，避免破坏JSON格式
    std::wstring safeContent = EscapeJsonString(incrementalContent);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"addToAIMessage\",\"content\":\"" + safeContent + 
                               L"\",\"id\":\"" + messageId + L"\",\"isComplete\":false}";
    
    // 发送消息到WebView
    PostWebMessageAsJson(jsonMessage);

	// 记录操作
    while (true)
    {
		ChatCtrlOp* lastOp = _GetLastOp();
		if (lastOp->type == ChatCtrlOp::Op_AddStreamingAIMessage)
		{
			if (lastOp->messageId == messageId)
			{
				lastOp->content += incrementalContent;
				break;
			}
		}

        ChatCtrlOp op(ChatCtrlOp::Op_AddStreamingAIMessage);
        op.messageId = messageId;
        op.content = incrementalContent;
        _AddOp(op);
		break;
    }
}

// 添加AI流式thinking消息增量内容
void CChatCtrl::AddStreamingAIMessage_Thinking(const std::wstring& messageId, const std::wstring& incrementalContent)
{
    if (!_IsReady())
        return;

	HideFileEditProgressLabel(messageId);

    // 转义增量内容中的特殊字符，避免破坏JSON格式
    std::wstring safeContent = EscapeJsonString(incrementalContent);
    
    // 构造JSON消息，使用新的action类型
    std::wstring jsonMessage = L"{\"action\":\"addToAIMessage_Thinking\",\"content\":\"" + safeContent + 
                               L"\",\"id\":\"" + messageId + L"\",\"isComplete\":false}";
    
    // 发送消息到WebView
    PostWebMessageAsJson(jsonMessage);

	// 记录操作
    while (true)
    {
		ChatCtrlOp* lastOp = _GetLastOp();
		if (lastOp->type == ChatCtrlOp::Op_AddStreamingAIMessage_Thinking)
		{
			if (lastOp->messageId == messageId)
			{
				lastOp->content += incrementalContent;
				break;
			}
		}

        ChatCtrlOp op(ChatCtrlOp::Op_AddStreamingAIMessage_Thinking);
        op.messageId = messageId;
        op.content = incrementalContent;
        _AddOp(op);
		break;
    }
}

// 完成流式AI消息
void CChatCtrl::CompleteStreamingAIMessage(const std::wstring& messageId)
{
    if (!_IsReady())
        return;
	HideFileEditProgressLabel(messageId);
    
    // 当前流式消息ID清空
    if (_currentStreamingMessageId == messageId)
        _currentStreamingMessageId.clear();
    
    // 构造JSON消息，标记为完成，触发Markdown渲染
    std::wstring jsonMessage = L"{\"action\":\"addToAIMessage\",\"content\":\"\",\"id\":\"" + 
                               messageId + L"\",\"isComplete\":true}";
    
    // 发送消息到WebView
    PostWebMessageAsJson(jsonMessage);

	// 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_CompleteStreamingAIMessage);
        op.messageId = messageId;
        _AddOp(op);
    }
}

// 添加系统消息
void CChatCtrl::AddSystemMessage(const std::wstring& message, const std::wstring& overrideMessageId)
{
    if (!_IsReady())
        return;
    
    // 转义消息内容以防XSS
    std::wstring safeMessage = EscapeJsonString(message);
    
    // 生成或使用指定的消息ID
    std::wstring messageId;
    if (overrideMessageId.empty())
    {
        messageId = _GenMsgId();
    }
    else
    {
        messageId = overrideMessageId;
    }
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"addSystemMessage\",\"content\":\"" + safeMessage + 
                               L"\",\"id\":\"" + messageId + L"\"}";
    
    // 发送消息到WebView
    PostWebMessageAsJson(jsonMessage);

	// 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_AddSystemMessage);
        op.content = message;
        op.messageId = messageId;
        _AddOp(op);
    }
}

// 清空聊天记录
void CChatCtrl::ClearChat()
{
    if (!_IsReady())
        return;
    
    _currentStreamingMessageId.clear();
    
    // 清空FileEdit窗口数据
    _fileEdits.clear();
    
    // 清空操作记录
    _ops.clear();

	_recentPrompToken = 0;
    
    // 发送清空聊天的消息
    std::wstring jsonMessage = L"{\"action\":\"clearChat\"}";
    PostWebMessageAsJson(jsonMessage);


}

// Disable某个消息之后的所有消息
void CChatCtrl::DisableMessagesAfter(const std::wstring& messageId)
{
    if (!_IsReady())
        return;
    
    // 转义消息ID
    std::wstring safeMessageId = EscapeJsonString(messageId);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"disableMessagesAfter\",\"messageId\":\"" + safeMessageId + L"\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);

    // 记录操作
    ChatCtrlOp op(ChatCtrlOp::Op_DisableMessagesAfter);
    op.messageId = messageId;
    _AddOp(op);
}

// 启用所有被disabled的消息
void CChatCtrl::EnableAllDisabledMessages()
{
    if (!_IsReady())
        return;
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"enableAllDisabledMessages\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);

    // 清除之前所有的DisableMessagesAfter操作记录
    auto it = std::remove_if(_ops.begin(), _ops.end(), [](const ChatCtrlOp& op) {
        return op.type == ChatCtrlOp::Op_DisableMessagesAfter;
    });
    if (it != _ops.end()) {
        _ops.erase(it, _ops.end());
        _ver++; // 操作记录已更改，增加版本号
    }
}

// 删除所有disabled的session，并返回需要丢弃的checkpoint
void CChatCtrl::RemoveDisabledSessions(std::vector<FilesCheckpointUID>& checkpointsToDiscard)
{
    checkpointsToDiscard.clear();
    
    if (!_IsReady())
        return;

    // 获取 disable 边界
    int disableAfterIndex = _GetDisableAfterIndex();
    
    // 如果没有被disabled的消息，直接返回
    if (disableAfterIndex == static_cast<int>(_ops.size()))
        return;
    
    // 找到被disabled消息所在session的开始位置
    int sessionStartIndex = disableAfterIndex;
    
    // 向前查找最近的BeginSession操作
    for (int i = disableAfterIndex - 1; i >= 0; i--)
    {
        if (_ops[i].type == ChatCtrlOp::Op_BeginSession)
        {
            sessionStartIndex = i;
            break;
        }
        // 如果遇到另一个EndSession，说明前面是另一个完整的session，停止查找
        if (_ops[i].type == ChatCtrlOp::Op_EndSession)
        {
            break;
        }
    }
    
    // 收集从session开始到队列末尾的所有session中涉及的checkpoint
    std::unordered_set<FilesCheckpointUID> checkpointSet;
    
    for (int i = sessionStartIndex; i < static_cast<int>(_ops.size()); i++)
    {
        const ChatCtrlOp& op = _ops[i];
        if (op.checkpointId != FilesCheckpointUID_Invalid)
            checkpointSet.insert(op.checkpointId);
	}
    
    // 将checkpoint集合转换为vector
    checkpointsToDiscard.assign(checkpointSet.begin(), checkpointSet.end());
	if (_undoCheckpointId != FilesCheckpointUID_Invalid)
		checkpointsToDiscard.push_back(_undoCheckpointId);
	if (_restoredCheckpointId != FilesCheckpointUID_Invalid)
		checkpointsToDiscard.push_back(_restoredCheckpointId);

	_undoCheckpointId = FilesCheckpointUID_Invalid;
	_restoredCheckpointId = FilesCheckpointUID_Invalid;
    
    // 从操作队列中删除从session开始到队列末尾的所有操作
    if (sessionStartIndex < static_cast<int>(_ops.size()))
    {
        _ops.erase(_ops.begin() + sessionStartIndex, _ops.end());
        _ver++; // 操作记录已更改，增加版本号
    }
    
    // 从UI中删除所有disabled的消息元素
    std::wstring jsonMessage = L"{\"action\":\"removeDisabledMessages\"}";
    PostWebMessageAsJson(jsonMessage);
}

// 设置主题 (light/dark)
void CChatCtrl::SetTheme(const std::wstring& theme)
{
    if (!_IsReady())
        return;
    
    // 发送主题设置消息
    std::wstring jsonMessage = L"{\"action\":\"setTheme\",\"theme\":\"" + theme + L"\"}";
    PostWebMessageAsJson(jsonMessage);
}

//====================== FileEdit 内嵌窗口功能实现 ======================

// 在指定AI消息中添加FileEdit窗口
std::wstring CChatCtrl::AddFileEditToAIMessage(const std::wstring& messageId, const std::wstring& title, const std::wstring& fullPath, const std::wstring& content, const std::wstring& overrideFileEditId)
{
    if (!_IsReady())
        return L"";

	HideFileEditProgressLabel(messageId);
    
    // 生成或使用指定的FileEdit窗口ID
    std::wstring fileEditId;
    if (overrideFileEditId.empty())
    {
        fileEditId = _GenFileEditId();
    }
    else
    {
        fileEditId = overrideFileEditId;
    }
    
    // 创建FileEdit窗口数据
    ChatCtrlFileEdit window;
    window.id = fileEditId;
    window.title = title;
    window.fullPath = fullPath;
    window.content = content;
    window.messageId = messageId;
    window.isCollapsed = false;
    
    // 保存到内部存储
    _fileEdits.push_back(window);
    
    // 发送创建FileEdit窗口的消息
    _SendFileEditMsg(L"addFileEdit", window);

	// 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_AddFileEditToAIMessage);
        op.messageId = messageId;
        op.fileEditId = fileEditId;
        op.title = title;
        op.fullPath = fullPath;
        op.content = content;
        _AddOp(op);
    }
    
    return fileEditId;
}

// 设置FileEdit窗口标题
void CChatCtrl::SetFileEditTitle(const std::wstring& fileEditId, const std::wstring& title)
{
    if (!_IsReady())
        return;
    
    ChatCtrlFileEdit* window = _FindFileEdit(fileEditId);
    if (window != nullptr)
    {
        window->title = title;
        _SendFileEditMsg(L"updateFileEditTitle", *window);
    }

	// 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_SetFileEditTitle);
        op.fileEditId = fileEditId;
        op.title = title;
        _AddOp(op);
    }

}

// 设置FileEdit窗口显示内容
void CChatCtrl::SetFileEditContent(const std::wstring& fileEditId, const std::wstring& content, const std::wstring& diffContent, FilesCheckpointUID checkpointID)
{
    if (!_IsReady())
        return;
    
    ChatCtrlFileEdit* window = _FindFileEdit(fileEditId);
    if (window != nullptr)
    {
        window->content = content;
        window->diffContent = diffContent;
        _SendFileEditMsg(L"updateFileEditContent", *window);
    }

	// 从操作队列末尾开始查找最近的同fileEditId的SetFileEditContent操作
    ChatCtrlOp* targetOp = nullptr;
    for (auto it = _ops.rbegin(); it != _ops.rend(); ++it)
    {
        if (it->type == ChatCtrlOp::Op_SetFileEditContent && it->fileEditId == fileEditId)
        {
            targetOp = &(*it);
            break;
        }
    }

    if (targetOp != nullptr)
    {
        // 合并到找到的操作
        targetOp->content = content;
        targetOp->diffContent = diffContent;
        targetOp->checkpointId = checkpointID;
        _ver++; // 操作记录已更改，增加版本号
    }
    else
    {
        // 没有找到，新建操作
        ChatCtrlOp op(ChatCtrlOp::Op_SetFileEditContent);
        op.fileEditId = fileEditId;
        op.content = content;
        op.checkpointId = checkpointID;
        op.diffContent = diffContent;
        _AddOp(op);
    }
}

// 添加FileEdit窗口标题栏按钮
std::wstring CChatCtrl::AddFileEditButton(const std::wstring& fileEditId, const std::wstring& buttonText, const std::wstring& buttonAction, const std::wstring& overrideButtonId)
{
    if (!_IsReady())
        return L"";
    
    ChatCtrlFileEdit* window = _FindFileEdit(fileEditId);
    if (window != nullptr)
    {
        // 生成或使用指定的按钮ID
        std::wstring buttonId;
        if (overrideButtonId.empty())
        {
            buttonId = _GenFileEditBtnId();
        }
        else
        {
            buttonId = overrideButtonId;
        }
        
        // 创建按钮数据
        ChatCtrlFileEditBtn button;
        button.id = buttonId;
        button.text = buttonText;
        button.action = buttonAction;
        
        // 添加按钮到窗口
        window->buttons.push_back(button);
        
        // 发送更新按钮的消息
        _SendFileEditMsg(L"updateFileEditButtons", *window);
        
        return buttonId;
    }
    
    return L"";
}

// 折叠/展开FileEdit窗口
void CChatCtrl::ToggleFileEditCollapse(const std::wstring& fileEditId)
{
    if (!_IsReady())
        return;
    
    ChatCtrlFileEdit* window = _FindFileEdit(fileEditId);
    if (window != nullptr)
    {
        window->isCollapsed = !window->isCollapsed;
        _SendFileEditMsg(L"toggleFileEditCollapse", *window);
    }
}

// 开始FileEdit修改状态动画
void CChatCtrl::StartFileEditModification(const std::wstring& fileEditId)
{
    if (!_IsReady())
        return;
    
    // 转义fileEditId
    std::wstring safeFileEditId = EscapeJsonString(fileEditId);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"startFileEditModification\",\"fileEditId\":\"" + safeFileEditId + L"\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);
}

// 停止FileEdit修改状态动画
void CChatCtrl::StopFileEditModification(const std::wstring& fileEditId)
{
    if (!_IsReady())
        return;
    
    // 转义fileEditId
    std::wstring safeFileEditId = EscapeJsonString(fileEditId);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"stopFileEditModification\",\"fileEditId\":\"" + safeFileEditId + L"\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);
}

//====================== FileSummarize 功能实现 ======================

// 在指定AI消息中添加FileSummarize按钮
void CChatCtrl::AddFileSummarizeToAIMessage(const std::wstring& messageId, const std::wstring& filePath)
{
    if (!_IsReady())
        return;
    
    // 转义参数
    std::wstring safeMessageId = EscapeJsonString(messageId);
    std::wstring safeFilePath = EscapeJsonString(filePath);
    
    // 构造JSON消息，使用 PostWebMessageAsJson 发送
    std::wstring jsonMessage = L"{\"action\":\"addFileSummarize\",\"messageId\":\"" + safeMessageId + L"\",\"filePath\":\"" + safeFilePath + L"\"}";
    PostWebMessageAsJson(jsonMessage);

    // 记录操作
    ChatCtrlOp op(ChatCtrlOp::Op_AddFileSummarizeToAIMessage);
    op.messageId = messageId;
    op.fullPath = filePath;
    _AddOp(op);
}

// 设置FileSummarize点击回调
void CChatCtrl::SetFileSummarizeClickedCallback(FileSummarizeClickedCallback callback)
{
    _fileSummarizeClickedCallback = callback;
}

// 查找是否存在和这个fileEditId的messageId一致的Summarize
bool CChatCtrl::ExistSummarizeInSession(const std::wstring& fileEditId) const
{
	// 查找fileEditId对应的操作
	int fileEditIndex = _FindFileEditOpIndex(fileEditId);
	if (fileEditIndex < 0)
		return false;

	// 获取该fileEdit的messageId
	std::wstring messageId = _ops[fileEditIndex].messageId;

	// 查找是否存在相同messageId的Op_AddFileSummarizeToAIMessage操作
	for (int i = 0; i < static_cast<int>(_ops.size()); i++)
	{
		const ChatCtrlOp& op = _ops[i];
		if (op.type == ChatCtrlOp::Op_AddFileSummarizeToAIMessage && op.messageId == messageId)
		{
			return true;
		}
	}

	return false;
}

//====================== FileEdit 私有辅助方法实现 ======================

// 生成唯一FileEdit窗口ID
std::wstring CChatCtrl::_GenFileEditId()
{
    WUID wuid = GenWUID();
    return L"fileedit_" + std::to_wstring(wuid);
}

// 生成唯一按钮ID
std::wstring CChatCtrl::_GenFileEditBtnId()
{
    WUID wuid = GenWUID();
    return L"btn_" + std::to_wstring(wuid);
}

// 查找FileEdit窗口
ChatCtrlFileEdit* CChatCtrl::_FindFileEdit(const std::wstring& fileEditId)
{
    auto it = std::find_if(_fileEdits.begin(), _fileEdits.end(),
        [&fileEditId](const ChatCtrlFileEdit& window) { return window.id == fileEditId; });
    
    return (it != _fileEdits.end()) ? &(*it) : nullptr;
}

// 发送FileEdit相关消息到WebView
void CChatCtrl::_SendFileEditMsg(const std::wstring& action, const ChatCtrlFileEdit& window)
{
    // 构建按钮JSON数组
    std::wstring buttonsJson = _BuildButtonsJson(window.buttons);
    
    // 转义所有字符串内容
    std::wstring safeTitle = EscapeJsonString(window.title);
    std::wstring safeMessageId = EscapeJsonString(window.messageId);
    std::wstring safeFileEditId = EscapeJsonString(window.id);
    
    // 同时发送原始内容和diff内容，让JavaScript端决定显示哪个
    std::wstring safeContent = EscapeJsonString(window.content);
    std::wstring safeDiffContent = EscapeJsonString(window.diffContent);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{";
    jsonMessage += L"\"action\":\"" + action + L"\",";
    jsonMessage += L"\"fileEditId\":\"" + safeFileEditId + L"\",";
    jsonMessage += L"\"messageId\":\"" + safeMessageId + L"\",";
    jsonMessage += L"\"title\":\"" + safeTitle + L"\",";
    jsonMessage += L"\"content\":\"" + safeContent + L"\",";
    jsonMessage += L"\"diffContent\":\"" + safeDiffContent + L"\",";
    jsonMessage += L"\"isCollapsed\":";
    jsonMessage += window.isCollapsed ? L"true" : L"false";
    jsonMessage += L",";
    jsonMessage += L"\"buttons\":" + buttonsJson;
    jsonMessage += L"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);
}

// 构建FileEdit按钮的JSON数组
std::wstring CChatCtrl::_BuildButtonsJson(const std::vector<ChatCtrlFileEditBtn>& buttons)
{
    std::wstring json = L"[";
    
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        if (i > 0) json += L",";
        
        json += L"{";
        json += L"\"id\":\"" + EscapeJsonString(buttons[i].id) + L"\",";
        json += L"\"text\":\"" + EscapeJsonString(buttons[i].text) + L"\",";
        json += L"\"action\":\"" + EscapeJsonString(buttons[i].action) + L"\"";
        json += L"}";
    }
    
    json += L"]";
    return json;
}

//====================== WebView 标题栏功能实现 ======================

// 设置WebView标题栏标题
void CChatCtrl::SetTitle(const std::wstring& title)
{
    if (!_IsReady())
        return;
    
    _title = title;
    
    // 转义标题内容
    std::wstring safeTitle = EscapeJsonString(title);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"setWebViewTitle\",\"title\":\"" + safeTitle + L"\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);

	// 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_SetTitle);
        op.title = title;
        _AddOp(op);
    }
}

//====================== 标题栏菜单功能实现 ======================

// 添加标题栏菜单项
void CChatCtrl::AddTitlebarMenuItem(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp)
{
	// 添加到 C++ 标题栏菜单
	_titleMenuWindow.AddMenuItem(menuItemId, content, stamp);

	if (!_IsReady())
		return;

	// 同步到WebView (保持向后兼容，如果HTML中还有需要的话)
	// 转义内容
	std::wstring safeMenuItemId = EscapeJsonString(menuItemId);
	std::wstring safeContent = EscapeJsonString(content);
	std::wstring safeStamp = EscapeJsonString(stamp);

	// 构造JSON消息
	std::wstring jsonMessage = L"{";
	jsonMessage += L"\"action\":\"addTitlebarMenuItem\",";
	jsonMessage += L"\"menuItemId\":\"" + safeMenuItemId + L"\",";
	jsonMessage += L"\"content\":\"" + safeContent + L"\",";
	jsonMessage += L"\"stamp\":\"" + safeStamp + L"\"";
	jsonMessage += L"}";

	// 发送到WebView
	PostWebMessageAsJson(jsonMessage);
}

// 清空所有标题栏菜单项
void CChatCtrl::ClearTitlebarMenuItems()
{
	// 清空 C++ 标题栏菜单
	_titleMenuWindow.ClearMenuItems();

	if (!_IsReady())
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"clearTitlebarMenuItems\"}";

	// 发送到WebView
	PostWebMessageAsJson(jsonMessage);
}

// 显示标题栏菜单（仅显示C++原生菜单，不显示HTML菜单）
void CChatCtrl::ShowTitlebarMenu()
{
	if (!_titleMenuWindow.HasMenuItems())
		return;

	// 获取ChatCtrl窗口的位置和大小
	CRect rect;
	GetWindowRect(&rect);
	int x = rect.left;  // 左侧对齐到ChatCtrl窗口的左侧
	int y = rect.top + 30; // 标题栏下方
	int width = rect.Width();  // 使用ChatCtrl的宽度

	// 显示 C++ 标题栏菜单，传入ChatCtrl的宽度
	_titleMenuWindow.ShowMenu(x, y, width);

	// 不再发送showTitlebarMenu消息到WebView，避免显示HTML菜单
}

// 隐藏标题栏菜单（仅隐藏C++原生菜单，不通知HTML）
void CChatCtrl::HideTitlebarMenu()
{
	// 隐藏 C++ 标题栏菜单
	_titleMenuWindow.HideMenu();

	// 不再发送hideTitlebarMenu消息到WebView
}

// 切换标题栏菜单显示状态
void CChatCtrl::ToggleTitlebarMenu()
{
	// 切换 C++ 标题栏菜单的显示状态
	if (_titleMenuWindow.IsWindowVisible())
	{
		_titleMenuWindow.HideMenu();
	}
	else
	{
		ShowTitlebarMenu();
	}

	if (!_IsReady())
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"toggleTitlebarMenu\"}";

	// 发送到WebView
	PostWebMessageAsJson(jsonMessage);
}

bool CChatCtrl::HasTitle()
{
	const ChatCtrlOp* op = _FindLastOp(ChatCtrlOp::Op_SetTitle);
	if (!op)
		return false;
	if (op->title == DEFAULT_CHAT_TITLE)
		return false;
	if (op->title.empty())
		return false;
	return  true;
}


//====================== 操作记录和还原功能实现 ======================

// 添加操作到记录队列
void CChatCtrl::_AddOp(const ChatCtrlOp& op)
{
    _ver++;

    // 检查是否可以与最后一个操作合并
    if (!_ops.empty() && (op.type == ChatCtrlOp::Op_AddStreamingAIMessage || op.type == ChatCtrlOp::Op_AddStreamingAIMessage_Thinking))
    {
        ChatCtrlOp& lastOp = _ops.back();
        
        // 如果最后一个操作也是相同类型的流式消息且messageId相同，则合并内容
        if (lastOp.type == op.type && lastOp.messageId == op.messageId)
        {
            lastOp.content += op.content;
            return; // 合并完成，不添加新操作
        }
    }

	//Op_SetTitle,合并到已存在的Op
	if (op.type == ChatCtrlOp::Op_SetTitle)
	{
		ChatCtrlOp* existOp = (ChatCtrlOp * )_FindLastOp(ChatCtrlOp::Op_SetTitle);
		if (existOp)
		{
			existOp->title = op.title;
			return;
		}
	}
    
    // 没有合并，添加新操作
    _ops.push_back(op);
}

void CChatCtrl::AddSessionCost(const LlmSessionUsage &usage, const std::wstring& messageId)
{
    if (!_IsReady())
        return;

    // 使用新的格式化方法生成显示文本
    std::wstring displayText = usage.FormatToCostText();

    _SetSessionCostDisplay(displayText, messageId);

	// 记录操作，将费用信息存储在content字段中
	ChatCtrlOp op(ChatCtrlOp::Op_SetSessionCost);
	op.messageId = messageId;
	op.content = displayText;
	_AddOp(op);
}

void CChatCtrl::NotifyPromptCache(const LlmSessionUsage& usage)
{
	if (!_IsReady())
		return;

	_recentPrompToken = usage.inputToken_;

}



// 内部函数：设置会话费用显示（不记录操作，供_ExecuteOp调用）
void CChatCtrl::_SetSessionCostDisplay(const std::wstring& costText, const std::wstring& messageId)
{
    if (!_IsReady())
        return;

    // 转义显示文本以防止JSON格式错误
    std::wstring safeDisplayText = EscapeJsonString(costText);
    
    // 构造JSON消息，发送费用信息给WebView
    std::wstring jsonMessage = L"{\"action\":\"setCostDisplay\",\"costText\":\"" + safeDisplayText + L"\"";
    if (!messageId.empty())
    {
        std::wstring safeMessageId = EscapeJsonString(messageId);
        jsonMessage += L",\"messageId\":\"" + safeMessageId + L"\"";
    }
    jsonMessage += L"}";
    
    PostWebMessageAsJson(jsonMessage);


}

// 清空操作记录
void CChatCtrl::ClearOps()
{
    _ops.clear();
    _ver++;
}

// 从操作记录还原内容
void CChatCtrl::_ExecuteOp(const ChatCtrlOp& op)
{
    if (!_IsReady())
        return;
   
    switch (op.type)
    {
        case ChatCtrlOp::Op_AddUserMessage:
            AddUserMessage(op.content, op.messageId);
            break;
        case ChatCtrlOp::Op_AddSessionTag:
            _AddSessionTag(op.content,op.fullPath,true);
            break;
		case ChatCtrlOp::Op_AddSessionDisabledTag:
			_AddSessionTag(op.content, op.fullPath, false);
			break;
		case ChatCtrlOp::Op_StartStreamingAIMessage:
            StartStreamingAIMessage(op.messageId);
            break;
        case ChatCtrlOp::Op_AddStreamingAIMessage:
            AddStreamingAIMessage(op.messageId, op.content);
            break;
        case ChatCtrlOp::Op_AddStreamingAIMessage_Thinking:
            AddStreamingAIMessage_Thinking(op.messageId, op.content);
            break;
                
        case ChatCtrlOp::Op_CompleteStreamingAIMessage:
            CompleteStreamingAIMessage(op.messageId);
            break;
                
        case ChatCtrlOp::Op_AddSystemMessage:
            AddSystemMessage(op.content, op.messageId);
            break;
                
        case ChatCtrlOp::Op_AddFileEditToAIMessage:
            AddFileEditToAIMessage(op.messageId, op.title, op.fullPath, op.content, op.fileEditId);
            break;
                
        case ChatCtrlOp::Op_SetFileEditTitle:
            SetFileEditTitle(op.fileEditId, op.title);
            break;
                
        case ChatCtrlOp::Op_SetFileEditContent:
			SetFileEditContent(op.fileEditId, op.content, op.diffContent,op.checkpointId);
            break;
                
        case ChatCtrlOp::Op_SetTitle:
            SetTitle(op.title);
            break;
                
        case ChatCtrlOp::Op_BeginSession:
			BeginSession(op.checkpointId);
			break;
                
        case ChatCtrlOp::Op_EndSession:
            EndSession();
            break;

        case ChatCtrlOp::Op_DisableMessagesAfter:
            DisableMessagesAfter(op.messageId);
            break;

        case ChatCtrlOp::Op_SetSessionCost:
        {
            LlmSessionUsage usage = LlmSessionUsage::ParseFromCostText(op.content);
            AddSessionCost(usage, op.messageId);
            break;
        }

		case ChatCtrlOp::Op_FileAttaches:
		{
			AddFileAttaches(op.content, op.checkpointId);
			break;
		}

		case ChatCtrlOp::Op_AddToolCallResult:
		{
			AddToolCallResult(op.content);
			break;
		}

		case ChatCtrlOp::Op_AddReplaceInFileResult:
		{
			AddReplaceInFileResult(op.fullPath, op.content);
			break;
		}

		case ChatCtrlOp::Op_AddToolCallMessage:
		{
			AddToolCallMessage(op.messageId, op.content);
			break;
		}

		case ChatCtrlOp::Op_AddFileSummarizeToAIMessage:
		{
			AddFileSummarizeToAIMessage(op.messageId, op.fullPath);
			break;
		}

        default:
            // 未知操作类型，跳过
            break;
    }
}

void CChatCtrl::BeginSession(FilesCheckpointUID checkpointId)
{
    if (!_IsReady())
        return;

    // 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_BeginSession);
		op.checkpointId = checkpointId;
        _AddOp(op);
    }
}


void CChatCtrl::SetUndoCheckpoint(FilesCheckpointUID checkpointId)
{
	if (!_IsReady())
		return;

	extern CCheckpoints* GetCheckpoints();
	CCheckpoints* pCheckpoints = GetCheckpoints();
	if (!pCheckpoints)
		return;

	_undoCheckpointId = checkpointId;

	pCheckpoints->DiscardCheckpoint(_restoredCheckpointId);
	_restoredCheckpointId = FilesCheckpointUID_Invalid;

	// 根据undo checkpoint里的文件列表创建一个新的checkpoint记录当前状态
	if (checkpointId != FilesCheckpointUID_Invalid)
	{
		// 获取undo checkpoint中的文件列表
		std::vector<const char*> fileList;
		if (pCheckpoints->GetCheckpointFileList(checkpointId, fileList))
		{
			// 将const char*转换为std::string以便传递给CreateCheckpointFromFilelist
			std::vector<const char*> filePathList;
			std::vector<std::string> filePathStrings;
				
			for (const char* filePath : fileList)
			{
				if (filePath && filePath[0] != '\0')
				{
					filePathStrings.push_back(std::string(filePath));
				}
			}
				
			// 重新构建const char*列表
			for (const auto& pathStr : filePathStrings)
			{
				filePathList.push_back(pathStr.c_str());
			}
				
			// 创建新的checkpoint记录这些文件的当前状态
			if (!filePathList.empty())
			{
				_restoredCheckpointId = pCheckpoints->CreateCheckpointFromFilelist(filePathList);
			}
		}
	}

	_ver++;
}


void CChatCtrl::EndSession()
{
    if (!_IsReady())
        return;

    // 记录操作
    if (true)
    {
        ChatCtrlOp op(ChatCtrlOp::Op_EndSession);
        _AddOp(op);
    }
}

void CChatCtrl::AccumulateSessionCostForFileEdit(const std::wstring& fileEditId, float price, int inputToken, int outputToken)
{
    if (!_IsReady())
        return;

    int sessionBegin = _GetSessionBeginOfFileEdit(fileEditId);
    if (sessionBegin == -1)
        return;

    int firstOpIndex = _FindFirstOpIndexInSession(sessionBegin, ChatCtrlOp::Op_SetSessionCost);
    if (firstOpIndex == -1)
        return;

	ChatCtrlOp* existingCostOp = &_ops[firstOpIndex];

    
    std::wstring newDisplayText;
    
    if (existingCostOp != nullptr)
    {
        // 已存在费用操作，需要累加
        // 解析现有的费用信息
        LlmSessionUsage existingUsage = LlmSessionUsage::ParseFromCostText(existingCostOp->content);
        
        // 累加费用
        LlmSessionUsage totalUsage;
        totalUsage.Accumulate(existingUsage);
        
        // 格式化新的显示文本
        newDisplayText = totalUsage.FormatToCostText();

        // 更新现有操作的内容
        existingCostOp->content = newDisplayText;
        _ver++; // 操作记录已更改，增加版本号

        _SetSessionCostDisplay(newDisplayText);
    }
}


void CChatCtrl::_AddSessionTag(const std::wstring& text, const std::wstring& path, bool enabled)
{
	ChatCtrlOp op(enabled? ChatCtrlOp::Op_AddSessionTag : ChatCtrlOp::Op_AddSessionDisabledTag);
	op.content = text;
	op.fullPath = path;
	_AddOp(op);
}


void CChatCtrl::AddSessionTag(const ChatInputTag& tag)
{
	if (!_IsReady())
		return;

	// 记录操作
	_AddSessionTag(tag.text, tag.path, tag.visible);
}


//====================== 操作记录文件保存和加载功能实现 ======================


// 保存操作记录到文件
/***重要: 修改存储格式, 要同步CChatHistory::_Entry2MenuItemInfo()****/
bool CChatCtrl::Save(const char* filePath)
{
    if (!filePath) return false;
    
    try
    {
        std::vector<BYTE> buffer;
        
        // 使用DP_BeginSave/DP_EndSave宏来处理DataPacket
        DP_BeginSave(dp, buffer)
        {
            // 写入文件头标识
            DWORD magic = 0x4F504348; // "HCPO" (HChat Ops)
            dp.Data_WriteSimple(magic);
                
            // 写入版本号
            DWORD version = CHATCTRL_OPERATIONS_VERSION_CURRENT;
            dp.Data_WriteSimple(version);

            dp.Data_WriteSimple(_undoCheckpointId);
			dp.Data_WriteSimple(_restoredCheckpointId);
			dp.Data_WriteSimple(_recentPrompToken);

            // 写入操作记录数量
            DWORD opCount = static_cast<DWORD>(_ops.size());
            dp.Data_WriteSimple(opCount);

            // 写入每个操作记录
            for (const auto& op : _ops)
            {
                const_cast<ChatCtrlOp&>(op).Save(dp);
            }
        }
        DP_EndSave()
        
        // 使用STL的ofstream写入文件
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;
            
        if (!buffer.empty())
        {
            file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        }
        file.close();
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}

// 从文件加载操作记录
bool CChatCtrl::Load(const char* filePath)
{
    if (!filePath)
        return false;
        
    try
    {
        // 使用STL的ifstream读取文件
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;
            
        // 读取文件大小
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        if (fileSize == 0)
        {
            file.close();
            return false;
        }
        
        // 读取文件内容到缓冲区
        std::vector<BYTE> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();
        
        // 创建DataPacket并设置数据缓冲区
        CDataPacket dp;
        dp.SetDataBufferPointer(buffer.data());
        
        // 读取文件头标识
        DWORD magic = dp.Data_ReadSimple<DWORD>();
        if (magic != 0x4F504348) // "HCPO"
        {
            return false; // 不是有效的操作记录文件
        }
            
        // 读取版本号
        DWORD version = dp.Data_ReadSimple<DWORD>();
            
        // 检查版本兼容性
        if (version > CHATCTRL_OPERATIONS_VERSION_CURRENT)
        {
            return false; // 版本过新，无法加载
        }

        // 显示加载遮罩层
        ShowLoadingOverlay();

        // 清空当前内容
        ClearChat(); 

        // 读取 undo checkpoint
        _undoCheckpointId = dp.Data_ReadSimple<FilesCheckpointUID>();
		if (version >= CHATCTRL_OPERATIONS_VERSION_1_1)
			_restoredCheckpointId = dp.Data_ReadSimple<FilesCheckpointUID>();
		if (version >= CHATCTRL_OPERATIONS_VERSION_1_2)
			_recentPrompToken = dp.Data_ReadSimple<int>();

        // 读取操作记录数量
        DWORD opCount = dp.Data_ReadSimple<DWORD>();
            
        // 读取每个操作记录
        for (DWORD i = 0; i < opCount; ++i)
        {
            ChatCtrlOp op;
            op.Load(dp,version);
            _ExecuteOp(op); // 注意：这里不直接调用AddUserMessage等，而是调用_ExecuteOp
        }

        // 隐藏加载遮罩层
        HideLoadingOverlay();
          
        return true;
    }
    catch (...)
    {
        // 加载失败时隐藏加载遮罩层
        HideLoadingOverlay();
        return false;
    }
}



//====================== Loading Overlay 功能实现 ======================

// 显示加载遮罩层
void CChatCtrl::ShowLoadingOverlay()
{
    if (!_IsReady())
        return;
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"showLoadingOverlay\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);
}

// 隐藏加载遮罩层
void CChatCtrl::HideLoadingOverlay()
{
    if (!_IsReady())
        return;
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"hideLoadingOverlay\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);
}

//====================== FileEdit Progress Label 功能实现 ======================

// 显示文件编辑进行中的标签
void CChatCtrl::ShowFileEditProgressLabel(const std::wstring& messageId, const std::wstring& fileName)
{
    if (!_IsReady())
        return;
    
    // 转义消息ID和文件名
    std::wstring safeMessageId = EscapeJsonString(messageId);
    std::wstring safeFileName = EscapeJsonString(fileName);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"showFileEditProgressLabel\",\"messageId\":\"" + safeMessageId + L"\",\"fileName\":\"" + safeFileName + L"\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);
}

// 隐藏文件编辑进行中的标签
void CChatCtrl::HideFileEditProgressLabel(const std::wstring& messageId)
{
    if (!_IsReady())
        return;
    
    // 转义消息ID
    std::wstring safeMessageId = EscapeJsonString(messageId);
    
    // 构造JSON消息
    std::wstring jsonMessage = L"{\"action\":\"hideFileEditProgressLabel\",\"messageId\":\"" + safeMessageId + L"\"}";
    
    // 发送到WebView
    PostWebMessageAsJson(jsonMessage);
}

int CChatCtrl::_FindLastOpIndex(ChatCtrlOp::Type tp) const
{
    for (int i = _ops.size() - 1; i >= 0; i--)
    {
        if (_ops[i].type == tp)
            return i;
    }
    return -1;
}

ChatCtrlOp* CChatCtrl::_GetLastOp()
{
	if (_ops.size() <= 0)
		return nullptr;
	return &_ops[_ops.size() - 1];
}


// 从操作队列中查找最后一个指定类型的操作
const ChatCtrlOp* CChatCtrl::_FindLastOp(ChatCtrlOp::Type tp) const
{
    int index = _FindLastOpIndex(tp);
    if (index == -1)
        return nullptr;
    return &_ops[index];
}


int CChatCtrl::_FindFileEditOpIndex(const std::wstring& fileEditId) const
{
    for (int i = _ops.size() - 1; i >= 0; i--)
    {
        if (_ops[i].type == ChatCtrlOp::Op_AddFileEditToAIMessage && _ops[i].fileEditId == fileEditId)
            return i;
    }
    return -1;
}

int CChatCtrl::_GetSessionBeginOfOpIndex(int idx) const
{
	if (idx < 0)
		return -1;

	for (int i = idx; i >= 0; i--)
	{
		if (_ops[i].type == ChatCtrlOp::Op_BeginSession)
			return i;
	}
	return -1;
}


int CChatCtrl::_GetSessionBeginOfFileEdit(const std::wstring& fileEditId) const
{
    int fileEditOpIndex = _FindFileEditOpIndex(fileEditId);
	return _GetSessionBeginOfOpIndex(fileEditOpIndex);
}

int CChatCtrl::_GetSessionBeginOfUserMessage(const std::wstring& messageId) const
{
    // 首先找到用户消息的索引
    int userMessageIndex = -1;
    for (int i = 0; i < static_cast<int>(_ops.size()); i++)
    {
        if (_ops[i].type == ChatCtrlOp::Op_AddUserMessage && _ops[i].messageId == messageId)
        {
            userMessageIndex = i;
            break;
        }
    }

	return _GetSessionBeginOfOpIndex(userMessageIndex);
}

int CChatCtrl::_GetLastNotDisabledSessionBegin() const
{
    int disableAfter = _GetDisableAfterIndex();
    if (disableAfter >= _ops.size())//没有disable
        return _FindLastOpIndex(ChatCtrlOp::Op_BeginSession);

	//disableAfter往前找到第二个BeginSession,第一个BeginSession是被disable的那个session的SessionBegin
    int sessionBeginCount = 0;
	for (int i = disableAfter - 1; i >= 0; i--)
	{
		if (_ops[i].type == ChatCtrlOp::Op_BeginSession)
		{
            sessionBeginCount++;
            if (sessionBeginCount > 1)
                return i;
		}
	}

    return -1;
}

int CChatCtrl::_GetLastNotDisabledSessionEnd() const
{
    int sessionBegin = _GetLastNotDisabledSessionBegin();
    if (sessionBegin < 0)
        return sessionBegin;
    return _FindFirstOpIndexInSession(sessionBegin, ChatCtrlOp::Op_EndSession);
}

// 查找 DisableMessagesAfter 操作的位置，返回被 disable-after 的消息在操作队列中的索引
// 如果没有找到 DisableMessagesAfter 操作，返回操作队列大小（表示没有被 disable 的消息）
int CChatCtrl::_GetDisableAfterIndex() const
{
    // 查找最新的 DisableMessagesAfter 操作
    const ChatCtrlOp* disableOp = _FindLastOp(ChatCtrlOp::Op_DisableMessagesAfter);
    
    // 如果没有找到 DisableMessagesAfter 操作，返回操作队列大小（表示没有被 disable 的消息）
    if (disableOp == nullptr)
        return static_cast<int>(_ops.size());
    
    // 找到被 disable 的消息在操作队列中的位置
    const std::wstring& disableAfterMessageId = disableOp->messageId;
    for (int i = 0; i < static_cast<int>(_ops.size()); i++)
    {
        if ((_ops[i].type == ChatCtrlOp::Op_AddUserMessage || 
             _ops[i].type == ChatCtrlOp::Op_StartStreamingAIMessage ||
             _ops[i].type == ChatCtrlOp::Op_AddSystemMessage) && 
            _ops[i].messageId == disableAfterMessageId)
        {
            return i;
        }
    }
    
    // 如果没有找到被 disable 的消息位置，返回操作队列大小
    return static_cast<int>(_ops.size());
}

//得到要恢复到user messageId之前的状态,需要restore哪些checkpoints,按照自后到前的顺序排列
bool CChatCtrl::GetRestoreCheckpoints(const std::wstring& userMessageId, std::vector<FilesCheckpointUID>& checkpointIds)
{
    // 清空输出参数
    checkpointIds.clear();

	int sessionBegin = _GetSessionBeginOfUserMessage(userMessageId);
	if (sessionBegin < 0)
		return false;
    
    // 获取 disable 边界
    int disableAfterIndex = _GetDisableAfterIndex();

	for (int i = disableAfterIndex - 1; i >= sessionBegin; i--)
	{
		ChatCtrlOp& op = _ops[i];
        if ((op.type == ChatCtrlOp::Op_SetFileEditContent)|| (op.type == ChatCtrlOp::Op_BeginSession))
        {
			if (op.checkpointId != FilesCheckpointUID_Invalid)
				checkpointIds.push_back(op.checkpointId);
        }
	}

    return !checkpointIds.empty();
}

bool CChatCtrl::UpdateFileEditDiffContent(const std::wstring& fileEditId, const std::wstring& diffContent, FilesCheckpointUID checkpointId)
{
    if (!_IsReady())
        return false;

    // 查找最后一个相关的 SetFileEditContent 操作
    ChatCtrlOp* lastSetContentOp = nullptr;
    for (auto it = _ops.rbegin(); it != _ops.rend(); ++it)
    {
        if (it->type == ChatCtrlOp::Op_SetFileEditContent && it->fileEditId == fileEditId)
        {
            lastSetContentOp = &(*it);
            break;
        }
    }

    bool hasModification = false;

    // 检查并更新操作记录中的内容
    if (lastSetContentOp != nullptr)
    {
        if (lastSetContentOp->diffContent != diffContent || lastSetContentOp->checkpointId != checkpointId)
        {
            lastSetContentOp->diffContent= diffContent;
            lastSetContentOp->checkpointId = checkpointId;
            hasModification = true;
        }
    }

    // 查找并更新内部存储的FileEdit窗口
    ChatCtrlFileEdit* window = _FindFileEdit(fileEditId);
    if (window != nullptr)
    {
        if (window->diffContent!= diffContent)
        {
            window->diffContent = diffContent;
            hasModification = true;
        }
        
        // 更新WebView中的显示
        _SendFileEditMsg(L"updateFileEditContent", *window);
    }

    // 如果有修改，增加操作记录版本号
    if (hasModification)
    {
        _ver++;
    }

    return hasModification;
}

bool CChatCtrl::SetFileEditHeadCheckpoint(const std::wstring& fileEditId, FilesCheckpointUID checkpointId)
{
	int sessionBeginIdx = _GetSessionBeginOfFileEdit(fileEditId);
	if (sessionBeginIdx < 0)
		return false;
	if (_ops[sessionBeginIdx].checkpointId != checkpointId)
	{
		_ops[sessionBeginIdx].checkpointId = checkpointId;
		_ver++;
	}
	return true;
}


bool CChatCtrl::GetFileEditCheckpoint(const std::wstring& fileEditId, FilesCheckpointUID& fileEditCheckpointId) const
{
    fileEditCheckpointId = FilesCheckpointUID_Invalid;

    // 查找该FileEdit的最新内容更新操作
    auto contentIt = std::find_if(_ops.rbegin(), _ops.rend(), [&](const ChatCtrlOp& op) {
        return op.type == ChatCtrlOp::Op_SetFileEditContent && op.fileEditId == fileEditId;
    });

	if (contentIt != _ops.rend())
	{
		fileEditCheckpointId = contentIt->checkpointId;
		return true;
	}

	return false;
}

bool CChatCtrl::GetFileEditCheckpointInSessionBegin(const std::wstring& fileEditId, FilesCheckpointUID& checkpointId)const
{
	checkpointId = FilesCheckpointUID_Invalid;

	int fileEditIndex = _FindFileEditOpIndex(fileEditId);
	if (fileEditIndex < 0)
		return false;

	for (int i = fileEditIndex - 1;i >= 0;i--)
	{
		if (_ops[i].type == ChatCtrlOp::Op_BeginSession)
		{
			checkpointId = _ops[i].checkpointId;
			return true;
		}
	}

	return false;
}


//得到FileEdit的checkpoint之前的那个checkpoint(这次修改之前的文件状态)
//isHead为true, 表示这个checkpoint是在这个session的Op_BeginSession 里
bool CChatCtrl::GetFileEditPrevCheckpointInSession(const std::wstring& fileEditId, FilesCheckpointUID& checkpointId,bool &isHead)const
{
	checkpointId = FilesCheckpointUID_Invalid;
	isHead = false;

	int fileEditIndex = _FindFileEditOpIndex(fileEditId);
	if (fileEditIndex < 0)
		return false;

	for (int i = fileEditIndex-1;i >= 0;i--)
	{
		if (_ops[i].type == ChatCtrlOp::Op_SetFileEditContent)
		{
			std::wstring fullPath;
			if (GetFileEditFullPath(_ops[i].fileEditId, fullPath))
			{
				if (fullPath == _ops[fileEditIndex].fullPath)
				{
					if (_ops[i].checkpointId != FilesCheckpointUID_Invalid)
					{
						checkpointId = _ops[i].checkpointId;
						isHead = false;
						return true;
					}
				}
			}
		}
		if (_ops[i].type == ChatCtrlOp::Op_BeginSession)
		{
			checkpointId = _ops[i].checkpointId;
			isHead = true;
			return true;
		}
	}

	return false;
}

bool CChatCtrl::IsFileEditInLastNotDisabledSession(const std::wstring& fileEditId) const
{
	int sessionBegin = _GetLastNotDisabledSessionBegin();
	if (sessionBegin < 0)
		return false;

	int fileEditSessionBegin = _GetSessionBeginOfFileEdit(fileEditId);
	if (fileEditSessionBegin < 0)
		return false;
	return (sessionBegin == fileEditSessionBegin);
}

void CChatCtrl::GetFileEditFilePathesByMessageId(const std::wstring& messageId, std::vector<std::wstring>& filePathes) const
{
	filePathes.clear();

	// 使用 set 来剔除重复路径
	std::unordered_set<std::wstring> uniquePaths;

	// 遍历 session 范围内的所有操作
	for (int i = 0; i < static_cast<int>(_ops.size()) ; i++)
	{
		const ChatCtrlOp& op = _ops[i];

		if(op.messageId!=messageId)
			continue;

		// 只处理有有效 checkpoint 的 SetFileEditContent 操作
		if (op.type == ChatCtrlOp::Op_AddFileEditToAIMessage)
		{
			FilesCheckpointUID checkpointId;
			if (GetFileEditCheckpoint(op.fileEditId,checkpointId))
			{
				if (checkpointId != FilesCheckpointUID_Invalid)
				{
					// 获取该 fileEditId 对应的文件路径
					std::wstring fullPath;
					if (GetFileEditFullPath(op.fileEditId, fullPath))
					{
						// 使用 set 去重
						if (uniquePaths.insert(fullPath).second)
						{
							filePathes.push_back(fullPath);
						}
					}
				}
			}
		}
	}
}

std::wstring CChatCtrl::GetLastFileEditCheckpointFromFilePath(const std::wstring& messageId, const std::wstring& fullPath) const
{
	// 从后向前遍历所有操作
	for (int i = static_cast<int>(_ops.size()) - 1; i >= 0; i--)
	{
		const ChatCtrlOp& op = _ops[i];

		// 检查messageId是否匹配
		if (op.messageId != messageId)
			continue;

		// 只处理Op_AddFileEditToAIMessage操作
		if (op.type != ChatCtrlOp::Op_AddFileEditToAIMessage)
			continue;

		// 检查路径是否匹配（不区分大小写）
		if (_wcsicmp(op.fullPath.c_str(), fullPath.c_str()) != 0)
			continue;

		// 获取这个fileEdit的checkpoint
		FilesCheckpointUID checkpointId;
		if (GetFileEditCheckpoint(op.fileEditId, checkpointId))
		{
			// 如果checkpoint有效，返回fileEditId
			if (checkpointId != FilesCheckpointUID_Invalid)
			{
				return op.fileEditId;
			}
		}
	}

	// 没有找到符合条件的fileEdit，返回空字符串
	return L"";
}

bool CChatCtrl::GetFileEditFullPath(const std::wstring& fileEditId, std::wstring& fullPath) const
{
    fullPath.clear();
    
    // 查找对应的FileEdit创建操作
    int fileEditIndex = const_cast<CChatCtrl*>(this)->_FindFileEditOpIndex(fileEditId);

    if (fileEditIndex != -1)
    {
        fullPath = _ops[fileEditIndex].fullPath;
        return !fullPath.empty();
    }
    
    return false;
}

bool CChatCtrl::GetFileEditContent(const std::wstring& fileEditId, std::wstring& content) const
{
    content.clear();
    
    // 查找最新的FileEdit内容设置操作
    auto contentIt = std::find_if(_ops.rbegin(), _ops.rend(), [&](const ChatCtrlOp& op) {
        return op.type == ChatCtrlOp::Op_SetFileEditContent && op.fileEditId == fileEditId;
    });

    if (contentIt != _ops.rend())
    {
        content = contentIt->content;
        return !content.empty();
    }
    
    return false;
}

void CChatCtrl::GetNotDisabledFileEditsStartingFrom(const std::wstring& fileEditId, std::vector<std::wstring>& fileEditIds) const
{
    fileEditIds.clear();
    
    // 查找指定fileEditId在操作队列中的位置
    int startIndex = const_cast<CChatCtrl*>(this)->_FindFileEditOpIndex(fileEditId);
    
    // 如果没有找到指定的fileEdit，直接返回
    if (startIndex == -1)
        return;
    
    // 获取disable边界索引
    int disableAfterIndex = const_cast<CChatCtrl*>(this)->_GetDisableAfterIndex();
    
    // 从指定fileEdit的位置开始，向后遍历所有操作
    for (int i = startIndex; i < static_cast<int>(_ops.size()) && i < disableAfterIndex; i++)
    {
        const ChatCtrlOp& op = _ops[i];
        
        // 如果是FileEdit添加操作
        if (op.type == ChatCtrlOp::Op_AddFileEditToAIMessage)
        {
			if (op.fullPath != _ops[startIndex].fullPath)
				continue;
            // 检查是否已经存在，避免重复添加
            bool exists = false;
            for (const auto& existingId : fileEditIds)
            {
                if (existingId == op.fileEditId)
                {
                    exists = true;
                    break;
                }
            }
            
            if (!exists)
            {
                fileEditIds.push_back(op.fileEditId);
            }
        }
    }
}

int CChatCtrl::_FindFirstOpIndexInSession(int sessionBeginIdx, ChatCtrlOp::Type tp) const
{
    for (int i = sessionBeginIdx; i < static_cast<int>(_ops.size()); i++)
    {
        if (_ops[i].type == tp)
            return i;
    }
    return -1;
}

// 检查WebView和Chat是否已初始化
bool CChatCtrl::_IsReady() const
{
    return _isWebViewCreated && _isChatInitialized;
}

void CChatCtrl::_CollectSessionTags(int sessionBeginIndex, int sessionEndIndex, std::vector<ChatInputTag>& tags) const
{
    for (int i = sessionBeginIndex; i < sessionEndIndex; i++)
    {
        if (_ops[i].type == ChatCtrlOp::Op_AddSessionTag || _ops[i].type == ChatCtrlOp::Op_AddSessionDisabledTag)
        {
            ChatInputTag tag;
            tag.path = _ops[i].fullPath;
            tag.text = _ops[i].content;
            if (!Utils::IsImageFile(widechar_to_utf8(tag.path.c_str()).c_str()))
                tag.type = L"file";
            else
                tag.type = L"image";
            tag.visible = (_ops[i].type == ChatCtrlOp::Op_AddSessionTag);
            tag.removable = true;
            tags.push_back(tag);
        }
    }
}

void CChatCtrl::GetLastNotDisabledSessionTags(std::vector< ChatInputTag>& tags) const
{
	tags.clear();
	int sessionBeginIndex = _GetLastNotDisabledSessionBegin();
    int sessionEndIndex=_GetLastNotDisabledSessionEnd();

    _CollectSessionTags(sessionBeginIndex, sessionEndIndex, tags);
}

void CChatCtrl::GetUserMessageSessionTags(const std::wstring& userMessageId, std::vector< ChatInputTag>& tags) const
{
	tags.clear();
	
	// 获取用户消息所在的session开始索引
	int sessionBeginIndex = _GetSessionBeginOfUserMessage(userMessageId);
	if (sessionBeginIndex < 0)
	{
		return; // 未找到用户消息
	}
	
	// 找到session的结束索引（下一个BeginSession或数组末尾）
	int sessionEndIndex = static_cast<int>(_ops.size());
	for (int i = sessionBeginIndex + 1; i < static_cast<int>(_ops.size()); i++)
	{
		if (_ops[i].type == ChatCtrlOp::Op_BeginSession)
		{
			sessionEndIndex = i;
			break;
		}
	}
	
	// 收集session中的所有tags
	_CollectSessionTags(sessionBeginIndex, sessionEndIndex, tags);
}

int CChatCtrl::AddFileAttaches(const std::wstring& fileList, FilesCheckpointUID checkpointId)
{
	ChatCtrlOp op(ChatCtrlOp::Op_FileAttaches);
	op.content = fileList;
	op.checkpointId = checkpointId;
	_AddOp(op);
	return _ops.size() - 1;
}

void CChatCtrl::AddToolCallResult(const std::wstring& jsonString)
{
	ChatCtrlOp op(ChatCtrlOp::Op_AddToolCallResult);
	op.content = jsonString;
	_AddOp(op);
}

void CChatCtrl::AddToolCallMessage(const std::wstring& messageId, const std::wstring& message)
{
	if (!_IsReady())
		return;
	
	// 转义消息内容以防XSS
	std::wstring safeMessage = EscapeJsonString(message);
	
	// 构造JSON消息，作为AI消息的一部分添加
	std::wstring jsonMessage = L"{\"action\":\"addToolCallMessageToAIMessage\",\"content\":\"" + safeMessage + L"\",\"id\":\"" + messageId + L"\"}";
	
	// 发送消息到WebView
	PostWebMessageAsJson(jsonMessage);

	// 记录操作
	ChatCtrlOp op(ChatCtrlOp::Op_AddToolCallMessage);
	op.content = message;
	op.messageId = messageId;
	_AddOp(op);
}

void CChatCtrl::AddReplaceInFileResult(const std::wstring& filePath, const std::wstring& result)
{
	ChatCtrlOp op(ChatCtrlOp::Op_AddReplaceInFileResult);
	op.content = result;
	op.fullPath = filePath;
	_AddOp(op);
}

void CChatCtrl::_GetFileAttachesList(int fileAttaches, std::vector<std::wstring>& filePathes)
{
	filePathes.clear();
	
	// 检查索引有效性
	if (fileAttaches < 0 || fileAttaches >= static_cast<int>(_ops.size()))
		return;
		
	// 检查操作类型
	const ChatCtrlOp& op = _ops[fileAttaches];
	if (op.type != ChatCtrlOp::Op_FileAttaches)
		return;
		
	// 从content字段中解析文件路径列表（以"|"分隔）
	if (!op.content.empty())
		SplitStringBy(L"|", op.content, &filePathes);
}

void CChatCtrl::_GetFileAttachesList(int fileAttaches, std::unordered_set<std::wstring>& filePathes)
{
	filePathes.clear();
    std::vector<std::wstring> filePathesList;
    _GetFileAttachesList(fileAttaches, filePathesList);
    for (const auto& path : filePathesList)
    {
        filePathes.insert(path);
    }
}


bool CChatCtrl::CheckValidFileAttachesCache(int fileAttaches,const std::vector<ChatInputTag>& visibleFileTags)
{
    // 收集所有可见Tags的文件路径
    std::vector<std::wstring> targetPaths;
    for (const auto& tag : visibleFileTags)
    {
        if (!tag.path.empty())
        {
            targetPaths.push_back(tag.path);
        }
    }

    // 获取disable边界，只考虑未被disabled的操作
    int disableAfterIndex = _GetDisableAfterIndex();

	if (fileAttaches >= disableAfterIndex)
		return false;
	if (fileAttaches < 0)
		return false;
    
    // 寻找最后一个Op_FileAttaches
    int lastFileAttachesIndex = -1;
    for (int i = disableAfterIndex - 1; i >= 0; i--)
    {
        if (_ops[i].type == ChatCtrlOp::Op_FileAttaches)
        {
            lastFileAttachesIndex = i;
            break;
        }
    }
    
    // 不是最后一个
    if (lastFileAttachesIndex != fileAttaches)
        return false;
    
    // 检查attachPaths是否能覆盖所有的targetPaths
    std::vector<std::wstring> attachPaths;
    _GetFileAttachesList(fileAttaches, attachPaths);
    for (const auto& targetPath : targetPaths)
    {
        bool found = false;
        for (const auto& attachPath : attachPaths)
        {
            if (attachPath == targetPath)
            {
                found = true;
                break;
            }
        }
        
        if (!found)
        {
            return false;
        }
    }

	for (int i = 0;i < attachPaths.size();i++)
	{
		AbsTick t;
		if (!_GetLastFileTimeInCheckpoint(attachPaths[i], fileAttaches, disableAfterIndex, t))
			continue;//如果这个文件不在之前任何checkpoint里,说明这个文件还没有被修改过

		if (t != Utils::GetFileTick(widechar_to_utf8(attachPaths[i].c_str()).c_str()))
			return false;
	}
    
    return true;
}


bool CChatCtrl::MakeSessionRequest(LlmSessionRequest& request, int fileAttaches)
{
// 	return MakeSessionRequest_Debug(request);
	if (!_IsReady())
		return false;

	// 清空 request 的消息
	request.Clear();

	// 获取 disable 边界，只处理未被 disabled 的操作
	int disableAfterIndex = _GetDisableAfterIndex();

	std::unordered_set<std::wstring> attachFilePathes;
	_GetFileAttachesList(fileAttaches, attachFilePathes);

	// 从找到的 Session 开始，按顺序处理操作 
	for (int i = 0; i < static_cast<int>(_ops.size()) && i < disableAfterIndex; i++)
	{
		const ChatCtrlOp& op = _ops[i];

		switch (op.type)
		{
		case ChatCtrlOp::Op_FileAttaches:
		{
			if (i != fileAttaches) 
				continue;

			extern CCheckpoints* GetCheckpoints();
			CCheckpoints* pCheckpoints = GetCheckpoints();

			for (const auto& filePathW : attachFilePathes)
			{
                std::string filePath = widechar_to_utf8(filePathW.c_str());//文件路径

				// 检查是否为图片文件
				if (Utils::IsImageFile(filePath.c_str()))
				{
					// 读取图片为 base64
					std::string base64Data;
					if (Utils::GetFileContentIntoBase64(filePath.c_str(), base64Data))
					{
						// 添加文件说明文字
						std::string message = u8"Here is the current content of image\"";
						message += filePath;
						message += u8"\":\n";
						request.AddUserMessage(message.c_str());

						// 根据 extension 确定 mimeType
						std::string mimeType = "image/jpeg";
						std::string ext = GetFileSuffix(filePath);
						StringLower(ext);
						if (ext == "png") mimeType = "image/png";
						else if (ext == "gif") mimeType = "image/gif";
						else if (ext == "webp") mimeType = "image/webp";
						else if (ext == "bmp") mimeType = "image/bmp";
						else if (ext == "tiff" || ext == "tif") mimeType = "image/tiff";
						else if (ext == "svg") mimeType = "image/svg+xml";
						
						request.AddUserMessageOfImage(base64Data.c_str(), mimeType.c_str());
					}
					continue;
				}

				if (Utils::CheckFileBinary(filePath.c_str()))
					continue;
                std::vector<BYTE> fileContent;
				Utils::FileContentCodingFormat codingFmt;
				if (pCheckpoints != nullptr && op.checkpointId != FilesCheckpointUID_Invalid)
                {
                    std::vector<BYTE> rawContent;
                    if (pCheckpoints->GetCheckpointFileContent(op.checkpointId, filePath.c_str(), rawContent))
                        Utils::ConvertFileContentIntoUTF8(rawContent, fileContent, codingFmt);
                }
                if(fileContent.empty())
                    Utils::GetFileContentIntoUTF8(filePath.c_str(), fileContent, codingFmt);
				if (!fileContent.empty())
				{
					// 构建系统消息内容
					std::string message = u8"Here is the current content of file\"";
					message += filePath;
					message += u8"\":\n";

					// 将文件内容转换为字符串（假设是文本文件）
					{
						std::string fileContentStr(fileContent.begin(), fileContent.end());
						message += fileContentStr;
					}
					message += u8"\n";

					// 添加到 request
//					request.AddSystemMessage(message.c_str());
					request.AddUserMessage(message.c_str());
				}
			}
			break;
		}

		case ChatCtrlOp::Op_AddUserMessage:
		{
			// 将 wstring 转换为 UTF-8 string
			std::string userMessage = widechar_to_utf8(ExtractPlainText(op.content.c_str()).c_str());

			// 如果用户消息为空或全是空白字符，则跳过
			if (userMessage.find_first_not_of(" \t\n\r") == std::string::npos)
				break;

			request.AddUserMessage(userMessage.c_str());
			break;
		}

		case ChatCtrlOp::Op_AddStreamingAIMessage:
		{
			std::string aiMessage = widechar_to_utf8(_ops[i].content.c_str());

   			// 如果ai消息为空或全是空白字符，则跳过
			if (aiMessage.find_first_not_of(" \t\n\r") == std::string::npos)
				break;

			// 如果有内容，添加为 assistant 消息
			if (!aiMessage.empty())
			{
				request.AddAssistMessage(aiMessage.c_str());
			}
			break;
		}

		case ChatCtrlOp::Op_AddStreamingAIMessage_Thinking:
		{
			std::string reasoning = widechar_to_utf8(_ops[i].content.c_str());

			// 如果ai消息为空或全是空白字符，则跳过
			if (reasoning.find_first_not_of(" \t\n\r") == std::string::npos)
				break;

			if (!reasoning.empty())
			{
				request.AddReasoningMessage(reasoning.c_str());
			}
			break;
		}

		case ChatCtrlOp::Op_AddToolCallResult:
		{
			std::string content = widechar_to_utf8(op.content.c_str());
			if(false)
			if (i < fileAttaches)
			{
				break;
				std::string filePath;
				if (g_llmTools.GetFilePathFromToolCallResultString(content.c_str(), filePath))
				{
					if (attachFilePathes.find(local_to_widechar(filePath.c_str())) != attachFilePathes.end())
						g_llmTools.OmitFileContentInToolCallResultString(content);//这个文件内容被之后的attach文件内容所覆盖,我们省略掉tool call里的那些文件修改内容
				}
			}
 			request.AddToolCallResult(content.c_str());
			break;
		}

		case ChatCtrlOp::Op_AddReplaceInFileResult:
		{
			if (i < fileAttaches)
			{
				if (attachFilePathes.find(op.fullPath) != attachFilePathes.end())
					continue;//这个文件内容被之后的attach文件内容所覆盖,没有必要发送
			}
			request.AddAssistMessage(widechar_to_utf8(_ops[i].content.c_str()).c_str());
			request.AddUserMessage("Successfully made the replacement!");
			break;
		}

		default:
			// 其他操作类型忽略
			break;
		}
	}

	extern bool IsPrompCachingEnabled();
	if (IsPrompCachingEnabled())
		request.AddCacheControl();

	return true;
}


bool CChatCtrl::MakeSessionRequest_Debug(LlmSessionRequest& request)
{
	if (!_IsReady())
		return false;

	// 清空 request 的消息
	request.Clear();

	std::string fileContent = "		Berver_FFFDASNPCMovementComponent		* .记录一个BP_MovementDataContainer: myMovementContainer		* .记录当前的MoveToken : myCurrentMovement		* .记录一系列事先规划好但未开始的移动(myPendingMovements, MovementToken的数组), 所谓多段移动		* .myMovements记录了最多3个最近被丢弃的Move的Token, 一个Move被丢弃后, 在被从container中丢弃前, 仍然会存在一段时间,		* .myTokensNeedNetworking记录等待被同步到client的移动		* .BP_MovementDataContainer : 记录(最近一段时间内的以及未开始的)所有的Move数据		* .BP_MovementData 的数组, 用MovementToken索引		* .BP_MovementToken : 一个ID, 代表一次Move		* .BP_MovementDataAccessor : 对于BP_MovementDataContainer中某一个Move的访问器		* .包含一个BP_MovementDataContainer和一个BP_MovementToken		* .BP_MovementRequest : 代表一次移动的请求, 一次移动可能会请求多段移动(A-- > B, B-- > C, C--D), 每一段对应一个MovementToken, 一次请求包含所有的这些移动的token, 如果不支持多段移动, 其实不需要这个类, 可以用Movement Token代替		* .一段移动的token只有在开始以后才会被加到Request中去		* .BP_MovementData : 一次Move的所有数据		* .BServer_NPCAgentController::GenerateMovement()在 BP_MovementDataContainer 中创建一个 BP_MovementData		* .包含一个 BP_Input		* .包含一个 BP_ReconciliationData(路径信息)		* .包含一个 BP_Plan(移动规划信息, 由多个Fragment构成)		* .BP_ValuationData : 移动过程中的瞬间状态(位置, 朝向, 速度, ..)		* .BP_Input : 一次移动的参数		* .类型有 : Movement, CustomAnimation, Turn, Stop, GoToIdle, FaceTarget		* .BP_ControlPoint : 代表一个状态的ID(和位置没关系)		* .一些事先定义好的状态, 比如Idle, Walk, Turn, Jump, CustomAnimation		* .这些状态定义在一个default_control_graph.bpcontrolgraph 文件内		* .每个BP_ControlPoint作为节点定义在这个graph内		* .ControlPoint之间通过Transition节点连接, 代表这些状态之间可以互相切换		* .Transition会连上很多各种类型Contraint节点, 代表了各种条件		* .当一个Transition上的条件都满足后，就会发生一次状态切换		* .比如一个Idle的ControlPoint, 可以Transition到Walk的ControlPoint, Transition条件之一可以是当Agent目前有一条准备好的路径可以走时, 这个条件满足后, 就会发生一次Transition		* .所有Agent目前都使用default_control_graph		* .BP_ExecutePoint: 一个ID, 可以认为代表一个动画信息的Entry		* .BP_ConstraintStateMachine, 一个graph, 包括 BP_ControlPoint 和一系列 constraint 节点, transition 节点,		* .就是 default_control_graph.bpcontrolgraph		* .default_control_graph是一个graph文件的名字, 所有Agent的 RServer_NPCConfig::myBehaviorPlannerControlGraphFile 都设成这个名字		* .本身不是一个状态机, 因为它内部并没有状态, 只是一个graph		* .这个graph里主要包含以下 重要的 类		* .各种 BP_Node 派生的类, BP_WalkNode, BP_IdleNode, 等等		* .每一种 BP_Node 在一个graph 只有一个, 和 一个 BP_ControlPoint 一对一绑定		* .BP_TransitionNode, 代表一个transition, 从一个 BP_Node 到 另一个 BP_Node		* .只是一个编辑用的类, 用来在graph 里 连接其它的 node, 不参与实际运行		* .有一个 in, 一个 out, 以及若干个 contraint的 pin		* .BP_Edge, 实际的用于 transition 检测的 运行类		* .会读取 BP_TransitionNode 以及它在 graph里的连接, 来构建一个 BP_Edge, 见(BP_ControlGraphInterpreter::Interpret(..))		* .一个 BP_Node 对于每一个 它能 连接到的 BP_Node 都维护一个 BP_Edge, BP_Edge 内部 记录了 那个目标 BP_Node		* .记录了 一个 BP_IConstraint 的数组, 用来判断条件是否满足(是否可以transition), 见 BP_Edge::Evaluate(..)		* .对于多个 BP_IConstraint, 有两种满足条件, AND 模式和 OR 模式		* .各种 BP_IConstraint 派生的类, 比如 BP_WaypointTypeConstraint, BP_HaveSpaceToMoveConstraint, BP_NoneConstraint, ...		*.BP_IConstraint 根据一个context(BP_IConstraint::EvaluationContext) 来了解 当前的 外部状态(比如说当前 进展到哪个waypoint)		* .BP_BehaviorPlannerModule, 一个singleton, 封装了一个 BP_ConstraintStateMachine 实例, 以及各种用到的 BP_ControlPoint, BP_ExecutionPoint 实例		* .所以全局只有一个 BP_ConstraintStateMachine		* .BP_ConstraintStateMachineState, 真正的状态机, 包含了 BP_ConstraintStateMachine, 以及一个当前的 BP_ControlPoint* 作为状态,		* .使用 BP_ConstraintStateMachine 这个graph(其实就是各种预先设好的切换条件),		* .这个状态机专门用来生成 BP_Plan, 并不是在单位移动时切换		* .在生成 BP_Plan 时, 会沿着 BP_WorkingSet 进行虚拟的移动, 从而触发 这个状态机的 状态变化, 根据这些状态的变化, 来生成 Plan, 产生各种各样的 command 加到 BP_Plan 中去		* .每个 BP_PlanGenerator 都会在内部维护一个 BP_ConstraintStateMachineState		* .RShared_NPCConfig: 一种类型的Agent的总表, 包含这种npc的各种参数		* .RShared_AnimationDatabase, 一个动画信息数据库, 不是骨骼动画数据本身, 而是描述一套动画的信息(比如位移, 转角, ..), 记录在RShared_NPCConfig中		* .是手工配置的表（不是自动生成的)		* .是一个 RShared_AnimationDatabaseEntry 的数组		* .每个Entry用一个 BP_ExecutePoint 来索引		* .RShared_AnimationDatabaseEntry, 一套动画的信息数据		* .里面还记了一个Handle, 其实是在数组里的索引		* .RShared_AnimationQueryHandler 为一个用来访问它的工具类		* .BP_AnimationQueryData: 用来到 RShared_AnimationDatabase 中进行查询的参数		* .BP_AnimationQueryResults : 用来保存到 RShared_AnimationDatabase 中进行查询的结果, 是一个 BP_AnimationQueryResult 数组		* .BP_AnimQueryDispatcher 为一个工具类, 用来使对 RShared_AnimationDatabase 进行查询		* .BP_AnimQueryDispatcher::ExecuteAnimationQuery(..),		* .BShared_AnimationMetricsData, 偏向于和移动相关的的capability的设置		* .每一个Agent有一个BShared_AnimationMetricsData, 配置在在RShared_NPCConfig中		* .和动画没什么关系		* .包含一些在不同速度下的转弯半径(曲率)的设置		* .在不同navmesh area type的移动速度		* .BServer_AnimationMetricsQueryHandler 为一个用来访问它的工具类		* .BP_BindingBridge, 是一张表, 为每一个 BP_ControlPoint 设置一个BP_ExecutePoint, 建立对应关系		* .每种NPC会配置一张表, 手工填写		* .BP_MovementTools, 一个工具类, 集合了 BServer_AnimationMetricsQueryHandler 和 RShared_AnimationQueryHandler 和 BP_BindingBridge		* .BP_WorkingSet 是一个waypoint(BP_Waypoint)的数组		* .BP_Waypoint 代表一个路点, 有各种各样类型(BP_WaypointType)		* .大杂烩		* .注意一个 waypoint 可以同时是多种类型(比如 BP_WaypointType::STOP_EXIT 和 BP_WaypointType::START_ENTRY)		* .如果某个Waypoint的类型为ARC, 表示这个点是一条弧的起点, 下个点是弧的终点(但它的类型大概率不是ARC)		* .参加 BP_WaypointUtil_Private::InsertWaypointArc(..), 这个函数参数是一个尖锐的拐点, 正常情况下, 会在它前方插入一个ARC的Waypoint, 作为弧的起点, 自己则向后挪动, 作为弧的终点, 结果就是用一个圆角代替了原来的尖角		* .如果类型为CUSTOM_ANIMATION, 表示这个点是一段放完一段动画的终点		* .参加 BP_WorkingSet::AddCustomAnimationWaypoints(..)		* .myAnimationQueryResults(BP_AnimationQueryResults), 记录了一系列动画信息		* .BP_Plan 代表一组预先设置好的命令		* .由一个fragment(BP_CommandStreamFragment)序列 组成		* .每个flagment 本身并不记录它的时间范围, 它的时间范围是由它的所有的track的时间范围 merge 出来的		* .每个fragment 在时间上和距离上都是相接的		* .BP_Plan::AddFragment(..) 中会把新加入的flagment的时间和距离都调到上一个fragment的最后		* .每个Fragment里包含多个不同类型的Track		* .BP_MovementChangeHandler, 一个工具类, 生成Movement的核心代码		* .BP_MovementChangeHandler::RunMovementInitialization() :核心函数		* .BServer_PathFinder, 提供路径搜索功能		* .BServer_NPCPathIntegrator, 在 BServer_PathFinder 外又包了一层, 实现一下路径搜索相关的功能		* .包含一个pathfinder(BServer_PathFinder)和agent(RServer_Agent)		* .BServer_NPCAgentController::RequestMovement() 启动一个移动		* .BServer_NPCAgentController::GenerateMovement()		* .用来生成一个Movement(BP_MovementData)	";

	request.AddUserMessage(fileContent.c_str());
	request.AddUserMessage("The above are required files");
// 	extern bool IsPrompCachingEnabled();
// 	if (IsPrompCachingEnabled())
// 		request.AddCacheControl();

	request.AddUserMessage("Hello?");
	request.AddAssistMessage("Hello! Can I help you");
	request.AddUserMessage("Can you write a poem for me?");
	extern bool IsPrompCachingEnabled();
	if (IsPrompCachingEnabled())
		request.AddCacheControl();

	return true;
}


bool CChatCtrl::_GetLastFileTimeInCheckpoint(const std::wstring& fullPath, int startIdx, int endIdx, AbsTick& t)
{
	t = 0;
	
	// 检查参数有效性
	if (startIdx < 0 || endIdx <= startIdx || endIdx > static_cast<int>(_ops.size()) || fullPath.empty())
		return false;
	
	extern CCheckpoints* GetCheckpoints();
	CCheckpoints* pCheckpoints = GetCheckpoints();
	if (!pCheckpoints)
		return false;
	
	// 将文件路径转换为string，用于checkpoint API调用
	std::string filePathStr = widechar_to_utf8(fullPath.c_str());
	
	// 从后往前遍历指定范围的操作，寻找最后一个包含该文件的checkpoint
	for (int i = endIdx - 1; i >= startIdx; i--)
	{
		const ChatCtrlOp& op = _ops[i];
		
		// 检查操作是否有有效的checkpoint
		if (op.checkpointId == FilesCheckpointUID_Invalid)
			continue;
		
		bool shouldCheckThisCheckpoint = false;
		
		// 根据操作类型判断是否需要检查此checkpoint
		switch (op.type)
		{
			case ChatCtrlOp::Op_SetFileEditContent:
			{
				// 通过fileEditId获取文件路径，如果匹配则检查checkpoint
				std::wstring fileEditFullPath;
				if (GetFileEditFullPath(op.fileEditId, fileEditFullPath) && fileEditFullPath == fullPath)
				{
					shouldCheckThisCheckpoint = true;
				}
				break;
			}
			
			case ChatCtrlOp::Op_AddFileEditToAIMessage:
			{
				// 检查fullPath字段是否匹配
				if (op.fullPath == fullPath)
				{
					shouldCheckThisCheckpoint = true;
				}
				break;
			}
			
			case ChatCtrlOp::Op_FileAttaches:
			{
				// 检查FileAttaches列表中是否包含指定文件
				std::vector<std::wstring> attachedFiles;
				SplitStringBy(L"|", op.content, &attachedFiles);
				
				for (const auto& attachedFile : attachedFiles)
				{
					if (attachedFile == fullPath)
					{
						shouldCheckThisCheckpoint = true;
						break;
					}
				}
				break;
			}
			
			case ChatCtrlOp::Op_BeginSession:
			{
				// BeginSession的checkpoint需要直接检查是否包含文件
				shouldCheckThisCheckpoint = true;
				break;
			}
			
			default:
				// 其他操作类型暂不处理
				continue;
		}
		
		// 如果应该检查此checkpoint，则验证它是否真的包含指定文件
		if (shouldCheckThisCheckpoint)
		{
			if (pCheckpoints->GetCheckpointFileTick(op.checkpointId, filePathStr.c_str(), t))
				return true;
		}
	}
	
	// 没有找到包含该文件的checkpoint
	t = 0;
	return false;
}

//====================== Symbol 链接功能实现 ======================

// 收集页面中所有消息的 symbols
void CChatCtrl::CollectSymbols()
{
	if (!_IsReady())
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"collectSymbols\"}";

	// 发送到WebView
	PostWebMessageAsJson(jsonMessage);
}

// 应用 Symbol 链接样式
// symbolsWithResults: vector<pair<symbol, resultsJson>>
// resultsJson 格式: [{"filePath":"xxx","lineNumber":123},...]
void CChatCtrl::ApplySymbolLinks(const std::wstring& messageId, const std::vector<std::pair<std::wstring, std::wstring>>& symbolsWithResults)
{
	if (!_IsReady())
		return;

	if (symbolsWithResults.empty())
		return;

	// 构建 symbols 的 JSON 数组，直接使用已构建好的 resultsJson
	std::wstring symbolsJson = L"[";
	for (size_t i = 0; i < symbolsWithResults.size(); i++)
	{
		if (i > 0)
			symbolsJson += L",";
		
		const std::wstring& symbol = symbolsWithResults[i].first;
		const std::wstring& resultsJson = symbolsWithResults[i].second;
		
		// 构建完整的 symbol 对象
		symbolsJson += L"{\"symbol\":\"" + EscapeJsonString(symbol) + L"\",\"results\":" + resultsJson + L"}";
	}
	symbolsJson += L"]";

	// 构造 JSON 消息
	std::wstring jsonMessage = L"{\"action\":\"applySymbolLinks\",\"messageId\":\"" + EscapeJsonString(messageId) + L"\",\"symbols\":" + symbolsJson + L"}";

	// 发送到 WebView
	PostWebMessageAsJson(jsonMessage);
}

// 设置 Symbol 链接点击回调
void CChatCtrl::SetSymbolLinkClickedCallback(SymbolLinkClickedCallback callback)
{
	_symbolLinkClickedCallback = callback;
}

// 设置 Symbol 查询回调
void CChatCtrl::SetQuerySymbolLocationsCallback(QuerySymbolLocationsCallback callback)
{
	_querySymbolLocationsCallback = callback;
}

