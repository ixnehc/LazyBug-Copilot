#pragma once

#include "SmartRepairDefines.h"

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "timer/timer.h"

// AI辅助编码补全触发器
// 负责根据用户键盘输入决定何时触发AI补全、显示建议或隐藏建议
// 采用状态机设计，通过轮询接口与编辑器通信
//
// 典型使用流程：
// 1. 编辑器在用户输入时调用 OnCharTyped() 或 OnKeyDown()
// 2. 编辑器在主循环中定期调用 Poll() 获取下一步动作
// 3. 编辑器根据返回的动作执行相应操作（发起AI请求、显示/隐藏建议等）
// 4. 当AI结果返回时，编辑器调用 SetAIResult() 将结果传递给触发器
class CSmartRepairTrigger
{
public:

	// Poll()方法返回的指令
	struct Instruction
	{
		// 编辑器需要执行的动作
		enum class Action {
			None,				// 无事可做
			PrepareContext,		// 编辑器应该准备AI请求的上下文信息
			TriggerAIRequest,	// 编辑器应该立即异步调用AI服务
			ShowSuggestion,		// 编辑器应该显示AI建议为"幽灵文本"
			HideSuggestion,		// 编辑器应该隐藏当前显示的"幽灵文本"
			AcceptSuggestion	// 编辑器应该将"幽灵文本"固化为真实代码
		};

		Action action = Action::None;

		// 当 action 是 PrepareContext 或 TriggerAIRequest 时，此ID有效
		// 编辑器在准备上下文和发起异步请求时必须携带此ID，并在收到结果后传回
		SmartRepairSessionID sessionID = 0;
	};

	CSmartRepairTrigger();
	~CSmartRepairTrigger();

	// ========== 输入接口 (由编辑器调用) ==========

	// 当用户输入一个可见字符时调用
	// 调用时机：用户在编辑器中输入任何可见字符（字母、数字、符号等）
	void OnCharTyped(char ch);

	// 当用户按下特殊按键时调用
	// 调用时机：用户按下非可见字符的按键（Tab、Escape、方向键等）
	// 在按键的 KeyDown 事件中调用，而不是 KeyUp
	void OnKeyDown(int vkCode);

	// 当光标位置发生移动时调用
	// 调用时机：用户通过鼠标点击改变光标位置、方向键移动光标、查找跳转等
	void OnCursorMoved();

	// 当文档发生非键盘输入导致的重大变化时调用
	// 调用时机：撤销/重做、外部文件修改、格式化、重构、粘贴大段文本等
	void OnDocumentChanged();

	// 当编辑器完成了 PrepareContext 动作后调用
	// 调用时机：编辑器已准备好上下文信息，可以发起AI请求时
	void NotifyContextPrepared(SmartRepairSessionID sessionID);

	// 当异步的AI调用返回结果后调用
	// 调用时机：编辑器发起异步AI请求后，AI服务返回结果时在网络回调中调用
	// 注意：如果传入的 sessionID 与当前会话不匹配，结果将被忽略
	void SetAIResult(SmartRepairSessionID sessionID);

	// 当编辑器完成了 AcceptSuggestion 动作后调用
	// 调用时机：编辑器已将"幽灵文本"固化为真实代码，并移动了光标后
	void NotifySuggestionAccepted();

	// 当编辑器完成了 HideSuggestion 动作后调用
	// 调用时机：编辑器已隐藏了当前显示的"幽灵文本"后
	void NotifySuggestionHidden();

	// ========== 轮询接口 (由编辑器在主循环中调用) ==========

	// 轮询下一步应执行的动作
	// 调用时机：编辑器在主循环的每一帧中调用（建议30-60 FPS）
	// 这是触发器与编辑器通信的主要接口
	Instruction Poll();

	// 获取当前的AI建议文本
	// 调用时机：仅当 Poll() 返回 ShowSuggestion 动作时调用
	const char* GetSuggestionText() const;

	// ========== 配置接口 ==========

	// 启用或禁用触发器
	void SetEnabled(bool enabled);

	// 获取触发器当前的启用状态
	bool IsEnabled() const;

	// 设置触发延迟时间
	// delayMs: 用户停止输入后多少毫秒触发AI请求（默认500ms）
	void SetTriggerDelay(int delayMs);

private:
	// ========== 内部状态枚举 ==========
	enum class State
	{
		Idle,					// 空闲状态，没有正在进行的补全会话
		WaitingForPause,		// 用户正在输入，等待输入暂停以触发AI请求
		ContextPreparing,		// 正在准备上下文，等待编辑器完成上下文准备
		RequestPending,			// 已决定需要AI补全，等待编辑器发起请求
		SuggestionAvailable,	// AI已返回建议，等待显示
		SuggestionDisplayed		// 建议已显示，等待用户交互
	};

	// ========== 私有成员变量 ==========
	
	bool _isEnabled;						// 触发器启用状态
	State _currentState;					// 当前内部状态
	int _sessionIDCounter;				// 会话ID计数器，用于生成唯一的会话标识
	SmartRepairSessionID _currentSessionID;			// 当前活跃的会话ID
	int _triggerDelayMs;					// 触发延迟时间（毫秒）
	AbsTick _triggerTimer;
	Instruction _pendingInstruction;	// 待返回的指令（用于在Poll中返回）

	// ========== 私有辅助方法 ==========
	
	// 检查当前是否适合触发AI补全
	bool _ShouldTriggerCompletion() const;
	
	// 重置到空闲状态
	void _ResetToIdle();
	
	// 生成新的会话ID
	SmartRepairSessionID _GenerateNewSessionID();
	
	// 启动触发延迟定时器
	void _StartTriggerTimer();
	
	// 停止触发延迟定时器
	void _StopTriggerTimer();
	
	// 定时器回调函数
	void _OnTriggerTimerExpired();
};
