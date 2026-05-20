#include "stdh.h"
#include "ChatTask_FastApply.h"
#include <algorithm>
#include <cstring>
#include "Utils.h"

#include "LlmChat.h"

#include "LlmLib.h"

// 声明外部函数
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);

CChatTask_FastApply::CChatTask_FastApply(const std::string& filePath, const std::string& updateContent, const std::wstring& fileEditId)
	: _filePath(filePath), 
	_updateContent(updateContent), 
	_fileEditId(fileEditId),
	_hasStartedRequest(false), 
	_requestInterrupt(false),
	_originalFileCodingFmt(Utils::FileContentCodingFormat::None)
{
}

bool CChatTask_FastApply::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("FastApply"))
		return false;

	CChatTask_FastApply* task = (CChatTask_FastApply*)task0;
	if (task->_filePath == _filePath)
		return true;
	return false;
}


void CChatTask_FastApply::Start()
{
	// 读取原始文件内容
	if (!Utils::GetFileContentIntoUTF8(_filePath.c_str(), _originalFileContent,_originalFileCodingFmt))
	{
		_status = TaskStatus::Failure;
		return;
	}

	// 构造消息内容，参考CChatDialog::_HandleCommand_RewriteFile()的格式
	std::string message;
	message += u8"<code>" + _originalFileContent + u8"</code>\n";
	
	// 处理updateContent的编码
	std::wstring wstr = utf8_to_widechar(_updateContent.c_str());
	std::string content = widechar_to_utf8(wstr.c_str());
	message += u8"<update>" + content + u8"</update>";

	// 	message = "<|im_start|>system\n";
	// 	message += "You are a coding assistant that helps merge code updates, ensuring every modification is fully integrated.\n";
	// 	message += "<|im_start|>user\n";
	// 	message += "Merge all changes from the <update> snippet into the <code> below.\n";
	// 	message += "- Preserve the code's structure, order, comments, and indentation exactly.\n";
	// 	message += "- Output only the updated code, enclosed within <updated - code> and </updated-code> tags.\n";
	// 	message += "- Do not include any additional text, explanations, placeholders, ellipses, or code fences.\n";
	// 
	// 	message += "<code>{" + fileContent + " }</code>\n";
	// 	message += "<update>{" + local_to_utf8(cmd.content) + "}</update>\n";
	// 
	// 	message += "Provide the complete updated code.<|im_end|>\n";
	// 	message += "<|im_start|>assistant";


	LlmSessionSetting setting;
	extern CLlmLib g_llmLib;
	if (!g_llmLib.LoadLlmSetting(setting, LlmApiPurpose::FastApply_Dedicated, ""))
	{
		_status = TaskStatus::Failure;
		_collectedResult = "";
		_collectedResult = _collectedResult + FILE_EDIT_RESULT_ERROR_PREFIX + "FastApply tool not available!";
		_SaveResult();

		return;
	}

	// 创建请求
	LlmSessionRequest request;
	request.AddUserMessage(message.c_str());
	request.isStreaming = false;

	// 发送请求到LLM
	if (!_llmChat->Request(request, setting))
	{
		_status = TaskStatus::Failure;
		return;
	}

	_hasStartedRequest = true;
	_collectedResult.clear();

}

void CChatTask_FastApply::Update()
{
	if (!_hasStartedRequest || !_llmChat)
		return;

	// 检查LLM会话状态
	if (_llmChat->HasActiveSession())
	{
		LlmSessionOutput output;
		
		if (_llmChat->Process(output, _requestInterrupt))
		{
			// 收集所有输出内容，参考CChatProcessor_CollectFileEdited的做法
			if (!output.content.empty())
			{
				_collectedResult += output.content;
			}

			// 检查会话是否完成
			if (output.isCompleted)
			{

				if (output.hasError && !output.errorMessage.empty())
				{
					// 有错误，设置为失败状态
					_status = TaskStatus::Failure;
					_collectedResult = FILE_EDIT_RESULT_ERROR_PREFIX;
					_collectedResult += output.errorMessage;
					_SaveResult();
				}
				else if (!_requestInterrupt)
				{
					// 成功完成，保存文件
					_SaveResult();
				}
				else
				{
					// 被中断
					_status = TaskStatus::Failure;
					_collectedResult = _collectedResult + FILE_EDIT_RESULT_ERROR_PREFIX + "Interrupted by user!";
					_SaveResult();
				}
			}
		}
	}
	else if (_hasStartedRequest)
	{
		always_assert(false);
		_status = TaskStatus::Failure;
	}
}

void CChatTask_FastApply::Interrupt()
{
	// 设置中断请求标志
	_requestInterrupt = true;
}



void CChatTask_FastApply::_SaveResult()
{
	if (_collectedResult.empty())
	{
		_status = TaskStatus::Failure;
		return;
	}

	if (_SaveFileEditResult(_filePath, _originalFileContent, _collectedResult,_originalFileCodingFmt, _fileEditId))
	{
		_status = TaskStatus::Success;
		return;
	}

	_status = TaskStatus::Failure;
}
