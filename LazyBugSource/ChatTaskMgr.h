#pragma once
#include <vector>
#include <memory>
#include <string>

#include "LlmChat.h"
#include "LlmLib.h"

#include "Utils.h"

// 前置声明
class CLlmChat;
class CChatTask_FastApply;
class CChatTask_VerifyLlmApiProvider;
class CChatTask_CompressSummarize;
class CChatTask_AddMcpServer;

class CChatFileWriter;

class CChatDialog;
class CChatDialogA;
class CChatCtrl;
class CCheckpoints;
class CChatSettingPage;
class CChatAgent;
class CChatOpsCtrl;
class IChatUi;

// 前置声明 SymbolLinkItem 结构体
struct SymbolLinkItem;

// 常量定义
const size_t MAX_LLMCHAT_POOL_SIZE = 10;

extern const char* FILE_EDIT_RESULT_ERROR_PREFIX;

struct ChatTaskContext
{
	ChatTaskContext()
	{
		fileWriter = nullptr;
		chatSettingPage = nullptr;
		chatOpsCtrl = nullptr;
		chatAgent = nullptr;
		chatUi = nullptr;
		chatDialogA = nullptr;
	}

	CChatAgent* chatAgent;
	CChatOpsCtrl* chatOpsCtrl;
	IChatUi* chatUi;

	CChatFileWriter* fileWriter;
	CChatSettingPage* chatSettingPage;
	CChatDialogA* chatDialogA;
};

enum class TaskStatus
{
	Pending,
	Running,
	Success,
	Failure
};

enum class CompressSummarizeMode
{
	Normal,      // workingOpIndex 对应 CChatOpsCompress::_workingOps
	Evaluation,  // 评估模式：只写日志，不写回结果
	Immediate,   // 立即模式：workingOpIndex 直接对应 CChatOpsCtrl::GetOps() 的索引，完成后写回对应 op
};

class CChatTask
{
public:
	CChatTask();
	virtual ~CChatTask() = default; 
	
	virtual const char* GetType() = 0;
	virtual void Start() = 0;
	virtual int GetLlmSessionCount()	{		return 0;	}
	bool IsFinished() { return _status == TaskStatus::Success || _status == TaskStatus::Failure; }
	virtual void Update() {} // 每帧更新，可选实现
	virtual void Interrupt() {} // 中断任务，可选实现
	
	virtual bool DependsOn(CChatTask* task)	{		return false;	}

	bool CheckType(const char* tp);

	const LlmToolCall& GetToolCall()	{		return _toolCall;	}
	void SetToolCall(const LlmToolCall&toolCall)	{		_toolCall = toolCall;	}

	//utility functions
	bool _SaveFileEditResult(const std::string& filePath, const std::string oldContent, const std::string newContent, Utils::FileContentCodingFormat codingFmt,const std::wstring& fileEditId, std::string& errorMsg);
	void _SendToolCallResult(const char* result, const char* resultPartial = nullptr, const LlmToolCall* toolCallPartial = nullptr, const char* resultFullCompress = nullptr, const LlmToolCall* toolCallFullCompress = nullptr);
	void _SendToolCallMessage_Exploring(const char* result);
	void _SendToolCallMessage_AddMcpServer(const char* result);
	void _RemoveToolCallMessage_AddMcpServer();


	ChatTaskContext* _context;
	TaskStatus _status;
	std::vector<CLlmChat*> _llmChats;

	LlmToolCall _toolCall;
};

class CChatTaskMgr
{
public:
	CChatTaskMgr();
	~CChatTaskMgr();
	
	bool Init(const ChatTaskContext& context);
	void Update();
	void Shutdown();
	bool IsRunning() const;
	
	// 中断所有任务
	void Interrupt();
	
	// 添加特定类型的任务
	void AddTask_FastApply(const std::string& filePath, const std::string& updateContent,const std::wstring &fileEditId);
	void AddTask_VerifyLlmApiProvider(const LlmApiProviderTypeName& providerTypeName);
	void AddTask_ReplaceInFile(const LlmToolCall &toolCall, const std::wstring& fileEditId);
	void AddTask_ReplaceInFile(const std::string& filePath, const std::string& oldLines, const std::string& newLines, const std::wstring& fileEditId);
	void AddTask_FindSymbolDefine(const LlmToolCall& toolCall);
	void AddTask_FindInFiles(const LlmToolCall& toolCall);
	void AddTask_SearchFile(const LlmToolCall& toolCall);
	void AddTask_ReadFile(const LlmToolCall& toolCall);
	void AddTask_CLI_Cmd(const LlmToolCall& toolCall);
	void AddTask_CLI_Bash(const LlmToolCall& toolCall);
	void AddTask_CLI_RunScript(const LlmToolCall& toolCall);
	void AddTask_Question(const LlmToolCall& toolCall);
	void AddTask_QueryFinish(const LlmToolCall& toolCall);
	void AddTask_ResolveSymbolLinks(const std::vector<SymbolLinkItem>& symbolLinks);
	void AddTask_CreateSkill(const LlmToolCall& toolCall);
	void AddTask_AddMcpServer(const LlmToolCall& toolCall);
	void AddTask_Mcp(const LlmToolCall& toolCall);
	void AddTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName, int originalTokenCount, CompressSummarizeMode mode = CompressSummarizeMode::Normal);
	void AddTask_InputHint(const std::wstring& content, const std::string& apiName, const CRect& anchorRect, int caretTokenPos, int contentVersion);
	void AddTask_InputHint2(const std::wstring& content, const std::string& apiName, const CRect& anchorRect, int caretTokenPos, int contentVersion);
	void AddTask_InputHint3(const std::wstring& content, const std::string& apiName, const CRect& anchorRect, int caretTokenPos, int contentVersion);

	void UpdateToolCalls(std::vector<LlmToolCall>& toolCalls);

	// 获取任务统计信息
	size_t GetPendingTaskCount() const { return _pending.size(); }
	size_t GetRunningTaskCount() const { return _running.size(); }
	
	// 检查特定类型的任务是否正在运行
	bool IsTaskTypeRunning(const char* taskType) const;
	
	// 中断指定类型的所有任务
	void InterruptTaskType(const char* taskType);

	// 设置单任务执行模式
	void SetSingleTaskMode(bool enable) { _singleTaskMode = enable; }
	bool IsSingleTaskMode() const { return _singleTaskMode; }
	
	// LlmChat pool相关
	size_t GetAvailableLlmChatCount() const { return _availableLlmChats.size(); }
	size_t GetUsedLlmChatCount() const { return _usedLlmChats.size(); }

protected:
	void _AddTask(CChatTask* task); // 如果不能立即执行,则放到_pending里,如果可以立即执行,则放到_running里
	TaskStatus _CheckPendingTaskNextStep(CChatTask* task); // 根据当前正在_running和_pending的task,以及它们的类型,判断这个task是否可以立即执行,或者立即失败,或者继续pending
	void _StartTask(CChatTask* task);
	void _RemoveFinishedTasks();
	void _ProcessPendingTasks();

	// 获取实际允许的最大并发任务数
	size_t _GetMaxConcurrentTasks() const;
	
	// LlmChat pool管理
	CLlmChat* _AllocateLlmChat();
	void _ReleaseLlmChat(CLlmChat* llmChat);
	void _InitLlmChatPool();
	void _DestroyLlmChatPool();
	
	std::vector<CChatTask*> _pending;
	std::vector<CChatTask*> _running;
	
	// LlmChat pool
	CLlmChat _llmChatPool[MAX_LLMCHAT_POOL_SIZE]; // 定长实例数组
	std::vector<CLlmChat*> _availableLlmChats;    // 可用的LlmChat指针
	std::vector<CLlmChat*> _usedLlmChats;         // 已分配的LlmChat指针
	
	ChatTaskContext _context; // 复制保存的上下文实例
	bool _isInitialized;
	
	// 最大并发任务数
	size_t _maxConcurrentTasks;
	
	// 单任务执行模式标志
	bool _singleTaskMode; // true表示同一时刻只能执行一个任务
};