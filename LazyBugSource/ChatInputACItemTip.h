#pragma once

#include "ChatInputACItem.h"


// 前向声明
namespace Gdiplus {
    class Graphics;
    class Bitmap;
    class Font;
}
using namespace Gdiplus;


class CChatInputTest2
{
	 
};


class CChatInputACItemTip : public CWnd
{
    friend class CChatInputACWindow; // 允许CChatInputACWindow访问私有成员
    
public:
    CChatInputACItemTip();
    virtual ~CChatInputACItemTip();

    // 创建与显示/隐藏
    BOOL CreateTipWindow(CWnd* pParent);
    void ShowTip(const ChatInputACItem& item, int x, int y);
    void HideTip();
    bool IsVisible() const { return IsWindowVisible() != FALSE; }

protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    enum class Mode { None, File, Symbol };

    void PrepareContent(const ChatInputACItem& item);
    CSize CalculateWindowSize(Graphics* g);
    void EnsureBufferBitmap(int width, int height);

    void DrawFileMode(Graphics* g, const CRect& rc);
    void DrawSymbolMode(Graphics* g, const CRect& rc);

private:
    Mode _mode;
    ChatInputACItem _item;

    // 展示文本（已准备好）
    std::vector<std::wstring> _displayLines;
    // 每行的高亮区间（start, length），只有一个高亮区间
    std::vector<std::pair<int,int>> _highlightRanges;

    // 字体与排版
    Gdiplus::Font* _textFont; // 常规文本（路径等）
    Gdiplus::Font* _codeFont; // 代码/上下文行
    float _lineHeight;        // 代码行高度（像素）
    int _lineSpacing;         // 行间距（像素）

    // 双缓冲
    Gdiplus::Bitmap* _bufferBitmap;
    int _bufferWidth;
    int _bufferHeight;

    // 布局
    int _paddingX;
    int _paddingY;
    int _maxVisibleLines;
    int _maxWidth;
};


