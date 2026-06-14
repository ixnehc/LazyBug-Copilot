#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include "timer/timer.h"
#include <fstream>

struct FindInFileResults;
struct SolutionDump;

namespace Utils
{

enum class FileContentCodingFormat 
{
	None,
	Utf8_WithSignature,
	Utf8,
	Local,
};

extern AbsTick GetFileTick(const char* path);
extern time_t GetFileTimeT(const char* path);
extern time_t GetCurFileTimeT();
extern FILETIME GetZeroFileTime();
extern bool IsFileExist(const char* path);
extern bool IsFileReadOnly(const char* path);
extern bool is_valid_utf8(const std::string& s);
extern void make_complete_utf8(std::string& s, std::string& tail);
extern void EnsureFolder(const char* path);
extern void EnsureFileFolder(const char* filePath);
extern FILETIME GetFileTime(const char* path);
extern FILETIME GetFileTime(const wchar_t* path);
extern bool SetFileTick(const char* path, AbsTick tick);
extern bool SetFileTime(const char* path, const FILETIME& fileTime);
extern bool EqualFileTime(const FILETIME& a, const FILETIME& b);
extern bool ConvertFileContentIntoUTF8(const std::vector<BYTE>& raw_content, std::vector<BYTE>& processed_content, FileContentCodingFormat& codingFmt);
extern bool ConvertStringIntoUtf8(const std::string& raw_str, std::string& str);
extern bool ConvertFileContentFromUTF8(std::vector<BYTE>& raw_content, const std::vector<BYTE>& processed_content, FileContentCodingFormat codingFmt);
extern bool GetFileContentIntoUTF8(const char* path, std::string& content, FileContentCodingFormat &codingFmt);
extern bool GetFileContentIntoUTF8(const char* path, std::vector<BYTE>& content, FileContentCodingFormat& codingFmt);
extern bool SetFileContentFromUTF8(const char* path, const std::vector<BYTE>& content, FileContentCodingFormat codingFmt);
extern bool SetFileContentFromUTF8(const char* path, const std::string& content, FileContentCodingFormat codingFmt);
extern bool GetFilePartIntoUTF8(const char* path, int startLine, int endLine, std::string& content, FileContentCodingFormat& codingFmt);//返回[startLine,endLine]的内容,为0-base的行号
extern bool LoadFileContent(const char* path, std::vector<BYTE>& content);
extern bool LoadFileContent(const char* path, std::string& content);
extern bool SaveFileContent(const char* path, const std::string& content);
extern bool SaveFileContent(const char* path, const std::vector<BYTE>& content);
extern const char* SkipCodingHead(const char* content);
extern std::wstring GetActualFilePath(const wchar_t* fullPath);
extern std::string GetActualFilePath(const char* fullPath);
extern std::string GetActualFileName(const char* fullPath);
extern bool RemoveFile(const char* path);
extern bool CopyFile(const char* srcPath, const char* targetPath);
extern bool OpenIFStream(std::ifstream& ifs, const char* path);
extern bool OpenOFStream(std::ofstream& ifs, const char* path, int mode = std::ios::binary| std::ios::out);


}
