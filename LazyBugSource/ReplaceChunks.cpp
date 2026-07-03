#include "stdh.h"

#include "ReplaceChunks.h"
#include <sstream>
#include <string>
#include <vector>

// 检查一行是否只包含空白字符
bool IsEmptyLine(const std::string& line)
{
	for (char c : line)
	{
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
		{
			return false;
		}
	}
	return true;
}

// 检查chunk是否只包含空行
bool AreChunkLinesAllEmpty(const ReplaceChunks::Chunk& chunk)
{
	if (chunk.lines.empty())
		return true;
	
	for (const std::string& line : chunk.lines)
	{
		if (!IsEmptyLine(line))
		{
			return false;
		}
	}
	return true;
}

bool AreLinesAllEmpty(const std::string* lines, int count)
{
	for (int i = 0;i < count;i++)
	{
		if (!IsEmptyLine(lines[i]))
			return false;
	}
	return true;
}

void ParseReplaceChunks(const std::string& content, ReplaceChunks& chunks)
{
	std::istringstream stream(content);
	std::string line;
	ReplaceChunks::Chunk currentChunk;

	while (std::getline(stream, line))
	{
		if (line.find("... existing") != std::string::npos)
		{
			if (!currentChunk.lines.empty() && !AreChunkLinesAllEmpty(currentChunk))
			{
				chunks._chunks.push_back(currentChunk);
			}
			currentChunk = ReplaceChunks::Chunk();
		}
		else
		{
			currentChunk.lines.push_back(line);
		}
	}

	if (!currentChunk.lines.empty() && !AreChunkLinesAllEmpty(currentChunk))
	{
		chunks._chunks.push_back(currentChunk);
	}
}

// 计算行的缩进层级
int CalculateIndentLevel(const std::string& line)
{
	int indentLevel = 0;
	for (char c : line)
	{
		if (c == ' ')
			indentLevel++;
		else if (c == '\t')
			indentLevel += 4; // 制表符按4个空格计算
		else
			break;
	}
	return indentLevel / 4; // 每4个空格为一个缩进层级
}

// 获取去除首尾空白的字符串
std::string GetTrimmedLine(const std::string& line)
{
	size_t start = 0, end = line.length();
	while (start < line.length() && (line[start] == ' ' || line[start] == '\t'))
		start++;
	while (end > start && (line[end-1] == ' ' || line[end-1] == '\t' || 
	                       line[end-1] == '\r' || line[end-1] == '\n'))
		end--;
	
	if (start >= end)
		return "";
	
	return line.substr(start, end - start);
}

// 检测注释行
bool IsCommentLine(const std::string& trimmed, const ReplaceTargetFileLines& fileLines, size_t currentLineIndex)
{
	return trimmed.length() >= 2 && 
	       (trimmed.substr(0, 2) == "//" || 
	        (trimmed.substr(0, 2) == "/*" && trimmed.find("*/") != std::string::npos));
}

// 检测单独的右大括号行
bool IsCloseBracketLine(const std::string& trimmed, const ReplaceTargetFileLines& fileLines, size_t currentLineIndex)
{
	return trimmed == "}" || trimmed == "};" || trimmed == "};";
}

// 检测条件语句行
bool IsConditionTitleLine(const std::string& trimmed, const ReplaceTargetFileLines& fileLines, size_t currentLineIndex)
{
	if (trimmed.length() < 2) return false;
	
	return (trimmed.length() >= 3 && (trimmed.substr(0, 3) == "if(" || trimmed.substr(0, 3) == "if ")) ||
	       (trimmed.length() >= 4 && (trimmed.substr(0, 4) == "for(" || trimmed.substr(0, 4) == "for ")) ||
	       (trimmed.length() >= 6 && (trimmed.substr(0, 6) == "while(" || trimmed.substr(0, 6) == "while ")) ||
	       (trimmed.length() >= 7 && (trimmed.substr(0, 7) == "switch(" || trimmed.substr(0, 7) == "switch ")) ||
	       (trimmed.length() >= 5 && trimmed.substr(0, 5) == "else ") ||
	       trimmed == "else";
}

// 检测类声明行
bool IsClassDeclLine(const std::string& trimmed, const ReplaceTargetFileLines& fileLines, size_t currentLineIndex)
{
	return trimmed.find("class ") != std::string::npos || 
	       trimmed.find("struct ") != std::string::npos ||
	       trimmed.find("enum ") != std::string::npos ||
	       trimmed.find("union ") != std::string::npos;
}

// 检测函数实现行
bool IsFunctionImplLine(const std::string& trimmed, const ReplaceTargetFileLines& fileLines, size_t currentLineIndex)
{
	// 检查是否包含函数特征：返回类型 函数名(参数) 或 函数名(参数)
	if (trimmed.find('(') == std::string::npos || trimmed.find(')') == std::string::npos)
		return false;
	
	// 排除一些明显不是函数定义的情况
	if (trimmed.find('=') != std::string::npos || // 不是赋值语句
	    trimmed.find("if") == 0 || trimmed.find("for") == 0 || 
	    trimmed.find("while") == 0 || trimmed.find("switch") == 0 || // 不是控制语句
	    trimmed.find("//") != std::string::npos) // 不在注释中
		return false;
	
	// 检查下一行是否有左大括号，这是函数实现的强烈信号
	if (currentLineIndex + 1 < fileLines._lines.size())
	{
		const std::string& nextLine = fileLines._lines[currentLineIndex + 1];
		std::string nextTrimmed = GetTrimmedLine(nextLine);
		
		if (!nextTrimmed.empty() && nextTrimmed[0] == '{')
		{
			return true;
		}
	}
	
	// 或者当前行末尾就有左大括号
	if (!trimmed.empty() && trimmed.back() == '{')
	{
		return true;
	}
	
	return false;
}

void DetectFileLinesSemantic(ReplaceTargetFileLines& fileLines)
{
	// 确保lineAttrs数组大小与lines匹配
	fileLines._lineAttrs.resize(fileLines._lines.size());
	
	for (size_t i = 0; i < fileLines._lines.size(); i++)
	{
		const std::string& line = fileLines._lines[i];
		ReplaceTargetFileLines::LineAttr& attr = fileLines._lineAttrs[i];
		
		// 计算缩进层级
		attr.indentLevel = CalculateIndentLevel(line);
		
		// 获取去除首尾空白的字符串
		std::string trimmed = GetTrimmedLine(line);
		
		if (trimmed.empty())
		{
			attr.sem = ReplaceTargetFileLines::LineAttr::Semantic::None;
			continue;
		}
		
		// 按优先级检测各种语义类型
		if (IsCommentLine(trimmed, fileLines, i))
		{
			attr.sem = ReplaceTargetFileLines::LineAttr::Semantic::Comment;
		}
		else if (IsCloseBracketLine(trimmed, fileLines, i))
		{
			attr.sem = ReplaceTargetFileLines::LineAttr::Semantic::CloseBracket;
		}
		else if (IsConditionTitleLine(trimmed, fileLines, i))
		{
			attr.sem = ReplaceTargetFileLines::LineAttr::Semantic::ConditionTitle;
		}
		else if (IsClassDeclLine(trimmed, fileLines, i))
		{
			attr.sem = ReplaceTargetFileLines::LineAttr::Semantic::ClassDecl;
		}
		else if (IsFunctionImplLine(trimmed, fileLines, i))
		{
			attr.sem = ReplaceTargetFileLines::LineAttr::Semantic::FunctionImpl;
		}
		else
		{
			attr.sem = ReplaceTargetFileLines::LineAttr::Semantic::None;
		}
	}
}

void ParseReplaceTargetFileLines(const std::string& content, ReplaceTargetFileLines& fileLines)
{
	fileLines._lines.clear();
	std::istringstream stream(content);
	std::string line;
	while (std::getline(stream, line))
	{
		fileLines._lines.push_back(line);
	}
}

void DumpReplaceTargetFileLines(std::string& content, const ReplaceTargetFileLines& fileLines)
{
	content.clear();
	
	// 预分配足够的空间以提高性能
	size_t totalSize = 0;
	for (const auto& line : fileLines._lines)
	{
		totalSize += line.length() + 1; // +1 为换行符
	}
	content.reserve(totalSize);
	
	// 将每行内容追加到结果字符串中
	for (size_t i = 0; i < fileLines._lines.size(); ++i)
	{
		content += fileLines._lines[i];
		
		// 除了最后一行外，每行后面都添加换行符
		if (i < fileLines._lines.size() - 1)
		{
			content += '\n';
		}
	}
}

bool CheckLineMatch(const std::string& line1, const std::string& line2)
{
	// 找到第一个字符串去除尾部空白后的有效长度
	size_t len1 = line1.length();
	while (len1 > 0 && (line1[len1 - 1] == ' ' || line1[len1 - 1] == '\t' || 
	                    line1[len1 - 1] == '\r' || line1[len1 - 1] == '\n'))
	{
		len1--;
	}
	
	// 找到第二个字符串去除尾部空白后的有效长度
	size_t len2 = line2.length();
	while (len2 > 0 && (line2[len2 - 1] == ' ' || line2[len2 - 1] == '\t' || 
	                    line2[len2 - 1] == '\r' || line2[len2 - 1] == '\n'))
	{
		len2--;
	}
	
	// 如果有效长度不同，则不匹配
	if (len1 != len2)
		return false;
	
	// 比较有效部分的内容
	return std::memcmp(line1.data(), line2.data(), len1) == 0;
}


//从fileLines里startLine开始找到与chunk里头部最多行一样的位置
ReplaceChunks::Anchor FindReplaceStartAnchor(const ReplaceTargetFileLines& fileLines, ReplaceChunks::Chunk& chunk, int fileStartLine)
{
	if (chunk.lines.empty() || fileLines._lines.empty() || fileStartLine >= fileLines._lines.size())
	{
		return ReplaceChunks::Anchor();
	}

	int bestMatchStartLine = -1;
	int bestMatchCount = 0;
	int bestMatchStartLineInChunk = -1;

	// 遍历从fileStartLine开始的每一行，尝试匹配
	for (int k=0;k<chunk.lines.size()-1;k++)
	for (int i = fileStartLine; i < fileLines._lines.size(); i++)
	{
		// 计算当前位置能匹配的最大行数
		int matchCount = 0;
		for (int j = k; j < chunk.lines.size() && (i + j) < fileLines._lines.size(); j++)
		{
			if (CheckLineMatch(fileLines._lines[i + j-k] ,chunk.lines[j]))
			{
				matchCount++;
			}
			else
			{
				break; // 一旦不匹配就退出内层循环
			}
		}

		if (matchCount > bestMatchCount)
		{
			//检查anchor的内容是否足够有意义
			if (AreLinesAllEmpty(&fileLines._lines[i], matchCount))
				continue;
		}

		// 如果找到更好的匹配，更新最佳匹配信息
		if (matchCount > bestMatchCount)
		{
			bestMatchCount = matchCount;
			bestMatchStartLine = i;
			bestMatchStartLineInChunk = k;
			
			// 如果匹配了chunk的所有行，可以提前结束
			if (matchCount == chunk.lines.size())
			{
				break;
			}
		}
	}

	ReplaceChunks::Anchor anchor;
	anchor.lineIdx = bestMatchStartLine;
	anchor.lineCount = bestMatchCount;
	anchor.lineIdxInChunk = bestMatchStartLineInChunk;
	return anchor;
}



//从fileLines里[startLine,endLine)范围内找到与chunk里尾部最多行一样的位置
ReplaceChunks::Anchor FindReplaceEndAnchor(const ReplaceTargetFileLines& fileLines, ReplaceChunks::Chunk& chunk, int startLine, int endLine)
{
	if (chunk.lines.empty() || fileLines._lines.empty() || startLine >= fileLines._lines.size() || startLine >= endLine)
	{
		return ReplaceChunks::Anchor();
	}

	// 确保endLine不超过fileLines的大小
	int safeEndLine = (endLine < (int)fileLines._lines.size()) ? endLine : (int)fileLines._lines.size();

	int bestMatchEndLine = -1;
	int bestMatchCount = 0;

	int startAnchorEndLineInChunk = 0;
	if (chunk.startAnchor.IsValid())
		startAnchorEndLineInChunk = chunk.startAnchor.GetEndLineInChunk();

	if (startAnchorEndLineInChunk >= chunk.lines.size())
	{
		ReplaceChunks::Anchor anchor;
		anchor.lineIdx = chunk.startAnchor.GetEndLine();
		anchor.lineCount = 0;
		anchor.lineIdxInChunk = chunk.lines.size();
		return anchor;
	}

	for (int i=startLine;i<safeEndLine;i++)
	{
		// 计算当前位置能匹配的最大行数（从尾部匹配）
		int matchCount = 0;
		for (int j = chunk.lines.size() - 1, k = i; j >= startAnchorEndLineInChunk && k >= startLine; j--, k--)
		{
			if (CheckLineMatch(fileLines._lines[k], chunk.lines[j]))
			{
				matchCount++;
			}
			else
			{
				break; // 一旦不匹配就退出内层循环
			}
		}

		// 如果找到更好的匹配，更新最佳匹配信息
		if (matchCount > bestMatchCount)
		{
			bestMatchCount = matchCount;
			bestMatchEndLine = i + 1; // 存储匹配结束位置的下一行（即范围末尾）
			
			// 如果匹配了chunk的所有行，可以提前结束
			if (matchCount == chunk.lines.size())
			{
				break;
			}
		}
	}

	// 计算匹配的起始行
	int bestMatchStartLine = bestMatchEndLine - bestMatchCount;
	
	ReplaceChunks::Anchor anchor;
	anchor.lineIdx = bestMatchStartLine;
	anchor.lineCount = bestMatchCount;
	anchor.lineIdxInChunk = chunk.lines.size() - bestMatchCount;
	return anchor;
}

ReplaceChunks::Anchor FindPreviousValidAnchor(const ReplaceChunks& chunks, int chunkIdx)
{
	// 从当前chunk向前查找有效的锚点
	for (int i = chunkIdx - 1; i >= 0; i--)
	{
		// 先检查endAnchor是否有效
		if (chunks._chunks[i].endAnchor.IsValid())
		{
			return chunks._chunks[i].endAnchor;
		}
		// 再检查startAnchor是否有效
		if (chunks._chunks[i].startAnchor.IsValid())
		{
			return chunks._chunks[i].startAnchor;
		}
	}
	
	// 如果没有找到有效锚点，返回无效锚点
	return ReplaceChunks::Anchor();
}

ReplaceChunks::Anchor FindNextValidAnchor(const ReplaceChunks& chunks, int chunkIdx)
{
	// 从当前chunk向后查找有效的锚点
	for (int i = chunkIdx + 1; i < (int)chunks._chunks.size(); i++)
	{
		// 先检查startAnchor是否有效
		if (chunks._chunks[i].startAnchor.IsValid())
		{
			return chunks._chunks[i].startAnchor;
		}
		// 再检查endAnchor是否有效
		if (chunks._chunks[i].endAnchor.IsValid())
		{
			return chunks._chunks[i].endAnchor;
		}
	}
	
	// 如果没有找到有效锚点，返回无效锚点
	return ReplaceChunks::Anchor();
}

//根据chunks里各个chunk的代码行,到原始文件里匹配这个chunk应该替换文件里哪个区段(哪几行)
//chunks里的各个chunk是按照顺序排列的,chunk里的头部和尾部可能包含原文件里一些不需要改动的代码行,可以用来定位
//先依次寻找各个chunk的开始anchor,再在每个chunk与下一个chunk的起始anchor之间寻找这个chunk的终止anchor
void LocateReplaceChunks(ReplaceChunks& chunks, const ReplaceTargetFileLines &fileLines)
{
	if (chunks._chunks.empty() || fileLines._lines.empty())
		return;
	
	//找到每个chunk的起始anchor
	for (size_t i = 0; i < chunks._chunks.size(); i++)
	{
		ReplaceChunks::Chunk& chunk = chunks._chunks[i];

		int searchStartLine = 0;
		ReplaceChunks::Anchor prevAnchor = FindPreviousValidAnchor(chunks, i);
		if (prevAnchor.IsValid())
			searchStartLine = prevAnchor.GetEndLine();

		ReplaceChunks::Anchor startAnchor = FindReplaceStartAnchor(fileLines, chunk, searchStartLine);
		// 保存到chunk的startAnchor中
		chunk.startAnchor = startAnchor;
	}
	
	//在每个chunk的起始位置和下一个chunk的起始位置之间找结束位置
	for (size_t i = 0; i < chunks._chunks.size(); i++)
	{
		ReplaceChunks::Chunk& chunk = chunks._chunks[i];

		if (!chunk.startAnchor.IsValid())
			continue;

		int searchStartLine = 0;
		searchStartLine = chunk.startAnchor.GetEndLine();
// 		if (true)
// 		{
// 			ReplaceChunks::Anchor prevAnchor = FindPreviousValidAnchor(chunks, i);
// 			if (prevAnchor.IsValid())
// 				searchStartLine = prevAnchor.GetEndLine();
// 		}

		int searchEndLine = fileLines.GetLineCount();
		if (true)
		{
			ReplaceChunks::Anchor nextAnchor = FindNextValidAnchor(chunks, i);
			if (nextAnchor.IsValid())
				searchEndLine = nextAnchor.GetStartLine();
		}

		// 在指定范围内找到尾部匹配
		chunk.endAnchor = FindReplaceEndAnchor(fileLines, chunk, searchStartLine, searchEndLine);
	}

	for (size_t i = 0; i < chunks._chunks.size(); i++)
	{
		ReplaceChunks::Chunk& chunk = chunks._chunks[i];
		if (!chunk.startAnchor.IsValid())
			chunk.startAnchor = FindPreviousValidAnchor(chunks, i);
		if (!chunk.startAnchor.IsValid())
		{
			chunk.startAnchor = chunk.endAnchor;
			chunk.startAnchor.lineCount = 0;
		}
		if (!chunk.startAnchor.IsValid())
		{
			chunk.startAnchor = FindNextValidAnchor(chunks, i);
			chunk.startAnchor.lineCount = 0;
		}
	}

	for (size_t i = 0; i < chunks._chunks.size(); i++)
	{
		ReplaceChunks::Chunk& chunk = chunks._chunks[i];
		if (!chunk.endAnchor.IsValid())
		{
			chunk.endAnchor = chunk.startAnchor;
			chunk.endAnchor.lineCount = 0;
		}
	}

	ReplaceChunks::Anchor tailAnchor;
	tailAnchor.lineIdx = fileLines.GetLineCount();
	tailAnchor.lineCount = 0;
	for (size_t i = 0; i < chunks._chunks.size(); i++)
	{
		ReplaceChunks::Chunk& chunk = chunks._chunks[i];
		if (!chunk.startAnchor.IsValid())
			chunk.startAnchor = tailAnchor;
		if (!chunk.endAnchor.IsValid())
			chunk.endAnchor = tailAnchor;
	}
}

void ApplyReplaceChunks(const ReplaceChunks& chunks, ReplaceTargetFileLines& fileLines)
{
	// 从后向前处理每个chunk，避免前面的替换影响后面chunk的位置
	for (int i = chunks._chunks.size() - 1; i >= 0; i--)
	{
		const ReplaceChunks::Chunk& chunk = chunks._chunks[i];
		
		// 确保锚点有效
		if (!chunk.startAnchor.IsValid() || !chunk.endAnchor.IsValid())
			continue;
		
		int startLine = chunk.startAnchor.GetStartLine();
		int endLine = chunk.endAnchor.GetEndLine();
		
		// 检查范围有效性
		if (startLine < 0 || endLine > fileLines.GetLineCount() || startLine > endLine)
			continue;
		
		// 计算要替换的行数
		int linesToRemove = endLine - startLine;
		
		// 从文件中移除这些行
		if (linesToRemove > 0)
		{
			fileLines._lines.erase(
				fileLines._lines.begin() + startLine,
				fileLines._lines.begin() + endLine
			);
		}
		
		// 插入chunk中的行
		fileLines._lines.insert(
			fileLines._lines.begin() + startLine,
			chunk.lines.begin(),
			chunk.lines.end()
		);
	}
}