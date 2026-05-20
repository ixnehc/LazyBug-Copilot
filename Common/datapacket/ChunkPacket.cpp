/********************************************************************
	created:	27:7:2006   20:30
	file path:	d:\IxEngine\Common\datapacket
	file base:	ChunkPacket
	file ext:	cpp
	author:		cxi
	
	purpose:	chunk packet,packet to store data with a readable name
*********************************************************************/
#include "stdh.h"
#include "ChunkPacket.h"

#pragma warning(disable:4267)


#define ChunkPacketType_4 1
#define ChunkPacketType_2 2
#define ChunkPacketType_1 3
#define ChunkPacketType_Data 4
 

void CChunkPacket::Clear()
{
	m_data.clear();
}
void *CChunkPacket::GetChunkData(DWORD &szData)//Return the the data pointer
{
	szData=m_data.size();
	return &(m_data[0]);
}
void CChunkPacket::SetChunkData(void *pData,DWORD szData)
{
	m_data.resize(szData);
	memcpy(m_data.data(),pData,szData);
}

BOOL CChunkPacket::CheckChunkHeader(char *&p,const char *name,DWORD iSerial,DWORD type)
{
	char *pNext;
	pNext=p+*(DWORD*)(p);
	p+=sizeof(DWORD);
	char *q;
	q=(char*)name;
	while(*q)
	{
		if(*p!=*q)
			break;
		p++;
		q++;
	}
	if ((*p)||(*q))
	{
		p=pNext;
		return FALSE;
	}
	p++;
	//ok,name match,check serial
	if (*(DWORD*)(p)!=iSerial)
	{
		p=pNext;
		return FALSE;
	}
	p+=sizeof(DWORD);

	//Serial match,check type
	if (*(DWORD*)(p)!=type)
	{
		p=pNext;
		return FALSE;
	}
	p+=sizeof(DWORD);

	//All match
	return TRUE;

}

DWORD CChunkPacket::CalcChunkSize(const char *name,DWORD szData)
{
	return sizeof(DWORD)//Total size
		+strlen(name)+1//name
		+sizeof(DWORD)//iSerial
		+sizeof(DWORD)//Type
		+szData;
}

void CChunkPacket::WriteChunkHeader(char *&p,DWORD szTotal,const char *name,DWORD iSerial,DWORD type)
{
	*(DWORD*)(p)=szTotal;
	p+=sizeof(DWORD);

	char *q;
	q=(char *)name;
	while(*q)
	{
		*p=*q;
		p++;
		q++;
	}
	*p=0;
	p++;

	*(DWORD*)(p)=iSerial;
	p+=sizeof(DWORD);

	*(DWORD*)(p)=type;
	p+=sizeof(DWORD);
}

#define ChunkWrite_XXX(func_name,type,datatype) \
datatype &CChunkPacket::func_name(const char *name,DWORD iSerial)\
{\
	char *p,*p0,*pEnd;\
	p0=p=(char *)m_data.data();\
	pEnd=p0+m_data.size();\
	if (m_data.size()>0)\
	{\
		while(p<pEnd)\
		{\
			if (CheckChunkHeader(p,name,iSerial,type))\
			{\
				return (*(datatype*)p);\
			}\
		}\
	}\
	DWORD szChunk;\
	szChunk=CalcChunkSize(name,sizeof(datatype));\
	DWORD szOld;\
	szOld=m_data.size();\
	m_data.resize(m_data.size()+szChunk);\
	p=(char *)&m_data[szOld];\
	WriteChunkHeader(p,szChunk,name,iSerial,type);\
	return (*(datatype*)p);\
}

#define ChunkRead_XXX(func_name,type,datatype)\
BOOL CChunkPacket::func_name(datatype&v,const char *name,DWORD iSerial)\
{\
	char *p,*p0,*pEnd;\
	p0=p=(char *)m_data.data();\
	pEnd=p0+m_data.size();\
	if (m_data.size()>0)\
	{\
		while(p<pEnd)\
		{\
			if (CheckChunkHeader(p,name,iSerial,type))\
			{\
				v=*(datatype*)p;\
				return TRUE;\
			}\
		}\
	}\
	return FALSE;\
}


ChunkWrite_XXX(ChunkWrite_Ptr,ChunkPacketType_4,void *)
ChunkWrite_XXX(ChunkWrite_Int,ChunkPacketType_4,int)
ChunkWrite_XXX(ChunkWrite_Dword,ChunkPacketType_4,DWORD)
ChunkWrite_XXX(ChunkWrite_Short,ChunkPacketType_2,short)
ChunkWrite_XXX(ChunkWrite_Word,ChunkPacketType_2,WORD)
ChunkWrite_XXX(ChunkWrite_Char,ChunkPacketType_1,char)
ChunkWrite_XXX(ChunkWrite_Byte,ChunkPacketType_1,BYTE)

ChunkRead_XXX(ChunkRead_Ptr,ChunkPacketType_4,void *)
ChunkRead_XXX(ChunkRead_Int,ChunkPacketType_4,int)
ChunkRead_XXX(ChunkRead_Dword,ChunkPacketType_4,DWORD)
ChunkRead_XXX(ChunkRead_Short,ChunkPacketType_2,short)
ChunkRead_XXX(ChunkRead_Word,ChunkPacketType_2,WORD)
ChunkRead_XXX(ChunkRead_Char,ChunkPacketType_1,char)
ChunkRead_XXX(ChunkRead_Byte,ChunkPacketType_1,BYTE)

void CChunkPacket::Chunk_WriteData(void *pData,DWORD szData,const char *name,DWORD iSerial)
{
	char *p,*p0,*pEnd;
	p0=p=(char *)m_data.data();
	pEnd=p0+m_data.size();
	if (m_data.size()>0)
	{
		while(p<pEnd)
		{
			char *pTemp;
			pTemp=p;
			if (CheckChunkHeader(p,name,iSerial,ChunkPacketType_Data))
			{	//Data is here,remove old
				DWORD szChunk;
				szChunk=*(DWORD*)pTemp;
				m_data.erase(m_data.begin()+(pTemp-p0),m_data.begin()+(pTemp-p0)+szChunk);
				break;
			}
		}
	}
	DWORD szChunk;
	szChunk=CalcChunkSize(name,szData);
	DWORD szOld;
	szOld=m_data.size();
	m_data.resize(m_data.size()+szChunk);
	p=(char *)&m_data[szOld];
	WriteChunkHeader(p,szChunk,name,iSerial,ChunkPacketType_Data);

	memcpy(p,pData,szData);
}

void *CChunkPacket::Chunk_ReadData(DWORD& szData,const char *name,DWORD iSerial)
{
	char *p,*p0,*pEnd;
	p0=p=(char *)m_data.data();
	pEnd=p0+m_data.size();
	if (m_data.size()>0)
	{
		while(p<pEnd)
		{
			char *pTemp;
			pTemp=p;
			if (CheckChunkHeader(p,name,iSerial,ChunkPacketType_Data))
			{//Data is here
				szData=*(DWORD*)pTemp-(DWORD)(p-pTemp);//Totol size-chunk size
				return p;
			}
		}
	}
	return NULL;
}


void CChunkPacket::Chunk_WriteString(const char *pString,const char *name,DWORD iSerial)
{
	DWORD len;
	len=strlen(pString)+1;
	Chunk_WriteData((void*)pString,len,name,iSerial);
}


char *CChunkPacket::Chunk_ReadString(const char *name,DWORD iSerial)
{
	DWORD sz;
	return (char*)Chunk_ReadData(sz,name,iSerial);
}


