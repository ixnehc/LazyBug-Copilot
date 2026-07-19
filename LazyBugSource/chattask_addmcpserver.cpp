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

bool CChatTask_AddMcpServer::_HandleExistingMcp(WUID existingUid, const CLlmMcps::Mcp& existingMcp)
{
	if (!existingMcp.enable || !existingMcp.disabledTools.empty())
	{
		// 未完全启用, 重新启用
		Utils::EnableMcpFullyAndReload(existingUid);
	}

	_uid = existingUid;

	// 发送 "Starting" 标签
	json startingJson;
	startingJson["name"] = _name;
	startingJson["starting"] = true;
	_SendToolCallMessage_AddMcpServer(startingJson.dump().c_str());

	// 由 Update() 等待 server Ready/Failed 后统一处理
	return false;
}

void CChatTask_AddMcpServer::Start()
{
	_status = TaskStatus::Running;

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
		json displayJson;
		displayJson["name"] = _name;
		displayJson["success"] = false;
		displayJson["message"] = "Failed to parse config: not valid JSON or MCP config file path";
		_SendToolCallMessage_AddMcpServer(displayJson.dump().c_str());

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
			_HandleExistingMcp(existing->uid, *existing);
			return;
		}
		else
		{
			std::string errMsg = "MCP server '" + _name + "' already exists with a different configuration. Use a different name.";
			_Fail(errMsg.c_str());

			json displayJson;
			displayJson["name"] = _name;
			displayJson["success"] = false;
			displayJson["message"] = "Name already exists with different configuration";
			_SendToolCallMessage_AddMcpServer(displayJson.dump().c_str());

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
			_existingName = mcp.name;
			_HandleExistingMcp(mcp.uid, mcp);
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

	_isNewMcp = true;

	// 发送 "Starting" 标签
	json startingJson;
	startingJson["name"] = _name;
	startingJson["starting"] = true;
	_SendToolCallMessage_AddMcpServer(startingJson.dump().c_str());

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
		return;  // 继续等待
	}

	if (state == CLlmMcpServers::State::Starting)
	{
		// 等待 server 线程完成, CLlmMcpServers 内部有多级超时 (initialize 10分钟, tools/list 5秒)
		return;  // 继续等待
	}

	if (state == CLlmMcpServers::State::Ready)
	{
		// 成功
		std::string result = "MCP server '" + _name + "' started successfully.";
		if (!_existingName.empty())
			result += "\nNote: This configuration already exists as '" + _existingName + "'.";
		std::string toolListStr;
		int toolCount = (int)tools.size();
		if (!tools.empty())
		{
			result += "\n\nAvailable tools:";
			for (const auto& tool : tools)
			{
				result += "\n- " + tool.name + ": " + tool.description;
				toolListStr += tool.name + "\n";
			}
		}
		else
		{
			result += "\n\nNo tools available from this server.";
		}
		_SendToolCallResult(result.c_str());

		json displayJson;
		displayJson["name"] = _name;
		displayJson["success"] = true;
		displayJson["message"] = (toolCount > 0 ? ("Loaded " + std::to_string(toolCount) + " tool(s)") : "No tools available")
			+ (_existingName.empty() ? "" : (" (already exists as '" + _existingName + "')"));
		displayJson["toolCount"] = toolCount;
		_SendToolCallMessage_AddMcpServer(displayJson.dump().c_str());

		_Succeed();
		return;
	}

	if (state == CLlmMcpServers::State::Failed)
	{
		std::string result = "Error: MCP server '" + _name + "' failed to start.";
		if (!_existingName.empty())
			result += "\nNote: This configuration already exists as '" + _existingName + "'.";
		if (!output.empty())
			result += "\n\nDetails:\n" + output;
		_SendToolCallResult(result.c_str());

		json displayJson;
		displayJson["name"] = _name;
		displayJson["success"] = false;
		displayJson["message"] = output
			+ (_existingName.empty() ? "" : (" (already exists as '" + _existingName + "')"));
		_SendToolCallMessage_AddMcpServer(displayJson.dump().c_str());

		// 如果是本次新增的 mcp, 连接失败则删除
		if (_isNewMcp)
			g_llmMcps.RemoveDynamicMcp(_uid);

		_status = TaskStatus::Failure;
		return;
	}
}

void CChatTask_AddMcpServer::Interrupt()
{
	if (_status != TaskStatus::Running)
		return;

	_status = TaskStatus::Failure;

	// 如果正在等待 MCP 连接完成, 清理 UI Starting 标签
	if (_uid != 0)
	{
		_RemoveToolCallMessage_AddMcpServer();

		// 如果是本次新增的 mcp, 删除它
		if (_isNewMcp)
		{
			g_llmMcps.RemoveDynamicMcp(_uid);
		}
	}
}