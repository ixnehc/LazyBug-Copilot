#ifndef __FilePackageFuns_H__
#define __FilePackageFuns_H__
#include "FilePackageDefines.h"
#include "FilePackage.h"

BOOL FilePackageExists(const char* pszPackage);
BOOL CleanupFilePackage(const char* pszPackage, CProgress* progress);

HPACKAGE OpenFilePackage(const char* pszPackage, FileAccessMode mode);
BOOL PackageFileExists(HPACKAGE hPackage, const char* pszPath);
BOOL GetPackageFileSize(HPACKAGE hPackage, const char* pszPath,DWORD &sz);
int SearchPackageFiles(HPACKAGE hPackage, const char* pszPath, FilePathList& rSearchResult);
BOOL AddPackageFile(HPACKAGE hPackage, const char* pszFileName, const char* pszPath);
BOOL RemovePackageFile(HPACKAGE hPackage, const char* pszPath);
BOOL ReplacePackageFile(HPACKAGE hPackage, const char* pszPath, const char* pszFileName);
BOOL CloseFilePackage(HPACKAGE hPackage);

HPACKAGEFILE CreatePackageFile(HPACKAGE hPackage, const char* pszPath);
HPACKAGEFILE OpenPackageFile(HPACKAGE hPackage, const char* pszPath);
LONG64 ReadPackageFile(HPACKAGEFILE hFile, void* lpBuf, LONG64 liSize);
LONG64 WritePackageFile(HPACKAGEFILE hFile, const void* lpBuf, LONG64 liSize);
LONG64 GetPackageFileSize(HPACKAGEFILE hFile);
LONG64 SetPackageFilePointer(HPACKAGEFILE hFile, LONG64 liPosition);
LONG64 GetPackageFilePointer(HPACKAGEFILE hFile);
void ClosePackageFile(HPACKAGE hPackage, HPACKAGEFILE hFile);

#endif