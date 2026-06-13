#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <fstream>
#include <map>

#include "Checkpoints.h"
#include "ChatInputTag.h"
#include "LlmSession.h"

#include "ChatAgentDefines.h"

class CDataPacket;


// ─────────────────────────────────────────────────────────────────────────────
//  版本号（与 CChatCtrl 原版本号共用同一套序列化格式，保持二进制兼容）
// ─────────────────────────────────────────────────────────────────────────────
#define CHATOPSCTRL_VERSION_1_0     0x00010000
#define CHATOPSCTRL_VERSION_1_1     0x00010001
#define CHATOPSCTRL_VERSION_1_2     0x00010002
#define CHATOPSCTRL_VERSION_1_3     0x00010003
#define CHATOPSCTRL_VERSION_1_4     0x00010004
#define CHATOPSCTRL_VERSION_1_5     0x00010005
#define CHATOPSCTRL_VERSION_1_6     0x00010006
#define CHATOPSCTRL_VERSION_CURRENT CHATOPSCTRL_VERSION_1_6

// ─────────────────────────────────────────────────────────────────────────────
//  ChatOp
//  对话历史的原子操作记录，原属 chatctrl.h，与 UI 无关，移至此处
// ─────────────────────────────────────────────────────────────────────────────
struct ChatOp
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
        Op_AddToolCallResult,
        Op_AddStreamingAIMessage_Thinking,
        Op_AddReplaceInFileResult_Obsolete,
        Op_AddToolCallMessage,
        Op_AddFileSummarizeToAIMessage,
		Op_AddUserInterject,
		Op_CliDisplay,
		Op_QuestionDisplay,
		Op_Stats_Obsolete,
        // 重要：后添加的 Op 一定要加在末尾
    };

    void Save(CDataPacket& dp);
    void Load(CDataPacket& dp, DWORD ver);
	void Load(std::ifstream& file, DWORD ver);

    Type type;

    std::wstring messageId;   // 见 chatctrl.h 原注释
    std::string contentUtf8;  // 内容统一使用 UTF-8 编码 (v1.6+ 从 wstring 迁移)
    std::wstring fileEditId;
    std::wstring title;
    std::wstring fullPath;
    std::wstring diffContent;

    FilesCheckpointUID checkpointId;

    // ── 压缩相关扩展 ──
    int currentCompressionLevel;                   // 当前生效的压缩等级 (0 = 无压缩)
    std::map<int, std::string> compressedContents; // level -> 压缩后内容 (UTF-8)

    ChatOp() : type(Op_AddUserMessage),
                   checkpointId(FilesCheckpointUID_Invalid),
                   currentCompressionLevel(0) {}

    explicit ChatOp(Type t) : type(t),
                                  checkpointId(FilesCheckpointUID_Invalid),
                                  currentCompressionLevel(0) {}
};

// ─────────────────────────────────────────────────────────────────────────────
//  CChatOpsCtrl
//
//  多轮对话数据层，完全与 UI 解耦。
//
//  职责：
//    - 存储并管理对话历史（Op 队列）
//    - 存储 FileEdit 元数据（_fileEdits）
//    - 提供 Session 生命周期管理（BeginSession / EndSession）
//    - 提供 FileAttaches 管理与缓存校验
//    - 提供 MakeSessionRequest：从 Op 队列重建 LlmSessionRequest
//    - 提供 Save / Load：序列化与反序列化
//
//  不包含任何 MFC / Win32 / WebView2 代码。
//  CChatCtrl（UI 窗口）持有 CChatOpsCtrl* 指针，通过 ReplayOps() 将数据渲染到 WebView2。
// ─────────────────────────────────────────────────────────────────────────────
class CChatOpsCtrl
{
    friend class CChatOpsCompress;  // 允许压缩器访问私有成员

public:
    CChatOpsCtrl();
    ~CChatOpsCtrl();

	void Zero();
	void Init(const ChatAgentContext &ctx);
	void Clear();

	struct FileEditBtn
	{
		std::wstring text;    // 按钮文字
		std::wstring action;  // 按钮动作标识
		std::wstring id;      // 按钮唯一 ID
	};

	struct FileEdit
	{
		std::wstring id;                        // 窗口唯一 ID
		std::wstring title;                     // 标题栏文字
		std::wstring fullPath;                  // 文件完整路径
		std::wstring content;                   // 显示区域内容
		std::wstring diffContent;               // Diff 内容
		std::wstring messageId;                 // 所属的 AI 消息 ID
		std::vector<FileEditBtn> buttons;
		bool isCollapsed; // 是否折叠
	};

	void AttachUI(IChatUi* ui);
	void DetachUI();

    // ── 消息追加 ──────────────────────────────────────────────────────────

    // 添加用户消息，返回消息 ID
    void AddUserMessage(const std::string& message,
                                const std::wstring& overrideMessageId = L"");

    // 获取用户消息内容
    bool GetUserMessageContent(const std::wstring& messageId,
                               std::wstring& content) const;

    // 开始新的 AI 流式消息，返回消息 ID
    std::wstring StartStreamingAIMessage(const std::wstring& overrideMessageId = L"");

    // 追加 AI 流式正文增量
    void AddStreamingAIMessage(const std::wstring& messageId,
                               const std::string& incrementalContentUtf8);

    // 追加 AI 流式 Thinking 增量
    void AddStreamingAIMessage_Thinking(const std::wstring& messageId,
                                        const std::string& incrementalContentUtf8);

    // 追加 ToolCall 消息（记录 Tool 调用信息）
    void AddToolCallMessage(const std::wstring& messageId,
                            const std::string& message);

    // 完成流式 AI 消息
    void CompleteStreamingAIMessage(const std::wstring& messageId);

    // 添加系统消息
    void AddSystemMessage(const std::string& message,
                          const std::wstring& overrideMessageId = L"");

    // 清空聊天记录
    void ClearChat();

    // ── FileAttaches ──────────────────────────────────────────────────────

    // 添加一条 FileAttaches 记录，返回其在 _ops 中的索引
    int AddFileAttaches(const std::string& fileList,
                        FilesCheckpointUID checkpointId);

	//fileAttaches为一个Op_FileAttaches的索引.这个函数检查它是否可以作为有效的cache
	// 首先检查fileAttaches是否是最后一个未被disable的Op_FileAttaches
	// 再检查这个fileAttaches里的文件列表是否可以cover visibleTags里的所有文件
	// 还会检查fileAttaches有没有一个有效的checkpoint,
	//TODO:要考虑这些文件有没有被外部改动过,如果某个文件被外部改动过,则这些Op全部无效,返回-1
	//所谓外部改动过是指没有被这个chat 里的checkpoint记录下的改动
	bool CheckValidFileAttachesCache(int fileAttaches,
                                     const std::vector<ChatInputTag>& visibleTags);

    // ── Session 生命周期 ──────────────────────────────────────────────────

    void BeginSession(FilesCheckpointUID checkpointId);
    void EndSession();

	bool GetRestoreCheckpoints(const std::wstring& userMessageId, std::vector<FilesCheckpointUID>& checkpointIds);

    // ── Session Tag ───────────────────────────────────────────────────────

    void AddSessionTag(const ChatInputTag& tag);
    void GetLastNotDisabledSessionTags(std::vector<ChatInputTag>& tags) const;
    void GetUserMessageSessionTags(const std::wstring& userMessageId,
                                   std::vector<ChatInputTag>& tags) const;

    // ── ToolCall Result ───────────────────────────────────────────────────

    void AddToolCallResult(const std::string& jsonString, const std::string& jsonStringPartial = "", const std::string& jsonStringFullCompress = "");
	void AddInterjectToLastToolCallResult(const std::string& interject);

    // ── User Interject ───────────────────────────────────────────────────

    // 在最后一个 AI 消息中添加用户插话
    void AddUserInterject(const std::wstring& messageId, const std::string& interject);

    // ── CLI Display ─────────────────────────────────────────────────────

    // 添加 CLI 命令显示，返回 CLI ID
    // displayStatus: Pending=等待用户确认, Accepted=白名单自动执行, None=其他
    std::wstring AddCliDisplay(const std::wstring& messageId, const std::string& command, const std::wstring& desc = L"", CliDisplayStatus displayStatus = CliDisplayStatus::None, const std::string& shellType = "");
    
    // 增量追加输出到最后的 CLI 显示
    void AppendOutputToLastCliDisplay(const std::wstring& messageId, const std::string& deltaOutput);
    
    // 显示/隐藏 CLI 输入区域
    void ShowCliInputArea(const std::wstring& cliId, bool bShow);
    
    // 完成 CLI 显示（更新状态）
    void CompleteCliDisplay(const std::wstring& cliId, int exitCode);

    // ── Question Display ─────────────────────────────────────────────────────

    // 添加问题和答案显示
    void AddQuestionDisplay(const std::wstring& messageId, const std::wstring& question, const std::string& answer);

    // ── Undo Checkpoints ─────────────────────────────────────────────────

    void SetUndoCheckpoint(FilesCheckpointUID checkpointId);
	FilesCheckpointUID GetUndoCheckpoint(FilesCheckpointUID& restoredCheckpoint) const { restoredCheckpoint = _restoredCheckpointId;		return _undoCheckpointId; }

    // ── Disable / Enable ─────────────────────────────────────────────────

    void DisableMessagesAfter(const std::wstring& messageId);
    void RemoveDisabledSessions(std::vector<FilesCheckpointUID>& checkpointsToDiscard);
    void EnableAllDisabledMessages();
	int GetDisableAfterIndex() const	{		return _GetDisableAfterIndex();	}

	// 查找最近 N 个未 disable 的 session 的 EndSession 索引
	std::vector<int> FindLastNNotDisabledSessionEnds(int count) const;

    // ── Session Cost ─────────────────────────────────────────────────────

    // 累积某个 FileEdit 所在 session 的费用
    void AccumulateSessionCostForFileEdit(const std::wstring& fileEditId,
                                          float price, int inputToken, int outputToken);
    // 追加本次 session 的总费用记录
    void AddSessionCost(const LlmSessionUsage& usage,
                        const std::wstring& messageId = L"");
    // 记录 prompt cache token 数（更新 _recentPrompToken）
    void NotifyPromptCache(const LlmSessionUsage& usage);
    int  GetRecentPromptToken() const { return _recentPrompToken; }

    // ── MakeSessionRequest ───────────────────────────────────────────────

    // 从 Op 队列重建 LlmSessionRequest（供 CChatAgent 发送给 LLM）
	bool MakeSessionRequest(LlmSessionRequest& request, int fileAttaches);
	bool MakeSessionRequest_Debug(LlmSessionRequest& request);
	void CollectUncompressedSessionAIContent(int targetSrcIndex, const std::vector<LlmToolType>& toolTypes, std::string& content);
	int  EstimateUncompressedSessionAIContentToken(int targetSrcIndex, const std::vector<LlmToolType>& toolTypes);

    // ── Title ────────────────────────────────────────────────────────────

    void SetTitle(const std::wstring& title);
    bool HasTitle() const;

    // ── FileEdit 元数据 ───────────────────────────────────────────────────

    std::wstring AddFileEditToAIMessage(const std::wstring& messageId,
                                        const std::wstring& title,
                                        const std::wstring& fullPath,
                                        const std::wstring& overrideFileEditId);

    void SetFileEditTitle(const std::wstring& fileEditId,
                          const std::wstring& title);

    void SetFileEditContent(const std::wstring& fileEditId,
                            const std::string& content,
                            const std::wstring& diffContent,
                            FilesCheckpointUID checkpointId);

    bool UpdateFileEditDiffContent(const std::wstring& fileEditId,
                                   const std::wstring& diffContent,
                                   FilesCheckpointUID checkpointId);

    bool GetFileEditCheckpoint(const std::wstring& fileEditId,
                               FilesCheckpointUID& checkpointId) const;

    bool GetFileEditPrevCheckpointInSession(const std::wstring& fileEditId,
                                            FilesCheckpointUID& checkpointId,
                                            bool& isHead) const;

    bool GetFileEditCheckpointInSessionBegin(const std::wstring& fileEditId,
                                             FilesCheckpointUID& checkpointId) const;

    bool GetFileEditFullPath(const std::wstring& fileEditId,
                             std::wstring& fullPath) const;

    bool GetFileEditContent(const std::wstring& fileEditId,
                            std::string& content) const;

    void GetNotDisabledFileEditsStartingFrom(const std::wstring& fileEditId,
                                             std::vector<std::wstring>& fileEditIds) const;

    bool IsFileEditInLastNotDisabledSession(const std::wstring& fileEditId) const;

    void GetFileEditFilePathesByMessageId(const std::wstring& messageId,
                                          std::vector<std::wstring>& filePathes) const;

    std::wstring GetLastFileEditCheckpointFromFilePath(const std::wstring& messageId,
                                                       const std::wstring& fullPath) const;

    bool SetFileEditHeadCheckpoint(const std::wstring& fileEditId,
                                   FilesCheckpointUID checkpointId);

    std::wstring AddFileEditButton(const std::wstring& fileEditId,
                                   const std::wstring& buttonText,
                                   const std::wstring& buttonAction,
                                   const std::wstring& overrideButtonId = L"");

    // FileEdit 折叠状态（仅修改数据；UI 折叠由 CChatCtrl 另行处理）
    void ToggleFileEditCollapse(const std::wstring& fileEditId);

    // FileEdit 修改状态动画
    void StartFileEditModification(const std::wstring& fileEditId);
    void StopFileEditModification(const std::wstring& fileEditId);

    // 添加 FileSummarize 按钮
    void AddFileSummarizeToAIMessage(const std::wstring& messageId, const std::wstring& filePath);

    bool ExistSummarizeInSession(const std::wstring& fileEditId) const;

    // ── FileEdit Progress Label ─────────────────────────────────────────────

    // 显示/隐藏文件编辑进度标签
    void ShowFileEditProgressLabel(const std::wstring& messageId, const std::wstring& fileName, const std::wstring& fullPath = L"");
    void HideFileEditProgressLabel(const std::wstring& messageId);

    // ── Loading Overlay ─────────────────────────────────────────────────────

    void ShowLoadingOverlay();
    void HideLoadingOverlay();

    // ── Op 队列访问 ───────────────────────────────────────────────────────

    const std::vector<ChatOp>& GetOps()      const { return _ops; }
    size_t GetOpsCount() const { return _ops.size(); }
    int FindLastOpIndex(ChatOp::Type tp)     const { return _FindLastOpIndex(tp); }

    // ── Session 边界查找 ──────────────────────────────────────────────────
    // 查找指定 op 所在的 session 边界
    // targetSrcIndex: 目标 op 在 _ops 中的索引
    // sessionBeginIndex: 输出参数，Op_BeginSession 的索引（如果没有找到则为 -1）
    // sessionEndIndex: 输出参数，Op_EndSession 的索引（如果没有找到则为 -1）
    // 返回值: 是否成功找到 session 边界（两个边界都找到返回 true）
    bool FindSessionBoundaries(int targetSrcIndex, int& sessionBeginIndex, int& sessionEndIndex) const;

    // ── 版本号（用于判断数据是否已被修改，需要保存）────────────────────

    DWORD GetVer() const { return _ver; }

	int GetEstimateTokens();
	int GetUncompressedEstimateTokens();

    // ── 序列化 ────────────────────────────────────────────────────────────

	bool Save(const char *filePath);
	bool Load(const char* filePath);

    // ── FileEdit 直接访问（供 CChatCtrl 在 ReplayOps 时使用）─────────────
    const std::vector<FileEdit>& GetFileEdits() const { return _fileEdits; }


private:
    // ── ID 生成 ───────────────────────────────────────────────────────────
    std::wstring _GenMsgId();
    std::wstring _GenFileEditId();
    std::wstring _GenFileEditBtnId();
    std::wstring _GenCliId();  // 生成 CLI ID

    // ── FileEdit 查找 ─────────────────────────────────────────────────────
    FileEdit* _FindFileEdit(const std::wstring& fileEditId);

    // ── Op 队列辅助 ───────────────────────────────────────────────────────
    void            _AddOp(const ChatOp& op);
    ChatOp*     _GetLastOp();
    const ChatOp* _FindLastOp(ChatOp::Type tp) const;
    int             _FindLastOpIndex(ChatOp::Type tp) const;
    int             _FindFirstOpIndexInSession(int sessionBeginIdx,
                                               ChatOp::Type tp) const;

    // ── Session 查找辅助 ──────────────────────────────────────────────────
    int _GetSessionBeginOfOpIndex(int idx) const;
    int _GetSessionBeginOfFileEdit(const std::wstring& fileEditId) const;
    int _FindFileEditOpIndex(const std::wstring& fileEditId) const;
    int _GetSessionBeginOfUserMessage(const std::wstring& messageId) const;
    int _GetDisableAfterIndex() const;
    int _GetLastNotDisabledSessionBegin() const;
    int _GetLastNotDisabledSessionEnd() const;

    // ── FileAttaches 辅助 ─────────────────────────────────────────────────
    void _GetFileAttachesList(int fileAttaches,
                              std::vector<std::string>& filePathes);
    void _GetFileAttachesList(int fileAttaches,
                              std::unordered_set<std::string>& filePathes);
    bool _GetLastFileTimeInCheckpoint(const std::string& fullPath,
                                      int startIdx, int endIdx, AbsTick& t);

    // ── Session Tag 辅助 ──────────────────────────────────────────────────
    void _AddSessionTag(const std::string& text,
                        const std::wstring& path, bool enabled);
    void _CollectSessionTags(int sessionBeginIndex, int sessionEndIndex,
                             std::vector<ChatInputTag>& tags) const;

    // ── 内部辅助 ──────────────────────────────────────────────────────────
    void _SetSessionCostDisplay(const std::string& costText, const std::wstring& messageId = L"");
    void _SendFileEditMsg(const std::wstring& action, const FileEdit& window);
    std::wstring _BuildButtonsJson(const std::vector<FileEditBtn>& buttons);

	int _EstimateTokenCountBetweenOps(int startIndex, int endIndex, bool useUncompressed = false);

	// 遍历 Session 内的 AI 内容（供 CollectUncompressedSessionAIContent 和 EstimateUncompressedSessionAIContentToken 共用逻辑）
	// callback 参数: (const std::string& contentFragment, LlmToolType toolType) -> bool，返回 false 可中断遍历
	void _IterateSessionAIContent(int targetSrcIndex, 
	                               const std::vector<LlmToolType>& toolTypes,
	                               std::function<bool(const std::string&, LlmToolType)> callback) const;

	void _ExecuteOp(const ChatOp& op);

	// ── CliDisplay 辅助 ───────────────────────────────────────────────────
	void _ParseCliDisplayContent(const std::string& content,
	                              std::string& cmd,
	                              std::string& output,
	                              std::string& shellType) const;
	std::string _BuildCliDisplayContent(const std::string& cmd,
	                                      const std::string& output,
	                                      const std::string& shellType = "") const;

    // ── 成员变量 ──────────────────────────────────────────────────────────
	IChatUi *_ui;

	ChatAgentContext _ctx;

    std::vector<ChatOp>      _ops;        // 全量 Op 队列（对话历史本体）
    std::vector<FileEdit> _fileEdits; // FileEdit 元数据（含 diff/checkpoint）

    DWORD _ver;                               // 数据版本号，每次写入 Op 后递增

    FilesCheckpointUID _undoCheckpointId;     // 最近一次 undo checkpoint
    FilesCheckpointUID _restoredCheckpointId; // 被 restore 的 checkpoint
		
    int _recentPrompToken;                    // 最近一个 session 的 prompt token 数

	// 估算所有有效历史消息（未被 disabled）的 Token 总数
	int _EstimateTokens() const;
	DWORD _verEstimateTokens;
	int _estimateTokensCache;

	// 估算未压缩的 Token 总数
	int _EstimateUncompressedTokens() const;
	DWORD _verUncompressedEstimateTokens;
	int _uncompressedEstimateTokensCache;

    std::wstring _currentStreamingMessageId;  // 当前正在流式写入的 AI 消息 ID
    
    int _cliCounter;  // CLI ID 计数器，确保唯一性

};
