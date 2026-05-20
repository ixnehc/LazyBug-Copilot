#include "stdh.h"
#include "ChatInputTagMenuWindow.h"

#include "stringparser/stringparser.h"
#include "timer/wuid.h"

#include <algorithm>

// GDI+相关
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

//====================== CChatInputTagMenuWindow 实现 ======================

BEGIN_MESSAGE_MAP(CChatInputTagMenuWindow, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

CChatInputTagMenuWindow::CChatInputTagMenuWindow()
    : _hoverIndex(-1)
    , _itemHeight(25) // 缩短高度
    , _maxVisibleItems(15)
    , _currentProcessId(0)
    , _bufferBitmap(NULL)
    , _bufferWidth(0)
    , _bufferHeight(0)
{
    // 获取当前进程ID
    _currentProcessId = GetCurrentProcessId();
}

CChatInputTagMenuWindow::~CChatInputTagMenuWindow()
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

BOOL CChatInputTagMenuWindow::CreateTagMenuWindow(CWnd* pParent)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL, // 设置背景画刷为NULL，防止系统擦除背景
        NULL);

    // 创建窗口
    return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        className, _T("TagMenu"),
        WS_POPUP | WS_BORDER,
        0, 0, 200, 100, pParent->GetSafeHwnd(), NULL);
}

void CChatInputTagMenuWindow::ShowWindow(const std::vector<ChatInputTag>& tags, int x, int y)
{
    // 按字母排序
    _tags = tags;
    std::sort(_tags.begin(), _tags.end(), [](const ChatInputTag& a, const ChatInputTag& b) {
        std::wstring aLower = a.text;
        std::wstring bLower = b.text;
        std::transform(aLower.begin(), aLower.end(), aLower.begin(), ::towlower);
        std::transform(bLower.begin(), bLower.end(), bLower.begin(), ::towlower);
        return aLower < bLower;
    });
    
    _hoverIndex = -1;
    
    if (_tags.empty())
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

void CChatInputTagMenuWindow::HideWindow()
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

CSize CChatInputTagMenuWindow::CalculateWindowSize()
{
    int visibleItems = min((int)_tags.size(), _maxVisibleItems);
    int height = visibleItems * _itemHeight + 4; // 2px边框
    return CSize(250, height);
}

void CChatInputTagMenuWindow::OnPaint()
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
    
    // 背景色
    SolidBrush backgroundBrush(Color(255, 13, 17, 23)); // GitHub深色背景
    bufferGraphics.FillRectangle(&backgroundBrush, 0, 0, clientRect.Width(), clientRect.Height());
    
    // 边框
    Pen borderPen(Color(255, 48, 54, 61), 1.0f);
    bufferGraphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width() - 1, clientRect.Height() - 1);
    
    // 绘制项目
    for (int i = 0; i < (int)_tags.size() && i < _maxVisibleItems; ++i)
    {
        CRect itemRect = GetItemRect(i);
        bool hovered = (i == _hoverIndex);
        
        DrawTagItem(&bufferGraphics, _tags[i], itemRect, hovered);
    }
    
    // 将缓冲位图绘制到屏幕
    screenGraphics.DrawImage(_bufferBitmap, 0, 0);
}

void CChatInputTagMenuWindow::DrawTagItem(Graphics* graphics, const ChatInputTag& tag, const CRect& rect, bool hovered)
{
    // 背景
    Color bgColor(255, 13, 17, 23); // 默认背景
    if (hovered)
        bgColor = Color(255, 33, 38, 45); // 悬停背景
    
    SolidBrush bgBrush(bgColor);
    graphics->FillRectangle(&bgBrush, rect.left, rect.top, rect.Width(), rect.Height());
    
    // 文本颜色
    Color textColor(255, 240, 246, 252);
    if (!tag.removable)
    {
        // 不可删除的标签使用较暗的文本颜色
        textColor = Color(255, 139, 148, 158);
    }
    
    // 字体
    FontFamily fontFamily(L"Segoe UI");
    Gdiplus::Font mainFont(&fontFamily, 9, FontStyleRegular, UnitPoint);
    
    // 文本画刷
    SolidBrush textBrush(textColor);
    
    // 绘制复选框
    int checkboxLeft = rect.left + 8;
    int checkboxTop = rect.top + (rect.Height() - 14) / 2;
    int checkboxSize = 14;

    if (!tag.removable)
    {
        // 不可删除的标签：复选框灰掉，但显示为勾选状态
        Color checkboxBg = Color(255, 33, 38, 45);
        Color checkboxBorder = Color(255, 72, 77, 82);
        SolidBrush checkboxBrush(checkboxBg);
        Pen checkboxPen(checkboxBorder, 1.0f);
        graphics->FillRectangle(&checkboxBrush, checkboxLeft, checkboxTop, checkboxSize, checkboxSize);
        graphics->DrawRectangle(&checkboxPen, checkboxLeft, checkboxTop, checkboxSize, checkboxSize);

        // 绘制灰色的勾选标记
        SolidBrush checkMarkBrush(Color(255, 72, 77, 82));
        PointF checkPoints[] =
        {
            PointF(checkboxLeft + 2.0f, checkboxTop + 8.0f),
            PointF(checkboxLeft + 5.0f, checkboxTop + 11.0f),
            PointF(checkboxLeft + 12.0f, checkboxTop + 4.0f),
            PointF(checkboxLeft + 11.0f, checkboxTop + 3.0f),
            PointF(checkboxLeft + 5.0f, checkboxTop + 9.0f),
            PointF(checkboxLeft + 3.0f, checkboxTop + 7.0f)
        };
        graphics->FillPolygon(&checkMarkBrush, checkPoints, 6);
    }
    else if (tag.visible)
    {
        // 选中状态: 蓝色边框，内部无填充
        Color checkboxBorder(255, 88, 166, 255);
        Pen checkboxPen(checkboxBorder, 1.0f);
        graphics->DrawRectangle(&checkboxPen, checkboxLeft, checkboxTop, checkboxSize, checkboxSize);

        // 绘制勾选标记 (多边形，居中)
        SolidBrush checkMarkBrush(Color(255, 255, 255, 255));
        PointF checkPoints[] =
        {
            PointF(checkboxLeft + 2.0f, checkboxTop + 8.0f),
            PointF(checkboxLeft + 5.0f, checkboxTop + 11.0f),
            PointF(checkboxLeft + 12.0f, checkboxTop + 4.0f),
            PointF(checkboxLeft + 11.0f, checkboxTop + 3.0f),
            PointF(checkboxLeft + 5.0f, checkboxTop + 9.0f),
            PointF(checkboxLeft + 3.0f, checkboxTop + 7.0f)
        };
        graphics->FillPolygon(&checkMarkBrush, checkPoints, 6);
    }
    else
    {
        // 未选中状态: 深灰色填充
        Color checkboxBg = Color(255, 48, 54, 61);
        Color checkboxBorder = Color(255, 139, 148, 158);
        SolidBrush checkboxBrush(checkboxBg);
        Pen checkboxPen(checkboxBorder, 1.0f);
        graphics->FillRectangle(&checkboxBrush, checkboxLeft, checkboxTop, checkboxSize, checkboxSize);
        graphics->DrawRectangle(&checkboxPen, checkboxLeft, checkboxTop, checkboxSize, checkboxSize);
    }
    
    // 绘制标签文本
    int textLeft = checkboxLeft + checkboxSize + 8;
    int textY = rect.top + (rect.Height() - 14) / 2;
    
    StringFormat stringFormat;
    stringFormat.SetTrimming(StringTrimmingEllipsisCharacter);
    stringFormat.SetFormatFlags(StringFormatFlagsNoWrap);
    
    RectF textRect((REAL)textLeft, (REAL)textY + 1, (REAL)(rect.right - textLeft - 4), 14.0f);
    graphics->DrawString(tag.text.c_str(), -1, &mainFont, textRect, &stringFormat, &textBrush);
}

CRect CChatInputTagMenuWindow::GetItemRect(int index)
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

int CChatInputTagMenuWindow::GetItemFromPoint(CPoint point)
{
    for (int i = 0; i < (int)_tags.size() && i < _maxVisibleItems; ++i)
    {
        CRect itemRect = GetItemRect(i);
        if (itemRect.PtInRect(point))
            return i;
    }
    return -1;
}

void CChatInputTagMenuWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    if (clientRect.PtInRect(point))
    {
		// 点击在窗口内部
		int itemIndex = GetItemFromPoint(point);
		if (itemIndex >= 0 && itemIndex < (int)_tags.size())
		{
			// 只有可删除的标签才能切换可见性
			if (_tags[itemIndex].removable)
			{
				// 点击在某个项目上，切换其可见性
				if (_tagVisibilityChangedCallback)
				{
					_tagVisibilityChangedCallback(_tags[itemIndex].id, !_tags[itemIndex].visible);
				}

				// 立即更新本地UI状态以提供即时反馈
				_tags[itemIndex].visible = !_tags[itemIndex].visible;
				Invalidate(); // 重绘窗口以显示复选框的变化
			}
		}
    }
    else
    {
        // 点击在窗口外部，隐藏窗口
        HideWindow();
    }
}

void CChatInputTagMenuWindow::OnMouseMove(UINT nFlags, CPoint point)
{
    int newHoverIndex = GetItemFromPoint(point);
    if (newHoverIndex != _hoverIndex)
    {
        _hoverIndex = newHoverIndex;
        Invalidate();
    }
    
    CWnd::OnMouseMove(nFlags, point);
}

void CChatInputTagMenuWindow::Update()
{
	if (!IsWindowVisible())
		return;

    CheckForegroundWindow();
}

BOOL CChatInputTagMenuWindow::OnEraseBkgnd(CDC* pDC)
{
    return TRUE; // 在OnPaint中绘制，避免闪烁
}

void CChatInputTagMenuWindow::CheckForegroundWindow()
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

void CChatInputTagMenuWindow::EnsureBufferBitmap(int width, int height)
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

void CChatInputTagMenuWindow::OnSize(UINT nType, int cx, int cy)
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

void CChatInputTagMenuWindow::OnCaptureChanged(CWnd* pWnd)
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
