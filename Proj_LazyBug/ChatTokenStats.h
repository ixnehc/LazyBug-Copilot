#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "TokenCalibrate.h"

// 前向声明
class CChatOpsCtrl;
class CChatInput;
class CLlmSkills;
class CChatDialogA;
struct ChatInputTag;
struct LlmApi;

namespace TokenStats
{
	// Token 统计的各个部分类型
	enum class SectionType
	{
		History,        // 历史消息
		Input,          // 当前输入
		Attachments,    // 附件文件
		SystemPrompt,   // System Prompt (Rules)
		Tools,          // Tools 定义
		Skills,         // Skills 内容
		Count           // 用于数组大小
	};

	// 各部分的统计结果
	struct SectionResult
	{
		int tokenCount = 0;           // Token 数量
		bool isDirty = false;          // 是否刚刚更新过
	};

	// 完整统计结果
	struct TotalResult
	{
		int totalTokens = 0;
		std::map<SectionType, SectionResult> sections;
		
		int GetSectionTokens(SectionType type) const;
	};
}

// ============================================================================
// Context 类
// 包含所有 Provider 需要访问的对象引用
// ============================================================================
struct ChatTokenStatsContext
{
	CChatOpsCtrl* opsCtrl = nullptr;
	CChatInput* chatInput = nullptr;
	CLlmSkills* llmSkills = nullptr;
	CChatDialogA* chatDialog = nullptr;
};

// ============================================================================
// Token 提供者接口
// ============================================================================
class ITokenProvider
{
public:
	virtual ~ITokenProvider() = default;

	// 获取当前数据版本（用于检测变化）
	// 返回值的任何变化都表示数据已变更，需要重新计算
	virtual uint64_t GetVersion() const = 0;

	// 计算 Token 数（仅在版本变化时调用）
	virtual int CalculateTokens() = 0;
};

// ============================================================================
// 历史消息 Token 提供者
// 关联 CChatOpsCtrl，获取历史对话消息的 Token 数
// ============================================================================
class HistoryTokenProvider : public ITokenProvider
{
public:
	explicit HistoryTokenProvider(const ChatTokenStatsContext& ctx);

	uint64_t GetVersion() const override;
	int CalculateTokens() override;

	// 获取未压缩 Token 数
	int GetUncompressedTokens() const;

private:
	const ChatTokenStatsContext& _ctx;
};

// ============================================================================
// 当前输入 Token 提供者
// 关联 CChatInput，获取当前输入文本和 Inline Tags 的 Token 数
// ============================================================================
class InputTokenProvider : public ITokenProvider
{
public:
	explicit InputTokenProvider(const ChatTokenStatsContext& ctx);

	uint64_t GetVersion() const override;
	int CalculateTokens() override;

private:
	const ChatTokenStatsContext& _ctx;
};

// ============================================================================
// 附件文件 Token 提供者
// 根据 ChatInputTag 列表计算附件文件的 Token 数
// ============================================================================
class AttachmentTokenProvider : public ITokenProvider
{
public:
	explicit AttachmentTokenProvider(const ChatTokenStatsContext& ctx);

	uint64_t GetVersion() const override;
	int CalculateTokens() override;

private:
	const ChatTokenStatsContext& _ctx;
	std::vector<std::string> _lastFileStates;  // 用于检测文件变化
};

// ============================================================================
// System Prompt Token 提供者
// 根据 Rules 文件路径计算 System Prompt 的 Token 数
// ============================================================================
class SystemPromptProvider : public ITokenProvider
{
public:
	explicit SystemPromptProvider(const ChatTokenStatsContext& ctx);

	uint64_t GetVersion() const override;
	int CalculateTokens() override;

private:
	const ChatTokenStatsContext& _ctx;
	std::string _lastRulesPath;
};

// ============================================================================
// Tools Token 提供者
// 根据当前选择的 LLM API 计算 Tools 定义的 Token 数
// ============================================================================
class ToolsTokenProvider : public ITokenProvider
{
public:
	explicit ToolsTokenProvider(const ChatTokenStatsContext& ctx);

	uint64_t GetVersion() const override;
	int CalculateTokens() override;

private:
	const ChatTokenStatsContext& _ctx;
	std::string _lastApiName;
	int _lastLlmLibVersion = 0;
};

// ============================================================================
// Skills Token 提供者
// 根据 CLlmSkills 中启用的 Skill 计算 Token 数
// ============================================================================
class SkillsTokenProvider : public ITokenProvider
{
public:
	explicit SkillsTokenProvider(const ChatTokenStatsContext& ctx);

	uint64_t GetVersion() const override;
	int CalculateTokens() override;

private:
	const ChatTokenStatsContext& _ctx;
	std::vector<std::pair<std::string, uint64_t>> _lastSkillStates;
};

// ============================================================================
// 对话上下文 Token 统计器
// 实时跟踪统计下一轮 LLM Session 的上下文 Token 数量
// ============================================================================
class CChatTokenStats
{
public:
	CChatTokenStats();
	~CChatTokenStats();

	// 禁止拷贝
	CChatTokenStats(const CChatTokenStats&) = delete;
	CChatTokenStats& operator=(const CChatTokenStats&) = delete;

	// ------------------------------------------------------------------------
	// 初始化：传入 Context（包含所有需要的外部对象引用）
	// ------------------------------------------------------------------------
	void Initialize(const ChatTokenStatsContext& ctx);

	// ------------------------------------------------------------------------
	// 更新检测（应在每帧或适当时机调用）
	// 检测各部分的版本变化，重新计算变化的部分的 Token 数
	// ------------------------------------------------------------------------
	void Update();

	// ------------------------------------------------------------------------
	// 查询统计结果
	// ------------------------------------------------------------------------
	
	// 获取总 Token 数（快速，无计算开销）
	int GetTotalTokens() const;

	// 获取总未压缩 Token 数（仅 History 部分使用未压缩值，其他使用已缓存值）
	int GetTotalUncompressedTokens() const;

	// 获取指定部分的 Token 数
	int GetSectionTokens(TokenStats::SectionType type) const;

	// 获取完整的统计结果（包含分项信息）
	TokenStats::TotalResult GetTotalResult() const;

	// ------------------------------------------------------------------------
	// 变化检测
	// ------------------------------------------------------------------------
	
	// 指定部分是否在上次 Update 后发生变化
	bool HasChanged(TokenStats::SectionType type) const;

	// 是否有任何部分发生变化
	bool HasAnyChanged() const;
	void ClearAllChanged();

	// ------------------------------------------------------------------------
	// 手动标记脏（强制重新计算）
	// ------------------------------------------------------------------------
	void MarkSectionDirty(TokenStats::SectionType type);
	void MarkAllDirty();

	// ------------------------------------------------------------------------
	// Token 矫正机制
	// 用于根据实际 token 数校正估算值
	// ------------------------------------------------------------------------
	
	// 开始矫正：记录当前的估计 token 数
	void BeginCalibration();
	
	// 应用矫正：传入真实的 token 数，累加到注册表
	void ApplyCalibration(int actualTokens);
	
	// 获取矫正后的 token 数
	int GetCalibratedTokens() const;
	
	// 获取矫正后的未压缩 token 数
	int GetUncompressedCalibratedTokens() const;
	
	// 获取当前矫正因子
	float GetCalibrationFactor() const;

private:
	// 内部缓存的结构
	struct CachedSection
	{
		ITokenProvider* provider = nullptr;
		uint64_t lastVersion = 0;
		int cachedTokens = 0;
		bool hasChanged = false;  // 上次 Update 是否变化
		bool isDirty = false;     // 是否需要强制重新计算
	};

	// Context（外部传入的引用）
	ChatTokenStatsContext _ctx;

	// 各个 Provider（作为成员直接维护）
	HistoryTokenProvider _historyProvider;
	InputTokenProvider _inputProvider;
	AttachmentTokenProvider _attachmentProvider;
	SystemPromptProvider _systemPromptProvider;
	ToolsTokenProvider _toolsProvider;
	SkillsTokenProvider _skillsProvider;

	// 各部分的缓存
	std::map<TokenStats::SectionType, CachedSection> _sections;

	// 获取缓存项
	CachedSection* _GetSection(TokenStats::SectionType type);
	const CachedSection* _GetSection(TokenStats::SectionType type) const;

	// 初始化 sections 映射
	void _InitSections();

	// Token 矫正器
	CTokenCalibrate _tokenCalibrate;
};




