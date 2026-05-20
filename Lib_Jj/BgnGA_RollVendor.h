#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"


struct RollVendorParam
{
	std::vector<i_math::matrix43f> sites;
	int nCandidates;
	float radiusDetect;

	RecordID idVendor;

	BEGIN_GOBJ_PURE(RollVendorParam,1);

		GELEM_VARVECTOR(i_math::matrix43f,sites)
			GELEM_EDITVAR("出现位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"出现位点");

		GELEM_VAR_INIT(DWORD,nCandidates,1);
			GELEM_EDITVAR("选几个",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"选几个");

		GELEM_VAR_INIT(float,radiusDetect,10.0f);
			GELEM_EDITVAR("感应范围",GVT_F,GSem(GSem_Float,"0.1,40.0,0.05"),"感应到Player的范围");

		GELEM_VAR_INIT(RecordID,idVendor,RecordID_Invalid);
			GELEM_EDITVAR("VendorID",GVT_U,GSem(GSem_RecordID,"agents"),"VendorID");
	END_GOBJ();

};


class CBgpGA_RollVendor:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollVendor);

	virtual const char *GetTypeName()	{		return "随机Vender";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_RollVendor,1);

		GELEM_OBJ(RollVendorParam,param);
			GELEM_EDITOBJ("产生Vender参数","产生Vender参数");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RollVendorParam,param);


};


class CBgnGA_RollVendor:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollVendor);

	CBgnGA_RollVendor()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

