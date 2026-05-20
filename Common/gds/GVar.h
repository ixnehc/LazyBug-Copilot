#pragma once

#include "GDefines.h"

#include <string>


#include <vector>

class CDataPacket;

struct GVar
{
	GVar()
	{
		Zero();
		SetDefault(GSem(GSem_Unknown));
	}
	GVar(GVarType vt,GSem&sem=GSem(GSem_Unknown))
	{
		type=vt;
		SetDefault(sem);
	}
	GVar(GVarType vt,GSemCode code,const char *constraint)
	{
		type=vt;
		GSem t(code,constraint);
		SetDefault(t);
	}
	void Zero()
	{
		type=GVT_None;
		str="";
		data.clear();
	}

	GVarType Type()	{		return type;	}

	void SetDefault(GSem& sem);//Set default value
	BYTE *GetData(DWORD &szData)
	{
		szData=data.size();
		if (szData<=0)
			return NULL;
		return data.data();
	}

	GVar(int i);
	GVar(int i0,int i1);
	GVar(int i0,int i1,int i2,int i3);
	GVar(unsigned int u);
	GVar(unsigned int u0,unsigned int u1,unsigned int u2,unsigned int u3);
	GVar(unsigned int u0,unsigned int u1);
	GVar(float f);
	GVar(float f0,float f1);
	GVar(float f0,float f1,float f2);
	GVar(float f0,float f1,float f2,float f3);


	int&Int();
	GInt4& Int4();
	GInt2& Int2();
	unsigned int&UInt();
	GUInt4& UInt4();
	GUInt2& UInt2();
	short &Short();
	unsigned short &Word();
	unsigned char &Byte();
	GByte2&Byte2();
	float&Float();
	GFloat2&Float2();
	GFloat3&Float3();
	GFloat4&Float4();
	GFloat6&Float6();
	GFloat12&Float12();
	GFloat16&Float16();
	std::string &Str();
	void SetRaw(void *data,unsigned int szData);
	void *GetRaw();
	GUInt8& UInt8();
	//XXXXX: more GVarType
	
	BOOL Equal(GVar &src);

	GVar &operator=(const GVar &src)
	{
		type=src.type;
		data=src.data;
		str=src.str;

		return *this;
	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	GVarType type;
	std::string str;
	std::vector<BYTE>data;
};




