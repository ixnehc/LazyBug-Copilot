#pragma once

#include <string>

//#define DATAPACKET_CHECKCONSISTENCY

class CChunkPacket;

class CDataPacket
{
public:
	CDataPacket()
	{
		m_iWorking=0;
		m_aData=NULL;
	}

	void SetDataBufferPointer(BYTE *pData);
	BYTE *GetDataBufferPointer()	{		return m_aData;	}
	void Reset();
	int GetDataSize();
	void *GetCurBufferPointer();
	void SetCurBufferPointer(void *p);

	void *&Data_NextPtr();
	float &Data_NextFloat();
	int &Data_NextInt();
	unsigned int&Data_NextDword();
	unsigned short&Data_NextWord();
	short &Data_NextShort();
	unsigned char&Data_NextByte();
	char &Data_NextChar();
	void Data_WriteString(const std::string &s);
	void Data_WriteString(const char *str);
	void Data_ReadString(std::string &s);
	void Data_ReadString(const char *&str);
	void Data_WriteStringSH(const char *str);//SH ����Small Header,ʹ�ø����õķ�ʽ�洢�ַ���,���������¿��Ա�û�д�SH�İ汾��ʡ4���ֽ�
	void Data_WriteStringSH(std::string &s)	{		return Data_WriteStringSH(s.c_str());	}
	void Data_ReadStringSH(std::string &s);
	
	void Data_WriteWString(const std::wstring &s);
	void Data_WriteWString(const wchar_t *str);
	void Data_ReadWString(std::wstring &s);

	void Data_WriteData(void *pData,int szData);
	void Data_ReadData(void *pData,int szData);

	template<typename T>
	void Data_WriteSimple(T v)
	{
		if (m_aData)
			memcpy(m_aData+m_iWorking,&v,sizeof(T));
		m_iWorking+=sizeof(T);
	}

	//R��������Ϊ����
	template<typename T>
	void Data_WriteSimpleR(T &v)
	{
		if (m_aData)
			memcpy(m_aData+m_iWorking,&v,sizeof(T));
		m_iWorking+=sizeof(T);
	}

	template<typename T>
	T &Data_ReadSimple()
	{
		T*p;
		if(m_aData)
			p=(T*)(m_aData+m_iWorking);
		else
			p=(T*)&m_FakeBuffer;
		m_iWorking+=sizeof(T);
		return *p;
	}

	template<typename T>
	void Data_ReadSimple(T &v)
	{
		v=Data_ReadSimple<T>();
	}

	void Data_WriteChunkPacket(CChunkPacket &cp);
	void Data_ReadChunkPacket(CChunkPacket &cp);

	void Data_EncodeDword(DWORD dw);
	DWORD Data_DecodeDword();

	void Data_MarchData(int szData);//skip some empty data

private:
	BYTE *m_aData;
	int m_iWorking;
	
	unsigned __int64 m_FakeBuffer;

	void _SaveSerial();
	void _LoadSerial();


#ifdef DATAPACKET_CHECKCONSISTENCY
public:
	void StartCheckConsistency()	{		_serial=137;	}
	void EndCheckConsistency()	{		_serial=0;	}

protected:

	DWORD _serial;

#endif

	friend class CBitPacket;
	
};

#define DP_ReadVector(dp,vec)\
{\
	DWORD sz;\
	sz=(dp).Data_NextDword();\
	(vec).resize(sz);\
	if (sz>0)\
		(dp).Data_ReadData((void*)&(vec)[0],sz*sizeof((vec)[0]));\
}

#define DP_WriteVector(dp,vec)\
{\
	DWORD sz;\
	sz=(DWORD)(vec).size();\
	(dp).Data_NextDword()=sz;\
	if (sz>0)\
		(dp).Data_WriteData((void*)&(vec)[0],sz*sizeof((vec)[0]));\
}

//N����New
#define DP_ReadVectorN(dp,vec)\
{\
	DWORD sz;\
	sz=(dp).Data_DecodeDword();\
	(vec).resize(sz);\
	if (sz>0)\
		(dp).Data_ReadData((void*)&(vec)[0],sz*sizeof((vec)[0]));\
}

#define DP_WriteVectorN(dp,vec)\
{\
	DWORD sz;\
	sz=(DWORD)(vec).size();\
	(dp).Data_EncodeDword(sz);\
	if (sz>0)\
		(dp).Data_WriteData((void*)&(vec)[0],sz*sizeof((vec)[0]));\
}


#define DP_WriteData(dp,pData,szData)\
{\
	if (pData)\
	{\
		(dp).Data_NextDword()=(szData);\
		(dp).Data_WriteData(pData,szData);\
	}\
	else\
		(dp).Data_NextDword()=0;\
}

//NOTE:pData will be overwritten with a pointer allocated by "new",or 
//be filled with NULL if no data available
#define DP_ReadData(dp,pData,szData)\
{\
	DWORD sz;\
	sz=(dp).Data_NextDword();\
	if (sz<=0)\
	{\
		(pData)=NULL;\
		szData=0;\
	}\
	else\
	{\
		((BYTE*&)(pData))=new BYTE[sz];\
		(dp).Data_ReadData(pData,sz);\
		szData=sz;\
	}\
}


//NOTE:pData will be overwritten with a pointer allocated by "new",or 
//be filled with NULL if no data available
//NS: no size
#define DP_ReadDataNS(dp,pData)\
{\
	DWORD szDummy;\
	DP_ReadData(dp,pData,szDummy)\
}

#define DP_WriteVariant(dp,v)\
{\
	(dp).Data_NextInt()=(v).GetType();\
	DWORD sz=(v).GetActualSize();\
	(dp)p.Data_NextDword()=sz;\
	(dp)p.Data_WriteData((v).GetDataPtr(),sz);\
}

#define DP_ReadVariant(dp,v)\
{\
	(v).SetType((dp)p.Data_NextInt());\
	DWORD sz=(dp).Data_NextDword();\
	(v).SetRawData((dp).Data_CurrentPos(),sz);\
	(dp).Data_MarchData(sz);\
}

#define  DP_WriteVar(dp,v)\
	(dp).Data_WriteData(&(v),sizeof(v))

#define  DP_ReadVar(dp,v)\
	(dp).Data_ReadData(&(v),sizeof(v))

#define DP_SafeWriteVar(dp,v)\
	(dp).Data_NextDword()=sizeof(v);		\
	(dp).Data_WriteData(&(v),sizeof(v))

inline BOOL _DP_SafeReadVar(CDataPacket &dp,BYTE *buf,DWORD szBuf)
{
	DWORD sz=dp.Data_NextDword();
	if (sz==szBuf)
	{
		dp.Data_ReadData(buf,szBuf);
		return TRUE;
	}
	dp.Data_MarchData(sz);
	return FALSE;
};
#define DP_SafeReadVar(dp,v)	_DP_SafeReadVar(dp,(BYTE*)&v,sizeof(v))

#define  DP_WriteArray(dp,v)\
	(dp).Data_WriteData((v),sizeof(v))

#define  DP_ReadArray(dp,v)\
	(dp).Data_ReadData((v),sizeof(v))


#define DP_BeginSave(__dp,__buf)																	\
{																															\
	CDataPacket __dp;																							\
	for (int __i=0;__i<2;__i++)																						\
	{																														\
		if (__i==1)																										\
		{																													\
			(__buf).resize(__dp.GetDataSize());															\
			__dp.SetDataBufferPointer(&(__buf)[0]);												\
		}

#define DP_EndSave()																						\
	}																														\
}


#define DP_PreSafeSave(__dp)																	\
{																													\
	CDataPacket *__t=&(__dp);																		\
	DWORD *__sz=(DWORD*)(__dp).GetCurBufferPointer();						\
	(__dp).Data_MarchData(sizeof(DWORD));

#define DP_PostSafeSave()																		\
	if (__sz)																										\
	{																												\
		*__sz=(DWORD)((BYTE*)__t->GetCurBufferPointer()-(BYTE*)__sz);	\
		*__sz-=sizeof(DWORD);																		\
	}																												\
}

#define DP_PreSafeLoad(__dp)																	\
{																													\
	CDataPacket *__t=&__dp;																		\
	DWORD __sz=__dp.Data_NextDword();												\
	BYTE *__p=(BYTE *)__dp.GetCurBufferPointer();

#define DP_SafeLoad_Size() (__sz)

#define DP_PostSafeLoad()																		\
	__t->SetCurBufferPointer(__p+__sz);														\
}