#include "stdh.h"
#include "SmartRepairTrigger.h"

extern AbsTick GetAbsTick();

CSmartRepairTrigger::CSmartRepairTrigger()
	: _isEnabled(true)
	, _currentState(State::Idle)
	, _sessionIDCounter(0)
	, _currentSessionID(SmartRepairSessionID_Invalid)
	, _triggerDelayMs(500)
	, _triggerTimer(0)
{
	_pendingInstruction.action = Instruction::Action::None;
	_pendingInstruction.sessionID = SmartRepairSessionID_Invalid;
}

CSmartRepairTrigger::~CSmartRepairTrigger()
{
}

// ========== 输入接口 (由编辑器调用) ==========

void CSmartRepairTrigger::OnCharTyped(char ch)
{
	if (!_isEnabled)
		return;

	switch (_currentState)
	{
	case State::Idle:
		// 开始新的输入会话，启动延迟定时器
		if (_ShouldTriggerCompletion())
		{
			_currentState = State::WaitingForPause;
			_StartTriggerTimer();
		}
		break;

	case State::WaitingForPause:
		// 用户继续输入，重置定时器
		if (_ShouldTriggerCompletion())
		{
			_StartTriggerTimer(); // 重置定时器
		}
		else
		{
			// 输入了不合适的字符，取消触发
			_ResetToIdle();
		}
		break;

	case State::ContextPreparing:
		// 正在准备上下文，用户继续输入
		// 当前上下文准备将变为无效，开始新的会话
		_currentState = State::WaitingForPause;
		_StartTriggerTimer();
		break;

	case State::RequestPending:
		// AI请求已发出但未返回，用户继续输入
		// 当前请求将变为无效，但不立即取消，等AI返回时会被忽略
		_currentState = State::WaitingForPause;
		_StartTriggerTimer();
		break;

	case State::SuggestionAvailable:
		// 有可用建议但未显示，用户继续输入
		_currentState = State::WaitingForPause;
		_StartTriggerTimer();
		break;

	case State::SuggestionDisplayed:
		// 建议正在显示，用户继续输入，需要隐藏当前建议
		_pendingInstruction.action = Instruction::Action::HideSuggestion;
		_currentState = State::WaitingForPause;
		_StartTriggerTimer();
		break;
	}
}

void CSmartRepairTrigger::OnKeyDown(int vkCode)
{
	if (!_isEnabled)
		return;

	switch (vkCode)
	{
// 	case VK_CONTROL:
// 		{
// 			// 手动触发补全,测试用
// 			_currentState = State::RequestPending;
// 			_pendingInstruction.action = Instruction::Action::TriggerAIRequest;
// 			_currentSessionID = _GenerateNewSessionID();
// 			_pendingInstruction.sessionID = _currentSessionID;
// 			break;
// 		}
	case VK_TAB:
		if (_currentState == State::SuggestionDisplayed)
		{
			// 用户接受建议
			_pendingInstruction.action = Instruction::Action::AcceptSuggestion;
		}
		break;

	case VK_ESCAPE:
		if (_currentState == State::SuggestionDisplayed)
		{
			// 用户拒绝建议
			_pendingInstruction.action = Instruction::Action::HideSuggestion;
		}
		else
		{
			// 取消当前的触发过程
			_ResetToIdle();
		}
		break;

	case VK_RETURN:
		// 换行通常意味着当前行结束，隐藏建议
		if (_currentState == State::SuggestionDisplayed)
		{
			_pendingInstruction.action = Instruction::Action::HideSuggestion;
		}
		_ResetToIdle();
		break;

	case VK_BACK:
	case VK_DELETE:
		if (_currentState == State::SuggestionDisplayed)
		{
// 			// 删除字符时，检查建议是否仍然有效
// 			if (_IsSuggestionStillValid())
// 			{
// 				// 建议可能仍然有效，但需要重新评估
// 				_currentState = State::WaitingForPause;
// 				_StartTriggerTimer();
// 			}
// 			else
			{
				_pendingInstruction.action = Instruction::Action::HideSuggestion;
				_currentState = State::Idle;
			}
		}
		else if (_currentState == State::WaitingForPause)
		{
			// 在等待期间删除字符，重新开始计时
			_StartTriggerTimer();
		}
		break;

	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_HOME:
	case VK_END:
	case VK_PRIOR:  // Page Up
	case VK_NEXT:   // Page Down
		// 光标移动，隐藏建议
		if (_currentState == State::SuggestionDisplayed)
		{
			_pendingInstruction.action = Instruction::Action::HideSuggestion;
		}
		_ResetToIdle();
		break;

	case VK_SPACE:
		// 检查是否是 Ctrl+Space 手动触发补全
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
		// 手动触发补全
		_currentState = State::ContextPreparing;
		_pendingInstruction.action = Instruction::Action::PrepareContext;
		_currentSessionID = _GenerateNewSessionID();
		_pendingInstruction.sessionID = _currentSessionID;
		}
		else
		{
			// 普通空格，按字符输入处理
			OnCharTyped(' ');
		}
		break;
	}
}

void CSmartRepairTrigger::OnCursorMoved()
{
	if (!_isEnabled)
		return;

	// 光标移动通常意味着用户点击了其他位置或使用了查找跳转等功能
	if (_currentState == State::SuggestionDisplayed)
	{
		_pendingInstruction.action = Instruction::Action::HideSuggestion;
	}
	_ResetToIdle();
}

void CSmartRepairTrigger::OnDocumentChanged()
{
	if (!_isEnabled)
		return;

	// 文档发生重大变化，取消所有进行中的操作
	if (_currentState == State::SuggestionDisplayed)
	{
		_pendingInstruction.action = Instruction::Action::HideSuggestion;
	}
	_ResetToIdle();
}

void CSmartRepairTrigger::NotifyContextPrepared(SmartRepairSessionID sessionID)
{
	if (!_isEnabled)
		return;

	// 检查会话ID是否匹配
	if (sessionID != _currentSessionID)
	{
		// 这是一个过期的通知，直接忽略
		return;
	}

	if (_currentState == State::ContextPreparing)
	{
		// 上下文准备完成，可以发起AI请求
		_currentState = State::RequestPending;
		_pendingInstruction.action = Instruction::Action::TriggerAIRequest;
		_pendingInstruction.sessionID = sessionID;
	}
}

void CSmartRepairTrigger::SetAIResult(SmartRepairSessionID sessionID)
{
	if (!_isEnabled)
		return;

	// 检查会话ID是否匹配，防止过期结果干扰
	if (sessionID != _currentSessionID)
	{
		// 这是一个过期的结果，直接忽略
		return;
	}

	if (_currentState == State::RequestPending)
	{
		_currentState = State::SuggestionAvailable;
		_pendingInstruction.action = Instruction::Action::ShowSuggestion;
		_pendingInstruction.sessionID = sessionID;
	}
}

void CSmartRepairTrigger::NotifySuggestionAccepted()
{
	if (_currentState == State::SuggestionDisplayed)
	{
		_ResetToIdle();
	}
}

void CSmartRepairTrigger::NotifySuggestionHidden()
{
	if (_currentState == State::SuggestionDisplayed)
	{
		_ResetToIdle();
	}
}

// ========== 轮询接口 (由编辑器在主循环中调用) ==========

CSmartRepairTrigger::Instruction CSmartRepairTrigger::Poll()
{
	if (!_isEnabled)
	{
		Instruction instruction;
		instruction.action = Instruction::Action::None;
		return instruction;
	}

	// 检查定时器是否到期
	if (_currentState == State::WaitingForPause && _triggerTimer != 0)
	{
		AbsTick currentTime = GetAbsTick();
		if (currentTime - _triggerTimer >= _triggerDelayMs)
		{
			_OnTriggerTimerExpired();
		}
	}

	// 返回待执行的指令
	Instruction result = _pendingInstruction;

	// 根据指令更新状态
	if (_pendingInstruction.action == Instruction::Action::PrepareContext)
	{
		_currentState = State::ContextPreparing;
		_pendingInstruction.action = Instruction::Action::None;
	}
	else if (_pendingInstruction.action == Instruction::Action::ShowSuggestion)
	{
		_currentState = State::SuggestionDisplayed;
		_pendingInstruction.action = Instruction::Action::None;
	}
	else if (_pendingInstruction.action == Instruction::Action::HideSuggestion ||
			 _pendingInstruction.action == Instruction::Action::AcceptSuggestion)
	{
		_pendingInstruction.action = Instruction::Action::None;
		// 状态已在相应的通知函数中处理
	}
	else if (_pendingInstruction.action == Instruction::Action::TriggerAIRequest)
	{
		_pendingInstruction.action = Instruction::Action::None;
		// 状态已在设置指令时更新
	}

	return result;
}

// ========== 配置接口 ==========

void CSmartRepairTrigger::SetEnabled(bool enabled)
{
	_isEnabled = enabled;
	if (!enabled)
	{
		_ResetToIdle();
	}
}

bool CSmartRepairTrigger::IsEnabled() const
{
	return _isEnabled;
}

void CSmartRepairTrigger::SetTriggerDelay(int delayMs)
{
	_triggerDelayMs = delayMs;
}

// ========== 私有辅助方法 ==========

bool CSmartRepairTrigger::_ShouldTriggerCompletion() const
{
	// 简化的触发条件判断
	// 实际实现中可能需要分析当前光标位置的上下文
	// 这里假设大部分字符输入都可能触发补全
	return true;
}

void CSmartRepairTrigger::_ResetToIdle()
{
	_currentState = State::Idle;
	_StopTriggerTimer();
	_pendingInstruction.action = Instruction::Action::None;
	_pendingInstruction.sessionID = 0;
}

SmartRepairSessionID CSmartRepairTrigger::_GenerateNewSessionID()
{
	return ++_sessionIDCounter;
}

void CSmartRepairTrigger::_StartTriggerTimer()
{
	_triggerTimer = GetAbsTick();
}

void CSmartRepairTrigger::_StopTriggerTimer()
{
	_triggerTimer = 0;
}

void CSmartRepairTrigger::_OnTriggerTimerExpired()
{
	if (_currentState == State::WaitingForPause)
	{
		// 用户输入暂停，开始准备上下文
		_currentState = State::ContextPreparing;
		_pendingInstruction.action = Instruction::Action::PrepareContext;
		_currentSessionID = _GenerateNewSessionID();
		_pendingInstruction.sessionID = _currentSessionID;
		_StopTriggerTimer();
	}
}
