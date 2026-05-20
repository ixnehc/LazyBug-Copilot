#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpThreat_FindAttackPos:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_FindAttackPos);

	virtual const char *GetTypeName()	{		return "寻找攻击Threat的位置";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_varPos!=StringID_Invalid)
		{
			static std::string s1,s2,s3;
			FormatString(s,"尝试%d次,找到攻击Theat的位置,存放在[%s]",_nTry,assist->GetStr(_varPos));
			if (_mode==0)
			{
				s1=GetBVRDesc_Float(BVR_ARG(_radiusMin),assist);
				s2=GetBVRDesc_Float(BVR_ARG(_radiusMax),assist);
				s3=GetBVRDesc_Float(BVR_ARG(_fov),assist);

				AppendFmtString(s,"\n离自己%s~%s米,FOV%s度以内",s1.c_str(),s2.c_str(),s3.c_str());

				s1=GetBVRDesc_Float(BVR_ARG(_distMinToThreat),assist);
				s2=GetBVRDesc_Float(BVR_ARG(_distMaxToThreat),assist);
				AppendFmtString(s,"\n离Threat%s米~%s米",s1.c_str(),s2.c_str(),assist);
				if (_bUnobstructedToMe)
					AppendFmtString(s,"\n要求与自己之间没有Static障碍");
				if (_bUnobstructedToThreat)
					AppendFmtString(s,"\n要求与Threat之间没有Static障碍");
				if (_radiusWalkable>=0.0f)
					AppendFmtString(s,"\n要求周围%.2f米内为可走区域",_radiusWalkable);
			}
			else
			{
				s1=GetBVRDesc_Float(BVR_ARG(_distMinToThreat),assist);
				s2=GetBVRDesc_Float(BVR_ARG(_distMaxToThreat),assist);
				s3=GetBVRDesc_Float(BVR_ARG(_fovThreat),assist);
				AppendFmtString(s,"\n离Threat%s米~%s米,FOV%s度以内",s1.c_str(),s2.c_str(),s3.c_str());

				if (_bUnobstructedToMe)
					AppendFmtString(s,"\n要求与自己之间没有Static障碍");
				if (_bUnobstructedToThreat)
					AppendFmtString(s,"\n要求与Threat之间没有Static障碍");
				if (_radiusWalkable>=0.0f)
					AppendFmtString(s,"\n要求周围%.2f米内为可走区域",_radiusWalkable);
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_FindAttackPos,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,_mode,0);GELEM_UID(8)
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"以自己为出发点找:0"		"|Threat的FOV,"
				"以Threat为出发点找:1"	"|FOV&最小半径&最大半径"
				),"模式");
		GELEM_BEHAVIORMEM_POS(_varPos,"[out]位置变量","找到的位置存放在哪里")
		GELEM_VAR_INIT(float,_radiusMin,3.0f);GELEM_UID(5)
			GELEM_EDITVAR("最小半径",GVT_F,GSem(GSem_Float,"0,200,0.1"),"自己为中心的最小半径");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_radiusMax,5.0f);GELEM_UID(4)
			GELEM_EDITVAR("最大半径",GVT_F,GSem(GSem_Float,"0,200,0.1"),"自己为中心的最大半径");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_fov,90.0f);GELEM_UID(3)
			GELEM_EDITVAR("FOV",GVT_F,GSem(GSem_Float,"0,360,0.1"),"以自己到Threat的方向为中心线的fov");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_fovThreat,90.0f);GELEM_UID(10)
			GELEM_EDITVAR("Threat的FOV",GVT_F,GSem(GSem_Float,"0,360,0.1"),"以自己到Threat的方向为中心线的fov");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_distMinToThreat,0.0f);GELEM_UID(2)
			GELEM_EDITVAR("离Threat的最小距离",GVT_F,GSem(GSem_Float,"0,200,0.1"),"离Threat的最小距离");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_distMaxToThreat,100.0f);GELEM_UID(9)
			GELEM_EDITVAR("离Threat的最大距离",GVT_F,GSem(GSem_Float,"0.1,200,0.1"),"离Threat的最大距离");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,_bUnobstructedToThreat,FALSE); GELEM_UID(1)
			GELEM_EDITVAR("要求Threat可达",GVT_S,GSem_Boolean,"要求与Threat之间没有Static障碍");
		GELEM_VAR_INIT(BOOL,_bUnobstructedToMe,FALSE); GELEM_UID(6)
			GELEM_EDITVAR("要求自己可达",GVT_S,GSem_Boolean,"要求与自己之间没有Static障碍");
		GELEM_VAR_INIT(float,_radiusWalkable,-1.0f);GELEM_UID(10)
			GELEM_EDITVAR("要求的可走区域半径",GVT_F,GSem(GSem_Float,"-1,200,0.1"),"要求的可走区域半径");
		GELEM_VAR_INIT(DWORD,_nTry,5);GELEM_UID(7)
			GELEM_EDITVAR("尝试次数",GVT_U,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:8"),"尝试几次");
		//Next GLEM_UID(11)

    END_GOBJ();    

public: //当作protected

	StringID _varPos;
	int _mode;
	DEFINE_BVR(float,_radiusMin);
	DEFINE_BVR(float,_radiusMax);
	DEFINE_BVR(float,_fov);
	DEFINE_BVR(float,_fovThreat);
	DEFINE_BVR(float,_distMinToThreat);
	DEFINE_BVR(float,_distMaxToThreat);
	float _radiusWalkable;
	BOOL _bUnobstructedToThreat;
	BOOL _bUnobstructedToMe;
	DWORD _nTry;
};


struct LevelRecordSkill;
class CBgnThreat_FindAttackPos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_FindAttackPos);

	CBgnThreat_FindAttackPos()
	{
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Destroy();

protected:


};

