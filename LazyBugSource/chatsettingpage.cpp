#include "stdh.h"
#include "ChatSettingPage.h"
#include <fstream>
#include <algorithm>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>
#include "llmlibloader.h"
#include "ChatDialogA.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern const char* GetCurModuleFolderPath_utf8();

//////////////////////////////////////////////////////////////////////////
// CChatSettingPage

BEGIN_MESSAGE_MAP(CChatSettingPage, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_MESSAGE(CChatSettingDirWatchTab::WM_DIRWATCH_PICK_FOLDER, OnDirWatchPickFolder)
END_MESSAGE_MAP()

// 构造函数
CChatSettingPage::CChatSettingPage()
    : _isWebViewCreated(false)
    , _isSettingInitialized(false)
    , _callbackId(0)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
    , _activeTabId(L"providers")
    , _llmLibVersion(-1)
{
}

// 析构函数
CChatSettingPage::~CChatSettingPage()
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

    // 终止所有任务
    _taskMgr.Interrupt();
}

// 创建WebView2控件
BOOL CChatSettingPage::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)::GetStockObject(WHITE_BRUSH),
        ::LoadIcon(NULL, IDI_APPLICATION));

    // 创建窗口
    BOOL result = CWnd::CreateEx(0, className, _T("Setting Page"),
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER, rect, pParentWnd, nID);

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

	ChatTaskContext ctx;
	ctx.chatSettingPage = this;
	ctx.chatDialogA = (CChatDialogA*)GetParent();
	_taskMgr.Init(ctx);

    // 初始化 Provider Tab 逻辑控制器
    _providerTab.Init(
        [this](const std::wstring& action, const std::wstring& data) { _PostWebMessage(action, data); },
        [this](const std::wstring& fullJson) { _PostWebMessage(L"", fullJson, true); },
        [this](const std::wstring& script) { ExecuteScript(script); },
        [this]() { return _IsReady(); },
        &_taskMgr,
        GetSafeHwnd()
    );
    _providerTab.SetFindSessionEndsCallback([this]() -> std::vector<int> {
        CChatDialogA* pDialog = (CChatDialogA*)GetParent();
        if (!pDialog) return {};
        CChatOpsCtrl* pOpsCtrl = &pDialog->GetOpsCtrl();
        if (!pOpsCtrl) return {};
        return pOpsCtrl->FindLastNNotDisabledSessionEnds(3);
    });

    // 初始化 DirWatch Tab 逻辑控制器
    _dirWatchTab.Init(
        [this](const std::wstring& action, const std::wstring& data) { _PostWebMessage(action, data); },
        [this](const std::wstring& fullJson) { _PostWebMessage(L"", fullJson, true); },
        [this](const std::wstring& script) { ExecuteScript(script); },
        [this]() { return _IsReady(); },
        GetSafeHwnd()
    );

    // 初始化 Database Tab 逻辑控制器
    _databaseTab.Init(
        [this](const std::wstring& action, const std::wstring& data) { _PostWebMessage(action, data); },
        [this](const std::wstring& fullJson) { _PostWebMessage(L"", fullJson, true); },
        [this](const std::wstring& script) { ExecuteScript(script); },
        [this]() { return _IsReady(); },
        GetSafeHwnd()
    );
    _databaseTab.SetDeleteOldChatsCallback([this](int days, const char* checkpointsDir) {
        CChatDialogA* pDialog = (CChatDialogA*)GetParent();
        if (pDialog)
            pDialog->GetChatHistory().DeleteOldChats(days, checkpointsDir);
    });

    return result;
}

// 初始化WebView2环境
HRESULT CChatSettingPage::InitializeWebView()
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
                                                
                                                // 如果导航成功且还没初始化设置界面，则初始化
                                                if (success && !_isSettingInitialized)
                                                {
                                                    InitializeSettingUI();
                                                }

                                                return S_OK;
                                            }).Get(),
                                        &_navigationCompletedToken);

                                    // Web消息接收事件
                                    _webView->add_WebMessageReceived(
                                        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                            [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                                LPWSTR message;
                                                args->get_WebMessageAsJson(&message);
                                                
                                                std::wstring msgStr(message);
                                                _HandleWebMessage(msgStr);
                                                
                                                if (_webMessageReceivedCallback)
                                                {
                                                    _webMessageReceivedCallback(message);
                                                }
                                                CoTaskMemFree(message);
                                                return S_OK;
                                            }).Get(),
                                        &_webMessageReceivedToken);

								std::string htmlPath = GetCurModuleFolderPath_utf8();
									htmlPath += "\\chatsettingpage\\chatsettingpage.html";
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
void CChatSettingPage::Navigate(const std::wstring& url)
{
    if (_webView != nullptr)
    {
        _webView->Navigate(url.c_str());
    }
}

// 导航到HTML字符串
void CChatSettingPage::NavigateToString(const std::wstring& htmlContent)
{
    if (_webView != nullptr)
    {
        _webView->NavigateToString(htmlContent.c_str());
    }
}

// 重新加载当前页面
void CChatSettingPage::Reload()
{
    if (_webView != nullptr)
    {
        _webView->Reload();
    }
}

// 执行JavaScript脚本
void CChatSettingPage::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
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
void CChatSettingPage::SetNavigationCompletedCallback(SettingPageNavigationCompletedCallback callback)
{
    _navigationCompletedCallback = callback;
}

// 设置Web消息接收回调
void CChatSettingPage::SetWebMessageReceivedCallback(SettingPageMessageReceivedCallback callback)
{
    _webMessageReceivedCallback = callback;
}

// 设置退出回调
void CChatSettingPage::SetExitCallback(SettingPageExitCallback callback)
{
    _exitCallback = callback;
}

// 调整WebView大小
void CChatSettingPage::ResizeWebView()
{
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

// 消息处理：大小变化
void CChatSettingPage::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    ResizeWebView();
}

// 消息处理：创建
int CChatSettingPage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

// 消息处理：销毁
void CChatSettingPage::OnDestroy()
{
    CWnd::OnDestroy();

    // 关闭WebView
    if (_controller != nullptr)
    {
        _controller->Close();
    }

	_dirWatchTab.Shutdown();
	_taskMgr.Shutdown();
}

// 处理延迟的文件夹选择消息（不能在 WebView 回调中直接弹模态对话框）
LRESULT CChatSettingPage::OnDirWatchPickFolder(WPARAM, LPARAM)
{
    _dirWatchTab.OnPickFolderDeferred("");
    return 0;
}

//====================== 设置页面功能相关实现 ======================

// 初始化设置界面
void CChatSettingPage::InitializeSettingUI()
{
    if (_isSettingInitialized)
        return;
    
    _isSettingInitialized = true;
    
    // 初始化LLM Lib版本号
    _llmLibVersion = g_llmLib.GetVer();
    
    // 初始化默认Tab
    _InitializeDefaultTabs();
    
    // 发送初始化完成消息到WebView
    _PostWebMessage(L"initializeComplete",L"");
}

// 初始化默认Tab
void CChatSettingPage::_InitializeDefaultTabs()
{
    // 清空现有Tab
    _tabs.clear();
    
    // Providers Tab (默认)
    SettingTab providersTab;
    providersTab.id = L"providers";
    providersTab.title = L"Providers & APIs";
    _tabs.push_back(providersTab);

    // Folder Watch Tab
    SettingTab folderWatchTab;
    folderWatchTab.id = L"dirwatch";
    folderWatchTab.title = L"Folder Watch";
    _tabs.push_back(folderWatchTab);

    // Database Tab
    SettingTab databaseTab;
    databaseTab.id = L"database";
    databaseTab.title = L"Database";
    _tabs.push_back(databaseTab);
    
    // 发送Tab数据到WebView
    std::wstring tabsJson = _BuildTabsJson();
    _PostWebMessage(L"setTabs", tabsJson);
    
    // 设置默认激活Tab为providers
    SetActiveTab(L"providers");
}

// 添加Tab
void CChatSettingPage::AddTab(const SettingTab& tab)
{
    if (!_IsReady())
        return;
        
    _tabs.push_back(tab);
    
    // 重新发送所有Tab数据
    std::wstring tabsJson = _BuildTabsJson();
    _PostWebMessage(L"setTabs", tabsJson);
}

// 设置激活Tab
void CChatSettingPage::SetActiveTab(const std::wstring& tabId)
{
    if (!_IsReady())
        return;
        
    _activeTabId = tabId;
    _PostWebMessage(L"setActiveTab", tabId);
}

// 清空Tab
void CChatSettingPage::ClearTabs()
{
    if (!_IsReady())
        return;
        
    _tabs.clear();
    _PostWebMessage(L"clearTabs", L"");
}

//====================== 私有辅助方法实现 ======================

// 检查WebView和Setting是否已初始化
bool CChatSettingPage::_IsReady() const
{
    return _isWebViewCreated && _isSettingInitialized;
}

// 生成唯一ID
std::wstring CChatSettingPage::_GenId()
{
    WUID wuid = GenWUID();
    return L"setting_" + std::to_wstring(wuid);
}

// 内部消息发送
void CChatSettingPage::_PostWebMessage(const std::wstring& action, const std::wstring& data)
{
    if (_webView == nullptr)
        return;
        
    std::wstring jsonMessage = L"{\"action\":\"" + action + L"\"";
    if (!data.empty())
    {
        jsonMessage += L",\"data\":" + data;
    }
    jsonMessage += L"}";
    
    _webView->PostWebMessageAsJson(jsonMessage.c_str());
}

// 重载版本：直接发送完整JSON消息
void CChatSettingPage::_PostWebMessage(const std::wstring& action, const std::wstring& data, bool isFullJson)
{
    if (_webView == nullptr)
        return;
    
    if (isFullJson)
    {
        // 直接发送完整的JSON消息
        _webView->PostWebMessageAsJson(data.c_str());
    }
    else
    {
        // 使用原来的格式
        std::wstring jsonMessage = L"{\"action\":\"" + action + L"\"";
        if (!data.empty())
        {
            jsonMessage += L",\"data\":" + data;
        }
        jsonMessage += L"}";
        
        _webView->PostWebMessageAsJson(jsonMessage.c_str());
    }
}

// JSON转义
std::wstring CChatSettingPage::_EscapeJsonString(const std::wstring& str)
{
    std::wstring result = str;
    // 简单的转义实现
    std::wstring::size_type pos = 0;
    while ((pos = result.find(L"\"", pos)) != std::wstring::npos)
    {
        result.replace(pos, 1, L"\\\"");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find(L"\\", pos)) != std::wstring::npos)
    {
        result.replace(pos, 1, L"\\\\");
        pos += 2;
    }
    return result;
}

// 构建Tab JSON
std::wstring CChatSettingPage::_BuildTabsJson()
{
    std::wstring json = L"[";
    
    for (size_t i = 0; i < _tabs.size(); ++i)
    {
        if (i > 0) json += L",";
        
        const SettingTab& tab = _tabs[i];
        json += L"{";
        json += L"\"id\":\"" + _EscapeJsonString(tab.id) + L"\",";
        json += L"\"title\":\"" + _EscapeJsonString(tab.title) + L"\"";
        json += L"}";
    }
    
    json += L"]";
    return json;
}

// 查找Tab
SettingTab* CChatSettingPage::_FindTab(const std::wstring& tabId)
{
    auto it = std::find_if(_tabs.begin(), _tabs.end(),
        [&tabId](const SettingTab& tab) { return tab.id == tabId; });
    
    return (it != _tabs.end()) ? &(*it) : nullptr;
}


// 处理来自JavaScript的消息
void CChatSettingPage::_HandleWebMessage(const std::wstring& message)
{
    // 将宽字符串转换为UTF-8字符串用于JSON解析
    std::string utf8Message = widechar_to_utf8(message.c_str());
    
    try {
        // 解析JSON消息
        nlohmann::json jsonMsg = nlohmann::json::parse(utf8Message);
        
        // 检查是否有action字段
        if (!jsonMsg.contains("action") || !jsonMsg["action"].is_string())
        {
            return; // 无效消息，忽略
        }
        
        std::string action = jsonMsg["action"];
        
        // 处理不同类型的消息
        // 先让 Provider Tab 处理 Provider 相关消息
        if (_providerTab.HandleWebMessage(action, jsonMsg))
            return;

        // 再让 DirWatch Tab 处理 DirWatch 相关消息
        if (_dirWatchTab.HandleWebMessage(action, jsonMsg))
            return;

        // 再让 Database Tab 处理 Database 相关消息
        if (_databaseTab.HandleWebMessage(action, jsonMsg))
            return;
        
        // 处理通用消息
        if (action == "tabChanged")
        {
            // Tab切换事件
            if (jsonMsg.contains("tabId"))
            {
                std::string tabId = jsonMsg["tabId"];
                SetActiveTab(utf8_to_widechar(tabId));
            }
        }
        else if (action == "exitSettings")
        {
            // 终止所有任务
            _taskMgr.Interrupt();

            // 重置 evaluation 状态
            _providerTab.ResetEvaluation();

            // 退出设置页面
            if (_exitCallback)
            {
                _exitCallback();
            }
        }
        else if (action == "readClipboard")
        {
            // 通过Windows API读取剪贴板文本，避免WebView权限弹窗
            std::wstring clipboardText;
            if (::OpenClipboard(GetSafeHwnd()))
            {
                if (IsClipboardFormatAvailable(CF_UNICODETEXT))
                {
                    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
                    if (hData != NULL)
                    {
                        wchar_t* pData = (wchar_t*)GlobalLock(hData);
                        if (pData != NULL)
                        {
                            clipboardText = pData;
                            GlobalUnlock(hData);
                        }
                    }
                }
                else if (IsClipboardFormatAvailable(CF_TEXT))
                {
                    HANDLE hData = GetClipboardData(CF_TEXT);
                    if (hData != NULL)
                    {
                        char* pData = (char*)GlobalLock(hData);
                        if (pData != NULL)
                        {
                            int len = MultiByteToWideChar(CP_ACP, 0, pData, -1, NULL, 0);
                            if (len > 0)
                            {
                                wchar_t* buf = new wchar_t[len];
                                MultiByteToWideChar(CP_ACP, 0, pData, -1, buf, len);
                                clipboardText = buf;
                                delete[] buf;
                            }
                            GlobalUnlock(hData);
                        }
                    }
                }
                CloseClipboard();
            }

            // 转义文本以安全嵌入JS字符串字面量
            std::wstring escaped;
            for (wchar_t ch : clipboardText)
            {
                switch (ch)
                {
                case L'\\': escaped += L"\\\\"; break;
                case L'"':  escaped += L"\\\""; break;
                case L'\n': escaped += L"\\n";  break;
                case L'\r': escaped += L"\\r";  break;
                case L'\t': escaped += L"\\t";  break;
                default:    escaped += ch;      break;
                }
            }

            std::wstring script = L"window.onClipboardData(\"" + escaped + L"\");";
            ExecuteScript(script);
        }
        else if (action == "showError")
        {
            // 显示错误消息
            if (jsonMsg.contains("message"))
            {
                std::string msg = jsonMsg["message"];
                MessageBoxW(GetSafeHwnd(), utf8_to_widechar(msg.c_str()).c_str(), L"Error", MB_OK | MB_ICONERROR);
            }
        }
        // 可以在这里添加其他消息类型的处理
    }
    catch (const std::exception& e)
    {
        // JSON解析错误，忽略消息
        TRACE(_T("Failed to parse WebView message: %s\n"), utf8_to_widechar(e.what()).c_str());
    }
}

//====================== Provider验证转发方法 ======================

void CChatSettingPage::StartValidatingProvider(const LlmApiProviderTypeName& providerTypeName)
{
    _providerTab.StartValidatingProvider(providerTypeName);
}

void CChatSettingPage::EndValidatingProvider(const LlmApiProviderTypeName& providerTypeName, bool available, const std::string& errorMessage)
{
    _providerTab.EndValidatingProvider(providerTypeName, available, errorMessage);
}

bool CChatSettingPage::IsValidatingProvider()
{
    return _providerTab.IsValidatingProvider();
}

//====================== 更新与刷新 ======================

void CChatSettingPage::Update()
{
	_taskMgr.Update();
	UpdateReload();
	_providerTab.Update();
	_dirWatchTab.Update();
	_databaseTab.Update();
}

// 检测并重新加载（如果LLM Lib配置有变化则更新显示）
void CChatSettingPage::UpdateReload()
{
	if (!_IsReady())
		return;

	int currentVersion = g_llmLib.GetVer();
	if (currentVersion != _llmLibVersion)
	{
		_llmLibVersion = currentVersion;
		_providerTab.RefreshData();
	}
}
