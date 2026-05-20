#ifndef __SscExecuteCommand_H__
#define __SscExecuteCommand_H__
#include "SscResultParser.h"
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
const char* const VSS_WHOAMI	= "Whoami";
const char* const VSS_DELETE	= "delete";
const char* const VSS_RENAME	= "rename";

const char* const VSS_OPTIONS_C	= "-C-";
const char* const VSS_OPTIONS_I	= "-I-";
const char* const VSS_OPTIONS_R	= "-R";
const char* const VSS_OPTIONS_O	= "-O";
const char* const VSS_OPTIONS_P	= "-P";
const char* const VSS_OPTIONS_N	= "-NL";
const char* const VSS_OPTIONS_F	= "-F-";
const char* const VSS_OPTIONS_B	= "-B-";
const char* const VSS_OPTIONS_G = "-GL";
const char* const VSS_OPTIONS_S = "-S";

const char* const VSS_ROOT		= "$/";

const DWORD VSS_E_OK			= 0;
const DWORD VSS_E_WARNING		= 1;
const DWORD VSS_E_FAIL			= 100;
const DWORD VSS_CONNECT_TIMEOUT = 2000;

const char* const DOUBLEQUOTES	= "\"";

const char* const VSS_DELIM		= "/";
const char* const WIN32_DELIM	= "\\";

class CSscSystem : public ISscSystem
{
public:
	CSscSystem();
	~CSscSystem();

public:
	virtual BOOL Connect(const char *pathServer,const char *user,const char *pwd);
	virtual void Disconnect()=0;

	virtual BOOL IsConnected() const;
	virtual int GetUserName(char* user, int nCount);

	virtual BOOL ListProject(const char* pathProject, const char**& items, int& nCount);
	virtual BOOL SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject);

	virtual BOOL IsControlled(const char *pathFolderOrFile);	

	virtual BOOL CheckIn(const char *pathFolderOrFile,BOOL *bModified);
	virtual BOOL CheckOut(const char *pathFolderOrFile,BOOL *bModified);
	virtual BOOL GetLatestVersion(const char *pathFolderOrFile,BOOL *bModified);

	virtual BOOL GetState(const char *pathFile,SscState &state);

	virtual BOOL Delete(const char *pathFolderOrFile);
	virtual BOOL Rename(const char *pathFolderOrFile, const char *pszNewName);

	virtual BOOL CreateSubProject(const char *pathProject) { return FALSE; }

	virtual const char* GetWorkingFolder() const;
	virtual const char* GetCurrentProject() const;

public:
	BOOL TestVss();
	BOOL Whoami();
	BOOL GetProjectItems(const char* pathProject, ProjItemList& items);
	BOOL SetCurrentProject(const char* proj);
	BOOL IsProject(const char *pathFolderOrFile);
	BOOL AddProject(const char *pathFolderOrFile);

	void GetWorkingPath(const char* pathFolderOrFile, String& rWorkingPath);
	void GetProjectPath(const char* pathFolderOrFile, String& rProjectPath);

public:
	BOOL CreateOutputFile();
	void DeleteOutputFile();

	DWORD ExecSscCommand(const char* cmd, const char* options[], DWORD dwTimeout = INFINITE);

public:
	static const String& GetDefaultOutputFileName(String& outputFile);
	static BOOL GetSsExePath(String& ssExePath);

	static void ReplaceString(String& str, const String& src, const String& dest);
	static int SplitPath(const String& path, SubDirList& subdirs, const String& delim = WIN32_DELIM);
	static inline void GetPrevPath(const String& path, String& prev, const String& delim = WIN32_DELIM);
	static inline void GetFileName(const String& path, String& fileName, const String& delim = WIN32_DELIM);

	static inline BOOL IsDirectory(const String& path);
	static BOOL CreateDirectories(const String& path, String& lastDir);

	static BOOL RenameFile(const String& fileName, const String& newFileName);

	static String SetCurDirectory(const String& dir);

private:
	BOOL _bConnected;
	String _server;
	String _user;
	String _pwd;
	String _ssExePath;
	String _outputFileName;
	String _workingFolder;
	String _currentProject;

	CSscResultParser _resultParser;
};
#endif