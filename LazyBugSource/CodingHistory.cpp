#include "stdh.h"

#include "CodingHistory.h"
#include "../Common/codediff/dmp_diff.h"

//注意,Split后的每一行都包含行末的所有换行符
void SplitLines(const std::string& str, std::vector<StringSegment>& lines)
{
	lines.clear();
	if (str.empty())
		return;

	const char* base = str.c_str();
	int len = (int)str.size();
	int lineStart = 0;

	for (int i = 0; i < len; )
	{
		if (str[i] == '\r')
		{
			if (i + 1 < len && str[i + 1] == '\n')
			{
				// 处理 \r\n，包含换行符
				i += 2;
				lines.push_back({ base, lineStart, i });
				lineStart = i;
			}
			else
			{
				// 单独 \r，包含换行符
				i += 1;
				lines.push_back({ base, lineStart, i });
				lineStart = i;
			}
		}
		else if (str[i] == '\n')
		{
			// 单独 \n，包含换行符
			i += 1;
			lines.push_back({ base, lineStart, i });
			lineStart = i;
		}
		else
		{
			i++;
		}
	}

	// 处理最后一行（即使没有换行符）
	if (lineStart < len)
	{
		lines.push_back({ base, lineStart, len });
	}
}


bool CCodingHistory::Replace::IsEmpty() const
{
	return filePath.empty();
}

// CCodingHistory 实现
CCodingHistory::CCodingHistory()
{
}

void CCodingHistory::SetCurEditFile(const char* filePath, const std::string& fileContent)
{
	std::string newFilePath = filePath;

	// 如果是不同的文件，保存当前的Replace
	if (_filePath != newFilePath)
	{
		if (!_curReplace.IsEmpty())
		{
			_AddHistory(_curReplace);
			_curReplace = Replace();
		}

		// 开始新文件的编辑
		_filePath = newFilePath;
		_fileBaseContent = fileContent;
		SplitLines(_fileBaseContent, _fileBaseLines);
	}
}

bool CCodingHistory::Edit(const char* filePath,const std::string& fileContent)
{
	if (_filePath.empty())
		return false;
	if (!(_filePath == filePath))
		return false;

	// 生成Replace对象
	std::vector<Replace> replaces = _GenerateReplaces(_fileBaseContent, fileContent);

	if (replaces.empty())
		return true;

	// 如果没有当前Replace，直接使用第一个作为当前Replace
	if (_curReplace.IsEmpty())
	{
		if (!replaces.empty())
		{
			_curReplace = replaces[0];

			// 剩余的Replace加入历史记录并Apply
			for (size_t i = 1; i < replaces.size(); i++)
			{
				_ApplyReplace(replaces[i]);
			}
		}
		return true;
	}

	// 计算每个Replace与当前Replace的距离，并排序
	std::vector<std::pair<int, int>> distanceAndIndex; // (距离, 索引)

	for (size_t i = 0; i < replaces.size(); i++)
	{
		int distance = _CalcDistanceBetweenReplaces(_curReplace, replaces[i]);
		distanceAndIndex.push_back({ distance, (int)i });
	}

	// 按距离排序，距离近的在前面
	std::sort(distanceAndIndex.begin(), distanceAndIndex.end());

	// 依次Apply Replace，越近的越先Apply
	for (size_t i = 0; i < distanceAndIndex.size(); i++)
	{
		int replaceIndex = distanceAndIndex[i].second;
		const Replace& replace = replaces[replaceIndex];

		if (i == distanceAndIndex.size() - 1)
		{
			_curReplace = replace;
		}
		else
		{
			_ApplyReplace(replace);
		}
	}
	return true;
}

std::vector<CCodingHistory::Replace> CCodingHistory::_GenerateReplaces(
	const std::string& oldContent, const std::string& newContent)
{
	std::vector<Replace> result;

	// 分行
	std::vector<StringSegment> oldLines, newLines;
	SplitLines(oldContent, oldLines);
	SplitLines(newContent, newLines);

	// 进行diff
	MyersDiff<std::vector<StringSegment>> diff(oldLines, newLines);

	// 分析diff结果，合并相邻的修改
	Replace currentReplace;
	currentReplace.filePath = _filePath;

	int oldLineIdx = 0;
	int newLineIdx = 0;
	bool hasChanges = false;
	int changeStartLine = -1;

	for (const auto& diffItem : diff.diffs())
	{
		switch (diffItem.operation)
		{
		case DiffOp_Equal:
		{
			int lineCount = (int)diffItem.text.size();

			// 如果有未处理的修改，且相等部分较长，则结束当前Replace
			if (hasChanges && lineCount > 5)
			{
				// 完成当前Replace
				currentReplace.oldCodeRange.start = changeStartLine;
				currentReplace.oldCodeRange.end = oldLineIdx;

				if (!currentReplace.IsEmpty())
				{
					result.push_back(currentReplace);
				}

				// 重置
				currentReplace = Replace();
				currentReplace.filePath = _filePath;
				hasChanges = false;
				changeStartLine = -1;
			}
			else if (hasChanges)
			{
				// 如果有修改且相等部分不长，将相等内容添加到当前Replace
				for (auto it = diffItem.text.from; it != diffItem.text.till; ++it)
				{
					std::string lineContent = it->toString();
					currentReplace.oldCode += lineContent;
					currentReplace.newCode += lineContent;
				}
			}

			oldLineIdx += lineCount;
			newLineIdx += lineCount;
		}
		break;

		case DiffOp_Delete:
		{
			if (!hasChanges)
			{
				changeStartLine = oldLineIdx;
				hasChanges = true;
			}

			// 添加删除的内容到oldCode
			for (auto it = diffItem.text.from; it != diffItem.text.till; ++it)
			{
				currentReplace.oldCode += it->toString();
				oldLineIdx++;
			}
		}
		break;

		case DiffOp_Insert:
		{
			if (!hasChanges)
			{
				changeStartLine = oldLineIdx;
				hasChanges = true;
			}

			// 添加插入的内容到newCode
			for (auto it = diffItem.text.from; it != diffItem.text.till; ++it)
			{
				currentReplace.newCode += it->toString();
				newLineIdx++;
			}
		}
		break;
		}
	}

	// 处理最后的修改
	if (hasChanges)
	{
		currentReplace.oldCodeRange.start = changeStartLine;
		currentReplace.oldCodeRange.end = oldLineIdx;

		if (!currentReplace.IsEmpty())
		{
			result.push_back(currentReplace);
		}
	}

	// 后处理：为每个Replace添加前后的上下文行
	_AddContextToReplaces(result, oldLines, newLines);

	return result;
}

void CCodingHistory::_ApplyReplace(const Replace& replace)
{
	if (replace.IsEmpty() || replace.oldCodeRange.start >= _fileBaseLines.size())
		return;

	// 确保范围有效
	int startLine = replace.oldCodeRange.start;
	int endLine = min((int)replace.oldCodeRange.end, (int)_fileBaseLines.size());

	if (startLine >= endLine)
		return;

	// 计算要替换的字符位置
	// 由于StringSegment现在包含换行符，直接使用start和end位置
	int startPos = _fileBaseLines[startLine].start;
	int endPos = (endLine >= _fileBaseLines.size()) ?
		(int)_fileBaseContent.size() :
		_fileBaseLines[endLine].start;

	// 构建新内容
	std::string newContent;

	// 添加替换前的内容
	if (startPos > 0)
	{
		newContent.append(_fileBaseContent, 0, startPos);
	}

	// 添加新的代码内容
	if (!replace.newCode.empty())
	{
		newContent += replace.newCode;
	}

	// 添加替换后的内容
	if (endPos < (int)_fileBaseContent.size())
	{
		newContent.append(_fileBaseContent, endPos, _fileBaseContent.size() - endPos);
	}

	// 更新基准内容和行信息
	_fileBaseContent = newContent;
	SplitLines(_fileBaseContent, _fileBaseLines);

	_AddHistory(replace);
}

int CCodingHistory::_CalcDistanceBetweenReplaces(const Replace& replaceA, const Replace& replaceB)
{
	// 如果不是同一个文件，返回最大距离
	if (replaceA.filePath != replaceB.filePath)
		return INT_MAX;

	// 获取两个Replace的行范围
	int startA = replaceA.oldCodeRange.start;
	int endA = replaceA.oldCodeRange.end;
	int startB = replaceB.oldCodeRange.start;
	int endB = replaceB.oldCodeRange.end;

	// 检查是否有重叠
	// 重叠条件：A的开始 < B的结束 && B的开始 < A的结束
	if (startA < endB && startB < endA)
	{
		// 有重叠，距离为0
		return 0;
	}

	// 没有重叠，计算距离
	if (endA <= startB)
	{
		// A在B之前，距离是B的开始到A的结束
		return startB - endA;
	}
	else
	{
		// B在A之前，距离是A的开始到B的结束
		return startA - endB;
	}
}

void CCodingHistory::_AddContextToReplaces(std::vector<Replace>& replaces, 
	const std::vector<StringSegment>& oldLines, const std::vector<StringSegment>& newLines)
{
	const int CONTEXT_LINES = 3; // 前后各添加3行上下文

	for (Replace& replace : replaces)
	{
		if (replace.IsEmpty())
			continue;

		int oldStart = replace.oldCodeRange.start;
		int oldEnd = replace.oldCodeRange.end;

		// 计算扩展后的范围
		int newOldStart = max(0, oldStart - CONTEXT_LINES);
		int newOldEnd = min((int)oldLines.size(), oldEnd + CONTEXT_LINES);

		// 如果范围没有扩展，跳过
		if (newOldStart == oldStart && newOldEnd == oldEnd)
			continue;

		// 重新构建oldCode和newCode
		std::string newOldCode;
		std::string newNewCode;

		// 添加前置上下文
		for (int i = newOldStart; i < oldStart; i++)
		{
			std::string lineContent = oldLines[i].toString();
			newOldCode += lineContent;
			newNewCode += lineContent; // 上下文在新旧版本中都相同
		}

		// 添加原有的修改内容
		newOldCode += replace.oldCode;
		newNewCode += replace.newCode;

		// 添加后置上下文
		for (int i = oldEnd; i < newOldEnd; i++)
		{
			std::string lineContent = oldLines[i].toString();
			newOldCode += lineContent;
			newNewCode += lineContent; // 上下文在新旧版本中都相同
		}

		// 更新Replace
		replace.oldCode = newOldCode;
		replace.newCode = newNewCode;
		replace.oldCodeRange.start = newOldStart;
		replace.oldCodeRange.end = newOldEnd;
	}
}

void CCodingHistory::_AddHistory(const Replace& replace)
{
	_history.push_back(replace);
	
	// 如果超过上限，从front弹出
	if (_history.size() > 10)
	{
		_history.pop_front();
	}
}

void CCodingHistory::Dump(std::string& str, int maxByte)
{
	str.clear();
	if (_history.empty())
		return;

	// 从最近的记录开始遍历（反向迭代器）
	auto it = _history.rbegin();
	int totalBytes = 0;
	bool hasOutput = false;

	for (; it != _history.rend(); ++it)
	{
		const Replace& replace = *it;

		// 构建单条记录的输出字符串
		std::string recordStr;
		recordStr += "File: " + replace.filePath + "\n";
		recordStr += "Old Code:\n" + replace.oldCode + "\n";
		recordStr += "New Code:\n" + replace.newCode + "\n";
		recordStr += "---\n";

		// 检查添加后是否超过maxByte
		int newTotalBytes = totalBytes + (int)recordStr.size();
		if (newTotalBytes > maxByte && hasOutput)
		{
			// 如果已经输出过记录，且添加这条会超过限制，则停止
			break;
		}

		// 添加到输出字符串
		str += recordStr;
		totalBytes = newTotalBytes;
		hasOutput = true;
	}
}
