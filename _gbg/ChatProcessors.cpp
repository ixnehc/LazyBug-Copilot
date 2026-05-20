#include "stdh.h"
#include "ChatProcessors.h"

#include "stringparser/stringparser.h"
#include "encrypt/encrypt.h"
#include "Registry/Registry.h"
#include <sstream>  // 用于std::istringstream
#include <fstream>  // 用于std::ifstream, std::ofstream
#include <regex>    // 用于std::regex, std::smatch, std::regex_search
#include <ios>      // 用于std::ios::cur
#include <vector>   // 用于std::vector
#include <string>
#include <iostream>

#include "Utils.h"

#include "ChatDialog.h"

////////////////////////////////////////////////////////////////////////////
//CChatProcessor_PatchFile

// 定义常量
const std::string PATCH_FILE_START_MARK = u8"```patch_file";
const std::string PATCH_FILE_END_MARK = u8"```";


int CChatProcessor_PatchFile::GetProcessStart(const std::string& workingStr, const LlmProcessorsContext& context) const
{
	return ::GetProcessStart(workingStr, PATCH_FILE_START_MARK);
}

bool CChatProcessor_PatchFile::Process(std::string& workingStr, const LlmProcessorsContext& context, LlmProcessorsResult& result)
{
	std::string content;

	bool isFullResult = false;
	if (!CullMarkDown(workingStr, PATCH_FILE_START_MARK, PATCH_FILE_END_MARK, content,isFullResult))
		return false;
	if (!isFullResult)
		return false;

// 	result.AddOutputDelta(std::string("\n"));
// 	result.AddOutputDelta(content);

	std::string filePath;
	if (CullHeadMarkLine(content, std::string("//file_path"), filePath))
	{
		if (true)
		{
			std::string s;
			s = u8"\n";
			s = s+u8"```\""+ GetFileName(filePath)+ u8"\"``` modified\n";

			result.AddOutputDelta(s);
		}
		UnifiedDiffChunk chunk;
		CollectUnifiedDiffChunk(content, filePath, chunk);

		// 将收集到的chunk添加到_chunks中
		_chunks.push_back(std::move(chunk));

		return true;
	}

	return false;
}

void CChatProcessor_PatchFile::Stop(const LlmProcessorsContext& context, LlmProcessorsResult& result)
{
	std::string filePath;

	while (!_chunks.empty())
	{
		filePath = _chunks[0].filePath;

		DiffTargetFile file;
		LoadDiffTargetFile(filePath, file);

		DWORD c = 0;
		for (int i = 0;i < _chunks.size();i++)
		{
			if (_chunks[i].filePath == filePath)
			{
				extern void ApplyUnifiedDiff(DiffTargetFile & file, UnifiedDiffChunk & chunk);
				ApplyUnifiedDiff(file, _chunks[i]);
				continue;
			}
			_chunks[c++] = _chunks[i];
		}
		_chunks.resize(c);

		SaveDiffTargetFile(filePath, file);
	}
}

//////////////////////////////////////////////////////////////////////////
//CChatProcessor_ReplaceInFile

const std::string REPLACE_IN_FILE_START_MARK = u8"```replace_in_file";
const std::string REPLACE_IN_FILE_END_MARK = u8"```";

int CChatProcessor_ReplaceInFile::GetProcessStart(const std::string& workingStr, const LlmProcessorsContext& context) const
{
	return ::GetProcessStart(workingStr, REPLACE_IN_FILE_START_MARK);
}

bool CChatProcessor_ReplaceInFile::Process(std::string& workingStr, const LlmProcessorsContext& context0, LlmProcessorsResult& result)
{
	std::string content;
	bool isFullResult = false;
	if (!CullMarkDown(workingStr, REPLACE_IN_FILE_START_MARK, REPLACE_IN_FILE_END_MARK, content, isFullResult))
		return false;

	std::string filePath;
	CullHeadMarkLine(content, std::string("//file_path"), filePath);

	ChatProcessorsContext& context = (ChatProcessorsContext&)context0;

	if (!isFullResult)
	{
		if (filePath.empty())
			context.chatDialog->ShowFileEditProgressLabel(std::wstring(L""));
		else
		{
			std::string fileName = GetFileName(filePath);
			context.chatDialog->ShowFileEditProgressLabel(utf8_to_widechar(fileName.c_str()));
		}
		return false;
	}

	if (isFullResult)
	{
		if (!filePath.empty())
		{
			extern bool BuildReplaceInFileToolCallsFromFileEditContent(const char* filePath, const char* fileEditContent, std::vector<LlmToolCall>&toolCalls);
			std::vector<LlmToolCall> toolCalls;
			if (BuildReplaceInFileToolCallsFromFileEditContent(filePath.c_str(), content.c_str(), toolCalls))
			{
				std::string filePath;
				std::string oldLines, newLines;
				for (int i = 0;i < toolCalls.size();i++)
				{
					toolCalls[i].GetStringParam("file_path", filePath);
					toolCalls[i].GetStringParam("old_lines", oldLines);
					toolCalls[i].GetStringParam("new_lines", newLines);

					context.chatTaskMgr->AddTask_ReplaceInFile(filePath, oldLines, newLines, std::wstring(L""));
				}
			}
		}
		return true;
	}

	return false;
}

void CChatProcessor_ReplaceInFile::Stop(const LlmProcessorsContext& context, LlmProcessorsResult& result)
{

}



//////////////////////////////////////////////////////////////////////////
//CChatProcessor_ReplaceInFileUseJson
// 从部分JSON内容中提取文件路径的辅助函数
bool CChatProcessor_ReplaceInFileUseJson::ExtractFilePathFromPartialJson(const std::string& content, std::string& filePath)
{
	// 直接通过字符串查找 "file_path" : "路径" 模式
	size_t pos = content.find("\"file_path\"");
	if (pos == std::string::npos)
		return false;
	
	// 查找冒号
	pos = content.find(":", pos);
	if (pos == std::string::npos)
		return false;
	
	// 跳过空白字符
	pos++;
	while (pos < content.length() && (content[pos] == ' ' || content[pos] == '\t'))
		pos++;
	
	// 查找开始引号
	if (pos >= content.length() || content[pos] != '"')
		return false;
	
	pos++; // 跳过开始引号
	size_t start = pos;
	
	// 查找结束引号
	size_t end = content.find('"', start);
	if (end == std::string::npos)
		return false;
	
	filePath = content.substr(start, end - start);
	return !filePath.empty();
}

bool CChatProcessor_ReplaceInFileUseJson::Process(std::string& workingStr, const LlmProcessorsContext& context0, LlmProcessorsResult& result)
{
	std::string content;
	bool isFullResult = false;
	if (!CullMarkDown(workingStr, REPLACE_IN_FILE_START_MARK, REPLACE_IN_FILE_END_MARK, content, isFullResult))
		return false;

	ChatProcessorsContext& context = (ChatProcessorsContext&)context0;

	// 如果还没有完整结果，显示进度
	if (!isFullResult)
	{
		if (_filePath.empty())
		{
			// 首先尝试通过字符串分析获取文件路径
			std::string filePath;
			if (ExtractFilePathFromPartialJson(content, filePath))
			{
				if (!filePath.empty() && context.chatDialog)
				{
					_filePath = filePath;
					std::string fileName = GetFileName(filePath);
					context.chatDialog->ShowFileEditProgressLabel(utf8_to_widechar(fileName.c_str()));
				}
			}
			else
			{
				// 如果字符串分析失败，尝试解析部分JSON来获取文件路径用于显示进度
				using json = nlohmann::json;
				if (json::accept(content.c_str()))
				{
					json data = json::parse(content);
					if (data.contains("file_path") && data["file_path"].is_string())
					{
						std::string filePath = data["file_path"];
						if (!filePath.empty() && context.chatDialog)
						{
							_filePath = filePath;
							std::string fileName = GetFileName(filePath);
							context.chatDialog->ShowFileEditProgressLabel(utf8_to_widechar(fileName.c_str()));
						}
					}
				}
			}
			if (_filePath.empty() && context.chatDialog)
				context.chatDialog->ShowFileEditProgressLabel(L"");
		}
		return false;
	}

	// 完整结果，解析JSON并执行替换操作
	using json = nlohmann::json;
	if (json::accept(content.c_str()))
	{
		json data = json::parse(content);

		// 检查JSON格式是否正确
		if (!data.contains("file_path") || !data["file_path"].is_string())
			return false;
		
		if (!data.contains("changes") || !data["changes"].is_array())
			return false;

		std::string filePath = data["file_path"];
		
		// 处理每个替换操作
		for (const auto& change : data["changes"])
		{
			if (!change.contains("find") || !change["find"].is_string())
				continue;
			if (!change.contains("replace_with") || !change["replace_with"].is_string())
				continue;

			std::string oldLines = change["find"];
			std::string newLines = change["replace_with"];

			// 添加替换任务
			if (context.chatTaskMgr)
			{
				context.chatTaskMgr->AddTask_ReplaceInFile(filePath, oldLines, newLines, std::wstring(L""));
			}
		}

		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//CChatProcessor_CollectFileEdit
const std::string EDIT_FILE_START_MARK = u8"```edit_file";
const std::string EDIT_FILE_END_MARK = u8"```";

int CChatProcessor_CollectFileEdit::GetProcessStart(const std::string& workingStr, const LlmProcessorsContext& context) const
{
	return ::GetProcessStart(workingStr,EDIT_FILE_START_MARK);
}

bool CullCommand_EditFile(std::string& str, ChatCmd_EditFile &cmd,bool &isFullResult)
{
	isFullResult = false;
	std::string content;

	if (!CullMarkDown(str, EDIT_FILE_START_MARK, EDIT_FILE_END_MARK, content,isFullResult))
		return false;

	if (CullHeadMarkLine(content, std::string("//file_path"), cmd.filePath))
	{
		if (CullHeadMarkLine(content, std::string("//instruction"), cmd.instruction))
			cmd.content = content;
	}

	return true;
}

bool CChatProcessor_CollectFileEdit::Process(std::string& workingStr, const LlmProcessorsContext& context0, LlmProcessorsResult& result0)
{
	ChatProcessorsResult& result = (ChatProcessorsResult&)result0;
	ChatCmd_EditFile cmd;
	bool isFullResult=false;
	if (CullCommand_EditFile(workingStr, cmd, isFullResult))
	{
		if (isFullResult)
		{
			extern ChatCmd_EditFile g_recentCmd;
			extern CCurrentUserRegistry g_reg;
			g_recentCmd = cmd;
			g_reg.WriteData("RecendEditFileCmd", "FilePath", (void*)cmd.filePath.c_str(), cmd.filePath.length() + 1);
			g_reg.WriteData("RecendEditFileCmd", "Content", (void*)cmd.content.c_str(), cmd.content.length() + 1);
			g_reg.WriteData("RecendEditFileCmd", "Instruction", (void*)cmd.instruction.c_str(), cmd.instruction.length() + 1);
		}

		ChatProcessorsContext& context = (ChatProcessorsContext&)context0;
		if (context.chatDialog)
		{
			if (!cmd.filePath.empty())
			{
				std::string fileName = GetFileName(cmd.filePath);

				const std::wstring& aiMessageId = context.chatDialog->GetCurAIMessageID();
				if (_fileEditId.empty())
					_fileEditId = context.chatDialog->GetChatCtrl().AddFileEditToAIMessage(aiMessageId, utf8_to_widechar(fileName), 
						utf8_to_widechar(cmd.filePath),std::wstring(),L"");
				context.chatDialog->GetChatCtrl().SetFileEditContent(_fileEditId, utf8_to_widechar(cmd.content), L"",FilesCheckpointUID_Invalid);
			}
		}

		if (isFullResult)
		{
			if (context.chatTaskMgr)
				context.chatTaskMgr->AddTask_FastApply(cmd.filePath, cmd.content, _fileEditId);

			_fileEditId.clear();

			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//CChatProcessors

void CChatProcessors::Init(CChatDialog* chatDialog)
{
	_context.chatDialog = chatDialog;
	_context.chatTaskMgr = chatDialog ? &chatDialog->GetChatTaskMgr() : nullptr;
}


void CChatProcessors::_GetProcessors(std::vector<CLlmProcessor*>& processors)
{
	processors.clear();
	processors.push_back(&_collectFileEdit);
	processors.push_back(&_replaceInFile);
	processors.push_back(&_replaceInFileUseJson);
	processors.push_back(&_patchFile);
}


void CChatProcessors::Interrupt()
{
	CLlmProcessors::Interrupt();
}
