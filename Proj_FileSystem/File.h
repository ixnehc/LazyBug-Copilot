#pragma once

#include <vector>
#include <string>

#include "FileSystem/IFileSystem.h"

#include "FilePackage/FilePackageFuns.h"

//internal use
#define PATH_SLASH2 "/"
#define PATH_SLASH2_C '/'


enum FileType
{
	FileType_None,
	FileType_File,
	FileType_PackageFile,
};

class CFileStream;
class CFile:public IFile
{
public:
	CFile(void);


	// insertion operations
	virtual IFile& operator<<(bool value);
	virtual IFile& operator<<(char value);
	virtual IFile& operator<<(short value);
	virtual IFile& operator<<(long value);
	virtual IFile& operator<<(int value);
	virtual IFile& operator<<(unsigned char value);
	virtual IFile& operator<<(unsigned short value);
	virtual IFile& operator<<(unsigned long value);
	virtual IFile& operator<<(unsigned int value);
	virtual IFile& operator<<(float value);
	virtual IFile& operator<<(double value);

	// extraction operations
	virtual IFile& operator>>(bool &value);
	virtual IFile& operator>>(char &value);
	virtual IFile& operator>>(short &value);
	virtual IFile& operator>>(long &value);
	virtual IFile& operator>>(int &value);
	virtual IFile& operator>>(unsigned char &value);
	virtual IFile& operator>>(unsigned short &value);
	virtual IFile& operator>>(unsigned long &value);
	virtual IFile& operator>>(unsigned int &value);
	virtual IFile& operator>>(float &value);
	virtual IFile& operator>>(double &value);

	virtual void Read(void* lpBuf, DWORD nMax);
	virtual void Write(const void* lpBuf, DWORD nMax);

	virtual BOOL IsReading();//Read ?
	virtual BOOL IsWriting();//Writing?

	virtual const char *GetPath();
	virtual const char *GetSuffix();
	virtual int GetSize();//Only valid when reading
	virtual void Reset();//Seek to begin
	virtual void Seek(DWORD iPos);//From beginning
	virtual DWORD GetCurPos();
	virtual void Close();

private:
	std::string _path;
	FileAccessMode _mode;
	FileType _type;
	IFileSystem *_pFS;
	IFolder *_folder;//the owner folder
	union
	{
		struct //FileType_File
		{
			CFileStream *_stream;
		};

		struct //FileType_Package
		{
			HANDLE _hPackage;
			HANDLE _hPackageFile;
		};
	};

	void _InitFile(CFileStream *stream,FileAccessMode mode,const char *path,IFileSystem *pFS);
	void _InitPackageFile(HANDLE hPackage,HANDLE hFile,FileAccessMode mode,const char *path,IFileSystem *pFS);

	void _Close(HANDLE &hNFS);//if a package file,in hNFS returns the handle to it
	void _ClosePackageFile(BOOL bClosePackage);

	void _SetOwnerFolder(IFolder *folder);


	void _WriteInternal(const void *buffer,UINT size);
	void _ReadInternal(void *buffer,UINT size);

	friend class CFileSystem;
	friend class CFolder;
};
