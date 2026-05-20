/********************************************************************
	created:	2006/05/21
	created:	21:5:2006   15:06
	filename: 	d:\IxEngine\Proj_FileSystem\File.cpp
	author:		cxi
	
	purpose:	implement of IFile
*********************************************************************/
#include "stdh.h"

#include "File.h"

#include "Folder.h"
#include "FileSystem.h"

#include "stringparser/stringparser.h"

#include "assert.h"
#pragma warning(disable:4312)


CFile::CFile(void)
{
	_type=FileType_None;

	_stream=NULL;
	
	_hPackage = NULL;
	_hPackageFile = NULL;

	_pFS=NULL;
	_folder=NULL;
}

const char *CFile::GetSuffix()
{
	int i1,i2;
	i1=StringReverseFind(_path.c_str(),'.');
	i2=StringReverseFind(_path.c_str(),'\\');
	if (i1>i2)
		return _path.c_str()+i1+1;
	return _path.c_str()+_path.length();
}


BOOL CFile::IsReading()//Read ?
{
	return (_mode==FileAccessMode_Read)||(_mode==FileAccessMode_Modify);
}
BOOL CFile::IsWriting()//Writing?
{
	return (_mode==FileAccessMode_Write)||(_mode==FileAccessMode_Modify);
}


void CFile::_Close(HANDLE &hNFS)
{
	hNFS=NULL;
	if (_type==FileType_File)
	{
		if (_stream)
			delete _stream;
		_stream=NULL;
	}
	if (_type==FileType_PackageFile)
	{
		// Close the open package file
		ClosePackageFile(_hPackage, _hPackageFile);

		// Return the file package handle
		hNFS = _hPackage;

		// Reset
		_hPackage = NULL;
		_hPackageFile = NULL;
	}

	_path="";
	_type=FileType_None;

}


void CFile::_InitFile(CFileStream *stream,FileAccessMode mode,const char *path,IFileSystem *pFS)
{
	assert(_type==FileType_None);

	_mode=mode;
	_type=FileType_File;
	_stream=stream;
	_path=path;
	_pFS=pFS;
}

void CFile::_InitPackageFile(HANDLE hPackage,HANDLE hFile,FileAccessMode mode,const char *path,IFileSystem *pFS)
{
	assert(_type==FileType_None);

	_mode=mode;
	_type=FileType_PackageFile;

	// Store the package & package file handles
	_hPackage = hPackage;
	_hPackageFile = hFile;

	_path=path;
	_pFS=pFS;
}

void CFile::_SetOwnerFolder(IFolder *folder)
{
	_folder=folder;
}



void CFile::_WriteInternal(const void *buffer,UINT size)
{
	if (size<=0)
		return;
	DWORD dwWritten;
	if (FileType_File==_type)
	{
		dwWritten=0;
		if (_stream)
			dwWritten=(DWORD)_stream->Write(buffer,size);
	}
	if (FileType_PackageFile==_type)
	{
		// Write file data in the package file
		dwWritten = (DWORD) WritePackageFile(_hPackageFile, buffer, size);
	}

#ifndef MOBILE
	if (dwWritten<size)
	{
		std::string s;
		s="File Write:Failed to write data to file:\"";
		s+=_path;
		s+="\"!";

		MessageBox(NULL,s.c_str(),"File Write Error",MB_OK);
	}
#endif
}
void CFile::_ReadInternal(void *buffer,UINT size)
{
	if (size<=0)
		return;
	memset(buffer,0,size);
	DWORD dwRead;
	dwRead=0;
	if (FileType_File==_type)
	{
		if (_stream)
		{
			dwRead=(DWORD)_stream->Read(buffer,size);
			if (dwRead<size)
			{
				for (int i=0;i<10;i++)
				{
					Sleep(100);
					dwRead+=(DWORD)_stream->Read((BYTE*)buffer+dwRead,size-dwRead);
					if (dwRead>=size)
						break;
				}
			}
		}
	}
	if (FileType_PackageFile==_type)
	{
		// Read file data from the package file
		dwRead = (DWORD) ReadPackageFile(_hPackageFile, buffer, size);
	}

	if (dwRead<size)
	{
		memset(buffer,0,size);//Clear as all zero
		std::string s;
		s="File Read:Failed to read data from file:\"";
		s+=_path;
		s+="\"!";

		MessageBox(NULL,s.c_str(),"File Read Error",MB_OK);
	}
}

const char *CFile::GetPath()
{
	return _path.c_str();
}


int CFile::GetSize()//Only valid when reading,otherwise return -1
{
	if (!IsReading())
		return -1;

	if (FileType_File==_type)
	{
		if(_stream)
			return (int)_stream->GetSize();
		return 0;
	}

	if (FileType_PackageFile==_type)
	{
		// Get the package file size
		return (DWORD) GetPackageFileSize(_hPackageFile);
	}

	return -1;
}



#define GENERIC_VALUEWRITE_CODE \
	if (_type==FileType_None)\
		return *this;\
	if (!IsWriting())\
		return *this;\
	_WriteInternal(&value,sizeof(value));\
		return *this;

#define GENERIC_VALUEREAD_CODE \
	if (_type==FileType_None)\
		return *this;\
	if (!IsReading())\
		return *this;\
	_ReadInternal(&value,sizeof(value));\
		return *this;

IFile& CFile::operator<<(bool value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(char value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(short value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(long value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(int value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(unsigned char value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(unsigned short value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(unsigned long value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(unsigned int value)
{
	GENERIC_VALUEWRITE_CODE;
}

IFile& CFile::operator<<(float value)
{
	GENERIC_VALUEWRITE_CODE;
}
IFile& CFile::operator<<(double value)
{
	GENERIC_VALUEWRITE_CODE;
}

// extraction operations
IFile& CFile::operator>>(bool& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(char& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(short& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(long& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(int& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(unsigned char& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(unsigned short& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(unsigned long& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(unsigned int& value)
{
	GENERIC_VALUEREAD_CODE;
}

IFile& CFile::operator>>(float& value)
{
	GENERIC_VALUEREAD_CODE;
}
IFile& CFile::operator>>(double& value)
{
	GENERIC_VALUEREAD_CODE;
}


void CFile::Read(void* lpBuf, DWORD nMax)
{
	if (_type==FileType_None)
		return;
	if (!IsReading())
		return;
	_ReadInternal(lpBuf,nMax);
	return;

}
void CFile::Write(const void* lpBuf, DWORD nMax)
{
	if (_type==FileType_None)
		return;
	if (!IsWriting())
		return;
	_WriteInternal(lpBuf,nMax);
	return;
}

//Seek to begin
void CFile::Reset()
{
	if (FileType_File==_type)
	{
		if (_stream)
			_stream->Seek(0);
	}
	if (FileType_PackageFile==_type)
	{
		// Set the package file cursor at the file beginning
		SetPackageFilePointer(_hPackageFile, 0);
	}

}

//From beginning
void CFile::Seek(DWORD iPos)
{
	if (FileType_File==_type)
	{
		if (_stream)
			_stream->Seek(iPos);
	}
	if (FileType_PackageFile==_type)
	{
		// Move the package file cursor
		SetPackageFilePointer(_hPackageFile, iPos);
	}
}

DWORD CFile::GetCurPos()
{
	if (FileType_File==_type)
	{
		if (_stream)
			return (DWORD)_stream->Tell();
		return 0;
	}
	else
		return (DWORD)GetPackageFilePointer(_hPackageFile);
}


void CFile::Close()
{
	if (_folder)
	{
		_folder->CloseSeekFile();
		return;
	}
	if (_pFS)
		_pFS->CloseFile(this);
}
