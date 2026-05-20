#ifndef __VssResultParser_H__
#define __VssResultParser_H__
#include <string>
#include <vector>
#include "FileSystem/ISscSystemDefines.h"

typedef std::string					String;
typedef std::vector<std::string>	ProjectItemList;

const char* const TAG_ENDLINE	= "\r\n";
const char* const TAG_NOTEXIST	= "is not an existing filename or project";
const char* const TAG_PROJECT	= "Project:";
const char* const TAG_DIR		= ":";

class CVssResultParser
{
public:
	CVssResultParser();

public:
	void SetResultFileName(const String& fileName)
	{
		_lastResultFile = fileName;
	}

public:
	void ClearLastResult();

	BOOL GetProjectItemsList(ProjectItemList& items);
	BOOL IsProject(const String& path);
	BOOL GetState(SscState& state);

public:
	BOOL GetResultString(String& rResult);

private:
	String _lastResultFile;
};
#endif