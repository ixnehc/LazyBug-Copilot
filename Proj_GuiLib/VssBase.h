#pragma once

#include "GuiLib.h"
#include "FileSystem/IFileSystem.h"
#include "FileSystem/ISscSystem.h"
#include "gds/GObj.h"

struct VssConfig
{
	std::string database;
	std::string user;
	std::string passwd;
	std::string project;
	std::string workingFolder;

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(VssConfig,1);
		GELEM_STRING(database)
		GELEM_STRING(user)
		GELEM_STRING(passwd)
		GELEM_STRING(project)
		GELEM_STRING(workingFolder)

	END_GOBJ();    
};

class GuiLib_Api CSscSystemWrapper
{
public:
	CSscSystemWrapper();

public:
	BOOL Init(ISscSystem* ss, IFileSystem* fs);

public:
	BOOL Connect(VssConfig& cfg, BOOL bSilent);
	BOOL IsConnected() const;

	int GetUserName(char* user, int len);

	BOOL ListProject(const char* pathProject, const char**& items, int& nCount);
	BOOL SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject);

	BOOL IsControlled(const char *pathFolderOrFile);

	BOOL CheckIn(const char *pathFolderOrFile,BOOL *bModified);
	BOOL CheckOut(const char *pathFolderOrFile,BOOL *bModified);
	BOOL GetLatestVersion(const char *pathFolderOrFile,BOOL *bModified);

	BOOL GetState(const char *pathFile,SscState &state);

	BOOL Delete(const char *pathFolderOrFile);
	BOOL Rename(const char *pathFolderOrFile, const char *pszNewName);

	const char* GetWorkingFolder() const;
	const char* GetCurrentProject() const;

public:
	void SetChooseParentWindow(HWND hWnd)
	{
		_hParentWnd = hWnd;
	}

public:
	static inline BOOL IsWritable(const char* pathFolderOrFile);

	//static BOOL RemoveFile(const char* pszFileName);
	//static BOOL RenameFile(const char* pszFileName, const char* pszNewFileName);

private:
	ISscSystem* _ss;
	IFileSystem* _fs;	
	HWND _hParentWnd;
};
