#include "stdh.h"
#include <cctype>
#include <map>
#include <vector>
#include <algorithm>

#include "dmp_diff.h"

#include "CodeDiff.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


bool CheckSameLine(const std::string& lineA, const std::string& lineB)
{
	// 不忽略头部空白字符，只处理尾部空白字符
	size_t startA = 0;
	size_t startB = 0;

	// 找到最后一个非空白字符的位置
	size_t endA = lineA.size();
	while (endA > startA && std::isspace((BYTE)lineA[endA - 1]))
	{
		endA--;
	}

	size_t endB = lineB.size();
	while (endB > startB && std::isspace((BYTE)lineB[endB - 1])) 
	{
		endB--;
	}

	// 长度不同则肯定不同
	if ((endA - startA) != (endB - startB)) 
	{
		return false;
	}

	// 比较实际内容（包含头部空白字符）
	for (size_t i = 0; i < (endA - startA); i++) 
	{
		if (lineA[startA + i] != lineB[startB + i]) 
		{
			return false;
		}
	}

	return true;
}

void BuildLCSTable(const std::vector<LineInfo>& oldLines,
	const std::vector<LineInfo>& newLines,
	std::vector<std::vector<int>>& lcsTable)
{
	int m =(int) oldLines.size();
	int n = (int)newLines.size();

	// ��ʼ��LCS��
	lcsTable.resize(m + 1, std::vector<int>(n + 1, 0));

	// ����LCSֵ
	for (int i = 1; i <= m; ++i)
	{
		for (int j = 1; j <= n; ++j)
		{
			if (CheckSameLine(oldLines[i - 1].contentForCompare, newLines[j - 1].contentForCompare))
// 			if (oldLines[i - 1].contentForCompare == newLines[j - 1].contentForCompare)
			{
				lcsTable[i][j] = lcsTable[i - 1][j - 1] + 1;
			}
			else
			{
				if (i == j)
				{
					int v = 0;
					v++;
				}

//				lcsTable[i][j] = max(lcsTable[i - 1][j], lcsTable[i][j - 1]);
				lcsTable[i][j] = lcsTable[i - 1][j] > lcsTable[i][j - 1] ? lcsTable[i - 1][j] : lcsTable[i][j - 1];
			}
		}
	}
}

// ��LCS�����ɲ�������
void GenerateCodeDiff(const std::vector<LineInfo>& oldLines,
	const std::vector<LineInfo>& newLines,
	const std::vector<std::vector<int>>& lcsTable, std::deque<CodeDiffLine>& diff)
{
	diff.clear();

	int i = (int)oldLines.size();
	int j = (int)newLines.size();

	// ����LCS�������ɲ���
	while (i > 0 || j > 0)
	{
		if (i > 0 && j > 0 && CheckSameLine(oldLines[i - 1].contentForCompare ,newLines[j - 1].contentForCompare))
// 		if (i > 0 && j > 0 && oldLines[i - 1].contentForCompare == newLines[j - 1].contentForCompare)
		{
			// ��ͬ��
			diff.insert(diff.begin(), CodeDiffLine(CodeDiffType::UNCHANGED, 
                                                 i, 
                                                 j, 
                                                 oldLines[i - 1].content, 
                                                 newLines[j - 1].content));
			--i;
			--j;
		}
		else if (j > 0 && (i == 0 || lcsTable[i][j - 1] >= lcsTable[i - 1][j]))
		{
			// ������
			diff.insert(diff.begin(), CodeDiffLine(CodeDiffType::ADDED, 
                                                 0, 
                                                 j, 
                                                 "", 
                                                 newLines[j - 1].content));
			--j;
		}
		else if (i > 0 && (j == 0 || lcsTable[i][j - 1] < lcsTable[i - 1][j]))
		{
			// ɾ����
			diff.insert(diff.begin(), CodeDiffLine(CodeDiffType::DELETED, 
                                                 i, 
                                                 0, 
                                                 oldLines[i - 1].content, 
                                                 ""));
			--i;
		}
	}

	// �������ڵ����Ӻ�ɾ��Ϊ�޸�
// 	for (size_t i = 0; i < diff.size() - 1; ++i)
// 	{
// 		if (diff[i].type == CodeDiffType::DELETED && diff[i + 1].type == CodeDiffType::ADDED)
// 		{
// 			diff[i].type = CodeDiffType::MODIFIED;
// 			diff[i].newLineNumber = diff[i + 1].newLineNumber;
// 			diff[i].newContent = diff[i + 1].newContent;
// 			diff.erase(diff.begin() + i + 1);
// 		}
// 	}
}

void PostProcessCodeDiff(std::deque<CodeDiffLine>& diff)
{
	for (int i = 0;i < diff.size();i++)
	{
		if (i > 0)
		{
			if ((diff[i].type == CodeDiffType::DELETED)&& (diff[i-1].type == CodeDiffType::UNCHANGED))
			{
				int iNextNotDelete = -1;
				for (int j = i + 1;j < diff.size();j++)
				{
					if (diff[j].type != CodeDiffType::DELETED)
					{
						iNextNotDelete = j;
						break;
					}
				}

				if (iNextNotDelete >= 0)
				{
					if (diff[iNextNotDelete].type == CodeDiffType::UNCHANGED)
					{
						if (CheckSameLine(diff[i].oldContent, diff[iNextNotDelete].oldContent))
						{
							diff[i].type = CodeDiffType::UNCHANGED;
							diff[iNextNotDelete].type = CodeDiffType::DELETED;
						}
					}
				}
			}
		}
	}

	for (int i = 0;i < diff.size();i++)
	{
		if (i > 0)
		{
			if ((diff[i].type == CodeDiffType::ADDED) && (diff[i - 1].type == CodeDiffType::UNCHANGED))
			{
				int iNextNotAdd= -1;
				for (int j = i + 1;j < diff.size();j++)
				{
					if (diff[j].type != CodeDiffType::ADDED)
					{
						iNextNotAdd = j;
						break;
					}
				}

				if (iNextNotAdd >= 0)
				{
					if (diff[iNextNotAdd].type == CodeDiffType::UNCHANGED)
					{
						if (CheckSameLine(diff[i].newContent, diff[iNextNotAdd].oldContent))
						{
							diff[i].type = CodeDiffType::UNCHANGED;
							diff[i].oldContent = diff[i].newContent;
							diff[iNextNotAdd].type = CodeDiffType::ADDED;
							diff[iNextNotAdd].newContent = diff[iNextNotAdd].oldContent;
						}
					}
				}
			}
		}
	}
}

//把一行的头部的空白部分,按照tabCount个空格转换为一个tab进行转换,多余的空格也转换为一个tab
void NormalizeCodeLineHead(std::string& line, int tabCount)
{
	if (tabCount <= 0) return;
	
	std::string result;
	result.reserve(line.size()); // 预留空间，转换后通常会更短
	
	size_t i = 0;
	int spaceCount = 0;
	
	// 处理行头部的空白字符
	while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
	{
		if (line[i] == '\t')
		{
			// 保留原有的tab
			result += '\t';
		}
		else if (line[i] == ' ')
		{
			spaceCount++;
			// 当累积的空格数达到tabCount时，转换为一个tab
			if (spaceCount >= tabCount)
			{
				result += '\t';
				spaceCount = 0; // 重置空格计数
			}
		}
		i++;
	}
	
	// 处理剩余的空格（不足tabCount个的空格）
	if (spaceCount > 0)
	{
		// 多余的空格也转换为一个tab
		result += '\t';
	}
	
	// 添加剩余的非空白字符部分
	if (i < line.size())
	{
		result.append(line, i, line.size() - i);
	}
	
	line = std::move(result);
}

//根据一个代码文件里的所有行,侦测这个文件的tab的空格数量,如果无法侦测到,返回false
bool DetectCodeTabCount(const std::vector<std::string> &lines, int& tabCount)
{
	std::map<int, int> indentCounts; // 缩进级别 -> 出现次数
	
	for (const auto& line : lines)
	{
		if (line.empty()) continue;
		
		// 计算行首的空格数量（忽略tab，只统计空格）
		int spaceCount = 0;
		bool hasTab = false;
		
		for (size_t i = 0; i < line.size(); i++)
		{
			if (line[i] == ' ')
			{
				spaceCount++;
			}
			else if (line[i] == '\t')
			{
				hasTab = true;
				break; // 遇到tab就停止，这行不适合用来推断
			}
			else
			{
				break; // 遇到非空白字符
			}
		}
		
		// 只统计纯空格缩进的行
		if (!hasTab && spaceCount > 0)
		{
			indentCounts[spaceCount]++;
		}
	}
	
	if (indentCounts.size() < 2)
	{
		// 缩进级别太少，无法推断
		return false;
	}
	
	// 寻找最大公约数来确定基本缩进单位
	std::vector<int> indentLevels;
	for (const auto& pair : indentCounts)
	{
		indentLevels.push_back(pair.first);
	}
	
	// 计算所有缩进级别的最大公约数
	int gcd = indentLevels[0];
	for (size_t i = 1; i < indentLevels.size(); i++)
	{
		// 手动实现最大公约数算法
		int a = gcd;
		int b = indentLevels[i];
		while (b != 0) {
			int temp = b;
			b = a % b;
			a = temp;
		}
		gcd = a;
	}
	
	// 验证这个gcd是否合理（通常tab对应2, 4, 8个空格）
	if (gcd >= 2 && gcd <= 8)
	{
		tabCount = gcd;
		return true;
	}
	
	// 如果gcd不合理，尝试寻找最常见的合理缩进
	std::map<int, int> possibleTabCounts; // 可能的tab数量 -> 支持度
	
	for (const auto& pair0 : indentCounts)
	{
		int possibleTab = pair0.first;
		int support = 0;
		for (const auto& pair : indentCounts)
		{
			if (pair.first % possibleTab == 0)
			{
				support += pair.second; // 按出现次数加权
			}
		}
		if (support > 0)
		{
			possibleTabCounts[possibleTab] = support;
		}
	}
	
	if (possibleTabCounts.empty())
	{
		return false;
	}
	
	// 选择支持度最高的tab数量
	int bestTab = 0;
	int maxSupport = 0;
	for (const auto& pair : possibleTabCounts)
	{
		if (pair.second > maxSupport)
		{
			maxSupport = pair.second;
			bestTab = pair.first;
		}
	}
	
	tabCount = bestTab;
	return true;
}


//把代码行的头部的空格转化为tab
void NormalizeLinesHead(std::vector<std::string>& lines)
{
	if (lines.empty()) return;
	
	// 首先检测代码的tab空格数量
	int tabCount = 4; // 默认值
	if (!DetectCodeTabCount(lines, tabCount))
	{
		// 如果无法检测到，使用常见的默认值4
		tabCount = 4;
	}
	
	// 对每一行进行空格到tab的转换
	for (auto& line : lines)
	{
		// 去除尾部空白字符用于比较
		size_t end = line.size();
		while (end > 0 && std::isspace((unsigned char)line[end - 1])) 
		{
			end--;
		}
		line = line.substr(0, end);
		NormalizeCodeLineHead(line, tabCount);
	}
}


void CompareCodeStrings(const std::string& oldCode, const std::string& newCode, std::deque<CodeDiffLine>& diffs)
{
	std::vector<LineInfo> oldLines, newLines;
	std::istringstream oldStream(oldCode);
	std::istringstream newStream(newCode);
	std::string line;

	while (std::getline(oldStream, line))
	{
		oldLines.push_back(LineInfo(line));
	}

	while (std::getline(newStream, line))
	{
		newLines.push_back(LineInfo(line));
	}

// 	NormalizeLinesHead(oldLines);
// 	NormalizeLinesHead(newLines);

	std::vector<std::vector<int>> lcsTable;
	BuildLCSTable(oldLines, newLines, lcsTable);

	GenerateCodeDiff(oldLines, newLines, lcsTable, diffs);

	PostProcessCodeDiff(diffs);
}

void CompareCodeStrings_Myers(const std::string& oldCode, const std::string& newCode, std::deque<CodeDiffLine>& diffs)
{
    diffs.clear();

    // 先把 oldCode 和 newCode 按行分割
    std::vector<std::string> oldLines,newLines;
	SplitLines(oldCode, oldLines);
	SplitLines(newCode, newLines);

	NormalizeLinesHead(oldLines);
	NormalizeLinesHead(newLines);

    // 执行 Myers 差异比较
    MyersDiff<std::vector<std::string>> myers(oldLines, newLines);
    const auto& d = myers.diffs();

    // 逐个转换为 CodeDiffLine
    int oldIndex = 1;
    int newIndex = 1;
    for (const auto& diff : d)
    {
        switch (diff.operation)
        {
        case DiffOp_Equal:
            for (auto it = diff.text.from; it != diff.text.till; ++it)
            {
                diffs.emplace_back(CodeDiffType::UNCHANGED,
                                   oldIndex,
                                   newIndex,
                                   *it,
                                   *it);
                ++oldIndex;
                ++newIndex;
            }
            break;
        case DiffOp_Insert:
            for (auto it = diff.text.from; it != diff.text.till; ++it)
            {
                diffs.emplace_back(CodeDiffType::ADDED,
                                   0,
                                   newIndex,
                                   "",
                                   *it);
                ++newIndex;
            }
            break;
        case DiffOp_Delete:
            for (auto it = diff.text.from; it != diff.text.till; ++it)
            {
                diffs.emplace_back(CodeDiffType::DELETED,
                                   oldIndex,
                                   0,
                                   *it,
                                   "");
                ++oldIndex;
            }
            break;
        }
    }

 	PostProcessCodeDiff(diffs);
}


void MakeCodeComparing_Lines(const std::string& oldCode, const std::string& newCode, CodeComparingLines& comparingContent)
{
	// ��ձȽ�����
	comparingContent.content.clear();
	comparingContent.lineOrigins.clear();
	comparingContent.oldLineIndices.clear();
	comparingContent.newLineIndices.clear();

	// ��ȡ������Ϣ
	std::deque<CodeDiffLine> diffs;
//	CompareCodeStrings(oldCode, newCode, diffs);
	CompareCodeStrings_Myers(oldCode, newCode, diffs);

	// ��ʼ���к�ӳ��
	comparingContent.oldLineIndices.resize(oldCode.size(), -1);
	comparingContent.newLineIndices.resize(newCode.size(), -1);

	// ���ɱȽ�����
	std::ostringstream oss;
	int currentLine = 0;

	for (const auto& diff : diffs)
	{
		CodeComparingLines::LineOrigin origin;
		origin.oldLineNumber = diff.oldLineNumber;
		origin.newLineNumber = diff.newLineNumber;

		switch (diff.type)
		{
		case CodeDiffType::UNCHANGED:
			origin.tp = CodeComparingLines::LineOrigin::Both;
			oss << diff.oldContent << "\n";
			break;
		case CodeDiffType::ADDED:
			origin.tp = CodeComparingLines::LineOrigin::NewCode;
			oss << diff.newContent << "\n";
			break;
		case CodeDiffType::DELETED:
			origin.tp = CodeComparingLines::LineOrigin::OldCode;
			oss << diff.oldContent << "\n";
			break;
		case CodeDiffType::MODIFIED:
			origin.tp = CodeComparingLines::LineOrigin::Both;
			oss << diff.oldContent << "\n";
			oss << diff.newContent << "\n";
			break;
		}

		// �����к�ӳ��
		if (diff.oldLineNumber > 0)
		{
			comparingContent.oldLineIndices[diff.oldLineNumber - 1] = currentLine;
		}
		if (diff.newLineNumber > 0)
		{
			comparingContent.newLineIndices[diff.newLineNumber - 1] = currentLine;
		}

		comparingContent.lineOrigins.push_back(origin);
		currentLine++;
	}

	comparingContent.content = oss.str();
}

void MakeCodeComparing_Chars_Old(const std::string& oldCode, const std::string& newCode, CodeComparingChars& comparingContent)
{
    // 清空比较内容
    comparingContent.content.clear();
    comparingContent.charTypes.clear();
    
    // 处理换行符，将\r\n替换为\n
    std::string processedOld = oldCode;
    std::string processedNew = newCode;
    
    const char* crlf = "\r\n";
    const char* lf = "\n";
    
    processedOld = ReplaceString(processedOld.c_str(), crlf, lf);
    processedNew = ReplaceString(processedNew.c_str(), crlf, lf);
    
    // 使用动态规划算法计算最长公共子序列(LCS)
    int m = (int)processedOld.length();
    int n = (int)processedNew.length();
    
    // 创建LCS表
    std::vector<std::vector<int>> lcsTable(m + 1, std::vector<int>(n + 1, 0));
    
    // 填充LCS表
    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (processedOld[i - 1] == processedNew[j - 1])
            {
                lcsTable[i][j] = lcsTable[i - 1][j - 1] + 1;
            }
            else
            {
                lcsTable[i][j] = max(lcsTable[i - 1][j], lcsTable[i][j - 1]);
            }
        }
    }
    
    // 根据LCS表生成差异结果
    int i = m;
    int j = n;
    
    std::string resultContent;
    std::vector<CodeComparingChars::CharType> resultTypes;
    
    // 从后向前遍历LCS表
    while (i > 0 || j > 0)
    {
        if (i > 0 && j > 0 && processedOld[i - 1] == processedNew[j - 1])
        {
            // 字符相同，属于Both类型
            resultContent.insert(resultContent.begin(), processedOld[i - 1]);
            resultTypes.insert(resultTypes.begin(), CodeComparingChars::Both);
            i--;
            j--;
        }
        else if (j > 0 && (i == 0 || lcsTable[i][j - 1] >= lcsTable[i - 1][j]))
        {
            // 字符在新代码中，属于NewCode类型
            resultContent.insert(resultContent.begin(), processedNew[j - 1]);
            resultTypes.insert(resultTypes.begin(), CodeComparingChars::NewCode);
            j--;
        }
        else if (i > 0 && (j == 0 || lcsTable[i][j - 1] < lcsTable[i - 1][j]))
        {
            // 字符在旧代码中，属于OldCode类型
            resultContent.insert(resultContent.begin(), processedOld[i - 1]);
            resultTypes.insert(resultTypes.begin(), CodeComparingChars::OldCode);
            i--;
        }
    }
    
    // 设置比较结果
    comparingContent.content = resultContent;
    comparingContent.charTypes = resultTypes;
}

void MakeCodeComparing_Chars_Myers(const std::string& oldCode, const std::string& newCode, CodeComparingChars& comparingContent)
{
	// 清空比较内容
	comparingContent.content.clear();
	comparingContent.charTypes.clear();

	// 处理换行符，将\r\n替换为\n
	std::string processedOld = oldCode;
	std::string processedNew = newCode;

	const char* crlf = "\r\n";
	const char* lf = "\n";

	processedOld = ReplaceString(processedOld.c_str(), crlf, lf);
	processedNew = ReplaceString(processedNew.c_str(), crlf, lf);

	// 使用 MyersDiff 进行字符级别的差异比较
	MyersDiff<std::string> myers(processedOld, processedNew);
	const auto& diffs = myers.diffs();

	// 根据 Myers 差异结果生成比较内容
	for (const auto& diff : diffs)
	{
		switch (diff.operation)
		{
		case DiffOp_Equal:
			// 相同的字符，属于Both类型
			for (auto it = diff.text.from; it != diff.text.till; ++it)
			{
				comparingContent.content += *it;
				comparingContent.charTypes.push_back(CodeComparingChars::Both);
			}
			break;

		case DiffOp_Insert:
			// 新增的字符，属于NewCode类型
			for (auto it = diff.text.from; it != diff.text.till; ++it)
			{
				comparingContent.content += *it;
				comparingContent.charTypes.push_back(CodeComparingChars::NewCode);
			}
			break;

		case DiffOp_Delete:
			// 删除的字符，属于OldCode类型
			for (auto it = diff.text.from; it != diff.text.till; ++it)
			{
				comparingContent.content += *it;
				comparingContent.charTypes.push_back(CodeComparingChars::OldCode);
			}
			break;
		}
	}
}

//生成特定格式的带有比较信息的文本
//删除的行以[-]开始
//新增的行以[+]开始
//有增删的行的前后最多保留四行未修改的行,其余未修改的行(代码块)用 [...] 代替
void GenerateDiffString(const std::string& oldContent, const std::string& newContent, std::string& diffString)
{
	CodeComparingLines comparingContent;
	std::deque<CodeDiffLine> diffs;

	// 获取差异信息
	CompareCodeStrings_Myers(oldContent, newContent, diffs);

	if (diffs.empty())
	{
		diffString = "[No Change]";
		return;
	}

	std::vector<std::string> result;
	const int CONTEXT_LINES = 4; // 上下文行数

	// 标记哪些行有变化或者需要显示
	std::vector<bool> shouldShow(diffs.size(), false);

	// 首先标记所有有变化的行
	for (size_t i = 0; i < diffs.size(); i++)
	{
		if (diffs[i].type != CodeDiffType::UNCHANGED)
		{
			shouldShow[i] = true;

			// 标记前后CONTEXT_LINES行也需要显示
			for (int j = max(0, (int)i - CONTEXT_LINES);
				j <= min((int)diffs.size() - 1, (int)i + CONTEXT_LINES);
				j++)
			{
				shouldShow[j] = true;
			}
		}
	}

	// 生成差异字符串
	bool inHiddenSection = false;
	for (size_t i = 0; i < diffs.size(); i++)
	{
		if (shouldShow[i])
		{
			// 如果之前有隐藏的部分，添加省略号
			if (inHiddenSection)
			{
				result.push_back("[...]");
				inHiddenSection = false;
			}

			// 根据差异类型添加相应的标记
			switch (diffs[i].type)
			{
			case CodeDiffType::UNCHANGED:
				result.push_back(diffs[i].oldContent);
				break;

			case CodeDiffType::ADDED:
				result.push_back("[+]" + diffs[i].newContent);
				break;

			case CodeDiffType::DELETED:
				result.push_back("[-]" + diffs[i].oldContent);
				break;

			case CodeDiffType::MODIFIED:
				// 修改的行显示为删除旧行+添加新行
				result.push_back("[-]" + diffs[i].oldContent);
				result.push_back("[+]" + diffs[i].newContent);
				break;
			}
		}
		else
		{
			// 标记进入隐藏部分
			inHiddenSection = true;
		}
	}

	// 将结果连接成字符串
	if (result.empty())
	{
		diffString = "";
	}
	else
	{
		// 清理头尾的空行和[...]行
		size_t start = 0;
		size_t end = result.size();

		// 清理开头的空行和[...]行
		while (start < result.size())
		{
			std::string& line = result[start];
			// 移除前后空白字符来判断是否为空行
			std::string trimmed = line;
			trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
			trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

			if (trimmed.empty() || trimmed == "[...]")
			{
				start++;
			}
			else
			{
				break;
			}
		}

		// 清理结尾的空行和[...]行
		while (end > start)
		{
			std::string& line = result[end - 1];
			// 移除前后空白字符来判断是否为空行
			std::string trimmed = line;
			trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
			trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

			if (trimmed.empty() || trimmed == "[...]")
			{
				end--;
			}
			else
			{
				break;
			}
		}

		// 连接清理后的结果
		if (start >= end)
		{
			diffString = "";
		}
		else
		{
			diffString = result[start];
			for (size_t i = start + 1; i < end; i++)
			{
				diffString += "\n" + result[i];
			}
		}
	}
}

int CodeComparingLines::NavigateDiff(int curLine, bool isNext) const
{
	if (lineOrigins.empty()) 
		return -1;

	// 验证当前行号是否有效
	if (curLine < 0 || curLine >= (int)lineOrigins.size())
		return -1;

	// 根据查找方向设置起始位置和步进值
	int start = curLine;
	int end = isNext ? (int)lineOrigins.size() : -1;
	int step = isNext ? 1 : -1;

	bool foundGap = false;  // 是否已找到间隔行(未改变行)

	for (int i = start; i != end; i += step) 
	{
		const LineOrigin& origin = lineOrigins[i];
		
		// 检查是否为未改变行
		if (origin.tp == LineOrigin::Both) 
		{
			foundGap = true;  // 标记已找到间隔行
		}
		// 检查是否为改变行(新增或删除)
		else if ((origin.tp == LineOrigin::NewCode || origin.tp == LineOrigin::OldCode) && foundGap) 
		{
			return i;  // 找到符合条件的改变行
		}
	}

	return -1;  // 未找到符合条件的改变行
}

