#include "stdh.h"
#include "ChatTask_ReplaceInFile.h"
#include <algorithm>
#include <cstring>
#include "Utils.h"
#include <sstream>
#include <vector>

#include "LlmChat.h"

#include "LlmLib.h"

#include "Utils_File.h"

#include "ChatAgent.h"

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <limits>



// 辅助函数：将字符串按行分割
std::vector<std::string> SplitLines(const std::string& str)
{
	std::vector<std::string> lines;
	std::string line;
	std::istringstream stream(str);

	while (std::getline(stream, line))
	{
		// 处理可能的\r字符
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}
		lines.push_back(line);
	}

	// 如果最后一个字符是换行符，需要添加一个空行
	if (!str.empty() && (str.back() == '\n' || str.back() == '\r'))
	{
		lines.push_back("");
	}

	return lines;
}

// 辅助函数：生成行范围字符串，格式为 "<line xx ~ line XX>"
std::string MakeLineRangeString(int startLine, int endLine)
{
	return "<line " + std::to_string(startLine) + " ~ line " + std::to_string(endLine) + ">";
}

// 辅助函数：精简内容，保留前后各 keepLines 行，中间省略
// 用于生成 partial tool call，减少发送给 LLM 的 token 数
std::string SimplifyLines(const std::string& lines, int keepLines = 20)
{
	std::vector<std::string> lineList = SplitLines(lines);
	
	// 如果行数不超过阈值，直接返回原内容
	int totalLines = static_cast<int>(lineList.size());
	int threshold = keepLines * 2 + 5; // 至少要省略5行才有意义
	if (totalLines <= threshold)
		return lines;
	
	std::string result;
	
	// 添加前 keepLines 行
	for (int i = 0; i < keepLines && i < totalLines; i++)
	{
		if (!result.empty())
			result += "\n";
		result += lineList[i];
	}
	
	// 添加省略标记
	int omittedLines = totalLines - keepLines * 2;
	result += "\n... (";
	result += std::to_string(omittedLines);
	result += " lines omitted) ...\n";
	
	// 添加后 keepLines 行
	for (int i = totalLines - keepLines; i < totalLines; i++)
	{
		if (i >= keepLines) // 确保不与前部分重叠
		{
			result += lineList[i];
			if (i < totalLines - 1)
				result += "\n";
		}
	}
	
	return result;
}

// 辅助函数：计算内容行数
int CountLines(const std::string& content)
{
	if (content.empty())
		return 0;
	std::vector<std::string> lines = SplitLines(content);
	return static_cast<int>(lines.size());
}

// 辅助函数：计算两个字符串的相似度（使用编辑距离）
int LevenshteinDistance(const std::string& s1, const std::string& s2)
{
	const size_t len1 = s1.size(), len2 = s2.size();
	std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

	// 初始化第一列和第一行
	for (size_t i = 0; i <= len1; i++)
	{
		dp[i][0] = i;
	}
	for (size_t j = 0; j <= len2; j++)
	{
		dp[0][j] = j;
	}

	// 计算编辑距离
	for (size_t i = 1; i <= len1; i++)
	{
		for (size_t j = 1; j <= len2; j++)
		{
			if (s1[i - 1] == s2[j - 1])
			{
				// 字符相同，不需要操作
				dp[i][j] = dp[i - 1][j - 1];
			}
			else
			{
				// 字符不同，需要进行操作
				int deleteOp = dp[i - 1][j] + 1;      // 删除s1中的字符
				int insertOp = dp[i][j - 1] + 1;      // 在s1中插入字符
				int replaceOp = dp[i - 1][j - 1] + 1;   // 替换s1中的字符

				// 选择代价最小的操作
				dp[i][j] = min(deleteOp, min(insertOp, replaceOp));
			}
		}
	}

	return dp[len1][len2];
}

// 辅助函数：去除行首尾的空白字符（用于比较）
std::string TrimWhitespace(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos)
	{
		return "";
	}
	size_t last = str.find_last_not_of(" \t");
	return str.substr(first, last - first + 1);
}

// 辅助函数：提取行首的空白字符
std::string GetLeadingWhitespace(const std::string& str)
{
	size_t pos = 0;
	while (pos < str.length() && (str[pos] == ' ' || str[pos] == '\t'))
	{
		pos++;
	}
	return str.substr(0, pos);
}

// 辅助函数：去除行末的空白字符
std::string TrimTrailingWhitespace(const std::string& str)
{
	size_t end = str.find_last_not_of(" \t\r\n");
	if (end == std::string::npos)
		return "";
	return str.substr(0, end + 1);
}



/**
 * @brief 内部模板函数，用于处理代码行的核心逻辑。
 * @tparam T_Char 字符类型 (char 或 wchar_t)
 * @tparam T_String 字符串类型 (std::string 或 std::wstring)
 * @param line 要处理的单行代码。
 * @return 处理后的字符串。
 *
 * 此函数通过一个状态机来解析代码行：
 * 1. 保留行首的所有空白字符。
 * 2. 在常规代码中，只有当空白字符两侧都不是字母/数字/下划线时，才将多个连续空白压缩成一个空格。
 * 3. 完整保留字符串字面量 ("...") 和字符字面量 ('...') 内部的所有字符，包括空白。
 * 4. 保留注释内容，不进行任何清理。
 *
 * 注意：此函数按行处理，无法跨行上下文来判断一个块注释的内部。
*/
template<typename T_Char, typename T_String>
T_String ProcessLineForCompareT(const T_String & line)
{
	enum State
	{
		LEADING_WHITESPACE, // 行首空白状态
		NORMAL,             // 常规代码状态
		IN_STRING,          // 字符串字面量状态 "..."
		IN_CHAR_LITERAL     // 字符字面量状态 '...'
	};

	// 辅助函数：检查字符是否为字母、数字或下划线
	auto isAlnumOrUnderscore = [](T_Char c) -> bool {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
		       (c >= '0' && c <= '9') || c == '_';
	};

	T_String result;
	result.reserve(line.length());
	State state = LEADING_WHITESPACE;

	const bool removeLeadingWhiteSpace = true;

	for (size_t i = 0; i < line.length(); ++i)
	{
		T_Char c = line[i];

		// 全局清理换行符（不论在哪种状态下）
		if (c == '\n' || c == '\r')
		{
			continue; // 跳过换行符
		}

		switch (state)
		{
		case LEADING_WHITESPACE:
			if (!removeLeadingWhiteSpace)
			{
				if (c == ' ' || c == '\t')
					result += c; // 在行首，完整保留空白
				else
				{
					state = NORMAL; // 遇到第一个非空白字符，切换到常规状态
					i--;            // 重新处理当前字符
				}
			}
			else
			{
				state = NORMAL; // 切换到常规状态
				i--;            // 重新处理当前字符
			}
			break;


		case NORMAL:
			if (c == ' ' || c == '\t')
			{
				// 检查空白字符两侧的字符
				T_Char prev_c = (i > 0) ? line[i - 1] : 0;
				T_Char next_c = (i + 1 < line.length()) ? line[i + 1] : 0;
				
				// 如果两侧都是字母/数字/下划线，则必须保留这个空白
				if (isAlnumOrUnderscore(prev_c) && isAlnumOrUnderscore(next_c))
				{
					result += c;
				}
				else
				{
					// 否则，完全跳过连续的空白字符
					while (i + 1 < line.length() && (line[i + 1] == ' ' || line[i + 1] == '\t'))
					{
						i++;
					}
				}
			}
			else
			{
				result += c; // 添加当前字符

				// 检查是否进入字符串或字符字面量状态
				if (c == '"')
				{
					state = IN_STRING;
				}
				else if (c == '\'')
				{
					state = IN_CHAR_LITERAL;
				}
			}
			break;

		case IN_STRING:
			result += c;
			// 检查是否为非转义的双引号，以确定是否退出字符串状态
			if (c == '"')
			{
				int backslash_count = 0;
				for (int k = i - 1; k >= 0 && line[k] == '\\'; --k)
				{
					backslash_count++;
				}
				if (backslash_count % 2 == 0) // 偶数个反斜杠表示引号未被转义
				{
					state = NORMAL;
				}
			}
			break;

		case IN_CHAR_LITERAL:
			result += c;
			// 检查是否为非转义的单引号，以确定是否退出字符状态
			if (c == '\'')
			{
				int backslash_count = 0;
				for (int k = i - 1; k >= 0 && line[k] == '\\'; --k)
				{
					backslash_count++;
				}
				if (backslash_count % 2 == 0) // 偶数个反斜杠表示引号未被转义
				{
					state = NORMAL;
				}
			}
			break;
		}
	}

	return result;
}

/**
 * @brief 清理单行代码字符串以用于比较。
 *
 * 该函数旨在标准化代码行的格式，同时保留其结构和语义，以便于比较。
 * - 保留行首的空白（用于维持缩进）。
 * - 只有当空白字符两侧都不是字母/数字/下划线时，才将多个连续的空白字符压缩为单个空格。
 * - 完整保留字符串字面量 ("...") 和字符字面量 ('...') 内部的所有字符，包括空白。
 * - 保留注释内容，不进行清理。
 * - 如果检测到非ASCII字符，会自动使用宽字符进行处理，以确保正确性。
 *
 * @param str 输入的单行代码字符串 (UTF-8编码)。
 * @return 标准化处理后的代码字符串。
 */
std::string TrimCodeLineForCompare(const std::string& str)
{
	// 根据要求，使用 is_pure_ascii 检查字符串
	if (is_pure_ascii(str.c_str()))
	{
		// 如果是纯ASCII，直接使用char类型处理
		return ProcessLineForCompareT<char, std::string>(str);
	}
	else
	{
		// 如果包含非ASCII字符，转换为宽字符串进行处理，以避免破坏多字节字符
		std::wstring wstr = utf8_to_widechar(str);
		std::wstring w_result = ProcessLineForCompareT<wchar_t, std::wstring>(wstr);
		return widechar_to_utf8(w_result.c_str());
	}
}

// 精确替换函数：要求每行内容必须完全一致（忽略行末空白），且只能有一个匹配
bool ReplaceInFileAccurately(const std::string& oldContent, const char* oldLines,
	const char* newLines, std::string& newContent, std::string& errorMessage,
	LineRange& oldLineRange, LineRange& newLineRange)
{
	// 初始化输出参数
	oldLineRange.Zero();
	newLineRange.Zero();

	if (!oldLines || !newLines)
	{
		errorMessage = "Invalid input: oldLines or newLines is null";
		return false;
	}

	if (IsAllBlank(oldContent.c_str()) && (!IsAllBlank(oldLines)))
	{
		errorMessage = "Target file is empty or not existing, could not modify it";
		return false;
	}


	if (IsAllBlank(oldContent.c_str()) && (IsAllBlank(oldLines)))
	{
		newContent = newLines;
		// 空文件情况下，newLines从第0行开始
		int lineCount = 0;
		for (const char* p = newLines; *p; p++)
		{
			if (*p == '\n') lineCount++;
		}
		if (newLines[0] != '\0') lineCount++; // 有内容但无结尾换行
		newLineRange.start = 0;
		newLineRange.end = lineCount > 0 ? lineCount - 1 : 0;
		return true;
	}

	// 分割所有内容为行
	std::vector<std::string> contentLines = SplitLines(oldContent);
	std::vector<std::string> searchLines = SplitLines(std::string(oldLines));
	std::vector<std::string> replaceLines = SplitLines(std::string(newLines));
	std::vector<std::string> untrimedReplaceLines = replaceLines;
	std::vector<std::string> untrimedContentLines = contentLines;

	// 立即去除所有行的尾部空白字符（不包括\n）
	for (auto& line : contentLines)
	{
		line = TrimCodeLineForCompare(line);
	}
	for (auto& line : searchLines)
	{
		line = TrimCodeLineForCompare(line);
	}
	for (auto& line : replaceLines)
	{
		line = TrimCodeLineForCompare(line);
	}

	// 如果没有要搜索的内容
	if (searchLines.empty())
	{
		errorMessage = "Invalid input: oldLines is empty";
		return false;
	}

	// 去除搜索行中的空行（首尾）
	while (!searchLines.empty() && TrimWhitespace(searchLines.front()).empty())
	{
		searchLines.erase(searchLines.begin());
	}
	while (!searchLines.empty() && TrimWhitespace(searchLines.back()).empty())
	{
		searchLines.pop_back();
	}

	if (searchLines.empty())
	{
		errorMessage = "Invalid input: oldLines contains only empty lines";
		return false;
	}

	// 检查oldLines和newLines是否完全相同
	if (searchLines == replaceLines)
	{
		errorMessage = "Old lines and new lines are identical";
		return false;
	}

	// 查找匹配位置，找到第二个匹配立即返回失败
	int matchPos = -1;
	
	// 遍历所有可能的起始位置
	if (searchLines.size() <= contentLines.size())
	{
		for (size_t i = 0; i <= contentLines.size() - searchLines.size(); i++)
		{
			bool isMatch = true;

			// 检查每一行是否精确匹配（已去除尾部空白）
			for (size_t j = 0; j < searchLines.size(); j++)
			{
				if (contentLines[i + j] != searchLines[j])
				{
					isMatch = false;
					break;
				}
			}

			if (isMatch)
			{
				if (matchPos == -1)
				{
					matchPos = static_cast<int>(i);
				}
				else
				{
					// 找到第二个匹配，立即返回失败
					errorMessage = "Multiple matching blocks found in the file (found at least 2 matches)";
					return false;
				}
			}
		}
	}

	// 检查匹配结果
	if (matchPos == -1)
	{
		errorMessage = "No matching block found in the file";
		return false;
	}

	// 构建新内容
	newContent.clear();

	// 添加匹配位置之前的内容
	for (int i = 0; i < matchPos; i++)
	{
		if (!newContent.empty())
			newContent += "\r\n";
		newContent += untrimedContentLines[i];
	}

	// 添加替换内容，保留替换内容原有的缩进
	for (size_t i = 0; i < untrimedReplaceLines.size(); i++)
	{
		if (!newContent.empty())
			newContent += "\r\n";
		newContent += untrimedReplaceLines[i];
	}

	// 添加匹配位置之后的内容
	for (size_t i = matchPos + searchLines.size(); i < untrimedContentLines.size(); i++)
	{
		if (!newContent.empty())
			newContent += "\r\n";
		newContent += untrimedContentLines[i];
	}

	// 设置oldLineRange: oldLines在oldContent中的行范围
	oldLineRange.start = static_cast<UINT16>(matchPos);
	oldLineRange.end = static_cast<UINT16>(matchPos + searchLines.size() - 1);

	// 设置newLineRange: newLines在newContent中的行范围
	// newLines在matchPos位置插入，占据相同的行号范围
	newLineRange.start = static_cast<UINT16>(matchPos);
	newLineRange.end = static_cast<UINT16>(matchPos + untrimedReplaceLines.size() - 1);

	return true;
}

// 主函数实现（保持原有的模糊匹配功能）
bool ReplaceInFile(const std::string& oldContent, const char* oldLines,
	const char* newLines, std::string& newContent)
{
	if (!oldLines || !newLines)
	{
		return false;
	}

	// 分割所有内容为行
	std::vector<std::string> contentLines = SplitLines(oldContent);
	std::vector<std::string> searchLines = SplitLines(std::string(oldLines));
	std::vector<std::string> replaceLines = SplitLines(std::string(newLines));

	// 如果没有要搜索的内容
	if (searchLines.empty())
	{
		return false;
	}

	// 去除搜索行中的空行（首尾）
	while (!searchLines.empty() && TrimWhitespace(searchLines.front()).empty())
	{
		searchLines.erase(searchLines.begin());
	}
	while (!searchLines.empty() && TrimWhitespace(searchLines.back()).empty())
	{
		searchLines.pop_back();
	}

	if (searchLines.empty())
	{
		return false;
	}

	// 寻找最佳匹配位置
	int bestMatchPos = -1;
	int minDistance = 0x7fffffff;

	// 遍历所有可能的起始位置
	for (size_t i = 0; i <= contentLines.size() - searchLines.size(); i++)
	{
		int totalDistance = 0;
		bool skipThisPosition = false;

		// 计算这个位置的总编辑距离
		for (size_t j = 0; j < searchLines.size(); j++)
		{
			// 比较时保留行首空白
			std::string contentLine = contentLines[i + j];
			std::string searchLine = searchLines[j];

			// 如果行首空白差异太大，跳过这个位置
			if (std::abs((int)GetLeadingWhitespace(contentLine).length() -
				(int)GetLeadingWhitespace(searchLine).length()) > 4)
			{
				skipThisPosition = true;
				break;
			}

			totalDistance += LevenshteinDistance(contentLine, searchLine);
		}

		if (!skipThisPosition && totalDistance < minDistance)
		{
			minDistance = totalDistance;
			bestMatchPos = i;
		}
	}

	// 如果没有找到合适的匹配
	if (bestMatchPos == -1)
	{
		return false;
	}

	// 构建新内容
	newContent.clear();

	// 添加匹配位置之前的内容
	for (int i = 0; i < bestMatchPos; i++)
	{
		if (!newContent.empty())
			newContent += "\n";
		newContent += contentLines[i];
	}

	// 添加替换内容，保留替换内容原有的缩进
	for (size_t i = 0; i < replaceLines.size(); i++)
	{
		if (!newContent.empty())
			newContent += "\n";
		// 直接使用替换行的原始内容，保留其所有空白字符
		newContent += replaceLines[i];
	}

	// 添加匹配位置之后的内容
	for (size_t i = bestMatchPos + searchLines.size(); i < contentLines.size(); i++)
	{
		if (!newContent.empty())
			newContent += "\n";
		newContent += contentLines[i];
	}

	return true;
}

//检测content是否为如下格式,如果是的话,根据它构建多个临时的 LlmToolType::ReplaceInFile 的 LlmToolCall
//[Old Lines]
//... old lines #1
//[New Lines]
//... new lines #1
//[Old Lines]
//... old lines #2
//[New Lines]
//... new lines #2
//...
//或者第一行没有[Old Lines]的情况:
//... old lines #1 (第一个[New Lines]之前的内容)
//[New Lines]
//... new lines #1
//[Old Lines]
//... old lines #2
//[New Lines]
//... new lines #2
//...
bool BuildReplaceInFileToolCallsFromFileEditContent(const char* filePath, const char* fileEditContent, std::vector<LlmToolCall>& toolCalls)
{
	std::string content(fileEditContent);
	std::string old_lines_tag = "[Old Lines]";
	std::string new_lines_tag = "[New Lines]";

	toolCalls.clear();

	// 使用SplitLines将内容分行
	std::vector<std::string> lines = SplitLines(content);
	if (lines.empty())
		return false;

	int current_line = 0;
	bool is_first_section = true;

	while (current_line < lines.size())
	{
		std::string old_lines;
		std::string new_lines;
		int old_lines_start = -1;
		int old_lines_end = -1;
		int new_lines_start = -1;
		int new_lines_end = -1;

		// 查找old lines段落
		if (is_first_section)
		{
			// 第一段：检查是否以[Old Lines]开头
			if (current_line < lines.size() && lines[current_line] == old_lines_tag)
			{
				// 标准格式：以[Old Lines]开头
				old_lines_start = current_line + 1;
				current_line++;
				
				// 查找对应的[New Lines]
				while (current_line < lines.size() && lines[current_line] != new_lines_tag)
				{
					current_line++;
				}
				
				if (current_line >= lines.size())
					break; // 没有找到[New Lines]
				
				old_lines_end = current_line - 1;
			}
			else
			{
				// 非标准格式：直接从内容开始
				old_lines_start = current_line;
				
				// 查找第一个[New Lines]
				while (current_line < lines.size() && lines[current_line] != new_lines_tag)
				{
					current_line++;
				}
				
				if (current_line >= lines.size())
					break; // 没有找到[New Lines]
				
				old_lines_end = current_line - 1;
			}
			is_first_section = false;
		}
		else
		{
			// 后续段落：必须以[Old Lines]开头
			if (current_line >= lines.size() || lines[current_line] != old_lines_tag)
				break;
			
			old_lines_start = current_line + 1;
			current_line++;
			
			// 查找对应的[New Lines]
			while (current_line < lines.size() && lines[current_line] != new_lines_tag)
			{
				current_line++;
			}
			
			if (current_line >= lines.size())
				break; // 没有找到[New Lines]
			
			old_lines_end = current_line - 1;
		}

		// 现在current_line指向[New Lines]
		if (current_line >= lines.size() || lines[current_line] != new_lines_tag)
			break;

		new_lines_start = current_line + 1;
		current_line++;

		// 查找new lines的结束位置（下一个[Old Lines]或文件结束）
		while (current_line < lines.size() && lines[current_line] != old_lines_tag)
		{
			current_line++;
		}
		new_lines_end = current_line - 1;

		// 提取old_lines内容
		if (old_lines_start >= 0 && old_lines_end >= old_lines_start)
		{
			for (int i = old_lines_start; i <= old_lines_end; i++)
			{
				if (i < lines.size())
				{
					if (!old_lines.empty())
						old_lines += "\n";
					old_lines += lines[i];
				}
			}
		}

		// 提取new_lines内容
		if (new_lines_start >= 0 && new_lines_end >= new_lines_start)
		{
			for (int i = new_lines_start; i <= new_lines_end; i++)
			{
				if (i < lines.size())
				{
					if (!new_lines.empty())
						new_lines += "\n";
					new_lines += lines[i];
				}
			}
		}

		// 创建 LlmToolCall
		LlmToolCall toolCall;
		toolCall.tp = LlmToolType::ReplaceInFile;
		toolCall.params_string["filePath"] = filePath;
		toolCall.params_string["oldLines"] = old_lines;
		toolCall.params_string["newLines"] = new_lines;
		toolCall.isComplete = true;

		toolCalls.push_back(toolCall);
	}

	return !toolCalls.empty();
}


CChatTask_ReplaceInFile::CChatTask_ReplaceInFile(const std::wstring& fileEditId)
{
	_fileEditId = fileEditId;
	_wasReadOnly = false;
}

CChatTask_ReplaceInFile::CChatTask_ReplaceInFile(const std::string& filePath, const std::string& oldLines, const std::string& newLines)
{
	_filePath = filePath;
	_oldLines = oldLines;
	_newLines = newLines;
	_wasReadOnly = false;
}


bool CChatTask_ReplaceInFile::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("ReplaceInFile"))
		return false;

	if (_filePath.empty())
		return true;

	CChatTask_ReplaceInFile* task = (CChatTask_ReplaceInFile*)task0;
	if (task->_filePath.empty())
		return true;
	if (task->_filePath == _filePath)
		return true;
	return false;
}

void CChatTask_ReplaceInFile::Start()
{
	// 优先使用 chatAgent
	if (_context->chatAgent)
	{
		_context->chatAgent->ShowFileEditProgressLabel(std::wstring(L""));
	}
}

void CChatTask_ReplaceInFile::Update()
{
	if (true)
	{
		if (_filePath.empty())
		{
			if (_toolCall.ExistParam("oldLines"))
				_toolCall.GetStringParam("filePath", _filePath);
			if (!_filePath.empty())
				FixSlashInPath_Utf8((char*)_filePath.c_str());//有时候会传反斜杠的路径名
		}

		if (_filePath.empty())
			return;
	}

	// 优先使用 chatAgent
	if (_context->chatAgent)
	{
		// 一旦获取到文件路径，就开始检测只读状态
		if (!_filePath.empty())
		{
			// 检查文件是否只读
			bool isReadOnly = Utils::IsFileReadOnly(_filePath.c_str());
			if (isReadOnly)
			{
				// 文件只读，只显示进度标签，不创建FileEdit
				// 只在状态变化时更新（从非只读变为只读，或首次检测到只读）
				if (!_wasReadOnly)
				{
					std::string fileName = GetFileName(_filePath);
					std::wstring progressText = utf8_to_widechar(fileName) + L" <span style='color:#ff6b6b;'>[read only]</span>";
					_context->chatAgent->ShowFileEditProgressLabel(progressText);
					_wasReadOnly = true;
				}
				
				// 继续等待，不执行替换操作
				return;
			}

			// 文件可写，隐藏进度标签
			// 只在状态变化时更新（从只读变为可写）
			if (_wasReadOnly)
			{
				_context->chatAgent->ShowFileEditProgressLabel(L"");
				_wasReadOnly = false;
			}
		}
	}

	if (!_toolCall.IsComplete())
	{
		std::string fileName = GetFileName(_filePath);
		if (_context->chatAgent)
			_context->chatAgent->ShowFileEditProgressLabel(utf8_to_widechar(fileName.c_str()));
		return;
	}

	if (_fileEditId.empty())
	{
		std::wstring aiMessageId;
		
		// 优先使用 chatAgent 获取当前 AI 消息 ID
		if (_context->chatAgent)
		{
			aiMessageId = _context->chatAgent->GetCurrentAIMessageId();
		}
		
		std::string fileName = GetFileName(_filePath);
		
		// 优先使用 chatOpsCtrl
		if (_context->chatOpsCtrl)
		{
			_fileEditId = _context->chatOpsCtrl->AddFileEditToAIMessage(aiMessageId, utf8_to_widechar(fileName), utf8_to_widechar(_filePath), L"");
		}
	}

	std::string errorMessage="Unknown reason";

	if (!_fileEditId.empty())
	{
		if (true)
		{
			_toolCall.GetStringParam("oldLines", _oldLines);
			_toolCall.GetStringParam("newLines", _newLines);
		}

		std::string full;
		full += u8"[Old Lines]\n";
		full += _oldLines + u8"\n";
		full += u8"[New Lines]\n";
		full += _newLines + u8"\n";

		// 优先使用 chatOpsCtrl
		if (_context->chatOpsCtrl)
		{
			_context->chatOpsCtrl->SetFileEditContent(_fileEditId, full, L"", FilesCheckpointUID_Invalid);
		}

		std::string oldContent;
		Utils::FileContentCodingFormat codingFmt= Utils::FileContentCodingFormat::Utf8;

		bool oldFileExist = Utils::IsFileExist(_filePath.c_str());
		bool loaded = false;
		if (oldFileExist)
			loaded = Utils::GetFileContentIntoUTF8(_filePath.c_str(), oldContent, codingFmt);

		if ((!oldFileExist) || loaded)
		{
			_status = TaskStatus::Success;
			std::string newContent;
			LineRange oldLineRange, newLineRange;

			if (!IsFullPath(_filePath.c_str()))
			{
				errorMessage = "Invalid file path -- Not a full path ";
				newContent= newContent + FILE_EDIT_RESULT_ERROR_PREFIX + errorMessage + " !";
			}
			else
			{
				if (!ReplaceInFileAccurately(oldContent, _oldLines.c_str(), _newLines.c_str(), newContent, errorMessage, oldLineRange, newLineRange))
				{
					newContent = "";
					newContent = newContent + FILE_EDIT_RESULT_ERROR_PREFIX + "Code replacing failure: " + errorMessage + " !";
					newContent += "\n";
					newContent += "***old lines***\n";
					newContent += _oldLines;
					newContent += "\n";
					newContent += "***new lines***\n";
					newContent += _newLines;
				}
			}

			Utils::EnsureFileFolder(_filePath.c_str());
			if (_SaveFileEditResult(_filePath, oldContent, newContent, codingFmt,_fileEditId))
			{
				_status = TaskStatus::Success;
				if (true)
				{
				// 创建 partial tool call：oldLines 精简（上下6行）
					LlmToolCall toolCallPartial = _toolCall;
					toolCallPartial.params_string["oldLines"] = SimplifyLines(_oldLines, 3);
					
					// 创建 fullCompress tool call：oldLines 精简（上下3行），newLines 精简（上下6行）
					LlmToolCall toolCallFullCompress = _toolCall;
					toolCallFullCompress.params_string["oldLines"] = SimplifyLines(_oldLines, 3);
					toolCallFullCompress.params_string["newLines"] = SimplifyLines(_newLines, 6);

					// fullCompress 的 result string
					std::string resultFullCompress = "Replaced old lines " + std::to_string(oldLineRange.start + 1) + "-" + std::to_string(oldLineRange.end);
					resultFullCompress += " with new content of line " + std::to_string(newLineRange.start + 1) + "-" + std::to_string(newLineRange.end);
					
					_SendToolCallResult("Successfully made the replacement in the file!", nullptr, &toolCallPartial, resultFullCompress.c_str(), &toolCallFullCompress);
					// UI 操作，继续使用 chatDialog
					if (_context->chatUi)
					{
						_context->chatUi->ActivateCheckpointFileChange(_fileEditId);
					}

				}

				return;
			}
		}
	}
	_status = TaskStatus::Failure;
	if (true)
	{
		std::string ret = std::string("Fail to make the replacement in the file!") + "[" + errorMessage + "]";
		
		_SendToolCallResult(ret.c_str());
	}
	return;
}

void CChatTask_ReplaceInFile::Interrupt()
{
	_status = TaskStatus::Failure;
}
