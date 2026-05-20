#pragma once

#include <vector>

class CChunkPacket
{
public:
	CChunkPacket()
	{
		Clear();
	}

	void Clear();
	void *GetChunkData(DWORD &szData);//Return the the data pointer
	void SetChunkData(void *pData,DWORD szData);

	//If ChunkRead_XXX return FALSE ,the v 's value will be left untouched
	BOOL ChunkRead_Ptr(void *&v,const char *name,DWORD iSerial=0);
	BOOL ChunkRead_Int(int &v,const char *name,DWORD iSerial=0);
	BOOL ChunkRead_Dword(DWORD &v,const char *name,DWORD iSerial=0);
	BOOL ChunkRead_Short(short &v,const char *name,DWORD iSerial=0);
	BOOL ChunkRead_Word(WORD &v,const char *name,DWORD iSerial=0);
	BOOL ChunkRead_Char(char &v,const char *name,DWORD iSerial=0);
	BOOL ChunkRead_Byte(BYTE &v,const char *name,DWORD iSerial=0);

	void *&ChunkWrite_Ptr(const char *name,DWORD iSerial=0);
	int &ChunkWrite_Int(const char *name,DWORD iSerial=0);
	DWORD &ChunkWrite_Dword(const char *name,DWORD iSerial=0);
	short &ChunkWrite_Short(const char *name,DWORD iSerial=0);
	WORD &ChunkWrite_Word(const char *name,DWORD iSerial=0);
	char &ChunkWrite_Char(const char *name,DWORD iSerial=0);
	BYTE &ChunkWrite_Byte(const char *name,DWORD iSerial=0);

	void Chunk_WriteString(const char *pString,const char *name,DWORD iSerial=0);
	char *Chunk_ReadString(const char *name,DWORD iSerial=0);
	void Chunk_WriteData(void *pData,DWORD szData,const char *name,DWORD iSerial=0);
	void *Chunk_ReadData(DWORD& szData,const char *name,DWORD iSerial=0);

private:
	std::vector<BYTE>m_data;

	BOOL CheckChunkHeader(char *&p,const char *name,DWORD iSerial,DWORD type);
	DWORD CalcChunkSize(const char *name,DWORD szData);
	void WriteChunkHeader(char *&p,DWORD szTotalData,const char *name,DWORD iSerial,DWORD type);

};


