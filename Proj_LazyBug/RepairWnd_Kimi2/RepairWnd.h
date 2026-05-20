#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <wrl.h>
#include <WebView2.h>
#include <afxwin.h>  // MFC支持

#include "../Common/codediff/CodeDiff.h"

class CRepairWnd : public CWnd
{
public:
	CRepairWnd();
	virtual ~CRepairWnd();

	// 创建WebView2控件
	BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	void Update();

	// 设置内容
	void SetContent(const CodeComparingChars& content);
	void Clear();

	//diffStr的格式如下
	//###old lines###
	//需要替换的代码行
	//###new lines###
	//替换内容
	void Show(const char* diffStr, const RECT& focusRect);

	void Navigate(const std::wstring& url);

	// 调整WebView大小
	void ResizeWebView();

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
	EventRegistrationToken _processFailedToken = {};

	// 状态标志
	bool _isWebViewCreated;
	bool _isContentLoaded;

	// 当前内容
	CodeComparingChars _currentContent;

	// 检查WebView是否已初始化
	bool _IsReady() const;

	// 发送内容到WebView
	void _SendContentToWebView();
	void _PostWebMessage(const std::wstring& action, const std::wstring& data);
	std::wstring _EscapeJsonString(const std::wstring& str);
	std::wstring _BuildContentJson();

	// Show函数相关私有方法
	void _ParseDiffString(const char* diffStr, std::string& oldCode, std::string& newCode);
	void _HandleWebMessage(const wchar_t* message);
	void _RepositionAndShow(int contentWidth, int contentHeight);
	void _CalculateBestPosition(const RECT& focusRect, const RECT& parentRect,
		int windowWidth, int windowHeight, CRect& bestRect);

	RECT _pendingFocusRect;  // 存储待处理的focusRect

	// 执行JavaScript脚本
	void ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback = nullptr);
};
