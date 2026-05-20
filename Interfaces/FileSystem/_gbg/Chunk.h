#pragma once

#include "SFMPQ_STATIC.h"

#include <vector>
#include <string>

#define RW_BUFFER_SIZE 1024

#define PATH_SLASH "\\"
#define PATH_SLASH_C '\\'

#define FILELISTPATH "[MpqFileListData]" 


enum ChunkAccessMode
{
	ChunkAccessMode_Read=1,
	ChunkAccessMode_Write=2,
	ChunkAccessMode_WritePackage=3,//Only used in OpenChunkFolder()
};

enum ChunkTypeEnum
{
	ChunkType_None,
	ChunkType_File,
	ChunkType_MPQFile,
};

enum ChunkFolderType
{
	ChunkFolderType_None,
	ChunkFolderType_Folder,
	ChunkFolderType_MPQFolder,
};

class CChunkFolder;
class CChunkFileSys;

class CMpqFileList;

class CChunk
{
public:
	CChunk(void);
	~CChunk(void);


	// insertion operations
	CChunk& operator<<(BYTE value);
	CChunk& operator<<(WORD value);
	CChunk& operator<<(LONG value);
	CChunk& operator<<(DWORD value);
	CChunk& operator<<(float value);
	CChunk& operator<<(double value);
	CChunk& operator<<(LONGLONG value);
	CChunk& operator<<(ULONGLONG value);

	CChunk& operator<<(int value);
	CChunk& operator<<(short value);
	CChunk& operator<<(char value);
	CChunk& operator<<(unsigned value);
	CChunk& operator<<(bool value);

	CChunk& operator<<(CString &value);

	// extraction operations
	CChunk& operator>>(BYTE &value);
	CChunk& operator>>(WORD &value);
	CChunk& operator>>(LONG &value);
	CChunk& operator>>(DWORD &value);
	CChunk& operator>>(float &value);
	CChunk& operator>>(double &value);
	CChunk& operator>>(LONGLONG &value);
	CChunk& operator>>(ULONGLONG &value);

	CChunk& operator>>(int &value);
	CChunk& operator>>(short &value);
	CChunk& operator>>(char &value);
	CChunk& operator>>(unsigned &value);
	CChunk& operator>>(bool &value);

	CChunk& operator>>(CString &value);


	void Read(void* lpBuf, UINT nMax);
	void Write(void* lpBuf, UINT nMax);

	void InitFile(HFILE hFile,ChunkAccessMode mode,CString &pathTotal);
	void InitMPQFile(MPQHANDLE hMPQArchive,MPQHANDLE hMPQFile,CString &pathTotal,BOOL bArchiveOwnedByOthers=FALSE);//for read
	void InitMPQFile(MPQHANDLE hMPQArchive,CMpqFileList *pMpqFileList,CString &pathSub,CString &pathTotal,BOOL bArchiveOwnedByOthers=FALSE);//for write

	BOOL IsReading();//Read ?
	BOOL IsWriting();//Writing?

	int GetSize();//Only valid when reading

private:
	void Close();
	ChunkAccessMode m_mode;
	ChunkTypeEnum m_type;

	CString m_pathTotal;

	BOOL m_bIgnoreWarning;//if this flag is set,no prompt dialog will be popped up when an error for reading/writing occurs.
	void SetIgnoreWarning(BOOL bIgnoreWarning=TRUE);


	struct
	{
		struct //ChunkType_File
		{
			HFILE m_hFile;
		};

		struct //ChunkType_MPQFile
		{
			MPQHANDLE m_hMPQArchive;
			CMpqFileList *m_pMpqFileList;//Companying the Archive
			MPQHANDLE m_hMPQFile;//For read
			CString m_pathSub;//For write

			FILETIME m_timeModified;//Only for write

			BOOL m_bArchiveOwnedByOthers;//if true,m_hMPQArchive is owned by others,and should not be closed when the chunk is closed
		};
	};

	std::vector <BYTE>m_vecWriteBuffer;
	std::vector <BYTE>m_vecReadBuffer;
	int m_iReadIndex;

	void WriteInternal(void *buffer,UINT size);
	void ReadInternal(void *buffer,UINT size);

	friend class CChunkFolder;
	friend class CChunkFileSys;

	friend CMpqFileList *GetMpqFileListFromMpqArchive(CString &pathArchive);
	friend BOOL RemoveChunk0(char *buffer);

};


class CChunkFolder
{
public:
	CChunkFolder(void);
	~CChunkFolder(void);

	BOOL IsReading();//Read ?
	BOOL IsWriting();//Writing?

	void InitFolder(CString &pathFolder,ChunkAccessMode mode);
	void InitMPQFolder(MPQHANDLE hMPQ,CMpqFileList *pMpqFileList,CString &pathMPQFolder,CString &pathSub,ChunkAccessMode mode);

	BOOL IsFolder();
	BOOL IsMpqFolder();

	CString GetFolderPath();

	struct
	{
		struct//ChunkType_Folder
		{
			CString m_pathFolder;//Not include the slash at the tail
		};
		struct //ChunkType_MPQFolder
		{
			CString m_pathMPQFolder;//Not include the slash at the tail
			MPQHANDLE m_hMPQArchive;
			CMpqFileList *m_pMpqFileList;//Companying the Archive
			CString m_pathSub;//Not include the leading slash,and neither the slash at tail
		};

	};


	void SetIgnoreWarning(BOOL bIgnoreWarning=TRUE);


	void CloseSeekedChunk();
	BOOL SeekChunk(LPCSTR pathSubChunk);
	BOOL RemoveChunk(LPCSTR pathSubChunk);
	BOOL NewChunk(LPCSTR pathSubChunk);

	//Chunk enumeration
	DWORD GetChunkCount();
	LPSTR GetChunkSubPath(DWORD idx);
	BOOL CheckChunkNameDupe(LPCSTR pathSubChunk);//return TRUE if the pathSubChunk is duplicated with a chunk already in this chunk folder
	//

	CChunk *GetChunk();//call this function after seeking a chunk .
	int GetChunkSize();//valid for reading.And call this function after seeking a chunk .

	// insertion operations
	CChunkFolder& operator<<(BYTE value);
	CChunkFolder& operator<<(WORD value);
	CChunkFolder& operator<<(LONG value);
	CChunkFolder& operator<<(DWORD value);
	CChunkFolder& operator<<(float value);
	CChunkFolder& operator<<(double value);
	CChunkFolder& operator<<(LONGLONG value);
	CChunkFolder& operator<<(ULONGLONG value);

	CChunkFolder& operator<<(int value);
	CChunkFolder& operator<<(short value);
	CChunkFolder& operator<<(char value);
	CChunkFolder& operator<<(unsigned value);
	CChunkFolder& operator<<(bool value);

	CChunkFolder& operator<<(CString &value);

	// extraction operations
	CChunkFolder& operator>>(BYTE &value);
	CChunkFolder& operator>>(WORD &value);
	CChunkFolder& operator>>(LONG &value);
	CChunkFolder& operator>>(DWORD &value);
	CChunkFolder& operator>>(float &value);
	CChunkFolder& operator>>(double &value);
	CChunkFolder& operator>>(LONGLONG &value);
	CChunkFolder& operator>>(ULONGLONG &value);

	CChunkFolder& operator>>(int &value);
	CChunkFolder& operator>>(short &value);
	CChunkFolder& operator>>(char &value);
	CChunkFolder& operator>>(unsigned &value);
	CChunkFolder& operator>>(bool &value);

	CChunkFolder& operator>>(CString &value);

	void Read(void* lpBuf, UINT nMax);
	void Write(void* lpBuf, UINT nMax);

private:
	void Close();

	BOOL m_bIgnoreWarning;
	CChunk *m_pSeekChunk;
	ChunkAccessMode m_mode;
	ChunkFolderType m_type;


	void CollectFolderChunkInfo(CString &pathRoot,CString &pathFolder);
	void CollectChunkInfo();
	std::vector<std::string>m_aChunkInfo;
	BOOL m_bChunkInfoDirty;

	BOOL PreWriteTest();
	BOOL PreReadTest();

	friend class CChunkFileSys;

};
