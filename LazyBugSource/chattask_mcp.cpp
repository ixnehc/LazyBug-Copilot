#include "stdh.h"
#include "ChatTask_Mcp.h"
#include "Utils.h"
#include "LlmChat.h"
#include "ChatAgent.h"
#include "ChatOpsCtrl.h"
#include "LlmMcpServers.h"
#include "LlmMcps.h"
#include "nlohmann/json.hpp"

// 递归遍历 JSON，将所有叶子节点扁平化为 path=value 对
static void _FlattenJson(const json& j, const std::string& prefix, json& outArr)
{
	if (j.is_object())
	{
		for (auto it = j.begin(); it != j.end(); ++it)
		{
			std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();
			_FlattenJson(it.value(), key, outArr);
		}
	}
	else if (j.is_array())
	{
		// 基本类型数组：合并为一行
		bool allPrimitive = true;
		for (const auto& elem : j)
		{
			if (elem.is_object() || elem.is_array())
			{
				allPrimitive = false;
				break;
			}
		}
		if (allPrimitive)
		{
			std::string valStr = j.dump();
			outArr.push_back({ {"k", prefix}, {"v", valStr} });
		}
		else
		{
			for (size_t i = 0; i < j.size(); ++i)
			{
				std::string key = prefix + "[" + std::to_string(i) + "]";
				_FlattenJson(j[i], key, outArr);
			}
		}
	}
	else
	{
		// 叶子节点：string/number/boolean/null
		std::string valStr;
		if (j.is_string())
			valStr = j.get<std::string>();
		else
			valStr = j.dump();
		outArr.push_back({ {"k", prefix}, {"v", valStr} });
	}
}

std::string CChatTask_Mcp::_ExtractArgsSummary(const std::string& rawArguments)
{
	if (rawArguments.empty())
		return "[]";

	try
	{
		json parsed = json::parse(rawArguments);
		if (!parsed.is_object())
			return "[]";

		json arr = json::array();
		_FlattenJson(parsed, "", arr);
		return arr.dump();
	}
	catch (const json::exception&)
	{
		return "[]";
	}
}

CChatTask_Mcp::CChatTask_Mcp()
{
	_hCancelEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);  // 手动复位，初始无信号
}

CChatTask_Mcp::~CChatTask_Mcp()
{
	Interrupt();
	if (_hCancelEvent)
	{
		CloseHandle(_hCancelEvent);
		_hCancelEvent = nullptr;
	}
}

bool CChatTask_Mcp::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("Mcp"))
		return false;
	return true;
}

void CChatTask_Mcp::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_Mcp::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_Mcp::Start()
{
	_status = TaskStatus::Running;
	_done.store(false);
	_success.store(false);

	// 通过 CChatOpsCtrl 添加 MCP 显示
	if (_context && _context->chatOpsCtrl && _context->chatAgent)
	{
		std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
		std::string argsSummary = _ExtractArgsSummary(_toolCall.raw_arguments);
		
		// 通过别名查找真实的 MCP 名称和工具名称
		const auto* aliasInfo = g_llmMcps.FindToolByAlias(_toolCall.mcpName);
		if (aliasInfo)
		{
			// 找到 MCP
			for (const auto& mcp : g_llmMcps._mcps)
			{
				if (mcp.uid == aliasInfo->mcpUid)
				{
					_mcpId = _context->chatOpsCtrl->AddMcpDisplay(messageId, mcp.name, aliasInfo->realToolName, _toolCall.raw_arguments, argsSummary);
					break;
				}
			}
		}
		else
		{
			// 未找到别名，使用原始名称
			_mcpId = _context->chatOpsCtrl->AddMcpDisplay(messageId, "", _toolCall.mcpName, _toolCall.raw_arguments, argsSummary);
		}
	}

	// 在线程中同步调用MCP工具
	_thread = std::thread([this]()
	{
		std::string result;
		bool ok = g_llmMcpServers.CallTool(_toolCall.mcpName, _toolCall.raw_arguments, result, _hCancelEvent);
		_result = result;
		_success.store(ok);
		_done.store(true);
	});
}

void CChatTask_Mcp::Update()
{
	if (_status != TaskStatus::Running)
		return;

	// 检查用户是否点击了停止按钮
	if (_context && _context->chatUi && !_mcpId.empty())
	{
		McpStatus status = _context->chatUi->GetMcpStatus(_mcpId);
		if (status == McpStatus::Stop)
		{
			// 用户点击停止按钮，中断 MCP 调用
			Interrupt();

			// 等待线程结束
			if (_thread.joinable())
				_thread.join();

			// 更新 UI 显示停止信息
			_result = "MCP tool call was stopped by user";
			if (_context && _context->chatOpsCtrl && _context->chatAgent)
			{
				std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
				_context->chatOpsCtrl->SetMcpResultToLastMcpDisplay(messageId, _result);
			}

			// 发送工具调用结果
			_SendToolCallResult(_result.c_str());
			_Fail();
			return;
		}
	}

	if (_done.load())
	{
		if (_thread.joinable())
			_thread.join();

		// 更新 UI 中的结果
		if (_context && _context->chatOpsCtrl && _context->chatAgent)
		{
			std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
			_context->chatOpsCtrl->SetMcpResultToLastMcpDisplay(messageId, _result);
		}

		_SendToolCallResult(_result.c_str());
		if (_success.load())
			_Succeed();
		else
			_Fail();
	}
}

void CChatTask_Mcp::Interrupt()
{
	// 触发取消事件，让 _SendMcpRequest 立即返回
	if (_hCancelEvent)
	{
		SetEvent(_hCancelEvent);
	}

	// 等待线程结束
	if (_thread.joinable())
		_thread.join();

	// 隐藏停止按钮并显示中断信息
	if (_context && _context->chatOpsCtrl && _context->chatAgent)
	{
		std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
		if (_result.empty())
			_result = "MCP tool call was interrupted";
		_context->chatOpsCtrl->SetMcpResultToLastMcpDisplay(messageId, _result);
	}

	if (_context && _context->chatUi && !_mcpId.empty())
	{
		_context->chatUi->SetMcpStatus(_mcpId, McpStatus::None);
	}

	if (_status == TaskStatus::Running)
		_status = TaskStatus::Failure;
}