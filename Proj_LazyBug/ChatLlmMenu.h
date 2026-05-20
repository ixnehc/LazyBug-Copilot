#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>

// 前向声明
namespace Gdiplus {
    class Graphics;
    class Bitmap;
}
using namespace Gdiplus;

// LLM API项结构
struct ChatLlmApiItem
{
    std::wstring name;      // API名称
    bool available;         // 是否可用
    bool selected;          // 是否被选中
};

using LlmApiSelectedCallback = std::function<void(const std::wstring&)>; // API选择回调

// LLM菜单窗口类
class CChatLlmMenu : public CWnd
{
public:
    CChatLlmMenu();
    virtual ~CChatLlmMenu();

    // 创建窗口
    BOOL CreateLlmMenuWindow(CWnd* pParent);

    // 显示/隐藏窗口
    void ShowWindow(const std::vector<ChatLlmApiItem>& apis, int x, int y);
    void HideWindow();

    // 设置回调
    void SetLlmApiSelectedCallback(LlmApiSelectedCallback callback) { _llmApiSelectedCallback = callback; }

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
    std::vector<ChatLlmApiItem> _apis;
    int _hoverIndex;
    LlmApiSelectedCallback _llmApiSelectedCallback;
    
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
    void DrawApiItem(Graphics* graphics, const ChatLlmApiItem& api, const CRect& rect, bool hovered);
    
    // 确保缓冲位图有效
    void EnsureBufferBitmap(int width, int height);
    
    // 获取项目矩形
    CRect GetItemRect(int index);
    
    // 根据点击位置获取项目索引
    int GetItemFromPoint(CPoint point);
    
    // 前台窗口监控
    void CheckForegroundWindow();
};
