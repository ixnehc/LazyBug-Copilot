#include "stdh.h"
#include "ChatTaskMgr.h"
#include "ChatTask_FastApply.h"
#include "ChatTask_VerifyLlmApiProvider.h"
#include "ChatTask_ReplaceInFile.h"
#include "ChatTask_FindSymbolDefine.h"
#include "ChatTask_FindInFiles.h"
#include "ChatTask_SearchFile.h"
#include "ChatTask_ReadFile.h"
#include "ChatTask_CLI.h"
#include "ChatTask_Question.h"
#include "ChatTask_QueryFinish.h"
#include "ChatTask_ResolveSymbolLinks.h"
#include "ChatTask_CreateSkill.h"
#include "ChatTask_CompressSummarize.h"
#include <algorithm>
#include <cstring>

#include "Checkpoints.h"
#include "BackupDepot.h"

#include "ChatAgent.h"

const char* FILE_EDIT_RESULT_ERROR_PREFIX = "~Error~ :";


extern CCheckpoints* GetCheckpoints();
extern CBackupDepot* GetBackupDepot();

bool CChatTask::_SaveFileEditResult(const std::string& filePath, const std::string oldContent, const std::string newContent, Utils::FileContentCodingFormat codingFmt,const std::wstring& fileEditId)
{
	if (!_context)
		return false;

	if (newContent.empty())
	{
		return false;
	}

	CCheckpoints* checkpoints = GetCheckpoints();
	always_assert(checkpoints);
	CBackupDepot* backupDepot = GetBackupDepot();
	always_assert(checkpoints);
	FilesCheckpointUID prevCheckpointId = FilesCheckpointUID_Invalid;
	bool isHead;
	
	// 优先使用 chatOpsCtrl
	if (_context->chatOpsCtrl)
	{
		_context->chatOpsCtrl->GetFileEditPrevCheckpointInSession(fileEditId, prevCheckpointId, isHead);
	}
	
	if (prevCheckpointId == FilesCheckpointUID_Invalid)
	{
		backupDepot->Add(filePath.c_str());
		prevCheckpointId = checkpoints->AddCheckpoint(filePath.c_str());
		
		// 优先使用 chatOpsCtrl
		if (_context->chatOpsCtrl)
		{
			_context->chatOpsCtrl->SetFileEditHeadCheckpoint(fileEditId, prevCheckpointId);
		}
	}
	else
	{
		if (isHead)
		{
			backupDepot->Add(filePath.c_str());

			//ensure the file is in the checkpoint
			checkpoints->AddFileToCheckpoint(prevCheckpointId, filePath.c_str());
		}
	}
	if (prevCheckpointId != FilesCheckpointUID_Invalid)
	{
		bool isError = false;
		if (newContent.find(FILE_EDIT_RESULT_ERROR_PREFIX) == 0)
			isError = true;

		if (!isError)
		{
			if (_context->fileWriter->Write(filePath.c_str(), newContent,codingFmt))
			{
				FilesCheckpointUID newCheckpointId = checkpoints->AddCheckpoint(filePath.c_str());
				always_assert(newCheckpointId != FilesCheckpointUID_Invalid);

				if (newCheckpointId != FilesCheckpointUID_Invalid)
				{
					// 生成差异字符串用于FileEdit显示
					std::string diffString;
					extern void GenerateDiffString(const std::string & oldContent, const std::string & newContent, std::string & diffString);
					GenerateDiffString(oldContent, newContent, diffString);

// 					always_assert(!diffString.empty());

					if (diffString.empty())
						diffString = "[No Change]";

					// 优先使用 chatOpsCtrl
					if (_context->chatOpsCtrl)
					{
						_context->chatOpsCtrl->UpdateFileEditDiffContent(fileEditId, utf8_to_widechar(diffString), newCheckpointId);
					}
					
					// 优先使用 chatAgent
					if (_context->chatAgent)
					{
						_context->chatAgent->RequestSave();
					}

					return true;
				}
			}
		}
		else
		{
			FilesCheckpointUID newCheckpointId = checkpoints->AddCheckpoint(filePath.c_str());
			always_assert(newCheckpointId != FilesCheckpointUID_Invalid);

			if (newCheckpointId != FilesCheckpointUID_Invalid)
			{
				// 优先使用 chatOpsCtrl
				if (_context->chatOpsCtrl)
				{
					_context->chatOpsCtrl->UpdateFileEditDiffContent(fileEditId, utf8_to_widechar(newContent), newCheckpointId);
				}

				// 优先使用 chatAgent
				if (_context->chatAgent)
				{
					_context->chatAgent->RequestSave();
				}

				return false;
			}
		}
	}
	return false;
}

void CChatTask::_SendToolCallResult(const char *result, const char* resultPartial, const LlmToolCall* toolCallPartial, const char* resultFullCompress, const LlmToolCall* toolCallFullCompress)
{
	if (!_toolCall.IsValid())
		return;

	std::string jsonString = g_llmTools.MakeToolCallResultString(_toolCall, result);
	std::string jsonStringPartial;
	std::string jsonStringFullCompress;
	
	// 处理 partial
	if (toolCallPartial != nullptr && toolCallPartial->IsValid())
	{
		// 如果 toolCallPartial 和 resultPartial 都存在，组合使用
		if (resultPartial != nullptr && resultPartial[0] != '\0')
		{
			jsonStringPartial = g_llmTools.MakeToolCallResultString(*toolCallPartial, resultPartial);
		}
		else
		{
			jsonStringPartial = g_llmTools.MakeToolCallResultString(*toolCallPartial, result ? result : "");
		}
	}
	else if (resultPartial != nullptr && resultPartial[0] != '\0')
	{
		jsonStringPartial = g_llmTools.MakeToolCallResultString(_toolCall, resultPartial);
	}
	
	// 处理 fullCompress
	if (toolCallFullCompress != nullptr && toolCallFullCompress->IsValid())
	{
		// 如果 toolCallFullCompress 和 resultFullCompress 都存在，组合使用
		if (resultFullCompress != nullptr && resultFullCompress[0] != '\0')
		{
			jsonStringFullCompress = g_llmTools.MakeToolCallResultString(*toolCallFullCompress, resultFullCompress);
		}
		else
		{
			jsonStringFullCompress = g_llmTools.MakeToolCallResultString(*toolCallFullCompress, result ? result : "");
		}
	}
	else if (resultFullCompress != nullptr && resultFullCompress[0] != '\0')
	{
		jsonStringFullCompress = g_llmTools.MakeToolCallResultString(_toolCall, resultFullCompress);
	}
	
	// 优先使用 chatOpsCtrl
	if (_context->chatOpsCtrl)
	{
		_context->chatOpsCtrl->AddToolCallResult(jsonString, jsonStringPartial, jsonStringFullCompress);
	}
	
	// 优先使用 chatAgent
	if (_context->chatAgent)
	{
		_context->chatAgent->RequestSendToolCallResult();
	}
}

void CChatTask::_SendToolCallMessage(const char* result)
{
	if (!_context || !result)
		return;
	
	// 优先使用 chatOpsCtrl
	if (_context->chatOpsCtrl)
	{
		// 使用当前 AI 消息 ID（从 chatAgent 获取）
		if (_context->chatAgent)
		{
			const std::wstring& aiMessageId = _context->chatAgent->GetCurrentAIMessageId();
			_context->chatOpsCtrl->AddToolCallMessage(aiMessageId, std::string(result));
		}
		else
		{
			// 如果没有 chatAgent，使用空的消息 ID（可能需要调整）
			_context->chatOpsCtrl->AddToolCallMessage(L"",  std::string(result));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//

CChatTask::CChatTask()
{
	_context = nullptr;
	_status = TaskStatus::Pending;
	_llmChat = nullptr;
}

bool CChatTask::CheckType(const char* tp)
{
	return strcmp(GetType(), tp) == 0;
}


//////////////////////////////////////////////////////////////////////////
//

CChatTaskMgr::CChatTaskMgr()
{
	_isInitialized = false;
	_maxConcurrentTasks = 10;
	_singleTaskMode = true; // 默认关闭单任务模式
}

CChatTaskMgr::~CChatTaskMgr()
{
	Shutdown();
}

bool CChatTaskMgr::Init(const ChatTaskContext& context)
{
	if (_isInitialized)
		return true;
		
	// 复制context内容到_context实例中
	_context = context;
	_InitLlmChatPool();
	_isInitialized = true;
	
	return true;
}

void CChatTaskMgr::Update()
{
	if (!_isInitialized)
		return;
		
	// 1. 更新所有CLlmChat实例
	for (auto* llmChat : _availableLlmChats)
	{
		if (llmChat)
		{
			LlmSessionOutput output;
			llmChat->Process(output);
		}
	}

	// 2. 更新正在运行的任务
	for (auto* task : _running)
	{
		if (task && task->_status == TaskStatus::Running)
		{
			task->Update();
		}
	}
	
	// 3. 检查并移除已完成的任务
	_RemoveFinishedTasks();
	
	// 4. 处理待执行任务
	_ProcessPendingTasks();
}

void CChatTaskMgr::Shutdown()
{
	if (!_isInitialized)
		return;
		
	// 中断所有任务
	Interrupt();
	
	// 清理所有任务内存
	for (auto* task : _pending)
	{
		delete task;
	}
	_pending.clear();
	
	for (auto* task : _running)
	{
		delete task;
	}
	_running.clear();
	
	// 销毁LlmChat pool
	_DestroyLlmChatPool();
	
	_isInitialized = false;
}

bool CChatTaskMgr::IsRunning() const
{
	return _isInitialized && 
		   (!_running.empty() || !_pending.empty());
}

void CChatTaskMgr::Interrupt()
{
	// 中断所有待执行任务
	for (auto* task : _pending)
	{
		if (task && task->_status == TaskStatus::Pending)
		{
			task->Interrupt();
			task->_status = TaskStatus::Failure;
		}
	}
	
	// 中断所有正在运行的任务
	for (auto* task : _running)
	{
		if (task && task->_status == TaskStatus::Running)
		{
			task->Interrupt();
		}
	}
}

void CChatTaskMgr::AddTask_FastApply(const std::string& filePath, const std::string& updateContent, const std::wstring& fileEditId)
{
	CChatTask_FastApply* task = new CChatTask_FastApply(filePath, updateContent, fileEditId);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_VerifyLlmApiProvider(const LlmApiProviderTypeName& providerTypeName)
{
	CChatTask_VerifyLlmApiProvider* task = new CChatTask_VerifyLlmApiProvider(providerTypeName);
	_AddTask(task);
}


bool CChatTaskMgr::IsTaskTypeRunning(const char* taskType) const
{
	if (!taskType)
		return false;
		
	// 检查 running 队列
	for (auto* task : _running)
	{
		if (task && strcmp(task->GetType(), taskType) == 0 && task->_status == TaskStatus::Running)
		{
			return true;
		}
	}
	
	// 检查 pending 队列
	for (auto* task : _pending)
	{
		if (task && strcmp(task->GetType(), taskType) == 0)
		{
			return true;
		}
	}
	
	return false;
}

void CChatTaskMgr::_AddTask(CChatTask* task)
{
	if (!task || (!_isInitialized))
		return;
		
	TaskStatus nextStatus = _CheckPendingTaskNextStep(task);
	if (nextStatus==TaskStatus::Failure)
	{
		delete task;
		return;
	}
	// 检查是否可以立即运行
	if (nextStatus==TaskStatus::Running)
	{
		_StartTask(task);
	}
	else
	{
		task->_status = TaskStatus::Pending;
		_pending.push_back(task);
	}
}

TaskStatus CChatTaskMgr::_CheckPendingTaskNextStep(CChatTask* task)
{
	if (!task || (!_isInitialized))
		return TaskStatus::Failure;
		
	// 检查并发任务数限制
	if (_running.size() >= _GetMaxConcurrentTasks())
		return TaskStatus::Pending;

	// 检查是否需要LlmSession但没有可用的
	if (task->NeedLlmSession() && _availableLlmChats.empty())
		return TaskStatus::Pending;

	// 检查任务依赖
	for (int i = 0;i < _running.size();i++)
	{
		if (task->DependsOn(_running[i]))
		{
			if (!_running[i]->IsFinished())
				return TaskStatus::Pending;
			if(_running[i]->_status== TaskStatus::Failure)
				return TaskStatus::Failure;//依赖的task 失败了,直接失败
		}
	}
	
	return TaskStatus::Running;
}

void CChatTaskMgr::_StartTask(CChatTask* task)
{
	if (!task || task->_status != TaskStatus::Pending)
		return;
		
	// 将context实例的地址传给任务
	task->_context = &_context;
	
	// 如果需要LlmSession，分配一个
	if (task->NeedLlmSession())
	{
		task->_llmChat = _AllocateLlmChat();
	}
	
	task->_status = TaskStatus::Running;
	task->Start();
	_running.push_back(task);
}

void CChatTaskMgr::_RemoveFinishedTasks()
{
	// 检查正在运行的任务，直接删除已完成的
	auto runningIt = _running.begin();
	while (runningIt != _running.end())
	{
		auto* task = *runningIt;
		if (task && task->IsFinished())
		{
			// 回收LlmChat资源
			if (task->_llmChat)
			{
				_ReleaseLlmChat(task->_llmChat);
				task->_llmChat = nullptr;
			}
			
			// 直接删除任务
			delete task;
			runningIt = _running.erase(runningIt);
		}
		else
		{
			++runningIt;
		}
	}
	
	// 移除已失败的待执行任务
	auto pendingIt = _pending.begin();
	while (pendingIt != _pending.end())
	{
		auto* task = *pendingIt;
		if (task && task->_status == TaskStatus::Failure)
		{
			delete task;
			pendingIt = _pending.erase(pendingIt);
		}
		else
		{
			++pendingIt;
		}
	}
}

void CChatTaskMgr::_ProcessPendingTasks()
{
	auto it = _pending.begin();
	while (it != _pending.end())
	{
		auto* task = *it;
		if (task && task->_status == TaskStatus::Pending)
		{
			TaskStatus nextStatus = _CheckPendingTaskNextStep(task);
			if (nextStatus == TaskStatus::Running)
			{
				_StartTask(task);
				it = _pending.erase(it);
				continue;
			}
			if (nextStatus == TaskStatus::Failure)
			{
				task->_status = TaskStatus::Failure;
			}
		}
		++it;
	}
}

CLlmChat* CChatTaskMgr::_AllocateLlmChat()
{
	if (_availableLlmChats.empty())
		return nullptr;
		
	CLlmChat* llmChat = _availableLlmChats.back();
	_availableLlmChats.pop_back();
	_usedLlmChats.push_back(llmChat);
	
	return llmChat;
}

void CChatTaskMgr::_ReleaseLlmChat(CLlmChat* llmChat)
{
	if (!llmChat)
		return;
		
	// 从已使用列表中移除
	auto it = std::find(_usedLlmChats.begin(), _usedLlmChats.end(), llmChat);
	if (it != _usedLlmChats.end())
	{
		_usedLlmChats.erase(it);
		_availableLlmChats.push_back(llmChat);
	}
}

void CChatTaskMgr::_InitLlmChatPool()
{
	// 初始化所有CLlmChat实例并添加到可用列表
	for (size_t i = 0; i < MAX_LLMCHAT_POOL_SIZE; ++i)
	{
		_llmChatPool[i].Init();
		_availableLlmChats.push_back(&_llmChatPool[i]);
	}
}

void CChatTaskMgr::_DestroyLlmChatPool()
{
	// 清理所有CLlmChat实例
	for (size_t i = 0; i < MAX_LLMCHAT_POOL_SIZE; ++i)
	{
		_llmChatPool[i].Clear();
	}
	
	// 清理指针向量，定长数组会自动析构
	_availableLlmChats.clear();
	_usedLlmChats.clear();
	// 定长数组不需要手动清理
}

void CChatTaskMgr::AddTask_ReplaceInFile(const LlmToolCall& toolCall, const std::wstring& fileEditId)
{
	CChatTask_ReplaceInFile* task = new CChatTask_ReplaceInFile(fileEditId);
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_ReplaceInFile(const std::string& filePath, const std::string& oldLines, const std::string& newLines, const std::wstring& fileEditId)
{
	CChatTask_ReplaceInFile* task = new CChatTask_ReplaceInFile(filePath,oldLines,newLines);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_FindSymbolDefine(const LlmToolCall& toolCall)
{
	CChatTask_FindSymbolDefine* task = new CChatTask_FindSymbolDefine;
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_FindInFiles(const LlmToolCall& toolCall)
{
	CChatTask_FindInFiles* task = new CChatTask_FindInFiles;
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_SearchFile(const LlmToolCall& toolCall)
{
	CChatTask_SearchFile* task = new CChatTask_SearchFile;
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_ReadFile(const LlmToolCall& toolCall)
{
	CChatTask_ReadFile* task = new CChatTask_ReadFile;
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_CLI_Cmd(const LlmToolCall& toolCall)
{
	CChatTask_CLI* task = new CChatTask_CLI("cmd.exe");
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_CLI_Bash(const LlmToolCall& toolCall)
{
	CChatTask_CLI* task = new CChatTask_CLI("bash.exe");
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_CLI_RunScript(const LlmToolCall& toolCall)
{
	CChatTask_CLI* task = new CChatTask_CLI("python.exe");
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_Question(const LlmToolCall& toolCall)
{
	CChatTask_Question* task = new CChatTask_Question;
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_QueryFinish(const LlmToolCall& toolCall)
{
	CChatTask_QueryFinish* task = new CChatTask_QueryFinish;
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_ResolveSymbolLinks(const std::vector<SymbolLinkItem>& symbolLinks)
{
	CChatTask_ResolveSymbolLinks* task = new CChatTask_ResolveSymbolLinks(symbolLinks);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_CreateSkill(const LlmToolCall& toolCall)
{
	CChatTask_CreateSkill* task = new CChatTask_CreateSkill;
	task->SetToolCall(toolCall);
	_AddTask(task);
}

void CChatTaskMgr::AddTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName, int originalTokenCount, bool evaluationMode)
{
	CChatTask_CompressSummarize* task = new CChatTask_CompressSummarize(workingOpIndex, summarizeApiName, originalTokenCount, evaluationMode);
	_AddTask(task);
}


void CChatTaskMgr::UpdateToolCalls(std::vector<LlmToolCall>& toolCalls)
{
	for (int k = 0;k < toolCalls.size();k++)
	{
		LlmToolCall& toolCall = toolCalls[k];
		if (!toolCall.IsValid())
			continue;

		bool handled = false;
		for (int i = 0;i < _running.size();i++)
		{
			CChatTask* task = _running[i];
			if (task->GetToolCall().id == toolCall.id)
			{
				task->SetToolCall(toolCall);
				handled = true;
				break;
			}
		}
		for (int i = 0;i < _pending.size();i++)
		{
			CChatTask* task = _pending[i];
			if (task->GetToolCall().id == toolCall.id)
			{
				task->SetToolCall(toolCall);
				handled = true;
				break;
			}
		}

		if (handled)
			continue;

		switch (toolCall.tp)
		{
		case LlmToolType::ReplaceInFile:
		{
			AddTask_ReplaceInFile(toolCall, L"");
			break;
		}
		case LlmToolType::FindSymbolDefine:
		{
			if (toolCall.IsComplete())
				AddTask_FindSymbolDefine(toolCall);
			break;
		}
		case LlmToolType::FindInFiles:
		{
			if (toolCall.IsComplete())
				AddTask_FindInFiles(toolCall);
			break;
		}
		case LlmToolType::SearchFile:
		{
			if (toolCall.IsComplete())
				AddTask_SearchFile(toolCall);
			break;
		}
		case LlmToolType::ReadFile:
		{
			if (toolCall.IsComplete())
				AddTask_ReadFile(toolCall);
			break;
		}
		case LlmToolType::CLI_Cmd:
		{
			if (toolCall.IsComplete())
				AddTask_CLI_Cmd(toolCall);
			break;
		}
		case LlmToolType::CLI_Bash:
		{
			if (toolCall.IsComplete())
				AddTask_CLI_Bash(toolCall);
			break;
		}
		case LlmToolType::CLI_RunScript:
		{
			if (toolCall.IsComplete())
				AddTask_CLI_RunScript(toolCall);
			break;
		}
		case LlmToolType::Question:
		{
			if (toolCall.IsComplete())
				AddTask_Question(toolCall);
			break;
		}
		case LlmToolType::QueryFinish:
		{
			if (toolCall.IsComplete())
				AddTask_QueryFinish(toolCall);
			break;
		}
		case LlmToolType::CreateSkill:
		{
			if (toolCall.IsComplete())
				AddTask_CreateSkill(toolCall);
			break;
		}
		}


	}
}

size_t CChatTaskMgr::_GetMaxConcurrentTasks() const
{
	// 如果启用单任务模式，则只允许同时执行1个任务
	return _singleTaskMode ? 1 : _maxConcurrentTasks;
}
