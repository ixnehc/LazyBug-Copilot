/********************************************************************
	created:	2006/10/12   16:30
	filename: 	e:\IxEngine\Common\variant\Variant.cpp
	author:		cxi
	
	purpose:	a data block contains data of different type
*********************************************************************/
#include "stdh.h"
#include "GVar.h"

#include "datapacket/DataPacket.h"
#include <assert.h>

GVar::GVar(int i)
{
	type=GVT_S;
	data.resize(sizeof(int));
	int *p=(int*)data.data();
	p[0]=i;
}
GVar::GVar(int i0,int i1)
{
	type=GVT_Sx2;
	data.resize(sizeof(int)*2);
	int *p=(int*)data.data();
	p[0]=i0;
	p[1]=i1;
}
GVar::GVar(int i0,int i1,int i2,int i3)
{
	type=GVT_Sx4;
	data.resize(sizeof(int)*4);
	int *p=(int*)data.data();
	p[0]=i0;
	p[1]=i1;
	p[2]=i2;
	p[3]=i3;
}
GVar::GVar(unsigned int u)
{
	type=GVT_U;
	data.resize(sizeof(unsigned int));
	unsigned int*p=(unsigned int*)data.data();
	p[0]=u;
}
GVar::GVar(unsigned int u0,unsigned int u1,unsigned int u2,unsigned int u3)
{
	type=GVT_Ux4;
	data.resize(sizeof(unsigned int)*4);
	unsigned int*p=(unsigned int*)data.data();
	p[0]=u0;
	p[1]=u1;
	p[2]=u2;
	p[3]=u3;
}
GVar::GVar(unsigned int u0,unsigned int u1)
{
	type=GVT_Ux2;
	data.resize(sizeof(unsigned int)*2);
	unsigned int*p=(unsigned int*)data.data();
	p[0]=u0;
	p[1]=u1;
}
GVar::GVar(float f)
{
	type=GVT_F;
	data.resize(sizeof(float));
	float*p=(float*)data.data();
	p[0]=f;
}
GVar::GVar(float f0,float f1)
{
	type=GVT_Fx2;
	data.resize(sizeof(float)*2);
	float*p=(float*)data.data();
	p[0]=f0;
	p[1]=f1;
}
GVar::GVar(float f0,float f1,float f2)
{
	type=GVT_Fx3;
	data.resize(sizeof(float)*3);
	float*p=(float*)data.data();
	p[0]=f0;
	p[1]=f1;
	p[2]=f2;
}
GVar::GVar(float f0,float f1,float f2,float f3)
{
	type=GVT_Fx4;
	data.resize(sizeof(float)*4);
	float*p=(float*)data.data();
	p[0]=f0;
	p[1]=f1;
	p[2]=f2;
	p[3]=f3;
}


#define GVAR_ACCESS_FUNC(func,rettype,typeAssure)\
rettype & GVar::func()\
{\
	assert(type==typeAssure);\
	data.resize(sizeof(rettype));\
	return *((rettype*)data.data());\
}

GVAR_ACCESS_FUNC(Int,int,GVT_S)
GVAR_ACCESS_FUNC(Int4,GInt4,GVT_Sx4)
GVAR_ACCESS_FUNC(Int2,GInt2,GVT_Sx2)
GVAR_ACCESS_FUNC(UInt,unsigned int,GVT_U)
GVAR_ACCESS_FUNC(UInt4,GUInt4,GVT_Ux4)
GVAR_ACCESS_FUNC(UInt2,GUInt2,GVT_Ux2)
GVAR_ACCESS_FUNC(Float,float,GVT_F)
GVAR_ACCESS_FUNC(Float2,GFloat2,GVT_Fx2)
GVAR_ACCESS_FUNC(Float3,GFloat3,GVT_Fx3)
GVAR_ACCESS_FUNC(Float4,GFloat4,GVT_Fx4)
GVAR_ACCESS_FUNC(Float12,GFloat12,GVT_Fx12)
GVAR_ACCESS_FUNC(Float16,GFloat16,GVT_Fx16)
GVAR_ACCESS_FUNC(Float6,GFloat6,GVT_Fx6)
GVAR_ACCESS_FUNC(Short,short,GVT_SS)
GVAR_ACCESS_FUNC(Word,unsigned short,GVT_SU)
GVAR_ACCESS_FUNC(Byte,unsigned char,GVT_B)
GVAR_ACCESS_FUNC(Byte2,GByte2,GVT_Bx2)
GVAR_ACCESS_FUNC(UInt8,GUInt8,GVT_Bx8)
//XXXXX: more GVarType

std::string &GVar::Str()
{
	assert(type==GVT_String);
	return str;
}

void GVar::SetRaw(void *data0,unsigned int szData)
{
	if (!data0)
	{
		data.clear();
		return;
	}
	assert(type==GVT_Raw);
	data.resize(szData);
	memcpy(data.data(),data0,szData);
}
void *GVar::GetRaw()
{
	assert(type==GVT_Raw);
	if(data.size()>0)
		return data.data();
	return NULL;
}

BOOL GVar::Equal(GVar &src)
{
	if (type!=src.type)
		return FALSE;
	if (type==GVT_String)
		return str==src.str;
	if (data.size()!=src.data.size())
		return FALSE;
	if (memcmp(data.data(),src.data.data(),data.size())!=0)
		return FALSE;
	return TRUE;
}

void GVar::Save(CDataPacket &dp)
{
	dp.Data_NextInt()=type;
	DP_WriteVector(dp,data);
	dp.Data_WriteString(str);
}

void GVar::Load(CDataPacket &dp)
{
	type=(GVarType)dp.Data_NextInt();
	DP_ReadVector(dp,data);
	dp.Data_ReadString(str);
}

//Set default value
void GVar::SetDefault(GSem& sem)
{
	switch(type)
	{
		case GVT_S:
			Int()=0;
			break;
		case GVT_U:
			UInt()=0;
			break;
		case GVT_F:
			Float()=0;
			if (sem.code==GSem_Alpha)
				Float()=1;
			if (sem.code==GSem_Shiness)
				Float()=100;
			if (sem.code==GSem_ShineStr)
				Float()=1.0f;
			if (sem.code==GSem_TexelLength)
				Float()=1.0f;
			break;
		case GVT_Fx2:
		{
			if (sem.code==GSem_TextureSize)
			{
				float v[2]={0,0};
				memcpy(&Float2(),v,sizeof(v));
			}
			break;
		}
		case GVT_Fx3:
		{
			if (sem.code==GSem_Normal)
			{
				float v[3]={1,0,0};
				memcpy(&Float3(),v,sizeof(v));
			}
			else
			{
				float v[3]={1,1,1};
				memcpy(&Float3(),v,sizeof(v));
			}
			break;
		}
		case GVT_Fx4:
		{
			if (sem.code==GSem_Plane)
			{
				float v[4]={0,1,0,0};
				memcpy(&Float4(),v,sizeof(v));
			}
			else
			{
				if (sem.code==GSem_ColorAlpha)
				{
					float v[4]={1,1,1,1};
					memcpy(&Float4(),v,sizeof(v));
				}
				else
				{
					float v[4]={0,0,0,0};
					memcpy(&Float4(),v,sizeof(v));
				}
			}
			break;
		}
		case GVT_Fx12:
		{
			float v[12]={1,0,0,0,1,0,0,0,1,0,0,0};
			memcpy(&Float12(),v,sizeof(v));
			break;
		}
		case GVT_Fx16:
		{
			float v[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
			memcpy(&Float16(),v,sizeof(v));
			break;
		}
		case GVT_Ux4:
		case GVT_Sx4:
		{
			if ((sem.code==GSem_UVAddr)&&(type==GVT_Sx4))
			{
				int v[4]={1,1,1,0};//All D3DTADDRESS_WRAP
				memcpy(&Int4(),v,sizeof(v));
				break;
			}
			int v[4]={0,0,0,0};
			memcpy(&Int4(),v,sizeof(v));
			break;
		}
		case GVT_Ux2:
		case GVT_Sx2:
		{
			int v[2]={0,0};
			memcpy(&Int2(),v,sizeof(v));
			break;
		}
		case GVT_String:
			Str()="";
			break;
		case GVT_Raw:
			SetRaw(NULL,0);
			break;
	}
	//XXXXX more GSem
}


