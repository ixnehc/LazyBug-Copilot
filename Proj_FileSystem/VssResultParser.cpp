/********************************************************************
created:	2008/02/22
created:	22:2:2008   16:36
filename: 	f:\Project Test\Vss\Vss\VssResultParser.cpp
file path:	f:\Project Test\Vss\Vss
file base:	VssResultParser
file ext:	cpp
author:		szg

purpose:	根据VSS的输出，判断文件状态，（特征字符串的选取来自测试输出）.
*********************************************************************/
#include "stdh.h"
#include "VssResultParser.h"
#include "stringparser/stringparser.h"

CVssResultParser::CVssResultParser()
{
}

void CVssResultParser::ClearLastResult()
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

BOOL CVssResultParser::GetProjectItemsList(ProjectItemList& items)
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
BOOL CVssResultParser::IsProject(const String& path)
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
	return TRUE;
}

BOOL CVssResultParser::GetState(SscState& state)
{
	//const char* const TAG_NOTEXIST	 = "is not an existing filename or project";
	//const char* const TAG_NOCHECKEDOUT = "No checked out files found.";

	// Set defualt values
	state.bControlled = FALSE;
	state.bControlled = FALSE;

	String sResult;
	if (GetResultString(sResult))
	{
		/*
		//$/[IxEngine]/IxEngine/Common/FilePackage.txt is not an existing filename or project
		if (String::npos != sResult.find(NOTEXIST_TAG, 0))
		{
		state.bControlled = FALSE;
		}
		else if (String::npos != sResult.find(NO_CHECKEDOUT, 0))
		{
		//No checked out files found.
		state.bControlled = TRUE;
		//state.bCheckOut = FALSE;
		}
		else 
		*/
		if (static_cast<int>(sResult.length()) > 40)
		{
			//FilePackage.cpp     Szg           Exc  08-02-21 14:58  C:\Documents and Settings\l
			state.bControlled = TRUE;
			state.bCheckOut = TRUE;
			state.owner = sResult.substr(20, 13);	//20: FileName(19) User(13) Exc
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CVssResultParser::GetResultString(String& rResult)
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