
#pragma once

#include "IResource.h"

#include "IMtrlExt.h"

#include "shaderlib/SLDefines.h"
#include "shaderlib/SLEffectParam.h"
#include "strlib/strlibdefines.h"

#define MAX_BONE 256
#define MIN_BONE 25 //any shader should at least support MIN_BONE bones for vertex blending

//SetEP_xxxx() 的返回值
enum EPResult
{
	EPFail=-1,//彻底的失败,这个参数没有被成功的设到shader对应的ep里,会影响渲染的结果
	EPMissing=0,//shader里不存在这个EP,一般不影响渲染
	EPOk=TRUE,
};


struct EpkHead
{
	EpkHead()
	{
		Zero();
	}
	void Zero()
	{
		ep=EP_None;
		type=GVT_None;
		sz=0;
	}

	short ep;//EffectParam
	BYTE type;//GVarType
	BYTE sz;
};

class ITexture;
//注意:如果某个Head中的ep为EP_Max,表示这个ep使用一个StringID作为name,并且这个StringID紧跟在数据的后面
//也就是说数据排列方式是这样: head-data-idName
template <int T_size,int T_bAllowOverwrite=1>
struct EffectParamPacket
{
	EffectParamPacket()
	{
		Zero();
	}
	void Zero()
	{
		n=0;
		nEP=0;
		memset(ref,0xff,sizeof(ref));
	}

	WORD GetEPOff(EffectParam ep)	{		return ref[ep];	}

	BOOL AddEP(EffectParam ep,int v)	{		return _AddEP(ep,GVT_S,&v,sizeof(v));	}
	BOOL AddEP(EffectParam ep,float v)	{		return _AddEP(ep,GVT_F,&v,sizeof(v));	}
	BOOL AddEP(EffectParam ep,float *v,int sz)		
	{		
		if (sz*sizeof(float)>240)
		{
			assert(FALSE);
			return FALSE;
		}
		return _AddEP(ep,GVT_F,v,sz*sizeof(float));	
	}
	BOOL AddEP(EffectParam ep,i_math::vector2df&v)	{		return _AddEP(ep,GVT_Fx2,&v,sizeof(v));	}
	BOOL AddEP(EffectParam ep,i_math::vector3df&v)	{		return _AddEP(ep,GVT_Fx3,&v,sizeof(v));	}
	BOOL AddEP(EffectParam ep,i_math::vector4df&v)	{		return _AddEP(ep,GVT_Fx4,&v,sizeof(v));	}
	BOOL AddEP(EffectParam ep,i_math::matrix43f&v)	{		return _AddEP(ep,GVT_Fx12,&v,sizeof(v));	}
	BOOL AddEP(EffectParam ep,i_math::matrix44f&v)	{		return _AddEP(ep,GVT_Fx16,&v,sizeof(v));	}
	BOOL AddEP(EffectParam ep,ITexture *v)	{		return _AddEP(ep,GVT_String,&v,sizeof(v));	}


	BOOL AddEP(StringID nm,int v)	{		return _AddEP(nm,GVT_S,&v,sizeof(v));	}
	BOOL AddEP(StringID nm,float v)	{		return _AddEP(nm,GVT_F,&v,sizeof(v));	}
	BOOL AddEP(StringID nm,float *v,int sz)		
	{		
		if (sz*sizeof(float)>240)
		{
			assert(FALSE);
			return FALSE;
		}
		return _AddEP(nm,GVT_F,v,sz*sizeof(float));	
	}
	BOOL AddEP(StringID nm,i_math::vector2df&v)	{		return _AddEP(nm,GVT_Fx2,&v,sizeof(v));	}
	BOOL AddEP(StringID nm,i_math::vector3df&v)	{		return _AddEP(nm,GVT_Fx3,&v,sizeof(v));	}
	BOOL AddEP(StringID nm,i_math::vector4df&v)	{		return _AddEP(nm,GVT_Fx4,&v,sizeof(v));	}
	BOOL AddEP(StringID nm,i_math::matrix43f&v)	{		return _AddEP(nm,GVT_Fx12,&v,sizeof(v));	}
	BOOL AddEP(StringID nm,i_math::matrix44f&v)	{		return _AddEP(nm,GVT_Fx16,&v,sizeof(v));	}
	BOOL AddEP(StringID nm,ITexture *v)	{		return _AddEP(nm,GVT_String,&v,sizeof(v));	}

public://take it as protected

	BOOL _AddEP(EffectParam ep,GVarType type,void *data,DWORD szData)
	{
		if (T_bAllowOverwrite)
			if (ref[ep]!=0xffff)
			{//already existing
				EpkHead *p=(EpkHead *)&buf[ref[ep]];
				if (p->type!=type)
					return FALSE;
				if (p->sz!=szData)
					return FALSE;
				if (data)
					memcpy(&p[1],data,szData);
				else
					memset(&p[1],0,szData);
				return TRUE;
			}

			if (n+szData+sizeof(EpkHead)>=ARRAY_SIZE(buf))
				return FALSE;

			EpkHead *p=(EpkHead *)&buf[n];
			p->ep=ep;
			p->type=type;
			p->sz=(BYTE)szData;
			if (data)
				memcpy(&p[1],data,szData);
			else
				memset(&p[1],0,szData);

			ref[ep]=(WORD)n;

			n+=sizeof(EpkHead)+szData;
			nEP++;
			return TRUE;
	}

	BOOL _AddEP(StringID nm,GVarType type,void *data,DWORD szData)
	{
		if (n+szData+sizeof(EpkHead)+sizeof(StringID)>=ARRAY_SIZE(buf))
			return FALSE;

		EpkHead *p=(EpkHead *)&buf[n];
		p->ep=EP_Max;
		p->type=type;
		p->sz=(BYTE)szData+sizeof(StringID);
		if (data)
			memcpy(&p[1],data,szData);
		else
			memset(&p[1],0,szData);
		memcpy(((BYTE*)&p[1])+szData,&nm,sizeof(StringID));

		n+=sizeof(EpkHead)+szData+sizeof(StringID);
		nEP++;
		return TRUE;
	}


	BYTE buf[T_size];
	DWORD n;
	DWORD nEP;
	WORD ref[EP_Max];
};


struct MtrlExtData;

class CShaderLib;
struct ShaderState;
class IShaderLibMgr:public IResourceMgr
{
public:
	virtual IResource *ObtainShader(const char *nameSL,FeatureCode &fc,MteID idMte=MteID_Invalid)=0;
	virtual IResource *ObtainShader(const char *nameSL,const char *nameUF,MteID idMte=MteID_Invalid)=0;
	virtual IResource *ObtainShader(ShaderCode &sc)=0;
	virtual ShaderCode MakeShaderCode(const char *nameSL,FeatureCode &fc,const char *nameUF,MteID idMte=MteID_Invalid)=0;
	virtual BOOL AddShaderLib(const char *path)=0;//a relative path to shader lib's res folder
	virtual CShaderLib*GetShaderLib(const char *nameSL)=0;
	virtual DWORD GetShaderCount()=0;//目前所有的shader个数
	
	virtual DWORD GetLibCount()=0;
	virtual const char *GetLibName(DWORD iLib)=0;
	virtual int FindLibByName(const char *name)=0;//如果找不到,返回-1
	virtual FeatureCode EnumFeatureCode(const char *nameLib,FeatureFlag flagMask=FF_All)=0;//Enumerate all the feature codes in the specified lib
	virtual const char *EnumFeatureNames(const char *nameLib,FeatureFlag flagMask=FF_All)=0;//Enumerate all the feature names in the specified lib,用","分隔

	//Enumerate all the unifeature in the specified lib
	//return string formatted like: uf1,uf2,uf3,... (use "," as seperator)
	//the return pointer should not be kept for later use
	virtual const char *EnumUniFeatures(const char *nameLib)=0;
	virtual BOOL ExistUniFeature(const char *nameLib,const char *nameUF)=0;

	//这两个接口主要用于编辑时用,一般不要用
	virtual BOOL RegisterMte(MtrlExtData *data,MteID id,const char *pathMte)=0;
	virtual MteID AbandonMte(const char *pathMte)=0;//返回这个pathMte对应的Mte名称ID

};

class IVertexBuffer;
class IIndexBuffer;
struct VBBindArg;

class IShader:public IResource
{
public:
	//caps retrieval
	virtual int GetCap_maxbones()=0;
	virtual int GetCap_weightcount()=0;
	//XXXXX:more ShaderCap

	virtual ShaderCode &GetShaderCode()=0;

	virtual BOOL ExistEP(EffectParam ep,DWORD idx=0)=0;//Check whether the shader need this effect param to work
	virtual BOOL ExistEP(StringID epname)=0;//Check whether the shader need this effect param to work
	virtual BOOL CheckValid()=0;
	virtual const char *GetFX()=0;
	virtual const char *GetFXErr()=0;

	//general SetEP functions
	virtual EPResult SetEP(EffectParam ep,i_math::color4df &c,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::vector3df &v,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::vector2df &v,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,ITexture *tex,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::f32 f,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::s32 s,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::vector4di &v,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::matrix43f &mat,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::matrix44f &mat,DWORD idx=0)=0;
	virtual EPResult SetEP(EffectParam ep,float *fs,DWORD count)=0;
	virtual EPResult SetEP(EffectParam ep,i_math::vector4df *data,DWORD count)=0;

	virtual EPResult SetEP(StringID nm,color4df &c)=0;
	virtual EPResult SetEP(StringID nm,vector3df &v)=0;
	virtual EPResult SetEP(StringID nm,ITexture *tex)=0;
	virtual EPResult SetEP(StringID nm,f32 f)=0;
	virtual EPResult SetEP(StringID nm,vector2df &v)=0;
	virtual EPResult SetEP(StringID nm,i_math::s32 s)=0;
	virtual EPResult SetEP(StringID nm,i_math::vector4di &v)=0;
	virtual EPResult SetEP(StringID nm,i_math::matrix43f &mat)=0;
	virtual EPResult SetEP(StringID nm,i_math::matrix44f &mat)=0;


	virtual EPResult SetEP_World(i_math::matrix43f *mats,DWORD count)=0;
	virtual EPResult SetEP_ViewProj(i_math::matrix44f &viewproj,i_math::matrix44f &view,i_math::matrix44f &proj)=0;

	virtual EPResult SetEPs(BYTE *data,DWORD nEP)=0;//data/nEP should be from EffectParamPacket

	virtual void SetState(ShaderState&state)=0;
	virtual void SetState(ShaderState &state,BOOL bForceAlpha)=0;//bForceAlpha表示要强制为透明模式
	virtual ShaderState& GetState()=0;

	virtual void SetDepthBias(float biasSlope,float bias)=0;


	virtual BOOL BindVB(IVertexBuffer *vb,IIndexBuffer *ib,VBBindArg *arg)=0;
	virtual BOOL BindVBs(IVertexBuffer **vbs,DWORD nVB,IIndexBuffer *ib,VBBindArg *arg)=0;
	virtual BOOL SetVBInstanceCount(DWORD count)=0;//启用/禁止Hardware的instance,使用前stream 0 必须绑定geometry data,stream 1 必须绑定instance data
													//count为0时禁止hardware的instance,

	virtual BOOL BeginRaw()=0;
	virtual void EndRaw()=0;
	virtual BOOL DoShadeRaw()=0;

};
