#pragma once

#include <vector>
#include <map>
#include <string>
#include "LlmLibDefines.h"  // for LlmToolType
#include "timer/timer.h"

#include "chatopsctrl.h"

// 前置声明
class CChatOpsCtrl;
struct ChatOp;

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

	// 工作 Op 结构：与 CChatOpsCtrl 中的 Op 一一对应
	struct Op
	{
		ChatOp::Type type;                          // Op 类型
		LlmToolType toolType = LlmToolType::None;   // ToolCall 类型（仅 Op_AddToolCallResult 等有效）

		int initialTokens = 0;          // 初始 token 数
		int sessionAge = 0;             // Session 年龄：0=当前session, 1=上一轮...
		std::string originalContent;   // 原始内容副本,utf8

		CompressLevel currentLevel = Level_None;    // 当前压缩等级
		std::map<int, std::string> compressedContents; // 各 level 的压缩内容,utf8

		// 计算当前实际占用的 token 数
		int GetCurrentTokens() const;
	};

public:
	CChatOpsCompress();
	~CChatOpsCompress();

	void Init(CChatOpsCtrl* opsCtrl);
	void Clear();

	bool TryTrigger();

	void SetTokenCalibrate(float v)	{		_tokenCalibrate = v;	}

	// 压缩强度
	void SetIntensity(ChatOpCompressIntensity intensity);
	ChatOpCompressIntensity GetIntensity() const { return _intensity; }

	// 从注册表读写当前 API 对应的压缩强度
	static ChatOpCompressIntensity LoadIntensityForCurrentApi();
	static void SaveIntensityForCurrentApi(ChatOpCompressIntensity intensity);

	// 发起压缩请求，传入要减少的 token 数
	void StartCompress(int reduceTokenCount);

	// 每帧调用，驱动压缩任务
	void UpdateCompress();

	// 获取当前状态
	State GetState() const { return _state; }
	bool IsCompressing() const { return _state == State_Compressing; }

	// 取消压缩
	void Cancel();

	// 一键解压：将所有 Op 的压缩等级重置为 Level_None
	void DecompressAll();

	// 获取已减少的 token 数
	int GetReducedTokens() const { return _reducedTokens; }

private:
	// 构建工作 Op 数组
	void _BuildWorkingOps();

	// 执行指定 Pass，返回是否达到目标
	// 执行指定 Pass
	void _ExecutePass(int pass);

	// 将压缩结果同步回 ChatOp 数组
	void _SyncBackToOps();

	// 估算单个 Op 的 token 数
	int _EstimateOpTokens(const ChatOp& op) const;

	// 应用压缩到单个工作 Op，返回减少的 token 数
	int _ApplyCompressToOp(Op& op, CompressLevel level, const std::string& content);

	// Pass 实现函数（参数：startSessionAge, endSessionAge，表示处理的 session 年龄范围）
	void _Pass_RemoveFailureFileEdit(int startSessionAge, int endSessionAge);
	void _Pass_RemoveFailureCMD(int startSessionAge, int endSessionAge);
	void _Pass_RemoveCoveredLines(int startSessionAge, int endSessionAge);
	void _Pass_RemoveIrrelavantSearchResult(int startSessionAge, int endSessionAge);
	void _Pass_ClearThinking(int startSessionAge, int endSessionAge);
	void _Pass_TruncateCmdResults(int startSessionAge, int endSessionAge);
	void _Pass_TruncateToolCallResult(int startSessionAge, int endSessionAge, const std::vector<LlmToolType>& toolTypes);
	void _Pass_TruncateFindSymbol(int startSessionAge, int endSessionAge);
	void _Pass_TruncateReadFile(int startSessionAge, int endSessionAge);
	void _Pass_TruncateFindInFiles(int startSessionAge, int endSessionAge);
	void _Pass_RemoveSearchOps(int startSessionAge, int endSessionAge);
	void _Pass_RemoveFindSymbol(int startSessionAge, int endSessionAge);
	void _Pass_RemoveToolCallResult(int startSessionAge, int endSessionAge, const std::vector<LlmToolType>& toolTypes);
	void _Pass_ClearMessages(int startSessionAge, int endSessionAge);

	// Pass 总数
	static constexpr int _passCount = 50;

	// 截断工具
	std::wstring _TruncateSearchResult(const std::wstring& content, int maxLines);
	std::string _TruncateCmdResult(const std::string& content, int maxLines);

private:
	CChatOpsCtrl* _opsCtrl = nullptr;
	State _state = State_Idle;

	ChatOpCompressIntensity _intensity = ChatOpCompressIntensity::Low;
	float _tokenCalibrate=1.0f;

	std::vector<Op> _workingOps;      // 工作 Op 数组
	int _reduceTokenCount = 0;        // 目标减少的 token 数
	int _reducedTokens = 0;           // 已减少的 token 数（累加）
	int _currentPass = 0;             // 当前执行的 Pass

	AbsTick _compressStartTime = 0;     // 压缩开始时间
	static constexpr int _compressTimeLimitMs = 20;  // 每次压缩时间限制（毫秒）

	// 判断是否超时
	bool _IsCompressTimeout() const;
};

