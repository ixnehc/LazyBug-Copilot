#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LoSlatesA.h"

class LevelSlatesGroup:public CBehaviorMemObj
{
	DECLARE_CLASS(LevelSlatesGroup);

	BEGIN_GOBJ_PURE_UID(LevelSlatesGroup,1);

		GELEM_VARVECTOR(LevelSlateIdx,slates);
			GELEM_UID(1);

	END_GOBJ();

	std::vector<LevelSlateIdx> slates;
};

class CBgpSetupSlatesA_SetType_RandomPick:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesA_SetType_RandomPick);

	virtual const char *GetTypeName()	{		return "随机选择";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

		if (entries.size()>0)
		{
			if (grp!=RecordID_Invalid)
				FormatString(s,"在石板组{%s}内随机选择:",StrLib_GetStr(grp));
			else
				FormatString(s,"在所有石板内随机选择:");

			for (int i=0;i<entries.size();i++)
			{
				SlatesRandomPickEntryA &e=entries[i];
				if (e.tpsTarget.size()<=0)
				{
					if (e.tp!=LevelSlateType_None)
						AppendFmtString(s,"\n随机选择%d个石板设为[%s]",e.count,LevelSlateA_NameFromType(e.tp));
					else
						AppendFmtString(s,"\n随机选择%d个石板",e.count);
				}
				else
				{
					std::string ss;
					for (int j=0;j<e.tpsTarget.size();j++)
					{
						if (ss.empty())
							FormatString(ss,"%s",LevelSlateA_NameFromType(e.tpsTarget[j]));
						else
							AppendFmtString(ss,"/%s",LevelSlateA_NameFromType(e.tpsTarget[j]));
					}
					if (e.tp!=LevelSlateType_None)
						AppendFmtString(s,"\n随机选择%d个[%s]石板设为[%s]",e.count,ss.c_str(),LevelSlateA_NameFromType(e.tp));
					else
						AppendFmtString(s,"\n随机选择%d个[%s]石板",e.count,ss.c_str());
				}
			}
			if (result!=StringID_Invalid)
				AppendFmtString(s,"\n结果保存在变量{%s}中",assist->GetStr(result));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpSetupSlatesA_SetType_RandomPick,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,grp,StringID_Invalid);	GELEM_UID(20);
			GELEM_EDITVAR( "石板组名", GVT_U, GSem(GSem_StringID,"石板组名称"), "石板组名称" );
		GELEM_OBJVECTOR(SlatesRandomPickEntryA,entries); GELEM_UID(21);
			GELEM_EDITOBJ("选择什么石板","选择什么石板");
		GELEM_VAR_INIT( StringID,result,StringID_Invalid);GELEM_UID(22);
			GELEM_EDITVAR( "产生石板保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "产生的石板保存在哪个变量里");
    END_GOBJ();    

public: //当作protected

	StringID grp;
	std::vector<SlatesRandomPickEntryA> entries;
	StringID result;

};


class CBgnSetupSlatesA_SetType_RandomPick:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesA_SetType_RandomPick);

	CBgnSetupSlatesA_SetType_RandomPick()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
