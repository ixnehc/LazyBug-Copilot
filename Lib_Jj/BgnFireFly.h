#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_FireFly_CheckFleeing:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_FireFly_CheckFleeing);

	virtual const char *GetTypeName()	{		return "FireFly_检测Flee";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

    BEGIN_GOBJ_PURE_UID2(CBgp_FireFly_CheckFleeing,445,1);
		GELEM_BGP_BASE();
	END_GOBJ();    

public: //当作protected
};


class CBgn_FireFly_CheckFleeing:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FireFly_CheckFleeing);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgp_FireFly_GetFleePos:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_FireFly_GetFleePos);

	virtual const char *GetTypeName()	{		return "FireFly_得到Flee位置";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_varPos!=StringID_Invalid)
		{
			s="(FireFly)得到Flee位置";
		}
	}

	BEGIN_GOBJ_PURE_UID2(CBgp_FireFly_GetFleePos,446,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_POS(_varPos,"[out]位置变量","找到的位置存放在哪里")
	END_GOBJ();    

public: //当作protected

	StringID _varPos;
};


class CBgn_FireFly_GetFleePos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FireFly_GetFleePos);

	CBgn_FireFly_GetFleePos()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};

struct FireFlyGuide
{
	std::vector<i_math::matrix43f> mats;

	BEGIN_GOBJ_PURE(FireFlyGuide,1);

	GELEM_VARVECTOR(i_math::matrix43f,mats)
		GELEM_EDITVAR("位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"位点");

	END_GOBJ();

};




class CBgpGA_AbsorbFireFly:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_AbsorbFireFly);

	virtual const char *GetTypeName()	{		return "FireFly_吸收";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_AbsorbFireFly,447,1);

		GELEM_OBJVECTOR(FireFlyGuide,guides)
			GELEM_EDITOBJ("FireFly导引信息","FireFly导引信息");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(std::vector<FireFlyGuide>,guides);
};


class CBgnGA_AbsorbFireFly:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_AbsorbFireFly);

	CBgnGA_AbsorbFireFly()
	{
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:



};


class CBgpGA_FireFlyTorchSpawn:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_FireFlyTorchSpawn);

	virtual const char *GetTypeName()	{		return "FireFly_火炬Spawn";	}
	virtual DWORD GetStubCount()
	{
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_FireFlyTorchSpawn,448,1);

		GELEM_OBJVECTOR(FireFlyGuide,guides)
			GELEM_EDITOBJ("FireFly导引信息","FireFly导引信息");
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位ID",GVT_U,GSem(GSem_RecordID,"units"),"单位ID");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("BuffID",GVT_U,GSem(GSem_RecordID,"buffs"),"BuffID");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(std::vector<FireFlyGuide>,guides);
	RecordID idUnit;
	RecordID idBuff;
};


class CBgnGA_FireFlyTorchSpawn:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_FireFlyTorchSpawn);

	CBgnGA_FireFlyTorchSpawn()
	{
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;
	void Update(BGNOutputs &outputs) override;

protected:

	void _Spawn();
	BOOL _CheckExistInTorch();

	LevelObjID _idCur;


};

