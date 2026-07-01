#include "stdh.h"
#include "ChatSettingPage.h"
#include <fstream>
#include <algorithm>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>
#include "llmlibloader.h"
#include "ChatDialogA.h"
#include "ChatOpsCompress.h"
#include "TokenCalibrate.h"

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
    , _isEvaluatingSummarize(false)
    , _needShowNoApiForValidation(false)
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
									htmlPath += "\\ChatSettingPage.html";
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

	_taskMgr.Shutdown();
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
        if (action == "requestProviderData")
        {
            // 请求Provider数据
            LoadProviderData();
            SendProviderDataToWebView();
        }
        else if (action == "requestCapabilityStatus")
        {
            // 请求Capability状态
            SendCapabilityStatusToWebView();
        }
        else if (action == "requestCastSheetData")
        {
            // 发送Cast Sheet数据
            SendCastSheetDataToWebView();
        }
        else if (action == "updateCastSheetApi")
        {
            if (jsonMsg.contains("apiType") && jsonMsg.contains("apiName"))
            {
                std::string apiType = jsonMsg["apiType"];
                std::string apiName = jsonMsg["apiName"];
                UpdateCastSheetApi(utf8_to_widechar(apiType), utf8_to_widechar(apiName));
            }
        }
        else if (action == "updateProviderKey")
        {
            // 更新Provider Key
            if (jsonMsg.contains("providerType") && jsonMsg.contains("key"))
            {
                std::string providerTypeStr = jsonMsg["providerType"];
                std::string key = jsonMsg["key"];
                
                UpdateProviderKey(utf8_to_widechar(providerTypeStr), utf8_to_widechar(key));
            }
        }
        else if (action == "updateProviderName")
        {
            if (jsonMsg.contains("oldName") && jsonMsg.contains("newName"))
            {
                std::string oldName = jsonMsg["oldName"];
                std::string newName = jsonMsg["newName"];
                UpdateProviderName(utf8_to_widechar(oldName), utf8_to_widechar(newName));
            }
        }
        else if (action == "updateProviderEndpoint")
        {
            if (jsonMsg.contains("providerName") && jsonMsg.contains("endpoint"))
            {
                std::string providerName = jsonMsg["providerName"];
                std::string endpoint = jsonMsg["endpoint"];
                UpdateProviderEndpoint(utf8_to_widechar(providerName), utf8_to_widechar(endpoint));
            }
        }
        else if (action == "updateProviderFormat")
        {
            if (jsonMsg.contains("providerName") && jsonMsg.contains("format"))
            {
                std::string providerName = jsonMsg["providerName"];
                std::string format = jsonMsg["format"];
                UpdateProviderFormat(utf8_to_widechar(providerName), utf8_to_widechar(format));
            }
        }
        else if (action == "updateApiName")
        {
            if (jsonMsg.contains("oldName") && jsonMsg.contains("newName"))
            {
                std::string oldName = jsonMsg["oldName"];
                std::string newName = jsonMsg["newName"];
                UpdateApiName(utf8_to_widechar(oldName), utf8_to_widechar(newName));
            }
        }
        else if (action == "updateApiField")
        {
            if (jsonMsg.contains("apiName") && jsonMsg.contains("field") && jsonMsg.contains("value"))
            {
                std::string apiName = jsonMsg["apiName"];
                std::string field = jsonMsg["field"];
                UpdateApiField(utf8_to_widechar(apiName), utf8_to_widechar(field), jsonMsg["value"]);
            }
        }
        else if (action == "tabChanged")
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
            if (_isEvaluatingSummarize)
            {
                _isEvaluatingSummarize = false;
                _PostWebMessage(L"endEvaluateSummarize", L"");
            }

            // 退出设置页面
            if (_exitCallback)
            {
                _exitCallback();
            }
        }
        else if (action == "addProvider")
        {
            // 添加新 Provider
            if (jsonMsg.contains("name"))
            {
                std::string name = jsonMsg["name"];
                AddProvider(utf8_to_widechar(name));
            }
        }
        else if (action == "addProviderFromClipboard")
        {
            // 从剪贴板粘贴创建Provider（含name/endpoint/format）
            if (jsonMsg.contains("name"))
            {
                std::string name = jsonMsg["name"];
                std::wstring nameW = utf8_to_widechar(name);
                if (g_llmLib.AddProvider(name))
                {
                    std::string endpoint = jsonMsg.contains("endpoint") ? jsonMsg["endpoint"].get<std::string>() : "";
                    std::string formatStr = jsonMsg.contains("format") ? jsonMsg["format"].get<std::string>() : "";
                    if (!endpoint.empty())
                        g_llmLib.SetProviderEndpoint(name, endpoint);
                    if (!formatStr.empty())
                    {
                        LlmApiFormat format = LlmApiFormat::Unknown;
                        if (formatStr == "OpenAI") format = LlmApiFormat::OpenAI_;
                        else if (formatStr == "Anthropic") format = LlmApiFormat::Anthropic_;
                        else if (formatStr == "Gemini") format = LlmApiFormat::Gemini_;
                        else if (formatStr == "OpenRouter") format = LlmApiFormat::OpenRouter;
                        else if (formatStr == "Kimi") format = LlmApiFormat::Kimi;
                        else if (formatStr == "GLM") format = LlmApiFormat::GLM;
                        else if (formatStr == "Minimax") format = LlmApiFormat::Minimax;
                        else if (formatStr == "DeepSeek") format = LlmApiFormat::DeepSeek;
                        g_llmLib.SetProviderFormat(name, format);
                    }
                    _SaveLlmJson();
                    LoadProviderData();
                    SendProviderDataToWebView();
                }
            }
        }
        else if (action == "deleteProvider")
        {
            // 删除 Provider
            if (jsonMsg.contains("name"))
            {
                std::string name = jsonMsg["name"];
                DeleteProvider(utf8_to_widechar(name));
            }
        }
        else if (action == "addApi")
        {
            // 添加新 API
            if (jsonMsg.contains("providerName") && jsonMsg.contains("apiName"))
            {
                std::string providerName = jsonMsg["providerName"];
                std::string apiName = jsonMsg["apiName"];
                AddApi(utf8_to_widechar(providerName), utf8_to_widechar(apiName));
            }
        }
        else if (action == "addApiFromClipboard")
        {
            // 从剪贴板粘贴创建API（含所有字段）
            if (jsonMsg.contains("providerName") && jsonMsg.contains("apiName"))
            {
                std::string providerName = jsonMsg["providerName"];
                std::string apiName = jsonMsg["apiName"];
                if (g_llmLib.AddApi(providerName, apiName))
                {
                    // 逐一应用各字段
                    if (jsonMsg.contains("model")) UpdateApiField(utf8_to_widechar(apiName), L"model", jsonMsg["model"]);
                    if (jsonMsg.contains("rule")) UpdateApiField(utf8_to_widechar(apiName), L"rule", jsonMsg["rule"]);
                    if (jsonMsg.contains("maxToken")) UpdateApiField(utf8_to_widechar(apiName), L"maxToken", jsonMsg["maxToken"]);
                    if (jsonMsg.contains("contextCapacity")) UpdateApiField(utf8_to_widechar(apiName), L"contextCapacity", jsonMsg["contextCapacity"]);
                    if (jsonMsg.contains("priceInputToken")) UpdateApiField(utf8_to_widechar(apiName), L"priceInputToken", jsonMsg["priceInputToken"]);
                    if (jsonMsg.contains("priceOutputToken")) UpdateApiField(utf8_to_widechar(apiName), L"priceOutputToken", jsonMsg["priceOutputToken"]);
                    if (jsonMsg.contains("priceCacheRead")) UpdateApiField(utf8_to_widechar(apiName), L"priceCacheRead", jsonMsg["priceCacheRead"]);
                    if (jsonMsg.contains("priceCacheWrite")) UpdateApiField(utf8_to_widechar(apiName), L"priceCacheWrite", jsonMsg["priceCacheWrite"]);
                    if (jsonMsg.contains("thinkingMode")) UpdateApiField(utf8_to_widechar(apiName), L"thinkingMode", jsonMsg["thinkingMode"]);
                    if (jsonMsg.contains("cacheControl")) UpdateApiField(utf8_to_widechar(apiName), L"cacheControl", jsonMsg["cacheControl"]);
                    if (jsonMsg.contains("role")) UpdateApiField(utf8_to_widechar(apiName), L"role", jsonMsg["role"]);
                    if (jsonMsg.contains("enable")) UpdateApiField(utf8_to_widechar(apiName), L"enable", jsonMsg["enable"]);
                    if (jsonMsg.contains("tools")) UpdateApiField(utf8_to_widechar(apiName), L"tools", jsonMsg["tools"]);
                    if (jsonMsg.contains("openRouterOptions"))
                    {
                        auto& opt = jsonMsg["openRouterOptions"];
                        if (opt.contains("disableReasoning")) UpdateApiField(utf8_to_widechar(apiName), L"disableReasoning", opt["disableReasoning"]);
                        if (opt.contains("only")) UpdateApiField(utf8_to_widechar(apiName), L"openRouterOnly", opt["only"]);
                    }
                    _SaveLlmJson();
                    LoadProviderData();
                    SendProviderDataToWebView();
                }
            }
        }
        else if (action == "deleteApi")
        {
            // 删除 API
            if (jsonMsg.contains("name"))
            {
                std::string name = jsonMsg["name"];
                DeleteApi(utf8_to_widechar(name));
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
        else if (action == "evaluateCompressSummarize")
        {
            // 评估压缩
            if (jsonMsg.contains("apiName"))
            {
                std::string apiName = jsonMsg["apiName"];
                EvaluateCompressSummarize(utf8_to_widechar(apiName));
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

//====================== Provider数据处理方法实现 ======================

extern CLlmLib g_llmLib;

// 加载Provider数据
void CChatSettingPage::LoadProviderData()
{
    // 从g_llmLib加载最新的Provider数据
    // 这里不需要特别的操作，因为我们直接从g_llmLib读取
}


// 发送Cast Sheet数据到WebView
void CChatSettingPage::SendCastSheetDataToWebView()
{
    if (!_IsReady())
        return;

    using json = nlohmann::json;

    // 获取当前选中的API
    std::string majorChatApi = g_llmLib.GetMajorChatApi();
    std::string briefApi = g_llmLib.GetBriefApi();
    std::string summarizeApi = g_llmLib.GetSummarizeApi();
    std::string autoCompleteApi = g_llmLib.GetAutoCompleteApi();
    std::string embeddingApi = g_llmLib.GetEmbeddingApi();

    // 收集可用的API指针
    std::vector<const LlmApi*> availableApis;
    const auto& allApis = g_llmLib.GetApis();
    for (const auto& api : allApis)
    {
        // 跳过enable=false的API
        if (!api.enable)
            continue;

        // 只添加可用的API
        const LlmApiProvider* provider = g_llmLib.GetProvider(api.providerTypeName);
        if (provider && provider->IsAvailable())
        {
            availableApis.push_back(&api);
        }
    }

    // 按名称字母排序（忽略大小写）
    std::sort(availableApis.begin(), availableApis.end(),
        [](const LlmApi* a, const LlmApi* b) {
            return _stricmp(a->name.c_str(), b->name.c_str()) < 0;
        });

    // 分别构建API列表
    json jMajorChatApis = json::array();
    json jBriefApis = json::array();
    json jSummarizeApis = json::array();
    json jAutoCompleteApis = json::array();
    json jEmbeddingApis = json::array();
    
    // 为 Summarize 列表添加特殊选项
    {
        json jDisable;
        jDisable["name"] = SUMMARIZE_API_DISABLE;
        jDisable["provider"] = "";
        jDisable["model"] = "";
        jSummarizeApis.push_back(jDisable);
        
        json jAuto;
        jAuto["name"] = SUMMARIZE_API_AUTO;
        jAuto["provider"] = "";
        jAuto["model"] = "";
        jSummarizeApis.push_back(jAuto);
    }
    
    // 为 Embedding 列表添加特殊选项
    {
        json jDisable;
        jDisable["name"] = EMBEDDING_API_DISABLE;
        jDisable["provider"] = "";
        jDisable["model"] = "";
        jEmbeddingApis.push_back(jDisable);
    }
    
    for (const auto* api : availableApis)
    {
        json jApi;
        jApi["name"] = api->name;
        jApi["provider"] = api->providerTypeName;
        jApi["model"] = api->model;
        
        // Major Chat 只包含 Agent 角色
        if (api->role == LlmApiRole::Agent)
        {
            jMajorChatApis.push_back(jApi);
        }
        
        // Brief 和 Summarize 可以是 Agent 或 Auxiliary
        jBriefApis.push_back(jApi);
        jSummarizeApis.push_back(jApi);
        
        // AutoComplete 可以是 Agent 或 Auxiliary
        jAutoCompleteApis.push_back(jApi);
        
        // Embedding 只包含 Embedding 角色
        if (api->role == LlmApiRole::Embedding)
        {
            jEmbeddingApis.push_back(jApi);
        }
    }

    json jCastSheet;
    jCastSheet["majorChatApi"] = majorChatApi;
    jCastSheet["briefApi"] = briefApi;
    jCastSheet["summarizeApi"] = summarizeApi;
    jCastSheet["autoCompleteApi"] = autoCompleteApi;
    jCastSheet["embeddingApi"] = embeddingApi;
    jCastSheet["majorChatApis"] = jMajorChatApis;
    jCastSheet["briefApis"] = jBriefApis;
    jCastSheet["summarizeApis"] = jSummarizeApis;
    jCastSheet["autoCompleteApis"] = jAutoCompleteApis;
    jCastSheet["embeddingApis"] = jEmbeddingApis;

    std::string utf8Json = jCastSheet.dump();
    _PostWebMessage(L"setCastSheetData", utf8_to_widechar(utf8Json));
}

// 更新Cast Sheet中的API选择
void CChatSettingPage::UpdateCastSheetApi(const std::wstring& apiTypeW, const std::wstring& apiNameW)
{
    std::string apiType = widechar_to_utf8(apiTypeW.c_str());
    std::string apiName = widechar_to_utf8(apiNameW.c_str());

    if (apiType == "majorChat")
    {
        g_llmLib.SetMajorChatApi(apiName);
    }
    else if (apiType == "brief")
    {
        g_llmLib.SetBriefApi(apiName);
    }
    else if (apiType == "summarize")
    {
        g_llmLib.SetSummarizeApi(apiName);
    }
    else if (apiType == "autoComplete")
    {
        g_llmLib.SetAutoCompleteApi(apiName);
    }
    else if (apiType == "embedding")
    {
        g_llmLib.SetEmbeddingApi(apiName);
    }
}

// 发送Provider数据到WebView
void CChatSettingPage::SendProviderDataToWebView()
{
    if (!_IsReady())
        return;
    
    using json = nlohmann::json;

    // 收集所有 role/tool/thinkingMode/cacheControl 枚举的字符串映射
    // （复用 llmlibloader 中已有的字符串，直接在此拼装 json）
    auto roleToStr = [](LlmApiRole r) -> std::string {
        switch (r) {
        case LlmApiRole::Agent:     return "Agent";
        case LlmApiRole::Auxiliary: return "Auxiliary";
        case LlmApiRole::Embedding: return "Embedding";
        default:                    return "None";
        }
    };
    auto toolToStr = [](LlmToolType t) -> std::string {
        switch (t) {
        case LlmToolType::ReplaceInFile:    return "ReplaceInFile";
        case LlmToolType::FindSymbolDefine: return "FindSymbolDefine";
        case LlmToolType::FindInFiles:      return "FindInFiles";
        case LlmToolType::SearchFile:       return "SearchFile";
        case LlmToolType::ReadFile:         return "ReadFile";
        case LlmToolType::CLI_Cmd:          return "CLI_Cmd";
        case LlmToolType::CLI_Bash:         return "CLI_Bash";
        case LlmToolType::CLI_RunScript:    return "CLI_RunScript";
        case LlmToolType::Question:         return "Question";
        case LlmToolType::QueryFinish:      return "QueryFinish";
        case LlmToolType::CreateSkill:      return "CreateSkill";
        case LlmToolType::Mcp:              return "Mcp";
        default:                            return "None";
        }
    };
    auto thinkingToStr = [](LlmThinkingMode m) -> std::string {
        switch (m) {
        case LlmThinkingMode::Enable:  return "Enable";
        case LlmThinkingMode::Disable: return "Disable";
        default:                       return "Auto";
        }
    };
    auto cacheToStr = [](LlmApiCacheControlType c) -> std::string {
        switch (c) {
        case LlmApiCacheControlType::Anthropic_: return "Anthropic";
        case LlmApiCacheControlType::None_:      return "None";
        default:                                 return "Auto";
        }
    };
    auto formatToStr = [](LlmApiFormat f) -> std::string {
        switch (f) {
        case LlmApiFormat::OpenAI_:     return "OpenAI";
        case LlmApiFormat::Anthropic_:  return "Anthropic";
        case LlmApiFormat::Gemini_:     return "Gemini";
        case LlmApiFormat::OpenRouter:  return "OpenRouter";
        case LlmApiFormat::Kimi:        return "Kimi";
        case LlmApiFormat::GLM:         return "GLM";
        case LlmApiFormat::Minimax:     return "Minimax";
        case LlmApiFormat::DeepSeek:    return "DeepSeek";
        default:                        return "Unknown";
        }
    };

    const auto& allApis = g_llmLib.GetApis();
    int providerCount = g_llmLib.GetProviderCount();

    json jProviders = json::array();
    for (int i = 0; i < providerCount; i++)
    {
        const LlmApiProvider* p = g_llmLib.GetProvider(i);
        if (!p || p->name.empty())
            continue;

        json jProvider;
        jProvider["name"]        = p->name;
        jProvider["endpoint"]    = p->endpoint;
        jProvider["key"]         = p->key;
        jProvider["type"]        = p->name;
        jProvider["format"]      = formatToStr(p->format);
        jProvider["isAvailable"] = p->IsAvailable();

        // 收集属于这个 provider 的所有 api
        json jApis = json::array();
        for (const auto& api : allApis)
        {
            if (api.providerTypeName != p->name)
                continue;

            json jApi;
            jApi["name"]             = api.name;
            jApi["model"]            = api.model;
            jApi["rule"]             = api.rule;
            jApi["maxToken"]         = api.maxToken;
            jApi["contextCapacity"]  = api.contextCapacity;
            jApi["priceInputToken"]  = api.priceInputToken;
            jApi["priceOutputToken"] = api.priceOutputToken;
            jApi["priceCacheRead"]   = api.priceCacheRead;
            jApi["priceCacheWrite"]  = api.priceCacheWrite;
            jApi["thinkingMode"]     = thinkingToStr(api.thinkingMode);
            jApi["cacheControl"]     = cacheToStr(api.cacheControlType);
            jApi["providerTypeName"] = api.providerTypeName;
            jApi["enable"]           = api.enable;

            jApi["role"] = roleToStr(api.role);

            json jTools = json::array();
            for (auto to : api.tools)
                jTools.push_back(toolToStr(to));
            jApi["tools"] = jTools;

            jApi["openRouterOptions"]["disableReasoning"] = api.openRouterOptions.disableReasoning;
            json jOnly = json::array();
            for (const auto& s : api.openRouterOptions.only)
                jOnly.push_back(s);
            jApi["openRouterOptions"]["only"] = jOnly;

            jApis.push_back(jApi);
        }
        jProvider["apis"] = jApis;
        jProviders.push_back(jProvider);
    }

    std::string utf8Json = jProviders.dump();
    _PostWebMessage(L"setProviderData", utf8_to_widechar(utf8Json));
}

// 更新Provider的API Key
void CChatSettingPage::UpdateProviderKey(const std::wstring& providerTypeStr, const std::wstring& key)
{
    try 
    {
        std::string providerTypeName = widechar_to_utf8(providerTypeStr.c_str());

        std::string keyUtf8 = widechar_to_utf8(key.c_str());

		bool needValidate = false;
		if (true)
		{
			const LlmApiProvider* provider = g_llmLib.GetProvider(providerTypeName);
			if (provider)
			{
				if (provider->status == LlmApiProvider::Status::Ok)
				{
					if (keyUtf8 != provider->key)
						needValidate = true;
				}
				else
				{
					if (!keyUtf8.empty())
						needValidate = true;
				}

				if (needValidate)
				{
					std::string apiName = g_llmLib.FindApiToValidateApiKey(providerTypeName);
					if (apiName.empty())
					{
						_needShowNoApiForValidation = true;
						needValidate = false;
					}
				}

			}
		}




		if (g_llmLib.SetProviderKey(providerTypeName, keyUtf8))
		{
			// 保存到注册表
			g_llmLib.SaveSettings();
		}

		if (needValidate)
			_taskMgr.AddTask_VerifyLlmApiProvider(providerTypeName);

		// Provider Key变更可能影响Cast Sheet的可用API列表
		SendCastSheetDataToWebView();

    }
    catch (...)
    {
        // 处理转换错误
    }
}


//====================== Provider验证方法实现 ======================

// 保存g_llmLib到llm.json
void CChatSettingPage::_SaveLlmJson()
{
    std::string dbFolder = Utils::GetDBRootFolder_utf8();
    std::string jsonPath = dbFolder + "\\llm.json";
    CLlmLibLoader::SaveJsonFile(g_llmLib, jsonPath.c_str());
}

// 更新Provider名称
void CChatSettingPage::UpdateProviderName(const std::wstring& oldNameW, const std::wstring& newNameW)
{
    std::string oldName = widechar_to_utf8(oldNameW.c_str());
    std::string newName = widechar_to_utf8(newNameW.c_str());
    if (g_llmLib.SetProviderName(oldName, newName))
        _SaveLlmJson();
}

// 更新Provider的endpoint
void CChatSettingPage::UpdateProviderEndpoint(const std::wstring& providerNameW, const std::wstring& endpointW)
{
    std::string providerName = widechar_to_utf8(providerNameW.c_str());
    std::string endpoint     = widechar_to_utf8(endpointW.c_str());
    if (g_llmLib.SetProviderEndpoint(providerName, endpoint))
        _SaveLlmJson();
}

// 更新Provider的format
void CChatSettingPage::UpdateProviderFormat(const std::wstring& providerNameW, const std::wstring& formatW)
{
    std::string providerName = widechar_to_utf8(providerNameW.c_str());
    std::string formatStr    = widechar_to_utf8(formatW.c_str());
    
    // 字符串转换为LlmApiFormat枚举
    LlmApiFormat format = LlmApiFormat::Unknown;
    if (formatStr == "OpenAI") format = LlmApiFormat::OpenAI_;
    else if (formatStr == "Anthropic") format = LlmApiFormat::Anthropic_;
    else if (formatStr == "Gemini") format = LlmApiFormat::Gemini_;
    else if (formatStr == "OpenRouter") format = LlmApiFormat::OpenRouter;
    else if (formatStr == "Kimi") format = LlmApiFormat::Kimi;
    else if (formatStr == "GLM") format = LlmApiFormat::GLM;
    else if (formatStr == "Minimax") format = LlmApiFormat::Minimax;
    else if (formatStr == "DeepSeek") format = LlmApiFormat::DeepSeek;
    
    if (g_llmLib.SetProviderFormat(providerName, format))
        _SaveLlmJson();
}

// 更新API名称
void CChatSettingPage::UpdateApiName(const std::wstring& oldNameW, const std::wstring& newNameW)
{
    std::string oldName = widechar_to_utf8(oldNameW.c_str());
    std::string newName = widechar_to_utf8(newNameW.c_str());
    if (g_llmLib.SetApiName(oldName, newName))
        _SaveLlmJson();
}

// 添加新 Provider
void CChatSettingPage::AddProvider(const std::wstring& nameW)
{
    std::string name = widechar_to_utf8(nameW.c_str());
    if (g_llmLib.AddProvider(name))
    {
        _SaveLlmJson();
        // 刷新页面显示
        LoadProviderData();
        SendProviderDataToWebView();
    }
}

// 删除 Provider
void CChatSettingPage::DeleteProvider(const std::wstring& nameW)
{
    std::string name = widechar_to_utf8(nameW.c_str());
    if (g_llmLib.DeleteProvider(name))
    {
        _SaveLlmJson();
        // 刷新页面显示（前端会自动从 expandedProviders 数组中移除已删除的 provider）
        LoadProviderData();
        SendProviderDataToWebView();
    }
}

// 添加新 API
void CChatSettingPage::AddApi(const std::wstring& providerNameW, const std::wstring& apiNameW)
{
    std::string providerName = widechar_to_utf8(providerNameW.c_str());
    std::string apiName = widechar_to_utf8(apiNameW.c_str());
    if (g_llmLib.AddApi(providerName, apiName))
    {
        _SaveLlmJson();
        // 刷新页面显示
        LoadProviderData();
        SendProviderDataToWebView();
    }
}

// 删除 API
void CChatSettingPage::DeleteApi(const std::wstring& nameW)
{
    std::string name = widechar_to_utf8(nameW.c_str());
    if (g_llmLib.DeleteApi(name))
    {
        _SaveLlmJson();
        // 刷新页面显示
        LoadProviderData();
        SendProviderDataToWebView();
    }
}

// 更新API单字段
void CChatSettingPage::UpdateApiField(const std::wstring& apiNameW, const std::wstring& fieldW, const nlohmann::json& value)
{
    using json = nlohmann::json;

    std::string apiName = widechar_to_utf8(apiNameW.c_str());
    std::string field   = widechar_to_utf8(fieldW.c_str());

    auto roleFromStr = [](const std::string& s) -> LlmApiRole {
        if (s == "Agent")     return LlmApiRole::Agent;
        if (s == "Auxiliary") return LlmApiRole::Auxiliary;
        if (s == "Embedding") return LlmApiRole::Embedding;
        return LlmApiRole::None;
    };
    auto toolFromStr = [](const std::string& s) -> LlmToolType {
        if (s == "ReplaceInFile")    return LlmToolType::ReplaceInFile;
        if (s == "FindSymbolDefine") return LlmToolType::FindSymbolDefine;
        if (s == "FindInFiles")      return LlmToolType::FindInFiles;
        if (s == "SearchFile")       return LlmToolType::SearchFile;
        if (s == "ReadFile")         return LlmToolType::ReadFile;
        if (s == "CLI_Cmd")          return LlmToolType::CLI_Cmd;
        if (s == "CLI_Bash")         return LlmToolType::CLI_Bash;
        if (s == "CLI_RunScript")    return LlmToolType::CLI_RunScript;
        if (s == "Question")         return LlmToolType::Question;
        if (s == "QueryFinish")      return LlmToolType::QueryFinish;
        if (s == "CreateSkill")      return LlmToolType::CreateSkill;
        return LlmToolType::None;
    };

    LlmApi* api = g_llmLib.GetApiMutable(apiName);
    if (!api)
        return;

    if (field == "model" && value.is_string())
        api->model = value.get<std::string>();
    else if (field == "rule" && value.is_string())
        api->rule = value.get<std::string>();
    else if (field == "maxToken" && value.is_number())
        api->maxToken = value.get<int>();
    else if (field == "contextCapacity" && value.is_number())
        api->contextCapacity = value.get<int>();
    else if (field == "priceInputToken" && value.is_number())
        api->priceInputToken = value.get<float>();
    else if (field == "priceOutputToken" && value.is_number())
        api->priceOutputToken = value.get<float>();
    else if (field == "priceCacheRead" && value.is_number())
        api->priceCacheRead = value.get<float>();
    else if (field == "priceCacheWrite" && value.is_number())
        api->priceCacheWrite = value.get<float>();
    else if (field == "thinkingMode" && value.is_string())
    {
        std::string v = value.get<std::string>();
        if (v == "Enable")       api->thinkingMode = LlmThinkingMode::Enable;
        else if (v == "Disable") api->thinkingMode = LlmThinkingMode::Disable;
        else                     api->thinkingMode = LlmThinkingMode::Auto;
    }
    else if (field == "cacheControl" && value.is_string())
    {
        std::string v = value.get<std::string>();
        if (v == "Anthropic")  api->cacheControlType = LlmApiCacheControlType::Anthropic_;
        else if (v == "None")  api->cacheControlType = LlmApiCacheControlType::None_;
        else                   api->cacheControlType = LlmApiCacheControlType::Auto;
    }
    else if (field == "role" && value.is_string())
    {
        std::string v = value.get<std::string>();
        api->role = roleFromStr(v);
    }
    else if (field == "tools" && value.is_array())
    {
        api->tools.clear();
        for (const auto& elem : value)
            if (elem.is_string())
                api->tools.push_back(toolFromStr(elem.get<std::string>()));
    }
    else if (field == "disableReasoning" && value.is_boolean())
        api->openRouterOptions.disableReasoning = value.get<bool>();
    else if (field == "openRouterOnly" && value.is_array())
    {
        api->openRouterOptions.only.clear();
        for (const auto& elem : value)
            if (elem.is_string())
                api->openRouterOptions.only.push_back(elem.get<std::string>());
    }
    else if (field == "enable" && value.is_boolean())
        api->enable = value.get<bool>();
    else
        return; // 未知字段，不保存

    _SaveLlmJson();
}

void CChatSettingPage::StartValidatingProvider(const LlmApiProviderTypeName& providerTypeName)
{
    if (!_IsReady())
        return;
    
    // 发送开始验证消息到WebView
    std::wstring jsonMessage = L"{\"action\":\"startValidatingProvider\",\"providerType\":\"" + utf8_to_widechar(providerTypeName.c_str()) + L"\"}";
    _PostWebMessage(L"", jsonMessage, true);
}

void CChatSettingPage::EndValidatingProvider(const LlmApiProviderTypeName& providerTypeName, bool available)
{
    if (!_IsReady())
        return;
    
    // 发送结束验证消息到WebView
    std::wstring jsonMessage = L"{\"action\":\"endValidatingProvider\",\"providerType\":\"" + utf8_to_widechar(providerTypeName.c_str()) + L"\",\"available\":" + (available ? L"true" : L"false") + L"}";
    _PostWebMessage(L"", jsonMessage, true);
    
    // 验证完成后重新发送Cast Sheet数据，因为provider状态可能已改变
    SendCastSheetDataToWebView();
}

void CChatSettingPage::SendCapabilityStatusToWebView()
{
    if (!_IsReady())
        return;
    
    extern CLlmLib g_llmLib;
    CLlmLib::WorkingCapability capability = g_llmLib.GetWorkingCapability();
    
    // 构建capability状态消息
    std::wstring capabilityJson = L"{";
    capabilityJson += L"\"action\":\"setCapabilityStatus\",";
    capabilityJson += L"\"capability\":" + std::to_wstring((int)capability);
    capabilityJson += L"}";
    
    // 发送capability状态到WebView
    _PostWebMessage(L"", capabilityJson, true); // 使用完整JSON格式
}

void CChatSettingPage::Update()
{
	_taskMgr.Update();
	UpdateReload();


	// 延迟显示消息框（因为webview回调中不能直接弹出MessageBox）
	if (_needShowNoApiForValidation)
	{
	_needShowNoApiForValidation = false;
		::MessageBox(GetSafeHwnd(), 
			_T("To validate API key, at least one API with a valid model name is required."), 
			_T("Validation Error"), 
			MB_OK | MB_ICONWARNING);
	}

	// 检测 evaluation task 完成状态
	if (_isEvaluatingSummarize && !_taskMgr.IsTaskTypeRunning("CompressSummarize"))
	{
		_isEvaluatingSummarize = false;

		// 发送消息隐藏 loading 动画
		_PostWebMessage(L"endEvaluateSummarize", L"");

		// 弹出日志文件
		extern std::string GetCompressSummarizeLogPath();
		std::string logPath = GetCompressSummarizeLogPath();
		std::wstring wLogPath = utf8_to_widechar(logPath);
		ShellExecuteW(NULL, L"open", wLogPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
}

bool CChatSettingPage::IsValidatingProvider()
{
	if (_taskMgr.IsTaskTypeRunning("VerifyLlmApiProvider"))
		return true;
	return false;
}

void CChatSettingPage::EvaluateCompressSummarize(const std::wstring& summarizeApiName)
{
	if (summarizeApiName.empty())
		return;

	// 从 parent 窗口获取 CChatDialogA
	CChatDialogA* pDialog = (CChatDialogA*)GetParent();
	if (!pDialog)
		return;

	// 获取 CChatOpsCtrl
	CChatOpsCtrl* pOpsCtrl = &pDialog->GetOpsCtrl();
	if (!pOpsCtrl)
		return;

	// 查找最近 3 个未 disable 的 session
	std::vector<int> sessionEnds = pOpsCtrl->FindLastNNotDisabledSessionEnds(3);
	if (sessionEnds.empty())
		return;

	// 发送消息让按钮显示 loading 动画
	_PostWebMessage(L"startEvaluateSummarize", L"");

	// 清空日志文件
	extern void ClearCompressSummarize();
	ClearCompressSummarize();

	// 标记正在评估
	_isEvaluatingSummarize = true;

	// 为每个 session 创建 evaluation task
	for (int sessionEndIndex : sessionEnds)
	{
		// 估算 token 数
		int nTokens = pOpsCtrl->EstimateUncompressedSessionAIContentToken(sessionEndIndex, CChatOpsCompress::GetSessionSummarizeToolTypes());
		int originalTokenCount = static_cast<int>(nTokens * CTokenCalibrate::GetCalibrationFactor());

		// 添加 evaluation task
		std::string apiNameUtf8 = widechar_to_utf8(summarizeApiName.c_str());
		_taskMgr.AddTask_CompressSummarize(sessionEndIndex, apiNameUtf8, originalTokenCount, true);
	}
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
		// 版本号变化，更新显示内容
		SendProviderDataToWebView();
		SendCapabilityStatusToWebView();
		SendCastSheetDataToWebView();
	}
}

