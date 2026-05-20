#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <fstream>
#include <wrl.h>
#include <WebView2.h>

#include "Checkpoints.h"

#include "ChatInputTag.h"

#include "ChatTitleMenu.h"

#include "LlmTools.h"

class CDataPacket;

typedef WUID FileChangeListUID;
#define FileChangeListUID_Invalid (0)

// 回调函数类型定义
using WebViewNavigationCompletedCallback = std::function<void(bool)>;
using WebViewMessageReceivedCallback = std::function<void(const std::wstring&)>;
using TitlebarMenuUpdateCallback = std::function<void()>;
using FileEditTitleClickedCallback = std::function<void(const std::wstring&)>; // fileEditId
using SettingsButtonClickedCallback = std::function<void()>; // 设置按钮点击回调
using TocButtonClickedCallback = std::function<void()>; // 目录按钮点击回调
using FileSummarizeClickedCallback = std::function<void(const std::wstring&, const std::wstring&)>; // messageId, filePath
using SymbolLinkClickedCallback = std::function<void(const std::wstring&)>; // symbol
using QuerySymbolLocationsCallback = std::function<void(const std::wstring&, const std::vector<std::wstring>&)>; // messageId, symbols


// 操作记录文件版本定义
#define CHATCTRL_OPERATIONS_VERSION_1_0  0x00010000  // 初始版本
#define CHATCTRL_OPERATIONS_VERSION_1_1  0x00010001  
#define CHATCTRL_OPERATIONS_VERSION_1_2  0x00010002  
#define CHATCTRL_OPERATIONS_VERSION_1_3  0x00010003  
#define CHATCTRL_OPERATIONS_VERSION_CURRENT CHATCTRL_OPERATIONS_VERSION_1_3

// 常量定义
#define DEFAULT_CHAT_TITLE L"[ Untitled Chat ]"

// FileEdit 窗口按钮结构
struct ChatCtrlFileEditBtn 
{
    std::wstring text;      // 按钮文字
    std::wstring action;    // 按钮动作标识
    std::wstring id;        // 按钮唯一ID
};

// FileEdit 窗口结构
struct ChatCtrlFileEdit 
{
    std::wstring id;                          // 窗口唯一ID
    std::wstring title;                       // 标题栏文字
    std::wstring fullPath;                    // 文件完整路径
    std::wstring content;                     // 显示区域内容
	std::wstring diffContent;                     // Diff内容
    std::wstring messageId;                   // 所属的AI消息ID
    std::vector<ChatCtrlFileEditBtn> buttons;      // 标题栏按钮列表（不包括大小调整按钮）
    bool isCollapsed;                         // 是否折叠
};


struct ChatCtrlOp
{
    enum Type
    {
        Op_AddUserMessage,
        Op_StartStreamingAIMessage,
        Op_AddStreamingAIMessage,
        Op_CompleteStreamingAIMessage,
        Op_AddSystemMessage,
        Op_AddFileEditToAIMessage,
        Op_SetFileEditTitle,
        Op_SetFileEditContent,
        Op_SetTitle,
        Op_BeginSession,
        Op_EndSession,
        Op_AddSessionTag,
		Op_AddSessionDisabledTag,
		Op_DisableMessagesAfter,
        Op_SetSessionCost,
		Op_FileAttaches,
		Op_AddToolCallResult,//call and result
		Op_AddStreamingAIMessage_Thinking,
		Op_AddReplaceInFileResult,
		Op_AddToolCallMessage,
		Op_AddFileSummarizeToAIMessage,
		//重要,后添加的Op一定要加在末尾
	};

    void Save(CDataPacket& dp);
	void Load(CDataPacket& dp,DWORD ver);
	void Load(std::ifstream& file, DWORD ver);

    Type type;
    

    // Op_AddUserMessage, Op_StartStreamingAIMessage, Op_AddStreamingAIMessage, 
	// Op_AddStreamingAIMessage_Thinking, Op_AddToolCallMessage,Op_CompleteStreamingAIMessage, Op_AddSystemMessage, 
	// Op_AddFileEditToAIMessage, Op_DisableMessagesAfter, Op_SetSessionCost: 消息ID
    std::wstring messageId;

    // Op_AddUserMessage: 用户消息内容
    // Op_AddStreamingAIMessage: AI流式消息增量内容
    // Op_AddStreamingAIMessage_Thinking: AI流式thinking消息增量内容
    // Op_AddSystemMessage: 系统消息内容
    // Op_AddFileEditToAIMessage: FileEdit窗口的初始内容
    // Op_SetFileEditContent: FileEdit窗口的新内容
    // Op_AddSessionTag, Op_AddSessionDisabledTag: Tag的显示文本
    // Op_SetSessionCost: 格式化后的费用字符串
	// Op_AddToolCallResult: 一次tool call(包括调用和结果)的json字串
	// Op_FileAttaches: 一个文件完整路径列表,以"|"分隔
	// Op_AddReplaceInFileResult: 描述一次ReplaceInFile操作的结果,包含具体的修改内容
    // Op_AddToolCallMessage: tool call结束后显示的文字信息内容，包含在当前AI消息流中
    std::wstring content;

    // Op_AddFileEditToAIMessage, Op_SetFileEditTitle, Op_SetFileEditContent: FileEdit窗口ID
    std::wstring fileEditId;

    // Op_AddFileEditToAIMessage: FileEdit窗口的初始标题
    // Op_SetFileEditTitle: FileEdit窗口的新标题
    // Op_SetTitle: 聊天窗口的新标题
    std::wstring title;

    // Op_AddFileEditToAIMessage: FileEdit中文件的完整路径
    // Op_AddSessionTag, Op_AddSessionDisabledTag: Tag关联的文件路径
	// Op_AddReplaceInFileResult: 记录对哪个文件修改
    std::wstring fullPath;

    // Op_SetFileEditContent: FileEdit窗口的Diff内容
    std::wstring diffContent;
    
    // Op_BeginSession: 会话开始时文件状态的Checkpoint ID
    // Op_SetFileEditContent: 文件内容变更后的Checkpoint ID
    // Op_FileAttaches: 附件内容在哪个Checkpoint ID里
    FilesCheckpointUID checkpointId;
    
    // 构造函数
    ChatCtrlOp() : type(Op_AddUserMessage)
	{
		checkpointId = FilesCheckpointUID_Invalid;
	}
    
    ChatCtrlOp(Type t) : type(t)
	{
		checkpointId = FilesCheckpointUID_Invalid;
	}
};

struct LlmSessionUsage;
struct LlmSessionRequest;
class CChatCtrl : public CWnd
{
public:
    // 构造函数和析构函数
    CChatCtrl();
    virtual ~CChatCtrl();

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
    void SetSettingsButtonClickedCallback(SettingsButtonClickedCallback callback);
    void SetTocButtonClickedCallback(TocButtonClickedCallback callback);
    void SetTitleMenuItemClickedCallback(TitleMenuItemClickedCallback callback);

    
    // 添加Web消息处理
    void PostWebMessageAsJson(const std::wstring& message);
    
    // 获取WebView2环境和核心WebView
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }
    
    // 调整WebView2大小
    void ResizeWebView();

    // ===== 聊天功能相关方法 =====
    
    // 初始化聊天界面
    void InitializeChatUI();

	//fileAttaches为一个Op_FileAttaches的索引.这个函数检查它是否可以作为有效的cache
	// 首先检查fileAttaches是否是最后一个未被disable的Op_FileAttaches
	// 再检查这个fileAttaches里的文件列表是否可以cover visibleTags里的所有文件
	// 还会检查fileAttaches有没有一个有效的checkpoint,
	//TODO:要考虑这些文件有没有被外部改动过,如果某个文件被外部改动过,则这些Op全部无效,返回-1
	//所谓外部改动过是指没有被这个chat 里的checkpoint记录下的改动
	bool CheckValidFileAttachesCache(int fileAttaches,const std::vector<ChatInputTag>& visibleTags);

    //搜集聊天记录里的旧的信息,到一个Session Request
	//按照顺序搜集	Op_AddUserMessage,	Op_AddStreamingAIMessage,	Op_FileAttaches,	Op_AddToolCallResult里的信息,填充到request里
	//只处理fileAttaches 指定的Op_FileAttaches
	// attach file的内容从会尝试优先从Op_FileAttaches里的checkpoint里获取
	bool MakeSessionRequest(LlmSessionRequest& request,int fileAttaches);
	bool MakeSessionRequest_Debug(LlmSessionRequest& request);

    // 添加用户消息
    void AddUserMessage(const std::wstring& message, const std::wstring& overrideMessageId = L"");
    
    // 获取用户消息内容
    bool GetUserMessageContent(const std::wstring& messageId, std::wstring& content) const;
    
    // 开始新的AI流式消息
    std::wstring StartStreamingAIMessage(const std::wstring& overrideMessageId = L"");
    

    // 添加AI流式消息增量内容
    void AddStreamingAIMessage(const std::wstring& messageId, const std::wstring& incrementalContent);
    
    // 添加AI流式thinking消息增量内容（小字体、浅灰色）
    void AddStreamingAIMessage_Thinking(const std::wstring& messageId, const std::wstring& incrementalContent);

	// 添加ToolCall消息（浅紫色显示，包含在当前AI消息流中）
	void AddToolCallMessage(const std::wstring& messageId, const std::wstring& message);

    // 完成流式AI消息
    void CompleteStreamingAIMessage(const std::wstring& messageId);
    
    // 添加系统消息
    void AddSystemMessage(const std::wstring& message, const std::wstring& overrideMessageId = L"");

	//File Attaches
	int AddFileAttaches(const std::wstring& fileList, FilesCheckpointUID checkpointId);

    //启动/终止 chat session,用来记录一些原始信息(记录在Op中)
	void BeginSession(FilesCheckpointUID checkpointId);
	void EndSession();//changelistId表示这个session生成的一个changelist
	bool GetRestoreCheckpoints(const std::wstring& userMessageId, std::vector<FilesCheckpointUID>& checkpointIds);//得到要恢复到user messageId之前的状态,需要restore哪些checkpoints,按照自后到前的顺序排列

    //添加attach
    void AddSessionTag(const ChatInputTag &tag);
	void GetLastNotDisabledSessionTags(std::vector< ChatInputTag>& tags) const;
	void GetUserMessageSessionTags(const std::wstring& userMessageId, std::vector< ChatInputTag>& tags) const;

	//添加ToolCall result
	void AddToolCallResult(const std::wstring& jsonString);


	//添加ReplaceInFile的结果,记录了修改的具体内容
	void AddReplaceInFileResult(const std::wstring& filePath, const std::wstring& result);

	//Undo checkpoints
	void SetUndoCheckpoint(FilesCheckpointUID checkpointId);//这是一个undo checkpoint,当用户restore一个之前的checkpoint时,一个undo checkpoint会被生成
	FilesCheckpointUID GetUndoCheckpoint(FilesCheckpointUID& restoredCheckpoint) const 	{ 		restoredCheckpoint = _restoredCheckpointId;		return _undoCheckpointId; 	}

    // 清空聊天记录
    void ClearChat();
    
    //Disable-After 相关
    void DisableMessagesAfter(const std::wstring& messageId);// Disable某个消息之后的所有消息
    void RemoveDisabledSessions(std::vector<FilesCheckpointUID>& checkpointsToDiscard);//删除所有disabled 的 session,并返回需要丢弃的checkpoint
    void EnableAllDisabledMessages();// 启用所有被disabled的消息
    
    // 设置主题 (light/dark)
    void SetTheme(const std::wstring& theme);

	// 得到最近一个session的prompt token数
	int GetRecentPromptToken()	{		return _recentPrompToken;	}

    // ===== Loading Overlay 相关方法 =====
    
    // 显示/隐藏加载遮罩层
    void ShowLoadingOverlay();
    void HideLoadingOverlay();

	//显示文件编辑进行中的标签
	void ShowFileEditProgressLabel(const std::wstring& messageId, const std::wstring& title);
	void HideFileEditProgressLabel(const std::wstring& messageId);

    // ===== FileEdit 内嵌窗口功能相关方法 =====
    
    // 在指定AI消息中添加FileEdit窗口
    std::wstring AddFileEditToAIMessage(const std::wstring& messageId, const std::wstring& title, const std::wstring& fullPath, 
                    const std::wstring& content, const std::wstring& overrideFileEditId = L"");
    
    // 设置FileEdit窗口标题
    void SetFileEditTitle(const std::wstring& fileEditId, const std::wstring& title);
    
    // 设置FileEdit窗口显示内容
    void SetFileEditContent(const std::wstring& fileEditId, const std::wstring& content, const std::wstring& diffContent,FilesCheckpointUID checkpointID);
	//更新FileEdit的diffContent和checkpointId,这个checkpoint里只包含这一个文件,记录这个文件经过这次fileEdit后的状态
	bool UpdateFileEditDiffContent(const std::wstring& fileEditId, const std::wstring& diffContent, FilesCheckpointUID checkpointId);
	//得到FileEdit的checkpoint
	bool GetFileEditCheckpoint(const std::wstring& fileEditId, FilesCheckpointUID &checkpointId)const;
	//得到FileEdit的checkpoint之前的那个checkpoint(这次修改之前的文件状态)
	bool GetFileEditPrevCheckpointInSession(const std::wstring& fileEditId, FilesCheckpointUID& checkpointId,bool &isHead)const;
	//得到FileEdit的在这个session里的第一个checkpoint
	bool GetFileEditCheckpointInSessionBegin(const std::wstring& fileEditId, FilesCheckpointUID& checkpointId)const;
	//得到FileEdit的fullPath
    bool GetFileEditFullPath(const std::wstring& fileEditId, std::wstring& fullPath) const;
    //得到FileEdit的content内容
    bool GetFileEditContent(const std::wstring& fileEditId, std::wstring& content) const;
	//获取某个fileEdit之后的未被disabled的所有FileEdit ID,包括这个fileEdit本身
	void GetNotDisabledFileEditsStartingFrom(const std::wstring& fileEditId,std::vector<std::wstring> &fileEditIds) const;
	//检查某个fileEdit是否在最后的未被Disabled的session里
	bool IsFileEditInLastNotDisabledSession(const std::wstring& fileEditId) const;
	//找到messageId代表的session里的所有file edit的文件路径(如果有有效checkpoint的话),剔除重复路径
	void GetFileEditFilePathesByMessageId(const std::wstring& messageId, std::vector<std::wstring>& filePathes) const;
	//从后向前找到第一个messageId匹配,路径也匹配的,并且checkpoint为有效的fileEdit,返回这个fileEdit的ID
	std::wstring GetLastFileEditCheckpointFromFilePath(const std::wstring& messageId, const std::wstring& fullPath) const;
	//找到FileEdit所在的session的Op_BeginSession,并设置它的checkpoint
	bool SetFileEditHeadCheckpoint(const std::wstring& fileEditId, FilesCheckpointUID checkpointId);
	// 添加FileEdit窗口标题栏按钮
    std::wstring AddFileEditButton(const std::wstring& fileEditId, const std::wstring& buttonText, const std::wstring& buttonAction, const std::wstring& overrideButtonId = L"");


    // 折叠/展开FileEdit窗口
    void ToggleFileEditCollapse(const std::wstring& fileEditId);

    // FileEdit 修改状态控制
    void StartFileEditModification(const std::wstring& fileEditId);
    void StopFileEditModification(const std::wstring& fileEditId);

    // ===== FileSummarize 功能相关方法 =====
    
    // 在指定AI消息中添加FileSummarize按钮
    void AddFileSummarizeToAIMessage(const std::wstring& messageId, const std::wstring& filePath);
    
    // 设置FileSummarize点击回调
    void SetFileSummarizeClickedCallback(FileSummarizeClickedCallback callback);

	// 查找是否存在和这个fileEditId的messageId一致的Summarize
	bool ExistSummarizeInSession(const std::wstring& fileEditId) const;

    // ===== Symbol 链接功能相关方法 =====
    
    // 收集页面中所有消息的 symbols
    void CollectSymbols();
    
    // 应用 Symbol 链接样式
    // symbolsWithResults: vector<pair<symbol, resultsJson>>
    // resultsJson 格式: [{"filePath":"xxx","lineNumber":123},...]
    void ApplySymbolLinks(const std::wstring& messageId, const std::vector<std::pair<std::wstring, std::wstring>>& symbolsWithResults);
    
    // 设置 Symbol 链接点击回调
    void SetSymbolLinkClickedCallback(SymbolLinkClickedCallback callback);
    
    // 设置 Symbol 查询回调
    void SetQuerySymbolLocationsCallback(QuerySymbolLocationsCallback callback);

    // ===== 标题栏功能相关方法 =====
    
    // 设置标题栏标题
    void SetTitle(const std::wstring& title);
    bool HasTitle();
    
	// 添加标题栏菜单项
    void AddTitlebarMenuItem(const std::wstring& menuItemId, const std::wstring& content, const std::wstring& stamp);
  
    // 清空所有标题栏菜单项
    void ClearTitlebarMenuItems();
    
    // 显示/隐藏标题栏菜单
    void ShowTitlebarMenu();
    void HideTitlebarMenu();
    void ToggleTitlebarMenu();

    //=====Session Cost =====
	// 累积并设置会话费用信息
	void AccumulateSessionCostForFileEdit(const std::wstring& fileEditId, float price, int inputToken, int outputToken);
    void AddSessionCost(const LlmSessionUsage &usage, const std::wstring& messageId = L"");
	void NotifyPromptCache(const LlmSessionUsage& usage);


    // ===== 操作记录=====
    void ClearOps();// 清空操作记录
    const std::vector<ChatCtrlOp>& GetOps() const { return _ops; }// 获取操作记录
    size_t GetOpsCount() const { return _ops.size(); }// 获取操作记录数量
	int FindLastOpIndex(ChatCtrlOp::Type tp) const	{		return _FindLastOpIndex(tp);	}

	//Save/Load
	
	DWORD GetVer() const { return _ver; }// 获取版本号
    bool Save(const char* filePath);// 保存内容到文件
    bool Load(const char* filePath);// 从文件加载内容

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
    SettingsButtonClickedCallback _settingsButtonClickedCallback;
    TocButtonClickedCallback _tocButtonClickedCallback;
    FileSummarizeClickedCallback _fileSummarizeClickedCallback;
    SymbolLinkClickedCallback _symbolLinkClickedCallback;
    QuerySymbolLocationsCallback _querySymbolLocationsCallback;
    
    // 脚本执行回调映射
    std::map<int, std::function<void(const std::wstring&)>> _scriptCallbacks;
    int _callbackId;
    
    // 状态标志
    bool _isWebViewCreated;
    bool _isChatInitialized;
    
    // 聊天相关数据
    std::wstring _currentStreamingMessageId;
    
    // FileEdit 相关数据
    std::vector<ChatCtrlFileEdit> _fileEdits;
    
    // 标题栏相关数据
    std::wstring _title;
    
    // 生成唯一消息ID
    std::wstring _GenMsgId();
    
    // 检查WebView和Chat是否已初始化
    bool _IsReady() const;
    
    // FileEdit 相关辅助方法
    
    // 生成唯一FileEdit窗口ID
    std::wstring _GenFileEditId();
    
    // 生成唯一按钮ID
    std::wstring _GenFileEditBtnId();
    
    // 查找FileEdit窗口
    ChatCtrlFileEdit* _FindFileEdit(const std::wstring& fileEditId);
    
    // 发送FileEdit相关消息到WebView
    void _SendFileEditMsg(const std::wstring& action, const ChatCtrlFileEdit& window);
    
    // 构建FileEdit按钮的JSON数组
    std::wstring _BuildButtonsJson(const std::vector<ChatCtrlFileEditBtn>& buttons);

    void _ExecuteOp(const ChatCtrlOp& op);

    //Op 队列
    void _AddOp(const ChatCtrlOp& op);
	ChatCtrlOp* _GetLastOp();
	const ChatCtrlOp* _FindLastOp(ChatCtrlOp::Type tp) const;
	int _FindLastOpIndex(ChatCtrlOp::Type tp) const;
	int _FindFirstOpIndexInSession(int sessionBeginIdx,ChatCtrlOp::Type tp) const;

    // 内部函数：设置会话费用显示（不记录操作，供_ExecuteOp调用）
    void _SetSessionCostDisplay(const std::wstring& costText, const std::wstring& messageId = L"");

	//得到某个op所在的session begin
	int _GetSessionBeginOfOpIndex(int idx) const;

    // 找到fileEdit所在的 session begin
    int _GetSessionBeginOfFileEdit(const std::wstring& fileEditId) const;
	int _FindFileEditOpIndex(const std::wstring& fileEditId) const;

    //找到userMessage所在的 session begin
    int _GetSessionBeginOfUserMessage(const std::wstring& messageId) const;
    
    // 查找 DisableMessagesAfter 操作的位置，返回被 disable-after 的消息在操作队列中的索引
    int _GetDisableAfterIndex() const;
    int _GetLastNotDisabledSessionBegin()const;
	int _GetLastNotDisabledSessionEnd()const;

	//得到Op_FileAttaches里的文件列表
	void _GetFileAttachesList(int fileAttaches, std::vector<std::wstring>& filePathes);
	void _GetFileAttachesList(int fileAttaches, std::unordered_set<std::wstring>& filePathes);

	//在一定范围内[startIdx,endIdx)的op里,找最后一个包含某个文件的checkpoint,如果找到,则读出这个文件在checkpoint里的修改时间(被加入checkpoint时该文件的修改时间)
	bool _GetLastFileTimeInCheckpoint(const std::wstring& fullPath, int startIdx,int endIdx,AbsTick &t);

	//Session Tag
	void _AddSessionTag(const std::wstring& text,const std::wstring& path, bool enabled);
	void _CollectSessionTags(int sessionBeginIndex, int sessionEndIndex, std::vector<ChatInputTag>& tags) const;

	// Title Menu
	CChatTitleMenu _titleMenuWindow;
	TitleMenuItemClickedCallback _titleMenuItemClickedCallback;

    std::vector<ChatCtrlOp> _ops;
    DWORD _ver;

	FilesCheckpointUID _undoCheckpointId;
	FilesCheckpointUID _restoredCheckpointId;
	int _recentPrompToken;
};
