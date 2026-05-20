#pragma once

#include "DataPacket.h"


class CBitPacket
{
public:
	typedef unsigned __int64 Position;
	CBitPacket()
	{
		_bits=NULL;
		_iCurBit=0;
	}

	void Reset()
	{
		_dp.Reset();
		_iCurBit=0;
	}

	BOOL IsEmpty()
	{
		return (_iCurBit==0)&&(_dp.m_iWorking==0);
	}

	void SetBufferPointer(BYTE *pData,BYTE *pBits)
	{
		_dp.SetDataBufferPointer(pData);
		_bits=pBits;
		_iCurBit=0;

	}
	void GetBufferPointer(BYTE *&pData,BYTE *&pBits)
	{
		pData=_dp.GetDataBufferPointer();
		pBits=_bits;
	}
	void GetBufferSize(DWORD &szData,DWORD &szBitsInByte)
	{
		szData=_dp.GetDataSize();
		if (_iCurBit<=0)
			szBitsInByte=0;
		else
			szBitsInByte=(_iCurBit+7)/8+3;//3是patch,凑足一个DWORD
	}
	DWORD GetBitSize()	{		return _iCurBit;	}
	Position GetCurPos()
	{
		Position pos;
		pos=_iCurBit;
		pos<<=32;
		pos+=_dp.m_iWorking;
		return pos;
	}
	void SetCurPos(Position pos)
	{
		_dp.m_iWorking=(int)(pos&0xffffffff);
		_iCurBit=(DWORD)(pos>>32);
	}

	CDataPacket *GetDP()	{		return &_dp;	}

	void Bit_Write_0()	{		Bit_Write(0);	}
	void Bit_Write_1()	{		Bit_Write(1);	}
	void Bit_Write(BOOL b)
	{
		if (_bits)
		{
			BYTE *p=(BYTE*)((_bits+(_iCurBit/8)));
			DWORD off=_iCurBit%8;
			if (b)
				(*p)|=(BYTE)(1<<off);
			else
				(*p)&=~(BYTE)(1<<off);
		}
		_iCurBit++;
	}
	BOOL Bit_Read()
	{
		if (!_bits)
		{
			_iCurBit++;
			return FALSE;
		}

		BYTE *p=(BYTE*)((_bits+(_iCurBit/8)));
		DWORD off=_iCurBit%8;
		_iCurBit++;
		if ((*p)&(BYTE)(1<<off))
			return TRUE;
		return FALSE;
	}

	//写入若干bit,c为bit的个数,最多为16
	void Bits_Write(DWORD v,DWORD c);
	DWORD Bits_Read(DWORD c)
	{
		DWORD *p=(DWORD*)((_bits+(_iCurBit/8)));
		DWORD off=_iCurBit%8;
		DWORD ret=(*p);
		ret>>=off;
		ret&=((1<<c)-1);
		_iCurBit+=c;
		return ret;
	}

// 	void WriteMC()
// 	{
// 		Bits_Write(12352,14);
// 	}
// 
// 	void AssertMC();



	//CDataPacket的函数
	void *&Data_NextPtr()	{		return _dp.Data_NextPtr();	}
	float &Data_NextFloat(){		return _dp.Data_NextFloat();	}
	int &Data_NextInt(){		return _dp.Data_NextInt();	}
	unsigned int&Data_NextDword(){		return _dp.Data_NextDword();	}
	unsigned short&Data_NextWord(){		return _dp.Data_NextWord();	}
	short &Data_NextShort(){		return _dp.Data_NextShort();	}
	unsigned char&Data_NextByte(){		return _dp.Data_NextByte();	}
	char &Data_NextChar(){		return _dp.Data_NextChar();	}
	void Data_WriteString(std::string &s){		_dp.Data_WriteString(s);	}
	void Data_WriteString(const char *str){		_dp.Data_WriteString(str);	}
	void Data_ReadString(std::string &s){		_dp.Data_ReadString(s);	}
	void Data_ReadString(const char *&str){		_dp.Data_ReadString(str);	}
	void Data_WriteStringSH(const char *str){		_dp.Data_WriteStringSH(str);	}
	void Data_WriteStringSH(std::string &s){		_dp.Data_WriteStringSH(s);	}
	void Data_ReadStringSH(std::string &s){		_dp.Data_ReadStringSH(s);	}

	template<typename T>
	void Data_WriteSimple(T v)
	{
		_dp.Data_WriteSimple<T>(v);
	}

	template<typename T>
	void Data_WriteSimpleR(T &v)
	{
		_dp.Data_WriteSimpleR<T>(v);
	}

	template<typename T>
	T &Data_ReadSimple()
	{
		return _dp.Data_ReadSimple<T>();
	}

	template<typename T>
	void Data_ReadSimple(T &v)
	{
		return _dp.Data_ReadSimple<T>(v);
	}

	void Data_WriteData(void *pData,int szData){		_dp.Data_WriteData(pData,szData);	}
	void Data_ReadData(void *pData,int szData){		_dp.Data_ReadData(pData,szData);	}

	void Data_EncodeDword(DWORD dw){		_dp.Data_EncodeDword(dw);	}
	DWORD Data_DecodeDword(){		return _dp.Data_DecodeDword();	}

	void Data_MarchData(int szData){		_dp.Data_MarchData(szData);	}

protected:

	CDataPacket _dp;
	BYTE *_bits;
	DWORD _iCurBit;

	
};
