#include "stdh.h"
#include "InputHintWindow.h"
#include "chatinput.h"
#include <nlohmann/json.hpp>
#include "Utils.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern const char* GetCurModuleFolderPath_utf8();

//////////////////////////////////////////////////////////////////////////
// CInputHintWindow

BEGIN_MESSAGE_MAP(CInputHintWindow, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_ACTIVATE()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
END_MESSAGE_MAP()

CInputHintWindow::CInputHintWindow()
    : _isWebViewCreated(false)
    , _isUIInitialized(false)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
    , _windowWidth(400)
    , _windowHeight(120)
    , _minWindowWidth(120)
    , _minWindowHeight(40)
    , _maxWindowWidth(800)
    , _maxWindowHeight(600)
    , _isContentSized(false)
    , _hasContent(false)
    , _currentProcessId(0)
{
    _currentProcessId = GetCurrentProcessId();
}

CInputHintWindow::~CInputHintWindow()
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

BOOL CInputHintWindow::CreateHintWindow(CWnd* pParent)
{
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL);

    return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        className, _T("InputHint"),
        WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
        0, 0, _windowWidth, _windowHeight, pParent->GetSafeHwnd(), NULL);
}

HRESULT CInputHintWindow::InitializeWebView()
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
                            if (_hasContent)
                            {
                                // 仅发送内容, 窗口显示交由contentSize回传(或兜底定时器)处理
                                _SendContent();
                            }
                        }

                        return S_OK;
                    }).Get(),
                        &_navigationCompletedToken);

                    // 监听来自页面的消息(内容尺寸回传)
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
                                    HideHint();
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
                    htmlPath += "\\InputHintWindow.html";
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

void CInputHintWindow::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

int CInputHintWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    return 0;
}

void CInputHintWindow::OnDestroy()
{
    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    CWnd::OnDestroy();
    if (_controller != nullptr)
    {
        _controller->Close();
    }
}

void CInputHintWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CWnd::OnActivate(nState, pWndOther, bMinimized);
    if (nState == WA_INACTIVE)
    {
        HideHint();
    }
}

void CInputHintWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_ESCAPE)
    {
        HideHint();
        return;
    }
    if (nChar == VK_TAB)
    {
        ApplyHint();
        return;
    }
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CInputHintWindow::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1 && _showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
        _DoShowWindow();
    }
    CWnd::OnTimer(nIDEvent);
}

void CInputHintWindow::_DoShowWindow()
{
    // 兜底显示: 仅在窗口尚未可见且contentSize未回传时才执行
    if (IsWindowVisible() || _isContentSized || _currentContent.plainContent.empty())
        return;

    SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
        _pendingWindowRect.Width(), _pendingWindowRect.Height(),
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

//====================== 显示/隐藏 ======================

void CInputHintWindow::ShowHint(const RECT& anchorRect, const Utils::DiffedInputContent& newDiff, const Utils::DiffedInputContent& oldDiff, const Utils::InputContent& newFullContent, int applyCaretTokenPos)
{
    if (newDiff.plainContent.empty() && std::find(oldDiff.diffStates.begin(), oldDiff.diffStates.end(), 2) == oldDiff.diffStates.end())
        return;

    _currentContent = newDiff;
    _newFullContent = newFullContent;
    _applyCaretTokenPos = applyCaretTokenPos;
    _hasContent = true;
    _currentAnchorRect = anchorRect;
    _isContentSized = false;

    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    // 先按默认尺寸隐藏定位, 等内容尺寸回传后再调整并显示
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
            _SendContent();
        }
    }

    // 兜底定时器: 若contentSize长时间未回传仍能显示
    _showTimerId = SetTimer(1, 2000, nullptr);

    // 根据 oldDiff 设置 CChatInput 的删除标记（红色背景）
    if (_pChatInput && !oldDiff.plainContent.empty())
    {
        std::vector<int> deletionIndices;
        int tokenIdx = 0;
        size_t pos = 0;

        for (const auto& seg : oldDiff.tagSegments)
        {
            // 该 tag 之前的普通字符
            while (pos < seg.startPos && pos < oldDiff.plainContent.size())
            {
                if (oldDiff.diffStates[pos] == 2)
                    deletionIndices.push_back(tokenIdx);
                tokenIdx++;
                pos++;
            }

            // tag 本身（1 个 token）：全部字符均为删除才标记
            bool allDeleted = true;
            for (size_t i = seg.startPos; i < seg.endPos && i < oldDiff.plainContent.size(); i++)
            {
                if (oldDiff.diffStates[i] != 2)
                {
                    allDeleted = false;
                    break;
                }
            }
            if (allDeleted)
                deletionIndices.push_back(tokenIdx);
            tokenIdx++;
            pos = seg.endPos;
        }

        // 最后一个 tag 之后的剩余字符
        while (pos < oldDiff.plainContent.size())
        {
            if (oldDiff.diffStates[pos] == 2)
                deletionIndices.push_back(tokenIdx);
            tokenIdx++;
            pos++;
        }

        _pChatInput->SetDeletionMarks(deletionIndices);
    }

    // 通知 CChatInput JS 层 hint 窗口可见，以启用 Tab 键拦截
    if (_pChatInput)
    {
        _pChatInput->SetHintVisible(true);
    }
}

void CInputHintWindow::HideHint()
{
    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    if (IsWindowVisible())
    {
        CWnd::ShowWindow(SW_HIDE);
    }

    // 同步清除 CChatInput 的删除标记
    if (_pChatInput)
    {
        _pChatInput->ClearDeletionMarks();
        _pChatInput->SetHintVisible(false);
    }
}

void CInputHintWindow::ApplyHint()
{
    if (!_hasContent || !_pChatInput)
        return;

    // 从完整的 _newFullContent 重建 JSON（而非仅含中间变化区的 _currentContent）
    std::wstring fullContent = Utils::BuildFullContent(_newFullContent);
    _pChatInput->SetInputContent_(fullContent, _applyCaretTokenPos);

    HideHint();
}

CRect CInputHintWindow::CalculateWindowRect(const RECT& anchorRect, int width, int height)
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

void CInputHintWindow::_OnContentSize(int contentWidth, int contentHeight)
{
    // 窗口边框/客户区差值(border等), 用窗口与客户区尺寸差补偿
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

    // 取消兜底定时器
    if (_showTimerId != 0)
    {
        KillTimer(_showTimerId);
        _showTimerId = 0;
    }

    // 方案B: 首次收到contentSize即调整尺寸并显示(无稳定期延迟)
    _pendingWindowRect = CalculateWindowRect(_currentAnchorRect, width, height);
    if (!_currentContent.plainContent.empty())
    {
        SetWindowPos(&CWnd::wndTopMost, _pendingWindowRect.left, _pendingWindowRect.top,
            _pendingWindowRect.Width(), _pendingWindowRect.Height(),
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }

    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

void CInputHintWindow::CheckForegroundWindow()
{
    HWND foregroundWnd = ::GetForegroundWindow();
    if (foregroundWnd == NULL)
        return;

    DWORD foregroundProcessId = 0;
    GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);

    if (foregroundProcessId != _currentProcessId)
    {
        HideHint();
    }
}

void CInputHintWindow::Update()
{
    if (!IsWindowVisible())
        return;

    CheckForegroundWindow();
}

//====================== 内部辅助方法 ======================

bool CInputHintWindow::_IsReady() const
{
    return _isWebViewCreated && _isUIInitialized;
}

void CInputHintWindow::_PostWebMessage(const std::wstring& action, const std::wstring& data)
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

void CInputHintWindow::_SendContent()
{
    if (!_IsReady() || !_hasContent)
        return;

    const Utils::DiffedInputContent& c = _currentContent;
    const std::wstring& s = c.plainContent;

    // 构建 token 数组: 遍历 plainContent, tag 作为整体一个 token, 其余按字符
    // 相邻同状态的普通字符合并为一个 text token, 以减少节点数
    nlohmann::json tokens = nlohmann::json::array();

    auto stateAt = [&](size_t idx) -> char {
        return idx < c.diffStates.size() ? c.diffStates[idx] : 0;
    };

    size_t i = 0;
    std::wstring textRun;
    char textRunState = 0;

    auto flushTextRun = [&]() {
        if (!textRun.empty())
        {
            nlohmann::json t;
            t["kind"]  = "text";
            t["text"]  = widechar_to_utf8(textRun.c_str());
            t["state"] = (int)textRunState;
            tokens.push_back(t);
            textRun.clear();
        }
    };

    while (i < s.size())
    {
        // 是否有 tag 从 i 处开始
        const Utils::InputContentTagSegment* seg = nullptr;
        for (const auto& tag : c.tagSegments)
        {
            if (tag.startPos == i) { seg = &tag; break; }
        }

        if (seg)
        {
            char st = stateAt(seg->startPos);
            flushTextRun();
            if (st != 2)  // 2 不显示
            {
                nlohmann::json t;
                t["kind"] = "tag";
                t["state"] = (int)st;
                // 解析 rawText 得到 tag 属性
                std::string tagType = "info";
                std::string text = widechar_to_utf8(seg->tagText.c_str());
                std::string data;
                std::string imgSrc;
                try
                {
                    nlohmann::json raw = nlohmann::json::parse(seg->rawText);
                    if (raw.contains("tagType") && raw["tagType"].is_string())
                        tagType = raw["tagType"].get<std::string>();
                    if (raw.contains("text") && raw["text"].is_string())
                        text = raw["text"].get<std::string>();
                    if (raw.contains("data") && raw["data"].is_string())
                        data = raw["data"].get<std::string>();
                    if (raw.contains("imgSrc") && raw["imgSrc"].is_string())
                        imgSrc = raw["imgSrc"].get<std::string>();
                }
                catch (...)
                {
                }
                t["tagType"] = tagType;
                t["text"]    = text;
                t["data"]    = data;
                t["imgSrc"]  = imgSrc;
                tokens.push_back(t);
            }
            i = seg->endPos;
        }
        else
        {
            char st = stateAt(i);
            if (st != 2)  // 2 不显示
            {
                if (!textRun.empty() && st != textRunState)
                    flushTextRun();
                textRunState = st;
                textRun += s[i];
            }
            else
            {
                flushTextRun();
            }
            ++i;
        }
    }
    flushTextRun();

    // 计算客户区可用宽度范围(窗口范围扣除边框), 供HTML测量时约束宽度
    CRect wndRect, clientRect;
    GetWindowRect(&wndRect);
    GetClientRect(&clientRect);
    int frameW = wndRect.Width() - clientRect.Width();
    int maxContentWidth = _maxWindowWidth - frameW;
    int minContentWidth = _minWindowWidth - frameW;
    if (maxContentWidth < 1) maxContentWidth = _maxWindowWidth;
    if (minContentWidth < 1) minContentWidth = 1;

    nlohmann::json jsonData;
    jsonData["tokens"] = tokens;
    jsonData["maxContentWidth"] = maxContentWidth;
    jsonData["minContentWidth"] = minContentWidth;

    try
    {
        std::string jsonStr = jsonData.dump();
        std::wstring wJsonStr = utf8_to_widechar(jsonStr);
        _PostWebMessage(L"setContent", wJsonStr);
    }
    catch (...)
    {
    }
}

