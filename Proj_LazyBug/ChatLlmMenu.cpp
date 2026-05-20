#include "stdh.h"
#include "ChatLlmMenu.h"

#include "stringparser/stringparser.h"
#include "timer/wuid.h"

#include <algorithm>

// GDI+相关
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

//====================== CChatLlmMenu 实现 ======================
 
BEGIN_MESSAGE_MAP(CChatLlmMenu, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

CChatLlmMenu::CChatLlmMenu()
    : _hoverIndex(-1)
    , _itemHeight(28)
    , _maxVisibleItems(32)
    , _currentProcessId(0)
    , _bufferBitmap(NULL)
    , _bufferWidth(0)
    , _bufferHeight(0)
{
    // 获取当前进程ID
    _currentProcessId = GetCurrentProcessId();
}

CChatLlmMenu::~CChatLlmMenu()
{
    // 清理缓冲位图
    if (_bufferBitmap)
    {
        delete _bufferBitmap;
        _bufferBitmap = NULL;
    }
    
    if (m_hWnd)
        DestroyWindow();
}

BOOL CChatLlmMenu::CreateLlmMenuWindow(CWnd* pParent)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL, // 设置背景画刷为NULL，防止系统擦除背景
        NULL);

    // 创建窗口
    return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        className, _T("LlmMenu"),
        WS_POPUP | WS_BORDER,
        0, 0, 200, 100, pParent->GetSafeHwnd(), NULL);
}

void CChatLlmMenu::ShowWindow(const std::vector<ChatLlmApiItem>& apis, int x, int y)
{
    _apis = apis;
    _hoverIndex = -1;
    
    if (_apis.empty())
    {
        HideWindow();
        return;
    }
    
    // 计算窗口大小
    CSize size = CalculateWindowSize();
    
    // 获取屏幕工作区
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    
    // 调整位置以显示在上方
    y = y - size.cy - 4; // 在指定位置上方4像素
    
    // 调整X位置，确保窗口不会超出屏幕
    if (x + size.cx > workArea.right)
        x = workArea.right - size.cx;
    if (x < workArea.left) 
        x = workArea.left;
    
    // 调整Y位置，确保窗口不会超出屏幕
    if (y < workArea.top)
        y = workArea.top;
    if (y + size.cy > workArea.bottom)
        y = workArea.bottom - size.cy;
    
    // 设置窗口位置和大小
    SetWindowPos(&CWnd::wndTopMost, x, y, size.cx, size.cy, 
        SWP_SHOWWINDOW | SWP_NOACTIVATE);

    SetCapture(); // 捕获鼠标，以便处理外部点击
    
    Invalidate();
}

void CChatLlmMenu::HideWindow()
{
    if (IsWindowVisible())
    {
        if (GetCapture() == this)
        {
            ReleaseCapture(); // 释放鼠标捕获
        }
        CWnd::ShowWindow(SW_HIDE);
    }
}

CSize CChatLlmMenu::CalculateWindowSize()
{
    int visibleItems = min((int)_apis.size(), _maxVisibleItems);
    int height = visibleItems * _itemHeight + 4; // 2px边框
    return CSize(280, height);
}

void CChatLlmMenu::OnPaint()
{
    CPaintDC dc(this);
    
    CRect clientRect;
    GetClientRect(&clientRect);

    // 创建屏幕Graphics对象
    Graphics screenGraphics(dc.GetSafeHdc());
    
    // 确保缓冲位图有效且大小正确
    EnsureBufferBitmap(clientRect.Width(), clientRect.Height());
    
    if (!_bufferBitmap)
        return; // 如果缓冲位图创建失败，直接返回
    
    // 从缓冲位图创建Graphics对象
    Graphics bufferGraphics(_bufferBitmap);
    
    // 在缓冲Graphics上启用抗锯齿
    bufferGraphics.SetSmoothingMode(SmoothingModeAntiAlias);
    bufferGraphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    
    // 背景色 - 使用GitHub深色主题
    SolidBrush backgroundBrush(Color(255, 13, 17, 23));
    bufferGraphics.FillRectangle(&backgroundBrush, 0, 0, clientRect.Width(), clientRect.Height());
    
    // 边框
    Pen borderPen(Color(255, 48, 54, 61), 1.0f);
    bufferGraphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width() - 1, clientRect.Height() - 1);
    
    // 绘制项目
    for (int i = 0; i < (int)_apis.size() && i < _maxVisibleItems; ++i)
    {
        CRect itemRect = GetItemRect(i);
        bool hovered = (i == _hoverIndex);
        
        DrawApiItem(&bufferGraphics, _apis[i], itemRect, hovered);
    }
    
    // 将缓冲位图绘制到屏幕
    screenGraphics.DrawImage(_bufferBitmap, 0, 0);
}

void CChatLlmMenu::DrawApiItem(Graphics* graphics, const ChatLlmApiItem& api, const CRect& rect, bool hovered)
{
    // 背景
    Color bgColor(255, 13, 17, 23); // 默认背景
    if (hovered)
        bgColor = Color(255, 33, 38, 45); // 悬停背景
    
    SolidBrush bgBrush(bgColor);
    graphics->FillRectangle(&bgBrush, rect.left, rect.top, rect.Width(), rect.Height());
    
    // 文本颜色
    Color textColor(255, 240, 246, 252);
    if (!api.available)
    {
        textColor = Color(255, 211, 93, 93); // 不可用时显示深红色
    }
    
    // 字体
    FontFamily fontFamily(L"Segoe UI");
    Gdiplus::Font mainFont(&fontFamily, 10, FontStyleRegular, UnitPoint);
    
    // 文本画刷
    SolidBrush textBrush(textColor);
    
    // 绘制选中标记（如果选中）
    int checkLeft = rect.left + 10;
    int checkTop = rect.top + (rect.Height() - 2) / 2;
    
    if (api.selected)
    {
        // 绘制蓝色选中标记
        SolidBrush checkBrush(Color(255, 88, 166, 255));
        graphics->FillEllipse(&checkBrush, checkLeft, checkTop, 6, 6);
    }
    
    // 绘制API名称文本
    int textLeft = checkLeft + 20;
    int textY = rect.top + (rect.Height() - 16) / 2;
    
    StringFormat stringFormat;
    stringFormat.SetTrimming(StringTrimmingEllipsisCharacter);
    stringFormat.SetFormatFlags(StringFormatFlagsNoWrap);
    
    RectF textRect((REAL)textLeft, (REAL)textY + 1, (REAL)(rect.right - textLeft - 10), 16.0f);
    graphics->DrawString(api.name.c_str(), -1, &mainFont, textRect, &stringFormat, &textBrush);
}

CRect CChatLlmMenu::GetItemRect(int index)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    
    CRect itemRect;
    itemRect.left = clientRect.left + 1;
    itemRect.right = clientRect.right - 1;
    itemRect.top = clientRect.top + 1 + index * _itemHeight;
    itemRect.bottom = itemRect.top + _itemHeight;
    
    return itemRect;
}

int CChatLlmMenu::GetItemFromPoint(CPoint point)
{
    for (int i = 0; i < (int)_apis.size() && i < _maxVisibleItems; ++i)
    {
        CRect itemRect = GetItemRect(i);
        if (itemRect.PtInRect(point))
            return i;
    }
    return -1;
}

void CChatLlmMenu::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    if (clientRect.PtInRect(point))
    {
        // 点击在窗口内部
        int itemIndex = GetItemFromPoint(point);
        if (itemIndex >= 0 && itemIndex < (int)_apis.size())
        {
            // 点击在某个项目上，触发回调
            if (_llmApiSelectedCallback && _apis[itemIndex].available)
            {
                _llmApiSelectedCallback(_apis[itemIndex].name);
            }
            HideWindow();
        }
    }
    else
       {
        // 点击在窗口外部，隐藏窗口
        HideWindow();
    }
}

void CChatLlmMenu::OnMouseMove(UINT nFlags, CPoint point)
{
    int newHoverIndex = GetItemFromPoint(point);
    if (newHoverIndex != _hoverIndex)
    {
        _hoverIndex = newHoverIndex;
        Invalidate();
    }
    
    CWnd::OnMouseMove(nFlags, point);
}

void CChatLlmMenu::Update()
{
    if (!IsWindowVisible())
        return;

    CheckForegroundWindow();
}

BOOL CChatLlmMenu::OnEraseBkgnd(CDC* pDC)
{
    return TRUE; // 在OnPaint中绘制，避免闪烁
}

void CChatLlmMenu::CheckForegroundWindow()
{
    // 获取前台窗口
    HWND foregroundWnd = ::GetForegroundWindow();
    if (foregroundWnd == NULL)
        return;
    
    // 获取前台窗口的进程ID
    DWORD foregroundProcessId = 0;
    GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);
    
    // 如果前台窗口不属于当前进程，隐藏菜单
    if (foregroundProcessId != _currentProcessId)
    {
        HideWindow();
    }
}

void CChatLlmMenu::EnsureBufferBitmap(int width, int height)
{
    // 检查是否需要重新创建缓冲位图
    if (!_bufferBitmap || _bufferWidth != width || _bufferHeight != height)
    {
        // 清理旧的缓冲位图
        if (_bufferBitmap)
        {
            delete _bufferBitmap;
            _bufferBitmap = NULL;
        }
        
        // 创建新的缓冲位图
        if (width > 0 && height > 0)
        {
            _bufferBitmap = new Bitmap(width, height, PixelFormat32bppARGB);
            _bufferWidth = width;
            _bufferHeight = height;
        }
    }
}

void CChatLlmMenu::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    
    // 窗口大小变化时，清理缓冲位图，让它在下次绘制时重新创建
    if (_bufferBitmap)
    {
        delete _bufferBitmap;
        _bufferBitmap = NULL;
        _bufferWidth = 0;
        _bufferHeight = 0;
    }
}

void CChatLlmMenu::OnCaptureChanged(CWnd* pWnd)
{
    // 当窗口失去鼠标捕获时，此消息被发送。
    // 这可能是因为我们自己调用了ReleaseCapture，也可能是其他原因。
    // 为确保一致性，我们在此处隐藏窗口。
    if (pWnd != this)
    {
        HideWindow();
    }
    
    CWnd::OnCaptureChanged(pWnd);
}
