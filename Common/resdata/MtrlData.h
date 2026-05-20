#pragma once

#include <string>
#include <vector>


#include "ResData.h"

#include "../valueset/valueset.h"


#define MAX_EP_NAME 24

struct FeatureParamA
{
	enum AnimType
	{
		Anim_None,
		Anim_ValueSet,//使用keyset实现动画
		Anim_Res,//使用动画资源实现动画
	};
	FeatureParamA()
	{
		memset(ep_name,0,sizeof(ep_name));
		at=Anim_None;
		bMte=0;
	}
	void SetEP(const char *name)
	{
		bMte=0;
		strncpy(ep_name,name,sizeof(ep_name)-1);
		ep_name[sizeof(ep_name)-1]=0;
	}
	void SetEP(StringID idName)
	{
		bMte=1;
		nm=idName;
	}
	const char *GetEPName()
	{
		if (bMte)
			return "";
		return ep_name;
	}

	StringID GetEPNameID()
	{
		if (bMte)
			return nm;
		return StringID_Invalid;
	}
	BOOL CheckSameEP(FeatureParamA &other)
	{
		if (other.var.type!=var.type)
			return FALSE;
		if (bMte!=other.bMte)
			return FALSE;
		if (bMte)
			return nm==other.nm;
		return strcmp(ep_name,other.ep_name)==0;
	}
	void ReplaceContent(FeatureParamA &src)//用src中的内容更新自己的内容
	{
		at=src.at;
		var=src.var;
		vs.Copy(&src.vs);
		pathAnim=src.pathAnim;
	}

	//标识信息
	BYTE bMte;//表示这个param是否来自与mtrl feature
	union
	{
		char ep_name[MAX_EP_NAME];//bMte为0时有效
		//bMte为1时有效
		struct //注意这个结构的大小不能超过MAX_EP_NAME
		{
			StringID nm;
			KeyType kt;
			GSemCode code;
		};
	};

	//内容数据
	BYTE at;
	GVar var;
	ValueSet vs;
	std::string pathAnim;

	//comparing by ep_name
//	bool operator>(const FeatureParamA&src)
//	{
//		return (strcmp(ep_name,src.ep_name)>0);
//	}


	FeatureParamA& operator=(const FeatureParamA&src)
	{
		bMte=src.bMte;
		memcpy(ep_name,src.ep_name,sizeof(ep_name));
		var=src.var;
		at=src.at;
		vs.Copy(&((FeatureParamA&)src).vs);
		pathAnim=src.pathAnim;
		return *this;
	}
};

#define MtrlDemandF_Warp 1	//单层扭曲效果
#define MtrlDemandF_WarpMultiLayor 2 //多层扭曲效果

struct MtrlDemand
{
	MtrlDemand()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	void MergeFrom(MtrlDemand &src)
	{
		flags|=src.flags;
	}
	DWORD flags;//MtrlDemandF_XXXXX
	DWORD reserved0;
	DWORD reserved1;
	DWORD reserved2;
};

//Material Capability
struct MtrlCap
{
	MtrlCap()
	{
		Zero();
	}
	void Zero()
	{
		flags=0;
	}
	DWORD flags;//MtrlDemandF_XXXXX

};

class CDataPacket;
struct MtrlData:public ResData
{
	DECLARE_CLASS(MtrlData);

	struct Lod
	{
		Lod()		{			Zero();		}
		~Lod()		{			Clear();		}
		void Zero()
		{
			demand.Zero();
			features="";
			unifeature="";
			slib="main";
			fps.clear();
			state.Zero();
		}
		void Clear()
		{
			Zero();
		}
		Lod &operator=(const Lod &src)
		{
			slib=src.slib;
			features=src.features;
			unifeature=src.unifeature;
			mte=src.mte;

			fps=src.fps;

			demand=src.demand;
			state=src.state;
			return *this;
		}


		//lib
		std::string slib;

		//the material features concerned
		std::string features;//format: f1,f2,f3,f4,f5,f6
		std::string unifeature;//unique feature
		std::string mte;//材质扩展的路径

		//params
		std::vector<FeatureParamA> fps;

		//demand
		MtrlDemand demand;

		//render state
		ShaderState state;

	};


	MtrlData();
	virtual ~MtrlData();
	void Zero();
	void Clear();



	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "mtl";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);

	virtual void CollectRefs(std::vector<std::string>&buf);

	std::vector<Lod>lods;
	

};

struct MtrlDataObj
{
	MtrlDataObj()
	{
		GConstructor();
	}
	BEGIN_GOBJ( MtrlDataObj, 1 );
	END_GOBJ();
	void Zero(BOOL bIntuitive)
	{
		md.Zero();
	}
	void Clear()
	{
		md.Clear();
	}
	void Copy(MtrlDataObj*src)
	{
		md.Copy(src->md);
	}
	BOOL Load(CDataPacket &dp)
	{
		md.Load(dp);
		return TRUE;
	}
	void Save(CDataPacket &dp)
	{
		md.Save(dp);
	}

	void SaveDelta(CDataPacket &dp,MtrlDataObj*pRef)
	{
		Save(dp);
	}

	void LoadDelta(CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		Load(dp);
		if (ptrsDelta)
			ptrsDelta->push_back(this);
	}


	MtrlData md;

};