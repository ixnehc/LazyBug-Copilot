#pragma once

#include <vector>
#include <map>
#include <unordered_set>
#include <string>
#include "LlmLibDefines.h"  // for LlmToolType
#include "timer/timer.h"

#include "chatopsctrl.h"

#include "chattaskmgr.h"

// 前置声明
class CChatOpsCtrl;
struct ChatOp;
class CChatTask_CompressSummarize;
class CChatAgent;

enum class ChatOpCompressIntensity
{
	None,
	Low,
	Medium,
	High,
	Extreme,
};

class CChatOpsCompress
{
	friend class CChatTask_CompressSummarize;
public:
	// 压缩等级
	enum CompressLevel
	{
		Level_None = 0,     // 无压缩
		Level_Partial = 1,  // 部分压缩（截断/摘要）
		Level_Full = 2,      // 完全清除
		Level_Remove =3 //彻底删除这个op
	};

	// 压缩状态
	enum State
	{
		State_Idle,         // 空闲
		State_Compressing,  // 正在压缩
	};

	struct Env
	{
		Env()
		{
			isValid = false;
		}
		void Clear()
		{
			isValid = false;
		}
		bool Equals(const Env& other)
		{
			if (!isValid)
				return false;
			return isValid == other.isValid &&
				intensity == other.intensity &&
				std::fabs(tokenCalibrate - other.tokenCalibrate) < 1e-6f &&
				opsVer == other.opsVer &&
				disableAfter == other.disableAfter;
		}
		bool isValid;
		ChatOpCompressIntensity intensity;
		float tokenCalibrate;
		DWORD opsVer;
		int disableAfter;
	};

	// 工作 Op 结构：与 CChatOpsCtrl 中的 Op 一一对应
	struct Op
	{
		ChatOp::Type type;                          // Op 类型
		LlmToolType toolType = LlmToolType::None;   // ToolCall 类型（仅 Op_AddToolCallResult 等有效）

		int srcIndex = -1;              // 对应 CChatOpsCtrl::_ops 的原始索引
		int initialTokens = 0;          // 初始 token 数
		int sessionAge = 0;             // Session 年龄：0=当前session, 1=上一轮...

		CompressLevel currentLevel = Level_None;    // 当前压缩等级
		std::map<int, std::string> newCompressedContents; // 本次压缩过程中新增的压缩内容,utf8
	};

public:
	CChatOpsCompress();
	~CChatOpsCompress();

	void Zero()
	{
		_opsCtrl = nullptr;
		_state = State_Idle;

		_reduceTokenCount = 0;        // 目标减少的 token 数
		_reducedTokens = 0;           // 已减少的 token 数（累加）
		_currentPass = 0;             // 当前执行的 Pass

		_compressStartTime = 0;     // 压缩开始时间

		_summarized.clear();        // 清空已摘要列表
	}

	void Init(CChatOpsCtrl* opsCtrl, CChatAgent* chatAgent);
	void Clear();

	// 从注册表读写当前 API 对应的压缩强度
	static ChatOpCompressIntensity LoadIntensityForCurrentApi();
	static void SaveIntensityForCurrentApi(ChatOpCompressIntensity intensity);

	// 获取用于 Session 摘要的 ToolCall 类型列表（供 CollectSessionContent 和 EstimateSessionAIContentToken 共用）
	static const std::vector<LlmToolType>& GetSessionSummarizeToolTypes();

	void UpdateCompressTriggering();
	void UpdateCompress();

	// 获取当前状态
	State GetState() const { return _state; }
	bool IsCompressing() const { return _state == State_Compressing; }
	bool IsSummarizing();

	void CancelCompress()	{		_CancelCompress();	}
	bool TryTrigger(const std::string& summarizeApiName, bool forceRecompress, bool allowDecompress)	{		return _TryTrigger(summarizeApiName, forceRecompress, allowDecompress);	}


	// 获取已减少的 token 数
	int GetReducedTokens() const { return _reducedTokens; }

	// 压缩结果提示相关
	// 获取压缩结果消息及版本号
	// 返回值: 是否有新消息（版本号变化）
	bool GetCompressSummarizeTip(std::wstring& message, bool& success, std::wstring& logPath, int& version);
	int GetCompressSummarizeTipVersion() const { return _tipVersion; }

private:
	// 设置压缩结果消息（由 CChatTask_CompressSummarize 调用）
	void _SetCompressSummarizeTip(bool success, const std::string& message, const std::string& logPath);

private:

	void _StartCompress(int reduceTokenCount, const std::string& summarizeApiName, bool allowDecompress);
	void _BuildWorkingOps();

	void _CollectEnv(Env &env);

	void _UpdateCompress();
	void _CancelCompress();
	void _DecompressAll();
	bool _TryTrigger(const std::string& summarizeApiName, bool forceRecompress, bool allowDecompress);

	// 执行指定 Pass，返回是否达到目标
	// 执行指定 Pass
	void _ExecutePass(int pass);

	// 将压缩结果同步回 ChatOp 数组
	void _SyncBackToOps();

	// 获取 Op 对应的原始 ChatOp
	const ChatOp& _GetSrcOp(const Op& op) const;

	// 获取预存的简化版内容（优先查本次新增，再查原始 ChatOp 历史内容）
	const std::string* _GetCompressedContent(const Op& op, CompressLevel level) const;

	// 获取 Op 当前实际占用的 token 数
	int _GetOpCurrentTokens(const Op& op) const;

	// 估算单个 Op 的 token 数
	int _EstimateOpTokens(const ChatOp& op, bool useUncompressed = false) const;

	// 估算指定 op 所在 session 中所有 AI 内容的 token 总数
	int _EstimateSessionAIContentTokens(Op& op);

	// 应用压缩到单个工作 Op，返回减少的 token 数
	int _ApplyCompressToOp(Op& op, CompressLevel level, const std::string& content);
	int _ApplySummarizeSession(Op& op, CompressLevel level, const std::string& content);

	// Pass 实现函数（参数：startSessionAge, endSessionAge，表示处理的 session 年龄范围）
	void _Pass_RemoveFailureFileEdit(int startSessionAge, int endSessionAge);
	void _Pass_RemoveFailureCMD(int startSessionAge, int endSessionAge);
	void _Pass_RemoveCoveredLines(int startSessionAge, int endSessionAge);
	void _Pass_RemoveIrrelavantSearchResult(int startSessionAge, int endSessionAge);
	void _Pass_RemoveThinking(int startSessionAge, int endSessionAge);
	void _Pass_TruncateCmdResults(int startSessionAge, int endSessionAge);
	void _Pass_TruncateToolCallResult(int startSessionAge, int endSessionAge, const std::vector<LlmToolType>& toolTypes, CompressLevel level = Level_Partial);
	void _Pass_TruncateFindSymbol(int startSessionAge, int endSessionAge);
	void _Pass_TruncateReadFile(int startSessionAge, int endSessionAge);
	void _Pass_TruncateFindInFiles(int startSessionAge, int endSessionAge);
	void _Pass_TruncateReplaceInFile(int startSessionAge, int endSessionAge);
	void _Pass_ClearReplaceInFile(int startSessionAge, int endSessionAge);
	void _Pass_RemoveSearchOps(int startSessionAge, int endSessionAge);
	void _Pass_RemoveFindSymbol(int startSessionAge, int endSessionAge);
	void _Pass_RemoveReplaceInFile(int startSessionAge, int endSessionAge);
	void _Pass_RemoveToolCallResult(int startSessionAge, int endSessionAge, const std::vector<LlmToolType>& toolTypes);
	void _Pass_ClearMessages(int startSessionAge, int endSessionAge);
	void _Pass_SummarizeSession(int startSessionAge, int endSessionAge);

	// Pass 总数
	static constexpr int _passCount = 50;

	// 截断工具
	std::wstring _TruncateSearchResult(const std::wstring& content, int maxLines);
	std::string _TruncateCmdResult(const std::string& content, int maxLines);

private:
	CChatOpsCtrl* _opsCtrl = nullptr;
	Env _env;

	static constexpr int _compressTimeLimitMs = 20;  // 每次压缩时间限制（毫秒）
	AbsTick _compressStartTime = 0;     // 压缩开始时间

	//working state
	State _state = State_Idle;
	bool _allowDecompress = false;
	std::string _summarizeApiName;  // 用于 summarize 的 API 名称（已解析 <auto>）
	Env _workingEnv;
	std::vector<Op> _workingOps;      // 工作 Op 数组
	int _initialTokens = 0;     //压缩开始时的压缩后的token数 (Not calibrated)
	int _initialUncompressedTokens = 0;//压缩开始时的未压缩的token数(Not calibrated)
	int _reduceTokenCount = 0;        // 目标减少的 token 数(Not calibrated)
	int _reducedTokens = 0;           // 已减少的 token 数（累加）(Not calibrated)
	int _currentPass = 0;             // 当前执行的 Pass
	std::unordered_set<int> _summarized; // 本次压缩过程中已尝试 AddTask_CompressSummarize 的 workingOp 索引

	// 压缩结果提示相关
	std::string _tipMessage;      // 提示消息（utf8）
	std::string _tipLogPath;      // 日志文件路径（utf8）
	bool _tipSuccess = false;     // 是否成功
	int _tipVersion = 0;          // 版本号，每次更新 +1

	// 判断是否超时
	bool _IsCompressTimeout() const;

	CChatTaskMgr _taskMgr;

};

