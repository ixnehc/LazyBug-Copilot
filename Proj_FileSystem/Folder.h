#pragma once

#include "FileSystem/IFileSystem.h"

#include <vector>
#include <string>



class CFolder;
class CFileSystem;


enum FolderType
{
	FolderType_None,
	FolderType_Folder,
};

class CFolder:public IFolder
{
public:
	CFolder(void);

	virtual IFileSystem *GetFS();

	virtual IFile *SeekFile(const char *pathFile);
	virtual void CloseSeekFile();
	virtual IFile *GetSeekFile();//call this function after seeking a file
	virtual BOOL RemoveFile(const char *pathFile);

	//File enumeration
	virtual DWORD EnumBegin(const char *path,BOOL bRecursive);//return how many files and folders are enumerated
	virtual DWORD EnumGetFileCount();
	virtual const char *EnumGetFile(DWORD idxFile);
	virtual DWORD EnumGetFolderCount();
	virtual const char *EnumGetFolder(DWORD idxFolder);
	virtual BOOL EnumEnd();
	//

	virtual const char *GetPath();
	virtual void Close();

private:
	void _Init(IFileSystem *pFileSystem,HANDLE hNFS,const char *pathFolder,FileAccessMode mode);
	void	_Close(HANDLE &hNFS);
	IFile *_pSeekedFile;
	FileAccessMode _mode;
	FolderType _type;
	std::string _path;
	IFileSystem *_pFS;
	HANDLE _hNFS;//if a package folder,hNFS is the handle to it,otherwise it's NULL


	friend class CFileSystem;
};
