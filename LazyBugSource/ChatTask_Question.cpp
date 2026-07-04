#include "stdh.h"
#include "ChatTask_Question.h"
#include "Utils.h"
#include "LlmChat.h"
#include "ChatAgent.h"
#include "ChatOpsCtrl.h"
#include <sstream>

CChatTask_Question::CChatTask_Question()
{
	_waitingForUser = false;
	_questionId = 0;
}

CChatTask_Question::~CChatTask_Question()
{
}

bool CChatTask_Question::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("Question"))
		return false;
	return true;
}

void CChatTask_Question::_Fail()
{
	_ClearQuestionInUi();

	_status = TaskStatus::Failure;
}

void CChatTask_Question::_Succeed()
{
	_ClearQuestionInUi();

	_status = TaskStatus::Success;
}

void CChatTask_Question::SetUserAnswer(const std::string& answer)
{
	_userAnswer = answer;
	_waitingForUser = false;
}

void CChatTask_Question::Start()
{
	_status = TaskStatus::Running;
	_waitingForUser = true;

	// 获取问题参数
	std::string question;
	if (!_toolCall.GetStringParam("question", question))
	{
		_SendToolCallResult("Error: Missing 'question' parameter");
		_Fail();
		return;
	}

	_question = question;

	// 获取选项参数
	std::string optionsStr;
	if (!_toolCall.GetStringParam("options", optionsStr))
	{
		_SendToolCallResult("Error: Missing 'options' parameter");
		_Fail();
		return;
	}

	// 解析选项（用逗号分隔）
	std::vector<std::string> options;
	std::stringstream ss(optionsStr);
	std::string option;
	while (std::getline(ss, option, ','))
	{
		// 去除前后空格
		size_t start = option.find_first_not_of(" \t");
		size_t end = option.find_last_not_of(" \t");
		if (start != std::string::npos && end != std::string::npos)
		{
			options.push_back(option.substr(start, end - start + 1));
		}
	}

	// 调用 UI 接口显示问题
	if (_context && _context->chatUi && _context->chatAgent)
	{
		// 获取当前 AI 消息的 ID
		std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
		
		// 转换为宽字符
		std::wstring wQuestion = utf8_to_widechar(question);
		std::vector<std::wstring> wOptions;
		for (const auto& opt : options)
		{
			wOptions.push_back(utf8_to_widechar(opt));
		}
		
		// 调用 UI 接口添加问题窗口
		_questionId = _context->chatUi->AddQuestion(messageId, wQuestion, wOptions);
	}
	else
	{
		// 如果 UI 不可用，直接返回错误
		_SendToolCallResult("Error: UI interface not available");
		_Fail();
	}
}

void CChatTask_Question::Update()
{
	if (_status != TaskStatus::Running)
		return;

	// 检查是否收到用户回答
	if (_waitingForUser && _context && _context->chatUi)
	{
		// 检查用户是否已经回答
		if (_context->chatUi->HasQuestionAnswer(_questionId))
		{
			// 获取用户的答案
			std::wstring wAnswer;
			if (_context->chatUi->GetQuestionAnswer(_questionId, wAnswer))
			{
				_userAnswer = widechar_to_utf8(wAnswer.c_str());
				_waitingForUser = false;
				
				// 显示问题和答案
				if (_context && _context->chatOpsCtrl && _context->chatAgent)
				{
					std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
					std::wstring wQuestion = utf8_to_widechar(_question);
					_context->chatOpsCtrl->AddQuestionDisplay(messageId, wQuestion, _userAnswer);
				}
				
				// 返回结果给 LLM
				std::string result = "User answer: " + _userAnswer;
				_SendToolCallResult(result.c_str());
				_Succeed();
			}
		}
	}
}

void CChatTask_Question::Interrupt()
{
	_ClearQuestionInUi();
	_status = TaskStatus::Failure;
	_waitingForUser = false;
}

void CChatTask_Question::_ClearQuestionInUi()
{
	if (_context && _context->chatUi)
		_context->chatUi->ClearQuestion();
}
