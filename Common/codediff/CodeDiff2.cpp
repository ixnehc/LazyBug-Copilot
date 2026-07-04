#include "stdh.h"
#include <cctype>
#include <map>
#include <vector>
#include <algorithm>

#include "dmp_diff.h"

#include "CodeDiff.h"


// 专门表示行的结构体
struct CodeLine
{
	std::wstring original;      // 原始行内容
	std::wstring trimmed;       // 清理后用于比较的内容
	int lineNumber;           // 行号（从1开始）

	CodeLine(const std::wstring& content, int num)
		: original(content), lineNumber(num)
	{
		trimmed = TrimCodeLineForCompareW(content);
	}
};

// 清除字符串中可以清除的空格，用于行比较
extern std::wstring TrimCodeLineForCompareW(const std::wstring& str);

// 判断两个字符串的是否足够相似,注意不要完全计算出相似度,只要发现相似度低于threshold就返回false
bool CheckStringSimilarEnoughW(const std::wstring& str1, const std::wstring& str2, float threshold)
{
	if (str1 == str2) return true;
	if (str1.empty() || str2.empty()) return false;

	// 使用简化的编辑距离算法，当相似度低于阈值时提前返回
	int len1 = (int)str1.length();
	int len2 = (int)str2.length();

	// 如果长度差异太大，直接返回false
	int maxLen = max(len1, len2);
	int minLen = min(len1, len2);
	if ((float)minLen / maxLen < threshold)
	{
		return false;
	}

	// 使用动态规划计算编辑距离，但设置阈值提前退出
	std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

	for (int i = 0; i <= len1; i++) dp[i][0] = i;
	for (int j = 0; j <= len2; j++) dp[0][j] = j;

	for (int i = 1; i <= len1; i++)
	{
		bool hasValidCell = false;
		for (int j = 1; j <= len2; j++)
		{
			if (str1[i - 1] == str2[j - 1])
			{
				dp[i][j] = dp[i - 1][j - 1];
			}
			else
			{
				dp[i][j] = 1 + min(min(dp[i - 1][j], dp[i][j - 1]), dp[i - 1][j - 1] );
			}

			// 检查当前位置的相似度是否还有希望达到阈值
			int maxPossibleSimilar = maxLen - dp[i][j];
			if ((float)maxPossibleSimilar / maxLen >= threshold)
			{
				hasValidCell = true;
			}
		}

		// 如果这一行没有任何位置能达到阈值，提前退出
		if (!hasValidCell)
		{
			return false;
		}
	}

	int editDistance = dp[len1][len2];
	float similarity = 1.0f - (float)editDistance / maxLen;
	return similarity >= threshold;
}

// 行相等性查询表
class LineEqualityTable
{
public:
	void Build(const std::vector<CodeLine>& oldLines, const std::vector<CodeLine>& newLines)
	{
		this->oldLines = &oldLines;
		this->newLines = &newLines;
		equalLevels.clear();

		// 预计算所有行对的相等级别
		for (int i = 0; i < oldLines.size(); i++)
		{
			for (int j = 0; j < newLines.size(); j++)
			{
				int level = CalculateEqualLevel(i, j);
				if (level > 0)
				{
					equalLevels[{i, j}] = level;
				}
			}
		}
	}

	// 判断两行是否相等
	bool AreEqual(int oldLineIndex, int newLineIndex) const
	{
		return GetEqualLevel(oldLineIndex, newLineIndex) > 0;
	}

	int GetEqualLevel(int oldLineIndex, int newLineIndex) const
	{
		auto it = equalLevels.find({ oldLineIndex, newLineIndex });
		return it != equalLevels.end() ? it->second : 0;
	}

private:
	const std::vector<CodeLine>* oldLines;
	const std::vector<CodeLine>* newLines;
	std::map<std::pair<int, int>, int> equalLevels;

	int CalculateEqualLevel(int oldLineIndex, int newLineIndex) const
	{
		const CodeLine& oldLine = (*oldLines)[oldLineIndex];
		const CodeLine& newLine = (*newLines)[newLineIndex];

		// 级别3：完全相同
		if (oldLine.original == newLine.original)
		{
			return 3;
		}

		// 级别2：清理后相同
		if (oldLine.trimmed == newLine.trimmed)
		{
			return 2;
		}

		// 级别1：相似度足够高
		if (CheckStringSimilarEnoughW(oldLine.trimmed, newLine.trimmed, 0.8f))
		{
			return 1;
		}

		return 0;
	}
};


// 辅助函数：将字符串分行（保留换行符）
void SplitIntoLinesW(const std::wstring& code, std::vector<CodeLine>& lines)
{
    lines.clear();
    std::wstring line;
    int lineNumber = 1;

    for (size_t i = 0; i < code.size(); ++i)
    {
        wchar_t c = code[i];
        line.push_back(c);

        if (c == L'\r')
        {
            // 处理 \r\n 作为一个换行
            if (i + 1 < code.size() && code[i + 1] == L'\n')
            {
                line.push_back(L'\n');
                ++i;
            }
            lines.emplace_back(line, lineNumber++);
            line.clear();
        }
        else if (c == L'\n')
        {
            lines.emplace_back(line, lineNumber++);
            line.clear();
        }
    }

    // 如果最后一行没有换行符，也要添加
    if (!line.empty())
    {
        lines.emplace_back(line, lineNumber);
    }
}



// 自定义比较器，用于Myers算法比较行
class CodeLineIndex
{
public:
	int lineIndex;
	bool isOldCode;  // true表示来自旧代码，false表示来自新代码
	const LineEqualityTable& euqlityTable;

	CodeLineIndex(int index, bool isOld, const LineEqualityTable& table)
		: lineIndex(index), isOldCode(isOld), euqlityTable(table) {
	}

	// 重载相等运算符，用于Myers算法比较
	bool operator==(const CodeLineIndex& other) const
	{
		// 如果都来自同一类型代码，不能相等
		if (isOldCode == other.isOldCode) {
			return false;
		}

		// 使用成员的行相等性查询表判断
		if (isOldCode) {
			return euqlityTable.AreEqual(lineIndex, other.lineIndex);
		}
		else {
			return euqlityTable.AreEqual(other.lineIndex, lineIndex);
		}
	}

	// 重载不等运算符
	bool operator!=(const CodeLineIndex& other) const
	{
		return !(*this == other);
	}
};

// 为std::vector<CodeLineIndex>特化hash函数，用于Myers算法
namespace std
{
	template<>
	struct hash<CodeLineIndex>
	{
		size_t operator()(const CodeLineIndex& idx) const
		{
			// 简单的hash组合
			return hash<int>()(idx.lineIndex) ^ (hash<bool>()(idx.isOldCode) << 1);
		}
	};
}

// 创建行索引向量用于Myers算法
std::vector<CodeLineIndex> CreateLineIndices(const std::vector<CodeLine>& lines, bool isOldCode, const LineEqualityTable& equalityTable)
{
	std::vector<CodeLineIndex> indices;
	indices.reserve(lines.size());

	for (int i = 0; i < lines.size(); i++)
	{
		indices.emplace_back(i, isOldCode, equalityTable);
	}

	return indices;
}


// 实现MakeCodeComparing_Chars2函数
void MakeCodeComparing_Chars2(const std::string& oldCode, const std::string& newCode, CodeComparingChars& comparingContent)
{
	// 清空结果
	comparingContent.content.clear();
	comparingContent.charTypes.clear(); 

	// 将UTF-8字符串转换为宽字符串
	std::wstring wOldCode = utf8_to_widechar(oldCode.c_str());
	std::wstring wNewCode = utf8_to_widechar(newCode.c_str());

	// 1. 分行
	std::vector<CodeLine> oldLines, newLines;
	SplitIntoLinesW(wOldCode, oldLines);
	SplitIntoLinesW(wNewCode, newLines);

	// 2. 建立行相等性查询表
	LineEqualityTable lineEqualTable;
	lineEqualTable.Build(oldLines, newLines);

	// 3. 使用Myers算法进行行级比较
	// 创建行索引向量
	std::vector<CodeLineIndex> oldLineIndices = CreateLineIndices(oldLines, true, lineEqualTable);
	std::vector<CodeLineIndex> newLineIndices = CreateLineIndices(newLines, false, lineEqualTable);

	// 使用Myers算法进行行级diff
	MyersDiff<std::vector<CodeLineIndex>> lineDiff(oldLineIndices, newLineIndices);
	const auto& lineDiffs = lineDiff.diffs();

	// 4. 根据行级diff结果生成字符级比较内容
	for (const auto& diff : lineDiffs)
	{
		switch (diff.operation)
		{
		case DiffOp_Equal:
			// 相等的行，进行字符级比较
		{
			auto lineRange = diff.text;
			for (auto it = lineRange.from; it != lineRange.till; ++it)
			{
				const CodeLineIndex& oldLineIdx = *it;
				// 找到对应的新行索引
				const CodeLineIndex* newLineIdx = nullptr;
				for (auto newIt = newLineIndices.begin(); newIt != newLineIndices.end(); ++newIt)
				{
					if (oldLineIdx == *newIt)
					{
						newLineIdx = &(*newIt);
						break;
					}
				}

				if (newLineIdx)
				{
					const std::wstring& oldLineContent = oldLines[oldLineIdx.lineIndex].original;
					const std::wstring& newLineContent = newLines[newLineIdx->lineIndex].original;

					// 使用Myers算法进行字符级比较
					MyersDiff<std::wstring> charDiff(oldLineContent, newLineContent);
					const auto& charDiffs = charDiff.diffs();

					// 处理字符级差异
					for (const auto& charDiff : charDiffs)
					{
						std::wstring diffText(charDiff.text.from, charDiff.text.till);

						switch (charDiff.operation)
						{
						case DiffOp_Equal:
							comparingContent.content += diffText;
							for (size_t i = 0; i < diffText.length(); i++)
							{
								comparingContent.charTypes.push_back(CodeComparingChars::Both);
							}
							break;
						case DiffOp_Delete:
							comparingContent.content += diffText;
							for (size_t i = 0; i < diffText.length(); i++)
							{
								comparingContent.charTypes.push_back(CodeComparingChars::OldCode);
							}
							break;
						case DiffOp_Insert:
							comparingContent.content += diffText;
							for (size_t i = 0; i < diffText.length(); i++)
							{
								comparingContent.charTypes.push_back(CodeComparingChars::NewCode);
							}
							break;
						}
					}
				}
			}
		}
		break;

		case DiffOp_Delete:
			// 删除的行
		{
			auto lineRange = diff.text;
			for (auto it = lineRange.from; it != lineRange.till; ++it)
			{
				const CodeLineIndex& lineIdx = *it;
				const std::wstring& lineContent = oldLines[lineIdx.lineIndex].original;
				comparingContent.content += lineContent;
				for (size_t i = 0; i < lineContent.length(); i++)
				{
					comparingContent.charTypes.push_back(CodeComparingChars::OldCode);
				}
			}
		}
		break;

		case DiffOp_Insert:
			// 插入的行
		{
			auto lineRange = diff.text;
			for (auto it = lineRange.from; it != lineRange.till; ++it)
			{
				const CodeLineIndex& lineIdx = *it;
				const std::wstring& lineContent = newLines[lineIdx.lineIndex].original;
				comparingContent.content += lineContent;
				for (size_t i = 0; i < lineContent.length(); i++)
				{
					comparingContent.charTypes.push_back(CodeComparingChars::NewCode);
				}
			}
		}
		break;
		}
	}
}
