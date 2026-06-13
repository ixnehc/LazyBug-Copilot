#include "stdh.h"

#include "ChatOpsCtrl.h"
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <functional>
#include "timer/wuid.h"
#include "datapacket/DataPacket.h"
#include "LlmSession.h"
#include "stringparser/stringparser.h"

#include "Utils.h"
#include "utils_image.h"
#include "Utils_Context.h"

#include "chatopscompress.h"
#include "LlmTools.h"

// JSON 库
#include "nlohmann/json.hpp"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const char* str);

//////////////////////////////////////////////////////////////////////////
//ChatOp

// ChatOp版本定义已删除

void ChatOp::Save(CDataPacket& dp)
{
	// 写入操作类型
	DWORD opType = static_cast<DWORD>(type);
	dp.Data_WriteSimple(opType);

	// 写入通用参数
	dp.Data_WriteWString(messageId);
	dp.Data_WriteString(contentUtf8);

	// 写入FileEdit相关参数
	dp.Data_WriteWString(fileEditId);
	dp.Data_WriteWString(title);
	dp.Data_WriteWString(fullPath);
	dp.Data_WriteWString(diffContent);

	// 写入ChatSession相关参数
	dp.Data_WriteSimple(checkpointId);

	// 写入压缩相关参数 (v1.4+)
	dp.Data_WriteSimple(currentCompressionLevel);
	DWORD compCount = static_cast<DWORD>(compressedContents.size());
	dp.Data_WriteSimple(compCount);
	for (const auto& pair : compressedContents)
	{
		dp.Data_WriteSimple(pair.first);
		dp.Data_WriteString(pair.second);
	}

}

void ChatOp::Load(CDataPacket& dp,DWORD ver)
{
	// 读取操作类型
	DWORD opType = dp.Data_ReadSimple<DWORD>();
	type = static_cast<Type>(opType);

	// 读取通用参数
	dp.Data_ReadWString(messageId);

	// v1.6+: content 改为 UTF-8 string；旧版本兼容：读取 wstring 后转换
	if (ver >= CHATOPSCTRL_VERSION_1_6)
	{
		dp.Data_ReadString(contentUtf8);
	}
	else
	{
		std::wstring contentW;
		dp.Data_ReadWString(contentW);
		contentUtf8 = widechar_to_utf8(contentW.c_str());
	}

	// 读取FileEdit相关参数
	dp.Data_ReadWString(fileEditId);
	dp.Data_ReadWString(title);
	dp.Data_ReadWString(fullPath);
	dp.Data_ReadWString(diffContent);

	// 读取ChatSession相关参数
	if (ver < CHATOPSCTRL_VERSION_1_3)
	{
		WUID changelistId = dp.Data_ReadSimple<WUID>();
	}
	checkpointId = dp.Data_ReadSimple<FilesCheckpointUID>();

	// 读取压缩相关参数 (v1.4+)
	if (ver >= CHATOPSCTRL_VERSION_1_4)
	{
		currentCompressionLevel = dp.Data_ReadSimple<int>();
		DWORD compCount = dp.Data_ReadSimple<DWORD>();
		for (DWORD i = 0; i < compCount; i++)
		{
			int level = dp.Data_ReadSimple<int>();
			if (ver >= CHATOPSCTRL_VERSION_1_6)
			{
				std::string compContent;
				dp.Data_ReadString(compContent);
				compressedContents[level] = compContent;
			}
			else
			{
				std::wstring compContentW;
				dp.Data_ReadWString(compContentW);
				compressedContents[level] = widechar_to_utf8(compContentW.c_str());
			}
		}
	}
	else
	{
		currentCompressionLevel = 0;
		compressedContents.clear();
	}

// 	if (opType == ChatOp::Op_EndSession)//XXXXXXXXXXXXXXXXXXXXSummarize
// 	{
// 		currentCompressionLevel = 0;
// 		compressedContents.clear();
// 	}
}

void ChatOp::Load(std::ifstream& file,DWORD ver)
{
	// 辅助函数：读取wstring
	auto ReadWString = [&file]() -> std::wstring {
		int len;
		file.read(reinterpret_cast<char*>(&len), sizeof(len));
		if (len <= 0) return L"";

		std::wstring str((len / sizeof(wchar_t)) - 1, L'\0');
		file.read(reinterpret_cast<char*>(&str[0]), len);
		return str;
	};

	// 辅助函数：读取string
	auto ReadString = [&file]() -> std::string {
		int len;
		file.read(reinterpret_cast<char*>(&len), sizeof(len));
		if (len <= 0) return "";

		std::string str(len - 1, '\0');
		file.read(reinterpret_cast<char*>(&str[0]), len);
		return str;
	};

	// 读取操作类型
	DWORD opType;
	file.read(reinterpret_cast<char*>(&opType), sizeof(opType));
	type = static_cast<Type>(opType);

	// 读取通用参数
	messageId = ReadWString();

	// v1.6+: content 改为 UTF-8 string；旧版本兼容：读取 wstring 后转换
	if (ver >= CHATOPSCTRL_VERSION_1_6)
	{
		contentUtf8 = ReadString();
	}
	else
	{
		std::wstring contentW = ReadWString();
		contentUtf8 = widechar_to_utf8(contentW.c_str());
	}

	// 读取FileEdit相关参数
	fileEditId = ReadWString();
	title = ReadWString();
	fullPath = ReadWString();
	diffContent = ReadWString();

	// 读取ChatSession相关参数
	if (ver < CHATOPSCTRL_VERSION_1_3)
	{
		WUID changelistId;
		file.read(reinterpret_cast<char*>(&changelistId), sizeof(changelistId));
	}
	file.read(reinterpret_cast<char*>(&checkpointId), sizeof(checkpointId));

	// 读取压缩相关参数 (v1.4+)
	if (ver >= CHATOPSCTRL_VERSION_1_4)
	{
		file.read(reinterpret_cast<char*>(&currentCompressionLevel), sizeof(currentCompressionLevel));
		DWORD compCount;
		file.read(reinterpret_cast<char*>(&compCount), sizeof(compCount));
		for (DWORD i = 0; i < compCount; i++)
		{
			int level;
			file.read(reinterpret_cast<char*>(&level), sizeof(level));
			if (ver >= CHATOPSCTRL_VERSION_1_6)
			{
				std::string compContent = ReadString();
				compressedContents[level] = compContent;
			}
			else
			{
				std::wstring compContentW = ReadWString();
				compressedContents[level] = widechar_to_utf8(compContentW.c_str());
			}
		}
	}
	else
	{
		currentCompressionLevel = 0;
		compressedContents.clear();
	}
}


// ─────────────────────────────────────────────────────────────────────────────

// 获取 Op 的有效内容（考虑压缩等级）
static std::string _GetEffectiveOpContent(const ChatOp& op)
{
	if (op.currentCompressionLevel > 0)
	{
		auto it = op.compressedContents.find(op.currentCompressionLevel);
		if (it != op.compressedContents.end())
			return it->second;
	}
	return op.contentUtf8;
}


// 构造函数
CChatOpsCtrl::CChatOpsCtrl()
{
	Zero();
	_ver = 0;
}

// 析构函数
CChatOpsCtrl::~CChatOpsCtrl()
{
}

void CChatOpsCtrl::Zero()
{
	_ui = nullptr;
	_undoCheckpointId = FilesCheckpointUID_Invalid;
	_restoredCheckpointId = FilesCheckpointUID_Invalid;
	_recentPrompToken = 0;
	_cliCounter = 0;
	_verEstimateTokens = 0xffffffff;
	_verUncompressedEstimateTokens = 0xffffffff;
	_estimateTokensCache = 0;

}


void CChatOpsCtrl::Init(const ChatAgentContext& ctx)
{
	_ctx = ctx;
}

void CChatOpsCtrl::Clear()
{
	DetachUI();

	ClearChat();

	_ctx = ChatAgentContext();

	Zero();
}

void CChatOpsCtrl::AttachUI(IChatUi* ui)
{
	DetachUI();

	_ui = ui;

	// 显示加载遮罩层
	ShowLoadingOverlay();

	std::vector<ChatOp> ops = std::move(_ops);
	ClearChat();

	DWORD opCount = ops.size();

	// 读取每个操作记录
	for (DWORD i = 0; i < opCount; ++i)
	{
		_ExecuteOp(ops[i]); 
	}

	// 隐藏加载遮罩层
	HideLoadingOverlay();
}

void CChatOpsCtrl::DetachUI()
{
	if (_ui)
	{
		// 发送清空聊天的消息
		std::wstring jsonMessage = L"{\"action\":\"clearChat\"}";
		_ui->PostJsonMessage(jsonMessage);
	}
	_ui = nullptr;
}



// 生成唯一消息ID
std::wstring CChatOpsCtrl::_GenMsgId()
{
	WUID wuid = GenWUID();
	return L"msg_" + std::to_wstring(wuid);
}

std::wstring CChatOpsCtrl::_GenCliId()
{
	// 使用时间戳 + 计数器确保唯一性
	__int64 timestamp = GetAbsTick();
	int counter = _cliCounter++;
	return L"cli-" + std::to_wstring(timestamp) + L"-" + std::to_wstring(counter);
}


// 添加用户消息
void CChatOpsCtrl::AddUserMessage(const std::string& utf8Message, const std::wstring& overrideMessageId)
{
	// 生成或使用指定的消息ID
	std::wstring messageId;
	if (overrideMessageId.empty())
	{
		messageId = _GenMsgId();
	}
	else
	{
		messageId = overrideMessageId;
	}

	// 尝试解析JSON判断是否为full content
	bool isFullContent = false;
	try
	{
		nlohmann::json parsed = nlohmann::json::parse(utf8Message);
		if (parsed.is_array())
		{
			isFullContent = true;
		}
	}
	catch (const nlohmann::json::exception&)
	{
		// 解析失败，不是JSON，按纯文本处理
		isFullContent = false;
	}

	if (_ui)
	{
		std::wstring message = utf8_to_widechar(utf8Message);
		// 构造JSON消息
		std::wstring jsonMessage;
		if (isFullContent)
		{
			// 完整内容，直接传递JSON数组
			jsonMessage = L"{\"action\":\"addUserMessage\",\"content\":" + message + L",\"id\":\"" + messageId + L"\",\"isFullContent\":true}";
		}
		else
		{
			// 纯文本，转义后传递
			std::wstring safeMessage = EscapeJsonString(message);
			jsonMessage = L"{\"action\":\"addUserMessage\",\"content\":\"" + safeMessage + L"\",\"id\":\"" + messageId + L"\",\"isFullContent\":false}";
		}

		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_AddUserMessage);
		op.contentUtf8 = utf8Message;
		op.messageId = messageId;
		_AddOp(op);
	}
}

// 获取用户消息内容
bool CChatOpsCtrl::GetUserMessageContent(const std::wstring& messageId, std::wstring& content) const
{
	content.clear();

	// 遍历操作记录，查找指定的用户消息
	for (const auto& op : _ops)
	{
		if (op.type == ChatOp::Op_AddUserMessage && op.messageId == messageId)
		{
			content = utf8_to_widechar(op.contentUtf8);
			return true;
		}
	}

	return false;
}

// 开始新的AI流式消息
std::wstring CChatOpsCtrl::StartStreamingAIMessage(const std::wstring& overrideMessageId)
{

	// 生成或使用指定的消息ID
	std::wstring messageId;
	if (overrideMessageId.empty())
	{
		if (!_currentStreamingMessageId.empty())
			return _currentStreamingMessageId;
		messageId = _GenMsgId();
	}
	else
	{
		messageId = overrideMessageId;
	}

	_currentStreamingMessageId = messageId;

	if (_ui)
	{
		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"startAIMessage\",\"id\":\"" + messageId + L"\"}";

		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_StartStreamingAIMessage);
		op.messageId = messageId;
		_AddOp(op);
	}

	return messageId;
}


// 添加AI流式消息增量内容
void CChatOpsCtrl::AddStreamingAIMessage(const std::wstring& messageId, const std::string& incrementalContentUtf8)
{
	HideFileEditProgressLabel(messageId);

	if (_ui)
	{
		// 转义增量内容中的特殊字符，避免破坏JSON格式
		std::wstring safeContent = EscapeJsonString(utf8_to_widechar(incrementalContentUtf8));

		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"addToAIMessage\",\"content\":\"" + safeContent +
			L"\",\"id\":\"" + messageId + L"\",\"isComplete\":false}";

		// 发送消息到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	while (true)
	{
		ChatOp* lastOp = _GetLastOp();
		if (lastOp->type == ChatOp::Op_AddStreamingAIMessage)
		{
			if (lastOp->messageId == messageId)
			{
				size_t oldChunk = lastOp->contentUtf8.size() / 256;
				lastOp->contentUtf8 += incrementalContentUtf8;
				size_t newChunk = lastOp->contentUtf8.size() / 256;
				if (newChunk != oldChunk)
					_ver++;
				break;
			}
		}

		ChatOp op(ChatOp::Op_AddStreamingAIMessage);
		op.messageId = messageId;
		op.contentUtf8 = incrementalContentUtf8;
		_AddOp(op);
		break;
	}
}

// 添加AI流式thinking消息增量内容
void CChatOpsCtrl::AddStreamingAIMessage_Thinking(const std::wstring& messageId, const std::string& incrementalContentUtf8)
{

	HideFileEditProgressLabel(messageId);

	if (_ui)
	{
		// 转义增量内容中的特殊字符，避免破坏JSON格式
		std::wstring safeContent = EscapeJsonString(utf8_to_widechar(incrementalContentUtf8));

		// 构造JSON消息，使用新的action类型
		std::wstring jsonMessage = L"{\"action\":\"addToAIMessage_Thinking\",\"content\":\"" + safeContent +
			L"\",\"id\":\"" + messageId + L"\",\"isComplete\":false}";

		// 发送消息到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	while (true)
	{
		ChatOp* lastOp = _GetLastOp();
		if (lastOp->type == ChatOp::Op_AddStreamingAIMessage_Thinking)
		{
			if (lastOp->messageId == messageId)
			{
				size_t oldChunk = lastOp->contentUtf8.size() / 256;
				lastOp->contentUtf8 += incrementalContentUtf8;
				size_t newChunk = lastOp->contentUtf8.size() / 256;
				if (newChunk != oldChunk)
					_ver++;
				break;
			}
		}

		ChatOp op(ChatOp::Op_AddStreamingAIMessage_Thinking);
		op.messageId = messageId;
		op.contentUtf8 = incrementalContentUtf8;
		_AddOp(op);
		break;
	}
}

// 完成流式AI消息
void CChatOpsCtrl::CompleteStreamingAIMessage(const std::wstring& messageId)
{
	HideFileEditProgressLabel(messageId);

	// 当前流式消息ID清空
	if (_currentStreamingMessageId == messageId)
		_currentStreamingMessageId.clear();

	if (_ui)
	{
		// 构造JSON消息，标记为完成，触发Markdown渲染
		std::wstring jsonMessage = L"{\"action\":\"addToAIMessage\",\"content\":\"\",\"id\":\"" +
			messageId + L"\",\"isComplete\":true}";

		// 发送消息到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_CompleteStreamingAIMessage);
		op.messageId = messageId;
		_AddOp(op);
	}
}

// 添加系统消息
void CChatOpsCtrl::AddSystemMessage(const std::string& message, const std::wstring& overrideMessageId)
{

	// 生成或使用指定的消息ID
	std::wstring messageId;
	if (overrideMessageId.empty())
	{
		messageId = _GenMsgId();
	}
	else
	{
		messageId = overrideMessageId;
	}

	if (_ui)
	{
		// 转义消息内容以防XSS
		std::wstring safeMessage = EscapeJsonString(utf8_to_widechar(message));

		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"addSystemMessage\",\"content\":\"" + safeMessage +
			L"\",\"id\":\"" + messageId + L"\"}";

		// 发送消息到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_AddSystemMessage);
		op.contentUtf8 = message;
		op.messageId = messageId;
		_AddOp(op);
	}
}

// 清空聊天记录
void CChatOpsCtrl::ClearChat()
{
	_currentStreamingMessageId.clear();

	// 清空FileEdit窗口数据
	_fileEdits.clear();

	// 清空操作记录
	_ops.clear();

	_recentPrompToken = 0;

	if (_ui)
	{
		// 发送清空聊天的消息
		std::wstring jsonMessage = L"{\"action\":\"clearChat\"}";
		_ui->PostJsonMessage(jsonMessage);
	}

	_ver++;
}

// Disable某个消息之后的所有消息
void CChatOpsCtrl::DisableMessagesAfter(const std::wstring& messageId)
{
	if (_ui)
	{
		// 转义消息ID
		std::wstring safeMessageId = EscapeJsonString(messageId);

		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"disableMessagesAfter\",\"messageId\":\"" + safeMessageId + L"\"}";

		// 发送到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	ChatOp op(ChatOp::Op_DisableMessagesAfter);
	op.messageId = messageId;
	_AddOp(op);
}

// 启用所有被disabled的消息
void CChatOpsCtrl::EnableAllDisabledMessages()
{

	if (_ui)
	{
		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"enableAllDisabledMessages\"}";

		// 发送到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 清除之前所有的DisableMessagesAfter操作记录
	auto it = std::remove_if(_ops.begin(), _ops.end(), [](const ChatOp& op) {
		return op.type == ChatOp::Op_DisableMessagesAfter;
	});
	if (it != _ops.end()) {
		_ops.erase(it, _ops.end());
		_ver++; // 操作记录已更改，增加版本号
	}
}

// 删除所有disabled的session，并返回需要丢弃的checkpoint
void CChatOpsCtrl::RemoveDisabledSessions(std::vector<FilesCheckpointUID>& checkpointsToDiscard)
{
	checkpointsToDiscard.clear();

	// 获取 disable 边界
	int disableAfterIndex = _GetDisableAfterIndex();

	// 如果没有被disabled的消息，直接返回
	if (disableAfterIndex == static_cast<int>(_ops.size()))
		return;

	// 找到被disabled消息所在session的开始位置
	int sessionStartIndex = disableAfterIndex;

	// 向前查找最近的BeginSession操作
	for (int i = disableAfterIndex - 1; i >= 0; i--)
	{
		if (_ops[i].type == ChatOp::Op_BeginSession)
		{
			sessionStartIndex = i;
			break;
		}
		// 如果遇到另一个EndSession，说明前面是另一个完整的session，停止查找
		if (_ops[i].type == ChatOp::Op_EndSession)
		{
			break;
		}
	}

	// 收集从session开始到队列末尾的所有session中涉及的checkpoint
	std::unordered_set<FilesCheckpointUID> checkpointSet;

	for (int i = sessionStartIndex; i < static_cast<int>(_ops.size()); i++)
	{
		const ChatOp& op = _ops[i];
		if (op.checkpointId != FilesCheckpointUID_Invalid)
			checkpointSet.insert(op.checkpointId);
	}

	// 将checkpoint集合转换为vector
	checkpointsToDiscard.assign(checkpointSet.begin(), checkpointSet.end());
	if (_undoCheckpointId != FilesCheckpointUID_Invalid)
		checkpointsToDiscard.push_back(_undoCheckpointId);
	if (_restoredCheckpointId != FilesCheckpointUID_Invalid)
		checkpointsToDiscard.push_back(_restoredCheckpointId);

	_undoCheckpointId = FilesCheckpointUID_Invalid;
	_restoredCheckpointId = FilesCheckpointUID_Invalid;

	// 从操作队列中删除从session开始到队列末尾的所有操作
	if (sessionStartIndex < static_cast<int>(_ops.size()))
	{
		_ops.erase(_ops.begin() + sessionStartIndex, _ops.end());
		_ver++; // 操作记录已更改，增加版本号
	}

	if (_ui)
	{
		// 从UI中删除所有disabled的消息元素
		std::wstring jsonMessage = L"{\"action\":\"removeDisabledMessages\"}";
		_ui->PostJsonMessage(jsonMessage);
	}
}


//====================== FileEdit 内嵌窗口功能实现 ======================

// 在指定AI消息中添加FileEdit窗口
std::wstring CChatOpsCtrl::AddFileEditToAIMessage(const std::wstring& messageId, const std::wstring& title, const std::wstring& fullPath, const std::wstring& overrideFileEditId)
{
	// 隐藏之前可能显示的文件编辑进度标签
	HideFileEditProgressLabel(messageId);

	// 生成或使用指定的FileEdit窗口ID
	// 如果提供了 overrideFileEditId，使用该ID；否则生成新的唯一ID
	std::wstring fileEditId;
	if (overrideFileEditId.empty())
	{
		fileEditId = _GenFileEditId();  // 调用生成函数创建唯一ID: "fileedit_" + wuid
	}
	else
	{
		fileEditId = overrideFileEditId;  // 使用调用者指定的ID
	}

	// 创建FileEdit窗口数据结构，用于内部存储
	FileEdit window;
	window.id = fileEditId;                              // 窗口唯一ID
	window.title = title;                                // 窗口标题（通常是文件名）
	window.fullPath = fullPath;                          // 完整文件路径
	window.messageId = messageId;                        // 关联的AI消息ID
	window.isCollapsed = true;                           // 初始状态：折叠

	// 保存FileEdit窗口到内部存储容器，用于后续查找和更新
	_fileEdits.push_back(window);

	// 发送创建FileEdit窗口的消息到WebView/UI前端
	// "addFileEdit" 动作通知前端创建新的编辑窗口
	_SendFileEditMsg(L"addFileEdit", window);

	// 记录此操作到操作历史队列
	// 便于后续还原、保存和重放（如加载历史会话时）
	if (true)
	{
		ChatOp op(ChatOp::Op_AddFileEditToAIMessage);  // 创建操作记录对象
		op.messageId = messageId;                       // 记录关联的消息ID
		op.fileEditId = fileEditId;                     // 记录窗口ID
		op.title = title;                               // 记录文件标题
		op.fullPath = fullPath;                         // 记录文件路径
		_AddOp(op);                                     // 将操作添加到历史队列
	}

	// 返回生成的或使用的FileEdit窗口ID，以便调用者后续操作此窗口
	return fileEditId;
}

// 设置FileEdit窗口标题
void CChatOpsCtrl::SetFileEditTitle(const std::wstring& fileEditId, const std::wstring& title)
{

	FileEdit* window = _FindFileEdit(fileEditId);
	if (window != nullptr)
	{
		window->title = title;
		_SendFileEditMsg(L"updateFileEditTitle", *window);
	}

	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_SetFileEditTitle);
		op.fileEditId = fileEditId;
		op.title = title;
		_AddOp(op);
	}

}

// 设置FileEdit窗口显示内容
void CChatOpsCtrl::SetFileEditContent(const std::wstring& fileEditId, const std::string& content, const std::wstring& diffContent, FilesCheckpointUID checkpointID)
{

	FileEdit* window = _FindFileEdit(fileEditId);
	if (window != nullptr)
	{
		window->content = utf8_to_widechar(content);
		window->diffContent = diffContent;
		_SendFileEditMsg(L"updateFileEditContent", *window);
	}

	// 从操作队列末尾开始查找最近的同fileEditId的SetFileEditContent操作
	ChatOp* targetOp = nullptr;
	for (auto it = _ops.rbegin(); it != _ops.rend(); ++it)
	{
		if (it->type == ChatOp::Op_SetFileEditContent && it->fileEditId == fileEditId)
		{
			targetOp = &(*it);
			break;
		}
	}

	if (targetOp != nullptr)
	{
		// 合并到找到的操作
		targetOp->contentUtf8 = content;
		targetOp->diffContent = diffContent;
		targetOp->checkpointId = checkpointID;
		_ver++; // 操作记录已更改，增加版本号
	}
	else
	{
		// 没有找到，新建操作
		ChatOp op(ChatOp::Op_SetFileEditContent);
		op.fileEditId = fileEditId;
		op.contentUtf8 = content;
		op.checkpointId = checkpointID;
		op.diffContent = diffContent;
		_AddOp(op);
	}
}

// 添加FileEdit窗口标题栏按钮
std::wstring CChatOpsCtrl::AddFileEditButton(const std::wstring& fileEditId, const std::wstring& buttonText, const std::wstring& buttonAction, const std::wstring& overrideButtonId)
{

	FileEdit* window = _FindFileEdit(fileEditId);
	if (window != nullptr)
	{
		// 生成或使用指定的按钮ID
		std::wstring buttonId;
		if (overrideButtonId.empty())
		{
			buttonId = _GenFileEditBtnId();
		}
		else
		{
			buttonId = overrideButtonId;
		}

		// 创建按钮数据
		FileEditBtn button;
		button.id = buttonId;
		button.text = buttonText;
		button.action = buttonAction;

		// 添加按钮到窗口
		window->buttons.push_back(button);

		// 发送更新按钮的消息
		_SendFileEditMsg(L"updateFileEditButtons", *window);

		return buttonId;
	}

	return L"";
}

// 折叠/展开FileEdit窗口
void CChatOpsCtrl::ToggleFileEditCollapse(const std::wstring& fileEditId)
{
	FileEdit* window = _FindFileEdit(fileEditId);
	if (window != nullptr)
	{
		window->isCollapsed = !window->isCollapsed;
		_SendFileEditMsg(L"toggleFileEditCollapse", *window);
	}
}

// 开始FileEdit修改状态动画
void CChatOpsCtrl::StartFileEditModification(const std::wstring& fileEditId)
{
	if (_ui)
	{
		// 转义fileEditId
		std::wstring safeFileEditId = EscapeJsonString(fileEditId);

		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"startFileEditModification\",\"fileEditId\":\"" + safeFileEditId + L"\"}";

		// 发送到WebView
		_ui->PostJsonMessage(jsonMessage);
	}
}

// 停止FileEdit修改状态动画
void CChatOpsCtrl::StopFileEditModification(const std::wstring& fileEditId)
{
	if (_ui)
	{
		// 转义fileEditId
		std::wstring safeFileEditId = EscapeJsonString(fileEditId);

		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"stopFileEditModification\",\"fileEditId\":\"" + safeFileEditId + L"\"}";

		// 发送到WebView
		_ui->PostJsonMessage(jsonMessage);
	}
}

//====================== FileSummarize 功能实现 ======================

// 在指定AI消息中添加FileSummarize按钮
void CChatOpsCtrl::AddFileSummarizeToAIMessage(const std::wstring& messageId, const std::wstring& filePath)
{
	if (_ui)
	{
		// 转义参数
		std::wstring safeMessageId = EscapeJsonString(messageId);
		std::wstring safeFilePath = EscapeJsonString(filePath);

		// 构造JSON消息，使用 PostWebMessageAsJson 发送
		std::wstring jsonMessage = L"{\"action\":\"addFileSummarize\",\"messageId\":\"" + safeMessageId + L"\",\"filePath\":\"" + safeFilePath + L"\"}";
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	ChatOp op(ChatOp::Op_AddFileSummarizeToAIMessage);
	op.messageId = messageId;
	op.fullPath = filePath;
	_AddOp(op);
}

// 查找是否存在和这个fileEditId的messageId一致的Summarize
bool CChatOpsCtrl::ExistSummarizeInSession(const std::wstring& fileEditId) const
{
	// 查找fileEditId对应的操作
	int fileEditIndex = _FindFileEditOpIndex(fileEditId);
	if (fileEditIndex < 0)
		return false;

	// 获取该fileEdit的messageId
	std::wstring messageId = _ops[fileEditIndex].messageId;

	// 查找是否存在相同messageId的Op_AddFileSummarizeToAIMessage操作
	for (int i = 0; i < static_cast<int>(_ops.size()); i++)
	{
		const ChatOp& op = _ops[i];
		if (op.type == ChatOp::Op_AddFileSummarizeToAIMessage && op.messageId == messageId)
		{
			return true;
		}
	}

	return false;
}

//====================== FileEdit 私有辅助方法实现 ======================

// 生成唯一FileEdit窗口ID
std::wstring CChatOpsCtrl::_GenFileEditId()
{
	WUID wuid = GenWUID();
	return L"fileedit_" + std::to_wstring(wuid);
}

// 生成唯一按钮ID
std::wstring CChatOpsCtrl::_GenFileEditBtnId()
{
	WUID wuid = GenWUID();
	return L"btn_" + std::to_wstring(wuid);
}

// 查找FileEdit窗口
CChatOpsCtrl::FileEdit* CChatOpsCtrl::_FindFileEdit(const std::wstring& fileEditId)
{
	auto it = std::find_if(_fileEdits.begin(), _fileEdits.end(),
		[&fileEditId](const FileEdit& window) { return window.id == fileEditId; });

	return (it != _fileEdits.end()) ? &(*it) : nullptr;
}

// 发送FileEdit相关消息到WebView
void CChatOpsCtrl::_SendFileEditMsg(const std::wstring& action, const FileEdit& window)
{
	if (!_ui)
		return;
	// 构建按钮JSON数组
	std::wstring buttonsJson = _BuildButtonsJson(window.buttons);

	// 转义所有字符串内容
	std::wstring safeTitle = EscapeJsonString(window.title);
	std::wstring safeMessageId = EscapeJsonString(window.messageId);
	std::wstring safeFileEditId = EscapeJsonString(window.id);
	std::wstring safeFullPath = EscapeJsonString(window.fullPath);

	// 同时发送原始内容和diff内容，让JavaScript端决定显示哪个
	std::wstring safeContent = EscapeJsonString(window.content);
	std::wstring safeDiffContent = EscapeJsonString(window.diffContent);

	// 构造JSON消息
	std::wstring jsonMessage = L"{";
	jsonMessage += L"\"action\":\"" + action + L"\",";
	jsonMessage += L"\"fileEditId\":\"" + safeFileEditId + L"\",";
	jsonMessage += L"\"messageId\":\"" + safeMessageId + L"\",";
	jsonMessage += L"\"title\":\"" + safeTitle + L"\",";
	jsonMessage += L"\"fullPath\":\"" + safeFullPath + L"\",";
	jsonMessage += L"\"content\":\"" + safeContent + L"\",";
	jsonMessage += L"\"diffContent\":\"" + safeDiffContent + L"\",";
	jsonMessage += L"\"isCollapsed\":";
	jsonMessage += window.isCollapsed ? L"true" : L"false";
	jsonMessage += L",";
	jsonMessage += L"\"buttons\":" + buttonsJson;
	jsonMessage += L"}";

	// 发送到WebView
	_ui->PostJsonMessage(jsonMessage);
}

// 构建FileEdit按钮的JSON数组
std::wstring CChatOpsCtrl::_BuildButtonsJson(const std::vector<FileEditBtn>& buttons)
{
	std::wstring json = L"[";

	for (size_t i = 0; i < buttons.size(); ++i)
	{
		if (i > 0) json += L",";

		json += L"{";
		json += L"\"id\":\"" + EscapeJsonString(buttons[i].id) + L"\",";
		json += L"\"text\":\"" + EscapeJsonString(buttons[i].text) + L"\",";
		json += L"\"action\":\"" + EscapeJsonString(buttons[i].action) + L"\"";
		json += L"}";
	}

	json += L"]";
	return json;
}

//====================== WebView 标题栏功能实现 ======================

// 设置WebView标题栏标题
void CChatOpsCtrl::SetTitle(const std::wstring& title)
{
	if (_ui)
	{
		// 转义标题内容
		std::wstring safeTitle = EscapeJsonString(title);

		// 构造JSON消息
		std::wstring jsonMessage = L"{\"action\":\"setWebViewTitle\",\"title\":\"" + safeTitle + L"\"}";

		// 发送到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_SetTitle);
		op.title = title;
		_AddOp(op);
	}
}

bool CChatOpsCtrl::HasTitle() const
{
	const ChatOp* op = _FindLastOp(ChatOp::Op_SetTitle);
	if (!op)
		return false;
	if (op->title == DEFAULT_CHAT_TITLE)
		return false;
	if (op->title.empty())
		return false;
	return  true;
}

const wchar_t* CChatOpsCtrl::GetTitle() const
{
	const ChatOp* op = _FindLastOp(ChatOp::Op_SetTitle);
	if (op)
		return op->title.c_str();
	return L"";
}


//====================== 操作记录和还原功能实现 ======================

// 添加操作到记录队列
void CChatOpsCtrl::_AddOp(const ChatOp& op)
{
	_ver++;

	// 检查是否可以与最后一个操作合并
	if (!_ops.empty() && (op.type == ChatOp::Op_AddStreamingAIMessage || op.type == ChatOp::Op_AddStreamingAIMessage_Thinking))
	{
		ChatOp& lastOp = _ops.back();

		// 如果最后一个操作也是相同类型的流式消息且messageId相同，则合并内容
		if (lastOp.type == op.type && lastOp.messageId == op.messageId)
		{
			lastOp.contentUtf8 += op.contentUtf8;
			return; // 合并完成，不添加新操作
		}
	}

	//Op_SetTitle,合并到已存在的Op
	if (op.type == ChatOp::Op_SetTitle)
	{
		ChatOp* existOp = (ChatOp*)_FindLastOp(ChatOp::Op_SetTitle);
		if (existOp)
		{
			existOp->title = op.title;
			return;
		}
	}

	// 没有合并，添加新操作
	_ops.push_back(op);
}

void CChatOpsCtrl::AddSessionCost(const LlmSessionUsage& usage, const std::wstring& messageId)
{
	// 使用新的格式化方法生成显示文本
	std::string displayText = usage.FormatToCostText();
	_SetSessionCostDisplay(displayText, messageId);

	// 记录操作，将费用信息存储在content字段中
	ChatOp op(ChatOp::Op_SetSessionCost);
	op.messageId = messageId;
	op.contentUtf8 = displayText;
	_AddOp(op);
}

void CChatOpsCtrl::NotifyPromptCache(const LlmSessionUsage& usage)
{
	_recentPrompToken = usage.inputToken_;
}


// 内部函数：设置会话费用显示（不记录操作，供_ExecuteOp调用）
void CChatOpsCtrl::_SetSessionCostDisplay(const std::string& costText, const std::wstring& messageId)
{
	if (!_ui)
		return;

	// 转义显示文本以防止JSON格式错误
	std::wstring safeDisplayText = EscapeJsonString(utf8_to_widechar(costText));

	// 构造JSON消息，发送费用信息给WebView
	std::wstring jsonMessage = L"{\"action\":\"setCostDisplay\",\"costText\":\"" + safeDisplayText + L"\"";
	if (!messageId.empty())
	{
		std::wstring safeMessageId = EscapeJsonString(messageId);
		jsonMessage += L",\"messageId\":\"" + safeMessageId + L"\"";
	}
	jsonMessage += L"}";

	_ui->PostJsonMessage(jsonMessage);
}


// 从操作记录还原内容
void CChatOpsCtrl::_ExecuteOp(const ChatOp& op)
{
	int newOpIndex = _ops.size();
	switch (op.type)
	{
	case ChatOp::Op_AddUserMessage:
		AddUserMessage(op.contentUtf8, op.messageId);
		break;
	case ChatOp::Op_AddSessionTag:
		_AddSessionTag(op.contentUtf8, op.fullPath, true);
		break;
	case ChatOp::Op_AddSessionDisabledTag:
		_AddSessionTag(op.contentUtf8, op.fullPath, false);
		break;
	case ChatOp::Op_StartStreamingAIMessage:
		StartStreamingAIMessage(op.messageId);
		break;
	case ChatOp::Op_AddStreamingAIMessage:
		AddStreamingAIMessage(op.messageId, op.contentUtf8);
		break;
	case ChatOp::Op_AddStreamingAIMessage_Thinking:
		AddStreamingAIMessage_Thinking(op.messageId, op.contentUtf8);
		break;

	case ChatOp::Op_CompleteStreamingAIMessage:
		CompleteStreamingAIMessage(op.messageId);
		break;

	case ChatOp::Op_AddSystemMessage:
		AddSystemMessage(op.contentUtf8, op.messageId);
		break;

	case ChatOp::Op_AddFileEditToAIMessage:
		AddFileEditToAIMessage(op.messageId, op.title, op.fullPath, op.fileEditId);
		break;

	case ChatOp::Op_SetFileEditTitle:
		SetFileEditTitle(op.fileEditId, op.title);
		break;

	case ChatOp::Op_SetFileEditContent:
		SetFileEditContent(op.fileEditId, op.contentUtf8, op.diffContent, op.checkpointId);
		break;

	case ChatOp::Op_SetTitle:
		SetTitle(op.title);
		break;

	case ChatOp::Op_BeginSession:
		BeginSession(op.checkpointId);
		break;

	case ChatOp::Op_EndSession:
		EndSession();
		break;

	case ChatOp::Op_DisableMessagesAfter:
		DisableMessagesAfter(op.messageId);
		break;

	case ChatOp::Op_SetSessionCost:
	{
		LlmSessionUsage usage = LlmSessionUsage::ParseFromCostText(op.contentUtf8);
		AddSessionCost(usage, op.messageId);
		break;
	}

	case ChatOp::Op_FileAttaches:
	{
		AddFileAttaches(op.contentUtf8, op.checkpointId);
		break;
	}

	case ChatOp::Op_AddToolCallResult:
	{
		AddToolCallResult(op.contentUtf8);
		break;
	}

	case ChatOp::Op_AddToolCallMessage_Exploring:
	{
		AddToolCallMessage_Exploring(op.messageId, op.contentUtf8);
		break;
	}

	case ChatOp::Op_AddFileSummarizeToAIMessage:
	{
		AddFileSummarizeToAIMessage(op.messageId, op.fullPath);
		break;
	}

	case ChatOp::Op_AddUserInterject:
	{
		AddUserInterject(op.messageId, op.contentUtf8);
		break;
	}

	case ChatOp::Op_CliDisplay:
	{
		// 从 content 中解析出 command、output 和 shellType（兼容旧格式和新格式）
		std::string command, output, shellType;
		_ParseCliDisplayContent(op.contentUtf8, command, output, shellType);
		
		// 调用方法显示，传递 shellType（重放历史记录时使用 None 状态）
		AddCliDisplay(op.messageId, command, op.title, CliDisplayStatus::None, shellType);
		
		// 如果有 output，再追加
		if (!output.empty())
		{
			AppendOutputToLastCliDisplay(op.messageId, output);
		}
		break;
	}

	case ChatOp::Op_QuestionDisplay:
	{
		// 从 title 字段获取 question，从 content 字段获取 answer
		AddQuestionDisplay(op.messageId, op.title, op.contentUtf8);
		break;
	}

	default:
		// 未知操作类型，跳过
		break;
	}

	if (newOpIndex + 1 == _ops.size())
	{
		if (_ops[newOpIndex].type == op.type)
		{
			_ops[newOpIndex].compressedContents = op.compressedContents;
			_ops[newOpIndex].currentCompressionLevel = op.currentCompressionLevel;
		}
	}
}

void CChatOpsCtrl::BeginSession(FilesCheckpointUID checkpointId)
{
	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_BeginSession);
		op.checkpointId = checkpointId;
		_AddOp(op);
	}
}


void CChatOpsCtrl::SetUndoCheckpoint(FilesCheckpointUID checkpointId)
{
	CCheckpoints* pCheckpoints = _ctx.checkpoints;
	if (!pCheckpoints)
		return;

	_undoCheckpointId = checkpointId;

	pCheckpoints->DiscardCheckpoint(_restoredCheckpointId);
	_restoredCheckpointId = FilesCheckpointUID_Invalid;

	// 根据undo checkpoint里的文件列表创建一个新的checkpoint记录当前状态
	if (checkpointId != FilesCheckpointUID_Invalid)
	{
		// 获取undo checkpoint中的文件列表
		std::vector<const char*> fileList;
		if (pCheckpoints->GetCheckpointFileList(checkpointId, fileList))
		{
			// 将const char*转换为std::string以便传递给CreateCheckpointFromFilelist
			std::vector<const char*> filePathList;
			std::vector<std::string> filePathStrings;

			for (const char* filePath : fileList)
			{
				if (filePath && filePath[0] != '\0')
				{
					filePathStrings.push_back(std::string(filePath));
				}
			}

			// 重新构建const char*列表
			for (const auto& pathStr : filePathStrings)
			{
				filePathList.push_back(pathStr.c_str());
			}

			// 创建新的checkpoint记录这些文件的当前状态
			if (!filePathList.empty())
			{
				_restoredCheckpointId = pCheckpoints->CreateCheckpointFromFilelist(filePathList);
			}
		}
	}

	_ver++;
}

void CChatOpsCtrl::EndSession()
{

	// 记录操作
	if (true)
	{
		ChatOp op(ChatOp::Op_EndSession);
		_AddOp(op);
	}
}

void CChatOpsCtrl::AccumulateSessionCostForFileEdit(const std::wstring& fileEditId, float price, int inputToken, int outputToken)
{
	int sessionBegin = _GetSessionBeginOfFileEdit(fileEditId);
	if (sessionBegin == -1)
		return;

	int firstOpIndex = _FindFirstOpIndexInSession(sessionBegin, ChatOp::Op_SetSessionCost);
	if (firstOpIndex == -1)
		return;

	ChatOp* existingCostOp = &_ops[firstOpIndex];

	std::string newDisplayText;

	if (existingCostOp != nullptr)
	{
		// 已存在费用操作，需要累加
		// 解析现有的费用信息
		LlmSessionUsage existingUsage = LlmSessionUsage::ParseFromCostText(existingCostOp->contentUtf8);

		// 累加费用
		LlmSessionUsage totalUsage;
		totalUsage.Accumulate(existingUsage);

		// 格式化新的显示文本
		newDisplayText = totalUsage.FormatToCostText();

		// 更新现有操作的内容
		existingCostOp->contentUtf8 = newDisplayText;
		_ver++; // 操作记录已更改，增加版本号

		_SetSessionCostDisplay(newDisplayText);
	}
}


void CChatOpsCtrl::_AddSessionTag(const std::string& text, const std::wstring& path, bool enabled)
{
	ChatOp op(enabled ? ChatOp::Op_AddSessionTag : ChatOp::Op_AddSessionDisabledTag);
	op.contentUtf8 = text;
	op.fullPath = path;
	_AddOp(op);
}

void CChatOpsCtrl::AddSessionTag(const ChatInputTag& tag)
{
	// 记录操作
	_AddSessionTag(widechar_to_utf8(tag.text.c_str()), tag.path, tag.visible);
}

//====================== 操作记录文件保存和加载功能实现 ======================


// 保存操作记录到文件
/***重要: 修改存储格式, 要同步CChatHistory::_Entry2MenuItemInfo()****/
bool CChatOpsCtrl::Save(const char* filePath)
{
	if (!filePath) return false;

	try
	{
		std::vector<BYTE> buffer;

		// 使用DP_BeginSave/DP_EndSave宏来处理DataPacket
		DP_BeginSave(dp, buffer)
		{
			// 写入文件头标识
			DWORD magic = 0x4F504348; // "HCPO" (HChat Ops)
			dp.Data_WriteSimple(magic);

			// 写入版本号
			DWORD version = CHATOPSCTRL_VERSION_CURRENT;
			dp.Data_WriteSimple(version);

			dp.Data_WriteSimple(_undoCheckpointId);
			dp.Data_WriteSimple(_restoredCheckpointId);
			dp.Data_WriteSimple(_recentPrompToken);

			// 写入操作记录数量
			DWORD opCount = static_cast<DWORD>(_ops.size());
			dp.Data_WriteSimple(opCount);

			// 写入每个操作记录
			for (const auto& op : _ops)
			{
				const_cast<ChatOp&>(op).Save(dp);
			}
		}
		DP_EndSave()

		// 使用STL的ofstream写入文件
		std::ofstream file;
		Utils::OpenOFStream(file, filePath);
		if (!file.is_open())
			return false;

		if (!buffer.empty())
		{
			file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		}
		file.close();

		return true;
	}
	catch (...)
	{
		return false;
	}
}

// 从文件加载操作记录
bool CChatOpsCtrl::Load(const char* filePath)
{
	if (!filePath)
		return false;
	if (!filePath[0])
		return false;

	try
	{
		// 使用STL的ifstream读取文件
		std::ifstream file;
		Utils::OpenIFStream(file, filePath);
		if (!file.is_open())
			return false;

		// 读取文件大小
		file.seekg(0, std::ios::end);
		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		if (fileSize == 0)
		{
			file.close();
			return false;
		}

		// 读取文件内容到缓冲区
		std::vector<BYTE> buffer(fileSize);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		// 创建DataPacket并设置数据缓冲区
		CDataPacket dp;
		dp.SetDataBufferPointer(buffer.data());

		// 读取文件头标识
		DWORD magic = dp.Data_ReadSimple<DWORD>();
		if (magic != 0x4F504348) // "HCPO"
		{
			return false; // 不是有效的操作记录文件
		}

		// 读取版本号
		DWORD version = dp.Data_ReadSimple<DWORD>();

		// 检查版本兼容性
		if (version > CHATOPSCTRL_VERSION_CURRENT)
		{
			return false; // 版本过新，无法加载
		}

		// 显示加载遮罩层
		ShowLoadingOverlay();

		// 清空当前内容
		ClearChat();

		// 读取 undo checkpoint
		_undoCheckpointId = dp.Data_ReadSimple<FilesCheckpointUID>();
		if (version >= CHATOPSCTRL_VERSION_1_1)
			_restoredCheckpointId = dp.Data_ReadSimple<FilesCheckpointUID>();
		if (version >= CHATOPSCTRL_VERSION_1_2)
			_recentPrompToken = dp.Data_ReadSimple<int>();

		// 读取操作记录数量
		DWORD opCount = dp.Data_ReadSimple<DWORD>();

		// 读取每个操作记录
		for (DWORD i = 0; i < opCount; ++i)
		{
			ChatOp op;
			op.Load(dp,version);
			_ExecuteOp(op); // 注意：这里不直接调用AddUserMessage等，而是调用_ExecuteOp
		}

		// 隐藏加载遮罩层
		HideLoadingOverlay();

		return true;
	}
	catch (...)
	{
		// 加载失败时隐藏加载遮罩层
		HideLoadingOverlay();
		return false;
	}
}

//====================== Loading Overlay 功能实现 ======================

// 显示加载遮罩层
void CChatOpsCtrl::ShowLoadingOverlay()
{
	if (!_ui)
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"showLoadingOverlay\"}";

	// 发送到WebView
	_ui->PostJsonMessage(jsonMessage);
}

// 隐藏加载遮罩层
void CChatOpsCtrl::HideLoadingOverlay()
{
	if (!_ui)
		return;

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"hideLoadingOverlay\"}";

	// 发送到WebView
	_ui->PostJsonMessage(jsonMessage);
}

//====================== FileEdit Progress Label 功能实现 ======================

// 显示文件编辑进行中的标签
void CChatOpsCtrl::ShowFileEditProgressLabel(const std::wstring& messageId, const std::wstring& fileName, const std::wstring& fullPath)
{
	if (!_ui)
		return;

	// 转义消息ID、文件名和完整路径
	std::wstring safeMessageId = EscapeJsonString(messageId);
	std::wstring safeFileName = EscapeJsonString(fileName);
	std::wstring safeFullPath = EscapeJsonString(fullPath);

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"showFileEditProgressLabel\",\"messageId\":\"" + safeMessageId + L"\",\"fileName\":\"" + safeFileName + L"\",\"fullPath\":\"" + safeFullPath + L"\"}";

	// 发送到WebView
	_ui->PostJsonMessage(jsonMessage);
}

// 隐藏文件编辑进行中的标签
void CChatOpsCtrl::HideFileEditProgressLabel(const std::wstring& messageId)
{
	if (!_ui)
		return;

	// 转义消息ID
	std::wstring safeMessageId = EscapeJsonString(messageId);

	// 构造JSON消息
	std::wstring jsonMessage = L"{\"action\":\"hideFileEditProgressLabel\",\"messageId\":\"" + safeMessageId + L"\"}";

	// 发送到WebView
	_ui->PostJsonMessage(jsonMessage);
}

int CChatOpsCtrl::_FindLastOpIndex(ChatOp::Type tp) const
{
	for (int i = _ops.size() - 1; i >= 0; i--)
	{
		if (_ops[i].type == tp)
			return i;
	}
	return -1;
}

bool CChatOpsCtrl::FindSessionBoundaries(int targetSrcIndex, int& sessionBeginIndex, int& sessionEndIndex) const
{
	sessionBeginIndex = -1;
	sessionEndIndex = -1;

	if (targetSrcIndex < 0 || targetSrcIndex >= (int)_ops.size())
		return false;

	// 向前找 Op_BeginSession
	for (int i = targetSrcIndex; i >= 0; i--)
	{
		if (_ops[i].type == ChatOp::Op_BeginSession)
		{
			sessionBeginIndex = i;
			break;
		}
	}

	// 向后找 Op_EndSession
	for (int i = targetSrcIndex; i < (int)_ops.size(); i++)
	{
		if (_ops[i].type == ChatOp::Op_EndSession)
		{
			sessionEndIndex = i;
			break;
		}
	}

	// 只有当两个边界都找到时才返回 true
	return (sessionBeginIndex >= 0 && sessionEndIndex >= 0);
}

ChatOp* CChatOpsCtrl::_GetLastOp()
{
	if (_ops.size() <= 0)
		return nullptr;
	return &_ops[_ops.size() - 1];
}


// 从操作队列中查找最后一个指定类型的操作
const ChatOp* CChatOpsCtrl::_FindLastOp(ChatOp::Type tp) const
{
	int index = _FindLastOpIndex(tp);
	if (index == -1)
		return nullptr;
	return &_ops[index];
}


int CChatOpsCtrl::_FindFileEditOpIndex(const std::wstring& fileEditId) const
{
	for (int i = _ops.size() - 1; i >= 0; i--)
	{
		if (_ops[i].type == ChatOp::Op_AddFileEditToAIMessage && _ops[i].fileEditId == fileEditId)
			return i;
	}
	return -1;
}

int CChatOpsCtrl::_GetSessionBeginOfOpIndex(int idx) const
{
	if (idx < 0)
		return -1;

	for (int i = idx; i >= 0; i--)
	{
		if (_ops[i].type == ChatOp::Op_BeginSession)
			return i;
	}
	return -1;
}


int CChatOpsCtrl::_GetSessionBeginOfFileEdit(const std::wstring& fileEditId) const
{
	int fileEditOpIndex = _FindFileEditOpIndex(fileEditId);
	return _GetSessionBeginOfOpIndex(fileEditOpIndex);
}

int CChatOpsCtrl::_GetSessionBeginOfUserMessage(const std::wstring& messageId) const
{
	// 首先找到用户消息的索引
	int userMessageIndex = -1;
	for (int i = 0; i < static_cast<int>(_ops.size()); i++)
	{
		if (_ops[i].type == ChatOp::Op_AddUserMessage && _ops[i].messageId == messageId)
		{
			userMessageIndex = i;
			break;
		}
	}

	return _GetSessionBeginOfOpIndex(userMessageIndex);
}

int CChatOpsCtrl::_GetLastNotDisabledSessionBegin() const
{
	int disableAfter = _GetDisableAfterIndex();
	if (disableAfter >= _ops.size())//没有disable
		return _FindLastOpIndex(ChatOp::Op_BeginSession);

	//disableAfter往前找到第二个BeginSession,第一个BeginSession是被disable的那个session的SessionBegin
	int sessionBeginCount = 0;
	for (int i = disableAfter - 1; i >= 0; i--)
	{
		if (_ops[i].type == ChatOp::Op_BeginSession)
		{
			sessionBeginCount++;
			if (sessionBeginCount > 1)
				return i;
		}
	}

	return -1;
}

int CChatOpsCtrl::_GetLastNotDisabledSessionEnd() const
{
	int sessionBegin = _GetLastNotDisabledSessionBegin();
	if (sessionBegin < 0)
		return sessionBegin;
	return _FindFirstOpIndexInSession(sessionBegin, ChatOp::Op_EndSession);
}

// 查找 DisableMessagesAfter 操作的位置，返回被 disable-after 的消息在操作队列中的索引
// 如果没有找到 DisableMessagesAfter 操作，返回操作队列大小（表示没有被 disable 的消息）
int CChatOpsCtrl::_GetDisableAfterIndex() const
{
	// 查找最新的 DisableMessagesAfter 操作
	const ChatOp* disableOp = _FindLastOp(ChatOp::Op_DisableMessagesAfter);

	// 如果没有找到 DisableMessagesAfter 操作，返回操作队列大小（表示没有被 disable 的消息）
	if (disableOp == nullptr)
		return static_cast<int>(_ops.size());

	// 找到被 disable 的消息在操作队列中的位置
	const std::wstring& disableAfterMessageId = disableOp->messageId;
	for (int i = 0; i < static_cast<int>(_ops.size()); i++)
	{
		if ((_ops[i].type == ChatOp::Op_AddUserMessage ||
			_ops[i].type == ChatOp::Op_StartStreamingAIMessage ||
			_ops[i].type == ChatOp::Op_AddSystemMessage) &&
			_ops[i].messageId == disableAfterMessageId)
		{
			return i;
		}
	}

	// 如果没有找到被 disable 的消息位置，返回操作队列大小
	return static_cast<int>(_ops.size());
}

// 查找最近 N 个未 disable 的 session 的 EndSession 索引
std::vector<int> CChatOpsCtrl::FindLastNNotDisabledSessionEnds(int count) const
{
	std::vector<int> result;
	int disableAfter = _GetDisableAfterIndex();
	
	// 从 disableAfter 开始往前遍历（disableAfter 之后的都是被 disable 的）
	for (int i = disableAfter - 1; i >= 0 && result.size() < static_cast<size_t>(count); i--)
	{
		if (_ops[i].type == ChatOp::Op_EndSession)
		{
			result.push_back(i);
		}
	}
	
	return result;
}


//得到要恢复到user messageId之前的状态,需要restore哪些checkpoints,按照自后到前的顺序排列
bool CChatOpsCtrl::GetRestoreCheckpoints(const std::wstring& userMessageId, std::vector<FilesCheckpointUID>& checkpointIds)
{
	// 清空输出参数
	checkpointIds.clear();

	int sessionBegin = _GetSessionBeginOfUserMessage(userMessageId);
	if (sessionBegin < 0)
		return false;

	// 获取 disable 边界
	int disableAfterIndex = _GetDisableAfterIndex();

	for (int i = disableAfterIndex - 1; i >= sessionBegin; i--)
	{
		ChatOp& op = _ops[i];
		if ((op.type == ChatOp::Op_SetFileEditContent) || (op.type == ChatOp::Op_BeginSession))
		{
			if (op.checkpointId != FilesCheckpointUID_Invalid)
				checkpointIds.push_back(op.checkpointId);
		}
	}

	return !checkpointIds.empty();
}

bool CChatOpsCtrl::UpdateFileEditDiffContent(const std::wstring& fileEditId, const std::wstring& diffContent, FilesCheckpointUID checkpointId)
{

	// 查找最后一个相关的 SetFileEditContent 操作
	ChatOp* lastSetContentOp = nullptr;
	for (auto it = _ops.rbegin(); it != _ops.rend(); ++it)
	{
		if (it->type == ChatOp::Op_SetFileEditContent && it->fileEditId == fileEditId)
		{
			lastSetContentOp = &(*it);
			break;
		}
	}

	bool hasModification = false;

	// 检查并更新操作记录中的内容
	if (lastSetContentOp != nullptr)
	{
		if (lastSetContentOp->diffContent != diffContent || lastSetContentOp->checkpointId != checkpointId)
		{
			lastSetContentOp->diffContent = diffContent;
			lastSetContentOp->checkpointId = checkpointId;
			hasModification = true;
		}
	}

	// 查找并更新内部存储的FileEdit窗口
	FileEdit* window = _FindFileEdit(fileEditId);
	if (window != nullptr)
	{
		if (window->diffContent != diffContent)
		{
			window->diffContent = diffContent;
			hasModification = true;
		}

		// 更新WebView中的显示
		_SendFileEditMsg(L"updateFileEditContent", *window);
	}

	// 如果有修改，增加操作记录版本号
	if (hasModification)
	{
		_ver++;
	}

	return hasModification;
}

bool CChatOpsCtrl::SetFileEditHeadCheckpoint(const std::wstring& fileEditId, FilesCheckpointUID checkpointId)
{
	int sessionBeginIdx = _GetSessionBeginOfFileEdit(fileEditId);
	if (sessionBeginIdx < 0)
		return false;
	if (_ops[sessionBeginIdx].checkpointId != checkpointId)
	{
		_ops[sessionBeginIdx].checkpointId = checkpointId;
		_ver++;
	}
	return true;
}


bool CChatOpsCtrl::GetFileEditCheckpoint(const std::wstring& fileEditId, FilesCheckpointUID& fileEditCheckpointId) const
{
	fileEditCheckpointId = FilesCheckpointUID_Invalid;

	// 查找该FileEdit的最新内容更新操作
	auto contentIt = std::find_if(_ops.rbegin(), _ops.rend(), [&](const ChatOp& op) {
		return op.type == ChatOp::Op_SetFileEditContent && op.fileEditId == fileEditId;
	});

	if (contentIt != _ops.rend())
	{
		fileEditCheckpointId = contentIt->checkpointId;
		return true;
	}

	return false;
}

bool CChatOpsCtrl::GetFileEditCheckpointInSessionBegin(const std::wstring& fileEditId, FilesCheckpointUID& checkpointId)const
{
	checkpointId = FilesCheckpointUID_Invalid;

	int fileEditIndex = _FindFileEditOpIndex(fileEditId);
	if (fileEditIndex < 0)
		return false;

	for (int i = fileEditIndex - 1;i >= 0;i--)
	{
		if (_ops[i].type == ChatOp::Op_BeginSession)
		{
			checkpointId = _ops[i].checkpointId;
			return true;
		}
	}

	return false;
}


//得到FileEdit的checkpoint之前的那个checkpoint(这次修改之前的文件状态)
//isHead为true, 表示这个checkpoint是在这个session的Op_BeginSession 里
bool CChatOpsCtrl::GetFileEditPrevCheckpointInSession(const std::wstring& fileEditId, FilesCheckpointUID& checkpointId, bool& isHead)const
{
	checkpointId = FilesCheckpointUID_Invalid;
	isHead = false;

	int fileEditIndex = _FindFileEditOpIndex(fileEditId);
	if (fileEditIndex < 0)
		return false;

	for (int i = fileEditIndex - 1;i >= 0;i--)
	{
		if (_ops[i].type == ChatOp::Op_SetFileEditContent)
		{
			std::wstring fullPath;
			if (GetFileEditFullPath(_ops[i].fileEditId, fullPath))
			{
				if (fullPath == _ops[fileEditIndex].fullPath)
				{
					if (_ops[i].checkpointId != FilesCheckpointUID_Invalid)
					{
						checkpointId = _ops[i].checkpointId;
						isHead = false;
						return true;
					}
				}
			}
		}
		if (_ops[i].type == ChatOp::Op_BeginSession)
		{
			checkpointId = _ops[i].checkpointId;
			isHead = true;
			return true;
		}
	}

	return false;
}

bool CChatOpsCtrl::IsFileEditInLastNotDisabledSession(const std::wstring& fileEditId) const
{
	int sessionBegin = _GetLastNotDisabledSessionBegin();
	if (sessionBegin < 0)
		return false;

	int fileEditSessionBegin = _GetSessionBeginOfFileEdit(fileEditId);
	if (fileEditSessionBegin < 0)
		return false;
	return (sessionBegin == fileEditSessionBegin);
}

void CChatOpsCtrl::GetFileEditFilePathesByMessageId(const std::wstring& messageId, std::vector<std::wstring>& filePathes) const
{
	filePathes.clear();

	// 使用 set 来剔除重复路径
	std::unordered_set<std::wstring> uniquePaths;

	// 遍历 session 范围内的所有操作
	for (int i = 0; i < static_cast<int>(_ops.size()); i++)
	{
		const ChatOp& op = _ops[i];

		if (op.messageId != messageId)
			continue;

		// 只处理有有效 checkpoint 的 SetFileEditContent 操作
		if (op.type == ChatOp::Op_AddFileEditToAIMessage)
		{
			FilesCheckpointUID checkpointId;
			if (GetFileEditCheckpoint(op.fileEditId, checkpointId))
			{
				if (checkpointId != FilesCheckpointUID_Invalid)
				{
					// 获取该 fileEditId 对应的文件路径
					std::wstring fullPath;
					if (GetFileEditFullPath(op.fileEditId, fullPath))
					{
						// 使用 set 去重
						if (uniquePaths.insert(fullPath).second)
						{
							filePathes.push_back(fullPath);
						}
					}
				}
			}
		}
	}
}

std::wstring CChatOpsCtrl::GetLastFileEditCheckpointFromFilePath(const std::wstring& messageId, const std::wstring& fullPath) const
{
	// 从后向前遍历所有操作
	for (int i = static_cast<int>(_ops.size()) - 1; i >= 0; i--)
	{
		const ChatOp& op = _ops[i];

		// 检查messageId是否匹配
		if (op.messageId != messageId)
			continue;

		// 只处理Op_AddFileEditToAIMessage操作
		if (op.type != ChatOp::Op_AddFileEditToAIMessage)
			continue;

		// 检查路径是否匹配（不区分大小写）
		if (_wcsicmp(op.fullPath.c_str(), fullPath.c_str()) != 0)
			continue;

		// 获取这个fileEdit的checkpoint
		FilesCheckpointUID checkpointId;
		if (GetFileEditCheckpoint(op.fileEditId, checkpointId))
		{
			// 如果checkpoint有效，返回fileEditId
			if (checkpointId != FilesCheckpointUID_Invalid)
			{
				return op.fileEditId;
			}
		}
	}

	// 没有找到符合条件的fileEdit，返回空字符串
	return L"";
}

bool CChatOpsCtrl::GetFileEditFullPath(const std::wstring& fileEditId, std::wstring& fullPath) const
{
	fullPath.clear();

	// 查找对应的FileEdit创建操作
	int fileEditIndex = const_cast<CChatOpsCtrl*>(this)->_FindFileEditOpIndex(fileEditId);

	if (fileEditIndex != -1)
	{
		fullPath = _ops[fileEditIndex].fullPath;
		return !fullPath.empty();
	}

	return false;
}

bool CChatOpsCtrl::GetFileEditContent(const std::wstring& fileEditId, std::string& content) const
{
	content.clear();

	// 查找最新的FileEdit内容设置操作
	auto contentIt = std::find_if(_ops.rbegin(), _ops.rend(), [&](const ChatOp& op) {
		return op.type == ChatOp::Op_SetFileEditContent && op.fileEditId == fileEditId;
	});

	if (contentIt != _ops.rend())
	{
		content = contentIt->contentUtf8;
		return !content.empty();
	}

	return false;
}

void CChatOpsCtrl::GetNotDisabledFileEditsStartingFrom(const std::wstring& fileEditId, std::vector<std::wstring>& fileEditIds) const
{
	fileEditIds.clear();

	// 查找指定fileEditId在操作队列中的位置
	int startIndex = const_cast<CChatOpsCtrl*>(this)->_FindFileEditOpIndex(fileEditId);

	// 如果没有找到指定的fileEdit，直接返回
	if (startIndex == -1)
		return;

	// 获取disable边界索引
	int disableAfterIndex = const_cast<CChatOpsCtrl*>(this)->_GetDisableAfterIndex();

	// 从指定fileEdit的位置开始，向后遍历所有操作
	for (int i = startIndex; i < static_cast<int>(_ops.size()) && i < disableAfterIndex; i++)
	{
		const ChatOp& op = _ops[i];

		// 如果是FileEdit添加操作
		if (op.type == ChatOp::Op_AddFileEditToAIMessage)
		{
			if (op.fullPath != _ops[startIndex].fullPath)
				continue;
			// 检查是否已经存在，避免重复添加
			bool exists = false;
			for (const auto& existingId : fileEditIds)
			{
				if (existingId == op.fileEditId)
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				fileEditIds.push_back(op.fileEditId);
			}
		}
	}
}

int CChatOpsCtrl::_FindFirstOpIndexInSession(int sessionBeginIdx, ChatOp::Type tp) const
{
	for (int i = sessionBeginIdx; i < static_cast<int>(_ops.size()); i++)
	{
		if (_ops[i].type == tp)
			return i;
	}
	return -1;
}

void CChatOpsCtrl::_CollectSessionTags(int sessionBeginIndex, int sessionEndIndex, std::vector<ChatInputTag>& tags) const
{
	for (int i = sessionBeginIndex; i < sessionEndIndex; i++)
	{
		if (_ops[i].type == ChatOp::Op_AddSessionTag || _ops[i].type == ChatOp::Op_AddSessionDisabledTag)
		{
			ChatInputTag tag;
			tag.path = _ops[i].fullPath;
			tag.text = utf8_to_widechar(_ops[i].contentUtf8);
			if (!Utils::IsImageFile(widechar_to_utf8(tag.path.c_str()).c_str()))
				tag.type = L"file";
			else
				tag.type = L"image";
			tag.visible = (_ops[i].type == ChatOp::Op_AddSessionTag);
			tag.removable = true;
			tags.push_back(tag);
		}
	}
}

void CChatOpsCtrl::GetLastNotDisabledSessionTags(std::vector< ChatInputTag>& tags) const
{
	tags.clear();
	int sessionBeginIndex = _GetLastNotDisabledSessionBegin();
	int sessionEndIndex = _GetLastNotDisabledSessionEnd();

	_CollectSessionTags(sessionBeginIndex, sessionEndIndex, tags);
}

void CChatOpsCtrl::GetUserMessageSessionTags(const std::wstring& userMessageId, std::vector< ChatInputTag>& tags) const
{
	tags.clear();

	// 获取用户消息所在的session开始索引
	int sessionBeginIndex = _GetSessionBeginOfUserMessage(userMessageId);
	if (sessionBeginIndex < 0)
	{
		return; // 未找到用户消息
	}

	// 找到session的结束索引（下一个BeginSession或数组末尾）
	int sessionEndIndex = static_cast<int>(_ops.size());
	for (int i = sessionBeginIndex + 1; i < static_cast<int>(_ops.size()); i++)
	{
		if (_ops[i].type == ChatOp::Op_BeginSession)
		{
			sessionEndIndex = i;
			break;
		}
	}

	// 收集session中的所有tags
	_CollectSessionTags(sessionBeginIndex, sessionEndIndex, tags);
}

int CChatOpsCtrl::AddFileAttaches(const std::string& fileList, FilesCheckpointUID checkpointId)
{
	ChatOp op(ChatOp::Op_FileAttaches);
	op.contentUtf8 = fileList;
	op.checkpointId = checkpointId;
	_AddOp(op);
	return _ops.size() - 1;
}

void CChatOpsCtrl::AddToolCallResult(const std::string& jsonString, const std::string& jsonStringPartial, const std::string& jsonStringFullCompress)
{
	ChatOp op(ChatOp::Op_AddToolCallResult);
	op.contentUtf8 = jsonString;
	if (!jsonStringPartial.empty())
	{
		op.compressedContents[1] = jsonStringPartial;  // level 1 = Level_Partial
	}
	if (!jsonStringFullCompress.empty())
	{
		op.compressedContents[2] = jsonStringFullCompress;  // level 2 = Level_Full
	}
	_AddOp(op);
}

void CChatOpsCtrl::AddInterjectToLastToolCallResult(const std::string& interject)
{
	if (interject.empty())
		return;

	// 找到最后一个 Op_AddToolCallResult
	int index = _FindLastOpIndex(ChatOp::Op_AddToolCallResult);
	if (index < 0)
		return;

	std::string& jsonString = _ops[index].contentUtf8;

	try
	{
		json parsed = json::parse(jsonString);

		// 格式为数组: [assistant_msg, tool_result_msg]
		if (!parsed.is_array() || parsed.size() < 2)
			return;

		// 在 tool result 消息的 content 字段末尾追加 interject
		json& toolResultMsg = parsed[1];
		if (!toolResultMsg.contains("content"))
			return;

		std::string resultContent = toolResultMsg["content"].get<std::string>();
		std::string interjectUtf8 = interject.c_str();
		resultContent += "\nThere is an interject from the user:\n";
		resultContent += interjectUtf8;
		resultContent += "\n";
		toolResultMsg["content"] = resultContent;

		// 序列化回宽字符串并更新
		jsonString = parsed.dump().c_str();

		_ver++;
	}
	catch (const json::parse_error&)
	{
		// 解析失败则不执行任何操作
	}
}


void CChatOpsCtrl::AddToolCallMessage_Exploring(const std::wstring& messageId, const std::string& message)
{

	if (_ui)
	{
		// 转义消息内容以防XSS
		std::wstring safeMessage = EscapeJsonString(utf8_to_widechar(message));

		// 构造JSON消息，作为AI消息的一部分添加
		std::wstring jsonMessage = L"{\"action\":\"addToolCallMessageToAIMessage_Exploring\",\"content\":\"" + safeMessage + L"\",\"id\":\"" + messageId + L"\"}";

		// 发送消息到WebView
		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	ChatOp op(ChatOp::Op_AddToolCallMessage_Exploring);
	op.contentUtf8 = message;
	op.messageId = messageId;
	_AddOp(op);
}

void CChatOpsCtrl::AddUserInterject(const std::wstring& messageId, const std::string& interject)
{
	if (interject.empty())
		return;

	if (_ui)
	{
		// 转义消息内容

		std::wstring safeInterject = EscapeJsonString(utf8_to_widechar(interject));
		std::wstring safeMessageId = EscapeJsonString(messageId);

		// 构造 JSON 消息
		std::wstring jsonMessage = L"{\"action\":\"addUserInterjectToAIMessage\",\"messageId\":\"" + safeMessageId + L"\",\"content\":\"" + safeInterject + L"\"}";

		_ui->PostJsonMessage(jsonMessage);
	}

	// 记录操作
	ChatOp op(ChatOp::Op_AddUserInterject);
	op.contentUtf8 = interject;
	op.messageId = messageId;
	_AddOp(op);
}

// ── CliDisplay 辅助函数 ───────────────────────────────────────────────────

void CChatOpsCtrl::_ParseCliDisplayContent(const std::string& content,
                                            std::string& cmd,
                                            std::string& output,
                                            std::string& shellType) const
{
	// 检查是否是 JSON 格式（以 '{' 开头）
	if (!content.empty() && content[0] == '{')
	{
		// JSON 格式：{"cmd":"...", "output":"...", "shellType":"..."}
		try
		{
			json j = json::parse(content);
			
			if (j.contains("cmd"))
			{
				cmd = j["cmd"].get<std::string>();
			}
			if (j.contains("output"))
			{
				output = j["output"].get<std::string>();
			}
			if (j.contains("shellType"))
			{
				shellType = j["shellType"].get<std::string>();
			}
		}
		catch (const json::parse_error&)
		{
			// JSON 解析失败，回退到旧格式
			size_t newlinePos = content.find('\n');
			if (newlinePos != std::string::npos)
			{
				cmd = content.substr(0, newlinePos);
				output = content.substr(newlinePos + 1);
			}
			else
			{
				cmd = content;
				output = "";
			}
		}
	}
	else
	{
		// 旧格式：用第一个回车符分隔
		size_t newlinePos = content.find('\n');
		if (newlinePos != std::string::npos)
		{
			cmd = content.substr(0, newlinePos);
			output = content.substr(newlinePos + 1);
		}
		else
		{
			cmd = content;
			output = "";
		}
	}
}

std::string CChatOpsCtrl::_BuildCliDisplayContent(const std::string& cmd,
                                                    const std::string& output,
                                                    const std::string& shellType) const
{
	try
	{
		// JSON 格式：{"cmd":"...", "output":"...", "shellType":"..."}
		json j;
		j["cmd"] = cmd;
		j["output"] = output;
		if (!shellType.empty())
		{
			j["shellType"] = shellType;
		}
		
		return j.dump();
	}
	catch (const json::exception&)
	{
		// JSON 构建失败，回退到旧格式：用换行分隔 cmd 和 output
		if (!output.empty())
			return cmd + "\n" + output;
		return cmd;
	}
}


std::wstring CChatOpsCtrl::AddCliDisplay(const std::wstring& messageId, const std::string& command, const std::wstring& desc, CliDisplayStatus displayStatus, const std::string& shellType)
{
	if (command.empty())
		return L"";

	// 生成 CLI ID（确保唯一性）
	std::wstring cliId = _GenCliId();

	// 通过 CChatUi 创建 CLI 显示
	if (_ui)
	{
		_ui->AddCliDisplay(messageId, cliId, utf8_to_widechar(command), desc, displayStatus, utf8_to_widechar(shellType));
	}

	// 记录操作，使用 JSON 格式存储 command、output 和 shellType
	ChatOp op(ChatOp::Op_CliDisplay);
	op.contentUtf8 = _BuildCliDisplayContent(command, "", shellType);  // 初始时 output 为空，包含 shellType
	op.title = desc;  // 存储描述信息在 title 字段
	op.messageId = messageId;
	_AddOp(op);

	return cliId;
}

void CChatOpsCtrl::AppendOutputToLastCliDisplay(const std::wstring& messageId, const std::string& deltaOutput)
{
	if (deltaOutput.empty())
		return;

	// 查找最后一个 Op_CliDisplay
	ChatOp* lastCliOp = nullptr;
	for (int i = static_cast<int>(_ops.size()) - 1; i >= 0; --i)
	{
		if (_ops[i].type == ChatOp::Op_CliDisplay && _ops[i].messageId == messageId)
		{
			lastCliOp = &_ops[i];
			break;
		}
	}

	if (!lastCliOp)
	{
		// 如果没找到对应的Op_CliDisplay，则忽略
		return;
	}

	// 解析现有的 content（兼容旧格式和新格式）
	std::string cmd, output, shellType;
	_ParseCliDisplayContent(lastCliOp->contentUtf8, cmd, output, shellType);

	// 追加新的 output
	output += deltaOutput;

	// 重新构建 JSON 格式的 content
	lastCliOp->contentUtf8 = _BuildCliDisplayContent(cmd, output, shellType);

	if (_ui)
	{
		// 转义消息内容
		std::wstring safeMessageId = EscapeJsonString(messageId);
		std::wstring safeOutput = EscapeJsonString(utf8_to_widechar(deltaOutput));

		// 构造 JSON 消息 - 发送增量输出
		std::wstring jsonMessage = L"{\"action\":\"appendCliOutput\",\"messageId\":\"" + safeMessageId + L"\",\"output\":\"" + safeOutput + L"\"}";

		_ui->PostJsonMessage(jsonMessage);
	}

	// 递增版本号（因为修改了Op内容）
	_ver++;
}

void CChatOpsCtrl::ShowCliInputArea(const std::wstring& cliId, bool bShow)
{
	if (_ui)
	{
		// 转义 CLI ID
		std::wstring safeCliId = EscapeJsonString(cliId);

		// 构造 JSON 消息
		std::wstring jsonMessage = L"{\"action\":\"showCliInput\",\"cliId\":\"" + safeCliId + L"\",\"show\":" + (bShow ? L"true" : L"false") + L"}";

		_ui->PostJsonMessage(jsonMessage);
	}
}

void CChatOpsCtrl::CompleteCliDisplay(const std::wstring& cliId, int exitCode)
{
	if (_ui)
	{
		// 转义 CLI ID
		std::wstring safeCliId = EscapeJsonString(cliId);

		// 构造 JSON 消息
		std::wstring jsonMessage = L"{\"action\":\"completeCliDisplay\",\"cliId\":\"" + safeCliId + L"\",\"exitCode\":" + std::to_wstring(exitCode) + L"}";

		_ui->PostJsonMessage(jsonMessage);
	}
}

void CChatOpsCtrl::AddQuestionDisplay(const std::wstring& messageId, const std::wstring& question, const std::string& answer)
{
	if (question.empty() || answer.empty())
		return;

	if (_ui)
	{
		// 调用 UI 方法显示
		_ui->AddQuestionDisplay(messageId, question, utf8_to_widechar(answer));
	}

	// 记录操作，存储 question 在 title 字段，answer 在 content 字段
	ChatOp op(ChatOp::Op_QuestionDisplay);
	op.title = question;     // 问题存储在 title 字段
	op.contentUtf8 = answer;     // 答案存储在 content 字段
	op.messageId = messageId;
	_AddOp(op);
}

void CChatOpsCtrl::_GetFileAttachesList(int fileAttaches, std::vector<std::string>& filePathes)
{
	filePathes.clear();

	// 检查索引有效性
	if (fileAttaches < 0 || fileAttaches >= static_cast<int>(_ops.size()))
		return;

	// 检查操作类型
	const ChatOp& op = _ops[fileAttaches];
	if (op.type != ChatOp::Op_FileAttaches)
		return;

	// 从content字段中解析文件路径列表（以"|"分隔）
	if (!op.contentUtf8.empty())
		SplitStringBy("|", op.contentUtf8, &filePathes);
}

void CChatOpsCtrl::_GetFileAttachesList(int fileAttaches, std::unordered_set<std::string>& filePathes)
{
	filePathes.clear();
	std::vector<std::string> filePathesList;
	_GetFileAttachesList(fileAttaches, filePathesList);
	for (const auto& path : filePathesList)
	{
		filePathes.insert(path);
	}
}


bool CChatOpsCtrl::CheckValidFileAttachesCache(int fileAttaches, const std::vector<ChatInputTag>& visibleFileTags)
{
	// 收集所有可见Tags的文件路径
	std::vector<std::string> targetPaths;
	for (const auto& tag : visibleFileTags)
	{
		if (!tag.path.empty())
		{
			targetPaths.push_back(widechar_to_utf8(tag.path.c_str()));
		}
	}

	// 获取disable边界，只考虑未被disabled的操作
	int disableAfterIndex = _GetDisableAfterIndex();

	if (fileAttaches >= disableAfterIndex)
		return false;
	if (fileAttaches < 0)
		return false;

	// 寻找最后一个Op_FileAttaches
	int lastFileAttachesIndex = -1;
	for (int i = disableAfterIndex - 1; i >= 0; i--)
	{
		if (_ops[i].type == ChatOp::Op_FileAttaches)
		{
			lastFileAttachesIndex = i;
			break;
		}
	}

	// 不是最后一个
	if (lastFileAttachesIndex != fileAttaches)
		return false;

	// 检查attachPaths是否能覆盖所有的targetPaths
	std::vector<std::string> attachPaths;
	_GetFileAttachesList(fileAttaches, attachPaths);
	for (const auto& targetPath : targetPaths)
	{
		bool found = false;
		for (const auto& attachPath : attachPaths)
		{
			if (attachPath == targetPath)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	for (int i = 0;i < attachPaths.size();i++)
	{
		AbsTick t;
		if (!_GetLastFileTimeInCheckpoint(attachPaths[i], fileAttaches, disableAfterIndex, t))
			continue;//如果这个文件不在之前任何checkpoint里,说明这个文件还没有被修改过

		if (t != Utils::GetFileTick(attachPaths[i].c_str()))
			return false;
	}

	return true;
}


bool CChatOpsCtrl::MakeSessionRequest(LlmSessionRequest& request, int fileAttaches)
{
	// 	return MakeSessionRequest_Debug(request);

	// 清空 request 的消息
	request.Clear();

	// 获取 disable 边界，只处理未被 disabled 的操作
	int disableAfterIndex = _GetDisableAfterIndex();

	std::unordered_set<std::string> attachFilePathes;
	_GetFileAttachesList(fileAttaches, attachFilePathes);

	// 从找到的 Session 开始，按顺序处理操作 
	for (int i = 0; i < static_cast<int>(_ops.size()) && i < disableAfterIndex; i++)
	{
		const ChatOp& op = _ops[i];

		if (op.currentCompressionLevel == CChatOpsCompress::Level_Remove)
			continue;

		switch (op.type)
		{
		case ChatOp::Op_FileAttaches:
		{
			if (i != fileAttaches)
				continue;

			CCheckpoints* pCheckpoints = _ctx.checkpoints;

			for (const auto& filePath : attachFilePathes)
			{
				// 检查是否为图片文件
				if (Utils::IsImageFile(filePath.c_str()))
				{
					// 读取图片为 base64
					std::string base64Data;
					if (Utils::GetFileContentIntoBase64(filePath.c_str(), base64Data))
					{
						// 添加文件说明文字
						std::string message = u8"Here is the current content of image\"";
						message += filePath;
						message += u8"\":\n";
						request.AddUserMessage(message.c_str());

						// 根据 extension 确定 mimeType
						std::string mimeType = "image/jpeg";
						std::string ext = GetFileSuffix(filePath);
						StringLower(ext);
						if (ext == "png") mimeType = "image/png";
						else if (ext == "gif") mimeType = "image/gif";
						else if (ext == "webp") mimeType = "image/webp";
						else if (ext == "bmp") mimeType = "image/bmp";
						else if (ext == "tiff" || ext == "tif") mimeType = "image/tiff";
						else if (ext == "svg") mimeType = "image/svg+xml";

						request.AddUserMessageOfImage(base64Data.c_str(), mimeType.c_str());
					}
					continue;
				}

				if (Utils::CheckFileBinary(filePath.c_str()))
					continue;
				std::vector<BYTE> fileContent;
				Utils::FileContentCodingFormat codingFmt;
				if (pCheckpoints != nullptr && op.checkpointId != FilesCheckpointUID_Invalid)
				{
					std::vector<BYTE> rawContent;
					if (pCheckpoints->GetCheckpointFileContent(op.checkpointId, filePath.c_str(), rawContent))
						Utils::ConvertFileContentIntoUTF8(rawContent, fileContent, codingFmt);
				}
				if (fileContent.empty())
					Utils::GetFileContentIntoUTF8(filePath.c_str(), fileContent, codingFmt);
				if (!fileContent.empty())
				{
					// 构建系统消息内容
					std::string message = u8"Here is the current content of file\"";
					message += filePath;
					message += u8"\":\n";

					// 将文件内容转换为字符串（假设是文本文件）
					{
						std::string fileContentStr(fileContent.begin(), fileContent.end());
						message += fileContentStr;
					}
					message += u8"\n";

					// 添加到 request
//					request.AddSystemMessage(message.c_str());
					request.AddUserMessage(message.c_str());
				}
			}
			break;
		}

		case ChatOp::Op_AddUserMessage:
		{
			// 使用压缩后的内容（如果有）
			std::string effectiveContent = _GetEffectiveOpContent(op);
			// 将 wstring 转换为 UTF-8 string
			std::string userMessage = ExtractPlainTextUtf8(effectiveContent.c_str());

			// 如果用户消息为空或全是空白字符，则跳过
			if (userMessage.find_first_not_of(" \t\n\r") == std::string::npos)
				break;

			request.AddUserMessage(userMessage.c_str());
			break;
		}

		case ChatOp::Op_AddStreamingAIMessage:
		{
			// 使用压缩后的内容（如果有）
			std::string aiMessage = _GetEffectiveOpContent(_ops[i]);

			// 如果ai消息为空或全是空白字符，则跳过
			if (aiMessage.find_first_not_of(" \t\n\r") == std::string::npos)
				break;

			// 如果有内容，添加为 assistant 消息
			if (!aiMessage.empty())
			{
				request.AddAssistMessage(aiMessage.c_str());
			}
			break;
		}

		case ChatOp::Op_AddStreamingAIMessage_Thinking:
		{
			// 使用压缩后的内容（如果有）
			std::string reasoning = _GetEffectiveOpContent(_ops[i]);

			// 如果ai消息为空或全是空白字符，则跳过
			if (reasoning.find_first_not_of(" \t\n\r") == std::string::npos)
				break;

			if (!reasoning.empty())
			{
				request.AddReasoningMessage(reasoning.c_str());
			}
			break;
		}

		case ChatOp::Op_AddToolCallResult:
		{
			// 使用压缩后的内容（如果有）
			std::string effectiveContent = _GetEffectiveOpContent(op);
			if (!effectiveContent.empty())
			{
				request.AddToolCallResult(effectiveContent.c_str());
			}
			break;
		}

		case ChatOp::Op_EndSession:
		{
			if (op.currentCompressionLevel == CChatOpsCompress::CompressLevel::Level_Partial)
			{
				std::string effectiveContent = _GetEffectiveOpContent(op);
				if (!effectiveContent.empty())
				{
					request.AddAssistMessage(effectiveContent.c_str());
				}
			}
			break;
		}

		default:
			// 其他操作类型忽略
			break;
		}
	}

	extern bool IsPrompCachingEnabled();
	if (IsPrompCachingEnabled())
		request.AddCacheControl();

	return true;
}

void CChatOpsCtrl::CollectUncompressedSessionAIContent(int targetSrcIndex, const std::vector<LlmToolType>& toolTypes, std::string& content)
{
	content.clear();

	std::string collectedContent;
	_IterateSessionAIContent(targetSrcIndex, toolTypes, [&collectedContent](const std::string& fragment, LlmToolType toolType) -> bool {
		if (toolType == LlmToolType::None)
		{
			// AI 消息内容
			collectedContent += fragment;
			collectedContent += "\n\n";
		}
		else
		{
			// ToolCall 内容
			collectedContent += "[ToolCall: ";
			collectedContent += g_llmTools.GetToolTypeName(toolType);
			collectedContent += "]\n";
			collectedContent += fragment;
			collectedContent += "\n\n";
		}
		return true; // 继续遍历
	});

	content = std::move(collectedContent);
}

int CChatOpsCtrl::EstimateUncompressedSessionAIContentToken(int targetSrcIndex, const std::vector<LlmToolType>& toolTypes)
{
	int totalTokens = 0;
	_IterateSessionAIContent(targetSrcIndex, toolTypes, [&totalTokens](const std::string& fragment, LlmToolType toolType) -> bool {
		// 使用 Utils::EstimateTokenCount 估算 token 数
		totalTokens += Utils::EstimateTokenCount(fragment);
		
		// ToolCall 的额外前缀也需要估算 token
		if (toolType != LlmToolType::None)
		{
			std::string prefix = "[ToolCall: ";
			prefix += g_llmTools.GetToolTypeName(toolType);
			prefix += "]\n";
			totalTokens += Utils::EstimateTokenCount(prefix);
		}
		
		// 分隔符 "\n\n" 的估算
		totalTokens += Utils::EstimateTokenCount("\n\n");
		
		return true; // 继续遍历
	});
	return totalTokens;
}

void CChatOpsCtrl::_IterateSessionAIContent(int targetSrcIndex, 
                                               const std::vector<LlmToolType>& toolTypes,
                                               std::function<bool(const std::string&, LlmToolType)> callback) const
{
	// 验证 targetSrcIndex 有效性
	if (targetSrcIndex < 0 || targetSrcIndex >= (int)_ops.size())
		return;

	// 找到 session 边界
	int sessionBeginIndex = -1;
	int sessionEndIndex = -1;
	if (!FindSessionBoundaries(targetSrcIndex, sessionBeginIndex, sessionEndIndex))
		return;

	// 定义需要收集的 ToolCall 类型（如果 toolTypes 为空，则收集所有类型）
	auto isTargetToolType = [&toolTypes](LlmToolType toolType) {
		if (toolTypes.empty())
			return true;
		for (LlmToolType t : toolTypes)
		{
			if (t == toolType)
				return true;
		}
		return false;
	};

	// 遍历 session 内的内容
	for (int i = sessionBeginIndex; i <= sessionEndIndex; i++)
	{
		const ChatOp& op = _ops[i];

		if (op.type == ChatOp::Op_AddStreamingAIMessage)
		{
			// AI 消息内容
			if (!op.contentUtf8.empty())
			{
				if (!callback(op.contentUtf8, LlmToolType::None))
					return; // 回调返回 false，中断遍历
			}
		}
		else if (op.type == ChatOp::Op_AddToolCallResult)
		{
			// 解析 ToolCall 类型并判断是否需要收集
			LlmToolType toolType = CLlmTools::ParseToolTypeFromToolCallResultString(op.contentUtf8);
			if (isTargetToolType(toolType))
			{
				if (!op.contentUtf8.empty())
				{
					if (!callback(op.contentUtf8, toolType))
						return; // 回调返回 false，中断遍历
				}
			}
		}
	}
}


bool CChatOpsCtrl::MakeSessionRequest_Debug(LlmSessionRequest& request)
{
// 	// 清空 request 的消息
// 	request.Clear();
// 
// 	std::string fileContent = "		Berver_FFFDASNPCMovementComponent		* .记录一个BP_MovementDataContainer: myMovementContainer		* .记录当前的MoveToken : myCurrentMovement		* .记录一系列事先规划好但未开始的移动(myPendingMovements, MovementToken的数组), 所谓多段移动		* .myMovements记录了最多3个最近被丢弃的Move的Token, 一个Move被丢弃后, 在被从container中丢弃前, 仍然会存在一段时间,		* .myTokensNeedNetworking记录等待被同步到client的移动		* .BP_MovementDataContainer : 记录(最近一段时间内的以及未开始的)所有的Move数据		* .BP_MovementData 的数组, 用MovementToken索引		* .BP_MovementToken : 一个ID, 代表一次Move		* .BP_MovementDataAccessor : 对于BP_MovementDataContainer中某一个Move的访问器		* .包含一个BP_MovementDataContainer和一个BP_MovementToken		* .BP_MovementRequest : 代表一次移动的请求, 一次移动可能会请求多段移动(A-- > B, B-- > C, C--D), 每一段对应一个MovementToken, 一次请求包含所有的这些移动的token, 如果不支持多段移动, 其实不需要这个类, 可以用Movement Token代替		* .一段移动的token只有在开始以后才会被加到Request中去		* .BP_MovementData : 一次Move的所有数据		* .BServer_NPCAgentController::GenerateMovement()在 BP_MovementDataContainer 中创建一个 BP_MovementData		* .包含一个 BP_Input		* .包含一个 BP_ReconciliationData(路径信息)		* .包含一个 BP_Plan(移动规划信息, 由多个Fragment构成)		* .BP_ValuationData : 移动过程中的瞬间状态(位置, 朝向, 速度, ..)		* .BP_Input : 一次移动的参数		* .类型有 : Movement, CustomAnimation, Turn, Stop, GoToIdle, FaceTarget		* .BP_ControlPoint : 代表一个状态的ID(和位置没关系)		* .一些事先定义好的状态, 比如Idle, Walk, Turn, Jump, CustomAnimation		* .这些状态定义在一个default_control_graph.bpcontrolgraph 文件内		* .每个BP_ControlPoint作为节点定义在这个graph内		* .ControlPoint之间通过Transition节点连接, 代表这些状态之间可以互相切换		* .Transition会连上很多各种类型Contraint节点, 代表了各种条件		* .当一个Transition上的条件都满足后，就会发生一次状态切换		* .比如一个Idle的ControlPoint, 可以Transition到Walk的ControlPoint, Transition条件之一可以是当Agent目前有一条准备好的路径可以走时, 这个条件满足后, 就会发生一次Transition		* .所有Agent目前都使用default_control_graph		* .BP_ExecutePoint: 一个ID, 可以认为代表一个动画信息的Entry		* .BP_ConstraintStateMachine, 一个graph, 包括 BP_ControlPoint 和一系列 constraint 节点, transition 节点,		* .就是 default_control_graph.bpcontrolgraph		* .default_control_graph是一个graph文件的名字, 所有Agent的 RServer_NPCConfig::myBehaviorPlannerControlGraphFile 都设成这个名字		* .本身不是一个状态机, 因为它内部并没有状态, 只是一个graph		* .这个graph里主要包含以下 重要的 类		* .各种 BP_Node 派生的类, BP_WalkNode, BP_IdleNode, 等等		* .每一种 BP_Node 在一个graph 只有一个, 和 一个 BP_ControlPoint 一对一绑定		* .BP_TransitionNode, 代表一个transition, 从一个 BP_Node 到 另一个 BP_Node		* .只是一个编辑用的类, 用来在graph 里 连接其它的 node, 不参与实际运行		* .有一个 in, 一个 out, 以及若干个 contraint的 pin		* .BP_Edge, 实际的用于 transition 检测的 运行类		* .会读取 BP_TransitionNode 以及它在 graph里的连接, 来构建一个 BP_Edge, 见(BP_ControlGraphInterpreter::Interpret(..))		* .一个 BP_Node 对于每一个 它能 连接到的 BP_Node 都维护一个 BP_Edge, BP_Edge 内部 记录了 那个目标 BP_Node		* .记录了 一个 BP_IConstraint 的数组, 用来判断条件是否满足(是否可以transition), 见 BP_Edge::Evaluate(..)		* .对于多个 BP_IConstraint, 有两种满足条件, AND 模式和 OR 模式		* .各种 BP_IConstraint 派生的类, 比如 BP_WaypointTypeConstraint, BP_HaveSpaceToMoveConstraint, BP_NoneConstraint, ...		*.BP_IConstraint 根据一个context(BP_IConstraint::EvaluationContext) 来了解 当前的 外部状态(比如说当前 进展到哪个waypoint)		* .BP_BehaviorPlannerModule, 一个singleton, 封装了一个 BP_ConstraintStateMachine 实例, 以及各种用到的 BP_ControlPoint, BP_ExecutionPoint 实例		* .所以全局只有一个 BP_ConstraintStateMachine		* .BP_ConstraintStateMachineState, 真正的状态机, 包含了 BP_ConstraintStateMachine, 以及一个当前的 BP_ControlPoint* 作为状态,		* .使用 BP_ConstraintStateMachine 这个graph(其实就是各种预先设好的切换条件),		* .这个状态机专门用来生成 BP_Plan, 并不是在单位移动时切换		* .在生成 BP_Plan 时, 会沿着 BP_WorkingSet 进行虚拟的移动, 从而触发 这个状态机的 状态变化, 根据这些状态的变化, 来生成 Plan, 产生各种各样的 command 加到 BP_Plan 中去		* .每个 BP_PlanGenerator 都会在内部维护一个 BP_ConstraintStateMachineState		* .RShared_NPCConfig: 一种类型的Agent的总表, 包含这种npc的各种参数		* .RShared_AnimationDatabase, 一个动画信息数据库, 不是骨骼动画数据本身, 而是描述一套动画的信息(比如位移, 转角, ..), 记录在RShared_NPCConfig中		* .是手工配置的表（不是自动生成的)		* .是一个 RShared_AnimationDatabaseEntry 的数组		* .每个Entry用一个 BP_ExecutePoint 来索引		* .RShared_AnimationDatabaseEntry, 一套动画的信息数据		* .里面还记了一个Handle, 其实是在数组里的索引		* .RShared_AnimationQueryHandler 为一个用来访问它的工具类		* .BP_AnimationQueryData: 用来到 RShared_AnimationDatabase 中进行查询的参数		* .BP_AnimationQueryResults : 用来保存到 RShared_AnimationDatabase 中进行查询的结果, 是一个 BP_AnimationQueryResult 数组		* .BP_AnimQueryDispatcher 为一个工具类, 用来使对 RShared_AnimationDatabase 进行查询		* .BP_AnimQueryDispatcher::ExecuteAnimationQuery(..),		* .BShared_AnimationMetricsData, 偏向于和移动相关的的capability的设置		* .每一个Agent有一个BShared_AnimationMetricsData, 配置在在RShared_NPCConfig中		* .和动画没什么关系		* .包含一些在不同速度下的转弯半径(曲率)的设置		* .在不同navmesh area type的移动速度		* .BServer_AnimationMetricsQueryHandler 为一个用来访问它的工具类		* .BP_BindingBridge, 是一张表, 为每一个 BP_ControlPoint 设置一个BP_ExecutePoint, 建立对应关系		* .每种NPC会配置一张表, 手工填写		* .BP_MovementTools, 一个工具类, 集合了 BServer_AnimationMetricsQueryHandler 和 RShared_AnimationQueryHandler 和 BP_BindingBridge		* .BP_WorkingSet 是一个waypoint(BP_Waypoint)的数组		* .BP_Waypoint 代表一个路点, 有各种各样类型(BP_WaypointType)		* .大杂烩		* .注意一个 waypoint 可以同时是多种类型(比如 BP_WaypointType::STOP_EXIT 和 BP_WaypointType::START_ENTRY)		* .如果某个Waypoint的类型为ARC, 表示这个点是一条弧的起点, 下个点是弧的终点(但它的类型大概率不是ARC)		* .参加 BP_WaypointUtil_Private::InsertWaypointArc(..), 这个函数参数是一个尖锐的拐点, 正常情况下, 会在它前方插入一个ARC的Waypoint, 作为弧的起点, 自己则向后挪动, 作为弧的终点, 结果就是用一个圆角代替了原来的尖角		* .如果类型为CUSTOM_ANIMATION, 表示这个点是一段放完一段动画的终点		* .参加 BP_WorkingSet::AddCustomAnimationWaypoints(..)		* .myAnimationQueryResults(BP_AnimationQueryResults), 记录了一系列动画信息		* .BP_Plan 代表一组预先设置好的命令		* .由一个fragment(BP_CommandStreamFragment)序列 组成		* .每个flagment 本身并不记录它的时间范围, 它的时间范围是由它的所有的track的时间范围 merge 出来的		* .每个fragment 在时间上和距离上都是相接的		* .BP_Plan::AddFragment(..) 中会把新加入的flagment的时间和距离都调到上一个fragment的最后		* .每个Fragment里包含多个不同类型的Track		* .BP_MovementChangeHandler, 一个工具类, 生成Movement的核心代码		* .BP_MovementChangeHandler::RunMovementInitialization() :核心函数		* .BServer_PathFinder, 提供路径搜索功能		* .BServer_NPCPathIntegrator, 在 BServer_PathFinder 外又包了一层, 实现一下路径搜索相关的功能		* .包含一个pathfinder(BServer_PathFinder)和agent(RServer_Agent)		* .BServer_NPCAgentController::RequestMovement() 启动一个移动		* .BServer_NPCAgentController::GenerateMovement()		* .用来生成一个Movement(BP_MovementData)	";
// 
// 	request.AddUserMessage(fileContent.c_str());
// 	request.AddUserMessage("The above are required files");
// 	// 	extern bool IsPrompCachingEnabled();
// 	// 	if (IsPrompCachingEnabled())
// 	// 		request.AddCacheControl();
// 
// 	request.AddUserMessage("Hello?");
// 	request.AddAssistMessage("Hello! Can I help you");
// 	request.AddUserMessage("Can you write a poem for me?");
// 	extern bool IsPrompCachingEnabled();
// 	if (IsPrompCachingEnabled())
// 		request.AddCacheControl();

	return true;
}


bool CChatOpsCtrl::_GetLastFileTimeInCheckpoint(const std::string& fullPath, int startIdx, int endIdx, AbsTick& t)
{
	t = 0;

	// 检查参数有效性
	if (startIdx < 0 || endIdx <= startIdx || endIdx > static_cast<int>(_ops.size()) || fullPath.empty())
		return false;

	CCheckpoints* pCheckpoints = _ctx.checkpoints;
	if (!pCheckpoints)
		return false;

	std::wstring fullPathW = utf8_to_widechar(fullPath);

	// 从后往前遍历指定范围的操作，寻找最后一个包含该文件的checkpoint
	for (int i = endIdx - 1; i >= startIdx; i--)
	{
		const ChatOp& op = _ops[i];

		// 检查操作是否有有效的checkpoint
		if (op.checkpointId == FilesCheckpointUID_Invalid)
			continue;

		bool shouldCheckThisCheckpoint = false;

		// 根据操作类型判断是否需要检查此checkpoint
		switch (op.type)
		{
		case ChatOp::Op_SetFileEditContent:
		{
			// 通过fileEditId获取文件路径，如果匹配则检查checkpoint
			std::wstring fileEditFullPath;
			if (GetFileEditFullPath(op.fileEditId, fileEditFullPath) && fileEditFullPath == fullPathW)
			{
				shouldCheckThisCheckpoint = true;
			}
			break;
		}

		case ChatOp::Op_AddFileEditToAIMessage:
		{
			// 检查fullPath字段是否匹配
			if (op.fullPath == fullPathW)
			{
				shouldCheckThisCheckpoint = true;
			}
			break;
		}

		case ChatOp::Op_FileAttaches:
		{
			// 检查FileAttaches列表中是否包含指定文件
			std::vector<std::string> attachedFiles;
			SplitStringBy("|", op.contentUtf8, &attachedFiles);

			for (const auto& attachedFile : attachedFiles)
			{
				if (attachedFile == fullPath)
				{
					shouldCheckThisCheckpoint = true;
					break;
				}
			}
			break;
		}

		case ChatOp::Op_BeginSession:
		{
			// BeginSession的checkpoint需要直接检查是否包含文件
			shouldCheckThisCheckpoint = true;
			break;
		}

		default:
			// 其他操作类型暂不处理
			continue;
		}

		// 如果应该检查此checkpoint，则验证它是否真的包含指定文件
		if (shouldCheckThisCheckpoint)
		{
			if (pCheckpoints->GetCheckpointFileTick(op.checkpointId, fullPath.c_str(), t))
				return true;
		}
	}

	// 没有找到包含该文件的checkpoint
	t = 0;
	return false;
}
 


// 估算 [startIndex, endIndex) 之间的op里包含的需要发送的消息的token数
// 只包含在 MakeSessionRequest 中会搜集的消息，不包含 attached file
// useUncompressed: 是否强制使用未压缩的原始内容
int CChatOpsCtrl::_EstimateTokenCountBetweenOps(int startIndex, int endIndex, bool useUncompressed)
{
	if (startIndex < 0 || endIndex <= startIndex || endIndex > static_cast<int>(_ops.size()))
		return 0;

	int totalTokens = 0;

	// 调试用：记录各类型token数
	int userMsgTokens = 0;
	int aiMsgTokens = 0;
	int thinkingTokens = 0;
	int toolCallTokens[12] = {0};  // 按tool type分类 (LlmToolType枚举值数量)

	for (int i = startIndex; i < endIndex; i++)
	{
		const ChatOp& op = _ops[i];
		if (!useUncompressed)
		{
			if (op.currentCompressionLevel == CChatOpsCompress::Level_Remove)
				continue;
		}
		if (useUncompressed)
		{
			if (op.type == ChatOp::Op_EndSession)
				continue;
		}

		std::string effectiveContent = useUncompressed ? op.contentUtf8 : _GetEffectiveOpContent(op);

		switch (op.type)
		{
		case ChatOp::Op_AddUserMessage:
		{
			// 用户消息需要通过 ExtractPlainText 处理
			std::string plainText = ExtractPlainTextUtf8(effectiveContent);
			int tokens = Utils::EstimateTokenCount(plainText);
			totalTokens += tokens;
			userMsgTokens += tokens;
			break;
		}

		case ChatOp::Op_AddStreamingAIMessage:
		case ChatOp::Op_EndSession:
		{
			// AI消息直接使用content
			int tokens = Utils::EstimateTokenCount(effectiveContent);
			totalTokens += tokens;
			aiMsgTokens += tokens;
			break;
		}

		case ChatOp::Op_AddStreamingAIMessage_Thinking:
		{
			// AI思考消息
			int tokens = Utils::EstimateTokenCount(effectiveContent);
			totalTokens += tokens;
			thinkingTokens += tokens;
			break;
		}

		case ChatOp::Op_AddToolCallResult:
		{
			// 工具调用结果
			int tokens = Utils::EstimateTokenCount(effectiveContent);
			totalTokens += tokens;
			LlmToolType toolType = CLlmTools::ParseToolTypeFromToolCallResultString(effectiveContent);
			toolCallTokens[static_cast<int>(toolType)] += tokens;
			break;
		}

		// Op_FileAttaches 不统计（按照要求）
		// 其他类型也不统计
		default:
			break;
		}
	}

	return totalTokens;
}

int CChatOpsCtrl::_EstimateTokens() const 
{
	int endIndex = _GetDisableAfterIndex();
	return const_cast<CChatOpsCtrl*>(this)->_EstimateTokenCountBetweenOps(0, endIndex);
}

int CChatOpsCtrl::GetEstimateTokens()
{
	if (_verEstimateTokens == _ver)
		return _estimateTokensCache;
	_estimateTokensCache = _EstimateTokens();
	_verEstimateTokens = _ver;

	return _estimateTokensCache;
}
 

int CChatOpsCtrl::_EstimateUncompressedTokens() const 
{
	int endIndex = _GetDisableAfterIndex();
	return const_cast<CChatOpsCtrl*>(this)->_EstimateTokenCountBetweenOps(0, endIndex, true);
}

int CChatOpsCtrl::GetUncompressedEstimateTokens() 
{
	if (_verUncompressedEstimateTokens == _ver)
		return _uncompressedEstimateTokensCache;
	_uncompressedEstimateTokensCache = _EstimateUncompressedTokens();
	_verUncompressedEstimateTokens = _ver;

	return _uncompressedEstimateTokensCache;
}

