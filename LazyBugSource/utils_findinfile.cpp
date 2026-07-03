#include "stdh.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cctype>
#include <mutex>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <direct.h>
#include <tlhelp32.h>
#include <process.h>
#include "Utils.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <sys/stat.h>
#include <string>

#include "utils_findinfile.h"

// 引入 local_to_utf8 函数声明
extern std::string local_to_utf8(const std::string& ansi_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);

namespace Utils
{

extern bool is_valid_utf8(const std::string& s);

bool ParseRipGrepLine(const std::string& line, FindInFileResults& results, FindInFileFilter filterCallback)
{
	// ripgrep 输出格式：文件路径:行号:行内容
	// 例如：C:\test.cpp:10:void test() { return; }
	// 需要处理 Windows 路径中的冒号（如 C:\）

	// 查找第一个冒号，但要跳过 Windows 驱动器号的冒号
	int firstColon = -1;
	int searchPos = 0;

	while (true) {
		int colonPos = StringFind(line.c_str() + searchPos, ':');
		if (colonPos == -1) break;

		colonPos += searchPos; // 转换为绝对位置

		// 检查冒号后面是不是反斜杠（Windows 路径格式）
		if (colonPos + 1 < (int)line.length() && line[colonPos + 1] == '\\') {
			// 这是 Windows 驱动器号的冒号，继续查找下一个冒号
			searchPos = colonPos + 2;
		}
		else {
			// 找到真正的分隔冒号
			firstColon = colonPos;
			break;
		}
	}

	if (firstColon == -1)
		return false;

	// 查找第二个冒号
	int secondColon = StringFind(line.c_str() + firstColon + 1, ':');
	if (secondColon == -1)
		return false;
	secondColon += firstColon + 1; // 转换为绝对位置

	// 提取文件路径（第一个冒号前）
	std::string filePath = line.substr(0, firstColon);
	RemoveHeadBlank(filePath);
	RemoveTailBlank(filePath);

	// 提取行号（第一个冒号后，第二个冒号前）
	std::string lineNumStr = line.substr(firstColon + 1, secondColon - firstColon - 1);
	RemoveHeadBlank(lineNumStr);
	RemoveTailBlank(lineNumStr);
	int lineNumber = IntFromString(lineNumStr.c_str());

	// 提取行内容（第二个冒号后）
	std::string lineContent = line.substr(secondColon + 1);
	RemoveHeadBlank(lineContent);
	RemoveTailBlank(lineContent);

	// 如果不是有效的 UTF-8，转换为 UTF-8
	if (!is_valid_utf8(lineContent))
		lineContent = local_to_utf8(lineContent);

	// 检查是否需要过滤该文件
	if (filterCallback && filterCallback(filePath.c_str()))
	{
		return false;
	}

	// 添加到结果
	results.AddResult(filePath, lineNumber, lineContent);
	return true;
}

// 解析包含多行结果的字符串
static void ParseRipGrepLines(const std::string& multiLineString, FindInFileResults& results, int& resultCount, int maxResults, FindInFileFilter filterCallback)
{
	if (multiLineString.empty())
		return;

	std::vector<std::string> lines;
	SplitLines(multiLineString, lines);

	for (const auto& line : lines)
	{
		if (IsBlankString(line.c_str()))
			continue;

		// 使用 ParseRipGrepLine 函数解析单行
		if (ParseRipGrepLine(line, results, filterCallback))
		{
			resultCount++;
			// 检查是否达到最大结果数
			if (resultCount >= maxResults)
				break;
		}
	}
}

bool SearchWithRipGrep(const char* key, std::vector<std::string>& folderPathes, FindInFileResults& results, int maxResults, FindInFileFilter filterCallback)
{
	if (!key || *key == '\0')
		return false;

	// 构建 ripgrep 命令
	std::string cmd = "rg -u --no-heading --line-number --color never --no-mmap --max-columns 0 ";

	// 添加文件扩展名过滤
	// ripgrep 使用 --type-add 自定义类型，然后 --type 使用
	std::vector<std::string> exts;
	SplitStringBy(";", CODE_FILE_EXTENSIONS, &exts);

	// 使用 {ext1,ext2,ext3} 格式构建扩展名过滤模式
	std::string extPattern = "*.{";
	for (size_t i = 0; i < exts.size(); ++i)
	{
		if (i > 0) extPattern += ",";
		extPattern += exts[i];
	}
	extPattern += "}";

	// 使用 -g (glob) 参数来过滤文件扩展名
	cmd += "-g \"" + extPattern + "\" ";

	// 添加搜索模式（用引号包裹，避免 shell 解析问题）
	// 注意：-- 分隔符告诉 ripgrep 后面的都是文件路径，而不是正则表达式选项
	cmd += " \"" + std::string(key) + "\"";

	// 添加路径（在 -- 分隔符之后，确保路径中的反斜杠不会被当作正则表达式）
	cmd += " --";
	if (!folderPathes.empty())
	{
		for (size_t i = 0; i < folderPathes.size(); ++i)
		{
			cmd += " \"" + folderPathes[i] + "\"";
		}
	}
	else
	{
		cmd += " .";
	}

	// 执行 ripgrep 命令
	SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
	HANDLE hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		return false;

	STARTUPINFOA si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = hWrite;
	si.hStdError = hWrite;

	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessA(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		CloseHandle(hRead);
		CloseHandle(hWrite);
		return false;
	}

	CloseHandle(hWrite);

	// 读取输出并实时解析
	std::string output;
	char buffer[4096];
	DWORD bytesRead;
	int resultCount = 0;

	bool truncated = false;
	while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		output += buffer;

		// 实时解析已读取的内容
		// 查找完整的行（以换行符结束）
		size_t lastNewline = output.find_last_of('\n');
		if (lastNewline != std::string::npos)
		{
			// 提取完整的行进行处理
			std::string completeLines = output.substr(0, lastNewline + 1);
			output = output.substr(lastNewline + 1); // 保留未完成的行

			// 使用 ParseRipGrepLines 函数解析多行
			ParseRipGrepLines(completeLines, results, resultCount, maxResults, filterCallback);

			// 检查是否达到最大结果数
			if (resultCount >= maxResults)
			{
				TerminateProcess(pi.hProcess, 0);
				truncated = true;
				break;
			}
		}
	}

	CloseHandle(hRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// 处理缓冲区中剩余的内容
	if (!truncated)
	{
		if (!output.empty())
		{
			ParseRipGrepLines(output, results, resultCount, maxResults, filterCallback);
		}
	}

	return results.GetTotalResults() > 0;
}


void FindInFile(const char* key, std::vector<std::string>& folderPathes, FindInFileResults& results, int maxResults, FindInFileFilter filterCallback)
{
	results.Clear();

	if (!key || *key == '\0')
		return;

// 	FindInFileResults result2;
// 	// 如果 ripgrep 搜索失败，尝试使用 Everything 搜索
// 	if (IsEverythingRunning())
// 	{
// 		SearchWithEverything(key, folderPathes, result2);
// 	}
// 
// 	// 如果 Everything 搜索失败，使用 PowerShell 搜索
// 	SearchWithPowerShell(key, folderPathes, results);

	SearchWithRipGrep(key, folderPathes, results, maxResults, filterCallback);
}


int FindMatchingLines(const std::string& filePath, const std::string& key,
	const std::string& content, FindInFileResults& results, int maxLines)
{
	std::istringstream stream(content);
	std::string line;
	int lineNumber = 0;
	int addedCount = 0;

	auto isWordChar = [](char c) -> bool {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
	};

	// 检查key是否包含非word字符
	bool hasNonWordChar = false;
	for (char c : key)
	{
		if (!isWordChar(c))
		{
			hasNonWordChar = true;
			break;
		}
	}

	while (std::getline(stream, line) && addedCount < maxLines)
	{
		++lineNumber;

		// 大小写敏感的匹配
		size_t pos = 0;
		while ((pos = line.find(key, pos)) != std::string::npos)
		{
			// 如果key包含非word字符，不进行全词匹配检查
			if (hasNonWordChar)
			{
				results.AddResult(filePath, lineNumber, line);
				++addedCount;
				break; // 当前行已匹配，跳到下一行
			}

			// 检查前面是否是单词边界
			bool leftBoundary = (pos == 0) || !isWordChar(line[pos - 1]);
			// 检查后面是否是单词边界
			bool rightBoundary = (pos + key.length() == line.length()) || !isWordChar(line[pos + key.length()]);

			if (leftBoundary && rightBoundary)
			{
				results.AddResult(filePath, lineNumber, line);
				++addedCount;
				break; // 当前行已匹配，跳到下一行
			}

			++pos; // 继续搜索下一个位置
		}
	}

	return addedCount;
}


void DumpFindInFileResult(const char* key, const FindInFileResults& results, std::string& resultString, int maxResult)
{
	resultString.clear();

	if (results.fileInfos.empty())
	{
		resultString += "=====================\n";
		resultString += "Search Result Summary\n";
		resultString += "=====================\n";
		resultString += "Search Keyword: " + std::string(key ? key : "") + "\n";
		resultString += "Matched Files: 0\n";
		resultString += "Matched Lines: 0\n";
		resultString += "No results found.\n";
		resultString += "=====================\n";
		return;
	}

	// Statistics
	size_t totalFiles = results.fileInfos.size();
	size_t totalLines = results.GetTotalResults();

	// Calculate actual displayed lines
	size_t displayLines = totalLines;
	bool hasMore = false;
	if (maxResult > 0 && totalLines >= static_cast<size_t>(maxResult))
	{
		hasMore = true;
	}

	resultString += "=====================\n";
	resultString += "Search Result Summary\n";
	resultString += "=====================\n";
	resultString += "Search Keyword: " + std::string(key ? key : "") + "\n";
	resultString += "Matched Files: " + std::to_string(totalFiles) + "\n";
	resultString += "Matched Lines: " + std::to_string(totalLines) + "\n";
	if (hasMore)
	{
		resultString += "\n";
		resultString += "Note: Only first " + std::to_string(maxResult) + " results are shown due to the return result limit.\n";
	}
	resultString += "=====================\n\n";

	// Iterate through each file result
	size_t displayedCount = 0;
	bool limitReached = false;
	bool firstFile = true;

	for (const auto& fileInfo : results.fileInfos)
	{
		if (limitReached)
			break;

		resultString += "--------------------\n";
		resultString += "File: " + fileInfo.filePath + "\n";
		resultString += "Matched Lines: " + std::to_string(fileInfo.lineInfos.size()) + "\n";
		resultString += "--------------------\n";

		// Iterate through each matched line
		for (const auto& lineInfo : fileInfo.lineInfos)
		{
			// Check if max display limit is reached
			if (maxResult > 0 && displayedCount >= static_cast<size_t>(maxResult))
			{
				limitReached = true;
				break;
			}

			resultString += "  [" + std::to_string(lineInfo.lineNumber) + "] ";
			
			// Display symbol name if available
			if (!lineInfo.symbolName.empty())
			{
				std::string symbolName = lineInfo.symbolName;
				symbolName = RestoreSymbolName(symbolName);
				resultString += "[in " + symbolName + "] ";
			}
			
			// Display line content
			resultString += lineInfo.lineContent + "\n";
			displayedCount++;
		}

		if (limitReached && displayedCount < static_cast<size_t>(totalLines))
		{
			resultString += "  ... (" + std::to_string(totalLines - displayedCount) + " more lines not shown)\n";
		}

		resultString += "\n";
	}

	resultString += "=====================\n";
	resultString += "Search completed";
	if (hasMore)
	{
		resultString += " (showing first " + std::to_string(maxResult) + " lines)";
	}
	resultString += "\n";
	resultString += "=====================\n";
}

void BuildFindInFilesResultJson(nlohmann::json& j, const std::unordered_map<std::string, FindInFileResults>& resultsList, int maxResult)
{
	j["error"] = nullptr;
	j["maxResult"] = maxResult;
	j["keywords"] = nlohmann::json::array();
	j["totalMatches"] = 0;
	j["totalFiles"] = 0;
	j["results"] = nlohmann::json::array();

	size_t totalMatchesAll = 0;
	size_t totalFilesAll = 0;

	for (const auto& pair : resultsList)
	{
		const std::string& keyword = pair.first;
		const FindInFileResults& results = pair.second;

		j["keywords"].push_back(keyword);

		nlohmann::json resultItem;
		resultItem["keyword"] = keyword;
		resultItem["totalMatches"] = results.GetTotalResults();
		resultItem["totalFiles"] = results.fileInfos.size();
		resultItem["files"] = nlohmann::json::array();

		for (const auto& fileInfo : results.fileInfos)
		{
			nlohmann::json fileItem;
			fileItem["filePath"] = fileInfo.filePath;
			fileItem["matchedLines"] = fileInfo.lineInfos.size();
			fileItem["lines"] = nlohmann::json::array();

			for (const auto& lineInfo : fileInfo.lineInfos)
			{
				nlohmann::json lineItem;
				lineItem["lineNumber"] = lineInfo.lineNumber;
				lineItem["symbolName"] = lineInfo.symbolName;
				lineItem["lineContent"] = lineInfo.lineContent;
				fileItem["lines"].push_back(lineItem);
			}

			resultItem["files"].push_back(fileItem);
		}

		j["results"].push_back(resultItem);

		totalMatchesAll += results.GetTotalResults();
		totalFilesAll += results.fileInfos.size();
	}

	j["totalMatches"] = totalMatchesAll;
	j["totalFiles"] = totalFilesAll;
}

void BuildFindInFilesErrorJson(nlohmann::json& j, const char* errorMessage)
{
	j["error"] = errorMessage ? errorMessage : "";
	j["keywords"] = nlohmann::json::array();
	j["maxResult"] = 0;
	j["totalMatches"] = 0;
	j["totalFiles"] = 0;
	j["results"] = nlohmann::json::array();
}

void DumpFindInFileResultsFromJson(nlohmann::json& j, std::string& outText)
{
	outText.clear();

	// Check for error
	if (j.contains("error") && !j["error"].is_null())
	{
		outText = j["error"].get<std::string>();
		return;
	}

	// Check for results
	if (!j.contains("results") || !j["results"].is_array())
	{
		return;
	}

	int maxResult = j.value("maxResult", 0);
	const auto& results = j["results"];

	for (size_t i = 0; i < results.size(); ++i)
	{
		const auto& resultItem = results[i];

		// Add separator between keywords
		if (i > 0)
		{
			outText += "\n";
// 			outText += "=====================\n";
// 			outText += "\n";
		}

		// Add keyword header
		std::string keyword = resultItem.value("keyword", "");

		// Check if there are files
		if (!resultItem.contains("files") || !resultItem["files"].is_array() || resultItem["files"].empty())
		{
			outText += "No results found for keyword: \"";
			outText += keyword;
			outText += "\"\n";
			continue;
		}

		// Build a temporary FindInFileResults to reuse DumpFindInFileResult
		FindInFileResults tempResults;
		for (const auto& fileItem : resultItem["files"])
		{
			FindInFileResults::FileInfo fi;
			fi.filePath = fileItem.value("filePath", "");

			if (fileItem.contains("lines") && fileItem["lines"].is_array())
			{
				for (const auto& lineItem : fileItem["lines"])
				{
					FindInFileResults::FileLineInfo li;
					li.lineNumber = lineItem.value("lineNumber", 0);
					li.lineContent = lineItem.value("lineContent", "");
					li.symbolName = lineItem.value("symbolName", "");
					fi.lineInfos.push_back(li);
				}
			}

			tempResults.fileInfos.push_back(fi);
		}

		// Reuse existing DumpFindInFileResult
		std::string keywordResult;
		DumpFindInFileResult(keyword.c_str(), tempResults, keywordResult, maxResult);
		outText += keywordResult;
	}
}

void DumpFindInFileSimpleResultsFromJson(nlohmann::json& j, std::string& outText)
{
	outText.clear();

	// Check for error
	if (j.contains("error") && !j["error"].is_null())
	{
		outText = j["error"].get<std::string>();
		return;
	}

	// Check for results
	if (!j.contains("results") || !j["results"].is_array())
	{
		return;
	}

	const auto& results = j["results"];

	for (size_t i = 0; i < results.size(); ++i)
	{
		const auto& resultItem = results[i];

		// Add separator between keywords
		if (i > 0)
		{
			outText += "\n";
		}

		// Add keyword header
		std::string keyword = resultItem.value("keyword", "");
		int totalMatches = resultItem.value("totalMatches", 0);
		int totalFiles = resultItem.value("totalFiles", 0);

		// Check if there are files
		if (!resultItem.contains("files") || !resultItem["files"].is_array() || resultItem["files"].empty())
		{
			outText += "No results found for keyword: \"";
			outText += keyword;
			outText += "\"\n";
			continue;
		}

		// Summary header
		outText += "=====================\n";
		outText += "Search Keyword: " + keyword + "\n";
		outText += "Matched Files: " + std::to_string(totalFiles) + "\n";
		outText += "Matched Lines: " + std::to_string(totalMatches) + "\n";
		outText += "=====================\n\n";

		// List files and line numbers only
		for (const auto& fileItem : resultItem["files"])
		{
			std::string filePath = fileItem.value("filePath", "");
			outText += "File: " + filePath + "\n";
			outText += "Lines: ";

			if (fileItem.contains("lines") && fileItem["lines"].is_array())
			{
				const auto& lines = fileItem["lines"];
				for (size_t k = 0; k < lines.size(); ++k)
				{
					if (k > 0)
						outText += ", ";
					outText += std::to_string(lines[k].value("lineNumber", 0));
				}
			}
			outText += "\n\n";
		}
	}
}

}
