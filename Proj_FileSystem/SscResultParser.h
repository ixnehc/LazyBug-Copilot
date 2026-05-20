#ifndef __SscResultParser_H__
#define __SscResultParser_H__
#include <string>
#include <vector>
#include "FileSystem/ISscSystemDefines.h"

typedef std::string					String;
typedef std::vector<std::string>	ProjItemList;

const char* const TAG_ENDLINE	= "\r\n";
const char* const TAG_NOTEXIST	= "is not an existing filename or project";
const char* const TAG_PROJECT	= "Project:";
const char* const TAG_DIR		= ":";

class CSscResultParser
{
public:
	CSscResultParser();

public:
	void SetResultFileName(const String& fileName)
	{
		_lastResultFile = fileName;
	}

public:
	void ClearLastResult();

	BOOL Whoami(String& who);
	BOOL GetProjectItemsList(ProjItemList& items);
	BOOL IsProject(const String& path);

public:
	BOOL GetResultString(String& rResult);

private:
	String _lastResultFile;
};
#endif