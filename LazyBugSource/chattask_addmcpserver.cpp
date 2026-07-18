#include "stdh.h"
#include "ChatTask_AddMcpServer.h"
#include "Utils.h"
#include "Utils_Mcp.h"
#include "LlmChat.h"
#include "ChatAgent.h"
#include "ChatOpsCtrl.h"
#include "LlmMcps.h"
#include "LlmMcpServers.h"
#include "nlohmann/json.hpp"
#include "Utils_File.h"

extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);

CChatTask_AddMcpServer::CChatTask_AddMcpServer()
{
}

CChatTask_AddMcpServer::~CChatTask_AddMcpServer()
{
}

void CChatTask_AddMcpServer::_Fail(const char* reason)
{
	std::string result = "Error: ";
	result += reason ? reason : "Unknown error";
	_SendToolCallResult(result.c_str());
	_status = TaskStatus::Failure;
}

void CChatTask_AddMcpServer::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_AddMcpServer::Start()
{
	_status = TaskStatus::Running;
	_startTick = GetTickCount();

	// 获取 name 参数
	if (!_toolCall.GetStringParam("name", _name))
	{
		_Fail("Missing 'name' parameter");
		return;
	}

	// 获取 config 参数
	std::string configStr;
	if (!_toolCall.GetStringParam("config", configStr))
	{
		_Fail("Missing 'config' parameter");
		return;
	}

	// 获取 workingDir 参数 (可选)
	std::string workingDir;
	_toolCall.GetStringParam("workingDir", workingDir);

	// 解析 config: 先尝试作为 JSON 字符串, 失败则作为文件路径
	std::string description, command, url;
	std::vector<std::string> args;
	std::unordered_map<std::string, std::string> env;

	bool parsed = Utils::ParseMcpConfigFromString(configStr, description, command, args, env, url);

	if (!parsed)
	{
		// 尝试作为文件路径读取
		std::wstring wFilePath = utf8_to_widechar(configStr.c_str());
		parsed = ParseMcpJson(wFilePath.c_str(), description, command, args, env, url);
	}

	if (!parsed)
	{
		_Fail("Failed to parse 'config': not a valid JSON string or MCP config file path");
		return;
	}

	// 构造连接设置
	CLlmMcps::McpConnectSetting connect;
	connect.url = url;
	connect.command = command;
	connect.args = args;
	connect.env = env;
	connect.workingDir = workingDir;

	// 检查同名 mcp 是否已存在
	const CLlmMcps::Mcp* existing = g_llmMcps.FindMcpByName(_name);
	if (existing)
	{
		if (CLlmMcps::IsSameConnect(existing->connect, connect))
		{
			// 相同配置已存在, 返回成功
			std::string result = "MCP server '" + _name + "' already exists with the same configuration.";
			if (existing->toolsLoaded && !existing->tools.empty())
			{
				result += "\n\nAvailable tools:";
				for (const auto& tool : existing->tools)
					result += "\n- " + tool.name + ": " + tool.description;
			}
			else if (!existing->lastError.empty())
			{
				result += "\n\nNote: Server has an error: " + existing->lastError;
			}
			else
			{
				result += "\n\nNote: Tools are still loading or not yet available.";
			}
			_SendToolCallResult(result.c_str());
			_Succeed();
			return;
		}
		else
		{
			_Fail(("MCP server '" + _name + "' already exists with a different configuration. Use a different name.").c_str());
			return;
		}
	}

	// 检查是否有相同配置的 mcp (不同名称) 已存在
	for (const auto& mcp : g_llmMcps._mcps)
	{
		if (mcp.name == _name)
			continue;  // 已在上面处理
		if (CLlmMcps::IsSameConnect(mcp.connect, connect))
		{
			// 相同配置已存在(不同名称), 返回成功
			std::string result = "An MCP server with the same configuration already exists under name '" + mcp.name + "'.";
			if (mcp.toolsLoaded && !mcp.tools.empty())
			{
				result += "\n\nAvailable tools:";
				for (const auto& tool : mcp.tools)
					result += "\n- " + tool.name + ": " + tool.description;
			}
			else if (!mcp.lastError.empty())
			{
				result += "\n\nNote: Server has an error: " + mcp.lastError;
			}
			else
			{
				result += "\n\nNote: Tools are still loading or not yet available.";
			}
			_SendToolCallResult(result.c_str());
			_Succeed();
			return;
		}
	}

	// 动态添加 mcp
	_uid = g_llmMcps.AddDynamicMcp(_name.c_str(), connect, description.c_str());

	if (_uid == 0)
	{
		_Fail("Failed to add MCP server");
		return;
	}

	// 等待 server 启动, 由 Update() 轮询
}

void CChatTask_AddMcpServer::Update()
{
	if (_status != TaskStatus::Running)
		return;

	if (_uid == 0)
	{
		// Start 中已完成 (成功或失败), 不应到达此处
		return;
	}

	// 轮询 server 状态
	CLlmMcpServers::State state;
	std::string output;
	std::vector<CLlmMcps::Mcp::Tool> tools;

	bool found = g_llmMcpServers.GetServerStateByUid(_uid, state, output, &tools);

	if (!found)
	{
		// server 尚未创建, 等待 (UpdateSync 会在后续帧创建)
		DWORD elapsed = GetTickCount() - _startTick;
		if (elapsed > 60000)
		{
			_Fail("Timeout: MCP server was not created within 60 seconds");
			return;
		}
		return;  // 继续等待
	}

	if (state == CLlmMcpServers::State::Starting)
	{
		DWORD elapsed = GetTickCount() - _startTick;
		if (elapsed > 60000)
		{
			std::string result = "Error: Timeout waiting for MCP server to start. Server output:\n" + output;
			_SendToolCallResult(result.c_str());
			_Fail("Timeout waiting for MCP server to start");
			return;
		}
		return;  // 继续等待
	}

	if (state == CLlmMcpServers::State::Ready)
	{
		// 成功
		std::string result = "MCP server '" + _name + "' started successfully.";
		if (!tools.empty())
		{
			result += "\n\nAvailable tools:";
			for (const auto& tool : tools)
				result += "\n- " + tool.name + ": " + tool.description;
		}
		else
		{
			result += "\n\nNo tools available from this server.";
		}
		_SendToolCallResult(result.c_str());
		_SendToolCallMessage_Exploring(result.c_str());
		_Succeed();
		return;
	}

	if (state == CLlmMcpServers::State::Failed)
	{
		std::string result = "Error: MCP server '" + _name + "' failed to start.";
		if (!output.empty())
			result += "\n\nDetails:\n" + output;
		_SendToolCallResult(result.c_str());
		_Fail("MCP server failed to start");
		return;
	}
}

void CChatTask_AddMcpServer::Interrupt()
{
	if (_status == TaskStatus::Running)
		_status = TaskStatus::Failure;
}