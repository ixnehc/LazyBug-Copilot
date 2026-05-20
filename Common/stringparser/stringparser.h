// STRINGPARSER.h: interface for the CSTRINGPARSER class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>
#include <list>
#include <cstdlib>

#if defined(UNICODE) || defined(_UNICODE)
inline const wchar_t* fromMBCS(const char* mbStr)
{
	static const int BUFFER_COUNT = 8;  // ѭ��buffer����
	static const int BUFFER_SIZE = 512; // ÿ��buffer��С
	static __declspec(thread) wchar_t buffers[BUFFER_COUNT][BUFFER_SIZE];
	static __declspec(thread) int currentBuffer = 0;

	wchar_t* buffer = buffers[currentBuffer];
	buffer[0] = L'\0';

	if (mbStr == NULL)
	{
		return buffer;
	}

	mbstowcs(buffer, mbStr, BUFFER_SIZE - 1);
	buffer[BUFFER_SIZE - 1] = L'\0';

	// ѭ��ʹ��buffer
	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;
	return buffer;
}

inline const char* toMBCS(const wchar_t* wStr)
{
	static const int BUFFER_COUNT = 8;
	static const int BUFFER_SIZE = 512;
	static __declspec(thread) char buffers[BUFFER_COUNT][BUFFER_SIZE];
	static __declspec(thread) int currentBuffer = 0;

	char* buffer = buffers[currentBuffer];
	buffer[0] = '\0';

	if (wStr == NULL)
	{
		return buffer;
	}

	wcstombs(buffer, wStr, BUFFER_SIZE - 1);
	buffer[BUFFER_SIZE - 1] = '\0';

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;
	return buffer;
}
#else
inline const char* fromMBCS(const char* str)
{
	return str;
}

inline const char* toMBCS(const char* str)
{
	return str;
}
#endif



#define LEFT_STRING(s,count) ((s).substr(0,count))
#define RIGHT_STRING(s,count) ((s).substr((s).length()-(count),count))

//append a string ,and add a seperator between the 2 parts
#define APPEND_SEP_STRING(s,sep,append)\
{	\
	if (!((s)==""))\
		(s)=(s)+(sep);\
	(s)=(s)+(append);\
}


extern int IntFromString(const char *s);
extern int IntFromStringW(unsigned short *pWideChar);
extern double DoubleFromString(const char *s);
extern int SeperateStringBy(const char *sep,std::string &s,std::string &sRet);
extern char *SeperateStringBy(char sep,char *&buf);
extern void SplitStringBy(const char *sep,const std::string &total,std::vector<std::string>*pPieces);
extern void SplitStringBy(const char *sep, const std::string &total,std::list<std::string>*pPieces);
extern void SplitStringBy(const wchar_t *sep,const std::wstring &total,std::vector<std::wstring>*pPieces);
extern void LinkStringBy(const char *linker,std::string &total,std::vector<std::string>*pPieces);
extern void LinkStringBy(const char *linker,std::string &total,std::list<std::string>*pPieces);
extern void ReplaceCharInString(char cOld,char cNew,char *pString);
extern void SplitLines(const std::string& str, std::vector<std::string>& lines);


extern void MakeFileSuffix(std::string &fn,const char *suffix);
extern void RemoveFileSuffix(std::string &fn);
extern BOOL CheckFileSuffix(std::string &fn,const char *suffix);
extern BOOL CheckFileSuffix(const char *fn,const char *suffix);
extern std::string GetFileSuffix(const std::string &fn);
extern std::string GetFileTitle(const std::string &fn);
extern std::string GetFileName(const std::string &fn);
extern BOOL CheckFileName(const char *fn,const char *name);
extern std::string GetFileFolderPath(const std::string &fn);
extern BOOL IncreaseFileTailOrdinal(std::string &fn,int nOrdinalWidth);//c:\\file_001.txt --> file_002.txt,when nOrdinalWidth is 3
extern BOOL IncreaseTailOrdinal(std::string &s,int nOrdinalWidth);
extern std::string CutHeadPath(const char *path,const char *pathHead);//will remove the leading slash
extern BOOL CheckPathContaining(const char *path1,const char *path2);//Check whether path2 is under path1
extern BOOL CheckStringHeadNoCase(const char* str, const char* strHead);//支持mbcs

extern BOOL ResolveRelativePath(std::string &pathRel,const char *pathBase,const char *path);//NOTE:pathBase should NOT contain the ending slash

extern std::string GetSubPathFromHead(const char *path,int &nLevels);//nLevels indicates how many level the sub path includes,if 0,calculate the actual levels.
extern std::string CutSubPathFromHead(const char *path,int nLevels);//nLevels indicates how many level the sub path includes,the sub path will be removed from the org path,(the leading slash of the remaining path will be removed too)


extern BOOL CutTailSubPath(std::string &path,std::string &pathSub);
extern int CutTailSubPath(char *path,char *pathSub);

extern std::string GetModuleFolderPath(HMODULE hModule);
extern BOOL IsFullPath(const char *path);//check whether this path is a full path

extern void StringUpper(std::string &s);
extern void StringLower(std::string &s);
extern void StringLower(char*s);

//utf8字串中,路径中的'/'替换为'\\'
extern void FixSlashInPath_Utf8(char* s);

extern BOOL StringEqualNoCase(const char *s1,const char *s2);//comparing without considering case sensitive,MBCS support
extern int StringCmpNoCase(const char *s1,const char *s2);//comparing without considering case sensitive,MBCS support

extern void RemoveTailBlank(std::string &s);
extern void RemoveHeadBlank(std::string &s);
extern void RemoveBlank(std::string &s);
extern BOOL IsBlankString(const char *s);

BOOL IsAllBlank(const char* s);


extern void RemoveTailNumber(std::string &s);

extern BOOL IsMBString(const std::string &s);//whether the string has multibyte code

extern void FormatString(std::string &s,const char *formatstring,...);
extern void FormatString(char *buf,const char *formatstring,...);
extern void AppendFmtString(std::string &s,const char *formatstring,...);

extern int StringReverseFind(const char *s,char c);//return -1 if  not found
extern int StringFind(const char *s,char c);//return -1 if  not found

extern const char *ReplaceString(const char *s,const char *toFind,const char *toReplace);
extern std::string CullStringChunk(const char* content, const char* startMark, const char* endMark);

extern void StrSafeCopy(char *dest,const char *src,DWORD sz);

extern DWORD CalcHashCode(const char *str);
extern DWORD CalcHashCodeReverse(const char *str);

extern const char *MakeShortStr(const char *str,DWORD nLen);


extern WORD*ConvertToWC(const char *str,DWORD &nLen);

extern const char *ResolveRefPath(const char *pathRel,const char *pathOwner);

// 处理相对路径,将包含..的路径转换为绝对路径
extern std::string ResolveRelativePathWithDots(const std::string& path);

// 生成目标路径相对于根路径的相对路径
extern std::string MakeRelativePath(const char *pathRoot, const char *path);

extern void ConvertFullPathToName(const char* pathSln, std::string& name);


extern std::wstring utf8_to_widechar(const char* utf8_str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::string utf8_to_local(const std::string& utf8_str);
extern std::wstring local_to_widechar(const char* str);
extern std::string local_to_utf8(const std::string& ansi_str);
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::string widechar_to_local(const wchar_t* str);


extern std::string MakeDateFileName(const char* suffix);

extern std::wstring EscapeJsonString(const std::wstring& input);
extern std::wstring UnescapeJsonString(const std::wstring& input);
extern std::wstring EscapeHtml(const std::wstring& input);

extern std::string FilePathToUri(const std::string& filePath);
extern std::string UriToFilePath(const std::string& uri);

extern bool is_pure_ascii(const char* str);

extern bool OmitFullPath(std::string& fullPath);