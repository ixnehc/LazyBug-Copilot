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
#include "Utils_SearchFile.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <sys/stat.h>
#include <string>


namespace Utils
{

// ---------------------------------------------------------------------------
// 内部辅助函数（仅限本编译单元）
// ---------------------------------------------------------------------------

// 判断字符是否是路径分隔符
static inline bool _IsPathSep(char c)
{
	return c == '/' || c == '\\';
}

// 通配符匹配核心（递归，pattern/text 均已小写）
// pi: pattern 当前位置索引，ti: text 当前位置索引
static bool _WildcardMatch(const char* pattern, int pi, int plen,
                           const char* text,    int ti, int tlen)
{
	// 递归终止：pattern 已耗尽
	if (pi == plen)
		return ti == tlen;

	// 检查是否是 "**"（可跨目录的多级通配符）
	if (pattern[pi] == '*' && pi + 1 < plen && pattern[pi + 1] == '*')
	{
		// 跳过连续的 "**"
		int nextPi = pi + 2;
		while (nextPi < plen && pattern[nextPi] == '*')
			nextPi++;

		// ** 可以匹配0个或多个任意字符（含路径分隔符）
		// 先尝试匹配0个字符
		if (_WildcardMatch(pattern, nextPi, plen, text, ti, tlen))
			return true;
		// 再逐步多匹配一个字符
		for (int i = ti; i < tlen; i++)
		{
			if (_WildcardMatch(pattern, nextPi, plen, text, i + 1, tlen))
				return true;
		}
		return false;
	}

	// 检查是否是单个 "*"（不跨目录）
	if (pattern[pi] == '*')
	{
		// 跳过连续的单 "*"
		int nextPi = pi + 1;
		while (nextPi < plen && pattern[nextPi] == '*' &&
		       !(nextPi + 1 < plen && pattern[nextPi + 1] == '*'))
			nextPi++;

		// * 匹配0个或多个非路径分隔符字符
		// 先尝试匹配0个字符
		if (_WildcardMatch(pattern, nextPi, plen, text, ti, tlen))
			return true;
		// 再逐步多匹配一个非分隔符字符
		for (int i = ti; i < tlen && !_IsPathSep(text[i]); i++)
		{
			if (_WildcardMatch(pattern, nextPi, plen, text, i + 1, tlen))
				return true;
		}
		return false;
	}

	// text 已耗尽但 pattern 还有内容
	if (ti == tlen)
		return false;

	// '?' 匹配任意单个非路径分隔符字符
	if (pattern[pi] == '?')
	{
		if (_IsPathSep(text[ti]))
			return false;
		return _WildcardMatch(pattern, pi + 1, plen, text, ti + 1, tlen);
	}

	// 普通字符：逐一比较
	if (pattern[pi] != text[ti])
		return false;
	return _WildcardMatch(pattern, pi + 1, plen, text, ti + 1, tlen);
}

// ---------------------------------------------------------------------------
// 公开接口实现
// ---------------------------------------------------------------------------

bool HasWildcard(const std::string& pattern)
{
	return pattern.find('*') != std::string::npos ||
	       pattern.find('?') != std::string::npos;
}

std::string WildcardToRegexString(const std::string& pattern)
{
	std::string regexStr;
	int len = (int)pattern.length();
	for (int i = 0; i < len; )
	{
		if (pattern[i] == '*' && i + 1 < len && pattern[i + 1] == '*')
		{
			// ** 匹配任意字符（包含路径分隔符）
			regexStr += ".*";
			i += 2;
			while (i < len && pattern[i] == '*') i++;
		}
		else if (pattern[i] == '*')
		{
			// * 匹配非路径分隔符的字符
			regexStr += "[^/\\\\]*";
			i++;
			while (i < len && pattern[i] == '*') i++;
		}
		else if (pattern[i] == '?')
		{
			// ? 匹配单个非路径分隔符的字符（支持 UTF-8 多字节序列）
			// 匹配单字节 ASCII (非分隔符) 或 2-4 字节的 UTF-8 字符
			regexStr += "(?:[^/\\\\\\x80-\\xFF]|[\\xC0-\\xDF][\\x80-\\xBF]|[\\xE0-\\xEF][\\x80-\\xBF]{2}|[\\xF0-\\xF7][\\x80-\\xBF]{3})";
			i++;
		}
		else
		{
			char c = pattern[i];
			// 对正则表达式保留字符进行转义
			if (c == '.' || c == '+' || c == '(' || c == ')' ||
			    c == '[' || c == ']' || c == '{' || c == '}' ||
			    c == '^' || c == '$' || c == '|')
			{
				regexStr += '\\';
				regexStr += c;
			}
			else if (c == '\\')
			{
				// 原通配符里的单个反斜杠转为正则需要匹配单个反斜杠，在 C++ 字符串中为 \\\\ 
				regexStr += "\\\\";
			}
			else
			{
				regexStr += c;
			}
			i++;
		}
	}
	// 为了确保是完整的匹配（等价于 std::regex_match 的默认行为），可以在首尾加上 ^ 和 $
	// 但 std::regex_match 本身就是全字符串匹配，因此只返回转换后的字符串即可。
	return regexStr;
}

bool MatchWildcardPath(const std::string& pattern, const std::string& text)
{
	std::string lowerPattern = pattern;
	std::string lowerText    = text;
	StringLower(lowerPattern);
	StringLower(lowerText);
	return _WildcardMatch(lowerPattern.c_str(), 0, (int)lowerPattern.size(),
	                      lowerText.c_str(),    0, (int)lowerText.size());
}

bool MatchFilePath(const std::string& lowerPattern, const std::string& lowerText)
{
	if (HasWildcard(lowerPattern))
	{
		// 隐式地在通配符表达式开头加上 "**" ，使得其支持在路径的任意位置进行匹配（类似子串匹配）
		if (lowerPattern.length() >= 2 && lowerPattern[0] == '*' && lowerPattern[1] == '*')
		{
			// 通配符匹配：对完整路径进行模式匹配
			return _WildcardMatch(lowerPattern.c_str(), 0, (int)lowerPattern.size(),
			                      lowerText.c_str(),    0, (int)lowerText.size());
		}
		else
		{
			std::string pattern = "**" + lowerPattern;
			return _WildcardMatch(pattern.c_str(), 0, (int)pattern.size(),
			                      lowerText.c_str(),    0, (int)lowerText.size());
		}
	}
	else
	{
		// 无通配符：子串包含匹配（原有逻辑）
		return lowerText.find(lowerPattern) != std::string::npos;
	}
}

}
