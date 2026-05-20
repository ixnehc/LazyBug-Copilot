/********************************************************************
	created:	2008/02/19
	created:	19:2:2008   13:35
	filename: 	f:\IxEngine\Common\FilePackage\FilePackageFuns.cpp
	file path:	f:\IxEngine\Common\FilePackage
	file base:	FilePackageFuns
	file ext:	cpp
	author:		szg
	
	purpose:	
*********************************************************************/
#include "stdh.h"
#include "FilePackageFuns.h"

BOOL FilePackageExists(const char* pszPackage)
{
	return CFilePackage::IsFilePackage(pszPackage);
}

BOOL CleanupFilePackage(const char* pszPackage, CProgress* progress)
{
	return CFilePackage::CleanupPackage(pszPackage, progress);
}

HPACKAGE OpenFilePackage(const char* pszPackage, FileAccessMode mode)
{
	CFilePackage* pPackage = new CFilePackage;
	if (pPackage)
	{
		CFileStream::OpenMode openMode;
		if (mode == FileAccessMode_Read)
			openMode = CFileStream::OM_READ;
		else if (mode == FileAccessMode_Write || mode == FileAccessMode_WritePackage)
			openMode = CFileStream::OM_WRITE;
		else if (mode == FileAccessMode_Modify)
			openMode = CFileStream::OM_WRITE;

		if (pPackage->Open(pszPackage, openMode))
			return reinterpret_cast<HPACKAGE>(pPackage);

		delete pPackage;
	}
	return NULL;
}

BOOL PackageFileExists(HPACKAGE hPackage, const char* pszPath)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		return pPackage->FileExists(pszPath);
	}
	return FALSE;
}

BOOL GetPackageFileSize(HPACKAGE hPackage, const char* pszPath,DWORD &sz)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
		return pPackage->GetFileSize(pszPath,sz);
	return FALSE;

}


int SearchPackageFiles(HPACKAGE hPackage, const char* pszPath, FilePathList& rSearchResult)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		return pPackage->SearchFiles(pszPath, rSearchResult);
	}
	return 0;
}

BOOL AddPackageFile(HPACKAGE hPackage, const char* pszFileName, const char* pszPath)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		return pPackage->AddFile(pszFileName, pszPath);
	}
	return FALSE;
}

BOOL RemovePackageFile(HPACKAGE hPackage, const char* pszPath)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		return pPackage->RemoveFile(pszPath);
	}
	return FALSE;
}
BOOL ReplacePackageFile(HPACKAGE hPackage, const char* pszPath, const char* pszFileName)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		return pPackage->ReplaceFile(pszPath, pszFileName);
	}
	return FALSE;
}

BOOL CloseFilePackage(HPACKAGE hPackage)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage && (pPackage->GetOpenFileCount() == 0))
	{
		pPackage->Close();
		delete pPackage;
		return TRUE;
	}
	return FALSE;
}

HPACKAGEFILE CreatePackageFile(HPACKAGE hPackage, const char* pszPath)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		return reinterpret_cast<HPACKAGEFILE>(pPackage->CreateFile(pszPath));
	}
	return NULL;
}

HPACKAGEFILE OpenPackageFile(HPACKAGE hPackage, const char* pszPath)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		return reinterpret_cast<HPACKAGEFILE>(pPackage->OpenFile(pszPath));
	}
	return NULL;
}

LONG64 ReadPackageFile(HPACKAGEFILE hFile, void* lpBuf, LONG64 liSize)
{
	CFilePackageFile* pFile = reinterpret_cast<CFilePackageFile*>(hFile);
	if (pFile)
	{
		return pFile->Read(lpBuf, liSize);
	}
	return 0;
}

LONG64 WritePackageFile(HPACKAGEFILE hFile, const void* lpBuf, LONG64 liSize)
{
	CFilePackageFile* pFile = reinterpret_cast<CFilePackageFile*>(hFile);
	if (pFile)
	{
		return pFile->Write(lpBuf, liSize);
	}
	return 0;
}

LONG64 GetPackageFileSize(HPACKAGEFILE hFile)
{
	CFilePackageFile* pFile = reinterpret_cast<CFilePackageFile*>(hFile);
	if (pFile)
	{
		return pFile->GetSize();
	}
	return 0;
}

LONG64 SetPackageFilePointer(HPACKAGEFILE hFile, LONG64 liPosition)
{
	CFilePackageFile* pFile = reinterpret_cast<CFilePackageFile*>(hFile);
	if (pFile)
	{
		return pFile->Seek(liPosition);
	}
	return 0;
}

LONG64 GetPackageFilePointer(HPACKAGEFILE hFile)
{
	CFilePackageFile* pFile = reinterpret_cast<CFilePackageFile*>(hFile);
	if (pFile)
	{
		return pFile->GetCurPos();
	}
	return 0;

}


void ClosePackageFile(HPACKAGE hPackage, HPACKAGEFILE hFile)
{
	CFilePackage* pPackage = reinterpret_cast<CFilePackage*>(hPackage);
	if (pPackage)
	{
		pPackage->CloseFile(reinterpret_cast<CFilePackageFile*>(hFile));
	}
}