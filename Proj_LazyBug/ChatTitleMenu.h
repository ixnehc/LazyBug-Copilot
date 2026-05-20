#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>

// 回调函数类型定义
using TitleMenuItemClickedCallback = std::function<void(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp)>;

// 标题栏菜单项结构
struct TitleMenuItem
{
    std::wstring id;      // 菜单项唯一ID
    std::wstring content;  // 显示文本
    std::wstring stamp;    // 时间戳/标记
};

// 前向声明
namespace Gdiplus {
    class Graphics;
    class Bitmap;
}
using namespace Gdiplus;


// 标题栏菜单窗口类
class CChatTitleMenu: public CWnd
{
public:
    CChatTitleMenu();
    virtual ~CChatTitleMenu();

    // 创建窗口
    BOOL CreateTitleMenuWindow(CWnd* pParent);

    // 显示/隐藏窗口
    void ShowMenu(int x, int y, int width = 0);  // width为0时自动计算宽度
    void HideMenu();

    // 设置回调
    void SetMenuItemClickedCallback(TitleMenuItemClickedCallback callback) { _menuItemClickedCallback = callback; }

    // 更新
    void Update();

    // 菜单项管理
    void AddMenuItem(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp);
    void RemoveMenuItem(const std::wstring& menuItemId);
    void ClearMenuItems();
    
    // 判断是否有菜单项
    bool HasMenuItems() const { return !_menuItems.empty(); }
    
    // 设置最大显示菜单项数量
    void SetMaxVisibleItems(int maxItems) { _maxVisibleItems = maxItems; }
    int GetMaxVisibleItems() const { return _maxVisibleItems; }

protected:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    DECLARE_MESSAGE_MAP()

private:
    std::vector<TitleMenuItem> _menuItems;
    int _hoverIndex;
    int _scrollOffset;  // 滚动偏移量，表示第一个显示的菜单项索引
    TitleMenuItemClickedCallback _menuItemClickedCallback;
    
    // UI参数
    int _itemHeight;
    int _maxVisibleItems;
    
    // GDI+缓冲位图相关
    Gdiplus::Bitmap* _bufferBitmap;
    int _bufferWidth;
    int _bufferHeight;
    
    // 前台窗口监控
    DWORD _currentProcessId;
    
    // 计算窗口大小
    CSize CalculateWindowSize();
    
    // 绘制菜单项
    void DrawMenuItem(Graphics* graphics, const TitleMenuItem& item, const CRect& rect, bool hovered, bool isLastItem);
    
    // 确保缓冲位图有效
    void EnsureBufferBitmap(int width, int height);
    
    // 获取项目矩形
    CRect GetItemRect(int index);
    
    // 根据点击位置获取项目索引
    int GetItemFromPoint(CPoint point);
    
    // 前台窗口监控
    void CheckForegroundWindow();
};
