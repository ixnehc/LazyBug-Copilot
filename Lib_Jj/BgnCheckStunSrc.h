#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelEvents.h"




class CBgp_CheckStunSrc:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckStunSrc);

	virtual const char *GetTypeName()	{		return "检测硬直";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"检测到");
			STUB_OUT(2,"未检测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBVRDesc_AnimTick(AnimTick v,StringID nmRef,FillDescAssist *assist);

		if (!bWait)
			FormatString(s,"检测%s秒内的硬直",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
		else
			FormatString(s,"持续检测%s秒内的硬直",GetBVRDesc_AnimTick(BVR_ARG(durGap),assist));
		if (idSkillBreak!=RecordID_Invalid)
			AppendFmtString(s,"\n要求打断技能:%s",assist->GetSkillName(idSkillBreak));
		if(idsSkillStageBreak.size()>0)
		{
			s+="\n要求打断技能阶段为:";
			for (int i=0;i<idsSkillStageBreak.size();i++)
			{
				if (i==0)
					AppendFmtString(s,"(%s)",assist->GetStr(idsSkillStageBreak[i]));
				else
					AppendFmtString(s," 或 (%s)",assist->GetStr(idsSkillStageBreak[i]));
			}
		}
		if (count>0)
			AppendFmtString(s,"\n要求连锁次数为%d",count);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckStunSrc,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(AnimTick,durGap,ANIMTICK_FROM_SECOND(0.5f));GELEM_UID(1);
			GELEM_EDITVAR("多长时间之前",GVT_U,GSem(GSem_AnimTick,"0,20,0.1"),"多长时间之前");
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idSkillBreak,RecordID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("打断技能",GVT_U,GSem(GSem_RecordID,"skills"),"打断技能");
		GELEM_VARVECTOR_INIT(StringID, idsSkillStageBreak,StringID_Invalid);GELEM_UID(3);
			GELEM_EDITVAR("打断技能阶段名称",GVT_U,GSem(GSem_StringID,"技能阶段"),"打断技能阶段名称");
		GELEM_VAR_INIT(DWORD,count,1);;GELEM_UID(4);
			GELEM_EDITVAR("连锁次数",GVT_U,GSem(GSem_Interger,"任意次:0,1:1,2:2,3:3,4:4,5:5,6,7,8,9"),"连锁次数");
		GELEM_VAR_INIT(BOOL,bWait,TRUE);;GELEM_UID(5);
			GELEM_EDITVAR("持续检测",GVT_S,GSem_Boolean,"持续检测直至检测到");
	END_GOBJ();    

public: //当作protected

	RecordID idSkillBreak;
	std::vector<StringID> idsSkillStageBreak;
	DWORD count;

	DEFINE_BVR(AnimTick,durGap);
	BOOL bWait;


};


class CBgn_CheckStunSrc:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckStunSrc);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	BOOL _Update(BGNOutputs &outputs);
};
