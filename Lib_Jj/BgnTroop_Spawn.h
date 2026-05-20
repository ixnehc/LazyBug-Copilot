#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelTroops.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"


struct SpawnTroopParam
{
	BEGIN_GOBJ_PURE(SpawnTroopParam,1);

		GELEM_VAR_INIT( float,speed,0.0f);	
			GELEM_EDITVAR( "Spawn速度", GVT_F, GSem(GSem_Float,"0,100,0.05"), "每秒召唤几个单位,0表示无限个" );
		GELEM_VAR_INIT(BOOL,bRandomRot,TRUE);
			GELEM_EDITVAR("随机旋转",GVT_S,GSem_Boolean,"随机旋转");
		GELEM_VARVECTOR(i_math::matrix43f,mats)
			GELEM_EDITVAR("位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"位点");
		GELEM_VAR_INIT( float,radiusMin,8.0f);	
			GELEM_EDITVAR( "最小范围", GVT_F, GSem(GSem_Float,"0,100,0.05"), "最小范围(未指定位点时有效)" );
		GELEM_VAR_INIT( float,radiusMax,12.0f);	
			GELEM_EDITVAR( "最大范围", GVT_F, GSem(GSem_Float,"0,100,0.05"), "最大范围(未指定位点时有效)" );

	END_GOBJ();

	float speed;
	BOOL bRandomRot;
	std::vector<i_math::matrix43f> mats;
	float radiusMin;
	float radiusMax;
};


class CBgpTroop_Spawn:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_Spawn);

	virtual const char *GetTypeName()	{		return "创建Troop单位";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
// 		s="n/a";
// 		if(_param.bRef&&_param.nmRef!=StringID_Invalid)
// 		{
// 			FormatString(s,"参数[%s]\r\n,",assist->GetStr(_param.nmRef));
// 		}
// 		if (_troop!=StringID_Invalid)
// 			AppendFmtString(s,"创建Troop[%s]的单位",assist->GetStr(_troop));
	}

    BEGIN_GOBJ_PURE(CBgpTroop_Spawn,1);

		GELEM_OBJ(SpawnTroopParam,_param);
			GELEM_EDITOBJ("创建参数","创建参数");
			GELEM_BVR();
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","创建哪个Troop的单位");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(SpawnTroopParam,_param);
	DEFINE_BVR(StringID,_troop);

};


class CBgnTroop_Spawn:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Spawn);

	CBgnTroop_Spawn()
	{
		_nSpawned=0;
		_tStart=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

	struct SpawnInfo
	{
		SpawnInfo()
		{
			memset(this,0,sizeof(*this));
		}
		LevelTroopDesc *desc;
		LevelPos pos;
		LevelFace face;
		float hang;
	};

protected:

	void _Spawn(CLevel *level,CLevelTroop *troop,SpawnInfo &info);

	std::deque<SpawnInfo> _infos;
	std::vector<short>_indices;
	DWORD _nSpawned;
	AnimTick _tStart;

};

