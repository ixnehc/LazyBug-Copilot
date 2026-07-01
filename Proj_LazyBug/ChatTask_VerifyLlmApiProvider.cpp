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

	// 查找可用于验证的API（优先Auxiliary角色，且model非空）
	std::string apiName = g_llmLib.FindApiToValidateApiKey(_providerTypeName);
	if (apiName.empty())
	{
		_Fail();
		return;
	}

	LlmSessionSetting setting;
	if (g_llmLib.LoadLlmSetting(setting, apiName,true, ""))
	{
		setting.api.tools.clear();
		setting.rulesFiles.clear();

		// 检测endpoint后缀，如果为embedding endpoint，使用embedding专用验证流程
		bool isEmbeddingEndpoint = false;
		const LlmApiProvider* provider = g_llmLib.GetProvider(_providerTypeName);
		if (provider)
		{
			std::string endpoint = provider->endpoint;
			while (!endpoint.empty() && endpoint.back() == '/')
				endpoint.pop_back();
			std::transform(endpoint.begin(), endpoint.end(), endpoint.begin(), ::tolower);
			if (endpoint.size() >= 11 && endpoint.compare(endpoint.size() - 11, 11, "/embeddings") == 0)
				isEmbeddingEndpoint = true;
		}

		if (isEmbeddingEndpoint)
		{
			// Embedding endpoint: 使用异步embedding请求验证
			if (!_llmChat->RequestEmbedding(u8"test", setting))
			{
				_Fail();
				return;
			}
			_hasStartedRequest = true;
			return;
		}

		// 普通Chat endpoint: 使用聊天请求验证
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
				if (output.content.empty() && output.fullContent.empty())
				{
					_Fail();
				}
				else
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
	}
	else if (_hasStartedRequest)
	{
		_Fail();
	}
}

void CChatTask_VerifyLlmApiProvider::Interrupt()
{ 
	_requestInterrupt = true; 
	Update();
}
