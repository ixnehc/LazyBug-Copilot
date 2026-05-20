#ifndef __VssExecuteCommand_H__
#define __VssExecuteCommand_H__
#include "VssResultParser.h"
#include "FileSystem/ISscSystem.h"

typedef std::vector<String> SubDirList;

const char* const VSS_EXE		= "ss.exe";
const char* const VSS_DIR		= "dir";
const char* const VSS_CP		= "CP";
const char* const VSS_ADD		= "add";
const char* const VSS_CHECKIN	= "checkin";
const char* const VSS_CHECKOUT	= "checkout";
const char* const VSS_GET		= "get";
const char* const VSS_LOCATE	= "locate";
const char* const VSS_STATUS	= "status";
const char* const VSS_PROJECT	= "project";
const char* const VSS_WORKFOLD	= "workfold";
const char* const VSS_PROPERTIES= "properties";

const char* const VSS_OPTIONS_C	= "-C-";
const char* const VSS_OPTIONS_I	= "-I-";
const char* const VSS_OPTIONS_R	= "-R";
const char* const VSS_OPTIONS_O	= "-O";
const char* const VSS_OPTIONS_P	= "-P";
const char* const VSS_OPTIONS_N	= "-NL";
const char* const VSS_OPTIONS_F	= "-F-";
const char* const VSS_OPTIONS_B	= "-B-";
const char* const VSS_OPTIONS_G = "-GL";

const DWORD VSS_E_OK			= 0;
const DWORD VSS_E_WARNING		= 1;
const DWORD VSS_E_FAIL			= 100;

const char* const DOUBLEQUOTES	= "\"";

const char* const VSS_DELIM		= "/";
const char* const WIN32_DELIM	= "\\";

class CVssExecuteCommand : public ISscSystem
{
public:
	CVssExecuteCommand();
	~CVssExecuteCommand();

public:
	BOOL SetSscServer(const char *pathServer,const char *user,const char *pwd);

	BOOL ListProject(const char* pathProject, const char**& items, int& nCount);
	BOOL SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject);

	BOOL IsControlled(const char *pathFolderOrFile);	

	BOOL CheckIn(const char *pathFolderOrFile,BOOL *bModified);
	BOOL CheckOut(const char *pathFolderOrFile,BOOL *bModified);
	BOOL GetLatestVersion(const char *pathFolderOrFile,BOOL *bModified);

	BOOL GetState(const char *pathFile,SscState &state);

public:
	BOOL GetProjectItems(const char* pathProject, ProjectItemList& items);
	BOOL SetCurrentProject(const char* proj);
	BOOL IsProject(const char *pathFolderOrFile);
	BOOL AddProject(const char *pathFolderOrFile);

	void GetWorkingPath(const char* pathFolderOrFile, String& rWorkingPath);
	void GetProjectPath(const char* pathFolderOrFile, String& rProjectPath);

public:
	BOOL CreateOutputFile();
	void DeleteOutputFile();

	DWORD ExecVssCommand(const char* cmd, const char* options[]);

public:
	static const String& GetDefaultOutputFileName(String& outputFile);
	static BOOL GetSsExePath(String& ssExePath);

	static void ReplaceString(String& str, const String& src, const String& dest);
	static int SplitPath(const String& path, SubDirList& subdirs, const String& delim = WIN32_DELIM);
	static inline void GetPrevPath(const String& path, String& prev, const String& delim = WIN32_DELIM);

	static BOOL IsDirectory(const String& path);
	static BOOL CreateDirectories(const String& path, String& lastDir);

	static String SetCurDirectory(const String& dir);

private:
	String _server;
	String _user;
	String _pwd;
	String _ssExePath;
	String _outputFileName;
	String _workingFolder;
	String _currentProject;

	CVssResultParser _resultParser;
};
#endif