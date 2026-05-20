#include "stdh.h"
#include "ChatTask_QueryFinish.h"
#include "Utils.h"
#include "LlmChat.h"
#include "ChatAgent.h"
#include "ChatOpsCtrl.h"

CChatTask_QueryFinish::CChatTask_QueryFinish()
{
}

CChatTask_QueryFinish::~CChatTask_QueryFinish()
{
}

bool CChatTask_QueryFinish::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("QueryFinish"))
		return false;
	return true;
}

void CChatTask_QueryFinish::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_QueryFinish::_Succeed()
{
	_status = TaskStatus::Success;
}


void CChatTask_QueryFinish::Start()
{
	_status = TaskStatus::Running;
}

void CChatTask_QueryFinish::Update()
{
	if (_status != TaskStatus::Running)
		return;

	_SendToolCallResult("Ok");
}

void CChatTask_QueryFinish::Interrupt()
{
	_status = TaskStatus::Failure;
}

