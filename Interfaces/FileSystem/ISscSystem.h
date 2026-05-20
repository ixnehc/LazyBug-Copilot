#pragma once

#include "ISscSystemDefines.h"

#include <string>

//source safe control system
class IFileSystem;
class ISscSystem
{
public:
	virtual void SetFS(IFileSystem * pFS) = 0;
	virtual BOOL Connect(const char *pathServer,const char *user,const char *pwd)=0;
	virtual void Disconnect()=0;

	virtual BOOL IsConnected() const =0;
	virtual int GetUserName(char* user, int len)=0;

	virtual BOOL ListProject(const char* pathProject, const char**& items, int& nCount, int piType = 0)=0;
	virtual BOOL SetWorkingFolder(const char *pathWorkingFolder, const char *pathProject)=0;

	//check whether the given path is under ssc control(existing on the ssc server)
	//path could be abs or relative (to the working folder)
	virtual BOOL IsControlled(const char *pathFolderOrFile)=0;

	//the operation functions
	//note all the operations are recursive
	//all the path could be full or relative(to the working folder),if "",the working folder 
	//will be used
	//and in bModfied will return whether any change occurs,if it's not NULL
	virtual BOOL CheckIn(const char *pathFolderOrFile,long flags)=0;
	virtual BOOL CheckOut(const char *pathFolderOrFile,long flags)=0;
	virtual BOOL GetLatestVersion(const char *pathFolderOrFile,long flags)=0;
	
	//path could be full or relative
	virtual BOOL GetState(const char *pathFile,SscState &state)=0;//get a file's scc state

	virtual BOOL Delete(const char *pathFolderOrFile)=0;
	virtual BOOL Rename(const char *pathFolderOrFile, const char *pszNewName)=0;

	virtual BOOL CreateSubProject(const char *pathProject)=0;

	virtual const char* GetWorkingFolder() const =0;
	virtual const char* GetCurrentProject() const =0;

	virtual BOOL Lock(const char * path) {return TRUE;}
	virtual BOOL UnLock(const char * path){return TRUE;}
};