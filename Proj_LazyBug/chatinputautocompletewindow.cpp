#include "stdh.h"
#include "chatinputautocompletewindow.h"

extern std::wstring utf8_to_widechar(const char* utf8_str);

CChatInputAutoCompleteWindow::CChatInputAutoCompleteWindow()
{
    _anchorRect.SetRectEmpty();
}

CChatInputAutoCompleteWindow::~CChatInputAutoCompleteWindow()
{
    if (_font.GetSafeHandle())
        _font.DeleteObject();
}

BEGIN_MESSAGE_MAP(CChatInputAutoCompleteWindow, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
END_MESSAGE_MAP()

void CChatInputAutoCompleteWindow::Create(CWnd* pParent)
{
    // Create a tooltip-like popup window
    CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)::GetStockObject(WHITE_BRUSH),
        NULL);

    CRect rect(0, 0, MAX_WIDTH, MAX_HEIGHT);
    CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        className,
        _T("AutoComplete"),
        WS_POPUP | WS_BORDER,
        rect,
        pParent,
        NULL);

    // Create font
    _font.CreateFont(
        -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN,
        _T("Consolas"));
}

void CChatInputAutoCompleteWindow::ShowCompletion(const std::string& utf8Text)
{
    _utf8Text = utf8Text;

    if (_utf8Text.empty())
    {
        Hide();
        return;
    }

    // 先以默认大小显示，确保窗口有有效 DC 后再计算实际尺寸
    if (!IsWindowVisible())
    {
        SetWindowPos(NULL, 0, 0, MAX_WIDTH, MAX_HEIGHT,
            SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }

    _RecalcSize();

    if (!_anchorRect.IsRectEmpty())
    {
        CRect myRect;
        GetWindowRect(&myRect);
        int x = _anchorRect.left - myRect.Width();   // 右边缘对齐 CChatInput 左边缘
        int y = _anchorRect.bottom - myRect.Height(); // 下沿对齐 CChatInput 下沿
        SetWindowPos(NULL, x, y, 0, 0,
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else
    {
        ShowWindow(SW_HIDE);
    }

    Invalidate();

    // 启动10秒自动隐藏定时器
    SetTimer(AUTO_HIDE_TIMER_ID, AUTO_HIDE_DELAY_MS, NULL);
}

void CChatInputAutoCompleteWindow::Hide()
{
    KillTimer(AUTO_HIDE_TIMER_ID);
    ShowWindow(SW_HIDE);
}

void CChatInputAutoCompleteWindow::SetAnchorRect(const CRect& inputScreenRect)
{
    _anchorRect = inputScreenRect;
}

void CChatInputAutoCompleteWindow::_RecalcSize()
{
    CClientDC dc(this);
    CFont* pOldFont = dc.SelectObject(&_font);

    std::wstring displayText = utf8_to_widechar(_utf8Text.c_str());

    // Calculate text size
    CRect textRect(0, 0, MAX_WIDTH - 16, MAX_HEIGHT - 8);
    ::DrawTextW(dc.GetSafeHdc(), displayText.c_str(), -1, &textRect, DT_LEFT | DT_TOP | DT_CALCRECT | DT_WORDBREAK);

    dc.SelectObject(pOldFont);

    int w = textRect.Width() + 16;
    int h = textRect.Height() + 8;
    if (w < 100) w = 100;
    if (h < 24) h = 24;

    SetWindowPos(NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

void CChatInputAutoCompleteWindow::OnPaint()
{
    CPaintDC dc(this);

    CRect clientRect;
    GetClientRect(&clientRect);

    // Fill background
    dc.FillSolidRect(clientRect, RGB(255, 255, 225)); // Light yellow background

    CFont* pOldFont = dc.SelectObject(&_font);
    dc.SetBkMode(TRANSPARENT);

    // Content text
    dc.SetTextColor(RGB(0, 0, 0));
    std::wstring displayText = utf8_to_widechar(_utf8Text.c_str());
    CRect textRect(clientRect);
    textRect.top += 4;
    textRect.left += 8;
    textRect.right -= 8;
    textRect.bottom -= 4;
    ::DrawTextW(dc.GetSafeHdc(), displayText.c_str(), -1, &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

    dc.SelectObject(pOldFont);
}

BOOL CChatInputAutoCompleteWindow::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void CChatInputAutoCompleteWindow::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == AUTO_HIDE_TIMER_ID)
    {
        Hide();
    }
    CWnd::OnTimer(nIDEvent);
}
