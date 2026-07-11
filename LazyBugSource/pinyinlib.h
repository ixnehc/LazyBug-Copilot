#pragma once

#include <unordered_map>
#include <string>
#include <vector>

// 拼音查找类：从拼音数据文件加载汉字→拼音映射，提供快速查找
// 数据文件格式：U+XXXX: pinyin  # char  (每行一个汉字)
class CPinyinLib
{
public:
	CPinyinLib();
	~CPinyinLib();

	// 从文件加载拼音数据，filePathUtf8 为 UTF-8 编码的文件路径
	bool Load(const char* filePathUtf8);

	// 是否已加载
	bool IsLoaded() const { return _loaded; }

	// 已加载的条目数
	size_t GetCount() const { return _mapCharToPinyin.size(); }

	// 查找单个汉字的拼音，未找到返回空字符串
	const char* Lookup(wchar_t ch) const;

	// 将整个字符串转为拼音串（空格分隔），未识别的字符跳过
	std::string GetPinyinString(const std::wstring& text) const;

	// 将字符串转为拼音数组
	void GetPinyinArray(const std::wstring& text, std::vector<const char*>& outPinyins) const;

	// 从模块目录加载拼音库文件（拼接 GetCurModuleFolderPath_utf8 + LAZYBUG_PINYIN_LIB_FILENAME）
	static bool LoadLib();

	// 清空所有数据
	void Clear();

private:
	// 解析一行数据，格式：U+XXXX: pinyin  # char
	bool _ParseLine(const std::string& line, wchar_t& outChar, std::string& outPinyin);

	std::unordered_map<wchar_t, std::string> _mapCharToPinyin;
	bool _loaded;
};
