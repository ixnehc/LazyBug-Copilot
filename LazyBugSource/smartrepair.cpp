#include "stdh.h"
#include "SmartRepair.h"

#include "stringparser/stringparser.h"

#include "CppSymbol.h"

#include "TreeSitterFiles.h"

#include "CodingHistory.h"

#include "timer/timer.h"

extern CTreeSitterFiles g_tsFiles;

//  
// //为fileContent的每一行在末尾加一个行号
// void AddLineSuffixForRepairing(const std::string& fileContent, int startLine,int endLine, std::string& fileContentWithLineNumber)
// {
// 	std::istringstream stream(fileContent);
// 	std::string line;
// 	int lineNumber = 0;
// 	fileContentWithLineNumber.clear();
// 
// 	// 计算行数，用于确定行号的宽度
// 	int totalLines = 0;
// 	size_t pos = 0;
// 	while ((pos = fileContent.find('\n', pos)) != std::string::npos)
// 	{
// 		++totalLines;
// 		++pos;
// 	}
// 	if (fileContent.empty() || fileContent.back() != '\n')
// 	{
// 		++totalLines; // 如果最后一行没有换行符，也计算在内
// 	}
// 
// 	// 为每一行添加行号
// 	while (std::getline(stream, line))
// 	{
// 		std::string formattedLine;
// 		char buffer[32];
// 		if ((lineNumber >= startLine) && (lineNumber < endLine))
// 			line = line + u8"//#此行需要补全#\n";
// 
// 		DWORD suffix = encrypt(lineNumber);
// 		sprintf(buffer, "//0x%08X\n", suffix);
// 		formattedLine = line + buffer;
// 		fileContentWithLineNumber += formattedLine;
// 		++lineNumber;
// 	}
// 
// 	// 如果原始内容最后没有换行符，删除最后添加的换行符
// 	if (!fileContent.empty() && fileContent.back() != '\n' && !fileContentWithLineNumber.empty())
// 	{
// 		fileContentWithLineNumber.pop_back();
// 	}
// }

void TrimSameLineFrom2End(const char* str1, const char* str2, std::string& trimmed1, std::string& trimmed2, int* prefixTrimCount = nullptr, int* suffixTrimCount = nullptr)
{
	std::vector<std::string> lines1,lines2;
	SplitLines(str1 ? str1 : "",lines1);
	SplitLines(str2 ? str2 : "",lines2);

	size_t prefix = 0;
	while (prefix < lines1.size() && prefix < lines2.size() && lines1[prefix] == lines2[prefix])
		++prefix;

	size_t suffix = 0;
	while (suffix < lines1.size() - prefix &&
		suffix < lines2.size() - prefix &&
		lines1[lines1.size() - 1 - suffix] == lines2[lines2.size() - 1 - suffix])
		++suffix;

	// 返回trim掉的行数
	if (prefixTrimCount)
		*prefixTrimCount = (int)prefix;
	if (suffixTrimCount)
		*suffixTrimCount = (int)suffix;

	std::vector<std::string> mid1(
		lines1.begin() + prefix,
		lines1.end() - suffix
	);
	std::vector<std::string> mid2(
		lines2.begin() + prefix,
		lines2.end() - suffix
	);

	LinkStringBy("\n", trimmed1, &mid1);
	LinkStringBy("\n", trimmed2, &mid2);
}

void CSmartRepair::_ParseDiffString(const char* diffStr, CodeComparingChars& comparingChars)
{
	comparingChars.content.clear();
	comparingChars.charTypes.clear();

	if (!diffStr || strlen(diffStr) == 0)
		return;

	std::string diffString(diffStr);

	diffString = ReplaceString(diffString.c_str(), "###Cursor###", "");
	diffString = ReplaceString(diffString.c_str(), "###Cursor###", "");

	// 查找分隔符
	const std::string oldMarker = "###old lines###";
	const std::string newMarker = "###new lines###";

	size_t oldPos = diffString.find(oldMarker);
	size_t newPos = diffString.find(newMarker);

	if (oldPos == std::string::npos || newPos == std::string::npos)
	{
		// 如果没有找到标记，将整个内容作为新代码
		comparingChars.content = utf8_to_widechar(diffString);
		comparingChars.charTypes.assign(comparingChars.content.size(), CodeComparingChars::NewCode);
		return;
	}

	// 提取旧代码和新代码
	std::string oldCode, newCode;

	if (oldPos < newPos)
	{
		// ###old lines### 在前
		size_t oldStart = oldPos + oldMarker.length();
		size_t oldEnd = newPos;
		oldCode = diffString.substr(oldStart, oldEnd - oldStart);

		size_t newStart = newPos + newMarker.length();
		newCode = diffString.substr(newStart);
	}
	else
	{
		// ###new lines### 在前
		size_t newStart = newPos + newMarker.length();
		size_t newEnd = oldPos;
		newCode = diffString.substr(newStart, newEnd - newStart);

		size_t oldStart = oldPos + oldMarker.length();
		oldCode = diffString.substr(oldStart);
	}

	// 去除首尾空白
	auto trim = [](std::string& str) {
		size_t start = str.find_first_not_of(" \t\r\n");
		if (start == std::string::npos) {
			str.clear();
			return;
		}
		size_t end = str.find_last_not_of(" \t\r\n");
		str = str.substr(start, end - start + 1);
	};

	trim(oldCode);
	trim(newCode);

	// 使用CodeDiff进行字符级比较
	MakeCodeComparing_Chars(oldCode, newCode, comparingChars);
}

void MakeRepairRequestMessage(const RepairRequest& context, std::string& message)
{
	// 	message = "";
	// 
	// 	std::string filePath;
	// 	int startLine, endLine;
	// 	if (TRUE)
	// 	{
	// 		filePath = GetFilePath();
	// 		CScintillaWnd* wnd = GetScintillaWnd();
	// 		int startPos = wnd->GetSelectionStart();
	// 		int endPos = wnd->GetSelectionEnd();
	// 		startLine = wnd->LineFromPosition(startPos);
	// 		endLine = wnd->LineFromPosition(endPos);
	// 		endLine++;
	// 
	// 		if ((startLine < 0) || (endLine < 0))
	// 			return;
	// 	}
	// 
	// 	std::string fileContentWithSuffix;
	// 	if (TRUE)
	// 	{
	// 		std::string fileContent = GetContent();
	// 
	// 		AddLineSuffixForRepairing(fileContent, startLine, endLine, fileContentWithSuffix);
	// 	}
	// 
	// 	// 准备发送给AI的消息
	// 	message = u8"请对以下文件内标记为需要补全的部分进行代码补全:\n";
	// 	message += fileContentWithSuffix;
}

bool MakeRepairRequestMessage_v2(const RepairRequest& context, std::string& message)
{
	message = "";

	// 	if (!context.sel.empty())
	// 		return false;
	// 
	// 	// 准备发送给AI的消息
	// 	message = context.prefix + "###Cursor###" + context.suffix;

	return true;
}

bool MakeRepairRequestMessage_fim(const RepairRequest& context, std::string& message)
{
	return false;
	// 	message = "";
	// 	CChildView* pChildView = _GetActiveView();
	// 	if (!pChildView)
	// 		return false;
	// 
	// 	std::string prefix, suffix, sel;
	// 	SplitScintillaWndContentAtCursor(pChildView->GetScintillaWnd(), prefix, suffix, sel);
	// 
	// 	if (sel.empty())
	// 		return false;
	// 
	// 	// 准备发送给AI的消息
	// 	message = prefix + "###start###" + sel + "###end###" + suffix;
	//	return true;
}

// bool MakeRepairRequestMessage_rewrite(const RepairRequest& request, std::string& userMessage, int& rewriteStartLine, int& rewriteEndLine)
// {
// 	userMessage = "";
// 
// 	userMessage =
// 		"### Instruction:\n"
// 		"You are a code completion assistant and your task is to analyze user edits and then rewrite an excerpt that the user provides, suggesting the appropriate edits within the excerpt, taking into account the cursor location.\n"
// 		"### User Edits:\n";
// 
// 	userMessage += "### User Excerpt:\n";
// 
// 	g_tsFiles.UpdateFile(request.filePath, request.fileContent);
// 
// 	CTreeSitterFiles::SCharRange range;
// 	range.nStartLine = request.cursorLine;
// 	range.nStartColumn = request.cursorColumn;
// 	range.nEndLine = range.nStartLine;
// 	range.nEndColumn = range.nStartColumn;
// 
// 	range.nStartColumn = 0;
// 	// 	if (range.nStartLine > 0)
// 	// 		range.nStartLine--;
// 
// 	int editableStartLine, editableEndLine;
// 	int contextStartLine, contextEndLine;
// 
// 	if (false)
// 	{
// 		CTreeSitterFiles::SCharRange editableRange;
// 		if (!g_tsFiles.ExpandRange(request.filePath, range, 1000, editableRange))
// 			return false;
// 
// 		CTreeSitterFiles::SCharRange contextRange;
// 		if (!g_tsFiles.ExpandRange(request.filePath, range, 5000, contextRange))
// 			return false;
// 
// 		editableStartLine = editableRange.nStartLine;
// 		editableEndLine = editableRange.nEndLine;
// 		if (editableRange.nEndColumn > 0)
// 			editableEndLine++;
// 		i_math::clamp_i(editableEndLine, 0, request.fileLines.size());
// 
// 		contextStartLine = contextRange.nStartLine;
// 		contextEndLine = contextRange.nEndLine;
// 		if (contextRange.nEndColumn > 0)
// 			contextEndLine++;
// 		i_math::clamp_i(contextEndLine, 0, request.fileLines.size());
// 	}
// 	else
// 	{
// 		editableEndLine = i_math::clamp_i(request.cursorLine + 10, 0, request.fileLines.size());
// 		editableStartLine = i_math::clamp_i(request.cursorLine - 10, 0, request.fileLines.size());
// 		contextEndLine = i_math::clamp_i(request.cursorLine + 600, 0, request.fileLines.size());
// 		contextStartLine = i_math::clamp_i(request.cursorLine - 600, 0, request.fileLines.size());
// 	}
// 
// 	userMessage += "```";
// 	userMessage += request.fileName + "\n";
// 
// 	for (int i = contextStartLine;i < editableStartLine;i++)
// 	{
// 		userMessage += request.fileLines[i];
// 		userMessage += "\n";
// 	}
// 	userMessage += "<|editable_region_start|>\n";
// 	for (int i = editableStartLine;i < editableEndLine;i++)
// 	{
// 		if (i != request.cursorLine)
// 			userMessage += request.fileLines[i];
// 		else
// 		{
// 			userMessage += request.cursorLinePrefix;
// 			userMessage += "<|user_cursor_is_here|>";
// 			userMessage += request.cursorLineSuffix;
// 		}
// 		userMessage += "\n";
// 	}
// 	userMessage += "<|editable_region_end|>\n";
// 
// 	for (int i = editableEndLine;i < contextEndLine;i++)
// 	{
// 		userMessage += request.fileLines[i];
// 		userMessage += "\n";
// 	}
// 	userMessage += "```\n";
// 
// 	userMessage += "### Response:\n";
// 	rewriteStartLine = editableStartLine;
// 	rewriteEndLine = editableEndLine;
// 
// 	return true;
// }

bool MakeRepairRequestMessage_rewrite2(const RepairRequest& request, const char *refStr,std::string& userMessage, int& rewriteStartLine, int& rewriteEndLine)
{
	userMessage = "";

	userMessage =
		"### Instruction:\n"
		"You are a code completion assistant and your task is to analyze user edits and then rewrite an excerpt that the user provides, suggesting the appropriate edits within the excerpt, taking into account the cursor location.\n";

	if (true)
	{
		if (refStr[0])
		{
			userMessage += "### Relevant context codes:\n";
			userMessage += refStr;
		}
	}

	if (true)
	{
		std::string codingHistory;
		extern CCodingHistory g_codingHistory;
		g_codingHistory.Dump(codingHistory, 2048);
		if (!codingHistory.empty())
		{
			userMessage += "### Recent user edits:\n";
			userMessage += codingHistory;
		}
	}


	userMessage += "### User Excerpt:\n";

	int editableStartLine, editableEndLine;
	int contextStartLine, contextEndLine;

	if(true)
	{
		editableEndLine = i_math::clamp_i(request.cursorLine + 10, 0, request.fileLines.size());
		editableStartLine = i_math::clamp_i(request.cursorLine - 10, 0, request.fileLines.size());
		contextEndLine = i_math::clamp_i(request.cursorLine + 600, 0, request.fileLines.size());
		contextStartLine = i_math::clamp_i(request.cursorLine - 600, 0, request.fileLines.size());
	}

	userMessage += "```";
	userMessage += request.fileName + "\n";

	for (int i = contextStartLine;i < editableStartLine;i++)
	{
		userMessage += request.fileLines[i];
		userMessage += "\n";
	}
	userMessage += "<|editable_region_start|>\n";
	for (int i = editableStartLine;i < editableEndLine;i++)
	{
		if (i != request.cursorLine)
			userMessage += request.fileLines[i];
		else
		{
			userMessage += request.cursorLinePrefix;
			userMessage += "<|user_cursor_is_here|>";
			userMessage += request.cursorLineSuffix;
		}
		userMessage += "\n";
	}
	userMessage += "<|editable_region_end|>\n";

	for (int i = editableEndLine;i < contextEndLine;i++)
	{
		userMessage += request.fileLines[i];
		userMessage += "\n";
	}
	userMessage += "```\n";

	rewriteStartLine = editableStartLine;
	rewriteEndLine = editableEndLine;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//CSmartRepair
CSmartRepair::CSmartRepair()
{
}

CSmartRepair::~CSmartRepair()
{
}

void CSmartRepair::Init(CWnd* pMainWnd)
{
	_resultWnd.Create(CRect(0, 0, 1, 1), pMainWnd, 10);
	_resultWnd.ShowWindow(SW_HIDE);

	// 初始化AI聊天
	_InitLlmChat();

	_refCollector.Init();
}

void CSmartRepair::Clear()
{
	_llmChat.Clear();

	_refCollector.Clear();
}


void CSmartRepair::_InitLlmChat()
{
	// 初始化AI聊天设置
	g_llmLib.LoadLlmSetting(_defaultSettings, LlmApiRole::Auxiliary, "");

	_llmChat.Init();
}

void CSmartRepair::_ProcessLlmChat()
{
	// 检查实例是否有活动会话
	if (!_llmChat.HasActiveSession())
		return;
	
	// 处理会话输出
	LlmSessionOutput output;
	if (_llmChat.Process(output))
	{
		// 如果会话完成
		if (output.isCompleted)
		{
			// 只处理未丢弃的实例的完成事件
			if (output.hasError && !output.errorMessage.empty())
			{
			}
			else
			{
				SmartRepairSessionID sessionId = _llmChat.FetchSessionID();
				if (sessionId == _repairRequest.sessionId)
				{
					_suggest = output.fullContent;
					_trigger.SetAIResult(sessionId);
				}
			}
		}
	}
}

void CSmartRepair::_ProcessRefCollect()
{
	CSmartRepairRefCollector::Result result;
	if (_refCollector.FetchLatestResult(result))
	{
		if (result.sessionId == _repairRequest.sessionId)
		{
			_collectedRef = std::move(result);
			_trigger.NotifyContextPrepared(result.sessionId);
		}
	}
}


void CSmartRepair::Update()
{
	_ProcessRefCollect();

	_ProcessLlmChat();

}

void CSmartRepair::_Reset()
{
	_repairRequest.Clear();

	_refCollector.Interrupt();
	_collectedRef.Clear();

	// 中断当前chat
	if (_llmChat.HasActiveSession())
	{
		LlmSessionOutput output;
		_llmChat.Process(output, true);
		_llmChat.FetchSessionID();
	}
	_repairTarget.Clear();
	_suggest.clear();
	
	_resultWnd.Hide();
}


void CSmartRepair::StartPrepareContext(RepairRequest &request0)
{
	_Reset();

	_repairRequest = std::move(request0);

	CSmartRepairRefCollector::Request request;
	request.filePath = _repairRequest.filePath;
	request.fileContent = _repairRequest.fileContent;
	request.line = _repairRequest.cursorLine;
	request.sessionId = _repairRequest.sessionId;
	_refCollector.RequestCollect(request);

}

void CSmartRepair::StartAIRequest()
{
	if (!_defaultSettings.IsValid())
		g_llmLib.LoadLlmSetting(_defaultSettings, LlmApiRole::Auxiliary, "");

	std::string userMessage;
	int rewriteStartLine, rewriteEndLine;
	//XXXXX:RepairSolution
// 	if (!MakeRepairRequestMessage_v2(context,message))
// 		return;
// 	if (!MakeRepairRequestMessage_rewrite(_repairRequest, userMessage, rewriteStartLine,rewriteEndLine))
// 		return;
	if (!MakeRepairRequestMessage_rewrite2(_repairRequest, _collectedRef.GetSnippetsStr(),userMessage, rewriteStartLine, rewriteEndLine))
		return;

	_repairTarget.rewriteStartLine = rewriteStartLine;
	_repairTarget.rewriteEndLine = rewriteEndLine;
	_repairTarget.debugUserMessage = userMessage;
	_repairTarget.debugStartTime = GetAbsTick();

	// 创建请求
	LlmSessionRequest request;
//	request.SetPrompt(userMessage.c_str());
 	request.AddUserMessage(userMessage.c_str());
	request.isStreaming = false;

	// 发送请求
	_llmChat.SetSessionID(_repairRequest.sessionId);
	_llmChat.Request(request, _defaultSettings);
}

void BuildOldChars(const CodeComparingChars& comparingChars, int oldCodeLineStart, CodeComparingOldChars& oldChars)
{
	oldChars.lines.clear();

	if (comparingChars.content.empty() || comparingChars.charTypes.empty())
		return;

	// 当前行号和字符索引
	int currentLine = oldCodeLineStart;
	std::vector<int> currentLineIndices;

	// 遍历comparingChars中的每个字符
	for (size_t i = 0; i < comparingChars.content.size() && i < comparingChars.charTypes.size(); ++i)
	{
		wchar_t ch = comparingChars.content[i];
		CodeComparingChars::CharType charType = comparingChars.charTypes[i];

		// 如果是换行符，处理当前行
		if (ch == L'\n')
		{
			// 如果当前行有old code字符，添加到结果中
			if (!currentLineIndices.empty())
			{
				CodeComparingOldChars::Line line;
				line.line = currentLine;
				line.indices = std::move(currentLineIndices);
				oldChars.lines.push_back(std::move(line));
				currentLineIndices.clear();
			}
			currentLine++;
		}
		else
		{
			// 如果是old code字符，记录其在当前行中的位置
			if (charType == CodeComparingChars::OldCode)
			{
				// 计算字符在当前行中的位置
				int charIndexInLine = 0;
				// 从当前位置向前查找，计算在当前行中的位置
				for (int j = (int)i - 1; j >= 0; --j)
				{
					if (comparingChars.content[j] == L'\n')
						break;
					charIndexInLine++;
				}
				currentLineIndices.push_back(charIndexInLine);
			}
		}
	}

	// 处理最后一行（如果没有以换行符结尾）
	if (!currentLineIndices.empty())
	{
		CodeComparingOldChars::Line line;
		line.line = currentLine;
		line.indices = std::move(currentLineIndices);
		oldChars.lines.push_back(std::move(line));
	}

	// 对每行的字符索引进行排序（从小到大）
	for (auto& line : oldChars.lines)
	{
		std::sort(line.indices.begin(), line.indices.end());
	}
}

void CSmartRepair::ShowSuggestion(const RECT& focusRect)
{
	if (!_repairTarget.IsValid())
		return;

	_repairTarget.debugDur = GetAbsTick() - _repairTarget.debugStartTime;

	std::string newCode = CullStringChunk(_suggest.c_str(), "<|editable_region_start|>", "<|editable_region_end|>");
	ReplaceString(newCode.c_str(), "<|user_cursor_is_here|>", "");

	if (newCode.back() == '\n')
		newCode.pop_back();

	if (newCode.front() == '\n')
		newCode = newCode.c_str() + 1;

	std::string oldCode;
	for (int i = _repairTarget.rewriteStartLine;i < _repairTarget.rewriteEndLine;i++)
	{
		if (i > _repairTarget.rewriteStartLine)
			oldCode += "\n";
		oldCode += _repairRequest.fileLines[i];
	}

	std::string oldCodeTrimmed, newCodeTrimmed;
	int prefixTrimCount = 0, suffixTrimCount = 0;
	TrimSameLineFrom2End(oldCode.c_str(), newCode.c_str(), oldCodeTrimmed, newCodeTrimmed, &prefixTrimCount, &suffixTrimCount);

// 	oldCodeTrimmed=
// // "\t\t_clssRecord=clss;\n"
// // "\n"
// // "\t\t//Ã•Ã’ÂµÂ½ÃƒÃ»ÃŽÂªNameÂµÃ„Ã„Ã‡Â¸Ã¶Elem\n"
// "\t\tCRecord* t = (CRecord*)_clssRecord->New();\n"
// "\t\tGObjBase* gobj = t->GetGObj();\n"
// "\t\tGElemBase* elem = gobj->GetElems();\n"
// "\n"
// "\t\tstd::string s = \"Name\";\n"
// "\t\twhile (elem)";
// 
// 	newCodeTrimmed=
// // "\t\t\tZero();\n"
// // "\t\t\t_clssRecord=clss;\n";
// 
// // "\t\t_clssRecord=clss;\n"
// // "\n"
// // "\t\t//Ã•Ã’ÂµÂ½ÃƒÃ»ÃŽÂªNameÂµÃ„Ã„Ã‡Â¸Ã¶Elem\n"
// "\t\tTCRecord* t = (CRecord*)_clssRecord->New();\n"
// "\t\tGObjBase* gobj = t->GetGObj();\n"
// "\t\tGElemBase* elem = gobj->GetElems();\n"
// "\n"
// "\t\tstd::string s = \"Name\";\n"
// "\t\twhile (elem)";


	// 解析diffStr内容，生成CodeComparingChars
	CodeComparingChars comparingChars;

	MakeCodeComparing_Chars2(oldCodeTrimmed, newCodeTrimmed, comparingChars);

	if (comparingChars.content.empty())
		return;

	// 生成_oldChars，记录old code中需要删除的字符位置
	int oldCodeLineStart = _repairTarget.rewriteStartLine + prefixTrimCount;
	BuildOldChars(comparingChars, oldCodeLineStart, _oldChars);

	_resultWnd.Show(comparingChars, focusRect);


}

void CSmartRepair::HideSuggestion()
{
	_resultWnd.Hide();
}
