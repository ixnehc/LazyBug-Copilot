#include "stdafx.h"
#include ".\chunkfilesys.h"

#include "..\timer\timer.h"

#include "MpqFileList.h"


CChunk::CChunk(void)
{
	m_type=ChunkType_None;

	m_bIgnoreWarning=FALSE;
}

CChunk::~CChunk(void)
{
	Close();
}

BOOL CChunk::IsReading()//Read ?
{
	return m_mode==ChunkAccessMode_Read;
}
BOOL CChunk::IsWriting()//Writing?
{
	return m_mode==ChunkAccessMode_Write;

}


void CChunk::Close()
{
	if (m_type==ChunkType_None)
		return;

	if (m_type==ChunkType_File)
	{
		if (m_hFile)
			CloseHandle((HANDLE)m_hFile);
		m_hFile=NULL;
	}
	if (m_type==ChunkType_MPQFile)
	{
		if (m_mode==ChunkAccessMode_Read)
		{
			if (m_hMPQFile)
				SFileCloseFile(m_hMPQFile);
			m_hMPQFile=NULL;

			if (!m_bArchiveOwnedByOthers)
			{
				if (m_hMPQArchive)
					CloseMpqHandleForRead(m_hMPQArchive);
			}
			m_hMPQArchive=NULL;
		}
		if(m_mode==ChunkAccessMode_Write)
		{
//			if (m_vecWriteBuffer.size()>0)
			{
				if (m_vecWriteBuffer.size()<=0)
					m_vecWriteBuffer.push_back(0);//Cannot add a zero size buffer to the archive,so we add a single byte here.
				if (FALSE==MpqAddFileFromBufferEx(m_hMPQArchive,&m_vecWriteBuffer[0],m_vecWriteBuffer.size(),(LPCTSTR)m_pathSub,MAFA_COMPRESS|MAFA_REPLACE_EXISTING,MAFA_COMPRESS_STANDARD,0))
				{
					if (TRUE)//Remove the original file,since we could not know whether the original file has been deleted after this failed call 
					{
						MpqDeleteFile(m_hMPQArchive,(LPCTSTR)m_pathSub);

						if (m_pMpqFileList)
						{
							m_pMpqFileList->RemoveFileRecord((LPSTR)(LPCTSTR)m_pathSub);

							if (TRUE)//Save to the archive
							{
								CChunk temp;
								CString s;
								s=m_pathTotal;
								s=s.Left(s.GetLength()-m_pathSub.GetLength());
								s+=FILELISTPATH;
								temp.InitMPQFile(m_hMPQArchive,NULL,CString(FILELISTPATH),s,TRUE);//transfer NULL to avoid recursively call

								m_pMpqFileList->WriteToChunk(&temp);

								temp.Close();
							}

						}

					}
					if (!m_bIgnoreWarning)
					{
						CString s;
						s="Chunk Write:Failed to add chunk data to package:\"";
						s+=m_pathTotal;
						s+="\"!";

						AfxMessageBox(s);
					}
				}
				else
				{
					//Successfully added.We should update the filelist here
					if (m_pMpqFileList)
					{
						CMpqFileRecord record;
						record.m_filename=m_pathSub;
						record.m_timeLatestModified=m_timeModified;

						m_pMpqFileList->AddFileRecord(record);

						if (TRUE)//Save to the archive
						{
							CChunk temp;
							CString s;
							s=m_pathTotal;
							s=s.Left(s.GetLength()-m_pathSub.GetLength());
							s+=FILELISTPATH;
							temp.InitMPQFile(m_hMPQArchive,NULL,CString(FILELISTPATH),s,TRUE);//transfer NULL to avoid recursively call

							m_pMpqFileList->WriteToChunk(&temp);

							temp.Close();
						}

					}
				}
			}
			if (!m_bArchiveOwnedByOthers)
			{
				if (m_hMPQArchive)
				{
					if (m_pMpqFileList)
					{
						delete m_pMpqFileList;

						m_pMpqFileList=NULL;
					}

					MpqCloseUpdatedArchive(m_hMPQArchive,0);
				}
			}
			m_hMPQArchive=NULL;

			m_hMPQFile=NULL;
		}
	}

	m_vecWriteBuffer.clear();
	m_vecReadBuffer.clear();
	m_iReadIndex=-1;
	m_bIgnoreWarning=FALSE;
	m_pathTotal="";
	m_pathSub="";

	m_type=ChunkType_None;

}



void CChunk::SetIgnoreWarning(BOOL bIgnoreWarning)
{
	m_bIgnoreWarning=bIgnoreWarning;
}

void CChunk::InitFile(HFILE hFile,ChunkAccessMode mode,CString &pathTotal)
{
	if (m_type!=ChunkType_None)
		Close();
	m_mode=mode;
	m_type=ChunkType_File;

	m_hFile=hFile;

	m_pathTotal=pathTotal;

	m_vecWriteBuffer.clear();
	m_vecReadBuffer.clear();

	m_iReadIndex=-1;
}

void CChunk::InitMPQFile(MPQHANDLE hMPQArchive,MPQHANDLE hMPQFile,CString &pathTotal,BOOL bArchiveOwnedByOthers)
{
	if (m_type!=ChunkType_None)
		Close();
	m_mode=ChunkAccessMode_Read;
	m_type=ChunkType_MPQFile;

	m_hMPQArchive=hMPQArchive;
	m_hMPQFile=hMPQFile;
	m_bArchiveOwnedByOthers=bArchiveOwnedByOthers;


	m_pathTotal=pathTotal;

	m_vecWriteBuffer.clear();
	m_vecReadBuffer.clear();

	m_iReadIndex=-1;
}
void CChunk::InitMPQFile(MPQHANDLE hMPQArchive,CMpqFileList *pMpqFileList,CString &pathSub,CString &pathTotal,BOOL bArchiveOwnedByOthers)
{
	if (m_type!=ChunkType_None)
		Close();
	m_mode=ChunkAccessMode_Write;
	m_type=ChunkType_MPQFile;

	m_hMPQArchive=hMPQArchive;
	m_pMpqFileList=pMpqFileList;
	m_hMPQFile=NULL;
	m_bArchiveOwnedByOthers=bArchiveOwnedByOthers;

	m_pathSub=pathSub;

	m_pathTotal=pathTotal;

	m_timeModified=GetCurFileTime();

	m_vecWriteBuffer.clear();
	m_vecReadBuffer.clear();

	m_iReadIndex=-1;
}

void CChunk::WriteInternal(void *buffer,UINT size)
{
	if (size<=0)
		return;
	DWORD dwWritten;
	if (ChunkType_File==m_type)
	{
		if (FALSE==WriteFile((HANDLE)m_hFile,buffer,size,&dwWritten,NULL))
			dwWritten=0;
	}
	if (ChunkType_MPQFile==m_type)
	{
		int sz;
		sz=m_vecWriteBuffer.size();
		m_vecWriteBuffer.resize(sz+size);
		memcpy(&m_vecWriteBuffer[sz],buffer,size);

		m_timeModified=GetCurFileTime();
		return;
	}

	if (dwWritten<size)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Chunk Write:Failed to write data to chunk :\"";
			s+=m_pathTotal;
			s+="\"!";

			AfxMessageBox(s);
		}
	}


}
void CChunk::ReadInternal(void *buffer,UINT size)
{
	if (size<=0)
		return;
	memset(buffer,0,size);
	DWORD dwRead;
	dwRead=0;
	if (ChunkType_File==m_type)
	{
		if (FALSE==ReadFile((HANDLE)m_hFile,buffer,size,&dwRead,NULL))
			dwRead=0;
	}
	if (ChunkType_MPQFile==m_type)
	{
		if (m_iReadIndex==-1)
		{
			DWORD sz;
			sz=SFileGetFileSize(m_hMPQFile,NULL);

			m_vecReadBuffer.resize(sz);

			DWORD dwRead;
			if (FALSE==SFileReadFile(m_hMPQFile,&m_vecReadBuffer[0],sz,&dwRead,NULL))
				dwRead=0;

			if (dwRead!=m_vecReadBuffer.size())
				m_vecReadBuffer.resize(dwRead);

			m_iReadIndex=0;

		}

		if (TRUE)//Copy  the data to the buffer
		{
			if (m_iReadIndex+size>m_vecReadBuffer.size())
				size-=m_iReadIndex+size-m_vecReadBuffer.size();
			if (size>0)
			{
				memcpy(buffer,&m_vecReadBuffer[m_iReadIndex],size);
				m_iReadIndex+=size;
			}
			dwRead=size;
		}
	}

	if (dwRead<size)
	{
		memset(buffer,0,size);//Clear as all zero
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Chunk Read:Failed to read data from chunk :\"";
			s+=m_pathTotal;
			s+="\"!";

			AfxMessageBox(s);
		}
	}
}

int CChunk::GetSize()//Only valid when reading,otherwise return -1
{
	if (m_mode!=ChunkAccessMode_Read)
		return -1;

	if (ChunkType_File==m_type)
		return GetFileSize((HANDLE)m_hFile,NULL);

	if (ChunkType_MPQFile==m_type)
	{
		if (m_iReadIndex!=-1)
			return m_vecReadBuffer.size();
		DWORD sz;
		sz=SFileGetFileSize(m_hMPQFile,NULL);

		return sz;
	}

	return -1;

}



#define GENERIC_VALUEWRITE_CODE \
	if (m_type==ChunkType_None)\
		return *this;\
	if (m_mode!=ChunkAccessMode_Write)\
		return *this;\
	WriteInternal(&value,sizeof(value));\
		return *this;

#define GENERIC_VALUEREAD_CODE \
	if (m_type==ChunkType_None)\
		return *this;\
	if (m_mode!=ChunkAccessMode_Read)\
		return *this;\
	ReadInternal(&value,sizeof(value));\
		return *this;

CChunk& CChunk::operator<<(BYTE value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(WORD value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(LONG value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(DWORD value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(float value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(double value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(LONGLONG value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(ULONGLONG value)
{
	GENERIC_VALUEWRITE_CODE;
}

CChunk& CChunk::operator<<(int value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(short value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(char value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(unsigned value)
{
	GENERIC_VALUEWRITE_CODE;
}
CChunk& CChunk::operator<<(bool value)
{
	GENERIC_VALUEWRITE_CODE;
}

CChunk& CChunk::operator<<(CString &value)
{
	if (m_type==ChunkType_None)
		return *this;
	if (m_mode!=ChunkAccessMode_Write)
		return *this;
	int sz;
	sz=value.GetLength();
	WriteInternal(&sz,sizeof(sz));
	WriteInternal((void*)(LPCTSTR)value,sz);
	return *this;
}


// extraction operations
CChunk& CChunk::operator>>(BYTE& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(WORD& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(DWORD& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(LONG& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(float& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(double& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(LONGLONG& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(ULONGLONG& value)
{
	GENERIC_VALUEREAD_CODE;
}

CChunk& CChunk::operator>>(int& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(short& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(char& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(unsigned& value)
{
	GENERIC_VALUEREAD_CODE;
}
CChunk& CChunk::operator>>(bool& value)
{
	GENERIC_VALUEREAD_CODE;
}

CChunk& CChunk::operator>>(CString &value)
{
	if (m_type==ChunkType_None)
		return *this;
	if (m_mode!=ChunkAccessMode_Read)
		return *this;
	int sz;
	ReadInternal(&sz,sizeof(sz));

	if (sz<0)
		sz=0;

	value.GetBuffer(sz+1);
	ReadInternal((void*)(LPCTSTR)value,sz);
	((LPSTR)(LPCTSTR)value)[sz]=0;
	value.ReleaseBuffer();
	return *this;
}



void CChunk::Read(void* lpBuf, UINT nMax)
{
	if (m_type==ChunkType_None)
		return;
	if (m_mode!=ChunkAccessMode_Read)
		return;
	ReadInternal(lpBuf,nMax);
	return;

}
void CChunk::Write(void* lpBuf, UINT nMax)
{
	if (m_type==ChunkType_None)
		return;
	if (m_mode!=ChunkAccessMode_Write)
		return;
	WriteInternal(lpBuf,nMax);
	return;
}





CChunkFolder::CChunkFolder(void)
{
	m_type=ChunkFolderType_None;
	m_bIgnoreWarning=FALSE;

	m_pSeekChunk=NULL;
	m_bChunkInfoDirty=TRUE;
}
CChunkFolder::~CChunkFolder(void)
{
	Close();
}

void CChunkFolder::Close()
{
	if (m_type==ChunkFolderType_None)
		return;
	CloseSeekedChunk();

	if (m_type==ChunkFolderType_MPQFolder)
	{
		if (m_mode==ChunkAccessMode_Read)
		{
			if (m_hMPQArchive)
				CloseMpqHandleForRead(m_hMPQArchive);
		}
		else
		{
			if (m_hMPQArchive)
			{
				if (FALSE)//File list updated when the seeked chunk is closed,so we need not update here
				if (m_pMpqFileList)
				{
					CChunk temp;
					CString s;
					s=m_pathMPQFolder;
					s+=FILELISTPATH;
					temp.InitMPQFile(m_hMPQArchive,m_pMpqFileList,CString(FILELISTPATH),s,TRUE);

					m_pMpqFileList->WriteToChunk(&temp);

					temp.Close();

					delete m_pMpqFileList;

					m_pMpqFileList=NULL;
				}
				MpqCloseUpdatedArchive(m_hMPQArchive,0);
			}
			if (m_pMpqFileList)
				delete m_pMpqFileList;
			m_pMpqFileList=NULL;
		}
		m_hMPQArchive=NULL;
	}

	m_type=ChunkFolderType_None;
	m_bIgnoreWarning=FALSE;
}


void CChunkFolder::SetIgnoreWarning(BOOL bIgnoreWarning)
{
	m_bIgnoreWarning=bIgnoreWarning;
}

void CChunkFolder::InitFolder(CString &pathFolder,ChunkAccessMode mode)
{
	if (m_type!=ChunkFolderType_None)
		Close();
	m_type=ChunkFolderType_Folder;
	m_mode=mode;

	m_pathFolder=pathFolder;

	m_bChunkInfoDirty=TRUE;

	m_bIgnoreWarning=FALSE;
}
void CChunkFolder::InitMPQFolder(MPQHANDLE hMPQ,CMpqFileList *pMpqFileList,CString &pathMPQFolder,CString &pathSub,ChunkAccessMode mode)
{
	if (m_type!=ChunkFolderType_None)
		Close();
	m_type=ChunkFolderType_MPQFolder;
	m_mode=mode;

	m_hMPQArchive=hMPQ;
	m_pMpqFileList=pMpqFileList;
	m_pathMPQFolder=pathMPQFolder;
	m_pathSub=pathSub;

	m_bChunkInfoDirty=TRUE;

	m_bIgnoreWarning=FALSE;
}


extern BOOL OpenChunk0(char *path,ChunkAccessMode modeOpen,CChunk *pChunk);
extern BOOL RemoveChunk0(char *path);

void CChunkFolder::CloseSeekedChunk()
{
	if (m_pSeekChunk)
	{
		m_pSeekChunk->Close();
		delete m_pSeekChunk;
		m_pSeekChunk=NULL;
	}
}

//if you call this function,no matter it will be successful or not,the original seeked chunk will be closed
BOOL CChunkFolder::SeekChunk(LPCSTR pathSubChunk)
{

	CloseSeekedChunk();

	if (pathSubChunk[0]==0)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Seek Chunk:sub path is empty(\"";
			s+=m_pathFolder;
			s+="\")!";
			AfxMessageBox(s);
		}
		return FALSE;
	}

	if (m_type==ChunkFolderType_Folder)
	{
		CString path;
		path=m_pathFolder;
		path+=PATH_SLASH;
		path+=pathSubChunk;

		CChunk *pChunk;
		pChunk=new CChunk;
		if (FALSE==OpenChunk0((LPSTR)(LPCTSTR)path,m_mode,pChunk))
		{
			delete pChunk;

			if (!m_bIgnoreWarning)
			{
				CString s;
				s="Seek Chunk:Fail to open chunk:\"";
				s+=path;
				s+="\"!";

				AfxMessageBox(s);
			}

			return FALSE;
		}

		pChunk->SetIgnoreWarning(m_bIgnoreWarning);

		m_pSeekChunk=pChunk;

		return TRUE;
	}

	if (m_type==ChunkFolderType_MPQFolder)
	{
		CString path,pathTotal;
		path=m_pathSub;
		if (m_pathSub!="")
			path+=PATH_SLASH;
		path+=pathSubChunk;

		pathTotal=m_pathMPQFolder;
		pathTotal+=PATH_SLASH;
		pathTotal+=path;

		if (m_mode==ChunkAccessMode_Read)
		{
			MPQHANDLE hMPQFile;
			if (FALSE==SFileOpenFileEx(m_hMPQArchive,(LPCTSTR)path,0,&hMPQFile))
			{
				if (!m_bIgnoreWarning)
				{
					CString s;
					s="Seek Chunk:Failed to open chunk:\"";
					s+=pathTotal;
					s+="\"!";

					AfxMessageBox(s);
				}

				return FALSE;
			}

			CChunk *pChunk;
			pChunk=new CChunk;

			pChunk->InitMPQFile(m_hMPQArchive,hMPQFile,pathTotal,TRUE);

			pChunk->SetIgnoreWarning(m_bIgnoreWarning);
			m_pSeekChunk=pChunk;

			return TRUE;
		}
		if (m_mode==ChunkAccessMode_Write)
		{
			CChunk *pChunk;
			pChunk=new CChunk;

			pChunk->InitMPQFile(m_hMPQArchive,m_pMpqFileList,path,pathTotal,TRUE);

			pChunk->SetIgnoreWarning(m_bIgnoreWarning);
			m_pSeekChunk=pChunk;

			return TRUE;
		}
	}

	if (!m_bIgnoreWarning)
	{
		CString s;
		s="Seek Chunk:Chunk folder not initialized yet!";

		AfxMessageBox(s);
	}
	return FALSE;
}

BOOL CChunkFolder::PreWriteTest()
{
	if (m_type==ChunkFolderType_None)
	{
		if (!m_bIgnoreWarning)
			AfxMessageBox("Chunk Folder Write:Chunk folder not initialized yet!");

		return FALSE;
	}
	CString pathFull;
	if (m_type==ChunkFolderType_Folder)
		pathFull=m_pathFolder;
	else
	{
		pathFull=m_pathMPQFolder;
		if (m_pathSub!="")
		{
			pathFull+=PATH_SLASH;
			pathFull+=m_pathSub;
		}
	}
	if (m_mode!=ChunkAccessMode_Write)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Chunk Folder Write:not opened for writing:\"";
			s+=pathFull;
			s+="\"!";

			AfxMessageBox(s);
		}

		return FALSE;
	}
	if (!m_pSeekChunk)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Chunk Folder Write:not seek any chunk yet:\"";
			s+=pathFull;
			s+="\"!";

			AfxMessageBox(s);
		}

		return FALSE;
	}
	return TRUE;
}
BOOL CChunkFolder::PreReadTest()
{
	if (m_type==ChunkFolderType_None)
	{
		if (!m_bIgnoreWarning)
			AfxMessageBox("Chunk Folder Read:Chunk folder not initialized yet!");

		return FALSE;
	}
	CString pathFull;
	if (m_type==ChunkFolderType_Folder)
		pathFull=m_pathFolder;
	else
	{
		pathFull=m_pathMPQFolder;
		if (m_pathSub!="")
		{
			pathFull+=PATH_SLASH;
			pathFull+=m_pathSub;
		}
	}
	if (m_mode!=ChunkAccessMode_Read)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Chunk Folder Read:not opened for reading:\"";
			s+=pathFull;
			s+="\"!";

			AfxMessageBox(s);
		}

		return FALSE;
	}
	if (!m_pSeekChunk)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Chunk Folder Read:not seek any chunk yet:\"";
			s+=pathFull;
			s+="\"!";

			AfxMessageBox(s);
		}

		return FALSE;
	}
	return TRUE;
}



#define GENERIC_FOLDERWRITE_CODE \
	if (PreWriteTest())\
		(*m_pSeekChunk)<<(value);\
	return *this;

#define GENERIC_FOLDERREAD_CODE \
	if (PreReadTest())\
		(*m_pSeekChunk)>>(value);\
	return *this;


CChunkFolder& CChunkFolder::operator<<(BYTE value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(WORD value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(LONG value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(DWORD value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(float value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(double value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(LONGLONG value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(ULONGLONG value)
{
	GENERIC_FOLDERWRITE_CODE;
}

CChunkFolder& CChunkFolder::operator<<(int value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(short value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(char value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(unsigned value)
{
	GENERIC_FOLDERWRITE_CODE;
}
CChunkFolder& CChunkFolder::operator<<(bool value)
{
	GENERIC_FOLDERWRITE_CODE;
}

CChunkFolder& CChunkFolder::operator<<(CString &value)
{
	GENERIC_FOLDERWRITE_CODE;
}


// extraction operations
CChunkFolder& CChunkFolder::operator>>(BYTE &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(WORD &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(LONG &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(DWORD &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(float &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(double &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(LONGLONG &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(ULONGLONG &value)
{
	GENERIC_FOLDERREAD_CODE;
}

CChunkFolder& CChunkFolder::operator>>(int &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(short &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(char &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(unsigned &value)
{
	GENERIC_FOLDERREAD_CODE;
}
CChunkFolder& CChunkFolder::operator>>(bool &value)
{
	GENERIC_FOLDERREAD_CODE;
}

CChunkFolder& CChunkFolder::operator>>(CString &value)
{
	GENERIC_FOLDERREAD_CODE;
}


void CChunkFolder::Read(void* lpBuf, UINT nMax)
{
	if (PreReadTest())
		m_pSeekChunk->Read(lpBuf,nMax);
}
void CChunkFolder::Write(void* lpBuf, UINT nMax)
{
	if (PreWriteTest())
		m_pSeekChunk->Write(lpBuf,nMax);
}


BOOL CChunkFolder::IsReading()//Read ?
{
	return m_mode==ChunkAccessMode_Read;
}
BOOL CChunkFolder::IsWriting()//Writing?
{
	return m_mode==ChunkAccessMode_Write;

}

CChunk *CChunkFolder::GetChunk()//call this function after seeking a chunk .
{
	return m_pSeekChunk;
}


int CChunkFolder::GetChunkSize()//valid for reading.And call this function after seeking a chunk .
{
	if (m_mode!=ChunkAccessMode_Read)
		return -1;

	if (!m_pSeekChunk)
		return -1;
	return m_pSeekChunk->GetSize();
}

//This function will automatically seek to the new chunk
BOOL CChunkFolder::NewChunk(LPCSTR pathSubChunk)
{

	CloseSeekedChunk();

	if ((m_mode!=ChunkAccessMode_Write)&&(m_mode!=ChunkAccessMode_WritePackage))
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="New Chunk:Not a writable chunk folder(\"";
			s+=m_pathFolder;
			s+="\")!";
			AfxMessageBox(s);
		}
	}

	if (pathSubChunk[0]==0)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="New Chunk:sub path is empty(\"";
			s+=m_pathFolder;
			s+="\")!";
			AfxMessageBox(s);
		}
		return FALSE;
	}

	if (m_type==ChunkFolderType_Folder)
	{
		CString path;
		path=m_pathFolder;
		path+=PATH_SLASH;
		path+=pathSubChunk;

		CChunk *pChunk;
		pChunk=new CChunk;
		if (FALSE==OpenChunk0((LPSTR)(LPCTSTR)path,m_mode,pChunk))
		{
			delete pChunk;

			if (!m_bIgnoreWarning)
			{
				CString s;
				s="Seek Chunk:Fail to open chunk:\"";
				s+=path;
				s+="\"!";

				AfxMessageBox(s);
			}

			return FALSE;
		}

		pChunk->SetIgnoreWarning(m_bIgnoreWarning);

		m_pSeekChunk=pChunk;


		m_bChunkInfoDirty=TRUE;
		return TRUE;
	}

	if (m_type==ChunkFolderType_MPQFolder)
	{
		CString path,pathTotal;
		path=m_pathSub;
		if (m_pathSub!="")
			path+=PATH_SLASH;
		path+=pathSubChunk;

		pathTotal=m_pathMPQFolder;
		pathTotal+=PATH_SLASH;
		pathTotal+=path;

		if (m_mode==ChunkAccessMode_Write)
		{
			CChunk *pChunk;
			pChunk=new CChunk;

			pChunk->InitMPQFile(m_hMPQArchive,m_pMpqFileList,path,pathTotal,TRUE);

			pChunk->SetIgnoreWarning(m_bIgnoreWarning);
			m_pSeekChunk=pChunk;

			CloseSeekedChunk();//Close to write the mpqfilelist

			m_bChunkInfoDirty=TRUE;
			return SeekChunk(pathSubChunk);
		}
	}

	if (!m_bIgnoreWarning)
	{
		CString s;
		s="New Chunk:Chunk folder not initialized yet!";

		AfxMessageBox(s);
	}
	return FALSE;

}


//This function will close the current seeked chunk,and the chunk folder should be opened with Write mode.
BOOL CChunkFolder::RemoveChunk(LPCSTR pathSubChunk)
{

	CloseSeekedChunk();

	if ((m_mode!=ChunkAccessMode_Write)&&(m_mode!=ChunkAccessMode_WritePackage))
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Remove Chunk:Not a writable chunk folder(\"";
			s+=m_pathFolder;
			s+="\")!";
			AfxMessageBox(s);
		}
	}

	if (pathSubChunk[0]==0)
	{
		if (!m_bIgnoreWarning)
		{
			CString s;
			s="Remove Chunk:sub path is empty(\"";
			s+=m_pathFolder;
			s+="\")!";
			AfxMessageBox(s);
		}
		return FALSE;
	}

	if (m_type==ChunkFolderType_Folder)
	{
		CString path;
		path=m_pathFolder;
		path+=PATH_SLASH;
		path+=pathSubChunk;

		if (FALSE==RemoveChunk0((LPSTR)(LPCTSTR)path))
		{
			if (!m_bIgnoreWarning)
			{
				CString s;
				s="Remove Chunk:Fail to remove chunk:\"";
				s+=path;
				s+="\"!";

				AfxMessageBox(s);
			}

			return FALSE;
		}

		return TRUE;
	}

	if (m_type==ChunkFolderType_MPQFolder)
	{
		CString path,pathTotal;
		path=m_pathSub;
		if (m_pathSub!="")
			path+=PATH_SLASH;
		path+=pathSubChunk;

		pathTotal=m_pathMPQFolder;
		pathTotal+=PATH_SLASH;
		pathTotal+=path;

		if (m_mode==ChunkAccessMode_Write)
		{
			if (FALSE==MpqDeleteFile(m_hMPQArchive,(LPCTSTR)path))
			{
				if (!m_bIgnoreWarning)
				{
					CString s;
					s="Remove Chunk:Failed to remove chunk:\"";
					s+=pathTotal;
					s+="\"!";

					AfxMessageBox(s);
				}

				return FALSE;
			}
			if (m_pMpqFileList)
			{
				m_pMpqFileList->RemoveFileRecord((LPSTR)(LPCTSTR)path);

				if (TRUE)//Save to the archive
				{
					CChunk temp;
					CString s;
					s=m_pathMPQFolder;
					s+=PATH_SLASH;
					s+=FILELISTPATH;
					temp.InitMPQFile(m_hMPQArchive,NULL,CString(FILELISTPATH),s,TRUE);//transfer NULL to avoid recursively call

					m_pMpqFileList->WriteToChunk(&temp);

					temp.Close();
				}

				m_bChunkInfoDirty=TRUE;
			}
			return TRUE;
		}
	}

	if (!m_bIgnoreWarning)
	{
		CString s;
		s="Remove Chunk:Chunk folder not initialized yet!";

		AfxMessageBox(s);
	}
	return FALSE;
}

void CChunkFolder::CollectFolderChunkInfo(CString &pathRoot,CString &pathFolder)
{
	CFileFind ff;
	BOOL bFound;
	CString pathToFind;
	pathToFind=pathFolder+"\\*.*";
	bFound=ff.FindFile((LPCTSTR)pathToFind);
	while (bFound)
	{
		bFound = ff.FindNextFile();

		CString ss;
		ss=ff.GetFilePath();

		if (ff.IsDots())//".", ".."
			continue;

		if (ff.IsDirectory())
		{
			CollectFolderChunkInfo(pathRoot,ss);
			continue;
		}

		CString pathSub;
		pathSub=(ss.Right(ss.GetLength()-pathRoot.GetLength()-1));
//		pathSub.MakeUpper();

		m_aChunkInfo.push_back(std::string((LPCTSTR)pathSub));
	}
}

void CChunkFolder::CollectChunkInfo()
{
	if (!m_bChunkInfoDirty)
		return;
	m_aChunkInfo.clear();
	if (m_type==ChunkFolderType_Folder)
		CollectFolderChunkInfo(m_pathFolder,m_pathFolder);
	else
	{
		if (!m_pMpqFileList)
			return;
		
		CString s;
		CString sss;
		s=m_pathSub+PATH_SLASH;
		int count;
		count=m_pMpqFileList->GetRecordCount();
		int i;
		for (i=0;i<count;i++)
		{
			CMpqFileRecord *p;
			p=m_pMpqFileList->GetRecord(i);
			if (0==p->m_filename.Find((LPCTSTR)s))//Can find and matching from the beginnig
			{
				sss=(p->m_filename.Right(p->m_filename.GetLength()-s.GetLength()));
				sss.MakeUpper();
				m_aChunkInfo.push_back(std::string((LPCTSTR)sss));
			}
		}
	}
	m_bChunkInfoDirty=FALSE;
}

DWORD CChunkFolder::GetChunkCount()
{
	if (m_bChunkInfoDirty)
		CollectChunkInfo();

	return m_aChunkInfo.size();
}
LPSTR CChunkFolder::GetChunkSubPath(DWORD idx)
{
	if (m_bChunkInfoDirty)
		CollectChunkInfo();

	if (idx>=m_aChunkInfo.size())
		return NULL;
	return (LPSTR)(m_aChunkInfo[idx].c_str());
}
BOOL CChunkFolder::CheckChunkNameDupe(LPCSTR pathSubChunk0)
{
	char *pathSubChunk;
	CString sUpperString;
	sUpperString=pathSubChunk0;
	sUpperString.MakeUpper();
	pathSubChunk=(LPSTR)(LPCTSTR)sUpperString;

	if (m_bChunkInfoDirty)
		CollectChunkInfo();

	int i;
	CString ss;
	ss=pathSubChunk;
	ss+=PATH_SLASH;
	for (i=0;i<m_aChunkInfo.size();i++)
	{
		CString s;
		s=m_aChunkInfo[i].c_str();
		s.MakeUpper();
		if (s==pathSubChunk)
			return TRUE;
		if (s.Find((LPSTR)(LPCTSTR)ss)==0)//Already contained in another chunk path
			return TRUE;			
	}

	return FALSE;
}

BOOL CChunkFolder::IsFolder()
{
	return m_type==ChunkFolderType_Folder;
}
BOOL CChunkFolder::IsMpqFolder()
{
	return m_type==ChunkFolderType_MPQFolder;

}

CString CChunkFolder::GetFolderPath()
{
	if (m_type==ChunkFolderType_Folder)
		return m_pathFolder;
	if (m_type==ChunkFolderType_MPQFolder)
		return m_pathMPQFolder;

	return CString("");

}

