#include "stdh.h"

#include "ChatAgent.h"
#include "llmlib.h"


void CChatAgent::Init(const char* chatFileName, ChatAgentContext& ctx, IChatUi *ui)
{
	_ctx = ctx;
	_fileName = chatFileName;

	_llmChat.Init();

	_opsCtrl.Init(ctx);
	_compressor.Init(&_opsCtrl);

	_ui = ui;
	if (ui)
		_opsCtrl.AttachUI(ui);

	std::string path = _MakeFilePath();
	_opsCtrl.Load(path.c_str());

	if (true)
	{
		ChatTaskContext ctx;
		ctx.fileWriter = &_chatFileWriter;
		ctx.chatOpsCtrl = &_opsCtrl;
		ctx.chatAgent= this;
		ctx.chatUi = _ui;
		_taskMgr.Init(ctx);
	}

	// 初始化文件版本号
	_chatFileVer = _opsCtrl.GetVer();
}

void CChatAgent::Shutdown()
{
	DetachUI();

	_taskMgr.Shutdown();
	_llmChat.Clear();

	_opsCtrl.Clear();

	_aiMessageId.clear();

	_fileName.clear();

	_lastCtx.Clear();

	_interject.clear();

	Zero();
}

//连接上ui,
void CChatAgent::AttachUI(IChatUi* ui)
{
	_ui = ui;
	_opsCtrl.AttachUI(ui);
}

void CChatAgent::DetachUI()
{
	_ui = nullptr;
	_opsCtrl.DetachUI();
}

void CChatAgent::Update()
{
	// 处理中断请求
	bool requestInterrupt = _requestInterrupt;
	_requestInterrupt = false;

	if (requestInterrupt)
		_taskMgr.Interrupt();
	_taskMgr.Update();

	// 处理挂起的请求（等待 context 压缩完成）
	if (_pendingRequest.valid)
	{
		// 检查是否被中断
		if (requestInterrupt)
		{
			// 被中断，清空挂起的请求
			_pendingRequest.valid = false;
			return;
		}

		// 检查压缩是否完成
		if (!_compressor.IsCompressing())
		{
			// 压缩完成，发送挂起的请求
			PendingRequest pending = _pendingRequest;
			_pendingRequest.valid = false;

			if (_DoRequest(pending.request, pending.isUserMessage))
			{
				_workingMode = WorkingMode::Chat;
				if (pending.isUserMessage)
				{
					// 开始 AI 流式消息（仅 user message 需要创建新的 AI 消息）
					_aiMessageId = _opsCtrl.StartStreamingAIMessage();
					_chatUsage.Zero();
				}
			}
			else
			{
				// 发送失败，清理状态
				if (pending.isUserMessage)
				{
					_opsCtrl.EndSession();
				}
				_FinishChat();
			}
		}
		else
		{
			// 压缩仍在进行，等待下一帧
			return;
		}
	}

	// 处理 LLM Chat 会话
	if (_llmChat.HasActiveSession())
	{
		LlmSessionOutput output;
		if (_llmChat.Process(output, requestInterrupt))
		{
			// 处理 thinking 内容
			if (!output.reasoning.empty())
				_opsCtrl.AddStreamingAIMessage_Thinking(_aiMessageId, output.reasoning);

			// 处理正文内容
			if (!output.content.empty())
				_opsCtrl.AddStreamingAIMessage(_aiMessageId, output.content);

			// 更新 ToolCalls（如果未被中断）
			if (!requestInterrupt)
				_taskMgr.UpdateToolCalls(output.updatedToolCalls);

			// 如果会话完成
			if (output.isCompleted)
			{
				_chatUsage.Accumulate(output.usage);
				_opsCtrl.NotifyPromptCache(output.usage);

				if (_ui)
					_ui->OnAfterReceiveFromLlm();

				// 处理错误
				if (output.hasError && !output.errorMessage.empty())
				{
					std::string msg = "Error: ";
					msg += output.errorMessage.c_str();
					_opsCtrl.AddSystemMessage(msg);
				}
			}
		}
	}

	// 检查是否需要发送 ToolCallResult
	if (!_llmChat.HasActiveSession() && !_taskMgr.IsRunning())
	{
		if (_requestSendToolCallResult)
		{
			_ExecuteSendToolCallResult();
			_requestSendToolCallResult = false;
			_interject.clear();
			return;
		}
	}

	// 会话结束清理
	if (!_llmChat.HasActiveSession() && !_taskMgr.IsRunning())
	{
		_FinishChat();
	}

	// 处理保存请求
	_UpdateSaveChatCtrl();
}

void CChatAgent::_UpdateSaveChatCtrl()
{
	bool needSave = false;
	if (!IsWorking())
		needSave = true;
	else
	{
		if (_requestSave)
			needSave = true;
	}
	_requestSave = false;
	if (!needSave)
		return;

	if (_opsCtrl.GetVer() <= _chatFileVer)
		return;

	if (_fileName.empty())
		_fileName = MakeDateFileName("chat");

	std::string filePath = _MakeFilePath();

	_opsCtrl.Save(filePath.c_str());
	_chatFileVer = _opsCtrl.GetVer();
}

std::string CChatAgent::_MakeFilePath() const
{
	if (_ctx.dbFolderPath.empty() || _fileName.empty())
		return "";

	return _ctx.dbFolderPath + "\\_chats\\" + _fileName;
}

bool CChatAgent::StartSession(const std::wstring& content, const std::string& apiName, const std::vector<ChatInputTag>& tags)
{
	// 检查内容是否为空
	std::wstring plainText = ExtractPlainText(content);
	if (plainText.empty())
		return false;

	// 如果已经有活动会话，不处理用户输入
	if (_llmChat.HasActiveSession())
		return false;

	// 检查 API 是否可用
	extern CLlmLib g_llmLib;
	if (!g_llmLib.IsApiAvailable(apiName))
		return false;

	// 从 tags 中提取 visible 的 tags（用于文件附件）
	std::vector<ChatInputTag> visibleFileTags;
	for (const auto& tag : tags)
	{
		if (tag.visible)
			visibleFileTags.push_back(tag);
	}

	// 准备会话上下文
	LlmSessionContext newCtx;
	newCtx.apiName = apiName;
	newCtx.visibleFileTags = visibleFileTags;
	newCtx.t = GetAbsTick();
	newCtx.fileAttaches = -1;

	// 尝试继承旧的 fileAttaches
	_TryInheritOldFileAttaches(newCtx, _lastCtx);

	// 更新上下文
	_lastCtx = newCtx;

	// 执行发送用户消息流程（传入完整的 tags，包括 visible 和非 visible）
	_ExecuteSendUserMessage(content, tags);

	return true;
}

void CChatAgent::_TryInheritOldFileAttaches(LlmSessionContext& newCtx, const LlmSessionContext& oldCtx)
{
	extern CLlmLib g_llmLib;

	// 检测我们是否可以沿用 oldCtx 里的 fileAttaches（这样可以利用 LLM 的 token cache 机制节省开销）
	newCtx.fileAttaches = -1;
	if (oldCtx.fileAttaches >= 0)
	{
		if (oldCtx.apiName == newCtx.apiName)
		{
			const LlmApi* api = g_llmLib.GetApi(newCtx.apiName);
			LlmApiCacheControlType cacheControlType = g_llmLib.GetApiCacheControlType(newCtx.apiName);
			if (api)
			{
				if (cacheControlType == LlmApiCacheControlType::Anthropic_)
				{
					// 目前 Anthropic 和 google 的 cache 的时效都是 5min
					if (newCtx.t < oldCtx.t + (5 * 60 - 10) * 1000)
					{
						if (!newCtx.visibleFileTags.empty())
						{
							if (_opsCtrl.CheckValidFileAttachesCache(oldCtx.fileAttaches, newCtx.visibleFileTags))
								newCtx.fileAttaches = oldCtx.fileAttaches;
						}
					}
				}
			}
		}
	}
}

void CChatAgent::_ExecuteSendUserMessage(const std::wstring& content, const std::vector<ChatInputTag>& allTags)
{
	// 删除所有 disabled sessions
	_RemoveDisabledSessions();

	// 为文件附件创建 checkpoint（如果需要）
	FilesCheckpointUID fileAttachesCheckpointId = FilesCheckpointUID_Invalid;
	if (_lastCtx.fileAttaches < 0 && !_lastCtx.visibleFileTags.empty())
	{
		fileAttachesCheckpointId = _OnCreateCheckpointForFileAttaches(_lastCtx);
	}

	// 创建 session head checkpoint
	FilesCheckpointUID sessionHeadCheckpointId = FilesCheckpointUID_Invalid;
	if (_ctx.checkpoints)
	{
		sessionHeadCheckpointId = _ctx.checkpoints->CreateEmptyCheckpoint();
	}

	// 开始会话
	_opsCtrl.BeginSession(sessionHeadCheckpointId);

	// 添加所有 session tags（包括 visible 和非 visible）
	for (const auto& tag : allTags)
	{
		_opsCtrl.AddSessionTag(tag);
	}

	// 如果需要，添加文件附件（仅使用 visible 的 tags）
	if (_lastCtx.fileAttaches < 0 && !_lastCtx.visibleFileTags.empty())
	{
		std::wstring filePathList;
		for (const auto& tag : _lastCtx.visibleFileTags)
		{
			if (!filePathList.empty())
				filePathList += L"|";
			filePathList += tag.path;
		}
		_lastCtx.fileAttaches = _opsCtrl.AddFileAttaches(widechar_to_utf8(filePathList.c_str()), fileAttachesCheckpointId);
	}

	// 添加用户消息
	_opsCtrl.AddUserMessage(widechar_to_utf8(content.c_str()));

	// 构建请求
	LlmSessionRequest request;
	_opsCtrl.MakeSessionRequest(request, _lastCtx.fileAttaches);

	// 检查是否需要 context 压缩
	_compressor.TryTriggerCompress();

	if (_compressor.IsCompressing())
	{
		// 需要压缩，将请求挂起
		_pendingRequest.request = request;
		_pendingRequest.isUserMessage = true;
		_pendingRequest.valid = true;
		_workingMode = WorkingMode::Chat;
		// 开始 AI 流式消息（提前创建，因为用户消息已添加）
		_aiMessageId = _opsCtrl.StartStreamingAIMessage();
		_chatUsage.Zero();
		return;
	}

	// 发送请求
	if (_DoRequest(request, true))
	{
		_workingMode = WorkingMode::Chat;
		// 开始 AI 流式消息
		_aiMessageId = _opsCtrl.StartStreamingAIMessage();
		_chatUsage.Zero();
	}
	else
	{
		// 发送失败，清理状态
		_opsCtrl.EndSession();
		_workingMode = WorkingMode::None;
	}
}

FilesCheckpointUID CChatAgent::_OnCreateCheckpointForFileAttaches(const LlmSessionContext& ctx)
{
	if (ctx.visibleFileTags.empty())
		return FilesCheckpointUID_Invalid;

	if (!_ctx.checkpoints)
		return FilesCheckpointUID_Invalid;

	FilesCheckpointUID checkpointId = _ctx.checkpoints->CreateEmptyCheckpoint();
	for (const auto& tag : ctx.visibleFileTags)
	{
		_ctx.checkpoints->AddFileToCheckpoint(checkpointId, widechar_to_utf8(tag.path.c_str()).c_str());
	}
	return checkpointId;
}

void CChatAgent::_RemoveDisabledSessions()
{
	std::vector<FilesCheckpointUID> checkpointsToDiscard;
	_opsCtrl.RemoveDisabledSessions(checkpointsToDiscard);

	// 回调通知丢弃 checkpoints
	if (_ctx.checkpoints)
	{
		for (const auto& checkpointId : checkpointsToDiscard)
		{
			_ctx.checkpoints->DiscardCheckpoint(checkpointId);
		}
	}
}

bool CChatAgent::_DoRequest(const LlmSessionRequest& request, bool isUserMessage)
{
	extern CLlmLib g_llmLib;

	// 获取 LLM 设置
	LlmSessionSetting setting;
	if (!g_llmLib.LoadLlmSetting(setting, _lastCtx.apiName, ""))
		return false;

	// 通知 UI 即将发送请求
	if (_ui)
		_ui->OnBeforeSendToLlm(false);

	// 添加 _ctx 中的 rulesFiles
	for (const auto& ruleFile : _ctx.rulesFiles)
	{
		setting.rulesFiles.push_back(ruleFile);
	}

	// 检查是否有数据库文件夹（solution 是否加载）
	bool solutionLoaded = !_ctx.dbFolderPath.empty();
	if (!solutionLoaded)
		setting.api.tools.clear();

	// 发起请求
	return _llmChat.Request(request, setting);
}

void CChatAgent::_FinishChat()
{

	// 如果没有正在进行的 AI 消息，直接返回
	if (_aiMessageId.empty())
		return;

	if (true)
	{
		std::vector<std::wstring> pathes;
		_opsCtrl.GetFileEditFilePathesByMessageId(_aiMessageId, pathes);

		for (int i = 0;i < pathes.size();i++)
			_opsCtrl.AddFileSummarizeToAIMessage(_aiMessageId, Utils::GetActualFilePath(pathes[i].c_str()));
	}

	// 完成 AI 流式消息
	_opsCtrl.CompleteStreamingAIMessage(_aiMessageId);

	// 添加本次会话的费用记录
	_opsCtrl.AddSessionCost(_chatUsage, _aiMessageId);

	// 结束会话
	_opsCtrl.EndSession();

	// 清空 AI 消息 ID
	_aiMessageId = L"";

	// 重置工作模式
	_workingMode = WorkingMode::None;

}


void CChatAgent::_ExecuteSendToolCallResult()
{
	if (!_interject.empty())
	{
		_opsCtrl.AddInterjectToLastToolCallResult(_interject);
		if (!_aiMessageId.empty())
			_opsCtrl.AddUserInterject(_aiMessageId,_interject);
		_interject.clear();
	}

	// 准备会话上下文（在会话中继续，继承之前的上下文）
	LlmSessionContext newCtx;
	newCtx.apiName = _lastCtx.apiName;
	newCtx.visibleFileTags = _lastCtx.visibleFileTags;
	newCtx.t = GetAbsTick();
	newCtx.fileAttaches = _lastCtx.fileAttaches;

	// 如果无法利用之前的 attaches，需要新添加一个 attaches
	if (newCtx.fileAttaches < 0 && !newCtx.visibleFileTags.empty())
	{
		FilesCheckpointUID checkpointId = _OnCreateCheckpointForFileAttaches(newCtx);

		std::wstring filePathList;
		for (const auto& tag : newCtx.visibleFileTags)
		{
			if (!filePathList.empty())
				filePathList += L"|";
			filePathList += tag.path;
		}
		newCtx.fileAttaches = _opsCtrl.AddFileAttaches(widechar_to_utf8(filePathList.c_str()), checkpointId);
	}

	// 更新上下文
	_lastCtx = newCtx;


	// 构建请求
	LlmSessionRequest request;
	_opsCtrl.MakeSessionRequest(request, _lastCtx.fileAttaches);

	// 检查是否需要 context 压缩
	if (_compressor.IsCompressing())
	{
		// 需要压缩，将请求挂起
		_pendingRequest.request = request;
		_pendingRequest.isUserMessage = false;
		_pendingRequest.valid = true;
		_workingMode = WorkingMode::Chat;
		return;
	}

	// 发送请求
	if (_DoRequest(request, false))
	{
		_workingMode = WorkingMode::Chat;
	}
	else
	{
		// 发送失败，清理状态
		_FinishChat();
	}
}

const std::wstring& CChatAgent::GetCurrentAIMessageId() const
{
	return _aiMessageId;
}

void CChatAgent::RequestSendToolCallResult()
{
	_requestSendToolCallResult = true;
}

// 显示文件编辑进度标签
void CChatAgent::ShowFileEditProgressLabel(const std::wstring& fileName)
{
	// 获取当前 AI 消息 ID
	std::wstring aiMessageId = GetCurrentAIMessageId();
	if (aiMessageId.empty())
	{
		// 如果没有当前 AI 消息，需要先创建一个
		aiMessageId = _opsCtrl.StartStreamingAIMessage();
		_aiMessageId = aiMessageId;

		_opsCtrl.AddStreamingAIMessage(aiMessageId, "\n");
	}
	
	// 调用 _opsCtrl 的方法显示进度标签
	_opsCtrl.ShowFileEditProgressLabel(aiMessageId, fileName);
}

// 隐藏文件编辑进度标签
void CChatAgent::HideFileEditProgressLabel()
{
	// 获取当前 AI 消息 ID
	std::wstring aiMessageId = GetCurrentAIMessageId();
	if (!aiMessageId.empty())
	{
		// 调用 _opsCtrl 的方法隐藏进度标签
		_opsCtrl.HideFileEditProgressLabel(aiMessageId);
	}
}

void CChatAgent::RemoveDisabledSessions()
{
	std::vector<FilesCheckpointUID> checkpointIds;
	_opsCtrl.RemoveDisabledSessions(checkpointIds);
	CCheckpoints* checkpoints = _ctx.checkpoints;
	if (checkpoints)
	{
		for (int i = 0;i < checkpointIds.size();i++)
			checkpoints->DiscardCheckpoint(checkpointIds[i]);
	}
}

bool CChatAgent::RestoreUserMessage(const std::wstring& messageId, RestoreUserMessageConfirmCallback confirmCallback)
{
	// 如果正在工作，不执行 restore
	if (IsWorking())
		return false;

	CCheckpoints* checkpoints = _ctx.checkpoints;
	if (!checkpoints)
		return false;

	// 获取要恢复的 checkpoint IDs
	std::vector<FilesCheckpointUID> checkpointIds;
	if (!_opsCtrl.GetRestoreCheckpoints(messageId, checkpointIds))
		return false;

	if (checkpointIds.empty())
	{
		// 没有 checkpoint 需要恢复，只需 disable messages
		_opsCtrl.DisableMessagesAfter(messageId);
		return true;
	}

	// 检查 checkpoint 链中的文件是否被修改过
	std::vector<std::string> modifiedFiles;
	if (checkpoints->CheckCheckpointsFilesModified(checkpointIds, &modifiedFiles))
	{
		// 有文件被修改，调用确认回调
		if (confirmCallback)
		{
			if (!confirmCallback(modifiedFiles))
			{
				// 用户取消
				return false;
			}
		}
		else
		{
			// 没有提供回调，默认取消
			return false;
		}
	}

	// 执行 restore
	FilesCheckpointUID restoredCheckpoint;
	FilesCheckpointUID undoCheckpoint = _opsCtrl.GetUndoCheckpoint(restoredCheckpoint);
	undoCheckpoint = checkpoints->Restore(checkpointIds, undoCheckpoint);
	_opsCtrl.SetUndoCheckpoint(undoCheckpoint);

	// Disable 消息
	_opsCtrl.DisableMessagesAfter(messageId);

	return true;
}

bool CChatAgent::RestoreDisabledMessage(UndoRestoreConfirmCallback confirmCallback)
{
	// 如果正在工作，不执行
	if (IsWorking())
		return false;

	CCheckpoints* checkpoints = _ctx.checkpoints;
	if (!checkpoints)
		return false;

	// 获取 undo checkpoint
	FilesCheckpointUID restoredCheckpoint;
	FilesCheckpointUID undoCheckpoint = _opsCtrl.GetUndoCheckpoint(restoredCheckpoint);
	if (undoCheckpoint == FilesCheckpointUID_Invalid)
	{
		// 没有 undo checkpoint，直接启用所有 disabled 消息
		_opsCtrl.EnableAllDisabledMessages();
		return true;
	}

	// 检查 restoredCheckpoint 中的文件是否被修改过
	std::vector<std::string> modifiedFiles;
	std::vector<FilesCheckpointUID> checkpointChain = { restoredCheckpoint };
	if (checkpoints->CheckCheckpointsFilesModified(checkpointChain, &modifiedFiles))
	{
		// 有文件被修改，调用确认回调
		if (confirmCallback)
		{
			if (!confirmCallback(modifiedFiles))
			{
				// 用户取消
				return false;
			}
		}
		else
		{
			// 没有提供回调，默认取消
			return false;
		}
	}

	// 执行 undo restore
	checkpoints->UndoRestore(undoCheckpoint);
	_opsCtrl.SetUndoCheckpoint(FilesCheckpointUID_Invalid);

	// 启用所有 disabled 消息
	_opsCtrl.EnableAllDisabledMessages();

	return true;
}

bool CChatAgent::GetFileEditDiff(const std::wstring& fileEditId, std::string& filePath, FilesCheckpointUID& oldCheckpointId, FilesCheckpointUID& newCheckpointId)
{
	// 获取新的 checkpoint ID
	if (!_opsCtrl.GetFileEditCheckpoint(fileEditId, newCheckpointId))
		return false;

	// 检查是否存在 Summarize
// 	bool existSummarize = _opsCtrl.ExistSummarizeInSession(fileEditId);
	if (true)
	{
		// 存在 Summarize，获取前一个 checkpoint
		bool isHead;
		if (!_opsCtrl.GetFileEditPrevCheckpointInSession(fileEditId, oldCheckpointId, isHead))
			return false;
	}
	else
	{
		// 不存在 Summarize，获取 session 开始的 checkpoint
		if (!_opsCtrl.GetFileEditCheckpointInSessionBegin(fileEditId, oldCheckpointId))
			return false;
	}

	// 验证 checkpoint 有效性
	if ((oldCheckpointId == FilesCheckpointUID_Invalid) || (newCheckpointId == FilesCheckpointUID_Invalid))
		return false;

	// 获取文件路径
	std::wstring wFilePath;
	if (!_opsCtrl.GetFileEditFullPath(fileEditId, wFilePath))
		return false;

	// 转换文件路径为本地编码
	filePath = widechar_to_utf8(wFilePath.c_str());

	return true;
}

bool CChatAgent::GetFileSummarizeDiff(const std::wstring& messageId, const std::wstring& filePath, FilesCheckpointUID& oldCheckpointId, FilesCheckpointUID& newCheckpointId)
{
	// 从 messageId 和 filePath 获取 fileEditId
	std::wstring fileEditId = _opsCtrl.GetLastFileEditCheckpointFromFilePath(messageId, filePath);
	if (fileEditId.empty())
		return false;

	// 获取新的 checkpoint ID
	if (!_opsCtrl.GetFileEditCheckpoint(fileEditId, newCheckpointId))
		return false;

	// 获取 session 开始的 checkpoint
	if (!_opsCtrl.GetFileEditCheckpointInSessionBegin(fileEditId, oldCheckpointId))
		return false;

	// 验证 checkpoint 有效性
	if ((oldCheckpointId == FilesCheckpointUID_Invalid) || (newCheckpointId == FilesCheckpointUID_Invalid))
		return false;

	return true;
}

