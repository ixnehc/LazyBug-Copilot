#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_Siege:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Siege);

	virtual const char *GetTypeName()	{		return "包围";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
			STUB_OUT(2,"中断");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

		if (nmVar!=StringID_Invalid)
		{
			FormatString(s,"尝试包围变量[%s]中的单位%.2f秒,移动速度为:%.2f米/秒",assist->GetStr(nmVar),ANIMTICK_TO_SECOND(dur),speed);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Siege,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(nmVar,"变量名称","对哪个变量中的单位进行包围")
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(4.0f));
			GELEM_EDITVAR("包围持续多长时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"包围持续多长时间,单位为秒");
		GELEM_VAR_INIT(float,speed,2.0f);
			GELEM_EDITVAR("包围时的移动速度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"包围时的移动速度");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("包围的Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"包围的Buff");
    END_GOBJ();    

public: //当作protected

	StringID nmVar;
	AnimTick dur;
	float speed;
	RecordID idBuff;
};

struct AttrNodeBase;
class CBgn_Siege:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Siege);

	CBgn_Siege()
	{
		_target=NULL;
		_attrnode=NULL;
		_bRight=0;
		_idBuff=LevelBuffID_Invalid;
		_bSiegePos=0;

		_radius=0.0f;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	void _Finish();

	BOOL _Step(BOOL bFirstStep);

	CLevelObj *_target;
	AttrNodeBase *_attrnode;
	LevelBuffID _idBuff;

	LevelPos _posTargetRecent;
	LevelPos _posSiege;

	AnimTick _tStart;

	BYTE _bRight:1;//往哪边包围
	BYTE _bSiegePos:1;//_posSiege是否有效

	float _radius;

};

