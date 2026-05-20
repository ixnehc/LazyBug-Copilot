#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_飞妖_更新Shuttle:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_飞妖_更新Shuttle);

	virtual const char *GetTypeName()	{		return "飞妖_更新Shuttle";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID(CBgp_飞妖_更新Shuttle,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_OBJID(varTarget,"目标对象","目标对象");
		GELEM_BEHAVIORMEM_NUMBER(varRotSpd,"旋转速度","旋转速度");

		GELEM_VAR_INIT(float,durAdjustMin,1.0f);
			GELEM_EDITVAR("最小调整周期",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"最小调整周期");
		GELEM_VAR_INIT(float,durAdjustMax,2.0f);
			GELEM_EDITVAR("最大调整周期",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"最大调整周期");
		GELEM_VAR_INIT(float,accRot,180.0f);
			GELEM_EDITVAR("旋转加速度",GVT_F,GSem(GSem_Float,"0.1,1800.0,0.05"),"旋转加速度");
		GELEM_VAR_INIT(float,spdRotMax,360.0f);
			GELEM_EDITVAR("最大旋转速度",GVT_F,GSem(GSem_Float,"0.1,1800.0,0.05"),"最大旋转速度");
		GELEM_VAR_INIT(float,radiusMin,3.0f);
			GELEM_EDITVAR("目标位置离中心点的最小半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"目标位置离中心点的最小半径");
		GELEM_VAR_INIT(float,radiusMax,7.0f);
			GELEM_EDITVAR("目标位置离中心点的最大半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"目标位置离中心点的最大半径");
		GELEM_VAR_INIT(float,distMin,4.0f);
			GELEM_EDITVAR("目标位置离自己的最短距离",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"目标位置离自己的最短距离");
// 		GELEM_VAR_INIT(float,spdCenterFollow,3.0f);
// 			GELEM_EDITVAR("中心点移动速度",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"中心点移动速度");

    END_GOBJ();    

public: //当作protected

	StringID varTarget;

	StringID varRotSpd;

 	float durAdjustMin;
	float durAdjustMax;
	float accRot;
	float spdRotMax;
	float radiusMin;
 	float radiusMax;
	float distMin;


};


class CBgn_飞妖_更新Shuttle:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_飞妖_更新Shuttle);

	CBgn_飞妖_更新Shuttle()
	{
		_tLastUpdate=0;
		_tNextAdjust=0;


		_spdRot_Cur=0.0f;
		_spdRot_Target=0.0f;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;

protected:

	AnimTick _tLastUpdate;
	AnimTick _tNextAdjust;

	LevelFace _spdRot_Cur;
	LevelFace _spdRot_Target;

	void _UpdateCenter(BOOL bFirst);
	LevelPos _posCenter;

};

