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
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <sys/stat.h>
#include <string>

// 引入 local_to_utf8 函数声明
extern std::string local_to_utf8(const std::string& ansi_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);

namespace Utils
{

const char* CODE_FILE_EXTENSIONS = "cpp;c;cc;cxx;h;hpp;hxx;inl;js;ts;java;py;cs;php;rb;go;rs;swift;kt;lua;sql;html;css;xml;sh;bat;cmake;mak;makefile;yml;yaml;json;vue;tsv;md;txt";

const char* FileEditResultErrorPrefix = "[Error]";

FILETIME GetZeroFileTime()
{
	FILETIME ft;
	memset(&ft, 0, sizeof(ft));
	return ft;
}

bool EqualFileTime(const FILETIME& a, const FILETIME& b)
{
	return a.dwHighDateTime == b.dwHighDateTime && a.dwLowDateTime == b.dwLowDateTime;
}

FILETIME GetFileTime(const char* path)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	if (is_pure_ascii(path))
	{
		if (GetFileAttributesExA(path, GetFileExInfoStandard, &fileAttrData))
		{
			return fileAttrData.ftLastWriteTime;
		}
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fileAttrData))
		{
			return fileAttrData.ftLastWriteTime;
		}
	}
	return GetZeroFileTime();
}

bool IsFileExist(const char* path)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	if (is_pure_ascii(path))
	{
		return GetFileAttributesExA(path, GetFileExInfoStandard, &fileAttrData) != 0;
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		return GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fileAttrData) != 0;
	}
}

bool IsFileReadOnly(const char* path)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	if (is_pure_ascii(path))
	{
		if (GetFileAttributesExA(path, GetFileExInfoStandard, &fileAttrData))
		{
			return (fileAttrData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
		}
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fileAttrData))
		{
			return (fileAttrData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
		}
	}
	return false; // 文件不存在或获取属性失败，返回false
}

// 辅助函数：创建目录（支持多级目录）
static bool CreateDirectoryRecursive(const char* path)
{
	if (!path || *path == '\0')
		return false;

	// 检查目录是否已存在
	if (_access(path, 0) == 0)
		return true;

	// 获取父目录
	std::string strPath(path);
	size_t pos = strPath.find_last_of("\\/");
	if (pos != std::string::npos)
	{
		std::string parentDir = strPath.substr(0, pos);
		if (!CreateDirectoryRecursive(parentDir.c_str()))
			return false;
	}

	// 创建当前目录
	return _mkdir(path) == 0;
}

void EnsureFolder(const char* path)
{
	CreateDirectoryRecursive(path);
}



AbsTick GetFileTick(const char *path)
{
	return AbsTickFromFILETIME(GetFileTime(path));
}

bool SetFileTick(const char* path, AbsTick tick)
{
	// 将 AbsTick 转换为 FILETIME
	FILETIME ft;
	unsigned __int64 tickValue = tick * 10000; // 转换为100纳秒单位
	ft.dwLowDateTime = (DWORD)(tickValue & 0xFFFFFFFF);
	ft.dwHighDateTime = (DWORD)(tickValue >> 32);
	
	return SetFileTime(path, ft);
}

bool SetFileTime(const char* path, const FILETIME& fileTime)
{
	if (!path || *path == '\0')
		return false;

	// 检查文件是否存在
	if (!IsFileExist(path))
		return false;

	HANDLE hFile;
	if (is_pure_ascii(path))
	{
		hFile = CreateFileA(
			path,
			FILE_WRITE_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		hFile = CreateFileW(
			wpath.c_str(),
			FILE_WRITE_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}

	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// 只设置文件的修改时间
	BOOL result = ::SetFileTime(hFile, NULL, NULL, &fileTime);
	
	CloseHandle(hFile);
	return result != 0;
}

time_t GetFileTimeT(const char* path)
{
	FILETIME ft = GetFileTime(path);
	
	// 检查是否为零文件时间
	if (ft.dwHighDateTime == 0 && ft.dwLowDateTime == 0)
	{
		return 0;
	}
	
	// 将 FILETIME 转换为 64 位整数（100纳秒间隔）
	ULARGE_INTEGER ull;
	ull.LowPart = ft.dwLowDateTime;
	ull.HighPart = ft.dwHighDateTime;
	
	// 从1601年1月1日到1970年1月1日的100纳秒间隔数
	const ULONGLONG EPOCH_AS_FILETIME = 116444736000000000ULL;
	
	// 转换为Unix时间戳（秒）
	return (time_t)((ull.QuadPart - EPOCH_AS_FILETIME) / 10000000ULL);
}

time_t GetCurFileTimeT()
{
	FILETIME ft = GetCurFileTime();
	
	// 检查是否为零文件时间
	if (ft.dwHighDateTime == 0 && ft.dwLowDateTime == 0)
	{
		return 0;
	}
	
	// 将 FILETIME 转换为 64 位整数（100纳秒间隔）
	ULARGE_INTEGER ull;
	ull.LowPart = ft.dwLowDateTime;
	ull.HighPart = ft.dwHighDateTime;
	
	// 从1601年1月1日到1970年1月1日的100纳秒间隔数
	const ULONGLONG EPOCH_AS_FILETIME = 116444736000000000ULL;
	
	// 转换为Unix时间戳（秒）
	return (time_t)((ull.QuadPart - EPOCH_AS_FILETIME) / 10000000ULL);
}


bool is_valid_utf8(const std::string& s) 
{
	size_t i = 0;
	size_t len = s.size();
	while (i < len) 
	{
		unsigned char c = static_cast<unsigned char>(s[i]);
		if (c <= 0x7F) 
		{
			// 1-byte (ASCII)
			i += 1;
		}
		else if ((c & 0xE0) == 0xC0) 
		{
			// 2-byte
			if (i + 1 >= len || (s[i + 1] & 0xC0) != 0x80)
				return false;
			i += 2;
		}
		else if ((c & 0xF0) == 0xE0) 
		{
			// 3-byte
			if (i + 2 >= len || (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80)
				return false;
			i += 3;
		}
		else if ((c & 0xF8) == 0xF0) 
		{
			// 4-byte
			if (i + 3 >= len ||
				(s[i + 1] & 0xC0) != 0x80 ||
				(s[i + 2] & 0xC0) != 0x80 ||
				(s[i + 3] & 0xC0) != 0x80)
				return false;
			i += 4;
		}
		else 
		{
			// Invalid first byte
			return false;
		}
	}
	return true;
}

bool ConvertFileContentIntoUTF8(const std::vector<BYTE>& raw_content, std::vector<BYTE>& processed_content, FileContentCodingFormat& codingFmt)
{
	codingFmt = FileContentCodingFormat::None;
	// 检查 UTF-8 BOM (EF BB BF)
	const BYTE utf8_bom[] = { 0xEF, 0xBB, 0xBF };
	if (raw_content.size() >= 3 &&
		raw_content[0] == utf8_bom[0] &&
		raw_content[1] == utf8_bom[1] &&
		raw_content[2] == utf8_bom[2])
	{
		// 有 BOM，跳过 BOM 并复制剩余内容
		processed_content.assign(raw_content.begin() + 3, raw_content.end());
		codingFmt = FileContentCodingFormat::Utf8_WithSignature;
	}
	else
	{
		// 没有 BOM，检查是否为 UTF-8
		std::string raw_str(reinterpret_cast<const char*>(raw_content.data()), raw_content.size());
		if (is_valid_utf8(raw_str))
		{
			// 是 UTF-8，直接复制原始内容
			processed_content.assign(raw_content.begin(), raw_content.end());
			codingFmt = FileContentCodingFormat::Utf8;
		}
		else
		{
			// 不是 UTF-8，假设是本地编码并转换为 UTF-8
			std::string utf8_str = local_to_utf8(raw_str);
			// 将转换后的 UTF-8 字符串内容复制到输出向量
			processed_content.assign(utf8_str.begin(), utf8_str.end());
			codingFmt = FileContentCodingFormat::Local;
		}
	}

	return true;

}

// 内部辅助函数：读取文件并处理编码
static bool ReadFileIntoUTF8(const char* path, std::vector<BYTE>& processed_content, FileContentCodingFormat& codingFmt)
{
	codingFmt = FileContentCodingFormat::None;

	std::ifstream file;
	if (is_pure_ascii(path))
	{
		file.open(path, std::ios::in | std::ios::binary);
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		file.open(wpath.c_str(), std::ios::in | std::ios::binary);
	}
	
	if (!file.is_open()) 
	{
		processed_content.clear();
		return false;
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	// 如果文件为空，直接返回
	if (size == 0)
	{
		processed_content.clear();
		file.close();
		return true;
	}

	// 读取整个文件到临时向量
	std::vector<BYTE> raw_content;
	raw_content.resize(size);
	if (!file.read(reinterpret_cast<char*>(raw_content.data()), size))
	{
		processed_content.clear();
		file.close();
		return false;
	}
	file.close();

	return ConvertFileContentIntoUTF8(raw_content, processed_content,codingFmt);
}

bool ConvertFileContentFromUTF8(std::vector<BYTE>& raw_content, const std::vector<BYTE>& processed_content, FileContentCodingFormat codingFmt)
{
	raw_content.clear();
	
	switch (codingFmt)
	{
	case FileContentCodingFormat::Utf8_WithSignature:
		{
			// 添加 UTF-8 BOM
			const BYTE utf8_bom[] = { 0xEF, 0xBB, 0xBF };
			raw_content.assign(utf8_bom, utf8_bom + 3);
			// 添加内容
			raw_content.insert(raw_content.end(), processed_content.begin(), processed_content.end());
		}
		break;
		
	case FileContentCodingFormat::Utf8:
		{
			// 直接复制 UTF-8 内容，不添加 BOM
			raw_content.assign(processed_content.begin(), processed_content.end());
		}
		break;
		
	case FileContentCodingFormat::Local:
		{
			// 将 vector<BYTE> 转换为 string
			std::string utf8_str(reinterpret_cast<const char*>(processed_content.data()), processed_content.size());
			// 转换为本地编码
			std::string local_str = utf8_to_local(utf8_str);
			// 将结果复制到 raw_content
			raw_content.assign(local_str.begin(), local_str.end());
		}
		break;
		
	case FileContentCodingFormat::None:
	default:
		// 对于未知格式，直接复制内容
		raw_content.assign(processed_content.begin(), processed_content.end());
		break;
	}
	
	return true;
}

// 内部辅助函数：将UTF-8内容写入文件
static bool WriteFileFromUTF8(const char* path, const std::vector<BYTE>& content, FileContentCodingFormat codingFmt)
{
	if (!path || *path == '\0')
		return false;
		
	// 将UTF-8内容转换为目标编码格式
	std::vector<BYTE> raw_content;
	if (!ConvertFileContentFromUTF8(raw_content, content, codingFmt))
		return false;
	
	// 写入文件
	std::ofstream file;
	if (is_pure_ascii(path))
	{
		file.open(path, std::ios::out | std::ios::binary);
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		file.open(wpath.c_str(), std::ios::out | std::ios::binary);
	}

	if (!file.is_open())
		return false;

	// 写入内容
	if (!raw_content.empty())
	{
		file.write(reinterpret_cast<const char*>(raw_content.data()), raw_content.size());
	}

	// 检查写入是否成功
	if (file.fail())
	{
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool GetFileContentIntoUTF8(const char* path, std::string& content, FileContentCodingFormat& codingFmt)
{
	std::vector<BYTE> processed_content;
	
	if (!ReadFileIntoUTF8(path, processed_content,codingFmt))
	{
		content.clear();
		return false;
	}
	
	// 将处理后的字节数组转换为字符串
	if (!processed_content.empty())
	{
		content.assign(reinterpret_cast<const char*>(processed_content.data()), processed_content.size());
	}
	else
	{
		content.clear();
	}
	
	return true;
}

bool SaveFileContent(const char* path, const std::string& content)
{
	// 创建输出文件流
	std::ofstream file;
	if (is_pure_ascii(path))
	{
		file.open(path, std::ios::out | std::ios::binary);
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		file.open(wpath.c_str(), std::ios::out | std::ios::binary);
	}

	if (!file.is_open())
	{
		return false;
	}

// 	// 写入 UTF-8 BOM (EF BB BF)
// 	const unsigned char utf8_bom[] = { 0xEF, 0xBB, 0xBF };
// 	file.write(reinterpret_cast<const char*>(utf8_bom), 3);

	// 写入内容
	if (!content.empty())
	{
		file.write(content.c_str(), content.size());
	}

	// 检查写入是否成功
	if (file.fail())
	{
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool SaveFileContent(const char* path, const std::vector<BYTE>& content)
{
	// 创建输出文件流
	std::ofstream file;
	if (is_pure_ascii(path))
	{
		file.open(path, std::ios::out | std::ios::binary);
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		file.open(wpath.c_str(), std::ios::out | std::ios::binary);
	}

	if (!file.is_open())
	{
		return false;
	}

	// 直接写入二进制内容
	if (!content.empty())
	{
		file.write(reinterpret_cast<const char*>(content.data()), content.size());
	}

	// 检查写入是否成功
	if (file.fail())
	{
		file.close();
		return false;
	}

	file.close();
	return true;
}


bool GetFileContentIntoUTF8(const char* path, std::vector<BYTE>& content, FileContentCodingFormat& codingFmt)
{
	if (!ReadFileIntoUTF8(path, content,codingFmt))
	{
		return false;
	}
	
	return true;
}

bool LoadFileContent(const char* path, std::vector<BYTE>& content)
{
	if (!path || *path == '\0')
		return false;

	std::ifstream file;
	if (is_pure_ascii(path))
	{
		file.open(path, std::ios::in | std::ios::binary);
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		file.open(wpath.c_str(), std::ios::in | std::ios::binary);
	}

	if (!file.is_open())
	{
		content.clear();
		return false;
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	// 如果文件为空，直接返回
	if (size == 0)
	{
		content.clear();
		file.close();
		return true;
	}

	// 读取整个文件到向量
	content.resize(static_cast<size_t>(size));
	if (!file.read(reinterpret_cast<char*>(content.data()), size))
	{
		content.clear();
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool LoadFileContent(const char* path, std::string& content)
{
	if (!path || *path == '\0')
		return false;

	std::ifstream file;
	if (is_pure_ascii(path))
	{
		file.open(path, std::ios::in | std::ios::binary);
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		file.open(wpath.c_str(), std::ios::in | std::ios::binary);
	}

	if (!file.is_open())
	{
		content.clear();
		return false;
	}

	// 获取文件大小
	file.seekg(0, std::ios::end);
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);

	// 如果文件为空，直接返回
	if (size == 0)
	{
		content.clear();
		file.close();
		return true;
	}

	// 读取整个文件到字符串
	content.resize(static_cast<size_t>(size));
	if (!file.read(&content[0], size))
	{
		content.clear();
		file.close();
		return false;
	}

	file.close();
	return true;
}

const char* SkipCodingHead(const char* content)
{
	if (content[0] == (char)0xef)
	{
		if (content[1] == (char)0xbb)
		{
			if (content[2] == (char)0xbf)
			{
				return content + 3;
			}
		}
	}
	return content;
}

std::string GetActualFilePath(const char* fullPath)
{
	if (!fullPath || *fullPath == '\0')
	{
		return "";
	}

	// Always use the wide-character version of the API for robustness with all path types.
	// The extern function utf8_to_widechar is available.
	std::wstring wPath = utf8_to_widechar(fullPath);
	HANDLE hFile = CreateFileW(
		wPath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS, // Needed to get a handle to a directory
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// File not found or other error, return original path as a fallback.
		return fullPath;
	}

	// Get the path length.
	DWORD pathLen = GetFinalPathNameByHandleW(hFile, NULL, 0, FILE_NAME_NORMALIZED);
	if (pathLen == 0)
	{
		CloseHandle(hFile);
		return fullPath; // Error, fallback to original path.
	}

	std::vector<wchar_t> wBuffer(pathLen);
	DWORD result = GetFinalPathNameByHandleW(hFile, wBuffer.data(), pathLen, FILE_NAME_NORMALIZED);
	CloseHandle(hFile);

	if (result == 0)
	{
		return fullPath; // Error, fallback to original path.
	}

	// The result from GetFinalPathNameByHandleW includes a null terminator, and pathLen includes it.
	// So we can construct the wstring directly.
	std::wstring wActualPath(wBuffer.data(), result);

	// The result often has the "\\?\" prefix. Remove it if present.
	if (wActualPath.rfind(L"\\\\?\\", 0) == 0)
	{
		wActualPath = wActualPath.substr(4);
	}

	// Convert the wide string result back to UTF-8.
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wActualPath.c_str(), (int)wActualPath.size(), NULL, 0, NULL, NULL);
	std::string actualPath(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wActualPath.c_str(), (int)wActualPath.size(), &actualPath[0], size_needed, NULL, NULL);

	return actualPath;
}

bool RemoveFile(const char* path)
{
	if (!path || *path == '\0')
		return false;

	// 检查文件是否存在
	if (!IsFileExist(path))
		return false;

	// 尝试删除文件
	if (is_pure_ascii(path))
	{
		return ::DeleteFileA(path) != 0;
	}
	else
	{
		std::wstring wpath = utf8_to_widechar(path);
		return ::DeleteFileW(wpath.c_str()) != 0;
	}
}

bool SetFileContentFromUTF8(const char* path, const std::vector<BYTE>& content, FileContentCodingFormat codingFmt)
{
	return WriteFileFromUTF8(path, content, codingFmt);
}

bool SetFileContentFromUTF8(const char* path, const std::string& content, FileContentCodingFormat codingFmt)
{
	// 将 std::string 转换为 std::vector<BYTE>
	std::vector<BYTE> byteContent;
	if (!content.empty())
	{
		byteContent.assign(content.begin(), content.end());
	}
	
	return WriteFileFromUTF8(path, byteContent, codingFmt);
}

bool GetFilePartIntoUTF8(const char* path, int startLine, int endLine, std::string& content, FileContentCodingFormat& codingFmt)
{
	content.clear();
	codingFmt = FileContentCodingFormat::None;

	// 参数验证
	if (!path || *path == '\0' || startLine < 0 || endLine < 0 || startLine > endLine)
	{
		return false;
	}

	// 读取整个文件内容为UTF-8字符串
	std::string fullContent;
	if (!GetFileContentIntoUTF8(path, fullContent, codingFmt))
	{
		return false;
	}

	// 使用SplitLines分割文件内容为行
	std::vector<std::string> lines;
	SplitLines(fullContent, lines);

	// 检查行号范围是否有效
	if (startLine >= (int)lines.size())
	{
		return false; // 起始行超出范围
	}

	// 调整结束行号，确保不超出实际行数
	if (endLine >= (int)lines.size())
	{
		endLine = (int)lines.size() - 1;
	}

	// 收集指定范围的行
	for (int i = startLine; i <= endLine; i++)
	{
		if (i > startLine) // 不是第一行，添加换行符
		{
			content += "\n";
		}
		content += lines[i];
	}

	return true;
}


bool ParseRipGrepLine(const std::string& line, FindInFileResults& results, FindInFileFilter filterCallback)
{
	// ripgrep 输出格式：文件路径:行号:行内容
	// 例如：C:\test.cpp:10:void test() { return; }
	// 需要处理 Windows 路径中的冒号（如 C:\）

	// 查找第一个冒号，但要跳过 Windows 驱动器号的冒号
	int firstColon = -1;
	int searchPos = 0;

	while (true) {
		int colonPos = StringFind(line.c_str() + searchPos, ':');
		if (colonPos == -1) break;

		colonPos += searchPos; // 转换为绝对位置

		// 检查冒号后面是不是反斜杠（Windows 路径格式）
		if (colonPos + 1 < (int)line.length() && line[colonPos + 1] == '\\') {
			// 这是 Windows 驱动器号的冒号，继续查找下一个冒号
			searchPos = colonPos + 2;
		}
		else {
			// 找到真正的分隔冒号
			firstColon = colonPos;
			break;
		}
	}

	if (firstColon == -1)
		return false;

	// 查找第二个冒号
	int secondColon = StringFind(line.c_str() + firstColon + 1, ':');
	if (secondColon == -1)
		return false;
	secondColon += firstColon + 1; // 转换为绝对位置

	// 提取文件路径（第一个冒号前）
	std::string filePath = line.substr(0, firstColon);
	RemoveHeadBlank(filePath);
	RemoveTailBlank(filePath);

	// 提取行号（第一个冒号后，第二个冒号前）
	std::string lineNumStr = line.substr(firstColon + 1, secondColon - firstColon - 1);
	RemoveHeadBlank(lineNumStr);
	RemoveTailBlank(lineNumStr);
	int lineNumber = IntFromString(lineNumStr.c_str());

	// 提取行内容（第二个冒号后）
	std::string lineContent = line.substr(secondColon + 1);
	RemoveHeadBlank(lineContent);
	RemoveTailBlank(lineContent);

	// 如果不是有效的 UTF-8，转换为 UTF-8
	if (!is_valid_utf8(lineContent))
		lineContent = local_to_utf8(lineContent);

	// 检查是否需要过滤该文件
	if (filterCallback && filterCallback(filePath.c_str()))
	{
		return false;
	}

	// 添加到结果
	results.AddResult(filePath, lineNumber, lineContent);
	return true;
}

// 解析包含多行结果的字符串
static void ParseRipGrepLines(const std::string& multiLineString, FindInFileResults& results, int& resultCount, int maxResults, FindInFileFilter filterCallback)
{
	if (multiLineString.empty())
		return;

	std::vector<std::string> lines;
	SplitLines(multiLineString, lines);

	for (const auto& line : lines)
	{
		if (IsBlankString(line.c_str()))
			continue;

		// 使用 ParseRipGrepLine 函数解析单行
		if (ParseRipGrepLine(line, results, filterCallback))
		{
			resultCount++;
			// 检查是否达到最大结果数
			if (resultCount >= maxResults)
				break;
		}
	}
}

bool SearchWithRipGrep(const char* key, std::vector<std::string>& folderPathes, FindInFileResults& results, int maxResults, FindInFileFilter filterCallback)
{
	if (!key || *key == '\0')
		return false;

	// 构建 ripgrep 命令
	std::string cmd = "rg -u --no-heading --line-number --color never --no-mmap --max-columns 0 ";

	// 添加文件扩展名过滤
	// ripgrep 使用 --type-add 自定义类型，然后 --type 使用
	std::vector<std::string> exts;
	SplitStringBy(";", CODE_FILE_EXTENSIONS, &exts);

	// 使用 {ext1,ext2,ext3} 格式构建扩展名过滤模式
	std::string extPattern = "*.{";
	for (size_t i = 0; i < exts.size(); ++i)
	{
		if (i > 0) extPattern += ",";
		extPattern += exts[i];
	}
	extPattern += "}";

	// 使用 -g (glob) 参数来过滤文件扩展名
	cmd += "-g \"" + extPattern + "\" ";

	// 添加搜索模式（用引号包裹，避免 shell 解析问题）
	// 注意：-- 分隔符告诉 ripgrep 后面的都是文件路径，而不是正则表达式选项
	cmd += " \"" + std::string(key) + "\"";

	// 添加路径（在 -- 分隔符之后，确保路径中的反斜杠不会被当作正则表达式）
	cmd += " --";
	if (!folderPathes.empty())
	{
		for (size_t i = 0; i < folderPathes.size(); ++i)
		{
			cmd += " \"" + folderPathes[i] + "\"";
		}
	}
	else
	{
		cmd += " .";
	}

	// 执行 ripgrep 命令
	SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
	HANDLE hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		return false;

	STARTUPINFOA si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = hWrite;
	si.hStdError = hWrite;

	PROCESS_INFORMATION pi = { 0 };
	if (!CreateProcessA(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		CloseHandle(hRead);
		CloseHandle(hWrite);
		return false;
	}

	CloseHandle(hWrite);

	// 读取输出并实时解析
	std::string output;
	char buffer[4096];
	DWORD bytesRead;
	int resultCount = 0;

	bool truncated = false;
	while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		output += buffer;

		// 实时解析已读取的内容
		// 查找完整的行（以换行符结束）
		size_t lastNewline = output.find_last_of('\n');
		if (lastNewline != std::string::npos)
		{
			// 提取完整的行进行处理
			std::string completeLines = output.substr(0, lastNewline + 1);
			output = output.substr(lastNewline + 1); // 保留未完成的行

			// 使用 ParseRipGrepLines 函数解析多行
			ParseRipGrepLines(completeLines, results, resultCount, maxResults, filterCallback);

			// 检查是否达到最大结果数
			if (resultCount >= maxResults)
			{
				TerminateProcess(pi.hProcess, 0);
				truncated = true;
				break;
			}
		}
	}

	CloseHandle(hRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// 处理缓冲区中剩余的内容
	if (!truncated)
	{
		if (!output.empty())
		{
			ParseRipGrepLines(output, results, resultCount, maxResults, filterCallback);
		}
	}

	return results.GetTotalResults() > 0;
}


void FindInFile(const char* key, std::vector<std::string>& folderPathes, FindInFileResults& results, int maxResults, FindInFileFilter filterCallback)
{
	results.Clear();

	if (!key || *key == '\0')
		return;

// 	FindInFileResults result2;
// 	// 如果 ripgrep 搜索失败，尝试使用 Everything 搜索
// 	if (IsEverythingRunning())
// 	{
// 		SearchWithEverything(key, folderPathes, result2);
// 	}
// 
// 	// 如果 Everything 搜索失败，使用 PowerShell 搜索
// 	SearchWithPowerShell(key, folderPathes, results);

	SearchWithRipGrep(key, folderPathes, results, maxResults, filterCallback);
}


int FindMatchingLines(const std::string& filePath, const std::string& key,
	const std::string& content, FindInFileResults& results, int maxLines)
{
	std::istringstream stream(content);
	std::string line;
	int lineNumber = 0;
	int addedCount = 0;

	auto isWordChar = [](char c) -> bool {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
	};

	// 检查key是否包含非word字符
	bool hasNonWordChar = false;
	for (char c : key)
	{
		if (!isWordChar(c))
		{
			hasNonWordChar = true;
			break;
		}
	}

	while (std::getline(stream, line) && addedCount < maxLines)
	{
		++lineNumber;

		// 大小写敏感的匹配
		size_t pos = 0;
		while ((pos = line.find(key, pos)) != std::string::npos)
		{
			// 如果key包含非word字符，不进行全词匹配检查
			if (hasNonWordChar)
			{
				results.AddResult(filePath, lineNumber, line);
				++addedCount;
				break; // 当前行已匹配，跳到下一行
			}

			// 检查前面是否是单词边界
			bool leftBoundary = (pos == 0) || !isWordChar(line[pos - 1]);
			// 检查后面是否是单词边界
			bool rightBoundary = (pos + key.length() == line.length()) || !isWordChar(line[pos + key.length()]);

			if (leftBoundary && rightBoundary)
			{
				results.AddResult(filePath, lineNumber, line);
				++addedCount;
				break; // 当前行已匹配，跳到下一行
			}

			++pos; // 继续搜索下一个位置
		}
	}

	return addedCount;
}


void DumpFindInFileResult(const char* key, FindInFileResults& results, std::string& resultString, int maxResult)
{
	resultString.clear();

	if (results.fileInfos.empty())
	{
		resultString += "========================================\n";
		resultString += "Search Result Summary\n";
		resultString += "========================================\n";
		resultString += "Search Keyword: " + std::string(key ? key : "") + "\n";
		resultString += "Matched Files: 0\n";
		resultString += "Matched Lines: 0\n";
		resultString += "No results found.\n";
		resultString += "========================================\n";
		return;
	}

	// Statistics
	size_t totalFiles = results.fileInfos.size();
	size_t totalLines = results.GetTotalResults();

	// Calculate actual displayed lines
	size_t displayLines = totalLines;
	bool hasMore = false;
	if (maxResult > 0 && totalLines >= static_cast<size_t>(maxResult))
	{
		hasMore = true;
	}

	resultString += "========================================\n";
	resultString += "Search Result Summary\n";
	resultString += "========================================\n";
	resultString += "Search Keyword: " + std::string(key ? key : "") + "\n";
	resultString += "Matched Files: " + std::to_string(totalFiles) + "\n";
	resultString += "Matched Lines: " + std::to_string(totalLines) + "\n";
	if (hasMore)
	{
		resultString += "\n";
		resultString += "Note: Only first " + std::to_string(maxResult) + " results are shown due to the return result limit.\n";
	}
	resultString += "========================================\n\n";

	// Iterate through each file result
	size_t displayedCount = 0;
	bool limitReached = false;
	bool firstFile = true;

	for (const auto& fileInfo : results.fileInfos)
	{
		if (limitReached)
			break;

		resultString += "----------------------------------------\n";
		resultString += "File: " + fileInfo.filePath + "\n";
		resultString += "Matched Lines: " + std::to_string(fileInfo.lineInfos.size()) + "\n";
		resultString += "----------------------------------------\n";

		// Iterate through each matched line
		for (const auto& lineInfo : fileInfo.lineInfos)
		{
			// Check if max display limit is reached
			if (maxResult > 0 && displayedCount >= static_cast<size_t>(maxResult))
			{
				limitReached = true;
				break;
			}

			resultString += "  [" + std::to_string(lineInfo.lineNumber) + "] ";
			
			// Display symbol name if available
			if (!lineInfo.symbolName.empty())
			{
				std::string symbolName = lineInfo.symbolName;
				symbolName = RestoreSymbolName(symbolName);
				resultString += "(" + symbolName + ") ";
			}
			
			// Display line content
			resultString += lineInfo.lineContent + "\n";
			displayedCount++;
		}

		if (limitReached && displayedCount < static_cast<size_t>(totalLines))
		{
			resultString += "  ... (" + std::to_string(totalLines - displayedCount) + " more lines not shown)\n";
		}

		resultString += "\n";
	}

	resultString += "========================================\n";
	resultString += "Search completed";
	if (hasMore)
	{
		resultString += " (showing first " + std::to_string(maxResult) + " lines)";
	}
	resultString += "\n";
	resultString += "========================================\n";
}


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


const char* GetDBRootFolder()
{
	static char db_folder[MAX_PATH] = {0};
	static bool initialized = false;
	
	if (!initialized)
	{
		char app_data_path[MAX_PATH];
		if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, app_data_path) == S_OK)
		{
			snprintf(db_folder, MAX_PATH, "%s\\%s", app_data_path, LAZYBUG_DB_FOLDER);
		}
		else
		{
			// 如果获取失败，使用默认路径
			snprintf(db_folder, MAX_PATH, "%s", LAZYBUG_DB_FOLDER);
		}
		initialized = true;
	}
	
	return db_folder;
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

bool CopyFile(const char* srcPath, const char* targetPath)
{
	// 参数验证
	if (!srcPath || *srcPath == '\0' || !targetPath || *targetPath == '\0')
		return false;

	// 检查源文件是否存在
	if (!IsFileExist(srcPath))
		return false;

	// 使用 GetFileFolderPath 获取目标路径的父目录，确保目录存在
	std::string strTargetPath(targetPath);
	std::string parentDir = GetFileFolderPath(strTargetPath);
	if (!parentDir.empty())
	{
		EnsureFolder(parentDir.c_str());
	}

	// 使用 Windows API CopyFile 函数进行文件复制
	BOOL success;
	if (is_pure_ascii(srcPath) && is_pure_ascii(targetPath))
	{
		// 第三个参数为 FALSE，表示如果目标文件存在则覆盖
		success = ::CopyFileA(srcPath, targetPath, FALSE);
	}
	else
	{
		std::wstring wSrcPath = utf8_to_widechar(srcPath);
		std::wstring wTargetPath = utf8_to_widechar(targetPath);
		// 第三个参数为 FALSE，表示如果目标文件存在则覆盖
		success = ::CopyFileW(wSrcPath.c_str(), wTargetPath.c_str(), FALSE);
	}

	return success != 0;
}



}
