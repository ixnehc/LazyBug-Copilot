#include "stdh.h"
#include "ChatSkillTip.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Utils.h"
#include "llmskills.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern const char* GetCurModuleFolderPath_utf8();
extern const wchar_t* GetWebViewUserFolder();

//////////////////////////////////////////////////////////////////////////
// CChatSkillTip

BEGIN_MESSAGE_MAP(CChatSkillTip, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_ACTIVATE()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
END_MESSAGE_MAP()

CChatSkillTip::CChatSkillTip()
    : _isWebViewCreated(false)
    , _isUIInitialized(false)
    , _callbackId(0)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
    , _windowWidth(400)
    , _windowHeight(375)  // 原300，增加1/4
    , _currentProcessId(0)
{
    _currentProcessId = GetCurrentProcessId();
}

CChatSkillTip::~CChatSkillTip()
{
    if (_webView != nullptr && _navigationCompletedToken.value != 0)
    {
        _webView->remove_NavigationCompleted(_navigationCompletedToken);
        _navigationCompletedToken.value = 0;
    }

    SAFE_RELEASE(_webView);
    SAFE_RELEASE(_controller);
    SAFE_RELEASE(_webViewEnvironment);
}

BOOL CChatSkillTip::CreateSkillTipWindow(CWnd* pParent)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL);

    // 创建弹出式窗口，WS_EX_NOACTIVATE 使窗口不获得焦点
    return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        className, _T("SkillTip"),
        WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
        0, 0, _windowWidth, _windowHeight, pParent->GetSafeHwnd(), NULL);
}

HRESULT CChatSkillTip::InitializeWebView()
{
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

                        if (success && !_isUIInitialized)
                        {
                            _isUIInitialized = true;
                            // 如果有待显示的内容，发送并显示窗口
                            if (!_currentSkillMdPath.empty())
                            {
                                _SendSkillContent();
                                // 首次初始化完成后显示窗口
                                ShowWindow(SW_SHOWNOACTIVATE);
                            }
                        }

                        return S_OK;
                    }).Get(),
                        &_navigationCompletedToken);

                    std::string htmlPath = GetCurModuleFolderPath_utf8();
                    htmlPath += "\\ChatSkillTip.html";
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

void CChatSkillTip::Navigate(const std::wstring& url)
{
    if (_webView != nullptr)
    {
        _webView->Navigate(url.c_str());
    }
}

void CChatSkillTip::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
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

void CChatSkillTip::ResizeWebView()
{
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

void CChatSkillTip::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    ResizeWebView();
}

int CChatSkillTip::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // WebView在ShowTip时按需初始化，而不是在OnCreate中
    // 这样可以避免创建窗口时就初始化WebView，减少资源占用

    return 0;
}

void CChatSkillTip::OnDestroy()
{
    CWnd::OnDestroy();

    if (_controller != nullptr)
    {
        _controller->Close();
    }
}

void CChatSkillTip::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CWnd::OnActivate(nState, pWndOther, bMinimized);

    // 当窗口失去激活状态时，隐藏窗口
    if (nState == WA_INACTIVE)
    {
        HideTip();
    }
}

void CChatSkillTip::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // 按ESC键关闭窗口
    if (nChar == VK_ESCAPE)
    {
        HideTip();
        return;
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChatSkillTip::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1 && _showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
        _DoShowWindow();
    }
    CWnd::OnTimer(nIDEvent);
}

void CChatSkillTip::_DoShowWindow()
{
    // 显示窗口
    SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
        _pendingWindowRect.Width(), _pendingWindowRect.Height(),
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

//====================== 显示/隐藏 ======================

void CChatSkillTip::ShowTip(const RECT& anchorRect, const std::wstring& skillMdPath)
{
    // 保存当前skill路径
    _currentSkillMdPath = skillMdPath;

    // 取消之前的显示定时器
    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    // 计算窗口位置和大小（固定大小，只计算位置）
    _pendingWindowRect = CalculateWindowRect(anchorRect);

    // 如果WebView未创建，先创建（首次显示时）
    if (!_isWebViewCreated)
    {
        // 先设置窗口位置（不显示），这样WebView创建时就有正确的大小
        SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
            _pendingWindowRect.Width(), _pendingWindowRect.Height(),
            SWP_NOACTIVATE | SWP_HIDEWINDOW);

        HRESULT hr = InitializeWebView();
        if (FAILED(hr))
        {
            TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
            return;
        }
        // WebView初始化是异步的，内容会在导航完成后通过_NavigationCompleted发送
    }
    else
    {
        // WebView已创建，先更新内容（不显示窗口）
        SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
            _pendingWindowRect.Width(), _pendingWindowRect.Height(),
            SWP_NOACTIVATE | SWP_HIDEWINDOW);

        // 调整WebView大小
        ResizeWebView();

        // 发送新内容到WebView
        if (_isUIInitialized)
        {
            _SendSkillContent();
        }
    }

    // 启动延迟显示定时器（500ms后显示窗口，避免内容闪烁）
    _showTimerId = SetTimer(1, 300, nullptr);
}

void CChatSkillTip::HideTip()
{
    // 取消显示定时器
    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    if (IsWindowVisible())
    {
        CWnd::ShowWindow(SW_HIDE);
    }
}

CRect CChatSkillTip::CalculateWindowRect(const RECT& anchorRect)
{
    // 使用固定尺寸
    int width = _windowWidth;
    int height = _windowHeight;

    // 获取屏幕工作区
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    // 计算窗口位置：默认显示在锚点右侧
    int x = anchorRect.right + 4;
    int y = anchorRect.top;

    // 如果右侧空间不够，显示在左侧
    if (x + width > workArea.right)
    {
        x = anchorRect.left - width - 4;
    }

    // 调整Y位置，确保窗口不超出屏幕底部
    if (y + height > workArea.bottom)
    {
        y = workArea.bottom - height;
    }
    if (y < workArea.top)
    {
        y = workArea.top;
    }

    // 确保不超出屏幕左侧
    if (x < workArea.left)
    {
        x = workArea.left;
    }

    return CRect(x, y, x + width, y + height);
}

void CChatSkillTip::CheckForegroundWindow()
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
        HideTip();
    }
}

void CChatSkillTip::Update()
{
    if (!IsWindowVisible())
        return;

    CheckForegroundWindow();
}

//====================== 内部辅助方法 ======================

bool CChatSkillTip::_IsReady() const
{
    return _isWebViewCreated && _isUIInitialized;
}

void CChatSkillTip::_PostWebMessage(const std::wstring& action, const std::wstring& data)
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

void CChatSkillTip::_SendSkillContent()
{
    if (!_IsReady() || _currentSkillMdPath.empty())
        return;

    std::string name, description, content;
    if (ParseSkillMd(widechar_to_utf8(_currentSkillMdPath.c_str()), name, description, &content))
    {
        // 构建JSON数据
        nlohmann::json jsonData;
        jsonData["name"] = name;
        jsonData["description"] = description;
        jsonData["content"] = content;

        try
        {
            std::string jsonStr = jsonData.dump();
            std::wstring wJsonStr = utf8_to_widechar(jsonStr);
            _PostWebMessage(L"setSkillContent", wJsonStr);
        }
        catch (const std::exception& e)
        {
            // dump失败，忽略
			int v = 0;
			v++;
        }
    }
}

