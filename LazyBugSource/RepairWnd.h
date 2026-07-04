#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

#include "codediff/CodeDiff.h"

// 回调函数类型定义
using RepairWndNavigationCompletedCallback = std::function<void(bool)>;

class CRepairWnd : public CWnd
{
public:
    // 构造函数和析构函数
    CRepairWnd();
    virtual ~CRepairWnd();

    // 创建WebView2控件
    BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
    
    // 导航方法
    void Navigate(const std::wstring& url);
    void NavigateToString(const std::wstring& htmlContent);
    void Reload();
    
    // 脚本执行
    void ExecuteScript(const std::wstring& script, 
                      std::function<void(const std::wstring&)> callback = nullptr);
    
    // 事件处理和回调设置
    void SetNavigationCompletedCallback(RepairWndNavigationCompletedCallback callback);
    
    // 获取WebView2环境和核心WebView
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }
    
    // 调整WebView2大小
    void ResizeWebView();

    // ===== 代码显示功能相关方法 =====
    
    // 初始化代码显示界面
    void InitializeCodeUI();
    
    // 设置代码内容
    void SetContent(const CodeComparingChars& content);

	void Show(const CodeComparingChars& content, const RECT& focusRect);
	void Hide();
    
    // 清空内容
    void Clear();

protected:
    // 初始化WebView2环境
    HRESULT InitializeWebView();
    
    // 消息处理函数
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    
    DECLARE_MESSAGE_MAP()

private:
    // WebView2相关组件
    ICoreWebView2Environment* _webViewEnvironment;
    ICoreWebView2* _webView;
    ICoreWebView2Controller* _controller;
    
    // 事件处理Token
    EventRegistrationToken _navigationCompletedToken = {};
    EventRegistrationToken _webMessageReceivedToken = {};

    // 回调函数
    RepairWndNavigationCompletedCallback _navigationCompletedCallback;
    
    // 脚本执行回调映射
    std::map<int, std::function<void(const std::wstring&)>> _scriptCallbacks;
    int _callbackId;
    
    // 状态标志
    bool _isWebViewCreated;
    bool _isCodeUIInitialized;
    bool _isPendingShow;
    RECT _pendingFocusRect;
    
    // 检查WebView和CodeUI是否已初始化
    bool _IsReady() const;
    
    // 内部消息发送
    void _PostWebMessage(const std::wstring& action, const std::wstring& data);
    
    // JSON辅助方法（使用外部函数）
    // std::wstring _EscapeJsonString(const std::wstring& str);
    
    // 构建代码内容JSON
    std::wstring _BuildCodeContentJson(const CodeComparingChars& content);
    
    // 计算窗口位置（避免遮挡focusRect）
    void _CalculateWindowPosition(const RECT& focusRect, int windowWidth, int windowHeight, int& x, int& y);
    
    // 完成显示流程
    void _CompleteShow();
    
    // 处理内容尺寸就绪消息
    void _HandleContentSizeReady(const std::wstring& sizeData);
};
