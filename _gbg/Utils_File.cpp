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

std::string GetRealPathCase(const char* inputPath)
{
	// 先转为绝对路径
	char absPath[MAX_PATH] = {};
	if (!GetFullPathNameA(inputPath, MAX_PATH, absPath, nullptr))
		return inputPath;

	std::string path(absPath);
	std::string result;

	// 处理盘符部分，如 "C:\"
	size_t pos = 0;
	if (path.size() >= 2 && path[1] == ':')
	{
		result = path.substr(0, 3);       // e.g. "c:\"
		result[0] = toupper(result[0]);   // 盘符强制大写 -> "C:\"
		pos = 3;
	}

	// 逐段解析
	while (pos < path.size())
	{
		size_t sep = path.find('\\', pos);
		bool isLast = (sep == std::string::npos);
		std::string segment = isLast ? path.substr(pos) : path.substr(pos, sep - pos);

		std::string query = result + segment;

		WIN32_FIND_DATAA fd = {};
		HANDLE hFind = FindFirstFileA(query.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			result += fd.cFileName;  // 使用文件系统中的真实大小写名称
			FindClose(hFind);
		}
		else
		{
			result += segment;       // 找不到则保持原样
		}

		if (!isLast)
			result += '\\';

		pos = isLast ? path.size() : sep + 1;
	}

	return result;
}
std::string GetActualFilePath(const char* fullPath)
{
	if (!fullPath || *fullPath == '\0')
	{
		return "";
	}

	return GetRealPathCase(fullPath);

	std::wstring wPath = utf8_to_widechar(fullPath);

	// GetLongPathNameW 会查询文件系统并返回磁盘上存储的真实大小写路径名
	DWORD longPathLen = GetLongPathNameW(wPath.c_str(), NULL, 0);
	if (longPathLen == 0)
	{
		return fullPath;
	}

	std::vector<wchar_t> wBuffer(longPathLen);
	DWORD result = GetLongPathNameW(wPath.c_str(), wBuffer.data(), longPathLen);
	if (result == 0 || result >= longPathLen)
	{
		return fullPath;
	}

	std::wstring wActualPath(wBuffer.data(), result);

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
