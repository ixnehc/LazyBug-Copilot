#pragma once

#define AUTO_HIDE_TIMER_ID  1
#define AUTO_HIDE_DELAY_MS  10000

// Temporary test window to display LLM-generated input auto-completion
class CChatInputAutoCompleteWindow : public CWnd
{
public:
    CChatInputAutoCompleteWindow();
    ~CChatInputAutoCompleteWindow();

    void Create(CWnd* pParent);
    void ShowCompletion(const std::string& utf8Text);
    void Hide();
    void SetAnchorRect(const CRect& inputScreenRect);

protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    DECLARE_MESSAGE_MAP()

private:
    void _RecalcSize();

    std::string _utf8Text;
    CFont _font;
    CRect _anchorRect;          // CChatInput 的屏幕矩形
    enum { MAX_WIDTH = 500, MAX_HEIGHT = 200 };
};
