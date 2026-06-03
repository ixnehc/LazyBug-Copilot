#include "stdh.h"
#include "ChatUi.h"
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <pathcch.h> // For PathCchRemoveFileSpec - Link with Pathcch.lib
#include "timer/wuid.h"
#include "datapacket/DataPacket.h"
#include "LlmSession.h"
#include "stringparser/stringparser.h"
#include "nlohmann/json.hpp"

#include "Utils.h"
#include "utils_image.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const char* str);

//////////////////////////////////////////////////////////////////////////
//ChatCtrlOp

//////////////////////////////////////////////////////////////////////////
//CChatUi


BEGIN_MESSAGE_MAP(CChatUi, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

// 构造函数
CChatUi::CChatUi()
    : _isWebViewCreated(false)
    , _isChatInitialized(false)
    , _callbackId(0)
    , _title(DEFAULT_CHAT_TITLE)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
{
	// 创建标题栏菜单窗口
	_titleMenuWindow.CreateTitleMenuWindow(this);
}

// 析构函数

// 析构函数
CChatUi::~CChatUi()
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
BOOL CChatUi::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
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
HRESULT CChatUi::InitializeWebView()
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
//                                                            int fileEditIndex = _FindFileEditOpIndex(fileEditId);
                                                            
//                                                             if (fileEditIndex != -1)
//                                                             {
//                                                                 // 获取disable边界
//                                                                 int disableAfterIndex = _GetDisableAfterIndex();
//                                                                 
//                                                                 // 如果FileEdit操作在disable边界之后，则认为是disabled
//                                                                 if (fileEditIndex >= disableAfterIndex)
//                                                                 {
//                                                                     isDisabled = true;
//                                                                 }
//                                                             }
                                                            
                                                            // 只有在未被disabled时才调用回调
                                                            if (!isDisabled && _fileEditTitleClickedCallback)
                                                            {
                                                                _fileEditTitleClickedCallback(fileEditId);
                                                            }
                                                        }
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
                                                // 处理问题答案消息
                                                else if (msgStr.find(L"questionAnswer") != std::wstring::npos)
                                                {
                                                    try
                                                    {
                                                        // 使用 JSON 库解析消息
                                                        std::string utf8Msg = widechar_to_utf8(msgStr.c_str());
                                                        auto jsonMsg = nlohmann::json::parse(utf8Msg);
                                                        
                                                        // 检查是否是字符串类型（被转义的JSON）
                                                        if (jsonMsg.is_string())
                                                        {
                                                            // 解析内层的 JSON 字符串
                                                            std::string innerJson = jsonMsg.get<std::string>();
                                                            jsonMsg = nlohmann::json::parse(innerJson);
                                                        }
                                                        
                                                        if (jsonMsg.contains("action") && jsonMsg["action"] == "questionAnswer")
                                                        {
                                                            if (jsonMsg.contains("questionId") && jsonMsg.contains("answer"))
                                                            {
                                                                __int64 questionId;
                                                                
                                                                // questionId 可能是字符串或数字
                                                                if (jsonMsg["questionId"].is_string())
                                                                {
                                                                    std::string idStr = jsonMsg["questionId"].get<std::string>();
                                                                    questionId = _atoi64(idStr.c_str());
                                                                }
                                                                else if (jsonMsg["questionId"].is_number())
                                                                {
                                                                    questionId = jsonMsg["questionId"].get<__int64>();
                                                                }
                                                                else
                                                                {
                                                                    return S_OK;
                                                                }
                                                                
                                                                std::string answer = jsonMsg["answer"].get<std::string>();
                                                                
                                                                // 保存答案
                                                                std::lock_guard<std::mutex> lock(_questionMutex);
                                                                if (_currentQuestion.id == questionId)
                                                                {
                                                                    _currentQuestion.answer = utf8_to_widechar(answer.c_str());
                                                                    _currentQuestion.hasAnswer = true;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    catch (const std::exception& )
                                                    {
                                                        // JSON parsing error - silently ignore
                                                    }
                                                }
                                                // 处理 CLI 输入消息
                                                else if (msgStr.find(L"\"action\":\"cliInput\"") != std::wstring::npos)
                                                {
                                                    try
                                                    {
                                                        // 使用 JSON 库解析消息
                                                        std::string utf8Msg = widechar_to_utf8(msgStr.c_str());
                                                        auto jsonMsg = nlohmann::json::parse(utf8Msg);
                                                        
                                                        if (jsonMsg.contains("cliId") && jsonMsg.contains("input"))
                                                        {
                                                            std::string cliId = jsonMsg["cliId"].get<std::string>();
                                                            std::string input = jsonMsg["input"].get<std::string>();
                                                            
                                                            // 直接保存 CLI 输入
                                                            SendCliInput(
                                                                utf8_to_widechar(cliId.c_str()),
                                                                utf8_to_widechar(input.c_str())
                                                            );
                                                        }
                                                    }
                                                    catch (const std::exception& e)
                                                    {
                                                        // JSON parsing error - silently ignore
                                                    }
                                                }
                                                // 处理启动 pending CLI 消息（用户点击播放按钮）
                                                else if (msgStr.find(L"\"action\":\"startPendingCli\"") != std::wstring::npos)
                                                {
                                                    try
                                                    {
                                                        // 使用 JSON 库解析消息
                                                        std::string utf8Msg = widechar_to_utf8(msgStr.c_str());
                                                        auto jsonMsg = nlohmann::json::parse(utf8Msg);
                                                        
                                                        if (jsonMsg.contains("cliId"))
                                                        {
                                                            std::string cliId = jsonMsg["cliId"].get<std::string>();
                                                            std::wstring wCliId = utf8_to_widechar(cliId.c_str());
                                                            
                                                            // 从 pending 集合中移除该 CLI ID
                                                            RemovePendingCli(wCliId);
                                                        }
                                                    }
                                                    catch (const std::exception& e)
                                                    {
                                                        // JSON parsing error - silently ignore
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
void CChatUi::Navigate(const std::wstring& url)
{
    if (_webView != nullptr)
    {
        _webView->Navigate(url.c_str());
    }
}

// 导航到HTML字符串
void CChatUi::NavigateToString(const std::wstring& htmlContent)
{
    if (_webView != nullptr)
    {
        _webView->NavigateToString(htmlContent.c_str());
    }
}

// 重新加载当前页面
void CChatUi::Reload()
{
    if (_webView != nullptr)
    {
        _webView->Reload();
    }
}

// 执行JavaScript脚本
void CChatUi::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
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
void CChatUi::SetNavigationCompletedCallback(WebViewNavigationCompletedCallback callback)
{
    _navigationCompletedCallback = callback;
}

// 设置Web消息接收回调
void CChatUi::SetWebMessageReceivedCallback(WebViewMessageReceivedCallback callback)
{
    _webMessageReceivedCallback = callback;
}

// 设置标题栏菜单更新回调
void CChatUi::SetTitlebarMenuUpdateCallback(TitlebarMenuUpdateCallback callback)
{
    _titlebarMenuUpdateCallback = callback;
}

// 设置FileEdit标题点击回调
void CChatUi::SetFileEditTitleClickedCallback(FileEditTitleClickedCallback callback)
{
    _fileEditTitleClickedCallback = callback;
}

// 设置标题栏菜单项点击回调
void CChatUi::SetTitleMenuItemClickedCallback(TitleMenuItemClickedCallback callback)
{
	_titleMenuItemClickedCallback = callback;
}

// 发送消息到WebView
void CChatUi::PostJsonMessage(const std::wstring& message)
{
    if (_webView != nullptr)
    {
        _webView->PostWebMessageAsJson(message.c_str());
    }
}

// 调整WebView大小
void CChatUi::ResizeWebView()
{
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

// 消息处理：大小变化
void CChatUi::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    ResizeWebView();
}

// 消息处理：创建
int CChatUi::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

// 消息处理：销毁
void CChatUi::OnDestroy()
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


// 初始化聊天界面
void CChatUi::InitializeChatUI()
{

    if (_isChatInitialized)
        return;

    _isChatInitialized = true;

	// 设置标题栏菜单回调
	_titleMenuWindow.SetMenuItemClickedCallback([this](const std::wstring& menuItemId,
		const std::wstring& content, const std::wstring& stamp) {
		if (_titleMenuItemClickedCallback)
		{
			_titleMenuItemClickedCallback(menuItemId, content, stamp);
		}
	});
}


// 设置主题 (light/dark)
void CChatUi::SetTheme(const std::wstring& theme)
{
    if (!_IsReady())
        return;
    
    // 发送主题设置消息
    std::wstring jsonMessage = L"{\"action\":\"setTheme\",\"theme\":\"" + theme + L"\"}";
    PostJsonMessage(jsonMessage);
}


// 设置FileSummarize点击回调
void CChatUi::SetFileSummarizeClickedCallback(FileSummarizeClickedCallback callback)
{
    _fileSummarizeClickedCallback = callback;
}


//====================== WebView 标题栏功能实现 ======================

// 添加标题栏菜单项
void CChatUi::AddTitlebarMenuItem(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp)
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
	PostJsonMessage(jsonMessage);
}

// 清空所有标题栏菜单项
void CChatUi::ClearTitlebarMenuItems()
{
	// 清空 C++ 标题栏菜单
	_titleMenuWindow.ClearMenuItems();

	if (!_IsReady())
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"clearTitlebarMenuItems\"}";

	// 发送到WebView
	PostJsonMessage(jsonMessage);
}

// 显示标题栏菜单（仅显示C++原生菜单，不显示HTML菜单）
void CChatUi::ShowTitlebarMenu()
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
void CChatUi::HideTitlebarMenu()
{
	// 隐藏 C++ 标题栏菜单
	_titleMenuWindow.HideMenu();

	// 不再发送hideTitlebarMenu消息到WebView
}

// 切换标题栏菜单显示状态
void CChatUi::ToggleTitlebarMenu()
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
	PostJsonMessage(jsonMessage);
}

// 显示/隐藏暂停状态边框
void CChatUi::ShowPause(bool bShow, bool bFlow)
{
	if (!_IsReady())
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"showPause\",\"show\":" + std::wstring(bShow ? L"true" : L"false") + L",\"flow\":" + std::wstring(bFlow ? L"true" : L"false") + L"}";

	// 发送到WebView
	PostJsonMessage(jsonMessage);
}

// 停止/恢复暂停边框的流动动画
void CChatUi::StopPauseFlow(bool bStop)
{
	if (!_IsReady())
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"stopPauseFlow\",\"stop\":" + std::wstring(bStop ? L"true" : L"false") + L"}";

	// 发送到WebView
	PostJsonMessage(jsonMessage);
}

// 检查WebView和Chat是否已初始化
bool CChatUi::_IsReady() const
{
    return _isWebViewCreated && _isChatInitialized;
}

//====================== Symbol 链接功能实现 ======================

// 收集页面中所有消息的 symbols
void CChatUi::CollectSymbols()
{
	if (!_IsReady())
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"collectSymbols\"}";

	// 发送到WebView
	PostJsonMessage(jsonMessage);
}

// 应用 Symbol 链接样式
// symbolsWithResults: vector<pair<symbol, resultsJson>>
// resultsJson 格式: [{"filePath":"xxx","lineNumber":123},...]
void CChatUi::ApplySymbolLinks(const std::wstring& messageId, const std::vector<std::pair<std::wstring, std::wstring>>& symbolsWithResults)
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
	PostJsonMessage(jsonMessage);
}

// 设置 Symbol 链接点击回调
void CChatUi::SetSymbolLinkClickedCallback(SymbolLinkClickedCallback callback)
{
	_symbolLinkClickedCallback = callback;
}

// 设置 Symbol 查询回调
void CChatUi::SetQuerySymbolLocationsCallback(QuerySymbolLocationsCallback callback)
{
	_querySymbolLocationsCallback = callback;
}

// 发送 CLI 输入
void CChatUi::SendCliInput(const std::wstring& cliId, const std::wstring& input)
{
	std::lock_guard<std::mutex> lock(_cliInputMutex);
	_currentCliInput.cliId = cliId;
	_currentCliInput.input = input;
	_currentCliInput.hasInput = true;
}

// 获取 CLI 输入
bool CChatUi::GetCliInput(const std::wstring& cliId, std::wstring& input)
{
	std::lock_guard<std::mutex> lock(_cliInputMutex);
	if (_currentCliInput.cliId == cliId && _currentCliInput.hasInput)
	{
		input = _currentCliInput.input;
		_currentCliInput.hasInput = false;  // 消费掉
		return true;
	}
	return false;
}

// 检查是否有 CLI 输入
bool CChatUi::HasCliInput(const std::wstring& cliId)
{
	std::lock_guard<std::mutex> lock(_cliInputMutex);
	return (_currentCliInput.cliId == cliId && _currentCliInput.hasInput);
}

// 清空 CLI 输入
void CChatUi::ClearCliInput()
{
	std::lock_guard<std::mutex> lock(_cliInputMutex);
	_currentCliInput.cliId.clear();
	_currentCliInput.input.clear();
	_currentCliInput.hasInput = false;
}

__int64 CChatUi::AddQuestion(const std::wstring& messageId, const std::wstring& question, const std::vector<std::wstring>& options)
{
	if (!_IsReady())
		return 0;

	// 生成唯一ID
	__int64 questionId = GetAbsTick();

	// 保存问题数据
	{
		std::lock_guard<std::mutex> lock(_questionMutex);
		_currentQuestion.id = questionId;
		_currentQuestion.question = question;
		_currentQuestion.options = options;
		_currentQuestion.hasAnswer = false;
		_currentQuestion.answer.clear();
	}

	// 构造 JSON 消息
	std::wstring optionsJson = L"[";
	for (size_t i = 0; i < options.size(); i++)
	{
		if (i > 0)
			optionsJson += L",";
		optionsJson += L"\"" + EscapeJsonString(options[i]) + L"\"";
	}
	optionsJson += L"]";

	std::wstring jsonMessage = L"{\"action\":\"addQuestion\",\"messageId\":\"" + EscapeJsonString(messageId) + 
		L"\",\"questionId\":\"" + std::to_wstring(questionId) + 
		L"\",\"question\":\"" + EscapeJsonString(question) + 
		L"\",\"options\":" + optionsJson + L"}";

	// 发送到 WebView
	PostJsonMessage(jsonMessage);

	return questionId;
}

bool CChatUi::GetQuestionAnswer(__int64 questionId, std::wstring& answer)
{
	std::lock_guard<std::mutex> lock(_questionMutex);
	if (_currentQuestion.id == questionId && _currentQuestion.hasAnswer)
	{
		answer = _currentQuestion.answer;
		return true;
	}
	return false;
}

bool CChatUi::HasQuestionAnswer(__int64 questionId)
{
	std::lock_guard<std::mutex> lock(_questionMutex);
	return (_currentQuestion.id == questionId && _currentQuestion.hasAnswer);
}

void CChatUi::ClearQuestion()
{
	if (!_IsReady())
		return;

	// 清空问题数据
	{
		std::lock_guard<std::mutex> lock(_questionMutex);
		_currentQuestion.id = 0;
		_currentQuestion.question.clear();
		_currentQuestion.options.clear();
		_currentQuestion.answer.clear();
		_currentQuestion.hasAnswer = false;
	}

	// 构造 JSON 消息
	std::wstring jsonMessage = L"{\"action\":\"clearQuestion\"}";

	// 发送到 WebView
	PostJsonMessage(jsonMessage);
}

void CChatUi::AddQuestionDisplay(const std::wstring& messageId, const std::wstring& question, const std::wstring& answer)
{
	if (!_IsReady())
		return;

	// 构造 JSON 消息
	std::wstring jsonMessage = L"{\"action\":\"addQuestionDisplay\",\"messageId\":\"" + EscapeJsonString(messageId) + 
		L"\",\"question\":\"" + EscapeJsonString(question) + 
		L"\",\"answer\":\"" + EscapeJsonString(answer) + L"\"}";

	// 发送到 WebView
	PostJsonMessage(jsonMessage);
}

void CChatUi::ActivateCheckpointFileChange(const std::wstring& fileEditId)
{
	if (_fileEditTitleClickedCallback)
	{
		_fileEditTitleClickedCallback(fileEditId);
	}
}

//====================== CLI Display 相关实现 ======================

void CChatUi::AddCliDisplay(const std::wstring& messageId, const std::wstring& cliId, const std::wstring& command, const std::wstring& desc, CliDisplayStatus displayStatus, const std::wstring& shellType)
{
	if (command.empty() || cliId.empty())
		return;

	// 设置 CLI 状态
	{
		std::lock_guard<std::mutex> lock(_cliStatusMutex);
		_cliStatus[cliId] = (displayStatus == CliDisplayStatus::Pending) ? CliStatus::Pending : CliStatus::Accept;
	}

	if (!_IsReady())
		return;

	// 转义消息内容
	std::wstring safeMessageId = EscapeJsonString(messageId);
	std::wstring safeCommand = EscapeJsonString(command);
	std::wstring safeDesc = EscapeJsonString(desc);
	std::wstring safeCliId = EscapeJsonString(cliId);
	std::wstring safeShellType = EscapeJsonString(shellType);

	// 将 displayStatus 转换为字符串
	std::wstring statusStr;
	switch (displayStatus)
	{
	case CliDisplayStatus::Pending:
		statusStr = L"pending";
		break;
	case CliDisplayStatus::Accepted:
		statusStr = L"accepted";
		break;
	case CliDisplayStatus::None:
	default:
		statusStr = L"none";
		break;
	}

	// 构造 JSON 消息 - 发送 command、desc、cliId、status 和 shellType
	std::wstring jsonMessage = L"{\"action\":\"addCliDisplay\",\"messageId\":\"" + safeMessageId +
		L"\",\"cliId\":\"" + safeCliId +
		L"\",\"command\":\"" + safeCommand +
		L"\",\"desc\":\"" + safeDesc +
		L"\",\"status\":\"" + statusStr +
		L"\",\"shellType\":\"" + safeShellType + L"\"}";

	PostJsonMessage(jsonMessage);
}

bool CChatUi::IsCliPending(const std::wstring& cliId)
{
	std::lock_guard<std::mutex> lock(_cliStatusMutex);
	auto it = _cliStatus.find(cliId);
	return it != _cliStatus.end() && it->second == CliStatus::Pending;
}

void CChatUi::RemovePendingCli(const std::wstring& cliId)
{
	std::lock_guard<std::mutex> lock(_cliStatusMutex);
	_cliStatus.erase(cliId);
}

CliStatus CChatUi::GetCliStatus(const std::wstring& cliId)
{
	std::lock_guard<std::mutex> lock(_cliStatusMutex);
	auto it = _cliStatus.find(cliId);
	if (it != _cliStatus.end())
		return it->second;
	return CliStatus::None;
}

void CChatUi::SetCliStatus(const std::wstring& cliId, CliStatus status)
{
	std::lock_guard<std::mutex> lock(_cliStatusMutex);
	_cliStatus[cliId] = status;
}

