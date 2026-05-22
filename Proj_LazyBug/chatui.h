#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include <functional>
#include <fstream>
#include <mutex>
#include <wrl.h>
#include <WebView2.h>

#include "ChatAgentDefines.h"

#include "ChatTitleMenu.h"


class CDataPacket;


// 回调函数类型定义
using WebViewNavigationCompletedCallback = std::function<void(bool)>;
using WebViewMessageReceivedCallback = std::function<void(const std::wstring&)>;
using TitlebarMenuUpdateCallback = std::function<void()>;
using FileEditTitleClickedCallback = std::function<void(const std::wstring&)>; // fileEditId
using FileSummarizeClickedCallback = std::function<void(const std::wstring&, const std::wstring&)>; // messageId, filePath
using SymbolLinkClickedCallback = std::function<void(const std::wstring&)>; // symbol
using QuerySymbolLocationsCallback = std::function<void(const std::wstring&, const std::vector<std::wstring>&)>; // messageId, symbols


// 操作记录文件版本定义

// 常量定义

class CChatUi : public CWnd, public IChatUi
{
public:
    // 构造函数和析构函数
    CChatUi();
    virtual ~CChatUi();

    // 创建WebView2控件
    BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);

	bool IsReady()	{		return _IsReady();	}
    
    // 导航方法
    void Navigate(const std::wstring& url);
    void NavigateToString(const std::wstring& htmlContent);
    void Reload();
    
    // 脚本执行
    void ExecuteScript(const std::wstring& script, 
                      std::function<void(const std::wstring&)> callback = nullptr);
    
    // 事件处理和回调设置
    void SetNavigationCompletedCallback(WebViewNavigationCompletedCallback callback);
    void SetWebMessageReceivedCallback(WebViewMessageReceivedCallback callback);
    void SetTitlebarMenuUpdateCallback(TitlebarMenuUpdateCallback callback);
    void SetFileEditTitleClickedCallback(FileEditTitleClickedCallback callback);
    void SetTitleMenuItemClickedCallback(TitleMenuItemClickedCallback callback);
	void SetFileSummarizeClickedCallback(FileSummarizeClickedCallback callback);
	void SetSymbolLinkClickedCallback(SymbolLinkClickedCallback callback);
	void SetQuerySymbolLocationsCallback(QuerySymbolLocationsCallback callback);

	// CLI 输入相关方法
	void SendCliInput(const std::wstring& cliId, const std::wstring& input);
	bool GetCliInput(const std::wstring& cliId, std::wstring& input);
	bool HasCliInput(const std::wstring& cliId);
	void ClearCliInput();

	// Question/Answer 相关方法
	__int64 AddQuestion(const std::wstring& messageId, const std::wstring& question, const std::vector<std::wstring>& options) override;
	bool GetQuestionAnswer(__int64 questionId, std::wstring& answer) override;
	bool HasQuestionAnswer(__int64 questionId) override;
	void ClearQuestion() override;
	
	// Question Display 方法 - 显示问题和答案
	void AddQuestionDisplay(const std::wstring& messageId, const std::wstring& question, const std::wstring& answer) override;

	// CLI Display 方法 - 添加 CLI 命令显示
	void AddCliDisplay(const std::wstring& messageId, const std::wstring& cliId, const std::wstring& command, const std::wstring& desc = L"", bool isPending = false, const std::wstring& shellType = L"") override;
	bool IsCliPending(const std::wstring& cliId) override;
	void RemovePendingCli(const std::wstring& cliId) override;
	CliStatus GetCliStatus(const std::wstring& cliId) override;
	void SetCliStatus(const std::wstring& cliId, CliStatus status) override;

    
    // 添加Web消息处理
	void PostJsonMessage(const std::wstring& message) override;

	// 应用 Symbol 链接样式
	// symbolsWithResults: vector<pair<symbol, resultsJson>>
	// resultsJson 格式: [{"filePath":"xxx","lineNumber":123},...]
	void ApplySymbolLinks(const std::wstring& messageId, const std::vector<std::pair<std::wstring, std::wstring>>& symbolsWithResults) override;

	void ActivateCheckpointFileChange(const std::wstring& fileEditId) override;

	// 显示/隐藏暂停状态边框
	// flow: true = 流动动画, false = 静止
	void ShowPause(bool bShow, bool bFlow = true) override;

	// 停止/恢复暂停边框的流动动画
	void StopPauseFlow(bool bStop) override;


    // 获取WebView2环境和核心WebView
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }
    
    // 调整WebView2大小
    void ResizeWebView();

    // ===== 聊天功能相关方法 =====
    
    // 初始化聊天界面
    void InitializeChatUI();

    
    // 设置主题 (light/dark)
    void SetTheme(const std::wstring& theme);

    
    // 收集页面中所有消息的 symbols
    void CollectSymbols();
    
    // ===== 标题栏功能相关方法 =====
    
    
	// 添加标题栏菜单项
    void AddTitlebarMenuItem(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp);
  
    // 清空所有标题栏菜单项
    void ClearTitlebarMenuItems();
    
    // 显示/隐藏标题栏菜单
    void ShowTitlebarMenu();
    void HideTitlebarMenu();
    void ToggleTitlebarMenu();



protected:
    // 初始化WebView2环境
    HRESULT InitializeWebView();
    
    // 消息处理函数
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    
    DECLARE_MESSAGE_MAP()

private:
    // WebView2相关组件，使用普通指针替代wil::co_ptr
    ICoreWebView2Environment* _webViewEnvironment;
    ICoreWebView2* _webView;
    ICoreWebView2Controller* _controller;
    
    // 事件处理Token
    EventRegistrationToken _navigationCompletedToken = {};
    EventRegistrationToken _webMessageReceivedToken = {};
	EventRegistrationToken _processFailedToken = {};

    // 回调函数
    WebViewNavigationCompletedCallback _navigationCompletedCallback;
    WebViewMessageReceivedCallback _webMessageReceivedCallback;
    TitlebarMenuUpdateCallback _titlebarMenuUpdateCallback;
    FileEditTitleClickedCallback _fileEditTitleClickedCallback;
    FileSummarizeClickedCallback _fileSummarizeClickedCallback;
    SymbolLinkClickedCallback _symbolLinkClickedCallback;
    QuerySymbolLocationsCallback _querySymbolLocationsCallback;
    
    // 脚本执行回调映射
    std::map<int, std::function<void(const std::wstring&)>> _scriptCallbacks;
    int _callbackId;
    
    // 状态标志
    bool _isWebViewCreated;
    bool _isChatInitialized;
    
    // 标题栏相关数据
    std::wstring _title;
    
    // 检查WebView和Chat是否已初始化
    bool _IsReady() const;
    
	// Title Menu
	CChatTitleMenu _titleMenuWindow;
	TitleMenuItemClickedCallback _titleMenuItemClickedCallback;

	// Question/Answer 数据结构
	struct QuestionData
	{
		__int64 id;
		std::wstring question;
		std::vector<std::wstring> options;
		std::wstring answer;
		bool hasAnswer;
	};
	QuestionData _currentQuestion;
	std::mutex _questionMutex;

	// CLI 输入数据结构
	struct CliInputData
	{
		std::wstring cliId;
		std::wstring input;
		bool hasInput;
	};
	CliInputData _currentCliInput;
	std::mutex _cliInputMutex;

	// CLI 状态管理
	std::map<std::wstring, CliStatus> _cliStatus;
	std::mutex _cliStatusMutex;

};
