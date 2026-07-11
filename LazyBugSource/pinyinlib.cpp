#include "stdh.h"
#include "PinyinLib.h"
#include "Utils_File.h"
#include <fstream>

CPinyinLib g_pinyinLib;

extern const char* GetCurModuleFolderPath_utf8();

bool CPinyinLib::LoadLib()
{
	std::string filePath = std::string(GetCurModuleFolderPath_utf8()) + "\\" + LAZYBUG_PINYIN_LIB_FILENAME;
	return g_pinyinLib.Load(filePath.c_str());
}

CPinyinLib::CPinyinLib()
	: _loaded(false)
{
}

CPinyinLib::~CPinyinLib()
{
	Clear();
}

bool CPinyinLib::Load(const char* filePathUtf8)
{
	Clear();

	std::ifstream ifs;
	if (!Utils::OpenIFStream(ifs, filePathUtf8))
		return false;

	std::string line;

	while (std::getline(ifs, line))
	{
		// 跳过空行
		if (line.empty())
			continue;

		// 跳过注释行（以 # 开头）
		if (line[0] == '#')
			continue;

		wchar_t ch;
		std::string pinyin;
		if (!_ParseLine(line, ch, pinyin))
			continue;

		_mapCharToPinyin[ch] = pinyin;
	}

	_loaded = true;
	return true;
}

const char* CPinyinLib::Lookup(wchar_t ch) const
{
	if (!_loaded)
		return "";

	auto it = _mapCharToPinyin.find(ch);
	if (it != _mapCharToPinyin.end())
		return it->second.c_str();

	return "";
}

std::string CPinyinLib::GetPinyinString(const std::wstring& text) const
{
	std::string result;
	result.reserve(text.size() * 6);  // 预分配：每个拼音平均约6字节

	for (wchar_t ch : text)
	{
		const char* py = Lookup(ch);
		if (py[0] != '\0')
		{
			if (!result.empty())
				result += ' ';
			result += py;
		}
	}

	return result;
}

void CPinyinLib::GetPinyinArray(const std::wstring& text, std::vector<const char*>& outPinyins) const
{
	outPinyins.clear();
	outPinyins.reserve(text.size());

	for (wchar_t ch : text)
	{
		const char* py = Lookup(ch);
		if (py[0] != '\0')
			outPinyins.push_back(py);
	}
}

void CPinyinLib::Clear()
{
	_mapCharToPinyin.clear();
	_loaded = false;
}

bool CPinyinLib::_ParseLine(const std::string& line, wchar_t& outChar, std::string& outPinyin)
{
	// 格式示例：U+4E00: yī  # 一
	//           U+4E0A: shàng,shang  # 上

	const char* p = line.c_str();

	// 1. 跳过前导空白
	while (*p == ' ' || *p == '\t') p++;

	// 2. 检查 "U+" 前缀
	if (p[0] != 'U' || p[1] != '+')
		return false;
	p += 2;

	// 3. 解析 Unicode 码点（十六进制）
	char* endPtr = nullptr;
	unsigned long codePoint = strtoul(p, &endPtr, 16);
	if (endPtr == p)
		return false;  // 解析失败

	outChar = static_cast<wchar_t>(codePoint);
	p = endPtr;

	// 4. 跳过冒号及空白
	while (*p == ' ' || *p == '\t' || *p == ':') p++;

	// 5. 提取拼音（只取第一个读音，逗号前的部分）
	const char* pinyinStart = p;
	while (*p != '\0' && *p != ',' && *p != ' ' && *p != '\t' && *p != '#' && *p != '\r' && *p != '\n')
		p++;

	if (p == pinyinStart)
		return false;  // 没有拼音数据

	outPinyin.assign(pinyinStart, p - pinyinStart);

	return true;
}
