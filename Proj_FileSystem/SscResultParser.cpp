/********************************************************************
created:	2008/02/22
created:	22:2:2008   16:36
filename: 	f:\Project Test\Ssc\Ssc\SscResultParser.cpp
file path:	f:\Project Test\Ssc\Ssc
file base:	SscResultParser
file ext:	cpp
author:		szg

purpose:	根据VSS的输出，判断文件状态，（特征字符串的选取来自测试输出）.
*********************************************************************/
#include "stdh.h"
#include "SscResultParser.h"
#include "stringparser/stringparser.h"

CSscResultParser::CSscResultParser()
{
}

void CSscResultParser::ClearLastResult()
{
	HANDLE hFile = CreateFile(_lastResultFile.c_str(), GENERIC_WRITE, 0, NULL, 
		OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile, 0, 0, FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);
	}
}

BOOL CSscResultParser::Whoami(String& who)
{
	String sResult;
	String::size_type pos;
	if (GetResultString(sResult) && 
		((pos = sResult.find(TAG_ENDLINE, 0)) != String::npos))
	{
		who = sResult.substr(0, pos);
		return TRUE;
	}
	return FALSE;
}

BOOL CSscResultParser::GetProjectItemsList(ProjItemList& items)
{	
	String sResult;
	if (GetResultString(sResult))
	{
		String::size_type off = 0;
		String::size_type pos;
		String::size_type end;
		while ((pos = sResult.find(TAG_ENDLINE, off)) != String::npos)
		{
			// Read line
			String line = sResult.substr(off, pos - off);
			if ((end = line.find(TAG_DIR)) != String::npos)
			{
				line.erase(line.begin() + end);
				items.push_back(line);
			}

			// Next line
			off = pos + 2;	// "\r\n"
		}
	}
	return (items.size() > 0);
}

/************************************************************************/
/*ss properties exec result:
Project:  $/[IxEngine]/IxEngine/Common/FilePackage
Contains:
12 Files
0 Subproject(s)
Latest:
Version:  13
Date:     08-02-03   18:15
Comment:                                                            */
/************************************************************************/
BOOL CSscResultParser::IsProject(const String& path)
{
	BOOL bOK = FALSE;
	String sResult;
	if (GetResultString(sResult))
	{
		String::size_type off = 0;
		String::size_type pos;
		String::size_type pos2;
		while ((pos = sResult.find(TAG_ENDLINE, off)) != String::npos)
		{
			String line = sResult.substr(off, pos - off);
			if ((pos2 = line.find(TAG_NOTEXIST, 0)) != String::npos)
			{
				bOK = FALSE;
				break;
			}
			else if ((pos2 = line.find(TAG_PROJECT, 0)) != String::npos)
			{
				pos2 += strlen(TAG_PROJECT);
				String project = line.substr(pos2, line.length() - pos2);
				RemoveHeadBlank(project);
				bOK = (project == path);
				break;
			}

			// Next line
			off = pos + 2;	// "\r\n"
		}
	}
	return bOK;
}

BOOL CSscResultParser::GetResultString(String& rResult)
{
	HANDLE hFile = CreateFile(_lastResultFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		const int MAX_BUFFER_LENGTH = 16 * 1024;
		char* pBuffer = new char[MAX_BUFFER_LENGTH];

		DWORD dwBytesRead = 0;
		while (ReadFile(hFile, pBuffer, MAX_BUFFER_LENGTH, &dwBytesRead, NULL) && dwBytesRead > 0)
		{
			rResult.append(pBuffer, dwBytesRead);
			dwBytesRead = 0;
		}

		delete []pBuffer;

		CloseHandle(hFile);
	}
	return (rResult.size() > 0);
}