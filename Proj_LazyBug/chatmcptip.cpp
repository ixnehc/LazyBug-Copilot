#include "stdh.h"
#include "ChatMcpTip.h"
#include <nlohmann/json.hpp>
#include "Utils.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern const char* GetCurModuleFolderPath_utf8();

//////////////////////////////////////////////////////////////////////////
// CChatMcpTip

BEGIN_MESSAGE_MAP(CChatMcpTip, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_ACTIVATE()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
END_MESSAGE_MAP()

CChatMcpTip::CChatMcpTip()
    : _isWebViewCreated(false)
    , _isUIInitialized(false)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
, _windowWidth(600)
    , _windowHeight(375)
    , _minWindowWidth(300)
    , _minWindowHeight(120)
    , _maxWindowWidth(800)
    , _maxWindowHeight(700)
    , _isContentSized(false)
    , _currentProcessId(0)
{
    _currentProcessId = GetCurrentProcessId();
}

CChatMcpTip::~CChatMcpTip()
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

BOOL CChatMcpTip::CreateMcpTipWindow(CWnd* pParent)
{
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL);

    return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        className, _T("McpTip"),
        WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
        0, 0, _windowWidth, _windowHeight, pParent->GetSafeHwnd(), NULL);
}

HRESULT CChatMcpTip::InitializeWebView()
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

                    _webView->add_NavigationCompleted(
                        Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                            [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                        BOOL success = FALSE;
                        args->get_IsSuccess(&success);

                        if (success && !_isUIInitialized)
                        {
                            _isUIInitialized = true;
                            if (!_currentToolMd.empty())
                            {
                                // 仅发送内容，窗口显示交由contentSize回传(或兜底定时器)处理
                                _SendToolContent();
                            }
                        }

                        return S_OK;
                    }).Get(),
                        &_navigationCompletedToken);

                    // 监听来自页面的消息（内容尺寸回传）
                    _webView->add_WebMessageReceived(
                        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                            [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                        LPWSTR rawJson = nullptr;
                        if (SUCCEEDED(args->get_WebMessageAsJson(&rawJson)) && rawJson != nullptr)
                        {
                            try
                            {
                                std::string jsonStr = widechar_to_utf8(rawJson);
                                nlohmann::json msg = nlohmann::json::parse(jsonStr);
                                // 页面通过postMessage(JSON.stringify(...))发送时，
                                // get_WebMessageAsJson得到的是被编码的字符串，需再次解析
                                if (msg.is_string())
                                {
                                    msg = nlohmann::json::parse(msg.get<std::string>());
                                }
                                std::string action = msg.value("action", "");
                                if (action == "contentSize" && msg.contains("data"))
                                {
                                    auto& data = msg["data"];
                                    int w = data.value("width", 0);
                                    int h = data.value("height", 0);
                                    _OnContentSize(w, h);
                                }
                                else if (action == "escapePressed")
                                {
                                    HideTip();
                                }
                            }
                            catch (...)
                            {
                            }
                            CoTaskMemFree(rawJson);
                        }
                        return S_OK;
                    }).Get(),
                        &_webMessageReceivedToken);


                    std::string htmlPath = GetCurModuleFolderPath_utf8();
                    htmlPath += "\\ChatMcpTip.html";
                    _webView->Navigate(utf8_to_widechar(htmlPath.c_str()).c_str());

                    _isWebViewCreated = true;
                }
                return S_OK;
            }).Get());
        }
        return S_OK;
    }).Get());

    return hr;
}

void CChatMcpTip::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

int CChatMcpTip::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    return 0;
}

void CChatMcpTip::OnDestroy()
{
    if (_sizeStableTimerId != 0)
    {
        KillTimer(_sizeStableTimerId);
        _sizeStableTimerId = 0;
    }

    CWnd::OnDestroy();
    if (_controller != nullptr)
    {
        _controller->Close();
    }
}

void CChatMcpTip::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CWnd::OnActivate(nState, pWndOther, bMinimized);
    if (nState == WA_INACTIVE)
    {
        HideTip();
    }
}

void CChatMcpTip::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_ESCAPE)
    {
        HideTip();
        return;
    }
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChatMcpTip::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1 && _showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
        _DoShowWindow();
    }
    else if (nIDEvent == 2 && _sizeStableTimerId != 0)
    {
        KillTimer(_sizeStableTimerId);
        _sizeStableTimerId = 0;
        // 尺寸已稳定300ms，显示窗口
        SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
            _pendingWindowRect.Width(), _pendingWindowRect.Height(),
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
        if (_controller != nullptr)
        {
            RECT bounds;
            GetClientRect(&bounds);
            _controller->put_Bounds(bounds);
        }
    }
    CWnd::OnTimer(nIDEvent);
}

void CChatMcpTip::_DoShowWindow()
{
    // 兜底显示：仅在窗口尚未可见、contentSize未回传、且尺寸稳定定时器未激活时才执行
    if (IsWindowVisible() || _isContentSized || _sizeStableTimerId != 0)
        return;

    SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
        _pendingWindowRect.Width(), _pendingWindowRect.Height(),
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

//====================== 显示/隐藏 ======================

void CChatMcpTip::ShowTip(const RECT& anchorRect, const std::string& toolMd)
{
    _currentToolMd = toolMd;
    _currentAnchorRect = anchorRect;
    _isContentSized = false;

    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    // 先按默认尺寸隐藏定位，等内容尺寸回传后再调整并显示
    _pendingWindowRect = CalculateWindowRect(anchorRect, _windowWidth, _windowHeight);

    if (!_isWebViewCreated)
    {
        SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
            _pendingWindowRect.Width(), _pendingWindowRect.Height(),
            SWP_NOACTIVATE | SWP_HIDEWINDOW);

        HRESULT hr = InitializeWebView();
        if (FAILED(hr))
        {
            TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
            return;
        }
    }
    else
    {
        SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
            _pendingWindowRect.Width(), _pendingWindowRect.Height(),
            SWP_NOACTIVATE | SWP_HIDEWINDOW);

        if (_controller != nullptr)
        {
            RECT bounds;
            GetClientRect(&bounds);
            _controller->put_Bounds(bounds);
        }

        if (_isUIInitialized)
        {
            _SendToolContent();
        }
    }

    // 兜底定时器：若contentSize长时间未回传仍能显示
    _showTimerId = SetTimer(1, 2000, nullptr);
}

void CChatMcpTip::HideTip()
{
    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    if (_sizeStableTimerId != 0)
    {
        KillTimer(_sizeStableTimerId);
        _sizeStableTimerId = 0;
    }

    if (IsWindowVisible())
    {
        CWnd::ShowWindow(SW_HIDE);
    }
}

CRect CChatMcpTip::CalculateWindowRect(const RECT& anchorRect, int width, int height)
{
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    int x = anchorRect.right + 4;
    int y = anchorRect.top;

    if (x + width > workArea.right)
    {
        x = anchorRect.left - width - 4;
    }

    if (y + height > workArea.bottom)
    {
        y = workArea.bottom - height;
    }
    if (y < workArea.top)
    {
        y = workArea.top;
    }

    if (x < workArea.left)
    {
        x = workArea.left;
    }

    return CRect(x, y, x + width, y + height);
}

void CChatMcpTip::_OnContentSize(int contentWidth, int contentHeight)
{
    // 窗口边框/客户区差值（border等），用窗口与客户区尺寸差补偿
    CRect wndRect, clientRect;
    GetWindowRect(&wndRect);
    GetClientRect(&clientRect);
    int frameW = wndRect.Width() - clientRect.Width();
    int frameH = wndRect.Height() - clientRect.Height();

    int width = contentWidth + frameW;
    int height = contentHeight + frameH;

    // 在最大/最小范围内约束
    if (width < _minWindowWidth) width = _minWindowWidth;
    if (width > _maxWindowWidth) width = _maxWindowWidth;
    if (height < _minWindowHeight) height = _minWindowHeight;
    if (height > _maxWindowHeight) height = _maxWindowHeight;

    _isContentSized = true;

    // 取消兜底定时器（contentSize已开始回传，不再需要超时保底）
    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    // 按新尺寸重新计算位置（不显示，仅调整大小和位置）
    _pendingWindowRect = CalculateWindowRect(_currentAnchorRect, width, height);
    SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
        _pendingWindowRect.Width(), _pendingWindowRect.Height(),
        SWP_NOACTIVATE | SWP_HIDEWINDOW);

    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }

    // 重置尺寸稳定定时器：300ms内无新contentSize才显示窗口
    if (_sizeStableTimerId != 0)
    {
        KillTimer(_sizeStableTimerId);
    }
    _sizeStableTimerId = SetTimer(2, 300, nullptr);
}



void CChatMcpTip::CheckForegroundWindow()
{
    HWND foregroundWnd = ::GetForegroundWindow();
    if (foregroundWnd == NULL)
        return;

    DWORD foregroundProcessId = 0;
    GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);

    if (foregroundProcessId != _currentProcessId)
    {
        HideTip();
    }
}

void CChatMcpTip::Update()
{
    if (!IsWindowVisible())
        return;

    CheckForegroundWindow();
}

//====================== 内部辅助方法 ======================

bool CChatMcpTip::_IsReady() const
{
    return _isWebViewCreated && _isUIInitialized;
}

void CChatMcpTip::_PostWebMessage(const std::wstring& action, const std::wstring& data)
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

void CChatMcpTip::_SendToolContent()
{
    if (!_IsReady() || _currentToolMd.empty())
        return;

    // 计算客户区可用宽度范围（窗口范围扣除边框），供HTML测量时约束宽度
    CRect wndRect, clientRect;
    GetWindowRect(&wndRect);
    GetClientRect(&clientRect);
    int frameW = wndRect.Width() - clientRect.Width();
    int maxContentWidth = _maxWindowWidth - frameW;
    int minContentWidth = _minWindowWidth - frameW;
    if (maxContentWidth < 1) maxContentWidth = _maxWindowWidth;
    if (minContentWidth < 1) minContentWidth = 1;

    nlohmann::json jsonData;
    jsonData["content"] = _currentToolMd;
    jsonData["maxContentWidth"] = maxContentWidth;
    jsonData["minContentWidth"] = minContentWidth;

    try
    {
        std::string jsonStr = jsonData.dump();
        std::wstring wJsonStr = utf8_to_widechar(jsonStr);
        _PostWebMessage(L"setToolContent", wJsonStr);
    }
    catch (...)
    {
    }
}
