/********************************************************************
	created:	2006/05/14
	created:	14:5:2006   10:52
	filename: 	d:\IxEngine\Proj_FileSystem\FileSystem.cpp
	author:		cxi
	
	purpose:	implement of IFileSystem
*********************************************************************/
#include "stdh.h"
#include "FileSystemExport.h"
#include ".\FileSystem.h"
 
#include <vector>
#include <string>

#include "File.h"
#include "Folder.h"

#include "stringparser\stringparser.h"
#include "HandlePool/HandlePool.h"

#include "Log/LastError.h"
#include "Log/LogFile.h"
#include "Log/LogDump.h"

#include "timer/profiler.h"

#include "shellapi.h"

#include "interface/interface.h"

#include "assert.h"



//These are copied from <fcntl.h>
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

#define O_CREAT 0x0100
//




//////////////////////////////////////////////////////////////////////////
//overide a HandlePool for package-access
class CPackageHandlePool:public CHandlePool
{
protected:
	virtual HANDLE Open(const char *abuffer);
	virtual void Close(HANDLE ahPackage);

private:
	std::vector<HANDLE> _hPackageList;
};

HANDLE CPackageHandlePool::Open(const char *abuffer)
{
	HANDLE hPackage = OpenFilePackage(abuffer, FileAccessMode_Modify);
	if (hPackage)
		_hPackageList.push_back(hPackage);
	return hPackage;
}
void CPackageHandlePool::Close(HANDLE ahPackage)
{
	std::vector<HANDLE>::iterator it = _hPackageList.begin();
	for (; it != _hPackageList.end(); it++)
	{
		if (*it == ahPackage)
		{
			CloseFilePackage(ahPackage);
			_hPackageList.erase(it);
			break;
		}
	}	
}



//////////////////////////////////////////////////////////////////////////
//CFileSystem
class CFileSystem:public IFileSystem
{
public:
	CFileSystem(void);
	~CFileSystem(void);

	virtual void SetSearchPath(const char *apath);//path should not include the slash at the tail
	virtual void PushSearchPath(const char *apathSearch);
	virtual void PopSearchPath();

	virtual DWORD GetFileSize(const char *apath);//如果失败,返回0xffffffff
	virtual DWORD GetFileSizeAbs(const char *apath);////如果失败,返回0xffffffff.path is an absolute path(will not search in search path)
	virtual IFile *OpenFile(const char *apath,FileAccessMode amodeOpen);
	virtual IFile *OpenFileAbs(const char *apath,FileAccessMode amodeOpen);//path is an absolute path
	virtual IFolder *OpenFolder(const char *apath,FileAccessMode amodeOpen);
	virtual IFolder *OpenFolderAbs(const char *apath,FileAccessMode amodeOpen);
	virtual void CloseFile(IFile *apFile);
	virtual void CloseFolder(IFolder *apFolder);

#ifndef MOBILE
	virtual BOOL ExistFolder(const char *apath);
	virtual BOOL ExistFolderAbs(const char *apath);//path is an absolute path(will not search in search path)
	virtual BOOL ExistFile(const char *apath);
	virtual BOOL ExistFileAbs(const char *apath);//path is an absolute path(will not search in search path)
	virtual FileAttr GetFileAttr(const char *apath);
	virtual FileAttr GetFileAttrAbs(const char *apath);//path is an absolute path(will not search in search path)
	virtual BOOL SetFileAttr(const char *apath,FileAttr aattr);
	virtual BOOL SetFileAttrAbs(const char *apath,FileAttr aattr);//path is an absolute path(will not search in search path)
	virtual BOOL Rename(const char *apath,const char *aname);
	virtual BOOL RenameAbs(const char *apath,const char *aname);
	virtual BOOL Move(const char *apath,const char *apathNew);
	virtual BOOL MoveAbs(const char *apath,const char *apathNew);
	virtual BOOL CopyAbs(const char *apath,const char *apathNew);
	virtual BOOL RemoveFile(const char *apath);
	virtual BOOL RemoveFileAbs(const char *apath);
	virtual BOOL RemoveFolder(const char *apath);
	virtual BOOL RemoveFolderAbs(const char *apath);//directory or package(or folder in package)
	virtual BOOL RemovePackage(const char *apath);
	virtual BOOL CleanUpPackage(const char *apath,CProgress *aprogress);

	virtual DWORD EnumBegin(const char *apath,BOOL abRecursive,EnumFileFilter afilter,void *aparam);//return how many files and folders are enumerated
	virtual DWORD EnumGetFileCount();
	virtual const char *EnumGetFile(DWORD aidxFile);
	virtual DWORD EnumGetFolderCount();
	virtual const char *EnumGetFolder(DWORD aidxFolder);
	virtual BOOL EnumEnd();


	virtual BOOL CheckResLeak();//Check whether there is any resource left not released in the system

	virtual ProfilerMgr *GetProfilerMgr();
	virtual void RegisterLogHandler(LogHandler &ahandler);

	FILETIME GetFileTimeAbs(const char *apath) override;
	BOOL SetFileTimeAbs(const char* apath, FILETIME atime) override;

#endif


protected:
	BOOL _CheckInputPathValidity(char *apath);
	char *_SearchAbsPath(const char *apath);
	DWORD _SplitAbsPath(const std::string &apathAbs,std::string &apathHeader,std::string &apathTail,std::vector<std::string>&avecSubPath);
	BOOL _BuildDirectory(const char *apathHeader,std::vector<std::string>&avecSubPath,BOOL abIgnoreTail);
	void _CollectFiles(const char *apathFull,const char *apathTail,BOOL abFolder,BOOL abRecursive,std::vector<std::string>&avecFiles,std::vector<std::string>&avecFolders);
	BOOL _DeleteFile(const char *apath,BOOL abForce);
	BOOL _Rename(const char *apath,const char *apath2,BOOL abNameOrPath);

	std::vector<std::string> _aSearchPaths;//The pathes to search file in.

	FileSystemResult _lastResult;

	CPackageHandlePool _handlepool;

	char _buf[1024];

	EnumFileFilter _filter;//这个指针只用于EnumBegin(..)中
	void *_enumparam;
	std::vector<std::string>_vecEnumFiles;
	std::vector<std::string>_vecEnumFolders;
};

#pragma message ("need to remove all the XXXXAbs() function,and auto judge whether a full path is provided")


CFileSystem g_fs01,g_fs02,g_fs03;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(fs01,IFileSystem,"FileSystem01",g_fs01);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(fs02,IFileSystem,"FileSystem02",g_fs02);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(fs03,IFileSystem,"FileSystem03",g_fs03);


IFileSystem* GetFileSystem()
{
	return &g_fs01;
}


CFileSystem::CFileSystem(void)
{
	SetSearchPath("");
	_lastResult=Result_Success;
	_filter=NULL;
	_enumparam=NULL;
}

CFileSystem::~CFileSystem(void)
{
}

void CFileSystem::SetSearchPath(const char *apath)//path should not include the slash at the tail
{
	_aSearchPaths.clear();
	_aSearchPaths.push_back(std::string(apath));
}

void CFileSystem::PushSearchPath(const char *apathSearch)
{
	_aSearchPaths.insert(_aSearchPaths.begin(),std::string(apathSearch));
}
void CFileSystem::PopSearchPath()
{
	if (_aSearchPaths.size()>0)
		_aSearchPaths.erase(_aSearchPaths.begin());
}

void CFileSystem::CloseFile(IFile *apFile)
{
	if (apFile)
	{
		HANDLE hNFS = NULL;
		((CFile*)apFile)->_Close(hNFS);
		delete ((CFile*)apFile);

		if (hNFS)
			_handlepool.CloseHandle(hNFS);
	}
}
void CFileSystem::CloseFolder(IFolder *apFolder)
{
	if (apFolder)
	{
		HANDLE hNFS = NULL;
		((CFolder*)apFolder)->_Close(hNFS);
		delete ((CFolder*)apFolder);
		if (hNFS)
			_handlepool.CloseHandle(hNFS);
	}
}

BOOL CFileSystem::_CheckInputPathValidity(char *apath)
{
	if (apath==NULL)
		return FALSE;
	if (*apath==0)
		return FALSE;

	if (*apath==PATH_SLASH_C)
		return FALSE;

	char *p;
	p=apath;
	while(*p)
		p++;

	p--;
	if (*p==PATH_SLASH_C)
		return FALSE;

	return TRUE;
}

char *CFileSystem::_SearchAbsPath(const char *apath)
{
	if (!_CheckInputPathValidity((char *)apath))
	{
		SetLastErrCode(Result_InvalidPath);
		return NULL;
	}

	if (_aSearchPaths.size()<=0)
	{
		SetLastErrCode(Result_NoSearchPath);
		return NULL;
	}

	int nLen=(int)_aSearchPaths[0].length();
	if (nLen>0)
	{
		memcpy(_buf,_aSearchPaths[0].c_str(),nLen);
		_buf[nLen]=PATH_SLASH_C;
		nLen++;
	}

	char *p=(char*)apath;

	while(*p)
	{
		if (nLen>=sizeof(_buf)-1)
		{
			SetLastErrCode(Result_TooLongPath);
			return NULL;
		}
		_buf[nLen]=*p;
		nLen++;
		p++;
	}

	_buf[nLen]=0;//terminator

	return _buf;
}

DWORD CFileSystem::_SplitAbsPath(const std::string &apathAbs,std::string &apathHeader,std::string &apathTail,std::vector<std::string>&avecSubPath)
{
	char buffer[1024];
	if (apathAbs.length()>sizeof(buffer)-1)
		return INVALID_FILE_ATTRIBUTES;
	strcpy(buffer,apathAbs.c_str());

	DWORD attrFile;
	while(1)
	{
		//if (nfs_exists(buffer))
		if (FilePackageExists(buffer))
			attrFile=0;
		else
			attrFile=GetFileAttributes(buffer);

		char temp[MAX_PATH];

		if (attrFile!=INVALID_FILE_ATTRIBUTES)
			break;

		if (CutTailSubPath(buffer,temp)<=0)
			break;

		avecSubPath.insert(avecSubPath.begin(),std::string(temp));
	}

	apathHeader=buffer;

	LinkStringBy(PATH_SLASH,apathTail,&avecSubPath);

	return attrFile;

}

BOOL CFileSystem::_BuildDirectory(const char *apathHeader,std::vector<std::string>&avecSubPath,BOOL abIgnoreTail)
{
	std::string s;
	s=apathHeader;
	int i;
	for (i=0;i<(int)avecSubPath.size();i++)
	{
		if (abIgnoreTail&&(i==avecSubPath.size()-1))
			continue;
		s+=PATH_SLASH;
		s+=avecSubPath[i];

		if (FALSE==CreateDirectoryA(s.c_str(),NULL))
		{
			SetLastErrCode(Result_CannotCreateDirectory);
			return FALSE;
		}
	}

	return TRUE;
}


IFolder *CFileSystem::OpenFolder(const char *apath,FileAccessMode amodeOpen)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return NULL;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return NULL;
	}

	CFolder *pFolder;
	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		if ((amodeOpen==FileAccessMode_Read)||(amodeOpen==FileAccessMode_Modify))
		{
			if (aSubPaths.size()>0)//Sub path could not be resolved,
			{
				SetLastErrCode(Result_InvalidPath);//for reading,we should fail here
				return NULL;
			}

			pFolder=new CFolder;
			pFolder->_Init(this,NULL,pathHeader.c_str(),amodeOpen);
			return pFolder;
		}

		if ((amodeOpen==FileAccessMode_WritePackage)||(amodeOpen==FileAccessMode_Write))
		{
			if (aSubPaths.size()<=0)
			{
				if (amodeOpen==FileAccessMode_WritePackage)//the required package is already here as a directory
					amodeOpen=FileAccessMode_Write;//We take it as directory
			}
			
			HANDLE hNFS=NULL;
			if (amodeOpen==FileAccessMode_Write)//Create the sub folders
			{
				if (FALSE==_BuildDirectory(pathHeader.c_str(),aSubPaths,FALSE))
					return NULL;
			}
			else
			{
				//FileAccessMode_WritePackage
				if (FALSE==_BuildDirectory(pathHeader.c_str(),aSubPaths,TRUE))//Do not create the final folder
					return NULL;
				
				//Now try to create a package here
				hNFS=_handlepool.OpenHandle(pathAbs.c_str());
				if (!hNFS)
				{
					SetLastErrCode(Result_CannotCreatePackage);//Fail
					return NULL;
				} 
			}

			pFolder=new CFolder;
			pFolder->_Init(this,hNFS,pathAbs.c_str(),FileAccessMode_Write);//change to Write after creating
			return pFolder;
		}
	}
	else
	{
		//it's a file
		if ((amodeOpen==FileAccessMode_Read)||(amodeOpen==FileAccessMode_Modify))
		{
			//check whether the package file exists
			//if (!nfs_exists((char*)pathHeader.c_str()))
			//{
			//	SetLastErrCode(Result_CannotOpenPackage);
			//	return NULL;
			//}
			if (!FilePackageExists(pathHeader.c_str()))
			{
				SetLastErrCode(Result_CannotOpenPackage);
				return NULL;
			}
		}

		HANDLE hNFS;
		hNFS=_handlepool.OpenHandle(pathHeader.c_str());
		if (!hNFS)
		{
			SetLastErrCode(Result_CannotOpenPackage);
			return NULL;
		}
		//_handlepool.CloseHandle(hNFS);

		if(amodeOpen==FileAccessMode_WritePackage)
			amodeOpen=FileAccessMode_Write;

		pFolder=new CFolder;
		pFolder->_Init(this,hNFS,pathAbs.c_str(),amodeOpen);
		return pFolder;
	}

	return NULL;
}

IFolder *CFileSystem::OpenFolderAbs(const char *apath,FileAccessMode amodeOpen)
{
	IFolder *ret;
	PushSearchPath("");
	ret=OpenFolder(apath,amodeOpen);
	PopSearchPath();
	return ret;
}



//only package folder(or folder in package)
BOOL CFileSystem::RemovePackage(const char *apath)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return FALSE;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}


	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		//it's a directory,no package found here,fail
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	//It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	//{
	//	SetLastErrCode(Result_InvalidPath);//cannot remove file as a folder
	//	return FALSE;
	//}
	if (!FilePackageExists(pathHeader.c_str()))
	{
		SetLastErrCode(Result_InvalidPath);//cannot remove file as a folder
		return FALSE;
	}
	else
	{
		//it's a package
		if (aSubPaths.size()<=0)
		{
			if (_handlepool.QueryHandle(pathHeader.c_str()))
			{
				SetLastErrCode(Result_DirectoryInUse);
				return FALSE;
			}

			//Not in use,delete the nfs file system
			if (TRUE)
			{
				//nfs_Handle *pFS;
				//pFS=nfs_start((char*)pathHeader.c_str(),FS_RW);
				//if (pFS)
				//	nfs_end(pFS,1);

				//if (nfs_exists((char*)pathHeader.c_str()))//additional check
				//{
				//	SetLastErrCode(Result_DirectoryInUse);
				//	return FALSE;
				//}
				if (!_DeleteFile(pathHeader.c_str(),TRUE))
				{
					SetLastErrCode(Result_DirectoryInUse);
					return FALSE;
				}
			}

			return TRUE;
		}
		else
		{
			HANDLE hNFS;
			hNFS=_handlepool.OpenHandle(pathHeader.c_str());

			if (hNFS==NULL)
			{
				SetLastErrCode(Result_CannotOpenPackage);
				return FALSE;
			}

			//std::string pattern;
			//nfs_glob_t tt;
			//pattern=pathTail+"*";
			//nfs_glob((nfs_Handle*)hNFS,pattern.c_str(),GLOB_NOSORT,NULL,&tt);

			//int i;
			//for (i = tt.gl_offs; i < tt.gl_pathc + tt.gl_offs; i++) 
			//	nfs_file_unlink((nfs_Handle*)hNFS,tt.gl_pathv[i]);

			//nfs_glob_free((nfs_Handle*)hNFS, &tt);

			// Delete all searched package files
			FilePathList pathList;
			SearchPackageFiles(hNFS, pathTail.c_str(), pathList);

			FilePathList::const_iterator it = pathList.begin();
			for (; it != pathList.end(); it++)
			{
				RemovePackageFile(hNFS, (*it).c_str());
			}

			_handlepool.CloseHandle(hNFS);

			return TRUE;
		}

	}
	return FALSE;
}

BOOL CFileSystem::CleanUpPackage(const char *apath,CProgress *aprogress)
{
	return CleanupFilePackage(apath, aprogress);
}

BOOL CFileSystem::RemoveFolder(const char *apath)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return FALSE;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}


	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		if (aSubPaths.size()>0)//Sub path could not be resolved,
		{
			SetLastErrCode(Result_InvalidPath);
			return FALSE;
		}
		//Now remove the directory
		TCHAR szzTo[] = { '\0', '\0' };
		char buffer[1024];
		strcpy(buffer,pathHeader.c_str());
		buffer[pathHeader.length()+1]=0;//an additional terminator

		SHFILEOPSTRUCT shFileOp;
		memset(&shFileOp,0,sizeof(SHFILEOPSTRUCT));
		shFileOp.hwnd                  = NULL;
		shFileOp.wFunc                 = FO_DELETE;
		shFileOp.pFrom                 = buffer;
		shFileOp.pTo                   = szzTo;
		shFileOp.fFlags                = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
		if (::SHFileOperation(&shFileOp) != 0 )
		{
			SetLastErrCode(Result_DirectoryInUse);
			return FALSE;
		}

		return TRUE;
	}

	////It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	//{
	//	SetLastErrCode(Result_InvalidPath);//cannot remove file as a folder
	//	return FALSE;
	//}
	if (!FilePackageExists(pathHeader.c_str()))
	{
		SetLastErrCode(Result_InvalidPath);//cannot remove file as a folder
		return FALSE;
	}
	else
	{
		//it's a package
		if (aSubPaths.size()<=0)
		{
			if (_handlepool.QueryHandle(pathHeader.c_str()))
			{
				SetLastErrCode(Result_DirectoryInUse);
				return FALSE;
			}

			//Not in use,delete the nfs file system
			if (TRUE)
			{
				//nfs_Handle *pFS;
				//pFS=nfs_start((char*)pathHeader.c_str(),FS_RW);
				//if (pFS)
				//	nfs_end(pFS,1);

				//if (nfs_exists((char*)pathHeader.c_str()))//additional check
				//{
				//	SetLastErrCode(Result_DirectoryInUse);
				//	return FALSE;
				//}
				if (!_DeleteFile(pathHeader.c_str(),TRUE))
				{
					SetLastErrCode(Result_DirectoryInUse);
					return FALSE;
				}
			}

			return TRUE;
		}
		else
		{
			HANDLE hNFS;
			hNFS=_handlepool.OpenHandle(pathHeader.c_str());

			if (hNFS==NULL)
			{
				SetLastErrCode(Result_CannotOpenPackage);
				return FALSE;
			}

			//std::string pattern;
			//nfs_glob_t tt;
			//pattern=pathTail+"*";
			//nfs_glob((nfs_Handle*)hNFS,pattern.c_str(),GLOB_NOSORT,NULL,&tt);

			//int i;
			//for (i = tt.gl_offs; i < tt.gl_pathc + tt.gl_offs; i++) 
			//	nfs_file_unlink((nfs_Handle*)hNFS,tt.gl_pathv[i]);

			//nfs_glob_free((nfs_Handle*)hNFS, &tt);

			// Delete all searched package files
			FilePathList pathList;
			SearchPackageFiles(hNFS, pathTail.c_str(), pathList);

			FilePathList::const_iterator it = pathList.begin();
			for (; it != pathList.end(); it++)
			{
				RemovePackageFile(hNFS, (*it).c_str());
			}

			_handlepool.CloseHandle(hNFS);

			return TRUE;
		}

	}
	return FALSE;
}

BOOL CFileSystem::RemoveFolderAbs(const char *apath)
{
	BOOL bRet;
	PushSearchPath("");
	bRet=RemoveFolder(apath);
	PopSearchPath();
	return bRet;
}


BOOL CFileSystem::_DeleteFile(const char *apath,BOOL abForce)
{
	if (abForce)
	{
		DWORD attr=GetFileAttributes(apath);
		if (attr&FILE_ATTRIBUTE_READONLY)
			attr&=~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes(apath,attr);
	}
	return DeleteFile(apath);
}


BOOL CFileSystem::RemoveFile(const char *apath)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return NULL;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	//It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	if (!FilePackageExists(pathHeader.c_str()))
	{
		//A normal file
		if (aSubPaths.size()>0)//the sub path could not be resolved
		{
			SetLastErrCode(Result_InvalidPath);
			return FALSE;
		}


		if (FALSE==_DeleteFile(pathHeader.c_str(),TRUE))
			return FALSE;

		return TRUE;
	}
	else
	{
		//it's a package
		if (aSubPaths.size()<=0)//a package could not be a file(to remove)
		{
			SetLastErrCode(Result_InvalidPath);
			return FALSE;
		}

		HANDLE hNFS;
		hNFS=_handlepool.OpenHandle(pathHeader.c_str());

		if (!hNFS)
		{
			SetLastErrCode(Result_CannotOpenPackage);
			return FALSE;
		}

		//if (-1==nfs_file_unlink((nfs_Handle*)hNFS,pathTail.c_str()))
		if (!RemovePackageFile(hNFS, pathTail.c_str()))
		{
			_handlepool.CloseHandle(hNFS);
			return FALSE;
		}
		_handlepool.CloseHandle(hNFS);
		return TRUE;
	}

	return FALSE;
}

BOOL CFileSystem::RemoveFileAbs(const char *apath)
{
	BOOL bRet;
	PushSearchPath("");
	bRet=RemoveFile(apath);
	PopSearchPath();
	return bRet;
}


BOOL CFileSystem::ExistFolder(const char *apath)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return FALSE;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		if (aSubPaths.size()>0)
			return FALSE;
		return TRUE;
	}

	//It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	if (!FilePackageExists(pathHeader.c_str()))
	{
		//Not a package
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}
	else
	{//it's a package
		if (aSubPaths.size()<=0)//no sub path to find,the package is taken as a folder
			return TRUE;

		assert(FALSE);
		//By now we could not easily decide whether a sub path within a package is a folder or not,
		//just return FALSE.
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	SetLastErrCode(Result_InvalidPath);
	return FALSE;
}


FileAttr CFileSystem::GetFileAttr(const char *apath)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return File_Miss;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return File_Miss;
	}

	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		SetLastErrCode(Result_InvalidPath);
		return File_Miss;
	}

	//It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	if (!FilePackageExists(pathHeader.c_str()))
	{
		//Not a package
		if (aSubPaths.size()<=0)
		{
			DWORD dwAttr;
			dwAttr=::GetFileAttributes(pathAbs.c_str());
			if (dwAttr==INVALID_FILE_ATTRIBUTES)
			{
				SetLastErrCode(Result_InvalidPath);
				return File_Miss;
			}
			if (dwAttr&FILE_ATTRIBUTE_READONLY)
				return File_ReadOnly;

			return File_Default;
		}
	}
	else
	{
		//it's a package
		if (aSubPaths.size()>0)//a package could not be a file
		{
			HANDLE hNFS;
			hNFS=_handlepool.OpenHandle(pathHeader.c_str());
			if (hNFS)
			{
				//if (nfs_file_exists((nfs_Handle*)hNFS,(char*)pathTail.c_str()))
				if (PackageFileExists(hNFS, pathTail.c_str()))
				{
					_handlepool.CloseHandle(hNFS);
					return (FileAttr)(File_Default|File_ReadOnly);//file in package is always readonly
				}
				_handlepool.CloseHandle(hNFS);
			}
		}
		SetLastErrCode(Result_InvalidPath);
		return File_Miss;
	}

	return File_Miss;
}

BOOL CFileSystem::SetFileAttr(const char *apath,FileAttr aattr)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if ((aattr!=File_Default)&&(aattr!=File_ReadOnly))
		return FALSE;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return FALSE;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	//It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	if (!FilePackageExists(pathHeader.c_str()))
	{
		//Not a package
		if (aSubPaths.size()<=0)
		{
			DWORD dwAttr;
			dwAttr=::GetFileAttributes(pathAbs.c_str());

			if (dwAttr==INVALID_FILE_ATTRIBUTES)
				return FALSE;
			if(aattr==File_ReadOnly)
				dwAttr|=FILE_ATTRIBUTE_READONLY;
			else
				dwAttr&=(~FILE_ATTRIBUTE_READONLY);
			return ::SetFileAttributes(pathAbs.c_str(),dwAttr);
		}
	}

	return FALSE;
}

DWORD CFileSystem::GetFileSize(const char *apath)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return 0xffffffff;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return 0xffffffff;
	}

	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		SetLastErrCode(Result_InvalidPath);
		return 0xffffffff;
	}

	//It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	if (!FilePackageExists(pathHeader.c_str()))
	{
		//Not a package
		if (aSubPaths.size()<=0)
		{
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;

			hFind=FindFirstFile(pathAbs.c_str(),&FindFileData);
			if (hFind==INVALID_HANDLE_VALUE)
				return 0xffffffff;
			FindClose(hFind);

			return FindFileData.nFileSizeLow;
		}
	}
	else
	{
		//it's a package
		if (aSubPaths.size()>0)//a package could not be a file
		{
			HANDLE hNFS;
			hNFS=_handlepool.OpenHandle(pathHeader.c_str());
			if (hNFS)
			{
				DWORD sz;
				if (!GetPackageFileSize((HPACKAGE)hNFS, pathTail.c_str(),sz))
					sz=0xffffffff;
				_handlepool.CloseHandle(hNFS);
				return sz;
			}
		}
		SetLastErrCode(Result_InvalidPath);
		return 0xffffffff;
	}

	return 0xffffffff;
}



//path is an absolute path(will not search in search path)
BOOL CFileSystem::ExistFolderAbs(const char *apath)
{
	BOOL bRet;
	PushSearchPath("");
	bRet=ExistFolder(apath);
	PopSearchPath();
	return bRet;
}

//path is an absolute path(will not search in search path)
FileAttr CFileSystem::GetFileAttrAbs(const char *apath)
{
	FileAttr ret;
	PushSearchPath("");
	ret=GetFileAttr(apath);
	PopSearchPath();
	return ret;
}

//path is an absolute path(will not search in search path)
DWORD CFileSystem::GetFileSizeAbs(const char *apath)
{
	DWORD ret;
	PushSearchPath("");
	ret=GetFileSize(apath);
	PopSearchPath();
	return ret;
}


BOOL CFileSystem::SetFileAttrAbs(const char *apath,FileAttr aattr)//path is an absolute path(will not search in search path)
{
	BOOL ret;
	PushSearchPath("");
	ret=SetFileAttr(apath,aattr);
	PopSearchPath();
	return ret;
}


BOOL CFileSystem::ExistFile(const char *apath)
{
	if (GetFileAttr(apath)&File_Miss)
		return FALSE;
	return TRUE;
}

BOOL CFileSystem::ExistFileAbs(const char *apath)
{
	if (GetFileAttrAbs(apath)&File_Miss)
		return FALSE;
	return TRUE;
}



//path is an absolute path
IFile *CFileSystem::OpenFileAbs(const char *apath,FileAccessMode amodeOpen)
{
	IFile *ret;
	PushSearchPath("");
	ret=OpenFile(apath,amodeOpen);
	PopSearchPath();
	return ret;
}

IFile *CFileSystem::OpenFile(const char *apath,FileAccessMode amodeOpen)
{
	if ((amodeOpen!=FileAccessMode_Read)&&
		(amodeOpen!=FileAccessMode_Write)&&
		(amodeOpen!=FileAccessMode_Modify)	)
	{
		SetLastErrCode(Result_InvalidOpenMode);
		return NULL;
	}

	std::string pathAbs,pathHeader,pathTail,pathAbsOrg;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return NULL;
		pathAbsOrg=pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return NULL;
	}

	CFile *pFile;

	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		if ((amodeOpen==FileAccessMode_Read)||(amodeOpen==FileAccessMode_Modify))
		{
			SetLastErrCode(Result_InvalidPath);
			return NULL;
		}

		//Write...
		if (aSubPaths.size()<=0)//Cannot write to a directory
		{
			SetLastErrCode(Result_InvalidPath);
			return NULL;
		}

		if (FALSE==_BuildDirectory(pathHeader.c_str(),aSubPaths,TRUE))
			return NULL;

		CFileStream *stream=new CFileStream;
		stream->Open(pathAbs.c_str(),CFileStream::OM_WRITE);

//		HANDLE hFile;
//		hFile=::CreateFile(pathAbs.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
//		hFile=::OpenFile(pathAbs.c_str(),&os,OF_WRITE|OF_CREATE);

		if (!stream->IsOpen())
		{
			delete stream;
			SetLastErrCode(Result_InvalidPath);
			return NULL;
		}

		pFile=new CFile;
		pFile->_InitFile(stream,FileAccessMode_Write,pathAbsOrg.c_str(),this);
		return pFile;
	}

	//It's a file,check whether it's a package
	//if (!nfs_exists((char*)pathHeader.c_str()))
	if (!FilePackageExists(pathHeader.c_str()))
	{//Not a package
		if (aSubPaths.size()>0)//the sub path could not be resolved
		{
			SetLastErrCode(Result_InvalidPath);
			return NULL;
		}

		UINT access,flag,share;
		CFileStream::OpenMode mode;

		share=0;
		switch (amodeOpen)
		{
			case FileAccessMode_Read:		mode=CFileStream::OM_READ;access=GENERIC_READ;	share=FILE_SHARE_READ|FILE_SHARE_WRITE;	flag=OPEN_EXISTING;	break;
			case FileAccessMode_Write:		mode=CFileStream::OM_WRITE;access=GENERIC_WRITE;	share=FILE_SHARE_READ;flag=CREATE_ALWAYS;	break;
			case FileAccessMode_Modify:	mode=CFileStream::OM_READ_AND_WRITE;access=GENERIC_READ|GENERIC_WRITE;	share=FILE_SHARE_READ;flag=OPEN_EXISTING;		break;
			default: assert(FALSE);		break;
		}
//		ProfilerStart_Recent(OpenFile);
//		hFile=::CreateFile(pathAbs.c_str(),access,share,NULL,flag,FILE_ATTRIBUTE_NORMAL,NULL);
//		hFile=::OpenFile(pathAbs.c_str(),&os,style);
//		ProfilerEnd();

		CFileStream *stream=new CFileStream;
		stream->Open(pathAbs.c_str(),mode);

		if (!stream->IsOpen())
		{
			delete stream;
			SetLastErrCode(Result_InvalidPath);
			return NULL;
		}

		pFile=new CFile;
		pFile->_InitFile(stream,amodeOpen,pathAbsOrg.c_str(),this);
		return pFile;
	}
	else
	{
		//it's a package
		if (aSubPaths.size()<=0)//a package could not be a file
		{
			SetLastErrCode(Result_InvalidPath);
			return NULL;
		}

		HANDLE hNFS;
		HANDLE hPackageFile;
		hNFS=_handlepool.OpenHandle(pathHeader.c_str());
		if (!hNFS)
		{
			SetLastErrCode(Result_CannotOpenPackage);
			return NULL;
		}
		if (amodeOpen==FileAccessMode_Read)
			//iFileDesc=nfs_file_open((nfs_Handle*)hNFS,pathTail.c_str(),O_RDONLY);
			hPackageFile = OpenPackageFile(hNFS, pathTail.c_str());
		else
		{
			if (amodeOpen==FileAccessMode_Modify)
				//iFileDesc=nfs_file_open((nfs_Handle*)hNFS,pathTail.c_str(),O_RDWR);
				hPackageFile = OpenPackageFile(hNFS, pathTail.c_str());
			else
			{
				//if (!nfs_file_exists((nfs_Handle*)hNFS,(char*)pathTail.c_str()))
				if (!PackageFileExists(hNFS, pathTail.c_str()))
					//iFileDesc=nfs_file_create((nfs_Handle*)hNFS,(char*)pathTail.c_str(),0);
					hPackageFile = CreatePackageFile(hNFS, pathTail.c_str());
				else
					//iFileDesc=nfs_file_open((nfs_Handle*)hNFS,pathTail.c_str(),O_WRONLY|O_CREAT);
					hPackageFile = OpenPackageFile(hNFS, pathTail.c_str());
			}
		}
		if (hPackageFile == NULL)
		{
			_handlepool.CloseHandle(hNFS);
			SetLastErrCode(Result_InvalidPath);
			return NULL;
		}

		pFile=new CFile;
		pFile->_InitPackageFile(hNFS,hPackageFile,amodeOpen,pathAbsOrg.c_str(),this);

		return pFile;
	}

	return NULL;
}


void CFileSystem::_CollectFiles(const char *apathFull,const char *apathTail,BOOL abFolder,BOOL abRecursive,std::vector<std::string>&avecFiles,std::vector<std::string>&avecFolders)
{
	if (abFolder)
	{
		std::string pathToFind,pathToCollect;
		pathToFind=apathFull;
		pathToFind+="\\*.*";
		pathToCollect=apathTail;
		if (!(pathToCollect==""))
			pathToCollect+=PATH_SLASH;
		HANDLE hFindFile;
		WIN32_FIND_DATA fd;
		hFindFile=FindFirstFile(pathToFind.c_str(),&fd);
		if (hFindFile)
		{
			do
			{
				if ((fd.cFileName[0] == '.')&&(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
				{
					if (fd.cFileName[1] == '\0' || (fd.cFileName[1] == '.' && fd.cFileName[2] == '\0'))
						continue;//ignore the dots
				}
				if (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
					continue;//忽略隐藏文件
				std::string pathSub,pathSubTail;					
				pathSub=apathFull;
				pathSub=pathSub+PATH_SLASH+fd.cFileName;
				pathSubTail=pathToCollect+fd.cFileName;
				if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					BOOL bOk=_filter?_filter(pathSubTail.c_str(),TRUE,_enumparam):TRUE;
					if (bOk)
						avecFolders.push_back(pathSubTail);

					if (!abRecursive)
						continue;					
					_CollectFiles(pathSub.c_str(),pathSubTail.c_str(),TRUE,TRUE,avecFiles,avecFolders);
					continue;
				}

				//It's a file,
				//if (nfs_exists((char*)pathSub.c_str()))//a package
				if (FilePackageExists(pathSub.c_str()))
				{
					if (!abRecursive)
						continue;
					_CollectFiles(pathSub.c_str(),pathSubTail.c_str(),FALSE,TRUE,avecFiles,avecFolders);
					continue;
				}

				BOOL bOk=_filter?_filter(pathSubTail.c_str(),FALSE,_enumparam):TRUE;
				if (bOk)
					avecFiles.push_back(pathSubTail);
			}while(FindNextFile(hFindFile,&fd));
			::FindClose(hFindFile);
		}
	}
	else
	{
		//It's a package file
		HANDLE hNFS;
		hNFS=_handlepool.OpenHandle(apathFull);
		if (hNFS)
		{
			//nfs_glob_t tt;
			//nfs_glob((nfs_Handle*)hNFS,"*",GLOB_NOSORT,NULL,&tt);

			//int i;
			//for (i = tt.gl_offs; i < tt.gl_pathc + tt.gl_offs; i++) 
			//{
			//	ReplaceCharInString(PATH_SLASH2_C,PATH_SLASH_C,tt.gl_pathv[i]);
			//	std::string s;
			//	s=pathTail;
			//	if (s=="")
			//		s+=PATH_SLASH;
			//	s+=tt.gl_pathv[i];
			//	vecFiles.push_back(s);
			//}

			//nfs_glob_free((nfs_Handle*)hNFS, &tt);
			const char* const ALLFILES = "*";			
			SearchPackageFiles(hNFS, ALLFILES, avecFiles);

			_handlepool.CloseHandle(hNFS);
		}
	}
}

   
DWORD CFileSystem::EnumBegin(const char *apath,BOOL abRecursive,EnumFileFilter afilter,void *aparam)
{
	_vecEnumFiles.clear();
	_vecEnumFolders.clear();
	_filter=afilter;
	_enumparam=aparam;
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (apath&&((*apath)==0))//""
		pathAbs=_aSearchPaths[0];
	else
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return -1;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return -1;
	}

	if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
	{
		if (aSubPaths.size()>0)
		{
			SetLastErrCode(Result_InvalidPath);
			return -1;
		}
		_CollectFiles(pathHeader.c_str(),"",TRUE,abRecursive,_vecEnumFiles,_vecEnumFolders);
		return (DWORD)(_vecEnumFiles.size()+_vecEnumFolders.size());
	}

	//if (!nfs_exists((char*)pathHeader.c_str()))
	if (!FilePackageExists(pathHeader.c_str()))
	{
		SetLastErrCode(Result_InvalidPath);
		return -1;
	}

	HANDLE hNFS;
	hNFS=_handlepool.OpenHandle(pathHeader.c_str());
	if (!hNFS)
	{
		SetLastErrCode(Result_InvalidPath);
		return -1;
	}

	//nfs_glob_t tt;
	//std::string pattern;
	//int patternlen;
	//pattern=pathTail;
	//if (!(pattern==""))
	//	pattern+=PATH_SLASH2;
	//patternlen=(int)pattern.length();
	//pattern+="*";
	//nfs_glob((nfs_Handle*)hNFS,pattern.c_str(),GLOB_NOSORT,NULL,&tt);

	//int i;
	//for (i = tt.gl_offs; i < tt.gl_pathc + tt.gl_offs; i++) 
	//{
	//	std::string s;
	//	s=tt.gl_pathv[i]+patternlen;
	//	if ((!bRecursive)&&(s.find(PATH_SLASH2_C)!=-1))
	//		continue;

	//	ReplaceCharInString(PATH_SLASH2_C,PATH_SLASH_C,(char*)s.c_str());
	//	_vecEnumFiles.push_back(s);
	//}

	//nfs_glob_free((nfs_Handle*)hNFS, &tt);

	if (pathTail.empty())
		pathTail = "*";

	SearchPackageFiles(hNFS, pathTail.c_str(), _vecEnumFiles);

	_handlepool.CloseHandle(hNFS);

	return (DWORD)(_vecEnumFiles.size()+_vecEnumFolders.size());
}

BOOL CFileSystem::EnumEnd()
{
	_vecEnumFiles.clear();
	_vecEnumFolders.clear();
	return TRUE;
}

DWORD CFileSystem::EnumGetFileCount()
{
	return (DWORD)_vecEnumFiles.size();
}

DWORD CFileSystem::EnumGetFolderCount()
{
	return (DWORD)_vecEnumFolders.size();
}


const char *CFileSystem::EnumGetFile(DWORD aidxFile)
{
	if (aidxFile>=_vecEnumFiles.size())
		return "";
	return _vecEnumFiles[aidxFile].c_str();
}

const char *CFileSystem::EnumGetFolder(DWORD aidxFolder)
{
	if (aidxFolder>=_vecEnumFolders.size())
		return "";
	return _vecEnumFolders[aidxFolder].c_str();
}


BOOL CFileSystem::_Rename(const char *apath,const char *apath2,BOOL abNameOrPath)
{
	std::string pathAbs,pathHeader,pathTail;
	std::vector<std::string>aSubPaths;

	if (TRUE)
	{
		char *p=_SearchAbsPath(apath);
		if (!p)
			return FALSE;
		pathAbs=p;
	}

	DWORD attrFile;
	StringLower(pathAbs);
	attrFile=_SplitAbsPath(pathAbs,pathHeader,pathTail,aSubPaths);
	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	if (aSubPaths.size()>0)//the sub path could not be resolved
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}

	if (!(attrFile&FILE_ATTRIBUTE_DIRECTORY))
	{
		//It's a file,check whether it's a package
		//if (!nfs_exists((char*)pathHeader.c_str()))
		if (FilePackageExists(pathHeader.c_str()))
			return FALSE;//not support renaming files in package by now.
	}

	std::string pathNew,tail;
	if (abNameOrPath)
	{
		pathNew=pathAbs;
		if (CutTailSubPath(pathNew,tail))
			pathNew=pathNew+PATH_SLASH+apath2;
		else
			pathNew=apath2;
	}
	else
		pathNew=apath2;

	//Now do the rename
	char buffer[MAX_PATH];
	strcpy(buffer,pathAbs.c_str());
	buffer[pathAbs.length()+1]=0;//an additional terminator
	char buffer2[MAX_PATH];
	strcpy(buffer2,pathNew.c_str());
	buffer2[pathNew.length()+1]=0;//an additional terminator

	SHFILEOPSTRUCT shFileOp;
	memset(&shFileOp,0,sizeof(SHFILEOPSTRUCT));
	shFileOp.hwnd                  = NULL;
	shFileOp.wFunc                 = FO_RENAME;
	shFileOp.pFrom                 = buffer;
	shFileOp.pTo                   = buffer2;
	shFileOp.fFlags                = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
	if (::SHFileOperation(&shFileOp) != 0 )
	{
		SetLastErrCode(Result_DirectoryInUse);
		return FALSE;
	}

	return TRUE;
}

BOOL CFileSystem::Rename(const char *apath,const char *aname)
{
	return _Rename(apath,aname,TRUE);
}

BOOL CFileSystem::RenameAbs(const char *apath,const char *aname)
{
	BOOL bRet;
	PushSearchPath("");
	bRet=Rename(apath,aname);
	PopSearchPath();
	return bRet;
}

BOOL CFileSystem::Move(const char *apath,const char *apathNew)
{
	return _Rename(apath,apathNew,FALSE);

}
BOOL CFileSystem::MoveAbs(const char *apath,const char *apathNew)
{
	BOOL bRet;
	PushSearchPath("");
	bRet=Move(apath,apathNew);
	PopSearchPath();
	return bRet;
}

BOOL CFileSystem::CopyAbs(const char *apath,const char *apathNew)
{
	char buffer[MAX_PATH];
	strcpy(buffer,apath);
	buffer[strlen(apath)+1]=0;//an additional terminator
	char buffer2[MAX_PATH];
	strcpy(buffer2,apathNew);
	buffer2[strlen(apathNew)+1]=0;//an additional terminator

	SHFILEOPSTRUCT shFileOp;
	memset(&shFileOp,0,sizeof(SHFILEOPSTRUCT));
	shFileOp.hwnd                  = NULL;
	shFileOp.wFunc                 = FO_COPY;
	shFileOp.pFrom                 = buffer;
	shFileOp.pTo                   = buffer2;
	shFileOp.fFlags                = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT|FOF_NOCONFIRMMKDIR;
	if (::SHFileOperation(&shFileOp) != 0 )
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}
	return TRUE;
}



//Check whether there is any resource left not released in the system
BOOL CFileSystem::CheckResLeak()
{
	if (_handlepool.m_aHandles.size()>0)
		return TRUE;
	return FALSE;
}

ProfilerMgr *CFileSystem::GetProfilerMgr()
{
	::GetProfilerMgr()->SetName("FileSystem");
	return ::GetProfilerMgr();
}

void CFileSystem::RegisterLogHandler(LogHandler &ahandler)
{
	extern void RegisterLogHandler(LogHandler &handler);
	::RegisterLogHandler(ahandler);
}

FILETIME CFileSystem::GetFileTimeAbs(const char *apath)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	if (GetFileAttributesExA(apath, GetFileExInfoStandard, &fileAttrData))
	{
		return fileAttrData.ftLastWriteTime;
	}
	FILETIME ft;
	ft.dwLowDateTime = 0;
	ft.dwHighDateTime = 0;
	return ft;
}

BOOL CFileSystem::SetFileTimeAbs(const char *apath, const FILETIME aft)
{
	HANDLE hFile = CreateFileA(apath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFile == INVALID_HANDLE_VALUE)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}
	
	BOOL bRet = SetFileTime(hFile, NULL, NULL, &aft);
	CloseHandle(hFile);
	
	if (!bRet)
	{
		SetLastErrCode(Result_InvalidPath);
		return FALSE;
	}
	
	return TRUE;
}
