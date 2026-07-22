#pragma once

#include "ChatAgentDefines.h"

#include "LlmChat.h"
#include "LlmSession.h"
#include "ChatTaskMgr.h"
#include "ChatOpsCtrl.h"
#include "ChatFileWriter.h"
#include "ChatOpsCompress.h"
#include "ChatOpsSummarizer.h"

class CChatAgent
{
public:

	struct LlmSessionContext
	{
		LlmSessionContext()
		{
			Zero();
		}
		void Zero()
		{
			t = 0;
			fileAttaches = -1;
		}

		std::string apiName;                  // 使用哪个 api
		std::vector<ChatInputTag> visibleFileTags; // 当前可见的文件 Tag 列表（来自 ChatInput）
		AbsTick t;                            // 发送的时间戳
		int fileAttaches;                     // CChatOpsCtrl 中对应 Op_FileAttaches 的索引，-1 表示无

		void Clear()
		{
			apiName.clear();
			visibleFileTags.clear();
			Zero();
		}
	};


public:
	// ─────────────────────────────────────────────────────────────────────
	//  生命周期
	// ─────────────────────────────────────────────────────────────────────

	CChatAgent()
	{
		Zero();
	}

	void Zero()
	{
		_ui = nullptr;
		_notify = nullptr;
		_requestInterrupt = false;
		_requestSendToolCallResult = false;
		_requestSave = false;
		_workingMode = WorkingMode::None;
		_chatFileVer = 0;

		_lastCtx.Zero();
		_chatUsage.Zero();
		_pendingRequest.valid = false;
	}

    // 初始化
    void Init(const char *chatFilePath, ChatAgentContext &ctx, IChatUi* ui,IChatNotify *notify);

    // 反初始化（中断所有任务、清理状态）
    void Shutdown();

	void AttachUI(IChatUi* ui);
	void DetachUI();

    void Update();

	const char* GetFileName() const	{		return _fileName.c_str();	}
	DWORD GetFileVer() const	{		return _chatFileVer;	}

	// 获取完整文件路径（用于检查 .fav 文件）
	std::string GetFilePath() const	{		return _MakeFilePath();	}

    // ─────────────────────────────────────────────────────────────────────
    //  对话控制
    // ─────────────────────────────────────────────────────────────────────

    // 发起一轮新的用户消息
    bool StartSession(const std::wstring& content,const std::string &apiName, const std::vector<ChatInputTag>& tags, const char* skillsDump);

	// 请求中断当前对话（异步，在下一次 Update() 里处理）
	void RequestInterrupt()	
	{		
		_requestSendToolCallResult = false;
		_interject.clear();
		_requestInterrupt = true;
		// 清空挂起的请求
		_pendingRequest.valid = false;
	}

    // 通知 CChatAgent 在下一次 Update() 里发送 ToolCallResult
    // 由宿主在 ToolCall 处理完毕后调用
    void RequestSendToolCallResult();

    // ─────────────────────────────────────────────────────────────────────
    //  暂停/恢复控制
    // ─────────────────────────────────────────────────────────────────────

    // 暂停 agent，会在下一次 tool call 返回时暂停
    // 暂停期间不会发送 tool call result，直到调用 Resume()

    // ─────────────────────────────────────────────────────────────────────
    //  持久化
    // ─────────────────────────────────────────────────────────────────────



    // 清空对话数据（委托 CChatOpsCtrl::Clear）
    // 清空后需由宿主调用 CChatCtrl::ClearChat() 同步清空 UI 显示
    void Clear();

    // ─────────────────────────────────────────────────────────────────────
    //  状态查询
    // ─────────────────────────────────────────────────────────────────────

	bool IsWorking() const	{		return _workingMode == WorkingMode::Chat || _pendingRequest.valid;	}
    bool IsInChat()  const; // LlmChat 正在进行流式接收
	bool IsRequestSave() const	{		return _requestSave;	}

    // 当前 AI 消息 ID（流式接收期间有效，供宿主查询以追加 FileSummarize 等）
    const std::wstring& GetCurrentAIMessageId() const;

    // 上一次/当前会话上下文
    const LlmSessionContext& GetLastSessionContext() const;

    // 本轮对话累积的 Token 用量
    const LlmSessionUsage& GetChatUsage() const;

	void RemoveDisabledSessions();

    // ── FileEdit Progress Label ─────────────────────────────────────────────

    // 显示/隐藏文件编辑进度标签
    void ShowFileEditProgressLabel(const std::wstring& fileName, const std::wstring& fullPath = L"");
    void HideFileEditProgressLabel();

	void RequestSave()	{		_requestSave = true;	}

	void GetLastNotDisabledSessionTags(std::vector<ChatInputTag>& tags) const	{		return _opsCtrl.GetLastNotDisabledSessionTags(tags);	}

	bool GetRestoreCheckpoints(const std::wstring& messageId, std::vector<FilesCheckpointUID>& checkpointIds)	{		return _opsCtrl.GetRestoreCheckpoints(messageId, checkpointIds);}

	// 恢复用户消息
	// messageId: 要恢复到的用户消息 ID
	// confirmCallback: 确认回调，当检测到文件被修改时调用
	// 返回值: true 表示成功，false 表示取消或失败
	typedef std::function<bool(const std::vector<std::string>& modifiedFiles)> RestoreUserMessageConfirmCallback;
	bool RestoreUserMessage(const std::wstring& messageId, RestoreUserMessageConfirmCallback confirmCallback);

	// 获取用户消息内容
	bool GetUserMessageContent(const std::wstring& messageId, std::wstring& content) const
	{
		return _opsCtrl.GetUserMessageContent(messageId, content);
	}

	// 获取用户消息所在 session 的 tags
	void GetUserMessageSessionTags(const std::wstring& messageId, std::vector<ChatInputTag>& tags) const
	{
		_opsCtrl.GetUserMessageSessionTags(messageId, tags);
	}

	// 恢复被 disabled 的消息（Undo Restore）
	// confirmCallback: 确认回调，当检测到文件被修改时调用
	// 返回值: true 表示成功，false 表示取消或失败
	typedef std::function<bool(const std::vector<std::string>& modifiedFiles)> UndoRestoreConfirmCallback;
	bool RestoreDisabledMessage(UndoRestoreConfirmCallback confirmCallback);

	FilesCheckpointUID GetUndoCheckpoint(FilesCheckpointUID& restoredCheckpoint) const { return _opsCtrl.GetUndoCheckpoint(restoredCheckpoint); }

	// 获取文件编辑的差异信息
	// fileEditId: 文件编辑ID
	// filePath: 输出文件路径
	// oldCheckpointId: 输出旧检查点ID
	// newCheckpointId: 输出新检查点ID
	// 返回值: true 表示成功，false 表示失败
	bool GetFileEditDiff(const std::wstring& fileEditId, std::string& filePath, FilesCheckpointUID& oldCheckpointId, FilesCheckpointUID& newCheckpointId);

	// 获取文件摘要的差异信息
	// messageId: AI消息ID
	// filePath: 文件路径（输入）
	// oldCheckpointId: 输出旧检查点ID
	// newCheckpointId: 输出新检查点ID
	// 返回值: true 表示成功，false 表示失败
	bool GetFileSummarizeDiff(const std::wstring& messageId, const std::wstring& filePath, FilesCheckpointUID& oldCheckpointId, FilesCheckpointUID& newCheckpointId);

	int  GetRecentPromptToken() const { return _opsCtrl.GetRecentPromptToken(); }

	CChatOpsCtrl& GetOpsCtrl() { return _opsCtrl; }

	bool HasTitle() const { return _opsCtrl.HasTitle(); }
	void SetTitle(const std::wstring& title) { _opsCtrl.SetTitle(title); }
	const wchar_t* GetTitle() const { return _opsCtrl.GetTitle(); }
	const std::vector<ChatOp>& GetOps() const { return _opsCtrl.GetOps(); }

	CChatOpsCompress& GetCompressor()	{		return _compressor;	}
	CChatOpsSummarizer& GetSummarizer()	{		return _summarizer;	}

private:
    // ─────────────────────────────────────────────────────────────────────
    //  内部流程
    // ─────────────────────────────────────────────────────────────────────

    // 向 LlmChat 发起一次请求（通过 OnPrepareLlmSetting 获取 setting）
    // isUserMessage: true = 用户发送消息, false = 发送 ToolCall 结果
    // 失败时回调 OnSendFailed，返回 false
    bool _DoRequest(const LlmSessionRequest& request, bool isUserMessage);

    void _ExecuteSendUserMessage(const std::wstring& content, const std::vector<ChatInputTag>& allTags);

    void _ExecuteSendToolCallResult();

    // 删除所有 disabled session，并回调 OnDiscardCheckpoints
    void _RemoveDisabledSessions();

    // 本轮对话结束时的收尾：
    //   CChatOpsCtrl::CompleteStreamingAIMessage → CChatOpsCtrl::AddSessionCost
    //   → CChatOpsCtrl::EndSession → 回调 OnChatFinished / OnDataDirty
    void _FinishChat();

    // 为文件附件创建 checkpoint
    FilesCheckpointUID _OnCreateCheckpointForFileAttaches(const LlmSessionContext& ctx);

    // 尝试继承旧的 fileAttaches（利用 LLM token cache 机制）
    void _TryInheritOldFileAttaches(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx);

    // 检查并保存对话数据
    void _UpdateSaveChatCtrl();

    // 根据 _fileName 生成完整文件路径
    std::string _MakeFilePath() const;

	ChatAgentContext _ctx;

	std::string _fileName;

	IChatUi* _ui;
	IChatNotify* _notify;

    CChatOpsCtrl       _opsCtrl;       // 对话数据层（无 UI，可独立序列化）
    CLlmChat        _llmChat;        // 底层流式 LLM HTTP 请求
    CChatTaskMgr    _taskMgr;        // ToolCall 任务调度器
	CChatFileWriter _chatFileWriter;
	CChatOpsCompress _compressor;  // 压缩器
	CChatOpsSummarizer _summarizer; // 后台总结器

    LlmSessionContext _lastCtx;     // 上一次/当前会话上下文
    LlmSessionUsage   _chatUsage;   // 本轮对话累积的 Token 用量
	std::shared_ptr<std::string> _skillsDump;  // working 周期开始时 dump 一次，整个周期复用

    std::wstring    _aiMessageId;   // 当前流式 AI 消息 ID

	bool _requestInterrupt;          // 是否请求中断
	bool _requestSendToolCallResult; // 是否请求发送 ToolCallResult
	bool _requestSave;
	std::string _interject;

	DWORD _chatFileVer;              // 对话文件版本号，用于判断是否需要保存

    enum class WorkingMode
    {
        None,
        Chat,   // LlmChat 正在进行流式对话
    };
    WorkingMode     _workingMode;

	// ── Context Compression 等待机制 ───────────────────────────────────────
	// 在发送请求前检查是否需要压缩 context，如果需要则等待压缩完成后再发送
	struct PendingRequest
	{
		int fileAttaches = -1;
		bool isUserMessage = false;
		bool valid = false;
	};
	PendingRequest _pendingRequest;  // 挂起的请求（等待压缩完成）

};
