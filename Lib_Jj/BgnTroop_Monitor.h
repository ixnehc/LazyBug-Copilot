#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpTroop_Monitor:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgpTroop_Monitor);

	enum Type
	{
		None,
		AllDead,
		HPRatio_AnyBelow,
	};

	virtual const char *GetTypeName()	{		return "监控Troop";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"OK");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=None)
		{
			std::string ss;
			switch(_tp)
			{
				case AllDead:
					ss="全部死亡";
					break;
				case HPRatio_AnyBelow:
					FormatString(ss,"任一单位HP低于%.1f%%",_ratioHP*100.0f);
					break;
				default:
					ss="";
			}

			if (!ss.empty())
				FormatString(s,"监控Troop(%s)内的(%s)\r\n直到%s",assist->GetStr(_troop),LevelTroopRankFlag_GetDesc(BVR_ARG(_flagsRank)),ss.c_str());
		}
	}

    BEGIN_GOBJ_PURE(CBgpTroop_Monitor,1);

		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","监控哪个Troop");

		GELEM_VAR_INIT(Type,_tp,AllDead);
			GELEM_EDITVAR("监控类型",GVT_S,GSem(GSem_Interger,"n/a,全部死亡,任一单位HP比率低于"),"监控的类型");

		GELEM_VAR_INIT(LevelTroopRankFlags,_flagsRank,LevelTroopRankFlag_All);
			GELEM_EDITVAR("职级",GVT_U,GSem(GSem_Flags,LevelTroopRankFlags_GSemContrains),"对哪些职级进行监控");
			GELEM_BVR();

		GELEM_VAR_INIT(float ,_ratioHP,0.5f);
			GELEM_EDITVAR("HP Ratio",GVT_F,GSem(GSem_Float,"0,1,0.01"),"HP 比率");

	END_GOBJ();    

public: //当作protected

	Type _tp;
	DEFINE_BVR(LevelTroopRankFlags,_flagsRank);
	float _ratioHP;
	StringID _troop;

};

class CBgnTroop_Monitor:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Monitor);
	CBgnTroop_Monitor()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

};
