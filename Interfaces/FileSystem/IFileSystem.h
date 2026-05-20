#pragma once
#include <vector>
#include <string>

#include "FileSystemDefines.h"

#define IFile_ReadVar(fl,v) (*(fl)).Read((void*)&(v),sizeof(v));
#define IFile_WriteVar(fl,v) (*(fl)).Write((void*)&(v),sizeof(v));

#define IFile_ReadArray(fl,v) (*(fl)).Read((void*)(v),sizeof(v));
#define IFile_WriteArray(fl,v) (*(fl)).Write((void*)(v),sizeof(v));

//IFile*fl,std::string s 或 std::wstring s
#define IFile_WriteString(fl,s) \
	{\
		long len;\
		len=(long)(s).length();\
		(*(fl))<<len;\
		if (len > 0) {\
			DWORD byteSize = (DWORD)(len * sizeof((s)[0]));\
			(*(fl)).Write((void*)(s).c_str(), byteSize);\
		}\
	}

//IFile*fl,std::string s 或 std::wstring s
#define IFile_ReadString(fl,s)\
	{\
		long len;\
		(*(fl))>>len;\
		(s).resize(len);\
		if (len > 0) {\
			DWORD byteSize = (DWORD)(len * sizeof((s)[0]));\
			(*(fl)).Read((void*)(s).data(), byteSize);\
		}\
	}


//IFile *fl,std::vector<???> vec
#define IFile_WriteVector(fl,vec)\
{\
	DWORD sz;\
	sz=(DWORD)(vec).size();\
	(*(fl))<<sz;\
	if (sz>0)\
	{\
		sz=sz*sizeof((vec)[0]);\
		(*(fl)).Write(&(vec)[0],sz);\
	}\
}

//IFile *fl,std::vector<???> vec
#define IFile_ReadVector(fl,vec)\
{\
	DWORD sz;\
	(*(fl))>>sz;\
	(vec).resize(sz);\
	if (sz>0)\
	{\
		sz=sz*sizeof((vec)[0]);\
		(*(fl)).Read(&(vec)[0],sz);\
	}\
}

#define IFile_ReadGObj(fl,obj)												\
{																								\
	std::vector<BYTE>__buf;													\
	IFile_ReadVector(fl,__buf);													\
	CDataPacket __dp;																\
	__dp.SetDataBufferPointer(__buf.data());									\
	obj.GLoad(__dp);																	\
}

#define IFile_WriteGObj(fl,obj)												\
{																								\
	std::vector<BYTE>__buf;													\
	DP_BeginSave(__dp,__buf);													\
		obj.GSave(__dp);																\
	DP_EndSave();																		\
	IFile_WriteVector(fl,__buf);													\
}

#define IFileSystem_EnsureFolderAbs(fs,path)								\
{\
IFolder* pFolder = (fs)->OpenFolderAbs(path, FileAccessMode_Write);\
if (pFolder)\
{\
	(fs)->CloseFolder(pFolder);\
}\
}


#define IFileSystem_EnumCore(fs,path,filelist,bRecursive,func1,func2)\
	{\
	(filelist).clear();\
	DWORD c;\
	(fs)->EnumBegin(path,bRecursive);\
	c=(fs)->func1();\
	if(c>0)\
		{\
		(filelist).resize(c);\
		DWORD i;\
		for (i=0;i<c;i++)\
		(filelist)[i]=(fs)->func2(i);\
		}\
		(fs)->EnumEnd();\
	}

#define IFileSystem_EnumBothCore(fs,path,filelist,folderlist,bRecursive)\
{\
	(filelist).clear();\
	(folderlist).clear();\
	DWORD c;\
	(fs)->EnumBegin(path,bRecursive);\
	c=(fs)->EnumGetFileCount();\
	if(c>0)\
	{\
		(filelist).resize(c);\
		DWORD i;\
		for (i=0;i<c;i++)\
		(filelist)[i]=(fs)->EnumGetFile(i);\
	}\
	c=(fs)->EnumGetFolderCount();\
	if(c>0)\
	{\
		(folderlist).resize(c);\
		DWORD i;\
		for (i=0;i<c;i++)\
		(folderlist)[i]=(fs)->EnumGetFolder(i);\
	}\
	(fs)->EnumEnd();\
}


//IFileSystem*fs,const char *path,std::vector<std::string>filelist
//Sample: IFileSystem_EnumFiles(pFS,"C:\\Windows",vecFiles);
#define IFileSystem_EnumFiles(fs,path,filelist)\
	IFileSystem_EnumCore(fs,path,filelist,FALSE,EnumGetFileCount,EnumGetFile)

//IFileSystem*fs,const char *path,std::vector<std::string>filelist
//recursive version
#define IFileSystem_EnumFilesR(fs,path,filelist)\
	IFileSystem_EnumCore(fs,path,filelist,TRUE,EnumGetFileCount,EnumGetFile)

#define IFileSystem_EnumFolders(fs,path,folderlist)\
	IFileSystem_EnumCore(fs,path,folderlist,FALSE,EnumGetFolderCount,EnumGetFolder)

#define IFileSystem_EnumFoldersR(fs,path,folderlist)\
	IFileSystem_EnumCore(fs,path,folderlist,TRUE,EnumGetFolderCount,EnumGetFolder)

//enumerate both file and folder
#define IFileSystem_EnumAllR(fs,path,filelist,folderlist)\
	IFileSystem_EnumBothCore(fs,path,filelist,folderlist,TRUE)

#define IFileSystem_EnumAll(fs,path,filelist,folderlist)\
	IFileSystem_EnumBothCore(fs,path,filelist,folderlist,FALSE)

#define IFolder_EnumFilesR(folder,path,filelist) IFileSystem_EnumFilesR(folder,path,filelist)
#define IFolder_EnumFiles(folder,path,filelist) IFileSystem_EnumFiles(folder,path,filelist)

#define IFolder_EnumFoldersR(folder,path,folderlist) IFileSystem_EnumFoldersR(folder,path,folderlist)
#define IFolder_EnumFolders(folder,path,folderlist) IFileSystem_EnumFolders(folder,path,folderlist)


class IFileSystem;
class IFile
{
public:
	// insertion operations
	virtual IFile& operator<<(bool value)=0;
	virtual IFile& operator<<(char value)=0;
	virtual IFile& operator<<(short value)=0;
	virtual IFile& operator<<(long value)=0;
	virtual IFile& operator<<(int value)=0;
	virtual IFile& operator<<(unsigned char value)=0;
	virtual IFile& operator<<(unsigned short value)=0;
	virtual IFile& operator<<(unsigned long value)=0;
	virtual IFile& operator<<(unsigned int value)=0;
	virtual IFile& operator<<(float value)=0;
	virtual IFile& operator<<(double value)=0;

	// extraction operations
	virtual IFile& operator>>(bool &value)=0;
	virtual IFile& operator>>(char &value)=0;
	virtual IFile& operator>>(short &value)=0;
	virtual IFile& operator>>(long &value)=0;
	virtual IFile& operator>>(int &value)=0;
	virtual IFile& operator>>(unsigned char &value)=0;
	virtual IFile& operator>>(unsigned short &value)=0;
	virtual IFile& operator>>(unsigned long &value)=0;
	virtual IFile& operator>>(unsigned int &value)=0;
	virtual IFile& operator>>(float &value)=0;
	virtual IFile& operator>>(double &value)=0;

	virtual void Read(void* lpBuf, DWORD nMax)=0;
	virtual void Write(const void* lpBuf, DWORD nMax)=0;

	virtual BOOL IsReading()=0;//Read ?
	virtual BOOL IsWriting()=0;//Writing?

	virtual const char *GetPath()=0;//a lower cased path,linked by "\\"
	virtual const char *GetSuffix()=0;
	virtual int GetSize()=0;//Only valid when reading
	virtual void Reset()=0;//Seek to begin
	virtual void Seek(DWORD iPos)=0;//From beginning
	virtual DWORD GetCurPos()=0;
	virtual void Close()=0;
};

#ifndef MOBILE

class IFolder
{
public:
	virtual IFileSystem *GetFS()=0;

	virtual IFile *SeekFile(const char *pathFile)=0;//The returned pointer should not be close using IFileSystem::CloseFile()
	virtual void CloseSeekFile()=0;
	virtual IFile *GetSeekFile()=0;//call this function after seeking a file
	virtual BOOL RemoveFile(const char *pathFile)=0;

	virtual DWORD EnumBegin(const char *path,BOOL bRecursive)=0;//return how many files and folders are enumerated
	virtual DWORD EnumGetFileCount()=0;
	virtual const char *EnumGetFile(DWORD idxFile)=0;
	virtual DWORD EnumGetFolderCount()=0;
	virtual const char *EnumGetFolder(DWORD idxFolder)=0;
	virtual BOOL EnumEnd()=0;

	virtual const char *GetPath()=0;//linked by "\\"
	virtual void Close()=0;
};

#endif


struct ProfilerMgr;
struct LogHandler;
class CProgress;
class IMapFile;
class IFileSystem
{
public:
	virtual void SetSearchPath(const char *pathSearch)=0;//path should not include the slash at the tail
	virtual void PushSearchPath(const char *pathSearch)=0;
	virtual void PopSearchPath()=0;


	virtual IFile *OpenFile(const char *path,FileAccessMode modeOpen)=0;
	virtual IFile *OpenFileAbs(const char *path,FileAccessMode modeOpen)=0;//path is an absolute path(will not search in search path)
	virtual void CloseFile(IFile *pFile)=0;

#ifndef MOBILE
	virtual DWORD GetFileSize(const char *path)=0;//如果失败,返回0xffffffff
	virtual DWORD GetFileSizeAbs(const char *path)=0;////如果失败,返回0xffffffff.path is an absolute path(will not search in search path)

	virtual IFolder *OpenFolder(const char *path,FileAccessMode modeOpen)=0;
	virtual IFolder *OpenFolderAbs(const char *path,FileAccessMode modeOpen)=0;
	virtual void CloseFolder(IFolder *pFolder)=0;

	virtual BOOL ExistFolder(const char *path)=0;
	virtual BOOL ExistFolderAbs(const char *path)=0;//path is an absolute path(will not search in search path)
	virtual BOOL ExistFile(const char *path)=0;
	virtual BOOL ExistFileAbs(const char *path)=0;//path is an absolute path(will not search in search path)


	virtual FileAttr GetFileAttr(const char *path)=0;
	virtual FileAttr GetFileAttrAbs(const char *path)=0;//path is an absolute path(will not search in search path)
	virtual BOOL SetFileAttr(const char *path,FileAttr attr)=0;
	virtual BOOL SetFileAttrAbs(const char *path,FileAttr attr)=0;//path is an absolute path(will not search in search path)
	virtual BOOL Rename(const char *path,const char *name)=0;
	virtual BOOL RenameAbs(const char *path,const char *name)=0;
	virtual BOOL Move(const char *path,const char *pathNew)=0;
	virtual BOOL MoveAbs(const char *path,const char *pathNew)=0;
	virtual BOOL CopyAbs(const char *path,const char *pathNew)=0;
	virtual BOOL RemoveFile(const char *path)=0;
	virtual BOOL RemoveFileAbs(const char *path)=0;
	virtual BOOL RemoveFolder(const char *path)=0;//directory or package(or folder in package)
	virtual BOOL RemoveFolderAbs(const char *path)=0;//directory or package(or folder in package)
	virtual BOOL RemovePackage(const char *path)=0;//only package folder(or folder in package)
	virtual BOOL CleanUpPackage(const char *path,CProgress *progress)=0;//remove the obsolete chunks in a file package

	//enumerate the files beneath the path and return how many files are enumerated
	//by now,folders will not be enumerated from a package file
	//目前这个函数不会enum隐藏文件
	//目前filter功能对package file无效
	//return 0xffffffff if some error occurs
	virtual DWORD EnumBegin(const char *path,BOOL bRecursive,EnumFileFilter filter=NULL,void *param=NULL)=0;//return how many files and folders are enumerated
	virtual DWORD EnumGetFileCount()=0;
	virtual const char *EnumGetFile(DWORD idxFile)=0;
	virtual DWORD EnumGetFolderCount()=0;
	virtual const char *EnumGetFolder(DWORD idxFolder)=0;
	virtual BOOL EnumEnd()=0;

	virtual BOOL CheckResLeak()=0;//Check whether there is any resource left not released in the system


	virtual ProfilerMgr *GetProfilerMgr()=0;
	virtual void RegisterLogHandler(LogHandler &handler)=0;

	virtual FILETIME GetFileTimeAbs(const char *path)=0;
	virtual BOOL SetFileTimeAbs(const char* path, FILETIME time) = 0;

#endif

};


