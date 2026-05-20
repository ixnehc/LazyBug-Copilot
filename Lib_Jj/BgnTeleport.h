#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

class CBgp_Teleport:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Teleport);

	virtual const char *GetTypeName()	{		return "单位瞬移";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (!((_modeFacing==1)&&(_varFace==StringID_Invalid)))
		{
			if (_idBuff==RecordID_Invalid)
				s="使用默认Buff传送至";
			else
				FormatString(s,"使用Buff[%s]传送至",assist->GetBuffName(_idBuff));
			if ((_loc!=StringID_Invalid)||(_route!=StringID_Invalid)||(_pos!=StringID_Invalid))
			{
				if (_pos)
					AppendFmtString(s,"位置变量 [%s] 处",StrLib_GetStr(_pos));
				else
				{
					if (_loc)
						AppendFmtString(s,"位点 [%s] 处",StrLib_GetStr(_loc));
					else
						AppendFmtString(s,"路线 [%s] 起始处",StrLib_GetStr(_route));
				}

				if (_modeFacing==0)
					s+="\n随机朝向";
				if (_modeFacing==1)
					AppendFmtString(s,"\n设定为[%s]中的朝向",StrLib_GetStr(_varFace));
			}
			if (_dur==0)
				s+="\n永久持续";
			else
				AppendFmtString(s,"\n持续%.2f秒",ANIMTICK_TO_SECOND(_dur));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Teleport,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(0.1f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0.0,100,0.1"),"Teleport的持续时间,0表示永久持续");

		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);
			GELEM_EDITVAR("传送Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");

		GELEM_VAR_INIT( StringID,_loc,StringID_Invalid);	
			GELEM_EDITVAR( "位置名称", GVT_U, GSem(GSem_StringID,"地图位置标签"), "什么位置");
		GELEM_VAR_INIT( StringID,_route,StringID_Invalid);	
			GELEM_EDITVAR( "路线名称", GVT_U, GSem(GSem_StringID,"路线名称"), "哪条路线的起始点");
		GELEM_BEHAVIORMEM_POS(_pos,"位置变量","什么位置")

		GELEM_VAR_INIT( int,_modeFacing,0);	
			GELEM_EDITVAR("朝向模式",GVT_S,GSem(GSem_Interger,
			"默认朝向:2"		"|朝向变量,"
			"随机朝向:0"		"|朝向变量,"
			"指定朝向:1"		""
			),"朝向模式");

		GELEM_BEHAVIORMEM_NUMBER(_varFace,"朝向变量","朝向变量");

		GELEM_VAR_INIT(BOOL,_bWaitFinish,TRUE);
			GELEM_EDITVAR("是否等待Teleport结束",GVT_S,GSem_Boolean,"是否等待Teleport结束");
    END_GOBJ();    

public: //当作protected

	AnimTick _dur;
	RecordID _idBuff;

	StringID _loc;
	StringID _route;
	StringID _pos;

	int _modeFacing;
	StringID _varFace;

	BOOL _bWaitFinish;

};


class CBgn_Teleport:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Teleport);

	CBgn_Teleport()
	{
		_idBuff=LevelBuffID_Invalid;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	RecordID _GetBuffRecID();
	LevelBuffID _idBuff;
};
