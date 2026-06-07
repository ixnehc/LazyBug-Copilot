#pragma once

#include <vector>
#include <string>
#include <set>
#include <windows.h>
#include <fstream>

class CChatHistory 
{
public:
	CChatHistory();

	struct Entry
	{
		std::wstring fileName;//.chat文件的文件名(注意不是全路径名)
		FILETIME modifiedTime;
		bool isFavorite;
	};

	struct MenuItemInfo
	{
		std::wstring uid;
		std::wstring content;
		std::wstring stamp;
		bool isFavorite;
	};

	//枚举folder下所有的.chat文件,读取到_entries里,并将它们按照文件修改时间排序,越近的排在越前面
	void Init(const char* folder);

	void Clear();

	const char* GetFolderPath()	{		return _folderPath.c_str();	}

	//添加一个新的fileName,加在最前面,如果fileName已经在_entries里,则把那一项提到最前面
	void Add(const char* fileName);

	//设置指定文件的favorite状态
	void SetFavorite(const char* fileName, bool isFavorite);

	std::string GetRecentFileName();

	//得到最近的若干项entry,并把它们转换成MenuItemInfo返回
	void GetRecentMenuItems(DWORD count, std::vector<MenuItemInfo>& items);

protected:

	//content:打开.chat文件,依次读入其中的op,找到第一个Op_SetTitle,使用里面的content
	//stamp:将文件的修改时间转换为可读的字符串,一天内的时间用 [XX]h[XX]m ago格式,一天前,30天内的的时间用 [XX]day ago格式,超过30天用具体时期 mm/dd 格式
	//uid:就是文件名
	void _Entry2MenuItemInfo(const Entry& entry, MenuItemInfo& info);

	// 辅助函数：枚举目录下的文件
	void _EnumerateFiles(const char* folder, std::vector<std::string>& fileList, std::set<std::string>& favFiles);

	std::string _folderPath;
	std::vector<Entry> _entries;//按照文件修改顺序排列
};
