#include "stdh.h"
#include "LlmProcessors.h"

#include "stringparser/stringparser.h"
#include "encrypt/encrypt.h"
#include <sstream>  // 用于std::istringstream
#include <fstream>  // 用于std::ifstream, std::ofstream
#include <regex>    // 用于std::regex, std::smatch, std::regex_search
#include <ios>      // 用于std::ios::cur
#include <vector>   // 用于std::vector
#include <string>
#include <iostream>

#include "UnifiedDiff.h"

int GetProcessStart(const std::string& workingStr, const std::string& expectTag)
{
	size_t startPos = workingStr.find(expectTag);
	if (startPos != std::string::npos)
	{
		return (int)startPos;
	}

	if (workingStr.find(std::string("``")) != std::string::npos)
	{
		int v = 0;
		v++;
	}

	// 如果没有找到完整的标记，检查是否有部分标记
	// 在字符串末尾
	const int diffMarkLen = (int)expectTag.length();

	// 检查 workingStr 是否以 expectTag 的部分开始
	for (int i = diffMarkLen - 1; i >= 1; i--)
	{
		// 创建 expectTag 的前缀
		std::string prefix = expectTag.substr(0, i);

		// 检查 workingStr 是否以该前缀结尾
		if (workingStr.length() >= prefix.length())
		{
			size_t tailPos = workingStr.length() - prefix.length();
			if (workingStr.substr(tailPos) == prefix)
			{
				// 找到了一个可能的不完整expectTag标记
				return (int)tailPos;
			}
		}
	}

	// 没有找到任何标记
	return -1;
}

//如果能够找到markdown的起始标记和终止标记,则返回true,并且isFullResult返回true,content返回标记间的内容,并从workingStr里去除整个markdown块
//如果能够找到markdown的起始标记但找不到终止标记,则返回true,并且isFullResult返回false,content返回标记间(不完整)的内容,但不从workingStr里去除整个markdown块
//如果找不到markdown的起始标记,返回false,并且isFullResult返回false
bool CullMarkDown(std::string& workingStr, const std::string &startMark , const std::string& endMark,std::string &content,bool & isFullResult)
{
	// 初始化isFullResult
	isFullResult = false;
	content.clear();
	
	// 检查是否有完整的开始标记
	size_t startPos = workingStr.find(startMark);
	if (startPos == std::string::npos)
	{
		// 找不到起始标记，返回false，isFullResult已经设为false
		return false;
	}

	// 找到了起始标记，寻找起始标记后的换行符
	size_t startMarkerEnd = workingStr.find("\n", startPos);
	if (startMarkerEnd == std::string::npos)
	{
		// 没有找到换行符，起始标记不完整
		// 返回false，isFullResult已经设为false
		return false;
	}

	// 跳过起始行回车
	startMarkerEnd++;

	// 在起始标记后寻找结束标记
	size_t endPos = workingStr.find(endMark, startMarkerEnd);
	if (endPos == std::string::npos)
	{
		// 找到了起始标记但没有找到终止标记
		// 提取从起始标记后到字符串结尾的内容
		content = workingStr.substr(startMarkerEnd);
		// 返回true，但isFullResult为false（表示不完整）
		// 注意：不从workingStr中删除任何内容
		return true;
	}

	// 找到了起始标记和终止标记
	// 提取内容（不含标记）
	content = workingStr.substr(startMarkerEnd, endPos - startMarkerEnd);

	// 计算结束标记后的位置
	size_t endPosAfterMarker = endPos + endMark.length(); 

	// 如果结束标记后面有换行符，也包含在删除范围内
	if (endPosAfterMarker < workingStr.length() && workingStr[endPosAfterMarker] == '\n')
	{
		endPosAfterMarker++;
	}

	// 只删除从开始标记到结束标记之间的内容（包括标记），保留前后的文本
	workingStr.erase(startPos, endPosAfterMarker - startPos);

	// 设置isFullResult为true，表示找到了完整的标记对
	isFullResult = true;
	return true;
}


//从workingStr的第一行以startMark开始,并且startMark之后紧跟一个":",则把这一行":"之后的部分返回到content中,并且从workingStr中删除这一行
bool CullHeadMarkLine(std::string& workingStr, const std::string& startMark, std::string& content)
{
	// 处理空字符串情况
	if (workingStr.empty() || startMark.empty())
	{
		return false;
	}
	
	// 查找第一行结束位置
	size_t endOfLine = workingStr.find('\n');
	std::string firstLine;
	
	if (endOfLine != std::string::npos)
	{
		// 有换行符，提取第一行（不包括换行符）
		firstLine = workingStr.substr(0, endOfLine);
	}
	else
	{
		// 没有换行符,失败
		return false;
	}
	
	// 检查第一行是否以startMark开头
	if (firstLine.find(startMark) != 0)
	{
		return false;
	}
	
	// 检查startMark后面是否跟着冒号
	size_t markEnd = startMark.length();
	if (markEnd >= firstLine.length() || firstLine[markEnd] != ':')
	{
		return false;
	}
	
	// 提取冒号后面的内容到content
	content = firstLine.substr(markEnd + 1);
	
	// 删除首行空格（如果有）
	if (!content.empty() && content[0] == ' ')
	{
		content.erase(0, 1);
	}
	
	// 从原字符串中删除这一行（包括换行符）
	if (endOfLine < workingStr.length())
	{
		// 如果有换行符，则删除包括换行符在内的整行
		workingStr.erase(0, endOfLine + 1);
	}
	else
	{
		// 如果没有换行符，则清空整个字符串
		workingStr.clear();
	}
	
	return true;
}

//为fileContent的每一行加一个行号
void AddLineNumber(const std::string& fileContent, std::string& fileContentWithLineNumber)
{
	std::istringstream stream(fileContent);
	std::string line;
	int lineNumber = 1;
	fileContentWithLineNumber.clear();
	
	// 计算行数，用于确定行号的宽度
	int totalLines = 0;
	size_t pos = 0;
	while ((pos = fileContent.find('\n', pos)) != std::string::npos)
	{
		++totalLines;
		++pos;
	}
	if (fileContent.empty() || fileContent.back() != '\n')
	{
		++totalLines; // 如果最后一行没有换行符，也计算在内
	}
	
	// 计算行号需要的宽度（数字位数）
	int width = 1;
	int temp = totalLines;
	while (temp >= 10)
	{
		++width;
		temp /= 10;
	}
	
	// 为每一行添加行号
	while (std::getline(stream, line))
	{
		std::string formattedLine;
		char buffer[32];
//		sprintf(buffer, "%*d | ", width, lineNumber);
		sprintf(buffer, "%d\t\t", lineNumber);

		formattedLine = buffer + line + "\n";
		fileContentWithLineNumber += formattedLine;
		++lineNumber;
	}
	
	// 如果原始内容最后没有换行符，删除最后添加的换行符
	if (!fileContent.empty() && fileContent.back() != '\n' && !fileContentWithLineNumber.empty())
	{
		fileContentWithLineNumber.pop_back();
	}
}


//为fileContent的每一行在末尾加一个行号
void AddLineSuffix(const std::string& fileContent, std::string& fileContentWithLineNumber)
{
	std::istringstream stream(fileContent);
	std::string line;
	int lineNumber = 1;
	fileContentWithLineNumber.clear();

	// 计算行数，用于确定行号的宽度
	int totalLines = 0;
	size_t pos = 0;
	while ((pos = fileContent.find('\n', pos)) != std::string::npos)
	{
		++totalLines;
		++pos;
	}
	if (fileContent.empty() || fileContent.back() != '\n')
	{
		++totalLines; // 如果最后一行没有换行符，也计算在内
	}

	// 为每一行添加行号
	while (std::getline(stream, line))
	{
		std::string formattedLine;
		char buffer[32];
		//		sprintf(buffer, "%*d | ", width, lineNumber);
//		sprintf(buffer, "//%08d\n", lineNumber);
		sprintf(buffer, "//0x%08X\n", encrypt(lineNumber));

		formattedLine = line + buffer;
		fileContentWithLineNumber += formattedLine;
		++lineNumber;
	}

	// 如果原始内容最后没有换行符，删除最后添加的换行符
	if (!fileContent.empty() && fileContent.back() != '\n' && !fileContentWithLineNumber.empty())
	{
		fileContentWithLineNumber.pop_back();
	}
}


//////////////////////////////////////////////////////////////////////////
//CChatProcessors
void CLlmProcessors::_PrepareProcessors(std::vector<CLlmProcessor*>& processors)
{
	std::vector<CLlmProcessor*> enabledProcessors;
	enabledProcessors.reserve(processors.size()); 

	for (size_t i = 0; i < processors.size(); ++i)
	{
		CLlmProcessor* p = processors[i];
		if (p != nullptr)
		{
			const char* name = p->GetName();
			if (name != nullptr)
			{
				if (_enables.count(name) > 0)
				{
					enabledProcessors.push_back(p);
				}
			}
		}
	}
	processors.swap(enabledProcessors);
}

CLlmProcessor* CLlmProcessors::FindProcessor(const char* name)
{
	if (name == nullptr)
	{
		return nullptr;
	}

	std::vector<CLlmProcessor*> processors;
	_GetProcessors(processors); 

	for (size_t i = 0; i < processors.size(); ++i)
	{
		CLlmProcessor* p = processors[i];
		if (p != nullptr)
		{
			const char* processorName = p->GetName();
			if (processorName != nullptr && strcmp(processorName, name) == 0)
			{
				return p;
			}
		}
	}

	return nullptr; 
}

void CLlmProcessors::Start(const std::vector<std::string>& enabledProcessorNames)
{
	_workingStr.clear();
	_GetResult().Clear();

	_enables.clear();
	for (const std::string& name : enabledProcessorNames)
	{
		_enables.insert(name);
	}

	std::vector<CLlmProcessor*>processors;
	_GetProcessors(processors);
	_PrepareProcessors(processors);

	for (int i = 0;i < processors.size();i++)
		processors[i]->Start();
}

void CLlmProcessors::Stop()
{
	std::vector<CLlmProcessor*>processors;
	_GetProcessors(processors);
	_PrepareProcessors(processors);

	_GetResult().outputDelta += _workingStr;
	_GetResult().outputTotal += _workingStr;

	for (int i = 0;i < processors.size();i++)
		processors[i]->Stop(_GetContext(), _GetResult());
}

void CLlmProcessors::Interrupt()
{
	std::vector<CLlmProcessor*>processors;
	_GetProcessors(processors);
	_PrepareProcessors(processors);

	_GetResult().outputDelta += _workingStr;
	_GetResult().outputTotal += _workingStr;

	for (int i = 0;i < processors.size();i++)
		processors[i]->Interrupt(_GetContext(), _GetResult());
}


void CLlmProcessors::Add(const char* str)
{
	std::vector<CLlmProcessor*>processors;
	_GetProcessors(processors);
	_PrepareProcessors(processors);

	_workingStr += str;

	int minProcessStart = 10000000;

	for (int i = 0;i < processors.size();i++)
	{
		int processStart = processors[i]->GetProcessStart(_workingStr,_GetContext());
		if (processStart >= 0)//valid process start
		{
			if (processStart < minProcessStart)
				minProcessStart = processStart;
		}
	}

	std::string output;
	if (minProcessStart < _workingStr.length())
	{
		output = LEFT_STRING(_workingStr, minProcessStart);
		_workingStr = _workingStr.c_str() + minProcessStart;
	}
	else
	{
		output = _workingStr;
		_workingStr.clear();
	}

	_GetResult().outputDelta += output;
	_GetResult().outputTotal += output;
	_GetResult().raw += str;

	for (int i = 0;i < processors.size();i++)
	{
		while (processors[i]->Process(_workingStr, _GetContext(), _GetResult()));
	}
}

void CLlmProcessors::FetchDelta(std::string& delta)
{
	delta = _GetResult().outputDelta;
	_GetResult().outputDelta = "";
}

