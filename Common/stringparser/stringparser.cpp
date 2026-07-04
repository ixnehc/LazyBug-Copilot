/********************************************************************
	created:	27:7:2006   20:29
	file path:	d:\IxEngine\Common\stringparser
	file base:	stringparser
	file ext:	cpp
	author:		cxi
	
	purpose:	useful functions for string operations
*********************************************************************/
#include "stdh.h"
#include "stringparser.h"
#include <sstream>

#include <stdio.h>

#pragma warning(disable:4267)
#pragma warning(disable:4018)


#define TRUE 1
typedef char * LPSTR;

//considering multibyte
//c should not be DBCS leading code(under 128)
int StringReverseFind(const char *s,char c)
{
	char *p;
	p=(char*)s;

	int iFind;
	iFind=-1;
	while(*p)
	{
		if(*p==c)
			iFind=(int)(p-s);

		if (IsDBCSLeadByte(*p))
			p++;
		p++;
	}
	return iFind;
}

//considering multibyte
//c should not be DBCS leading code(under 128)
int StringFind(const char *s,char c)
{
	char *p;
	p=(char*)s;

	while(*p)
	{
		if(*p==c)
			return (int)(p-s);

		if (IsDBCSLeadByte(*p))
			p++;
		p++;
	}
	return -1;
}


 
int IntFromStringW(unsigned short *pWideChar)
{
	return (int)_wtol((const wchar_t *)pWideChar);
}

//Return whether something is cut
BOOL CutTailSubPath(std::string &path,std::string &pathSub)
{
	std::string s;
	s=path;
	int iFind;
	iFind=StringReverseFind(s.c_str(),'\\');
	if (iFind==-1)
		return false;
	pathSub=path.c_str()+iFind+1;
	path=path.substr(0,iFind);
	return true;
}

//Return the length of the subpath.if 0,nothing is cut.the remaining path does not include the slash
//Supports MBCS(the path should not contain any Chinese code)
//the buffer of pathSub is assumed big enough
int CutTailSubPath(char *path,char *pathSub)
{
	std::string s;
	s=path;
	int iFind;
	iFind=StringReverseFind(s.c_str(),'\\');
	if (iFind==-1)
		return 0;
	strcpy(pathSub,path+iFind+1);
	path[iFind]=0;
	return strlen(pathSub);
}


//////////////////////////////////////////////////////////////////////////
//Stl string


int IntFromString(const char *s)
{
	return (int)atol(s);
}

double DoubleFromString(const char *s)
{
	return atof(s);
}

//Seperate s,the head part is stored in sRet,while the tail part is stored in s. The sep is deleted from s.
BOOL SeperateStringBy(const char *sep,std::string &s,std::string &sRet)
{ 
	int nStart;
	nStart=0;

	if (true)
	{
		int len,len2;
		len=strlen(sep);
		len2=s.length();
		char *p;
		while(1)
		{
			p=(char*)(s.c_str()+nStart);
			if (memcmp(p,sep,len)==0)
				break;
			if (IsDBCSLeadByte(*p))
				nStart+=2;
			else
				nStart++;
			if (nStart>=len2)
			{
				nStart=-1;
				break;
			}
		}
	}

	if (nStart==-1)
		return false;

	sRet=LEFT_STRING(s,nStart);
	int count;
	count=s.length()-(sRet+sep).length();
	s=RIGHT_STRING(s,count);
	return true;
}

void SplitStringBy(const char *sep,const std::string &total,std::vector<std::string>*pPieces)
{
	std::string s,ss;
	s=total;
	pPieces->clear();
	if (total=="")
		return;
	while(1)
	{
		if (SeperateStringBy(sep,s,ss))
			pPieces->push_back(ss);		
		else
		{
			pPieces->push_back(s);
			break;
		}
	}
}

//从buf开始找第一个sep,找到后,将找到的位置上的字符置为0,将buf指向sep后的第一个字符,返回buf被修改前的值
//目前不支持MBCS(中文)
char *SeperateStringBy(char sep,char *&buf)
{
	char *ret=buf;
	char *p=buf;
	while(*p)
	{
		if (*p==sep)
		{
			*p=0;
			buf=p+1;
			break;
		}
		p++;
	}
	return ret;

}



//use the pPieces to combine a full string into total,use linker to link them
//A reverse doing of SplitStringBy(...)
void LinkStringBy(const char *linker,std::string &total,std::vector<std::string>*pPieces)
{
	total="";
	if (pPieces->size()<=0)
		return;
	int i;
	total=(*pPieces)[0];
	for (i=1;i<pPieces->size();i++)
	{
		total+=linker;
		total+=(*pPieces)[i];
	}
}

void SplitStringBy(const char *sep,const std::string &total,std::list<std::string>*pPieces)
{
	std::string s,ss;
	s=total;
	pPieces->clear();
	if (total=="")
		return;
	while(1)
	{
		if (SeperateStringBy(sep,s,ss))
			pPieces->push_back(ss);		
		else
		{
			pPieces->push_back(s);
			break;
		}
	}
}

// 辅助函数：分离宽字符串的第一部分
BOOL SeperateStringByW(const wchar_t *sep, std::wstring &s, std::wstring &sRet)
{
	int nStart = 0;
	int len = static_cast<int>(wcslen(sep));
	int len2 = static_cast<int>(s.length());
	
	while (nStart < len2)
	{
		const wchar_t *p = s.c_str() + nStart;
		if (wmemcmp(p, sep, len) == 0)
			break;
		nStart++;
		if (nStart >= len2)
		{
			nStart = -1;
			break;
		}
	}

	if (nStart == -1)
		return FALSE;

	sRet = LEFT_STRING(s, nStart);
	int count = s.length() - (sRet + sep).length();
	s = RIGHT_STRING(s, count);
	return TRUE;
}

void SplitStringBy(const wchar_t *sep,const std::wstring &total,std::vector<std::wstring>*pPieces)
{
	std::wstring s, ss;
	s = total;
	pPieces->clear();
	if (total == L"")
		return;
	while (1)
	{
		if (SeperateStringByW(sep, s, ss))
			pPieces->push_back(ss);
		else
		{
			pPieces->push_back(s);
			break;
		}
	}
}

//use the pPieces to combine a full string into total,use linker to link them
//A reverse doing of SplitStringBy(...)
void LinkStringBy(const char *linker,std::string &total,std::list<std::string>*pPieces)
{
	total="";
	if (pPieces->size()<=0)
		return;
	std::list<std::string>::iterator it=pPieces->begin();
	total=*it;
	it++;
	while(it!=pPieces->end())
	{
		total+=linker;
		total+=(*it);
		it++;
	}
}

void SplitLines(const std::string& str, std::vector<std::string> &lines)
{
	lines.clear();
	std::string line;
	for (size_t i = 0; i < str.size(); )
	{
		if (str[i] == '\r')
		{
			if (i + 1 < str.size() && str[i + 1] == '\n')
			{
				lines.push_back(line);
				line.clear();
				i += 2;
			}
			else
			{
				lines.push_back(line);
				line.clear();
				i += 1;
			}
		}
		else if (str[i] == '\n')
		{
			lines.push_back(line);
			line.clear();
			i += 1;
		}
		else
		{
			line.push_back(str[i]);
			i += 1;
		}
	}
	lines.push_back(line);
}


//Remove the file's suffix
void RemoveFileSuffix(std::string &fn)
{
	int lTitle,lTitle2;
	lTitle=StringReverseFind(fn.c_str(),'.');
	lTitle2=StringReverseFind(fn.c_str(),'\\');
	if (lTitle>lTitle2)
		fn=LEFT_STRING(fn,lTitle);
}

//add a suffix to the file name if it hasn't one.If the fn already has a suffix,donot modify it.
void MakeFileSuffix(std::string &fn,const char *suffix)
{
	int lTitle,lTitle2;
	lTitle=StringReverseFind(fn.c_str(),'.');
	lTitle2=StringReverseFind(fn.c_str(),'\\');
	if (lTitle>lTitle2)
	{
		//Already has one,donot mofify it
	}
	else
	{
		fn+=".";
		fn+=suffix;
	}
}

std::string GetFileName(const std::string &fn)
{
	std::string s=GetFileSuffix(fn);
	if (!s.empty())
		return GetFileTitle(fn)+"."+GetFileSuffix(fn);
	return GetFileTitle(fn);
}

//注意:支持MBCS
BOOL CheckFileName(const char *fn,const char *name)
{
	int lTitle=StringReverseFind(fn,'\\');

	char *p=(char *)fn+lTitle+1;
	char *q=(char*)name;
	while(*p)
	{
		char c1=*p;
		char c2=*q;
		if (IsDBCSLeadByte(c1))
		{
			if (c1!=c2)
				return false;
			p++;
			q++;
			if (*p!=*q)
				return false;
			p++;
			q++;
			continue;
		}
		if ( ('A' <= c1) && (c1<= 'Z') )
			c1 += 'a' - 'A';
		if ( ('A' <= c2) && (c2<= 'Z') )
			c2 += 'a' - 'A';
		if (c1!=c2)
			return false;
		p++;
		q++;
	}

	return true;
}



std::string GetFileSuffix(const std::string &fn)
{
	int lTitle;
	lTitle=StringReverseFind(fn.c_str(),'.');
	if (lTitle>=0)
	{
		int count;
		count=fn.length()-1-lTitle;
		return RIGHT_STRING(fn,count);
	}
	else
		return std::string("");
}


//注意:没有考虑MBCS
BOOL CheckFileSuffix(const char *fn,const char *suffix)
{
	int lTitle=StringReverseFind(fn,'.');

	char *p=(char *)fn+lTitle+1;
	char *q=(char*)suffix;
	while(*p)
	{
		char c1=*p;
		char c2=*q;
		if ( ('A' <= c1) && (c1<= 'Z') )
			c1 += 'a' - 'A';
		if ( ('A' <= c2) && (c2<= 'Z') )
			c2 += 'a' - 'A';
		if (c1!=c2)
			return false;
		p++;
		q++;
	}

	return true;
}

BOOL CheckFileSuffix(std::string &fn,const char *suffix)
{
	return CheckFileSuffix(fn.c_str(),suffix);
}


std::string GetFileTitle(const std::string &fn)
{
	int lPath;
	lPath=StringReverseFind(fn.c_str(),'\\');

	std::string sTitle;
	if (lPath>=0)
	{
		int count;
		count=fn.length()-1-lPath;
		sTitle=RIGHT_STRING(fn,count);
	}
	else
		sTitle=fn;

	RemoveFileSuffix(sTitle);
	return sTitle;

}

std::string GetFileFolderPath(const std::string &fn)
{
	size_t lPath = fn.find_last_of("\\/");

	std::string sFolder;
	if (lPath != std::string::npos)
		sFolder = fn.substr(0, lPath);

	return sFolder;
}



//Not include the "\\" at the tail
std::string GetSubPathFromHead(const char *path,int &nLevels)//nLevels indicates how many levels the sub path includes
{
	LPSTR p,p0;
	p0=p=(LPSTR)path;

	std::string s;

	int n;
	n=0;
	while(*p)
	{
		if (*p=='\\')
		{
			if (p-1>=p0)
			{
				if (*(p-1)=='\\')
				{
					if (nLevels>0)
						s+=*p;
					p++;
					continue;
				}
			}

			n++;

			if (nLevels>0)
			{
				if (n>=nLevels)
					return s;
			}
		}

		if (IsDBCSLeadByte(*p))
		{
			if (nLevels>0)
				s+=*p;
			p++;
		}
		if (nLevels>0)
			s+=*p;
		p++;
	}

	nLevels=n+1;
	return std::string(path);
}


//nLevels indicates how many level the sub path includes,the sub path will be removed from the org path,(the leading slash of the remaining path will be removed too)
std::string CutSubPathFromHead(const char *path_,int nLevels)
{
	std::string path=path_;
	std::string s;
	s=GetSubPathFromHead(path.c_str(),nLevels);

	int len;
	len=s.length()+1;//The leading slash for the remaining path

	len=path.length()-len;
	if (len<=0)
		path="";
	else
		path=RIGHT_STRING(path,len);

	return s;
}

//cOld,cNew should be a normal char(under 128,not a DBCS leading byte)
void ReplaceCharInString(char cOld,char cNew,char *pString)
{
	char *p;
	p=pString;
	while(*p)
	{
		if ((*p)==cOld)
			*p=cNew;
		if (IsDBCSLeadByte(*p))
			p++;
		p++;
	}
}


std::string GetModuleFolderPath(HMODULE hModule)
{
#if defined(UNICODE) || defined(_UNICODE)
	wchar_t buffer[512];
#else
	char buffer[512];
#endif
	GetModuleFileName(hModule,buffer,500);
	std::string s;
	s = toMBCS(buffer);
	int l;
	l=s.rfind('\\');
	s=((s).substr(0,l));
	return s;
}

//support multibyte
void StringUpper(std::string &s)
{
	char *p;
	p=(char*)s.c_str();
	while(*p)
	{
		if (IsDBCSLeadByte(*p))
		{
			p+=2;
			continue;
		}
		if ( ('a' <= *p) && (*p <= 'z') )
			*p -= 'a' - 'A';
		p++;
	}
}

//support multibyte
void StringLower(char*s)
{
	char *p;
	p=(char*)s;
	while(*p)
	{
		if (IsDBCSLeadByte(*p))
		{
			p+=2;
			continue;
		}
		if ( ('A' <= *p) && (*p <= 'Z') )
			*p += 'a' - 'A';
		p++;
	}
}


void StringLower(std::string &s)
{
	char *p;
	p=(char*)s.c_str();
	StringLower(p);
}

//utf8字串中,路径中的'/'替换为'\\'
void FixSlashInPath_Utf8(char* s)
{
    if (!s || !*s)
        return;
        
    char* p = s;
    while (*p)
    {
        // UTF-8 多字节字符的第一个字节最高位为1
        if ((*p & 0x80) != 0)
        {
            // 跳过整个UTF-8字符
            // 根据第一个字节确定字符长度
            int charLen = 0;
            if ((*p & 0xE0) == 0xC0) charLen = 2;  // 2字节字符
            else if ((*p & 0xF0) == 0xE0) charLen = 3;  // 3字节字符
            else if ((*p & 0xF8) == 0xF0) charLen = 4;  // 4字节字符
            else
            {
                // 无效的UTF-8字符，跳过
                p++;
                continue;
            }
            
            p += charLen;
            continue;
        }
        
        // 处理ASCII字符
        if (*p == '/')
            *p = '\\';
            
        p++;
    }
}

std::string CutHeadPath(const char *path_,const char *pathHead_)
{
	if (path_[0]==0)
		return std::string("");
	std::string path=path_;
	std::string pathHead=pathHead_;
	return RIGHT_STRING(path,path.length()-(pathHead.length()+1));//+1 for a leading slash
}


//comparing without considering case sensitive,MBCS support
BOOL StringEqualNoCase(const char *s1,const char *s2)
{
	return (_stricmp(s1,s2)==0);
}

int StringCmpNoCase(const char *s1,const char *s2)
{
	return _stricmp(s1,s2);
}

void RemoveTailNumber(std::string &s)
{
	char *p;
	p=(char *)s.c_str();
	char *pLastSpace;
	pLastSpace=p;

	while(*p)
	{
		if (IsDBCSLeadByte(*p))
		{
			p++;
			if (*p)
				p++;
			pLastSpace=p;
			continue;
		}
		if (!isdigit(*p))
			pLastSpace=p+1;
		p++;
	}

	s=s.substr(0,pLastSpace-s.c_str());
}

//support mbcs
void RemoveBlank(std::string &s)
{
	std::string ss;

	char *p=(char *)s.c_str();
	while(*p)
	{
		if (IsDBCSLeadByte(*p))
		{
			ss+=(*p);
			ss+=(*(p+1));
			p+=2;
			continue;
		}
		if (!isspace(*p))
			ss+=(*p);
		p++;
	}

	s=ss;
}

BOOL IsAllBlank(const char* s)
{
	if (!s || *s == '\0')          // NULL 或空串
		return TRUE;

	for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
		*p != '\0'; ++p)
	{
		if (!std::isspace(*p))
			return FALSE;
	}
	return TRUE;
}

//support mbcs
void RemoveHeadBlank(std::string &s)
{
	char *p=(char *)s.c_str();
	while(*p)
	{
		if (IsDBCSLeadByte(*p))
			break;
		if (!isspace(*p))
			break;
		p++;
	}
	std::string s2=p;
	s=s2;
}


//support mbcs
void RemoveTailBlank(std::string &s)
{
	char *p;
	p=(char *)s.c_str();
	char *pLastSpace;
	pLastSpace=p;

	while(*p)
	{
		if (IsDBCSLeadByte(*p))
		{
			p++;
			if (*p)
				p++;
			pLastSpace=p;
			continue;
		}
		if (!isspace(*p))
			pLastSpace=p+1;
		p++;
	}

	s=s.substr(0,pLastSpace-s.c_str());
}

BOOL IsBlankString(const char *s)
{
	std::string s1=s;
	RemoveTailBlank(s1);
	if (s1=="")
		return true;
	return false;
}


//whether the string has multibyte code
BOOL IsMBString(const std::string &s)
{
	char *p;
	p=(char *)s.c_str();

	while(*p)
	{
		if (IsDBCSLeadByte(*p))
			return true;
		p++;
	}
	return false;
}


char g_strbuffer[4096];
void FormatString(std::string &s,const char *formatstring,...)
{
	va_list args;

	va_start(args,formatstring);
	int nSize = _vsnprintf(g_strbuffer,sizeof(g_strbuffer), formatstring,args);
	va_end(args);

	s=g_strbuffer;
}

void FormatString(char *buf,const char *formatstring,...)
{
	va_list args;

	va_start(args,formatstring);
	int nSize = _vsnprintf(buf,4096, formatstring,args);
	va_end(args);
}

void AppendFmtString(std::string &s,const char *formatstring,...)
{
	va_list args;

	va_start(args,formatstring);
	int nSize = _vsnprintf(g_strbuffer,sizeof(g_strbuffer), formatstring,args);
	va_end(args);

	s+=g_strbuffer;
}


const char *ReplaceString(const char *s0,const char *toFind,const char *toReplace)
{
	static std::string ret;
	ret="";
	std::string s=s0;

	int lenToFind=strlen(toFind);

	while(1)
	{
		int iFind;
		iFind=s.find(toFind);
		if (iFind==-1)
		{
			ret+=s;
			break;
		}
		
		ret+=LEFT_STRING(s,iFind);
		ret+=toReplace;

		s=RIGHT_STRING(s,s.length()-lenToFind-iFind);
	}

	return ret.c_str();
}

BOOL CheckStringHeadNoCase(const char* str, const char* strHead)
{
	if (!str || !strHead)
		return false;

	while (*strHead)
	{
		// 如果str已经结束但strHead还没结束，返回false
		if (*str == '\0')
			return false;

		// 检查是否是中文字符（GBK/GB2312中首字节通常是负值）
		if ((unsigned char)*strHead > 0x7F)
		{
			// 中文或其他双字节字符，需要精确比较两个字节
			if (*str != *strHead)
				return false;

			// 比较第二个字节（如果第一个字节已经不同，就不会执行到这里）
			str++;
			strHead++;

			// 防止第二个字节为空的异常情况
			if (*str == '\0' || *strHead == '\0')
				return false;

			if (*str != *strHead)
				return false;

			str++;
			strHead++;
		}
		else
		{
			// ASCII字符，忽略大小写比较
			char c1 = (*str >= 'A' && *str <= 'Z') ? *str + 32 : *str;
			char c2 = (*strHead >= 'A' && *strHead <= 'Z') ? *strHead + 32 : *strHead;

			if (c1 != c2)
				return false;

			str++;
			strHead++;
		}
	}

	// strHead已经结束，表示str确实以strHead开头
	return true;
}

//Check whether path2 is under path1
//note:path1 should not contains the ending slash("\")
//if the 2 pathes are the same,return false;
BOOL CheckPathContaining(const char *path1,const char *path2)
{
	int len1,len2;
	len1=strlen(path1);
	len2=strlen(path2);
	if (len1>=len2)
		return false;

	std::string s=path2;
	s.resize(len1);
	BOOL bRet=true;
	if (StringCmpNoCase(path1,s.c_str())!=0)
		bRet=false;
	else
	{
		if (path2[len1]!='\\')
			bRet=false;
	}

	return bRet;
}

//NOTE:pathBase should NOT contain the ending slash
//计算出path对于pathBase的相对路径,返回在pathRel
BOOL ResolveRelativePath(std::string &pathRel,const char *pathBase,const char *path)
{
	std::string s;
	int nLevels=0;
	GetSubPathFromHead(pathBase,nLevels);//get the total levels

	for(int i=nLevels;i>0;i--)
	{
		std::string s,s2;
		s=GetSubPathFromHead(pathBase,i);
		if (StringEqualNoCase(s.c_str(),path))
			s2="";
		else
		{
			if (CheckPathContaining(s.c_str(),path))
				s2=CutHeadPath(path,s.c_str());
			else
				continue;
		}

		pathRel="";
		for (int j=0;j<nLevels-i;j++)
		{
			if (j==0)
				pathRel+="..";
			else
				pathRel+="\\..";
		}

		if (s2!="")
		{
			if (pathRel!="")
				pathRel+="\\";
			pathRel+=s2;
		}


		return true;
	}

	return false;

}



//Add a tail ordinal with format "_%0nd" to the given file name
//example:
//when nOrdinalWidth is 3
//c:\file_001.txt --> c:\file_002.txt,
//c:\file_01.txt --> c:\file_01_001.txt,
//c:\file.txt --> c:\file_001.txt,
BOOL IncreaseFileTailOrdinal(std::string &fn,int nOrdinalWidth)
{
	BOOL bRet;
	std::string suffix;
	suffix=GetFileSuffix(fn);
	RemoveFileSuffix(fn);

	bRet=IncreaseTailOrdinal(fn,nOrdinalWidth);

	if (!suffix.empty())
		MakeFileSuffix(fn,suffix.c_str());

	return bRet;
}


BOOL IncreaseTailOrdinal(std::string &s,int nOrdinalWidth)
{
	std::string sBack=s;
	int ordinal=0;

	if (s!="")
	{
		char *p;
		p=(char*)s.c_str()+s.length()-1;

		int i;
		for (i=0;i<nOrdinalWidth;i++)
		{
			if (p<s.c_str())
				break;
			if (((BYTE)(*p))>=128)
				break;
			if (!isdigit(*p))
				break;
			p--;
		}
		if (i>=nOrdinalWidth)
		{
			if (*p=='_')
			{
				p++;
				ordinal=IntFromString(p);

				s=LEFT_STRING(s,s.length()-(nOrdinalWidth+1));
			}
		}
	}

	ordinal++;
	DWORD max=1;
	for (int i=0;i<nOrdinalWidth;i++)
		max*=10;
	if (ordinal>=max)
	{
		s=sBack;
		return false;
	}

	std::string pattern;
	FormatString(pattern,"_%%0%dd",nOrdinalWidth);
	AppendFmtString(s,pattern.c_str(),ordinal);

	return true;
}

void StrSafeCopy(char *dest,const char *src,DWORD sz)
{
	strncpy(dest,src,sz);
	dest[sz-1]=0;
}


//check whether this path is a full path
BOOL IsFullPath(const char *path)
{
	if (StringFind(path,':')>0)
		return true;
	if (path[0]=='\'')
	{
		if (path[1]=='\'')
			return true;
	}
	return false;
}

//use djb2 algorythm
DWORD CalcHashCode(const char *str)
{
	DWORD hash = 5381;
	DWORD c;
	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

/*
 Purpose: Calculate a reverse string hash code
*/
DWORD CalcHashCodeReverse(const char *str)
{
	DWORD hash = 5381;
	int len = static_cast<int>(strlen(str));
	char* p = const_cast<char*>(str);
	if (len > 0)
		p += len - 1;
	while (len-- > 0)
		hash = (hash<<5) + hash + *p--;
	return hash;
}


const char *MakeShortStr(const char *str,DWORD nLen)
{
	static char buf[256];
	if (nLen>sizeof(buf)-8)
		nLen=sizeof(buf)-8;
	char *p=(char*)str;
	int idx=0;
	while(*p)
	{
		char c=*p++;
		if ((c==0x0d)||(c==0x0a))
			break;
		buf[idx]=c;
		idx++;
		if (idx>=nLen)
			break;
	}
	if (*p)
	{
		buf[idx++]='.';
		buf[idx++]='.';
		buf[idx++]='.';
	}
	buf[idx]=0;
	return buf;
}

//如果s为空,s=s2,否则s=s+sep+s2;
void AppendSepStr(std::string &s,const char *sep,const char *s2)
{
	if (s.empty())
		s=s2;
	else
		s=s+sep+s2;
}

#define CODEPAGE_GB 936

WORD*ConvertToWC(const char *str,DWORD &nLen)
{
	static std::vector<WORD>codes;
	DWORD len=strlen(str);
	codes.resize(len);

	nLen=MultiByteToWideChar(CODEPAGE_GB,0,str,len,(LPWSTR)codes.data(),2*len);

	return codes.data();
}

const char *ConvertToMB(WORD*codes,DWORD nCodes)
{
	static std::vector<char>str;
	str.resize(nCodes*2);

	DWORD nLen=WideCharToMultiByte(CODEPAGE_GB,0,(LPCWSTR)codes,nCodes,str.data(),str.size(),NULL,0);
	str[nLen]=0;
	return str.data();
}

//把相对路径转换成绝对路径
//比如pathRel为".\abc.dds",pathOwner为"test\folder\abc.mtl",转换后的结果为"test\folder\abc.dds"
const char *ResolveRefPath(const char *pathRel,const char *pathOwner)
{
	static char buf[1024];
	if (pathRel[0]=='.')
	{
		if (pathRel[1]=='\\')
		{
			int idx=StringReverseFind(pathOwner,'\\');
			if (idx==-1)
				return pathRel+2;
			memcpy(buf,pathOwner,idx+1);
			strcpy(buf+idx+1,pathRel+2);
			return buf;
		}
	}
	return pathRel;
}

BOOL IsStringBeginWith(const char *s,const char *head)
{
	while((*s)&&(*head))
	{
		if ((*s)!=(*head))
			return false;
		s++;
		head++;
	}
	if ((*s)==0)
		return false;
	return true;
}

// 处理相对路径,将包含..的路径转换为绝对路径
std::string ResolveRelativePathWithDots(const std::string& path)
{
	// 使用SplitStringBy分割路径
	std::vector<std::string> pathParts;
	SplitStringBy("\\", path, &pathParts);

	BOOL modified = false;
	// 处理路径中的..和.
	std::vector<std::string> validParts;
	for (const auto& part : pathParts)
	{
		if (part == "..")
		{
			modified = true;
			if (!validParts.empty())
			{
				validParts.pop_back();
			}
		}
		else if (part != ".")
		{
			validParts.push_back(part);
		}
		else
			modified = true;
	}

	if (!modified)
		return path;

	// 使用LinkStringBy重新组合路径
	std::string result;
	LinkStringBy("\\", result, &validParts);
	return result;
}

void ConvertFullPathToName(const char* pathSln, std::string& name)
{
	name.clear();

	if (!pathSln)
		return;

	size_t len = strlen(pathSln);
	for (size_t i = 0; i < len; ++i)
	{
		// 处理":\\"模式 (如"C:\")
		if (i < len - 1 && pathSln[i] == ':' && pathSln[i + 1] == '\\')
		{
			name += '_';
			++i;  // 跳过后面的'\'
		}
		// 处理单独的'\'
		else if (pathSln[i] == '\\')
		{
			name += '_';
		}
		// 其他字符直接添加
		else
		{
			name += pathSln[i];
		}
	}
}

std::wstring utf8_to_widechar(const char* utf8_str)
{
	if (!utf8_str[0])
		return std::wstring(L"");
	// Step 1: UTF-8 → UTF-16
	// Calculate required buffer size for UTF-16 (excluding null terminator)
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str, (int)strlen(utf8_str), nullptr, 0);
	if (wlen == 0) return L""; // Error
	std::wstring utf16_str(wlen, 0);
	// Perform the conversion from UTF-8 to UTF-16
	MultiByteToWideChar(CP_UTF8, 0, utf8_str, (int)strlen(utf8_str), &utf16_str[0], wlen);

	return utf16_str;
}


std::wstring utf8_to_widechar(const std::string& utf8_str)
{
	if (utf8_str.empty())
		return std::wstring(L"");
	// Step 1: UTF-8 → UTF-16
	// Calculate required buffer size for UTF-16 (excluding null terminator)
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.length(), nullptr, 0);
	if (wlen == 0) return L""; // Error
	std::wstring utf16_str(wlen, 0);
	// Perform the conversion from UTF-8 to UTF-16
	MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.length(), &utf16_str[0], wlen);

	return utf16_str;
}

std::string utf8_to_local(const std::string& utf8_str)
{
	if (utf8_str.empty())
		return utf8_str;
	// Step 1: UTF-8 → UTF-16
	// Calculate required buffer size for UTF-16 (excluding null terminator)
	int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.length(), nullptr, 0);
	if (wlen == 0) return ""; // Error
	std::wstring utf16_str(wlen, 0);
	// Perform the conversion from UTF-8 to UTF-16
	MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.length(), &utf16_str[0], wlen);

	// Step 2: UTF-16 → Local Codepage (e.g., GBK)
	// Calculate required buffer size for local encoding (excluding null terminator)
	int len = WideCharToMultiByte(CP_ACP, 0, utf16_str.c_str(), wlen, nullptr, 0, nullptr, nullptr);
	if (len == 0) return ""; // Error
	std::string local_str(len, 0);
	// Perform the conversion from UTF-16 to local encoding
	WideCharToMultiByte(CP_ACP, 0, utf16_str.c_str(), wlen, &local_str[0], len, nullptr, nullptr);

	return local_str;
}

std::wstring local_to_widechar(const char* str)
{
	// 如果输入为空，直接返回
	if (!str[0])
	{
		return L"";
	}

	// 第一步：本地编码（如GBK）→ UTF-16
	int wlen = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
	if (wlen == 0)
	{
		return L"";
	}
	std::wstring utf16_str(wlen-1, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1, &utf16_str[0], wlen);
	return utf16_str;
}

std::string local_to_utf8(const std::string& ansi_str)
{
	// 如果输入为空，直接返回
	if (ansi_str.empty())
	{
		return "";
	}

	// 第一步：本地编码（如GBK）→ UTF-16
	int wlen = MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), -1, nullptr, 0);
	if (wlen == 0)
	{
		return "";
	}
	std::wstring utf16_str(wlen-1, 0);
	MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), -1, &utf16_str[0], wlen);

	// 第二步：UTF-16 → UTF-8
	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (utf8_len == 0)
	{
		return "";
	}
	std::string utf8_str(utf8_len-1, 0);
	WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), -1, &utf8_str[0], utf8_len, nullptr, nullptr);

// 	// 返回UTF-8字符串（去除结尾的null字符）
// 	if (!utf8_str.empty() && utf8_str.back() == '\0')
// 	{
// 		utf8_str.pop_back();
// 	}

	return utf8_str;
}

std::string widechar_to_utf8(const wchar_t* str)
{
	// 如果输入为空，直接返回
	if (str[0] == 0)
	{
		return "";
	}

	// 第二步：UTF-16 → UTF-8
	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	if (utf8_len == 0)
	{
		return "";
	}
	std::string utf8_str(utf8_len-1, 0);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, &utf8_str[0], utf8_len, nullptr, nullptr);

	return utf8_str;
}

std::string widechar_to_utf8(const wchar_t* str, int charCount)
{
	// 如果输入为空或长度为0，直接返回
	if (!str || charCount <= 0)
	{
		return "";
	}

	// UTF-16 → UTF-8 (指定字符数，不要求null终止)
	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, str, charCount, nullptr, 0, nullptr, nullptr);
	if (utf8_len == 0)
	{
		return "";
	}
	std::string utf8_str(utf8_len, 0);
	WideCharToMultiByte(CP_UTF8, 0, str, charCount, &utf8_str[0], utf8_len, nullptr, nullptr);

	return utf8_str;
}

std::string widechar_to_local(const wchar_t* str)
{
	// 如果输入为空，直接返回
	if (!str || str[0] == 0)
	{
		return "";
	}

	// UTF-16 → 本地编码（如GBK）
	int len = WideCharToMultiByte(CP_ACP, 0, str, -1, nullptr, 0, nullptr, nullptr);
	if (len == 0)
	{
		return "";
	}
	std::string local_str(len-1, 0);
	WideCharToMultiByte(CP_ACP, 0, str, -1, &local_str[0], len, nullptr, nullptr);

	return local_str;
}

std::string MakeDateFileName(const char* suffix)
{
	// 获取当前时间
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	// 创建文件名格式: YYYYMMDD_HHMMSS + suffix
	char timeStr[64];
	sprintf_s(timeStr, sizeof(timeStr), "%04d%02d%02d_%02d%02d%02d", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	
	std::string fileName = timeStr;
	
	// 如果提供了后缀，添加到文件名中
	if (suffix && strlen(suffix) > 0)
	{
		if (suffix[0] != '.')
		{
			fileName += ".";
		}
		fileName += suffix;
	}
	
	return fileName;
}

std::wstring EscapeJsonString(const std::wstring& input)
{
	std::wstring output;
	output.reserve(input.size() * 2);

	for (wchar_t c : input)
	{
		switch (c)
		{
		case L'\"': output += L"\\\""; break;
		case L'\\': output += L"\\\\"; break;
		case L'\b': output += L"\\b"; break;
		case L'\f': output += L"\\f"; break;
		case L'\n': output += L"\\n"; break;
		case L'\r': output += L"\\r"; break;
		case L'\t': output += L"\\t"; break;
		default:
			if (c < 32)
			{
				wchar_t buf[7];
				swprintf_s(buf, 7, L"\\u%04x", (int)c);
				output += buf;
			}
			else
			{
				output += c;
			}
			break;
		}
	}

	return output;
}

std::wstring UnescapeJsonString(const std::wstring& input)
{
	std::wstring output;
	output.reserve(input.size());

	for (size_t i = 0; i < input.size(); ++i)
	{
		wchar_t c = input[i];
		
		if (c == L'\\' && i + 1 < input.size())
		{
			// 处理转义字符
			wchar_t next = input[i + 1];
			switch (next)
			{
			case L'\"': output += L'\"'; ++i; break;
			case L'\\': output += L'\\'; ++i; break;
			case L'b': output += L'\b'; ++i; break;
			case L'f': output += L'\f'; ++i; break;
			case L'n': output += L'\n'; ++i; break;
			case L'r': output += L'\r'; ++i; break;
			case L't': output += L'\t'; ++i; break;
			case L'u':
				// 处理 \uXXXX 格式
				if (i + 5 < input.size())
				{
					wchar_t hex[5] = {0};
					for (int j = 0; j < 4; ++j)
					{
						hex[j] = input[i + 2 + j];
					}
					unsigned int unicode;
					if (swscanf_s(hex, L"%x", &unicode) == 1)
					{
						output += static_cast<wchar_t>(unicode);
						i += 5;
					}
					else
					{
						output += c;
					}
				}
				else
				{
					output += c;
				}
				break;
			default:
				// 未知的转义序列，保留原样
				output += c;
				break;
			}
		}
		else
		{
			output += c;
		}
	}

	return output;
}

std::wstring EscapeHtml(const std::wstring& input)
{
	std::wstring output;
	output.reserve(input.size());

	for (wchar_t c : input)
	{
		switch (c)
		{
		case L'&': output += L"&amp;"; break;
		case L'<': output += L"&lt;"; break;
		case L'>': output += L"&gt;"; break;
		case L'"': output += L"&quot;"; break;
		case L'\'': output += L"&#39;"; break;
		default: output += c; break;
		}
	}

	return output;
}


// 文件路径转换为LSP URI
std::string FilePathToUri(const std::string& filePath)
{
	std::string uri = "file:///";

	for (size_t i = 0; i < filePath.length(); ++i)
	{
		char c = filePath[i];

		if (c == '\\')
		{
			uri += '/';
		}
		else if (c == ' ')
		{
			uri += "%20";
		}
		else
		{
			uri += c;
		}
	}

	return uri;
}

// LSP URI转换为文件路径
std::string UriToFilePath(const std::string& uri)
{
	// 移除file:///前缀
	std::string filePath;
	const std::string prefix = "file:///";

	if (uri.compare(0, prefix.length(), prefix) == 0)
	{
		filePath = uri.substr(prefix.length());
	}
	else
	{
		return uri; // 非文件URI，直接返回
	}

	// 处理转义字符
	std::string result;
	for (size_t i = 0; i < filePath.length(); ++i)
	{
		if (filePath[i] == '%' && i + 2 < filePath.length())
		{
			int value;
			std::string hex = filePath.substr(i + 1, 2);
			std::stringstream ss;
			ss << std::hex << hex;
			ss >> value;
			result += static_cast<char>(value);
			i += 2;
		}
		else if (filePath[i] == '/')
		{
			result += '\\';
		}
		else
		{
			result += filePath[i];
		}
	}

	return result;
}

bool is_pure_ascii(const char* str)
{
	if (!str)
	{
		return true;
	}
	while (*str)
	{
		if (static_cast<unsigned char>(*str) > 127)
			return false;
		str++;
	}
	return true;
}

std::string CullStringChunk(const char* content, const char* startMark, const char* endMark)
{
	if (!content || !startMark || !endMark)
		return {};

	const char* startPos = strstr(content, startMark);
	if (!startPos)
		return {};

	startPos += strlen(startMark); // 跳过 startMark

	const char* endPos = strstr(startPos, endMark);
	if (!endPos)
		return {};

	return std::string(startPos, endPos); // [startPos, endPos)
}

// 省略文件路径中间的某一段
// 比如: C:\Users\xi.chen\AppData\Roaming\LazyBugDB\file.txt
// 第一次调用: C:\(...)\xi.chen\AppData\Roaming\LazyBugDB\file.txt
// 第二次调用: C:\(...)\AppData\Roaming\LazyBugDB\file.txt
// 如果无法再省略(只剩头尾),返回false
bool OmitFullPath(std::string& fullPath)
{
	if (fullPath.empty())
		return false;

	// 查找已有的(...)标记
	size_t omitPos = fullPath.find("(...)");
	
	// 路径分割成段
	std::vector<std::string> segments;
	
	if (omitPos != std::string::npos)
	{
		// 已经有(...)标记,我们需要在它后面继续省略
		// 将路径分为: 头部 + "(...)" + 尾部
		std::string headPart = fullPath.substr(0, omitPos);
		std::string tailPart = fullPath.substr(omitPos + 5); // 跳过"(...)"
		
		// 如果尾部以\\开头,跳过
		if (!tailPart.empty() && tailPart[0] == '\\')
			tailPart = tailPart.substr(1);
		
		// 分割尾部
		SplitStringBy("\\", tailPart, &segments);
		
		// 如果尾部只剩一段(文件名或最后一级目录),无法再省略
		if (segments.size() <= 1)
			return false;
		
		// 省略第一段
		fullPath = headPart + "(...)\\" + tailPart.substr(segments[0].length() + 1);
		return true;
	}
	else
	{
		// 第一次调用,还没有(...)标记
		SplitStringBy("\\", fullPath, &segments);
		
		// 需要至少3段才能省略(头、中间、尾)
		// 例如: C:\Users\file.txt 有3段: "C:", "Users", "file.txt"
		if (segments.size() < 3)
			return false;
		
		// 第一段是盘符或根目录
		std::string head = segments[0];
		if (head.empty())
			return false;
		
		// 最后一段是文件名
		std::string tail;
		for (size_t i = 2; i < segments.size(); i++)
		{
			if (!tail.empty())
				tail += "\\";
			tail += segments[i];
		}
		
		// 组合新路径: 头部 + \\(...)\\ + 尾部(从第三段开始)
		fullPath = head + "\\(...)\\" + tail;
		return true;
	}
}


// 生成目标路径相对于根路径的相对路径
// std::string MakeRelativePath(const char *pathRoot, const char *path)
// {
// 	// 处理空路径情况
// 	if (!pathRoot || !path || !*pathRoot || !*path)
// 	{
// 		return path;
// 	}
// 	
// 	// 标准化路径(去除结尾的反斜杠)
// 	std::string rootPath = pathRoot;
// 	std::string targetPath = path;
// 	
// 	// 去除路径末尾的反斜杠
// 	if (rootPath.back() == '\\')
// 	{
// 		rootPath.pop_back();
// 	}
// 	if (targetPath.back() == '\\')
// 	{
// 		targetPath.pop_back();
// 	}
// 	
// 	// 如果目标路径与根路径相同,返回"."
// 	if (StringEqualNoCase(rootPath.c_str(), targetPath.c_str()))
// 	{
// 		return ".";
// 	}
// 	
// 	// 检查目标路径是否在根路径下
// 	if (CheckPathContaining(rootPath.c_str(), targetPath.c_str()))
// 	{
// 		// 目标路径是根路径的子路径,移除根路径部分
// 		return targetPath.substr(rootPath.length() + 1); // +1跳过分隔符
// 	}
// 	
// 	// 如果不是子路径,需要计算相对路径
// 	// 分割路径
// 	std::vector<std::string> rootParts;
// 	std::vector<std::string> targetParts;
// 	SplitStringBy("\\", rootPath, &rootParts);
// 	SplitStringBy("\\", targetPath, &targetParts);
// 	
// 	// 找到公共前缀
// 	size_t commonPrefixLength = 0;
// 	size_t minLength = std::min(rootParts.size(), targetParts.size());
// 	
// 	for (; commonPrefixLength < minLength; ++commonPrefixLength)
// 	{
// 		if (!StringEqualNoCase(rootParts[commonPrefixLength].c_str(), targetParts[commonPrefixLength].c_str()))
// 		{
// 			break;
// 		}
// 	}
// 	
// 	// 构建相对路径
// 	std::string relativePath;
// 	
// 	// 首先,添加足够的".."返回到公共祖先
// 	for (size_t i = commonPrefixLength; i < rootParts.size(); ++i)
// 	{
// 		if (relativePath.empty())
// 		{
// 			relativePath = "..";
// 		}
// 		else
// 		{
// 			relativePath += "\\..";
// 		}
// 	}
// 	
// 	// 然后,从公共祖先添加到目标路径
// 	for (size_t i = commonPrefixLength; i < targetParts.size(); ++i)
// 	{
// 		if (relativePath.empty())
// 		{
// 			relativePath = targetParts[i];
// 		}
// 		else
// 		{
// 			relativePath += "\\";
// 			relativePath += targetParts[i];
// 		}
// 	}
// 	
// 	// 如果结果为空,返回"."表示当前目录
// 	if (relativePath.empty())
// 	{
// 		return ".";
// 	}
// 	
// 	return relativePath;
// }