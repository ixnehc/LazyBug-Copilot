#include "stdh.h"
#include "ChatSettingPage.h"
#include <fstream>
#include <algorithm>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>

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

// 设置编辑扩展模型回调
void CChatSettingPage::SetEditModelsCallback(SettingPageEditModelsCallback callback)
{
    _editModelsCallback = callback;
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
    providersTab.title = L"Providers";
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
            // 退出设置页面
            if (_exitCallback)
            {
                _exitCallback();
            }
        }
        else if (action == "editModels")
        {
            // 编辑扩展模型
            if (_editModelsCallback)
            {
                _editModelsCallback();
            }
        }
        else if (action == "openSettingHelp")
        {
            std::string htmlPath = GetCurModuleFolderPath_utf8();
            htmlPath += "\\llm_setting_intro.html";
            
            ShellExecuteW(NULL, L"open", utf8_to_widechar(htmlPath.c_str()).c_str(), NULL, NULL, SW_SHOWNORMAL);
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

// 发送Provider数据到WebView
void CChatSettingPage::SendProviderDataToWebView()
{
    if (!_IsReady())
        return;
    
    // 构建Provider数据JSON
    std::wstring providersJson = L"[";
    
    int providerCount = g_llmLib.GetProviderCount();
    bool first = true;
    
    for (int i = 0; i < providerCount; i++)
    {
        const LlmApiProvider* provider = g_llmLib.GetProvider(i);
        if (provider && !provider->name.empty())
        {
            if (!first) providersJson += L",";
            first = false;
            
            // 转义字符串
            std::wstring safeName = _EscapeJsonString(utf8_to_widechar(provider->name));
            std::wstring safeKey = _EscapeJsonString(utf8_to_widechar(provider->key));
            
            providersJson += L"{";
            providersJson += L"\"name\":\"" + safeName + L"\",";
            providersJson += L"\"key\":\"" + safeKey + L"\",";
            providersJson += L"\"type\":\"" + safeName + L"\",";
			providersJson += L"\"isAvailable\":";
			providersJson += (provider->IsAvailable() ? L"true" : L"false");
            providersJson += L"}";
        }
    }
    
    providersJson += L"]";
    
    // 发送Provider数据到WebView
    _PostWebMessage(L"setProviderData", providersJson);
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
			}
		}

		if (g_llmLib.SetProviderKey(providerTypeName, keyUtf8))
		{
			// 保存到注册表
			g_llmLib.SaveSettings();
		}

		if (needValidate)
			_taskMgr.AddTask_VerifyLlmApiProvider(providerTypeName);


    }
    catch (...)
    {
        // 处理转换错误
    }
}

//====================== Provider验证方法实现 ======================

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
    
    // 验证完成后重新发送capability状态，因为provider状态可能已改变
    SendCapabilityStatusToWebView();
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
}

bool CChatSettingPage::IsValidatingProvider()
{
	if (_taskMgr.IsTaskTypeRunning("VerifyLlmApiProvider"))
		return true;
	return false;
}

//====================== 编辑扩展模型配置方法实现 ======================

void CChatSettingPage::EditModels()
{
	// 获取llm.ini文件的完整路径
	std::string path = Utils::GetDBRootFolder_utf8();
	path += "\\llm.ini";
	
	// 使用ShellExecute打开文件（使用默认编辑器）
	ShellExecuteW(NULL, L"open", utf8_to_widechar(path).c_str(), NULL, NULL, SW_SHOWNORMAL);
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
	}
}

