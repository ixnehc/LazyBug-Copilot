#pragma once

#include <vector>
#include <string>
#include <deque>
#include <cstring>

struct StringSegment
{
	const char* str;
	//[start,end)
	int start;
	int end;

	// 为了支持MyersDiff，添加必要的方法和操作符
	bool operator==(const StringSegment& other) const 
	{
		int len1 = end - start;
		int len2 = other.end - other.start;
		if (len1 != len2) return false;
		
		// 比较内容，忽略行末的换行符差异
		const char* p1 = str + start;
		const char* p2 = other.str + other.start;
		
		// 找到第一个字符串的内容结束位置（不包括换行符）
		int content_len1 = len1;
		while (content_len1 > 0 && (p1[content_len1-1] == '\r' || p1[content_len1-1] == '\n'))
			content_len1--;
			
		// 找到第二个字符串的内容结束位置（不包括换行符）
		int content_len2 = len2;
		while (content_len2 > 0 && (p2[content_len2-1] == '\r' || p2[content_len2-1] == '\n'))
			content_len2--;
		
		// 比较内容部分
		if (content_len1 != content_len2) return false;
		return std::memcmp(p1, p2, content_len1) == 0;
	}

	bool operator!=(const StringSegment& other) const 
	{
		return !(*this == other);
	}

	// 获取长度
	int size() const 
	{
		return end - start;
	}

	// 获取字符
	char operator[](int index) const 
	{
		return str[start + index];
	}

	// 转换为string（调试用）
	std::string toString() const 
	{
		return std::string(str + start, end - start);
	}
};

// 声明SplitLines函数
void SplitLines(const std::string& str, std::vector<StringSegment>& lines);

class CCodingHistory
{
public:
	struct Replace
	{
		std::string filePath;
		std::string oldCode;
		LineRange oldCodeRange;
		std::string newCode;
		
		Replace()
		{
			// 构造函数
		}
		
		// 检查Replace是否为空
		bool IsEmpty() const;
	};

	CCodingHistory();

	//开始编辑一个文件
	const char* GetCurEditFilePath()	{		return _filePath.c_str();	}
	void SetCurEditFile(const char* filePath, const std::string& fileContent);
	bool Edit(const char* filePath,const std::string& fileContent);

	void Dump(std::string& str, int maxByte);

private:
	std::string _filePath;
	std::string _fileBaseContent;
	std::vector<StringSegment> _fileBaseLines;

	std::deque<Replace> _history;
	Replace _curReplace;
	
	// 内部辅助函数
	std::vector<Replace> _GenerateReplaces(const std::string& oldContent, const std::string& newContent);
	void _ApplyReplace(const Replace& replace);
	void _AddContextToReplaces(std::vector<Replace>& replaces, const std::vector<StringSegment>& oldLines, const std::vector<StringSegment>& newLines);
	int _CalcDistanceBetweenReplaces(const Replace& replaceA, const Replace& replaceB);
	void _AddHistory(const Replace& replace);
};