#include "stdh.h"
#include "ChatSettingMenu.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>
#include "Utils.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern const char* GetCurModuleFolderPath_utf8();

//////////////////////////////////////////////////////////////////////////
// CChatSettingMenu

BEGIN_MESSAGE_MAP(CChatSettingMenu, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_ACTIVATE()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CChatSettingMenu::CChatSettingMenu()
    : _isWebViewCreated(false)
    , _isUIInitialized(false)
    , _callbackId(0)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
    , _windowWidth(200)
    , _windowHeight(255)
    , _currentProcessId(0)
{
    _currentProcessId = GetCurrentProcessId();
}

CChatSettingMenu::~CChatSettingMenu()
{
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

    SAFE_RELEASE(_webView);
    SAFE_RELEASE(_controller);
    SAFE_RELEASE(_webViewEnvironment);
}

BOOL CChatSettingMenu::CreateSettingMenuWindow(CWnd* pParent)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL, // 设置背景画刷为NULL，防止系统擦除背景
        NULL);

    // 创建弹出式窗口
    return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        className, _T("SettingMenu"),
        WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
        0, 0, _windowWidth, _windowHeight, pParent->GetSafeHwnd(), NULL);
}

HRESULT CChatSettingMenu::InitializeWebView()
{
    extern const wchar_t* GetWebViewUserFolder();
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, Utils::GetWebViewUserFolder(), nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
        if (SUCCEEDED(result))
        {
            _webViewEnvironment = environment;
            if (_webViewEnvironment) _webViewEnvironment->AddRef();

            return _webViewEnvironment->CreateCoreWebView2Controller(GetSafeHwnd(),
                Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                if (SUCCEEDED(result))
                {
                    _controller = controller;
                    if (_controller) _controller->AddRef();

                    _controller->put_IsVisible(TRUE);

                    _controller->get_CoreWebView2(&_webView);

                    RECT bounds;
                    GetClientRect(&bounds);
                    _controller->put_Bounds(bounds);

#ifdef DISABLE_WEBVIEW_CONTEXTMENU
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

                        if (success && !_isUIInitialized)
                        {
                            InitializeUI();
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
                    htmlPath += "\\ChatSettingMenu.html";
                    Navigate(utf8_to_widechar(htmlPath.c_str()));

                    _isWebViewCreated = true;
                }
                return S_OK;
            }).Get());
        }
        return S_OK;
    }).Get());

    return hr;
}

void CChatSettingMenu::Navigate(const std::wstring& url)
{
    if (_webView != nullptr)
    {
        _webView->Navigate(url.c_str());
    }
}

void CChatSettingMenu::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
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

void CChatSettingMenu::SetNavigationCompletedCallback(SettingMenuNavigationCompletedCallback callback)
{
    _navigationCompletedCallback = callback;
}

void CChatSettingMenu::SetWebMessageReceivedCallback(SettingMenuMessageReceivedCallback callback)
{
    _webMessageReceivedCallback = callback;
}

void CChatSettingMenu::ResizeWebView()
{
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

void CChatSettingMenu::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    ResizeWebView();
}

int CChatSettingMenu::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!_isWebViewCreated)
    {
        HRESULT hr = InitializeWebView();
        if (FAILED(hr))
        {
            TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
            return 0;
        }
    }

    return 0;
}

void CChatSettingMenu::OnDestroy()
{
    CWnd::OnDestroy();

    if (_controller != nullptr)
    {
        _controller->Close();
    }
}

void CChatSettingMenu::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CWnd::OnActivate(nState, pWndOther, bMinimized);

    // 当窗口失去激活状态时，隐藏窗口
    if (nState == WA_INACTIVE)
    {
        HideWindow();
    }
}

void CChatSettingMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // 按ESC键关闭窗口
    if (nChar == VK_ESCAPE)
    {
        HideWindow();
        return;
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

//====================== 弹出窗口显示/隐藏 ======================

void CChatSettingMenu::ShowWindow(const RECT& btnRect)
{
    // 如果WebView未创建，先创建
    if (!_isWebViewCreated)
    {
        HRESULT hr = InitializeWebView();
        if (FAILED(hr))
        {
            TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
            return;
        }
    }

    // 计算窗口位置和大小
    CRect windowRect = CalculateWindowRect(btnRect);

    // 设置窗口位置和大小，显示并激活窗口
    SetWindowPos(&CWnd::wndTopMost, windowRect.left, windowRect.top,
        windowRect.Width(), windowRect.Height(),
        SWP_SHOWWINDOW);

    // 调整WebView大小
    ResizeWebView();
}

void CChatSettingMenu::HideWindow()
{
    if (IsWindowVisible())
    {
        CWnd::ShowWindow(SW_HIDE);
    }
}

CRect CChatSettingMenu::CalculateWindowRect(const RECT& btnRect)
{
    // 使用固定尺寸
    int width = _windowWidth;
    int height = _windowHeight;

    // 获取屏幕工作区
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    // 计算窗口位置：默认显示在按钮下方
    int x = btnRect.left;
    int y = btnRect.bottom + 2;

    // 如果下方空间不够，显示在按钮上方
    if (y + height > workArea.bottom)
    {
        y = btnRect.top - height - 2;
    }

    // 调整X位置，确保窗口不超出屏幕右侧
    if (x + width > workArea.right)
    {
        x = workArea.right - width;
    }
    if (x < workArea.left)
    {
        x = workArea.left;
    }

    // 调整Y位置，确保窗口不超出屏幕顶部
    if (y < workArea.top)
    {
        y = workArea.top;
    }

    return CRect(x, y, x + width, y + height);
}

void CChatSettingMenu::CheckForegroundWindow()
{
    // 获取前台窗口
    HWND foregroundWnd = ::GetForegroundWindow();
    if (foregroundWnd == NULL)
        return;

    // 获取前台窗口的进程ID
    DWORD foregroundProcessId = 0;
    GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);

    // 如果前台窗口不属于当前进程，隐藏窗口
    if (foregroundProcessId != _currentProcessId)
    {
        HideWindow();
    }
}

void CChatSettingMenu::Update()
{
    if (!IsWindowVisible())
        return;

    CheckForegroundWindow();
}

//====================== UI初始化 ======================

void CChatSettingMenu::InitializeUI()
{
    if (_isUIInitialized)
        return;

    _isUIInitialized = true;

    _PostWebMessage(L"initializeComplete", L"");
}

// 设置菜单项启用/禁用状态
void CChatSettingMenu::SetItemEnabled(const std::wstring& itemName, bool enabled)
{
    if (!_IsReady())
        return;

    std::wstring jsonData = L"{\"itemName\":\"" + itemName + L"\",\"enabled\":" + (enabled ? L"true" : L"false") + L"}";
    _PostWebMessage(L"setItemEnabled", jsonData);
}

//====================== 内部辅助方法 ======================

bool CChatSettingMenu::_IsReady() const
{
    return _isWebViewCreated && _isUIInitialized;
}

void CChatSettingMenu::_PostWebMessage(const std::wstring& action, const std::wstring& data)
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

void CChatSettingMenu::_HandleWebMessage(const std::wstring& message)
{
    std::string utf8Message = widechar_to_utf8(message.c_str());

    try {
        nlohmann::json jsonMsg = nlohmann::json::parse(utf8Message);

        if (!jsonMsg.contains("action") || !jsonMsg["action"].is_string())
        {
            return;
        }

        std::string action = jsonMsg["action"];

        if (action == "itemClicked")
        {
            // 处理菜单项点击事件
            if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
            {
                auto& data = jsonMsg["data"];
                if (data.contains("itemName") && data["itemName"].is_string())
                {
                    std::string itemName = data["itemName"];
                    // 回调通知选中的菜单项
                    if (_itemSelectedCallback)
                    {
                        _itemSelectedCallback(utf8_to_widechar(itemName));
                    }
                    // 隐藏窗口
                    HideWindow();
                }
            }
        }
        else if (action == "escapePressed")
        {
            // 处理ESC键按下，关闭窗口
            HideWindow();
        }
    }
    catch (const std::exception& e)
    {
        TRACE(_T("Failed to parse WebView message: %s\n"), utf8_to_widechar(e.what()).c_str());
    }
}

