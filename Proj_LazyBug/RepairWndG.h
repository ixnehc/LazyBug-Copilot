#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <gdiplus.h>

#include "codediff/CodeDiff.h"

// 外部函数声明
extern std::wstring utf8_to_widechar(const std::string& utf8_str);

class CRepairWndG : public CWnd
{
public:
    // 构造函数和析构函数
    CRepairWndG();
    virtual ~CRepairWndG();

    // 创建窗口
    BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
    
    void Show(const CodeComparingChars& content, const RECT& focusRect);
    void Hide();
    
    // 清空内容
    void Clear();

protected:
    // 消息处理函数
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    
    DECLARE_MESSAGE_MAP()

private:
    // GDI+相关
    ULONG_PTR _gdiplusToken;
    Gdiplus::Font* _font;
    Gdiplus::SolidBrush* _textBrush;
    Gdiplus::SolidBrush* _newCodeBrush;
    Gdiplus::SolidBrush* _backgroundBrush;
    Gdiplus::SolidBrush* _newCodeBackBrush;
    
    // 内容相关
    CodeComparingChars _content;
    std::vector<std::wstring> _lines;
    std::vector<std::vector<CodeComparingChars::CharType>> _lineCharTypes;
    
    // 布局相关
    int _lineHeight;
    int _charWidth;
    int _padding;
    
    // 状态标志
    bool _isPendingShow;
    RECT _pendingFocusRect;
    
    // 私有方法
    void _InitializeGDIPlus();
    void _CleanupGDIPlus();
    void _ProcessContent();
    void _CalculateSize(int& width, int& height);
    void _CalculateWindowPosition(const RECT& focusRect, int windowWidth, int windowHeight, int& x, int& y);
    void _DrawContent(Gdiplus::Graphics& graphics);
    void _CompleteShow();
};
