#include "stdh.h"
#include "ChatTask_VerifyLlmApiProvider.h"

#include "ChatSettingPage.h"
#include <algorithm>
#include <cstring>
#include "Utils.h"

#include "LlmChat.h"

#include "LlmLib.h"


// 声明外部函数
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);

CChatTask_VerifyLlmApiProvider::CChatTask_VerifyLlmApiProvider(const LlmApiProviderTypeName& providerTypeName)
{
	_providerTypeName = providerTypeName;
	_hasStartedRequest = false;
}

bool CChatTask_VerifyLlmApiProvider::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("VerifyLlmApiProvider"))
		return false;

	CChatTask_VerifyLlmApiProvider* task = (CChatTask_VerifyLlmApiProvider*)task0;
	if (task->_providerTypeName == _providerTypeName)
		return true;
	return false;
}

void CChatTask_VerifyLlmApiProvider::_Fail()
{
	g_llmLib.SetProviderStatus(_providerTypeName, LlmApiProvider::Status::Unavailable);
	if (_context->chatSettingPage)
		_context->chatSettingPage->EndValidatingProvider(_providerTypeName, false);
	g_llmLib.SaveSettings();
	_status = TaskStatus::Failure;
}

void CChatTask_VerifyLlmApiProvider::_Succeed()
{
	g_llmLib.SetProviderStatus(_providerTypeName, LlmApiProvider::Status::Ok);
	if (_context->chatSettingPage)
		_context->chatSettingPage->EndValidatingProvider(_providerTypeName, true);
	g_llmLib.SaveSettings();
	_status = TaskStatus::Success;
}

void CChatTask_VerifyLlmApiProvider::Start()
{
	if (_context->chatSettingPage)
		_context->chatSettingPage->StartValidatingProvider(_providerTypeName);

	LlmSessionSetting setting;
	if (g_llmLib.LoadLlmSetting(setting, LlmApiPurpose::MinorChat, _providerTypeName, true, ""))
	{
		setting.api.tools.clear();
		setting.rulesFiles.clear();
		LlmSessionRequest request;
		request.AddUserMessage(u8"Please give me an animal's name of 4 letters");
		request.isStreaming = false;

		if (!_llmChat->Request(request, setting))
		{
			_Fail();
			return;
		}
		_hasStartedRequest = true;
		return;
	}

	if (g_llmLib.LoadLlmSetting(setting, LlmApiPurpose::Complete, _providerTypeName, true, ""))
	{
		LlmSessionRequest request;
		request.AddUserMessage(u8"Please give me an animal's name of 4 letters");
		request.isStreaming = false;

		if (!_llmChat->Request(request, setting))
		{
			_Fail();
			return;
		}
		_hasStartedRequest = true;
		return;
	}

	if (g_llmLib.LoadLlmSetting(setting, LlmApiPurpose::MajorChat, _providerTypeName, true, ""))
	{
		LlmSessionRequest request;
		request.AddUserMessage(u8"Please give me an animal's name of 4 letters");
		request.isStreaming = false;

		if (!_llmChat->Request(request, setting))
		{
			_Fail();
			return;
		}
		_hasStartedRequest = true;
		return;
	}

	if (g_llmLib.LoadLlmSetting(setting, LlmApiPurpose::FastApply_Dedicated, _providerTypeName, true, ""))
	{
		std::string message;
		message += u8"<code>";
		message += u8"int main()";
		message += u8"{";
		message += u8"}";
		message += u8"</code>\n";
		message += u8"<code>";
		message += u8"int main()";
		message += u8"{";
		message += u8" return 0";
		message += u8"}";
		message += u8"</code>\n";

		LlmSessionRequest request;
		request.AddUserMessage( message.c_str());
		request.isStreaming = false;

		// 发送请求到LLM
		if (!_llmChat->Request(request, setting))
		{
			_Fail();
			return;
		}
		_hasStartedRequest = true;
		return;
	}

	_Fail();
}

void CChatTask_VerifyLlmApiProvider::Update()
{
	if (!_llmChat)
		return;

	// 检查LLM会话状态
	if (_llmChat->HasActiveSession())
	{
		LlmSessionOutput output;
		
		if (_llmChat->Process(output, _requestInterrupt))
		{
			// 检查会话是否完成
			if (output.isCompleted)
			{
				if (output.hasError)
				{
					_Fail();
				}
				else if (!_requestInterrupt)
				{
					_Succeed();
				}
				else
				{
					_Fail();
				}
			}
		}
	}
	else if (_hasStartedRequest)
	{
		_Fail();
	}
}
