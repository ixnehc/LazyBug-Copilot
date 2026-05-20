/********************************************************************
	created:	2006/05/13
	created:	13:5:2006   16:11
	filename: 	d:\IxEngine\Interfaces\FileSystem\IFileSystem.cpp
	author:		cxi
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include ".\chunkfilesys.h"

#include "StringParser\stringparser.h"

#include "MpqFileList.h"

//Check whether path is a MPQ file
BOOL CheckMPQFormat(char *path)
{
	OFSTRUCT os;
	os.cBytes=sizeof(os);
	HFILE hFile;
	hFile=OpenFile(path,&os,OF_READ);

	if (hFile==HFILE_ERROR)
		return FALSE;//The file cannot be read;

	MPQHEADER header;
	DWORD dwRead;
	if (FALSE==ReadFile((HANDLE)hFile,&header,sizeof(header),&dwRead,NULL))
		dwRead=0;

	if (dwRead<sizeof(header))
	{
		CloseHandle((HANDLE)hFile);
		return FALSE;//Cannot even read the head;
	}

	char mpqid[]="MPQ\x1A";
	if (header.dwMPQID!=*((DWORD*)mpqid))
	{
		CloseHandle((HANDLE)hFile);
		return FALSE;
	}
	if (header.dwHeaderSize!=sizeof(header))
	{
		CloseHandle((HANDLE)hFile);
		return FALSE;
	}

	DWORD szFile;
	szFile=GetFileSize((HANDLE)hFile,NULL);

	if (szFile!=header.dwMPQSize)
	{
		CloseHandle((HANDLE)hFile);
		return FALSE;
	}

	CloseHandle((HANDLE)hFile);

	return TRUE;
}


HANDLE CReadMpqHandlePool::Open(char *buffer)
{
	MPQHANDLE hMPQArchive;
	if (!CheckMPQFormat(buffer))
		hMPQArchive=NULL;
	else
	{
		if(FALSE==SFileOpenArchive(buffer,0,0,&hMPQArchive))
			hMPQArchive=NULL;
	}

	return (HANDLE)hMPQArchive;

}
void CReadMpqHandlePool::Close(HANDLE hHandle)
{
	SFileCloseArchive(hHandle);
}

CReadMpqHandlePool g_ReadMpqHandlePool;

HANDLE OpenMpqHandleForRead(char *buffer)
{
	return g_ReadMpqHandlePool.OpenHandle(buffer);
}

void CloseMpqHandleForRead(HANDLE hHandle)
{
	g_ReadMpqHandlePool.CloseHandle(hHandle);
}




CChunkFileSys g_ChunkFileSys;
CChunkFileSys *g_pChunkFileSys=&g_ChunkFileSys;




CChunkFileSys::CChunkFileSys(void)
{
	SetSearchPath("");
}

CChunkFileSys::~CChunkFileSys(void)
{
}

void CChunkFileSys::SetSearchPath(const char *path)//path should not include the slash at the tail
{
	m_aSearchPaths.RemoveAll();
	m_aSearchPaths.Add(path);
}

void CChunkFileSys::SetSearchPathAsModulePath(HMODULE hModule)
{
	CString strTemp;
	GetModuleFileName(hModule,strTemp.GetBuffer(1024),1024);
	strTemp.ReleaseBuffer();
	strTemp = strTemp .Left(strTemp.ReverseFind('\\'));
	SetSearchPath((LPSTR)(LPCTSTR)strTemp);
}

void CChunkFileSys::PushSearchPath(const char *pathSearch)
{
	m_aSearchPaths.InsertAt(0,pathSearch);
}
void CChunkFileSys::PopSearchPath()
{
	m_aSearchPaths.RemoveAt(0);
}



void CChunkFileSys::CloseChunk(CChunk *pChunk)
{
	if (pChunk)
	{
		pChunk->Close();
		delete pChunk;
	}
}
void CChunkFileSys::CloseChunkFolder(CChunkFolder *pChunkFolder)
{
	if (pChunkFolder)
	{
		pChunkFolder->Close();
		delete pChunkFolder;
	}
}

BOOL CChunkFileSys::CheckInputPathValidity(char *path)
{
	if (path==NULL)
		return FALSE;
	if (*path==0)
		return FALSE;

	if (*path==PATH_SLASH_C)
		return FALSE;

	char *p;
	p=path;
	while(*p)
		p++;

	p--;
	if (*p==PATH_SLASH_C)
		return FALSE;

	return TRUE;
}

CMpqFileList *GetMpqFileListFromMpqArchive(CString &pathArchive)
{
	CMpqFileList *p;
	p=new CMpqFileList;

	CString path;
	path=pathArchive;
	path+=PATH_SLASH;
	path+=FILELISTPATH;

	MPQHANDLE hMPQArchive;
	hMPQArchive=OpenMpqHandleForRead((LPSTR)(LPCTSTR)pathArchive);

	if (IS_MPQHANDLE_VALID(hMPQArchive))
	{
		MPQHANDLE hMPQFile;
		if (SFileOpenFileEx(hMPQArchive,FILELISTPATH,0,&hMPQFile))
		{
			CChunk temp;
			temp.InitMPQFile(hMPQArchive,hMPQFile,path);
			
			p->ReadFromChunk(&temp);

			temp.Close();//The hMPQArchive,hMPQFile is closed in the CChunk::Close();
		}
		else
			CloseMpqHandleForRead(hMPQArchive);
	}

	return p;
}

CChunkFolder *CChunkFileSys::OpenChunkFolder(const char *path,ChunkAccessMode modeOpen,int packagesize)
{
	if (!CheckInputPathValidity((char *)path))
	{
		SetLastError(ChunkFileResult_InvalidPath);
		return NULL;
	}

	if (m_aSearchPaths.GetSize()<=0)
	{
		SetLastError(ChunkFileResult_NoSearchPath);
		return NULL;
	}

	int i;
	for (i=0;i<1;i++)
	{
		char buffer[1024];
		int nLen;
		nLen=m_aSearchPaths[i].GetLength();
		if (nLen>0)
		{
			memcpy(buffer,(LPCTSTR)m_aSearchPaths[i],nLen);
			buffer[nLen]=PATH_SLASH_C;
			nLen++;
		}

		char *p;
		p=(char*)path;

		BOOL bTooLongPath;
		bTooLongPath=FALSE;
		while(*p)
		{
			if (nLen>=sizeof(buffer)-1)
			{
				SetLastError(ChunkFileResult_TooLongPath);
				bTooLongPath=TRUE;
				break;
			}
			buffer[nLen]=*p;
			nLen++;
			p++;
		}

		if (bTooLongPath)
			continue;

		buffer[nLen]=0;//terminator


		CStringArray aSubPaths;
		DWORD attrFile;
		while(1)
		{
			attrFile=GetFileAttributes(buffer);

			char temp[MAX_PATH];

			if (attrFile!=INVALID_FILE_ATTRIBUTES)
				break;


			if (CutTailSubPath(buffer,temp)<=0)
				break;

			aSubPaths.Add(CString(temp));
		}

		if (attrFile==INVALID_FILE_ATTRIBUTES)
		{
			SetLastError(ChunkFileResult_InvalidPath);
			continue;//for the next search path
		}

		if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (modeOpen==ChunkAccessMode_Read)
			{
				if (aSubPaths.GetSize()>0)//Sub path could not be resolved,
				{
					//for reading,we should fail here
					SetLastError(ChunkFileResult_InvalidPath);
					continue;//for the next search path
				}

				CChunkFolder *pChunkFolder;

				pChunkFolder=new CChunkFolder;

				pChunkFolder->InitFolder(CString(buffer),modeOpen);

				return pChunkFolder;
			}

			if ((modeOpen==ChunkAccessMode_WritePackage)||(modeOpen==ChunkAccessMode_Write))
			{
				if (aSubPaths.GetSize()<=0)
				{
					if (modeOpen==ChunkAccessMode_WritePackage)//the required package is already here as a directory
						modeOpen=ChunkAccessMode_Write;//We take it as directory
				}
				
				if (modeOpen==ChunkAccessMode_Write)//Create the sub folders
				{
					CString s;
					s=buffer;
					int i;
					for (i=aSubPaths.GetSize()-1;i>=0;i--)
					{
						s+=PATH_SLASH;
						s+=aSubPaths[i];

						if (FALSE==CreateDirectoryA((LPCTSTR)s,NULL))
						{
							SetLastError(ChunkFileResult_CannotCreateDirectory);
							break;
						}
					}
					if (i>=0)
						continue;//Fail

					CChunkFolder *pChunkFolder;

					pChunkFolder=new CChunkFolder;
					pChunkFolder->InitFolder(s,ChunkAccessMode_Write);//change to Write after creating
					return pChunkFolder;
				}
				else
				{
					//ChunkAccessMode_WritePackage
					CString s;
					s=buffer;
					int i;
					for (i=aSubPaths.GetSize()-1;i>=1;i--)
					{
						s+=PATH_SLASH;
						s+=aSubPaths[i];

						if (FALSE==CreateDirectoryA((LPCTSTR)s,NULL))
						{
							SetLastError(ChunkFileResult_CannotCreateDirectory);
							break;
						}
					}
					if (i>=1)
						continue;//Fail

					s+=PATH_SLASH;
					s+=aSubPaths[0];//The package file name
					
					//Now try to create a package here
					CMpqFileList *pMpqFileList;
					pMpqFileList=GetMpqFileListFromMpqArchive(s);
					MPQHANDLE hMPQArchive;
					hMPQArchive=MpqOpenArchiveForUpdate((LPCTSTR)s,MOAU_CREATE_NEW,packagesize);
					if (!IS_MPQHANDLE_VALID(hMPQArchive))
					{
						//Fail
						SetLastError(ChunkFileResult_CannotCreatePackage);

						if (pMpqFileList)
							delete pMpqFileList;
						continue;
					} 

					CChunkFolder *pChunkFolder;

					pChunkFolder=new CChunkFolder;
					pChunkFolder->InitMPQFolder(hMPQArchive,pMpqFileList,s,CString(""),ChunkAccessMode_Write);//change to Write after creating
					return pChunkFolder;
				}
			}
		}
		else
		{
			//it's a file

			//Make the sub path IN the archive
			CString s;
			int i;
			for (i=aSubPaths.GetSize()-1;i>=0;i--)
			{
				if (i!=aSubPaths.GetSize()-1)
					s+=PATH_SLASH;
				s+=aSubPaths[i];
			}

			if (modeOpen==ChunkAccessMode_Read)
			{
				//check whether it's a package file
				CMpqFileList *pMpqFileList;
				pMpqFileList=NULL;

				MPQHANDLE hMPQArchive;
				hMPQArchive=NULL;

				if(CheckMPQFormat(buffer))
				{
					pMpqFileList=GetMpqFileListFromMpqArchive(CString(buffer));
					hMPQArchive=OpenMpqHandleForRead(buffer);
				}

				if (!IS_MPQHANDLE_VALID(hMPQArchive))
				{
					if (pMpqFileList)
						delete pMpqFileList;
					SetLastError(ChunkFileResult_CannotOpenPackage);
					continue;
				}

				CChunkFolder *pChunkFolder;

				pChunkFolder=new CChunkFolder;
				pChunkFolder->InitMPQFolder(hMPQArchive,pMpqFileList,CString(buffer),s,modeOpen);
				return pChunkFolder;
			}

			if ((modeOpen==ChunkAccessMode_Write)||
				(modeOpen==ChunkAccessMode_WritePackage))
			{
				//check whether it's a package file
				CMpqFileList *pMpqFileList;
				pMpqFileList=NULL;

				MPQHANDLE hMPQArchive;
				hMPQArchive=NULL;
				if(CheckMPQFormat(buffer))
				{
					pMpqFileList=GetMpqFileListFromMpqArchive(CString(buffer));
					hMPQArchive=MpqOpenArchiveForUpdate(buffer,MOAU_OPEN_EXISTING,packagesize);
				}
				if (!IS_MPQHANDLE_VALID(hMPQArchive))
				{
					//Fail
					if (pMpqFileList)
						delete pMpqFileList;
					SetLastError(ChunkFileResult_CannotOpenPackage);
					continue;
				} 

				CChunkFolder *pChunkFolder;

				pChunkFolder=new CChunkFolder;
				pChunkFolder->InitMPQFolder(hMPQArchive,pMpqFileList,CString(buffer),s,ChunkAccessMode_Write);
				return pChunkFolder;
			}
		}
	}

	return NULL;
}

BOOL RemoveChunk0(char *buffer)
{
	CString path;
	path=buffer;
	CStringArray aSubPaths;
	DWORD attrFile;
	while(1)
	{
		attrFile=GetFileAttributes(buffer);

		char temp[MAX_PATH];

		if (attrFile!=INVALID_FILE_ATTRIBUTES)
			break;


		if (CutTailSubPath(buffer,temp)<=0)
			break;

		aSubPaths.Add(CString(temp));
	}

	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastError(ChunkFileResult_InvalidPath);
		return FALSE;//for the next search path
	}

	if (TRUE)
	{
		if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
		{
			SetLastError(ChunkFileResult_InvalidPath);
			return FALSE;
		}
		//It's a file,check whether it's a package
		CMpqFileList *pMpqFileList;
		pMpqFileList=NULL;
		MPQHANDLE hMPQArchive;
		hMPQArchive=NULL;
		if (CheckMPQFormat(buffer))
		{
			pMpqFileList=GetMpqFileListFromMpqArchive(CString(buffer));
			hMPQArchive=MpqOpenArchiveForUpdate(buffer,MOAU_OPEN_EXISTING,MAX_PACKAGE_FILES);
		}
		if(!IS_MPQHANDLE_VALID(hMPQArchive))
		{
			//A normal file
			if (aSubPaths.GetSize()>0)//the sub path could not be resolved
			{
				SetLastError(ChunkFileResult_InvalidPath);
				return FALSE;
			}

			if (FALSE==DeleteFile(buffer))
				return FALSE;

			return TRUE;
		}
		else
		{
			//it's a mpq package
			if (aSubPaths.GetSize()<=0)//a package could not be a chunk (to remove)
			{
				SetLastError(ChunkFileResult_InvalidPath);
				MpqCloseUpdatedArchive(hMPQArchive,0);
				return FALSE;
			}

			//make the sub path
			CString s;
			int i;
			for (i=aSubPaths.GetSize()-1;i>=0;i--)
			{
				if (i!=aSubPaths.GetSize()-1)
					s+=PATH_SLASH;
				s+=aSubPaths[i];
			}

			if (FALSE==MpqDeleteFile(hMPQArchive,(LPCTSTR)s))
			{
				MpqCloseUpdatedArchive(hMPQArchive,0);
				return FALSE;
			}

			if (pMpqFileList)
			{
				//Now update the mpq file list
				pMpqFileList->RemoveFileRecord((LPSTR)(LPCTSTR)s);

				if (TRUE)//Save to the archive
				{
					CChunk temp;
					CString s;
					s=buffer;
					s+=PATH_SLASH;
					s+=FILELISTPATH;
					temp.InitMPQFile(hMPQArchive,NULL,CString(FILELISTPATH),s,TRUE);//transfer NULL to avoid recursively call

					pMpqFileList->WriteToChunk(&temp);
					temp.Close();
				}

				delete pMpqFileList;

			}

			MpqCloseUpdatedArchive(hMPQArchive,0);
			return TRUE;
		}
	}


	return FALSE;
}

BOOL OpenChunk0(char *buffer,ChunkAccessMode modeOpen,CChunk *pChunk)
{
	CString path;
	path=buffer;
	CStringArray aSubPaths;
	DWORD attrFile;
	while(1)
	{
		attrFile=GetFileAttributes(buffer);

		char temp[MAX_PATH];

		if (attrFile!=INVALID_FILE_ATTRIBUTES)
			break;


		if (CutTailSubPath(buffer,temp)<=0)
			break;

		aSubPaths.Add(CString(temp));
	}

	if (attrFile==INVALID_FILE_ATTRIBUTES)
	{
		SetLastError(ChunkFileResult_InvalidPath);
		return FALSE;//for the next search path
	}

	if (modeOpen==ChunkAccessMode_Read)
	{
		if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
		{
			SetLastError(ChunkFileResult_InvalidPath);
			return FALSE;
		}
		//It's a file,check whether it's a package
		MPQHANDLE hMPQArchive;
		hMPQArchive=OpenMpqHandleForRead(buffer);
		if(!IS_MPQHANDLE_VALID(hMPQArchive))
		{
			if (aSubPaths.GetSize()>0)//the sub path could not be resolved
			{
				SetLastError(ChunkFileResult_InvalidPath);
				return FALSE;
			}

			OFSTRUCT os;
			os.cBytes=sizeof(os);
			HFILE hFile;
			hFile=OpenFile(buffer,&os,OF_READ);

			if (hFile==HFILE_ERROR)
			{
				SetLastError(ChunkFileResult_InvalidPath);
				return FALSE;
			}

			pChunk->InitFile(hFile,ChunkAccessMode_Read,path);
			return TRUE;
		}
		else
		{
			//it's a mpq package
			if (aSubPaths.GetSize()<=0)//a package could not be a chunk
			{
				SetLastError(ChunkFileResult_InvalidPath);
				CloseMpqHandleForRead(hMPQArchive);
				return FALSE;
			}

			//make the sub path
			CString s;
			int i;
			for (i=aSubPaths.GetSize()-1;i>=0;i--)
			{
				if (i!=aSubPaths.GetSize()-1)
					s+=PATH_SLASH;
				s+=aSubPaths[i];
			}

			MPQHANDLE hMPQFile;
			if (FALSE==SFileOpenFileEx(hMPQArchive,(LPCTSTR)s,0,&hMPQFile))
			{
				//Cannot find the file in the archive
				SetLastError(ChunkFileResult_InvalidPath);
				CloseMpqHandleForRead(hMPQArchive);
				return FALSE;
			}
			pChunk->InitMPQFile(hMPQArchive,hMPQFile,path);

			return TRUE;

		}
	}

	if (modeOpen==ChunkAccessMode_Write)
	{
		if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (aSubPaths.GetSize()<=0)//Cannot write to a directory
			{
				SetLastError(ChunkFileResult_InvalidPath);
				return FALSE;
			}

			CString s;
			s=buffer;
			int i;
			for (i=aSubPaths.GetSize()-1;i>=1;i--)
			{
				s+=PATH_SLASH;
				s+=aSubPaths[i];

				if (FALSE==CreateDirectoryA((LPCTSTR)s,NULL))
				{
					SetLastError(ChunkFileResult_CannotCreateDirectory);
					break;
				}
			}
			if (i>=1)
				return FALSE;

			s+=PATH_SLASH;
			s+=aSubPaths[0];//The file name

			OFSTRUCT os;
			os.cBytes=sizeof(os);
			HFILE hFile;
			hFile=OpenFile((LPSTR)(LPCTSTR)s,&os,OF_WRITE|OF_CREATE);

			if (hFile==HFILE_ERROR)
			{
				SetLastError(ChunkFileResult_InvalidPath);
				return FALSE;
			}

			pChunk->InitFile(hFile,ChunkAccessMode_Write,path);

			return TRUE;
		}
		//It's a file,check whether it's a package
		CMpqFileList *pMpqFileList;
		pMpqFileList=NULL;

		MPQHANDLE hMPQArchive;
		hMPQArchive=NULL;
		if (CheckMPQFormat(buffer))
		{
			pMpqFileList=GetMpqFileListFromMpqArchive(CString(buffer));
			hMPQArchive=MpqOpenArchiveForUpdate(buffer,MOAU_OPEN_EXISTING,MAX_PACKAGE_FILES);
		}

		if(!IS_MPQHANDLE_VALID(hMPQArchive))
		{
			if (pMpqFileList)
				delete pMpqFileList;
			if (aSubPaths.GetSize()>0)//the sub path could not be resolved
			{
				SetLastError(ChunkFileResult_InvalidPath);
				return FALSE;
			}

			OFSTRUCT os;
			os.cBytes=sizeof(os);
			HFILE hFile;
			hFile=OpenFile(buffer,&os,OF_WRITE);

			if (hFile==HFILE_ERROR)
			{
				SetLastError(ChunkFileResult_InvalidPath);
				return FALSE;
			}

			pChunk->InitFile(hFile,ChunkAccessMode_Write,path);

			return TRUE;
		}
		else
		{
			//it's a mpq package
			if (aSubPaths.GetSize()<=0)//a package could not be a chunk
			{
				SetLastError(ChunkFileResult_InvalidPath);
				MpqCloseUpdatedArchive(hMPQArchive,0);
				if (pMpqFileList)
					delete pMpqFileList;
				return FALSE;
			}

			//make the sub path
			CString s;
			int i;
			for (i=aSubPaths.GetSize()-1;i>=0;i--)
			{
				if (i!=aSubPaths.GetSize()-1)
					s+=PATH_SLASH;
				s+=aSubPaths[i];
			}

			pChunk->InitMPQFile(hMPQArchive,pMpqFileList,s,path);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CChunkFileSys::RemoveDirectoryChunkFolder(const char *path)
{
	if (!CheckInputPathValidity((char *)path))
	{
		SetLastError(ChunkFileResult_InvalidPath);
		return NULL;
	}

	if (m_aSearchPaths.GetSize()<=0)
	{
		SetLastError(ChunkFileResult_NoSearchPath);
		return NULL;
	}

	int i;
	for (i=0;i<1;i++)
	{
		char buffer[1024];
		int nLen;
		nLen=m_aSearchPaths[i].GetLength();
		if (nLen>0)
		{
			memcpy(buffer,(LPCTSTR)m_aSearchPaths[i],nLen);
			buffer[nLen]=PATH_SLASH_C;
			nLen++;
		}

		char *p;
		p=(char*)path;

		BOOL bTooLongPath;
		bTooLongPath=FALSE;
		while(*p)
		{
			if (nLen>=sizeof(buffer)-1)
			{
				SetLastError(ChunkFileResult_TooLongPath);
				bTooLongPath=TRUE;
				break;
			}
			buffer[nLen]=*p;
			nLen++;
			p++;
		}

		if (bTooLongPath)
			continue;

		buffer[nLen]=0;//terminator
		buffer[nLen+1]=0;//terminator


		CStringArray aSubPaths;
		DWORD attrFile;
		while(1)
		{
			attrFile=GetFileAttributes(buffer);

			char temp[MAX_PATH];

			if (attrFile!=INVALID_FILE_ATTRIBUTES)
				break;


			if (CutTailSubPath(buffer,temp)<=0)
				break;

			aSubPaths.Add(CString(temp));
		}

		if (attrFile==INVALID_FILE_ATTRIBUTES)
		{
			SetLastError(ChunkFileResult_InvalidPath);
			continue;//for the next search path
		}

		if (attrFile&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (aSubPaths.GetSize()>0)//Sub path could not be resolved,
			{
				//for reading,we should fail here
				SetLastError(ChunkFileResult_InvalidPath);
				continue;//for the next search path
			}

			//Now remove the directory
			TCHAR szzTo[] = { '\0', '\0' };

			SHFILEOPSTRUCT shFileOp;
			memset(&shFileOp,0,sizeof(SHFILEOPSTRUCT));
			shFileOp.hwnd                  = NULL;
			shFileOp.wFunc                 = FO_DELETE;
			shFileOp.pFrom                 = buffer;
			shFileOp.pTo                   = szzTo;
			shFileOp.fFlags                = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
			//shFileOp.fAnyOperationsAborted = FALSE;
			//shFileOp.hNameMappings         = NULL;
			//shFileOp.lpszProgressTitle     = 0;
			if (::SHFileOperation(&shFileOp) != 0 )
				continue;

			return TRUE;
		}
	}

	SetLastError(ChunkFileResult_CannotRemoveDirectory);
	return FALSE;

}


BOOL CChunkFileSys::RemoveChunk(const char *path)
{
	if (!CheckInputPathValidity((char*)path))
	{
		SetLastError(ChunkFileResult_InvalidPath);
		return FALSE;
	}

	if (m_aSearchPaths.GetSize()<=0)
	{
		SetLastError(ChunkFileResult_NoSearchPath);
		return FALSE;
	}

	int i;
	for (i=0;i<1;i++)
	{
		char buffer[1024];
		int nLen;
		nLen=m_aSearchPaths[i].GetLength();
		if (nLen>0)
		{
			memcpy(buffer,(LPCTSTR)m_aSearchPaths[i],nLen);
			buffer[nLen]='\\';
			nLen++;
		}

		char *p;
		p=(char*)path;

		BOOL bTooLongPath;
		bTooLongPath=FALSE;
		while(*p)
		{
			if (nLen>=sizeof(buffer)-1)
			{
				SetLastError(ChunkFileResult_TooLongPath);
				bTooLongPath=TRUE;
				break;
			}
			buffer[nLen]=*p;
			nLen++;
			p++;
		}

		if (bTooLongPath)
			continue;

		buffer[nLen]=0;//terminator

		if (FALSE==RemoveChunk0(buffer))
			continue;

		return TRUE;
	}

	return FALSE;
}


CChunk *CChunkFileSys::OpenChunk(const char *path,ChunkAccessMode modeOpen)
{
	if (!CheckInputPathValidity((char*)path))
	{
		SetLastError(ChunkFileResult_InvalidPath);
		return NULL;
	}

	if (m_aSearchPaths.GetSize()<=0)
	{
		SetLastError(ChunkFileResult_NoSearchPath);
		return NULL;
	}

	if ((modeOpen!=ChunkAccessMode_Read)&&(modeOpen!=ChunkAccessMode_Write))
	{
		SetLastError(ChunkFileResult_InvalidOpenMode);
		return NULL;
	}

	int i;
	for (i=0;i<1;i++)
	{
		char buffer[1024];
		int nLen;
		nLen=m_aSearchPaths[i].GetLength();
		if (nLen>0)
		{
			memcpy(buffer,(LPCTSTR)m_aSearchPaths[i],nLen);
			buffer[nLen]='\\';
			nLen++;
		}

		char *p;
		p=(char*)path;

		BOOL bTooLongPath;
		bTooLongPath=FALSE;
		while(*p)
		{
			if (nLen>=sizeof(buffer)-1)
			{
				SetLastError(ChunkFileResult_TooLongPath);
				bTooLongPath=TRUE;
				break;
			}
			buffer[nLen]=*p;
			nLen++;
			p++;
		}

		if (bTooLongPath)
			continue;

		buffer[nLen]=0;//terminator

		CChunk *pChunk;
		pChunk=new CChunk;
		if (FALSE==OpenChunk0(buffer,modeOpen,pChunk))
		{
			delete pChunk;
			continue;
		}
		return pChunk;
	}

	return NULL;
}
