#pragma once

#include <string>
#include <vector>
#include "../math/matrix43.h"
#include "../math/vector3d.h"
#include "../math/vector2d.h"
#include "../math/quaternion.h"

#include "anim/animbase.h"

#include "anim/KeySet.h"
#include "anim/AnimPiece.h"

#include "ResData.h"
#include "MeshData.h"

#include "datapacket/DataPacket.h"
#include "../class/class.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)

struct KeySet;
struct AnimData:public ResData
{
	virtual KeySet *GetKeySet()=0;

	void GetAPTickRange(DWORD iAP,AnimTick &start,AnimTick &end);
	void MakeDefaultAP();//如果animpieces为空,生成一个名叫[Default]的animpiece,指定整段动画

	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);

	virtual void SaveHeader(CDataPacket &dp)
	{
	}
	virtual void LoadHeader(CDataPacket &dp)
	{
	}

	virtual WORD GetVer()	{		return 2;	};

	void Load(CDataPacket &dp,WORD &ver);

	std::vector<AnimPiece> animpieces;
};

template <KeyType TKeyType>
struct TAnimData:public AnimData
{
	TAnimData()
	{
		KeySet_Define(&keyset,TKeyType);
	}
	virtual KeySet *GetKeySet()	{		return (KeySet *)&keyset;	}

	virtual void Save(CDataPacket &dp)
	{
		AnimData::Save(dp);
		keyset.Save(dp);
	}
	virtual void Load(CDataPacket &dp)
	{
		WORD ver;
		AnimData::Load(dp,ver);
		if (ver==0)
			keyset.LoadOld(dp);
		else
			keyset.Load_(dp);
	}

	KeySet keyset;
};

struct XFormData:public TAnimData<KT_XForm>
{
	DECLARE_CLASS(XFormData);
	XFormData()
	{
		bCircular=0;
		reserved=0;
		distTotal=0.0f;
	}

	struct CtrlPointSampleInfo
	{
		CtrlPointSampleInfo()
		{
			distTotal=0.0f;
			tTotal=0.0f;
		}
		std::vector<float>cptimes;
		float distTotal;
		float tTotal;
	};

	void Clean();

	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);

	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "pth";	}
	virtual void CalcContent(std::string &s);

	void CalcFromCPs(CtrlPointSampleInfo *info=NULL);//info用来接收计算过程中产生的控制点及sample的相关信息
	void Compress();
	void Compress2();

	float distTotal;//这个值主要为CalcContent(..)服务


	struct CtrlPoint
	{
		CtrlPoint()
		{
			bVelocityAligned=1;
			vel=10.0f;
			reserved=0;
		}
		i_math::xformf xfm;
		DWORD bVelocityAligned:1;//表示这个CtrlPoint的旋转方向与速度方向是否一致
		DWORD reserved:31;
		float vel;
	};


	std::vector<CtrlPoint> cps;
	union 
	{
		DWORD flagsCP;
		struct
		{
			DWORD bCircular:1;
			DWORD reserved:31;
		};
	};


};

struct BonesData2 :public ResData
{	
	struct BonePos
	{
		DWORD sPos;
		DWORD sScale;
		DWORD sRot;	
	};

	struct BoneAnimPiece
	{
		void SaveHeader(CDataPacket &dp)
		{
			DP_WriteVar(dp,name);
			dp.Data_NextDword() = tStart;
			dp.Data_NextDword() = tEnd;

		}

		void LoadHeader(CDataPacket &dp)
		{
			DP_ReadVar(dp,name);
			tStart = dp.Data_NextDword();
			tEnd = dp.Data_NextDword();
		}

		void Save(CDataPacket &dp)
		{
			SaveHeader(dp);

			DP_WriteVector(dp,events);
			dp.Data_NextFloat() = params[0];
			dp.Data_NextFloat() = params[1];

			DP_WriteVector(dp,bones);
		}

		void Load(CDataPacket &dp,DWORD ver)
		{
			LoadHeader(dp);

			DP_ReadVector(dp,events);
			params[0] = dp.Data_NextFloat();
			params[1] = dp.Data_NextFloat();

			DP_ReadVector(dp,bones);
		}
		
		StringID name;

		//tStart/tEnd describes a range(inclusive) in a KeySet
		AnimTick tStart;
		AnimTick tEnd;//if ANIMTICK_INFINITE,index to the end 

		std::vector<AnimEvent> events;//the tick in events are always 0-based

		float params[2];

		std::vector<BonePos> bones; //记录每根骨骼的位置
	};
	
	BonesData2(void);
	~BonesData2(void);
	
	DECLARE_CLASS(BonesData2);
	
	virtual DWORD GetVersion() {return 2;}
	virtual ResType GetType(){return  ResA_Bones2;}
	virtual const char *GetTypeName(){return "ResA_Bones2";}
	virtual const char *GetTypeSuffix()	{		return "ba2";	}
	virtual void CalcContent(std::string &s){}
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);
	
	void GetTickRange(AnimTick &tStart,AnimTick &tEnd);

	SkeletonInfo skeleton;

	struct _Data{
		std::vector<i_math::vector3df> bufPos;
		std::vector<float> bufScale;
		std::vector<i_math::vector4ds> bufRot;	
		std::vector<Key_ref> ksPos;
		std::vector<Key_ref> ksRot;
		std::vector<Key_ref> ksScale;
	};

	struct _Data2{
		std::vector<Key_pos> keysPos;
		std::vector<Key_f> keysScale;
		std::vector<Key_s4> keysRot;
	};

	BYTE		dataType;	//根据数据的容量选择一个比较小的数据结构
	_Data		data1;		//当dataType :1
	_Data2		data2;		//当dataType :2
	
	std::vector<BoneAnimPiece> animpieces;
};


struct MtrlColorData:public TAnimData<KT_Color>
{
	DECLARE_CLASS(MtrlColorData);
	MtrlColorData(void);
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "ca";	}
	virtual void CalcContent(std::string &s);
	virtual void Load(CDataPacket &dp);
	virtual void Save(CDataPacket &dp);
	DWORD GetVersion(){return 1;}
	BOOL bOpacity;
	BOOL bDiffuse;	
};

struct MapCoordData:public TAnimData<KT_MapCoord>
{
	DECLARE_CLASS(MapCoordData);
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "uva";	}
	virtual void CalcContent(std::string &s);
	DWORD   GetVersion(){return 1;}
	virtual void Load(CDataPacket &dp);
	virtual void Save(CDataPacket &dp);
};



