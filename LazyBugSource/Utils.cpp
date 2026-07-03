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
#include "Utils.h"
#include "Utils_File.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <sys/stat.h>
#include <string>
#include "LlmSkills.h"
#include "LazyBugConfig.h"

// 引入 local_to_utf8 函数声明
extern std::string local_to_utf8(const std::string& ansi_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);

// 外部函数声明
extern const char* GetCurModuleFolderPath_utf8();
extern const char* GetOpenedDBFolderPath_utf8();

namespace Utils
{

const char* CODE_FILE_EXTENSIONS = "cpp;c;cc;cxx;h;hpp;hxx;inl;js;ts;java;py;cs;php;rb;go;rs;swift;kt;lua;sql;html;css;xml;sh;bat;cmake;mak;makefile;yml;yaml;json;vue;tsv;md;txt";

const char* FileEditResultErrorPrefix = "[Error]";



// 将::分割符替换为.的函数
std::string NormalizeSymbolName(const std::string& symbolName)
{
	std::string result = symbolName;
	size_t pos = 0;
	while ((pos = result.find("::", pos)) != std::string::npos)
	{
		result.replace(pos, 2, ".");
		pos += 1; // 跳过新插入的字符，避免无限循环
	}
	return result;
}

// 将.分割符替换为::的函数（NormalizeSymbolName的反操作）
std::string RestoreSymbolName(const std::string& symbolName)
{
	std::string result = symbolName;
	size_t pos = 0;
	while ((pos = result.find(".", pos)) != std::string::npos)
	{
		result.replace(pos, 1, "::");
		pos += 2; // 跳过新插入的"::"，避免无限循环
	}
	return result;
}


// const char* GetDBRootFolder()
// {
// 	static char db_folder[MAX_PATH] = {0};
// 	static bool initialized = false;
// 	
// 	if (!initialized)
// 	{
// 		char app_data_path[MAX_PATH];
// 		if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, app_data_path) == S_OK)
// 		{
// 			snprintf(db_folder, MAX_PATH, "%s\\%s", app_data_path, LAZYBUG_DB_FOLDER);
// 		}
// 		else
// 		{
// 			// 如果获取失败，使用默认路径
// 			snprintf(db_folder, MAX_PATH, "%s", LAZYBUG_DB_FOLDER);
// 		}
// 		initialized = true;
// 	}
// 	
// 	return db_folder;
// }

const char* GetDBRootFolder_utf8()
{
	static std::string db_folder_utf8;
	static bool initialized = false;
	
	if (!initialized)
	{
		wchar_t app_data_path[MAX_PATH];
		if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, app_data_path) == S_OK)
		{
			wchar_t db_folder_wide[MAX_PATH];
			// 将 LAZYBUG_DB_FOLDER (ANSI) 转换为宽字符串
			wchar_t wDbFolder[64];
			MultiByteToWideChar(CP_ACP, 0, LAZYBUG_DB_FOLDER, -1, wDbFolder, 64);
			_snwprintf(db_folder_wide, MAX_PATH, L"%s\\%s", app_data_path, wDbFolder);
			db_folder_utf8 = widechar_to_utf8(db_folder_wide);
		}
		else
		{
			// 如果获取失败，使用默认路径
			db_folder_utf8 = LAZYBUG_DB_FOLDER;
		}
		initialized = true;
	}
	
	return db_folder_utf8.c_str();
}

// 检查文件是否为二进制文件（非文本文件）
bool CheckFileBinary(const char* path)
{
	if (!path || *path == '\0')
		return true;

	// 1. 取得文件后缀
	std::string suffix = GetFileSuffix(path);

	// 无后缀的文件视为二进制
	if (suffix.empty())
		return true;

	// 后缀统一转小写, 用于hash表查询
	StringLower(suffix);

	// 2. 白名单过滤: hash表查询, 命中则返回false(非二进制)
	{
		static const std::unordered_set<std::string> WHITE_LIST =
		{
			// C/C++
			"cpp", "c", "cc", "cxx", "h", "hpp", "hxx", "inl",
			// 脚本语言
			"js", "ts", "java", "py", "cs", "php", "rb", "go", "rs", "swift", "kt", "lua",
			// 标记/配置/数据
			"sql", "html", "css", "xml", "sh", "bat", "cmd", "cmake", "mak",
			"yml", "yaml", "json", "vue", "tsv", "md", "txt",
			// Windows/构建
			"rc", "def", "manifest", "asm", "s", "dsp", "dsw", "vcxproj", "props", "targets", "sln",
			// 其他文本
			"ini", "cfg", "conf", "log", "csv", "tlog", "rsp", "pubxml",
			"gitignore", "editorconfig", "dockerfile", "makefile",
		};

		if (WHITE_LIST.count(suffix))
			return false;
	}

	// 3. 黑名单过滤: hash表查询, 命中则返回true(二进制)
	{
		static const std::unordered_set<std::string> BLACK_LIST =
		{
			// 可执行/库
			"exe", "dll", "obj", "o", "lib", "pdb", "ilk", "exp",
			// 数据
			"bin", "dat", "db", "sqlite", "sqlite3",
			// 图片
			"png", "jpg", "jpeg", "gif", "bmp", "ico", "svg", "webp", "tif", "tiff", "psd","tga","dds",
			// 压缩
			"zip", "rar", "7z", "tar", "gz", "bz2", "xz", "cab",
			// 文档
			"pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx",
			// 音视频
			"mp3", "mp4", "avi", "mkv", "wav", "flv", "wmv", "mov",
			// 字体
			"ttf", "otf", "woff", "woff2", "eot",
			// 镜像
			"iso", "img", "dmg",
			// Java/Python编译产物
			"class", "jar", "war", "ear", "pyc", "pyo",
			// NuGet
			"nupkg", "snupkg",
		};

		if (BLACK_LIST.count(suffix))
			return true;
	}

	// 4. 读取文件头部内容(最多8KB), 检查是否为二进制文件
	{
		std::wstring wPath = utf8_to_widechar(path);
		FILE* fp = _wfopen(wPath.c_str(), L"rb");
		if (!fp)
			return true; // 读不出文件, 当作二进制

		BYTE buffer[8192];
		size_t readLen = fread(buffer, 1, sizeof(buffer), fp);
		fclose(fp);

		if (readLen == 0)
			return false; // 空文件不算二进制

		int controlCount = 0;
		const int THRESHOLD_CONTROL = 5; // 控制字符过多则认为是二进制

		for (size_t i = 0; i < readLen; i++)
		{
			BYTE ch = buffer[i];
			if (ch == 0x00)
				return true; // NUL字节, 直接判定二进制

			// 统计非文本控制字符(不包含 \t \r \n 这几个常见文本控制字符)
			if (ch < 0x20 && ch != '\t' && ch != '\r' && ch != '\n' && ch != '\x1b')
				controlCount++;

			if (controlCount >= THRESHOLD_CONTROL)
				return true;
		}
	}

	return false;
}

const wchar_t* GetWebViewUserFolder()
{
	static wchar_t webview_folder[MAX_PATH] = {0};
	static bool initialized = false;
	
	if (!initialized)
	{
		wchar_t app_data_path[MAX_PATH];
		if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, app_data_path) == S_OK)
		{
			_snwprintf(webview_folder, MAX_PATH, L"%s\\%s", app_data_path, LAZYBUG_WEBVIEW_USERFOLDER);
		}
		else
		{
			// 如果获取失败，使用默认路径
			_snwprintf(webview_folder, MAX_PATH, L"%s", LAZYBUG_WEBVIEW_USERFOLDER);
		}
		initialized = true;
	}
	
	return webview_folder;
}

bool IsCppFile(std::string& lowerCasedSuffix)
{
	if ((lowerCasedSuffix.c_str()[0] != 'c') && (lowerCasedSuffix.c_str()[0] != 'i') && (lowerCasedSuffix.c_str()[0] != 'h'))
		return false;
	if ((lowerCasedSuffix == "cpp") || (lowerCasedSuffix == "c") || (lowerCasedSuffix == "h") || (lowerCasedSuffix == "cxx") ||
		 (lowerCasedSuffix == "hpp") || (lowerCasedSuffix == "hxx") || 
		(lowerCasedSuffix == "cc") || (lowerCasedSuffix == "inl"))
		return true;
	return false;
}

bool IsImageFile(const char* filePath)
{
	if (!filePath || !*filePath)
		return false;
	
	// 图片文件扩展名列表（小写）
// 	static const char* IMAGE_EXTENSIONS[] = 
// 	{
// 		"jpg", "jpeg", "png", "bmp", "gif", "tif", "tiff", "tga", 
// 		"webp", "ico", "svg", "dds", "tex"
// 	};
	static const char* IMAGE_EXTENSIONS[] =
	{
		"jpg", "jpeg", "png", "webp"
	};

	// 使用 GetFileSuffix 获取文件后缀
	std::string suffix = GetFileSuffix(std::string(filePath));
	
	if (suffix.empty())
		return false;
	
	// 转换为小写
	StringLower(suffix);
	
	// 检查是否匹配图片扩展名
	for (int i = 0; i < sizeof(IMAGE_EXTENSIONS) / sizeof(char*); i++)
	{
		if (suffix == IMAGE_EXTENSIONS[i])
			return true;
	}
	
	return false;
}

}
