#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

#include "ChatInputTag.h"
#include "ChatInputACList.h"
#include "ChatInputTagMenuWindow.h"
#include "ChatLlmMenu.h"
#include "ChatInputImageTip.h"

// 回调函数类型定义
using InputSendCallback = std::function<void(const std::wstring&, const std::wstring&)>; // contentJson, plainText
using InputToolButtonClickedCallback = std::function<void(const std::wstring&, const std::wstring&)>; // buttonId, action
using InputContentChangedCallback = std::function<void(const std::wstring&)>;
using InputTagRemovedCallback = std::function<void(const std::wstring&)>; // tagId
using InputAutoCompleteRequestCallback = std::function<void(const std::wstring&)>; // 请求自动补全候选项
using InputPageNavigationCallback = std::function<void(bool, const std::wstring&)>; // isPrevious - true为Page Up，false为Page Down, currentContent - 当前输入内容(与OnSendMessage的content格式一致)
using InputStopButtonClickedCallback = std::function<void()>; // stop按钮点击回调
using InputMajorChatApiChangedCallback = std::function<void(const std::wstring&)>; // majorChatApi changed callback
using InputTagClickedCallback = std::function<void(const std::wstring&)>; // tagId
using InputEscapeCallback = std::function<void()>; // Esc键按下回调
using InputReadyCallback = std::function<void()>; // WebView和Input准备就绪回调
using InputFilePastedCallback = std::function<void(const std::wstring&)>; // 文件粘贴回调，参数为文件类型：L"files"或L"image"
using InputSkillButtonClickedCallback = std::function<void(const RECT&)>; // Skill按钮点击回调，参数为按钮屏幕绝对坐标
using InputMcpButtonClickedCallback = std::function<void(const RECT&)>; // MCP按钮点击回调，参数为按钮屏幕绝对坐标
using InputCompressIntensityChangedCallback = std::function<void(int)>; // 压缩强度改变回调，参数为强度值(0-5)

// 工具栏按钮结构
struct ChatInputToolButton
{
    std::wstring id;          // 按钮唯一ID
    std::wstring text;        // 按钮文字
    std::wstring icon;        // 按钮图标
    std::wstring action;      // 按钮动作
    std::wstring tooltip;     // 提示文字
    bool enabled;             // 是否启用
};

class CChatInput : public CWnd
{
public:
    // 构造函数和析构函数
    CChatInput();
    virtual ~CChatInput();

    // 创建WebView2控件
    BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);

	bool IsReady()	{		return _IsReady();	}

	void Update();
    
    // 导航方法
    void Navigate(const std::wstring& url);
    void Reload();
    
    // 脚本执行
    void ExecuteScript(const std::wstring& script, 
                      std::function<void(const std::wstring&)> callback = nullptr);
    
// 事件处理和回调设置
    void SetSendCallback(InputSendCallback callback);
    void SetToolButtonClickedCallback(InputToolButtonClickedCallback callback);
    void SetContentChangedCallback(InputContentChangedCallback callback);
    void SetTagRemovedCallback(InputTagRemovedCallback callback);
    void SetAutoCompleteRequestCallback(InputAutoCompleteRequestCallback callback);
    void SetPageNavigationCallback(InputPageNavigationCallback callback);
    void SetStopButtonClickedCallback(InputStopButtonClickedCallback callback);
    void SetMajorChatApiChangedCallback(InputMajorChatApiChangedCallback callback);
    void SetTagClickedCallback(InputTagClickedCallback callback);
    void SetEscapeCallback(InputEscapeCallback callback);
    void SetReadyCallback(InputReadyCallback callback);
    void SetFilePastedCallback(InputFilePastedCallback callback);
    void SetSkillButtonClickedCallback(InputSkillButtonClickedCallback callback);
    void SetMcpButtonClickedCallback(InputMcpButtonClickedCallback callback);
    void SetCompressIntensityChangedCallback(InputCompressIntensityChangedCallback callback);
    
    // 获取WebView2环境和核心WebView
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }
    
    // 调整WebView2大小
    void ResizeWebView();

    // ===== 输入内容相关方法 =====
    
    // 初始化输入界面
    void InitializeInputUI();
    
    // 获取和设置输入内容
    void GetInputContent(std::function<void(const std::wstring&)> callback);  // 获取包含标签信息的完整内容
    void GetInputPlainText(std::function<void(const std::wstring&)> callback); // 获取纯文本内容
    void SetInputContent_(const std::wstring& content);  // 设置包含标签信息的完整内容
    void ClearInput_();
    
    // 插入文本到光标位置
    void InsertText(const std::wstring& text);
    
    // 插入内联标签到光标位置
    void InsertInlineTag(const std::wstring& text, const std::wstring& type = L"info", 
                        const std::wstring& data = L"");

    // 解析输入内容中的标签信息
    void ParseInlineTags(const std::wstring& inputContent, std::vector<ChatInputTag>& tags);
    
    
    // 获取选中文本
    void GetSelectedText(std::function<void(const std::wstring&)> callback);

    // ===== 标签栏相关方法 =====

	void AddFilePathTag(const char* fullPath,bool isVisible)	{		_AddFilePathTag(fullPath,isVisible);	}
	void AddFilePathTag(const wchar_t* fullPath,bool isVisible);
	
	// 处理粘贴操作（从剪贴板读取文件路径并添加为标签）
	// 返回值：true 表示已处理，false 表示未处理（调用者应执行默认粘贴行为）
	bool HandlePaste();

    // 添加标签到标签栏
    std::wstring AddTag(const std::wstring& text, const std::wstring& type = L"info", 
                       const std::wstring& path= L"", const std::wstring& color = L"", 
                       bool removable = true, bool visible = true);
    
    // 移除标签（真正删除）
    void RemoveTag(const std::wstring& tagId);
    
    // 隐藏标签（设置为不可见）
    void HideTag(const std::wstring& tagId);
    
    // 清空所有标签
    void ClearTags();
    
    // 获取所有标签（包括不可见的）
    const std::vector<ChatInputTag>& GetTags() const { return _tags; }
    
    // 获取可见的标签
	std::vector<ChatInputTag> GetVisibleFileTags() const;

    // 检查标签是否存在
    bool HasTag(const std::wstring& tagId) const;
    
    // 设置标签可见性
    void SetTagVisible(const std::wstring& tagId, bool visible);

    // ===== 工具栏相关方法 =====
    
    // 添加工具按钮
    std::wstring AddToolButton(const std::wstring& text, const std::wstring& icon,
                              const std::wstring& action, const std::wstring& tooltip = L"");
    
    // 移除工具按钮
    void RemoveToolButton(const std::wstring& buttonId);
    
    // 设置按钮启用状态
    void SetToolButtonEnabled(const std::wstring& buttonId, bool enabled);
    
    // 清空所有工具按钮
    void ClearToolButtons();

    // ===== 发送按钮相关方法 =====
    
    // 设置发送按钮启用状态
    void SetSendButtonEnabled(bool enabled);
    
    // 设置发送按钮文字
    void SetSendButtonText(const std::wstring& text);
    
    // ===== Stop按钮相关方法 =====
    
    // 显示/隐藏stop按钮
    void ShowStopButton();
    void HideStopButton();

    // ===== 压缩结果提示相关方法 =====
    
    // 显示压缩结果提示
    // success: 是否成功
    // message: 显示的消息
    // logPath: 日志文件路径（成功时显示链接）
    void ShowCompressSummarizeTip(bool success, const std::wstring& message, const std::wstring& logPath = L"");
    
    // 隐藏压缩结果提示
    void HideCompressSummarizeTip();

    // ===== 外观设置方法 =====
    
    // 设置占位符文字
    void SetPlaceholder(const std::wstring& placeholder);


    // 设置压缩强度 (0=None, 1=Low, 2=Medium, 3=High, 4=Extreme)
    bool SetCompressIntensity(int intensity, const std::wstring& tooltip);

    // 设置压缩后大小显示 (如 "18K", "1.21M", "0B" 等)
    bool SetCompressedSize(const std::wstring& sizeText, const std::wstring& tooltip);

    // 开始 Context Level 按钮的流光效果
    void StartContextLevelFlowing();

    // 停止 Context Level 按钮的流光效果
    void StopContextLevelFlowing();

    // ===== Agent API 相关方法 =====
    
    // 获取当前的Agent API
    std::wstring GetCurrentMajorChatApi() const;
    
    // 设置当前的Agent API
    void SetCurrentMajorChatApi(const std::wstring& apiName);
    
    // 更新API菜单显示
    void UpdateMajorChatApiMenu();
    
    // 显示/隐藏LLM菜单
    void ShowLlmMenu(int x, int y);
    void HideLlmMenu();

    // ===== 自动补全相关方法 =====
    
    // 获取自动补全列表
    CChatInputACList* GetAutoCompleteList() { return &_autoCompleteList; }
    
    
    // 启用/禁用自动补全
    void SetAutoCompleteEnabled(bool enabled) { _autoCompleteEnabled = enabled; }
    
    // 占据焦点
	void RequestOccupyFocus() { _requestGainFocus = true; }
	void OccupyFocus();
	void FocusEditor();

	void WaitTillWebViewReady();

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
    EventRegistrationToken _acceleratorKeyPressedToken = {};
    
    // 回调函数
    InputSendCallback _sendCallback;
    InputToolButtonClickedCallback _toolButtonClickedCallback;
    InputContentChangedCallback _contentChangedCallback;
    InputTagRemovedCallback _tagRemovedCallback;
    InputAutoCompleteRequestCallback _autoCompleteRequestCallback;
    InputPageNavigationCallback _pageNavigationCallback;
    InputStopButtonClickedCallback _stopButtonClickedCallback;
    InputMajorChatApiChangedCallback _majorChatApiChangedCallback;
    InputTagClickedCallback _tagClickedCallback;
    InputEscapeCallback _escapeCallback;
    InputReadyCallback _readyCallback;
    InputFilePastedCallback _filePastedCallback;
    InputSkillButtonClickedCallback _skillButtonClickedCallback;
    InputMcpButtonClickedCallback _mcpButtonClickedCallback;
    InputCompressIntensityChangedCallback _compressIntensityChangedCallback;
    
    // 脚本执行回调映射
    std::map<int, std::function<void(const std::wstring&)>> _scriptCallbacks;
    int _callbackId;
    
    // 状态标志
    bool _isWebViewCreated;
    bool _isInputInitialized;

	//Focus控制
	//如果当前为foreground,并且上一次拥有focus的时间和上一次是前台窗口的时间很接近的话,表示自己是因为窗口切到后台丢失的focus,这种情况下要夺回focus
	void _UpdateGainFocus();
	DWORD _lastTimeOwnFocus;
	DWORD _lastTimeForeground;
	bool _wasForeground;
	bool _requestGainFocus;

	void _GenClipboardImageName(std::string& name, std::wstring& path);

    
    // 标签相关数据
	void _AddFilePathTag(const char* fullPath,bool isVisible);
	std::vector<ChatInputTag> _tags;
    
    // 工具按钮相关数据
    std::vector<ChatInputToolButton> _toolButtons;
    
    // 自动补全相关
    CChatInputACList _autoCompleteList;
    bool _autoCompleteEnabled;
    std::wstring _autoCompletePrefix; // 当前自动补全的前缀（@符号的位置到当前光标）

    bool _contextLevelFlowing = false; // Context Level 按钮流光效果是否开启

    
    // 标签菜单相关
    CChatInputTagMenuWindow _tagMenuWindow;
    
    // LLM菜单相关
    CChatLlmMenu _llmMenuWindow;

    // 图片 Tag 预览提示窗口
    CChatInputImageTip _imageTipWindow;

    // 图片 Tag 悬停状态（用于轮询检测）
    std::wstring _currentHoveredImageFilePath;
    
    // 生成唯一ID的辅助方法
    std::wstring _GenTagId();
    std::wstring _GenButtonId();
    
    // 检查WebView和Input是否已初始化
    bool _IsReady() const;
    
    // 查找标签和按钮的辅助方法
    ChatInputTag* _FindTag(const std::wstring& tagId);
    const ChatInputTag* _FindTag(const std::wstring& tagId) const;
    ChatInputToolButton* _FindToolButton(const std::wstring& buttonId);
    
    // 发送消息到WebView的辅助方法
    void _SendTagsUpdateMessage();
    void _SendToolButtonsUpdateMessage();
    void _PostWebMessageAsJson(const std::wstring& message);
    
    // 生成标签JSON的辅助方法
    std::wstring _BuildImgSrc(const std::wstring& type, const std::wstring& data);
    std::wstring _BuildTagJson(const std::wstring& text, const std::wstring& type, const std::wstring& data);
    
    // 处理粘贴的文件路径（公共逻辑）
    void _ProcessPastedFilePath(const wchar_t* filePath);
    
    // 自动补全事件处理方法
    void OnAutoCompleteItemSelected(const ChatInputACItem& item);
    void OnAutoCompleteCancelled();
};
