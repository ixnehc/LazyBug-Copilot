/********************************************************************
	created:	2006/05/21
	created:	21:5:2006   15:06
	filename: 	d:\IxEngine\Proj_FileSystem\Folder.cpp
	author:		cxi
	
	purpose:	implement of IFolder
*********************************************************************/
#include "stdh.h"
#include "assert.h"
#include "File.h"
#include ".\Folder.h"

#include "Log/LastError.h"





CFolder::CFolder(void)
{
	_type=FolderType_None;
	_pSeekedFile=NULL;

	_hNFS=NULL;

	_pFS=NULL;
}

void CFolder::_Close(HANDLE &hNFS)
{
	if (_type==FolderType_None)
		return;

	CloseSeekFile();

	_type=FolderType_None;

	_pFS=NULL;

	hNFS=_hNFS;
	_hNFS=NULL;
}


void CFolder::_Init(IFileSystem *pFileSystem,HANDLE hNFS,const char *pathFolder,FileAccessMode mode)
{
	assert(_type==FolderType_None);

	_type=FolderType_Folder;
	_mode=mode;

	_path=pathFolder;

	_pFS=pFileSystem;

	_hNFS=hNFS;
}

IFileSystem *CFolder::GetFS()
{
	return _pFS;
}


void CFolder::CloseSeekFile()
{
	if (_pSeekedFile&&_pFS)
	{
		((CFile *)_pSeekedFile)->_SetOwnerFolder(NULL);//dismiss it
		_pFS->CloseFile(_pSeekedFile);
		_pSeekedFile=NULL;
	}
}

//if you call this function,no matter it will be successful or not,the original seeked file will be closed
IFile *CFolder::SeekFile(const char *path)
{
	CloseSeekFile();
	assert(_pFS);

	_pFS->PushSearchPath(_path.c_str());
	_pSeekedFile=_pFS->OpenFile(path,_mode);
	if (_pSeekedFile)
		((CFile *)_pSeekedFile)->_SetOwnerFolder(this);//own it
	_pFS->PopSearchPath();

	return _pSeekedFile;
}

IFile *CFolder::GetSeekFile()//call this function after seeking a file.
{
	return _pSeekedFile;
}


BOOL CFolder::RemoveFile(const char *path)
{
	if (_mode==FileAccessMode_Read)
	{
		SetLastErrCode(Result_WritingReadOnly);
		return FALSE;
	}
	assert(_pFS);

	BOOL bRet;
	_pFS->PushSearchPath(_path.c_str());
	bRet=_pFS->RemoveFile(path);
	_pFS->PopSearchPath();

	return bRet;
}

const char *CFolder::GetPath()
{
	return _path.c_str();
}

DWORD CFolder::EnumBegin(const char *path,BOOL bRecursive)
{
	assert(_pFS);

	DWORD dwRet;
	_pFS->PushSearchPath(_path.c_str());
	dwRet=_pFS->EnumBegin(path,bRecursive);
	_pFS->PopSearchPath();

	return dwRet;
}

BOOL CFolder::EnumEnd()
{
	assert(_pFS);

	return _pFS->EnumEnd();
}

DWORD CFolder::EnumGetFileCount()
{
	assert(_pFS);
	return _pFS->EnumGetFileCount();
}
const char *CFolder::EnumGetFile(DWORD idxFile)
{
	assert(_pFS);
	return _pFS->EnumGetFile(idxFile);
}
DWORD CFolder::EnumGetFolderCount()
{
	assert(_pFS);
	return _pFS->EnumGetFolderCount();
}
const char *CFolder::EnumGetFolder(DWORD idxFolder)
{
	assert(_pFS);
	return _pFS->EnumGetFolder(idxFolder);
}


void CFolder::Close()
{
	assert(_pFS);
	_pFS->CloseFolder((IFolder *)this);
}
