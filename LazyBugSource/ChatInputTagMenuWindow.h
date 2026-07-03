#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include "ChatInputTag.h"

using TagVisibilityChangedCallback = std::function<void(const std::wstring&, bool)>; // 标签可见性变化回调

// 前向声明
namespace Gdiplus {
    class Graphics;
    class Bitmap;
}
using namespace Gdiplus;


// 标签菜单窗口类
class CChatInputTagMenuWindow : public CWnd
{
public:
    CChatInputTagMenuWindow();
    virtual ~CChatInputTagMenuWindow();

    // 创建窗口
    BOOL CreateTagMenuWindow(CWnd* pParent);

    // 显示/隐藏窗口
    void ShowWindow(const std::vector<ChatInputTag>& tags, int x, int y);
    void HideWindow();

    // 设置回调
    void SetTagVisibilityChangedCallback(TagVisibilityChangedCallback callback) { _tagVisibilityChangedCallback = callback; }

    // 更新
    void Update();

protected:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    DECLARE_MESSAGE_MAP()

private:
    std::vector<ChatInputTag> _tags;
    int _hoverIndex;
    TagVisibilityChangedCallback _tagVisibilityChangedCallback;
    
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
    
    // 使用GDI+绘制项目
    void DrawTagItem(Graphics* graphics, const ChatInputTag& tag, const CRect& rect, bool hovered);
    
    // 确保缓冲位图有效
    void EnsureBufferBitmap(int width, int height);
    
    // 获取项目矩形
    CRect GetItemRect(int index);
    
    // 根据点击位置获取项目索引
    int GetItemFromPoint(CPoint point);
    
    // 前台窗口监控
    void CheckForegroundWindow();
};
