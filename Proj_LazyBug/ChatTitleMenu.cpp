#include "stdh.h"
#include "ChatTitleMenu.h"

#include "stringparser/stringparser.h"
#include "timer/wuid.h"

#include <algorithm>

// 定义M_PI（如果未定义）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// GDI+相关
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

//====================== CChatTitleMenu 实现 ======================

BEGIN_MESSAGE_MAP(CChatTitleMenu, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
	ON_WM_CAPTURECHANGED()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CChatTitleMenu::CChatTitleMenu()
    : _hoverIndex(-1)
    , _scrollOffset(0)
    , _itemHeight(32) // 标题栏菜单项高度
    , _maxVisibleItems(16)
    , _favoriteIconSize(16) // 五角星图标大小
    , _currentProcessId(0)
    , _bufferBitmap(NULL)
    , _bufferWidth(0)
    , _bufferHeight(0)
{
    // 获取当前进程ID
    _currentProcessId = GetCurrentProcessId();
}

CChatTitleMenu::~CChatTitleMenu()
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

BOOL CChatTitleMenu::CreateTitleMenuWindow(CWnd* pParent)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
        ::LoadCursor(NULL, IDC_ARROW),
        NULL, // 设置背景画刷为NULL，防止系统擦除背景
        NULL);

    // 创建窗口
    return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        className, _T("TitleMenu"),
        WS_POPUP | WS_BORDER,
        0, 0, 400, 100, pParent->GetSafeHwnd(), NULL);
}

void CChatTitleMenu::ShowMenu(int x, int y, int width)
{
    _hoverIndex = -1;
    _scrollOffset = 0;  // 重置滚动位置
    
    if (_menuItems.empty())
    {
        HideMenu();
        return;
    }
    
    // 计算窗口大小
    CSize size = CalculateWindowSize();
    
    // 如果指定了宽度，使用指定的宽度；否则使用计算出的宽度
    if (width > 0)
        size.cx = width;
    
    // 获取屏幕工作区
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    
    // 调整Y位置，在标题栏下方显示
    y = y + 4; // 在指定位置下方4像素
    
    // 调整X位置，确保窗口不会超出屏幕
    if (x + size.cx > workArea.right)
        x = workArea.right - size.cx;
    if (x < workArea.left) 
        x = workArea.left;
    
    // 调整Y位置，确保窗口不会超出屏幕
    if (y + size.cy > workArea.bottom)
        y = workArea.bottom - size.cy;
    if (y < workArea.top)
        y = workArea.top;
    
    // 设置窗口位置和大小
    SetWindowPos(&CWnd::wndTopMost, x, y, size.cx, size.cy, 
        SWP_SHOWWINDOW | SWP_NOACTIVATE);

	SetCapture(); // 捕获鼠标，以便处理外部点击
    
    Invalidate();
}

void CChatTitleMenu::HideMenu()
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

void CChatTitleMenu::AddMenuItem(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp, bool isFavorite)
{
    // 检查是否已存在相同ID的菜单项
    auto it = std::find_if(_menuItems.begin(), _menuItems.end(),
        [&menuItemId](const TitleMenuItem& item) {
            return item.id == menuItemId;
        });
    
    if (it != _menuItems.end())
    {
        // 更新现有菜单项
        it->content = content;
        it->stamp = stamp;
        it->isFavorite = isFavorite;
    }
    else
    {
        // 添加新菜单项
        TitleMenuItem newItem;
        newItem.id = menuItemId;
        newItem.content = content;
        newItem.stamp = stamp;
        newItem.isFavorite = isFavorite;
        _menuItems.push_back(newItem);
    }
    
    if (IsWindowVisible())
        Invalidate();
}

void CChatTitleMenu::RemoveMenuItem(const std::wstring& menuItemId)
{
    auto it = std::find_if(_menuItems.begin(), _menuItems.end(),
        [&menuItemId](const TitleMenuItem& item) {
            return item.id == menuItemId;
        });
    
    if (it != _menuItems.end())
    {
        _menuItems.erase(it);
        if (IsWindowVisible())
            Invalidate();
    }
}

void CChatTitleMenu::ClearMenuItems()
{
    _menuItems.clear();
    if (IsWindowVisible())
        HideMenu();
}

void CChatTitleMenu::UpdateMenuItemFavorite(const std::wstring& menuItemId, bool isFavorite)
{
    // 查找对应的菜单项
    auto it = std::find_if(_menuItems.begin(), _menuItems.end(),
        [&menuItemId](const TitleMenuItem& item) {
            return item.id == menuItemId;
        });
    
    if (it != _menuItems.end())
    {
        // 更新favorite状态
        it->isFavorite = isFavorite;
        
        // 重绘菜单（不关闭）
        if (IsWindowVisible())
            Invalidate();
    }
}

CSize CChatTitleMenu::CalculateWindowSize()
{
    int visibleItems = min((int)_menuItems.size(), _maxVisibleItems);
    int height = visibleItems * _itemHeight + 4; // 2px边框
    return CSize(400, height); // 标题栏菜单宽度为400px
}

void CChatTitleMenu::OnPaint()
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
    
    // 背景色 - 与HTML中一致的颜色
    SolidBrush backgroundBrush(Color(255, 13, 61, 84)); // 很深的湖蓝色背景
    bufferGraphics.FillRectangle(&backgroundBrush, 0, 0, clientRect.Width(), clientRect.Height());
    
    // 边框
    Pen borderPen(Color(255, 64, 64, 64), 1.0f);
    bufferGraphics.DrawRectangle(&borderPen, 0, 0, clientRect.Width() - 1, clientRect.Height() - 1);
    
    // 绘制菜单项
    int totalItems = (int)_menuItems.size();
    int endOffset = min(_scrollOffset + _maxVisibleItems, totalItems);
    
    for (int i = _scrollOffset; i < endOffset; ++i)
    {
        CRect itemRect = GetItemRect(i - _scrollOffset);  // 使用相对索引
        bool hovered = ((i == _scrollOffset + _hoverIndex) && (_hoverIndex >= 0));
        
        DrawMenuItem(&bufferGraphics, _menuItems[i], itemRect, hovered, (i == totalItems - 1));
    }
    
    // 将缓冲位图绘制到屏幕
    screenGraphics.DrawImage(_bufferBitmap, 0, 0);
}

void CChatTitleMenu::DrawMenuItem(Graphics* graphics, const TitleMenuItem& item, const CRect& rect, bool hovered, bool isLastItem)
{
    // 背景
    bool isNewChat = (item.id == L"newchat");
    Color bgColor(255, 13, 61, 84); // 默认背景 - 很深的湖蓝色
    if (isNewChat)
        bgColor = Color(255, 7, 50, 70); // New Chat 菜单项背景 - 柔和青绿色
    if (hovered)
    {
        if (isNewChat)
            bgColor = Color(255, 15, 72, 84); // New Chat 悬停背景 - 稍亮的青绿色
        else
            bgColor = Color(255, 15, 72, 84); // 悬停背景 - 稍亮一点的湖蓝色
    }
    
    SolidBrush bgBrush(bgColor);
    graphics->FillRectangle(&bgBrush, rect.left, rect.top, rect.Width(), rect.Height());
    
    // 分割线（最后一个项目不绘制）
    if (!isLastItem && _menuItems.size() > 1)
    {
        Pen separatorPen(Color(255, 64, 64, 64), 1.0f);
        graphics->DrawLine(&separatorPen, rect.left + 8, rect.bottom - 1, rect.right - 8, rect.bottom - 1);
    }
    
    // 绘制五角星（非New Chat项）
    if (!isNewChat)
    {
        DrawFavoriteStar(graphics, rect, item.isFavorite, hovered);
    }
    
    // 文本颜色
    Color textColor(255, 224, 224, 224); // --text-color
    
    // 字体 - 使用粗体字体
    FontFamily fontFamily1(L"Microsoft YaHei UI");
    FontFamily fontFamily2(L"Segoe UI");
    FontFamily* pFontFamily = fontFamily1.IsAvailable() ? &fontFamily1 : &fontFamily2;
    Gdiplus::Font mainFont(pFontFamily, 11, FontStyleBold, UnitPoint);
    
    // 文本画刷
    SolidBrush textBrush(textColor);
    
    // 布局参数 - 左侧留出五角星的空间
    int contentLeft = rect.left + 12 + (isNewChat ? 0 : (_favoriteIconSize + 8)); // 五角星区域 + 间距
    int contentRight = rect.right - 120; // 为stamp留出更多空间（80像素）
    int stampLeft = contentRight + 8;    // stamp起始位置
    int textY = rect.top + (rect.Height() - 18) / 2;  // 扩大文本区域高度

    // 绘制content - 扩大高度
    StringFormat contentFormat;
    contentFormat.SetTrimming(StringTrimmingEllipsisCharacter);
    contentFormat.SetFormatFlags(StringFormatFlagsNoWrap);

    RectF contentRect((REAL)contentLeft, (REAL)textY, (REAL)(contentRight - contentLeft), 18.0f);
    graphics->DrawString(item.content.c_str(), -1, &mainFont, contentRect, &contentFormat, &textBrush);

    // 绘制stamp（如果存在）- 右对齐
    if (!item.stamp.empty())
    {
        Color stampColor(255, 136, 136, 136); // 更浅的灰色
        SolidBrush stampBrush(stampColor);

        FontFamily stampFontFamily1(L"Microsoft YaHei UI");
        FontFamily stampFontFamily2(L"Segoe UI");
        FontFamily* pStampFontFamily = stampFontFamily1.IsAvailable() ? &stampFontFamily1 : &stampFontFamily2;
        Gdiplus::Font stampFont(pStampFontFamily, 9, FontStyleRegular, UnitPoint);  // 减小stamp字体大小

        StringFormat stampFormat;
        stampFormat.SetTrimming(StringTrimmingEllipsisCharacter);
        stampFormat.SetFormatFlags(StringFormatFlagsNoWrap);
        stampFormat.SetAlignment(StringAlignmentFar);    // 右对齐
        stampFormat.SetLineAlignment(StringAlignmentNear);

        // stamp区域：从stampLeft到右边距10像素，给予足够宽度显示完整文本
        RectF stampRect((REAL)stampLeft, (REAL)textY + 2, (REAL)(rect.right - stampLeft - 10), 16.0f);
        graphics->DrawString(item.stamp.c_str(), -1, &stampFont, stampRect, &stampFormat, &stampBrush);
    }
}

void CChatTitleMenu::DrawFavoriteStar(Graphics* graphics, const CRect& rect, bool isFavorite, bool hovered)
{
    // 计算五角星中心位置
    int centerX = rect.left + 12 + _favoriteIconSize / 2;
    int centerY = rect.top + (rect.Height()) / 2;
    
    // 五角星颜色
    Color starColor;
    if (isFavorite)
    {
        // 实心五角星 - 金黄色
        starColor = Color(255, 255, 193, 7);
    }
    else
    {
        // 空心五角星 - 灰色
        starColor = Color(255, 128, 128, 128);
    }
    
    // 创建五角星路径
    GraphicsPath starPath;
    
    // 五角星的5个外顶点和5个内顶点
    const int points = 5;
    float outerRadius = _favoriteIconSize / 2.0f;
    float innerRadius = outerRadius * 0.38f; // 内半径比例
    
    // 先计算所有顶点
    PointF starPoints[points * 2];
    for (int i = 0; i < points * 2; i++)
    {
        float radius = (i % 2 == 0) ? outerRadius : innerRadius;
        float angle = (float)(M_PI / 2.0 + i * M_PI / points); // 从顶部开始
        starPoints[i].X = centerX + radius * (float)cos(angle);
        starPoints[i].Y = centerY - radius * (float)sin(angle);
    }
    
    // 添加多边形
    starPath.AddPolygon(starPoints, points * 2);
    
    // 绘制五角星
    if (isFavorite)
    {
        // 实心填充
        SolidBrush starBrush(starColor);
        graphics->FillPath(&starBrush, &starPath);
    }
    else
    {
        // 空心边框
        Pen starPen(starColor, 1.5f);
        graphics->DrawPath(&starPen, &starPath);
    }
}

CRect CChatTitleMenu::GetItemRect(int index)
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

int CChatTitleMenu::GetItemFromPoint(CPoint point)
{
    for (int i = 0; i < _maxVisibleItems; ++i)
    {
        CRect itemRect = GetItemRect(i);
        if (itemRect.PtInRect(point))
        {
            int actualIndex = _scrollOffset + i;
            if (actualIndex < (int)_menuItems.size())
                return actualIndex;
        }
    }
    return -1;
}

CRect CChatTitleMenu::GetFavoriteRect(int index)
{
    CRect itemRect = GetItemRect(index);
    
    // 五角星区域：左侧12像素开始，大小为_favoriteIconSize
    CRect favRect;
    favRect.left = itemRect.left + 12;
    favRect.top = itemRect.top + (itemRect.Height() - _favoriteIconSize) / 2;
    favRect.right = favRect.left + _favoriteIconSize;
    favRect.bottom = favRect.top + _favoriteIconSize;
    
    return favRect;
}

bool CChatTitleMenu::IsPointInFavoriteRect(CPoint point, int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= (int)_menuItems.size())
        return false;
    
    // New Chat项没有五角星
    if (_menuItems[itemIndex].id == L"newchat")
        return false;
    
    CRect favRect = GetFavoriteRect(itemIndex - _scrollOffset); // 使用相对索引
    return favRect.PtInRect(point) != FALSE;
}

void CChatTitleMenu::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    if (clientRect.PtInRect(point))
    {
		// 点击在窗口内部
		int actualIndex = GetItemFromPoint(point);
		if (actualIndex >= 0 && actualIndex < (int)_menuItems.size())
		{
            // 检查是否点击在五角星区域
            if (IsPointInFavoriteRect(point, actualIndex))
            {
                // 点击五角星，触发favorite回调
                if (_favoriteClickedCallback)
                {
                    _favoriteClickedCallback(
                        _menuItems[actualIndex].id,
                        !_menuItems[actualIndex].isFavorite); // 切换状态
                }
                // 不隐藏菜单，允许继续操作
            }
            else
            {
			    // 点击在菜单项上，触发回调
			    if (_menuItemClickedCallback)
			    {
				    _menuItemClickedCallback(
                        _menuItems[actualIndex].id,
                        _menuItems[actualIndex].content,
                        _menuItems[actualIndex].stamp);
			    }
                // 点击菜单项后隐藏菜单
                HideMenu();
            }
		}
        else
        {
            // 点击在空白区域，隐藏菜单
            HideMenu();
        }
    }
    else
    {
        // 点击在窗口外部，隐藏窗口
        HideMenu();
    }
}

void CChatTitleMenu::OnMouseMove(UINT nFlags, CPoint point)
{
    int newHoverIndex = GetItemFromPoint(point);
    if (newHoverIndex >= 0)
    {
        // 将实际索引转换为相对索引
        newHoverIndex = newHoverIndex - _scrollOffset;
    }
    
    if (newHoverIndex != _hoverIndex)
    {
        _hoverIndex = newHoverIndex;
        Invalidate();
    }
    
    CWnd::OnMouseMove(nFlags, point);
}

void CChatTitleMenu::Update()
{
	if (!IsWindowVisible())
		return;

    CheckForegroundWindow();
}

BOOL CChatTitleMenu::OnEraseBkgnd(CDC* pDC)
{
    return TRUE; // 在OnPaint中绘制，避免闪烁
}

void CChatTitleMenu::CheckForegroundWindow()
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
        HideMenu();
    }
}

void CChatTitleMenu::EnsureBufferBitmap(int width, int height)
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

void CChatTitleMenu::OnSize(UINT nType, int cx, int cy)
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

void CChatTitleMenu::OnCaptureChanged(CWnd* pWnd)
{
    // 当窗口失去鼠标捕获时，此消息被发送。
    // 这可能是因为我们自己调用了ReleaseCapture，也可能是其他原因。
    // 为确保一致性，我们在此处隐藏窗口。
    if (pWnd != this)
    {
        HideMenu();
    }
    
    CWnd::OnCaptureChanged(pWnd);
}

BOOL CChatTitleMenu::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // 计算滚动方向
    // zDelta > 0: 向上滚动（显示前面的项）
    // zDelta < 0: 向下滚动（显示后面的项）
    
    if (_menuItems.size() <= (size_t)_maxVisibleItems)
    {
        // 菜单项少于或等于最大显示数，不需要滚动
        return CWnd::OnMouseWheel(nFlags, zDelta, pt);
    }
    
    int scrollLines = 0;
    
    // WHEEL_DELTA 是120，每次滚动120个单位
    if (zDelta > 0)
    {
        // 向上滚动，scrollOffset减小
        scrollLines = zDelta / WHEEL_DELTA;
        _scrollOffset -= scrollLines;
    }
    else if (zDelta < 0)
    {
        // 向下滚动，scrollOffset增加
        scrollLines = abs(zDelta) / WHEEL_DELTA;
        _scrollOffset += scrollLines;
    }
    
    // 限制scrollOffset的范围
    int maxScrollOffset = (int)_menuItems.size() - _maxVisibleItems;
    if (_scrollOffset < 0)
        _scrollOffset = 0;
    if (_scrollOffset > maxScrollOffset)
        _scrollOffset = maxScrollOffset;
    
    // 根据鼠标当前位置更新悬停索引
    CPoint mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(&mousePos);
    
    int newHoverIndex = GetItemFromPoint(mousePos);
    if (newHoverIndex >= 0)
    {
        // 将实际索引转换为相对索引
        newHoverIndex = newHoverIndex - _scrollOffset;
    }
    _hoverIndex = newHoverIndex;
    
    // 重绘窗口
    Invalidate();
    
    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


