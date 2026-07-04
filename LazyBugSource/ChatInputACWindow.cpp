#include "stdh.h"
#include "ChatInputACWindow.h"
#include "ChatInput.h"
#include <algorithm>
#include <cctype>

#include "stringparser/stringparser.h"
#include "nlohmann/json.hpp"

// GDI+相关
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

//====================== CChatInputACWindow 实现 ======================

BEGIN_MESSAGE_MAP(CChatInputACWindow, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_CAPTURECHANGED()
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

CChatInputACWindow::CChatInputACWindow()
    : _selectedIndex(0)
    , _hoverIndex(-1)
    , _itemHeight(24)
    , _maxVisibleItems(12)
	, _owner(NULL)
	, _currentProcessId(0)
    , _bufferBitmap(NULL)
    , _bufferWidth(0)
    , _bufferHeight(0)
    , _scrollOffset(0)
    , _tipTimerId(0)
{
    // 创建字体
    LOGFONT lf;
    GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
    lf.lfHeight = -12;
    _font.CreateFontIndirect(&lf);
    
    lf.lfHeight = -10;
    _descFont.CreateFontIndirect(&lf);
    
    // 获取当前进程ID
    _currentProcessId = GetCurrentProcessId();
}

CChatInputACWindow::~CChatInputACWindow()
{
    // 停止定时器
    StopTipTimer();
    
    // 清理缓冲位图
    if (_bufferBitmap)
    {
        delete _bufferBitmap;
        _bufferBitmap = NULL;
    }
    
    if (m_hWnd)
        DestroyWindow();
}

BOOL CChatInputACWindow::CreateACWindow(CWnd* pParent,CChatInputACList *owner)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL, // 设置背景画刷为NULL，防止系统擦除背景
        NULL);

	_owner = owner;

    // 创建窗口
	BOOL result = CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		className, _T("AutoComplete"),
		WS_POPUP | WS_BORDER,
		0, 0, 200, 100, pParent->GetSafeHwnd(), NULL);
    
    // 创建tip窗口
    if (result)
    {
        _itemTip.CreateTipWindow(this);
    }
    
    return result;
}

void CChatInputACWindow::ShowWindow(const std::vector<ChatInputACItem>& items, int x, int y, int selectedIndex)
{
    _items = items;
    _selectedIndex = selectedIndex;
    _hoverIndex = -1;
    
    // 隐藏tip并停止定时器
    _itemTip.HideTip();
    StopTipTimer();
    
    if (_items.empty())
    {
        HideWindow();
        return;
    }
    
    // 计算窗口大小
    CSize size = CalculateWindowSize();
    
    // 获取屏幕工作区
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    
    // 调整X位置，确保窗口不会超出屏幕
    if (x + size.cx > workArea.right)
        x = workArea.right - size.cx;
    if (x < workArea.left) 
        x = workArea.left;
    
    // 调整Y位置，确保窗口不会超出屏幕
    if (y < workArea.top)
    {
        // 如果窗口顶部会超出屏幕顶部，则显示在@符号下方
        if (_owner && _owner->GetChatInput())
        {
            // 计算@符号的下方位置
            // 这里y是原始的@符号上沿位置减去窗口高度后的结果
            // 所以原始@符号位置应该是 y + size.cy
            int atSymbolTop = y + size.cy;
            y = atSymbolTop + 20; // 显示在@符号下方20像素
        }
        else
        {
            y = workArea.top;
        }
    }
    
    // 确保窗口底部不超出屏幕
    if (y + size.cy > workArea.bottom)
        y = workArea.bottom - size.cy;
    
    // 设置窗口位置和大小
    SetWindowPos(&CWnd::wndTopMost, x, y, size.cx, size.cy, 
        SWP_SHOWWINDOW | SWP_NOACTIVATE);

    SetCapture(); // 捕获鼠标，以便处理外部点击

    // 启动tip定时器
    StartTipTimer();
    
    Invalidate();
}

void CChatInputACWindow::HideWindow()
{
    if (IsWindowVisible())
    {
        if (GetCapture() == this)
        {
            ReleaseCapture(); // 释放鼠标捕获
        }
        CWnd::ShowWindow(SW_HIDE);
    }
    
    // 隐藏tip并停止定时器
    _itemTip.HideTip();
    StopTipTimer();
}

void CChatInputACWindow::SetSelectedIndex(int index)
{
    if (index >= 0 && index < (int)_items.size())
    {
        bool indexChanged = (_selectedIndex != index);
        _selectedIndex = index;
        EnsureSelectedVisible();
        Invalidate();
        
        // 如果选中项改变了，重新启动tip定时器
        if (indexChanged)
        {
            _itemTip.HideTip();
            StartTipTimer();
        }
    }
}

CSize CChatInputACWindow::CalculateWindowSize()
{
    int visibleItems = min((int)_items.size(), _maxVisibleItems);
    int height = visibleItems * _itemHeight + 4; // 2px边框

	return CSize(300, height);
}

void CChatInputACWindow::OnPaint()
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
    
    // --- 在缓冲位图上进行所有绘制操作 ---
    
    // 背景色
    SolidBrush backgroundBrush(Color(255, 13, 17, 23)); // GitHub深色背景
    bufferGraphics.FillRectangle(&backgroundBrush, 0, 0, clientRect.Width(), clientRect.Height());
    
    // 边框
    Pen borderPen(Color(255, 48, 54, 61), 1.0f);
    bufferGraphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width() - 1, clientRect.Height() - 1);
    
    // 绘制项目
    for (int i = 0; i < (int)_items.size(); ++i)
    {
        CRect itemRect = GetItemRect(i);
        
        // 只绘制可见区域内的项目
        if (itemRect.bottom > 1 && itemRect.top < clientRect.bottom - 1)
        {
            bool selected = (i == _selectedIndex);
            bool hovered = (i == _hoverIndex);
            
            DrawItemGDIPlus(&bufferGraphics, _items[i], itemRect, selected, hovered);
        }
    }
    
    // --- 绘制完成，将缓冲位图一次性绘制到屏幕 ---
    
    screenGraphics.DrawImage(_bufferBitmap, 0, 0);
}


void CChatInputACWindow::DrawItemGDIPlus(Graphics* graphics, const ChatInputACItem& item, const CRect& rect, bool selected, bool hovered)
{
    // 背景
    Color bgColor(255, 13, 17, 23); // 默认背景
    if (selected)
        bgColor = Color(255, 88, 166, 255); // 选中背景
    else if (hovered)
        bgColor = Color(255, 33, 38, 45); // 悬停背景
    
    SolidBrush bgBrush(bgColor);
    graphics->FillRectangle(&bgBrush, rect.left, rect.top, rect.Width(), rect.Height());
    
    // 文本颜色
    Color textColor = selected ? Color(255, 255, 255, 255) : Color(255, 240, 246, 252);
    Color descColor = selected ? Color(255, 255, 255, 255) : Color(255, 139, 148, 158);
    
    // 字体
    FontFamily fontFamily(L"Segoe UI");
    Gdiplus::Font mainFont(&fontFamily, 9, FontStyleRegular, UnitPoint);
    Gdiplus::Font descFont(&fontFamily, 8, FontStyleRegular, UnitPoint);
    
    // 文本画刷
    SolidBrush textBrush(textColor);
    SolidBrush descBrush(descColor);
    
    // 文本区域
    int textLeft = rect.left + 8;
    
    // 绘制图标（根据类型绘制不同颜色的小球）
    if (!item.type.empty())
    {
        // 计算小球位置和大小
        int ballSize = 12;
        int ballX = textLeft + 2;
        int ballY = rect.top + (rect.Height() - ballSize) / 2;
        
        // 根据类型确定颜色
        Color ballColor1, ballColor2;
        if (item.type == "file")
        {
            // 白色小球
            ballColor1 = Color(255, 255, 255, 255); // 高光
            ballColor2 = Color(255, 200, 200, 200); // 阴影
        }
        else if (item.type == "symbol")
        {
            // 湖蓝色小球
            ballColor1 = Color(255, 135, 206, 250); // 高光（浅湖蓝）
            ballColor2 = Color(255, 70, 130, 180);  // 阴影（深湖蓝）
        }
        else
        {
            // 默认灰色小球
            ballColor1 = Color(255, 180, 180, 180); // 高光
            ballColor2 = Color(255, 120, 120, 120); // 阴影
        }
        
        // 创建径向渐变画刷（模拟立体效果）
        GraphicsPath ballPath;
        ballPath.AddEllipse(ballX, ballY, ballSize, ballSize);
        
        PathGradientBrush gradientBrush(&ballPath);
        gradientBrush.SetCenterColor(ballColor1);
        
        Color surroundColors[] = { ballColor2 };
        int count = 1;
        gradientBrush.SetSurroundColors(surroundColors, &count);
        
        // 设置渐变中心点（偏向左上角，模拟光照效果）
        gradientBrush.SetCenterPoint(PointF((REAL)(ballX + ballSize * 0.3f), (REAL)(ballY + ballSize * 0.3f)));
        
        // 绘制小球
        graphics->FillEllipse(&gradientBrush, ballX, ballY, ballSize, ballSize);
        
        // 添加高光点（增强立体效果）
        SolidBrush highlightBrush(Color(100, 255, 255, 255));
        int highlightSize = ballSize / 3;
        int highlightX = ballX + ballSize / 4;
        int highlightY = ballY + ballSize / 4;
        graphics->FillEllipse(&highlightBrush, highlightX, highlightY, highlightSize, highlightSize);
        
        textLeft += 20;
    }
    
    // 绘制主文本
    std::wstring textW = utf8_to_widechar(item.text.c_str());
    
    StringFormat stringFormat;
    stringFormat.SetTrimming(StringTrimmingEllipsisCharacter);
    stringFormat.SetFormatFlags(StringFormatFlagsNoWrap);
    
    // 测量主文本的宽度
    RectF measureRect;
    graphics->MeasureString(textW.c_str(), -1, &mainFont, PointF(0, 0), &measureRect);
    
    // 计算主文本的绘制区域，垂直居中
    int textY = rect.top + (rect.Height() - 14) / 2;
    RectF mainTextRect((REAL)textLeft, (REAL)textY, measureRect.Width, 100.0f);
    
    // 绘制主文本
    graphics->DrawString(textW.c_str(), -1, &mainFont, mainTextRect, &stringFormat, &textBrush);
    
    // 在主文本右侧绘制描述文本
    if (!item.description.empty())
    {
        std::wstring descW = utf8_to_widechar(item.description.c_str());
        
        // 描述文本起始位置：主文本右侧 + 10像素间距
        int descLeft = textLeft + (int)measureRect.Width + 10;
        
        // 确保描述文本不会超出右边界
        int maxDescWidth = rect.right - descLeft - 4;
        if (maxDescWidth > 0)
        {
            RectF descRect((REAL)descLeft, (REAL)textY, (REAL)maxDescWidth, 14.0f);
            graphics->DrawString(descW.c_str(), -1, &descFont, descRect, &stringFormat, &descBrush);
        }
    }
}

CRect CChatInputACWindow::GetItemRect(int index)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    
    CRect itemRect;
    itemRect.left = clientRect.left + 1;
    itemRect.right = clientRect.right - 1;
    itemRect.top = clientRect.top + 1 + index * _itemHeight - _scrollOffset;
    itemRect.bottom = itemRect.top + _itemHeight;
    
    return itemRect;
}

int CChatInputACWindow::GetItemFromPoint(CPoint point)
{
    for (int i = 0; i < (int)_items.size() && i < _maxVisibleItems; ++i)
    {
        CRect itemRect = GetItemRect(i);
        if (itemRect.PtInRect(point))
            return i;
    }
    return -1;
}

void CChatInputACWindow::EnsureSelectedVisible()
{
    if (_selectedIndex < 0 || _selectedIndex >= (int)_items.size())
        return;
    
    // 计算当前可见范围
    CRect clientRect;
    GetClientRect(&clientRect);
    int visibleItems = clientRect.Height() / _itemHeight;
    
    // 计算选中项的位置
    int selectedTop = _selectedIndex * _itemHeight;
    int selectedBottom = selectedTop + _itemHeight;
    
    // 如果选中项在可见范围之外
    if (selectedTop < _scrollOffset)
    {
        // 需要向上滚动，使选中项在顶部
        _scrollOffset = selectedTop;
    }
    else if (selectedBottom > _scrollOffset + clientRect.Height())
    {
        // 需要向下滚动，使选中项在底部
        _scrollOffset = selectedBottom - clientRect.Height();
    }
    
    // 确保滚动偏移有效
    int maxScroll = max(0, (int)_items.size() * _itemHeight - clientRect.Height());
    _scrollOffset = min(maxScroll, _scrollOffset);
    
    Invalidate();
}

void CChatInputACWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    if (clientRect.PtInRect(point))
    {
        // 点击在窗口内部
        int itemIndex = GetItemFromPoint(point);
        if (itemIndex >= 0 && itemIndex < (int)_items.size())
        {
            _selectedIndex = itemIndex;
            if (_itemSelectedCallback)
            {
                _itemSelectedCallback(_items[itemIndex]);
            }
            HideWindow();
        }
    }
    else
    {
        // 点击在窗口外部，隐藏窗口
        if (_owner)
            _owner->Hide();
        else
            HideWindow();
    }
    
    CWnd::OnLButtonDown(nFlags, point);
}

void CChatInputACWindow::OnMouseMove(UINT nFlags, CPoint point)
{
    int newHoverIndex = GetItemFromPoint(point);
    if (newHoverIndex != _hoverIndex)
    {
        _hoverIndex = newHoverIndex;
        Invalidate();
    }
    
    CWnd::OnMouseMove(nFlags, point);
}

void CChatInputACWindow::Update()
{
    CheckForegroundWindow();

	// 检查关联的CChatInput是否被隐藏
	if (_owner && _owner->GetChatInput())
	{
		CChatInput* pChatInput = _owner->GetChatInput();
		if (pChatInput->GetSafeHwnd() && !pChatInput->IsWindowVisible())
		{
			// CChatInput被隐藏了，也隐藏自己
			if (_owner)
			{
				_owner->Hide();
			}
			else
			{
				HideWindow();
			}
		}
	}
}

BOOL CChatInputACWindow::OnEraseBkgnd(CDC* pDC)
{
    return TRUE; // 在OnPaint中绘制，避免闪烁
}

// 检查前台窗口
void CChatInputACWindow::CheckForegroundWindow()
{
    // 获取前台窗口
    HWND foregroundWnd = ::GetForegroundWindow();
    if (foregroundWnd == NULL)
        return;
    
    // 获取前台窗口的进程ID
    DWORD foregroundProcessId = 0;
    GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);
    
    // 如果前台窗口不属于当前进程，隐藏自动补全列表
    if (foregroundProcessId != _currentProcessId)
    {
        // 应用程序已经切换到后台，隐藏自动补全列表
        if (_owner)
        {
            _owner->Hide();
        }
        else
        {
            HideWindow();
        }
    }
}

void CChatInputACWindow::EnsureBufferBitmap(int width, int height)
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

void CChatInputACWindow::OnSize(UINT nType, int cx, int cy)
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

void CChatInputACWindow::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIP_TIMER_ID)
    {
        StopTipTimer();
        ShowItemTip();
    }
    
    CWnd::OnTimer(nIDEvent);
}

void CChatInputACWindow::StartTipTimer()
{
    StopTipTimer();
    _tipTimerId = SetTimer(TIP_TIMER_ID, TIP_DELAY_MS, NULL);
}

void CChatInputACWindow::StopTipTimer()
{
    if (_tipTimerId != 0)
    {
        KillTimer(_tipTimerId);
        _tipTimerId = 0;
    }
}

void CChatInputACWindow::ShowItemTip()
{
    if (_selectedIndex >= 0 && _selectedIndex < (int)_items.size())
    {
        const ChatInputACItem& item = _items[_selectedIndex];
        
        // 计算tip显示位置
        CRect windowRect;
        GetWindowRect(&windowRect);
        
        // 获取屏幕工作区
        RECT workArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
        
        // 预估tip窗口大小（使用临时DC）
        CClientDC dc(this);
        Graphics g(dc.GetSafeHdc());
        
        // 临时创建tip来计算大小
        CChatInputACItemTip tempTip;
        tempTip._item = item;
        tempTip.PrepareContent(item);
        CSize tipSize = tempTip.CalculateWindowSize(&g);
        
        int tipX, tipY;
        
        // 优先尝试在AC窗口右侧显示
        tipX = windowRect.right + 5;
        tipY = windowRect.top + _selectedIndex * _itemHeight - _scrollOffset;
        
        // 检查右侧是否放得下
        if (tipX + tipSize.cx > workArea.right)
        {
            // 右侧放不下，放到左侧
            tipX = windowRect.left - tipSize.cx - 5;
            
            // 如果左侧也放不下，就放在屏幕左边界
            if (tipX < workArea.left)
            {
                tipX = workArea.left;
            }
        }
        
        // 确保Y坐标在屏幕范围内
        if (tipY + tipSize.cy > workArea.bottom)
        {
            tipY = workArea.bottom - tipSize.cy;
        }
        if (tipY < workArea.top)
        {
            tipY = workArea.top;
        }
        
        _itemTip.ShowTip(item, tipX, tipY);
    }
}

void CChatInputACWindow::OnCaptureChanged(CWnd* pWnd)
{
    // 当窗口失去鼠标捕获时，此消息被发送。
    // 这可能是因为我们自己调用了 ReleaseCapture，也可能是其他原因（如系统弹窗）。
    // 为确保一致性，在此处隐藏窗口。
    if (pWnd != this)
    {
        if (_owner)
            _owner->Hide();
        else
            HideWindow();
    }

    CWnd::OnCaptureChanged(pWnd);
}

void CChatInputACWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
    if (!bShow)
    {
        // 窗口被隐藏，确保释放鼠标捕获
        if (GetCapture() == this)
            ReleaseCapture();
    }
    
    CWnd::OnShowWindow(bShow, nStatus);
}
