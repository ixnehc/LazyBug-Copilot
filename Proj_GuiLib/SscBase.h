#pragma once

#include "GuiLib.h"
#include "FileSystem/IFileSystem.h"
#include "FileSystem/ISscSystem.h"
#include "gds/GObj.h"

const char* const VSS_ROOT		= "$/";

const char* const VSS_DELIM		= "/";
const char* const WIN32_DELIM	= "\\";

typedef std::vector<std::string> SubDirList;

struct SscConfig
{
	std::string database;
	std::string user;
	std::string passwd;
	std::string project;
	std::string workingFolder;

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(SscConfig,1);
		GELEM_STRING(database)
		GELEM_STRING(user)
		GELEM_STRING(passwd)
		GELEM_STRING(project)
		GELEM_STRING(workingFolder)

	END_GOBJ();    
};

class CCurrentUserRegistry;
class GuiLib_Api CSscSystemWrapper
{
public:
	CSscSystemWrapper(int iSerial);

public:
	BOOL Init(ISscSystem* ss, IFileSystem* fs);

	ISscSystem *GetSS()	{		return _pSS;	}
	IFileSystem *GetFS(){		return _pFS;	}

	void SetEnginePath(const char *path)	{		_pathEngine=path;	}

	void SetChooseParentWindow(HWND hWnd)
	{
		_hParentWnd = hWnd;
	}

public:

	BOOL HasConfig(CCurrentUserRegistry &reg);

	void PromptConnect(CCurrentUserRegistry &reg,BOOL bSilent);
	BOOL TryConnect(CCurrentUserRegistry&reg);

	BOOL Connect(SscConfig& cfg, BOOL bSilent);
	BOOL IsConnected() const;
	void Disconnect();

	int GetUserName(char* user, int len);

	BOOL ListProject(const char* pathProject, const char**& items, int& nCount);
	BOOL SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject);

	BOOL IsControlled(const char *pathFolderOrFile);

	BOOL CheckIn(const char *pathFolderOrFile,long flags = 0);
	BOOL CheckOut(const char *pathFolderOrFile,long flags = 0);
	BOOL GetLatestVersion(const char *pathFolderOrFile,long flags = 0);
	BOOL CheckOut(const char **pathFoldersOrFiles,DWORD count,long flags = 0);
	BOOL GetLatestVersion(const char **pathFoldersOrFiles,DWORD count,long flags = 0);

	BOOL GetState(const char *pathFile,SscState &state);

	BOOL Delete(const char *pathFolderOrFile);
	BOOL Rename(const char *pathFolderOrFile, const char *pszNewName);

	BOOL CreateSubProject(const char *pathProject);

	const char* GetWorkingFolder() const;
	const char* GetCurrentProject() const;

protected:
	BOOL _CheckOutFile(const char* pathFile, BOOL bSilent = FALSE);
	BOOL _GetFileLatestVersion(const char* pathFile, BOOL bSilent = FALSE);

	void _GetWorkingPath(const char* pathFolderOrFile, std::string& rWorkingPath);
	void _GetProjectPath(const char* pathFolderOrFile, std::string& rProjectPath);

	const char *_GetRegKey();

	static void _ReplaceString(std::string& str, const std::string& src, const std::string& dest);

	static inline BOOL _IsWritable(const char* pathFolderOrFile);

private:
	ISscSystem* _pSS;
	IFileSystem* _pFS;	
	HWND _hParentWnd;

	int _iSerial;

	BOOL _bLeave;
	BOOL _bApplyAllItems;
	BOOL _bRestoreDefaultSetting;

	std::string _pathEngine;
};
