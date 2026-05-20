/********************************************************************
	created:	27:7:2006   20:31
	file path:	d:\IxEngine\Common\datapacket
	file base:	DataPacket
	file ext:	cpp
	author:		cxi
	
	purpose:	data packet,used to serialize data to a continous memory
*********************************************************************/
#include "stdh.h"
#include "DataPacket.h"

#include "ChunkPacket.h"

#include <assert.h>

void CDataPacket::_SaveSerial()
{
#ifdef DATAPACKET_CHECKCONSISTENCY
	if (_serial!=0)
	{
		_serial++;

		if (m_aData)
			memcpy(m_aData+m_iWorking,&_serial,sizeof(_serial));
		m_iWorking+=sizeof(_serial);
	}
#endif
}
void CDataPacket::_LoadSerial()
{
#ifdef DATAPACKET_CHECKCONSISTENCY

	if (_serial!=0)
	{
		_serial++;

		DWORD *v;
		if(m_aData)
			v=(DWORD*)(m_aData+m_iWorking);
		else
			v=(DWORD*)&m_FakeBuffer;
		m_iWorking+=sizeof(DWORD);

		assert(v==_serial);
	}

#endif
}

 

void CDataPacket::SetDataBufferPointer(BYTE *pData)
{
	m_aData=pData;
	Reset();
}
void CDataPacket::Reset()
{
	m_iWorking=0;
}
int CDataPacket::GetDataSize()
{
	return m_iWorking;
}

void *CDataPacket::GetCurBufferPointer()
{
	if (!m_aData)
		return NULL;

	return m_aData+m_iWorking;
}

void CDataPacket::SetCurBufferPointer(void *p)
{
	if (!m_aData)
		return;

	m_iWorking=(int)(((BYTE*)p)-m_aData);
}



void *&CDataPacket::Data_NextPtr()
{
	m_iWorking+=4;
	if (m_aData)
		return *((PVOID*)(m_aData+m_iWorking-4));
	else
		return *(void **)&m_FakeBuffer;
}

float &CDataPacket::Data_NextFloat()
{
	m_iWorking+=4;
	if (m_aData)
		return *((float *)(m_aData+m_iWorking-4));
	else
		return *(float*)(&m_FakeBuffer);
}


int &CDataPacket::Data_NextInt()
{
	m_iWorking+=4;
	if (m_aData)
		return *((int *)(m_aData+m_iWorking-4));
	else
		return *(int*)(&m_FakeBuffer);
}

unsigned int&CDataPacket::Data_NextDword()
{
	m_iWorking+=4;
	if (m_aData)
		return *((unsigned int*)(m_aData+m_iWorking-4));
	else
		return *(unsigned int*)(&m_FakeBuffer);
}


unsigned short&CDataPacket::Data_NextWord()
{
	m_iWorking+=2;
	if (m_aData)
		return *((unsigned short*)(m_aData+m_iWorking-2));
	else
		return *(unsigned short*)(&m_FakeBuffer);
}

short &CDataPacket::Data_NextShort()
{
	m_iWorking+=2;
	if (m_aData)
		return *((short *)(m_aData+m_iWorking-2));
	else
		return *(short*)(&m_FakeBuffer);
}

unsigned char&CDataPacket::Data_NextByte()
{
	m_iWorking+=1;
	if (m_aData)
		return *((unsigned char*)(m_aData+m_iWorking-1));
	else
		return *(unsigned char*)(&m_FakeBuffer);
}
char &CDataPacket::Data_NextChar()
{
	m_iWorking+=1;
	if (m_aData)
		return *((char*)(m_aData+m_iWorking-1));
	else
		return *(char*)(&m_FakeBuffer);
}

void CDataPacket::Data_WriteData(void *pData,int szData)
{
	if (m_aData)
		memcpy(m_aData+m_iWorking,pData,szData);
	m_iWorking+=szData;
}
void CDataPacket::Data_ReadData(void *pData,int szData)
{
	if (m_aData)
		memcpy(pData,m_aData+m_iWorking,szData);
	m_iWorking+=szData;
}



void CDataPacket::Data_WriteString(const char *str)
{
	int len=strlen(str)+1;
	Data_NextInt()=len;
	Data_WriteData((char*)str,len);
}

void CDataPacket::Data_WriteString(const std::string &s)
{
	Data_WriteString(s.c_str());
}

void CDataPacket::Data_ReadString(std::string &s)
{
	int len;
	len=Data_NextInt();

	if (m_aData)
		s=(char*)(m_aData+m_iWorking); 

	assert(s.length()+1==len);
	m_iWorking+=len;

}

void CDataPacket::Data_ReadString(const char *&str)
{
	int len=Data_NextInt();

	if (m_aData)
		str=(const char*)(m_aData+m_iWorking); 

	m_iWorking+=len;
}

void CDataPacket::Data_WriteStringSH(const char *str)
{
	DWORD len=strlen(str);
	Data_EncodeDword(len);
	Data_WriteData((char*)str,len);
}

void CDataPacket::Data_ReadStringSH(std::string &s)
{
	DWORD len=(int)Data_DecodeDword();

	if (m_aData)
		s.assign((char*)(m_aData+m_iWorking),len);

	assert(s.length()==len);
	m_iWorking+=len;
}

void CDataPacket::Data_WriteWString(const wchar_t *str)
{
	int len = (wcslen(str) + 1) * sizeof(wchar_t);
	Data_NextInt() = len;
	Data_WriteData((void*)str, len);
}

void CDataPacket::Data_WriteWString(const std::wstring &s)
{
	Data_WriteWString(s.c_str());
}

void CDataPacket::Data_ReadWString(std::wstring &s)
{
	int len;
	len = Data_NextInt();
	
	if (m_aData && len > 0)
	{
		s.assign((wchar_t*)(m_aData + m_iWorking), (len / sizeof(wchar_t)) - 1);
	}
	else
	{
		s.clear();
	}
	
	m_iWorking += len;
}

void CDataPacket::Data_EncodeDword(DWORD dw)
{
	if (dw<253)
		Data_NextByte()=(BYTE)dw;
	else
	{
		if (dw<65536)
		{
			Data_NextByte()=253;
			Data_NextWord()=(WORD)dw;
		}
		else
		{
			Data_NextByte()=254;
			Data_NextDword()=dw;
		}
	}
}

DWORD CDataPacket::Data_DecodeDword()
{
	BYTE hd=Data_NextByte();
	if (hd<253)
		return (DWORD)hd;
	if (hd==253)
		return Data_NextWord();
	if (hd==254)
		return Data_NextDword();
	assert(FALSE);
	return 0;
}



void CDataPacket::Data_WriteChunkPacket(CChunkPacket &cp)
{
	DWORD szData;
	BYTE *pData;
	pData=(BYTE *)cp.GetChunkData(szData);

	Data_NextDword()=szData;
	Data_WriteData(pData,szData);

}

void CDataPacket::Data_ReadChunkPacket(CChunkPacket &cp)
{
	DWORD szData;
	szData=Data_NextDword();
	cp.SetChunkData(GetCurBufferPointer(),szData);
	Data_MarchData(szData);
}


void CDataPacket::Data_MarchData(int szData)//skip some empty data
{
	m_iWorking+=szData;
}



