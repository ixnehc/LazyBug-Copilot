#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>

#include "../stringparser/stringparser.h"

// 行信息数据结构
struct LineInfo
{
	std::string content;  // 行内容
	std::string contentForCompare;

	LineInfo() {}
	LineInfo(const std::string& c) : content(c) 
	{
		contentForCompare = content;
//		RemoveTailBlank(contentForCompare);
	}
};

enum class CodeDiffType
{
	UNCHANGED,  // 未修改
	ADDED,      // 添加行
	DELETED,    // 删除行
	MODIFIED    // 修改行
};

struct CodeDiffLine
{
	CodeDiffType type;
	int oldLineNumber;  // 字符串1中的行号
	int newLineNumber;  // 字符串2中的行号
	std::string oldContent;  // 字符串1中的内容
	std::string newContent;  // 字符串2中的内容

	CodeDiffLine(CodeDiffType t, int ln1, int ln2, const std::string& c1, const std::string& c2)
		: type(t), oldLineNumber(ln1), newLineNumber(ln2), oldContent(c1), newContent(c2)
	{
	}
};

// 求解最长公共子序列(LCS)问题的辅助函数
extern void BuildLCSTable(const std::vector<LineInfo>& lines1,
	const std::vector<LineInfo>& lines2,
	std::vector<std::vector<int>>& lcsTable);

// 从LCS表生成差异数据
extern void GenerateCodeDiff(const std::vector<LineInfo>& lines1,
	const std::vector<LineInfo>& lines2,
	const std::vector<std::vector<int>>& lcsTable, std::deque<CodeDiffLine>& diff);
// 主函数：比较两个代码字符串
extern void CompareCodeStrings(const std::string& oldCode, const std::string& newCode, std::deque<CodeDiffLine>& diffs);

struct CodeComparingLines
{
	void Clear()
	{
		content.clear();
		lineOrigins.clear();
		oldLineIndices.clear();
		newLineIndices.clear();
	}
	struct LineOrigin
	{
		enum Type
		{
			Both,
			NewCode,
			OldCode
		};
		Type tp;
		int oldLineNumber;
		int newLineNumber;
	};

	int NavigateDiff(int curLine, bool isNext) const;

	std::string content;//比较内容,用于显示比较结果的文本文件
	std::vector< LineOrigin> lineOrigins;//比较内容每一行的来源
	std::vector< int> oldLineIndices;//旧文件中每一行对应比较内容哪一行
	std::vector< int> newLineIndices;//新文件中每一行对应比较内容哪一行
};

struct CodeComparingChars
{
	enum CharType
	{
		Both,
		NewCode,
		OldCode
	};
	std::string content;//比较内容,用于显示比较结果的文本文件
	std::vector<CharType> charTypes;//content中每个字符的类型
};

extern void MakeCodeComparing_Lines(const std::string& oldCode, const std::string& newCode, CodeComparingLines &comparingContent);

extern void MakeCodeComparing_Chars(const std::string& oldCode, const std::string& newCode, CodeComparingChars& comparingContent);

