#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_ModVar_Obsolete:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_ModVar_Obsolete);

	virtual const char *GetTypeName()	{		return "修改变量(Obsolete";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (nm!=StringID_Invalid)
		{
			if(mode==2)
				FormatString(s,"变量(%s)设为%s",assist->GetStr(nm),GetBPRDesc(bprRef,assist));
			else
			{
				if (mode==0)
					FormatString(s,"变量(%s)+=%s",assist->GetStr(nm),GetBPRDesc(bprRef,assist));
				if (mode==1)
					FormatString(s,"变量(%s)-=%s",assist->GetStr(nm),GetBPRDesc(bprRef,assist));
				if (mode==3)
					FormatString(s,"变量(%s)*=%s",assist->GetStr(nm),GetBPRDesc(bprRef,assist));
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_ModVar_Obsolete,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_NUMBER(nm,"名称","行为图内存变量名称")
		GELEM_VAR_INIT(int,mode,0);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,"增加:0,减少:1,缩放:3,设置:2"),"使用何种模式修改变量");
		GELEM_BPR_INT(bprRef,0,"参考值","参考值");
    END_GOBJ();    

public: //当作protected

	StringID nm;
	BPR_Int bprRef;
	int mode;
};


class CBgn_ModVar_Obsolete:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ModVar_Obsolete);

	CBgn_ModVar_Obsolete()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgp_ModVar:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_ModVar);

	virtual const char *GetTypeName()	{		return "修改变量";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

    BEGIN_GOBJ_PURE_UID(CBgp_ModVar,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_ALL(nm,"名称","行为图内存变量名称")
		GELEM_VAR_INIT(int,mode,0);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,"增加:0,减少:1,缩放:3,设置:2,取反:4"),"使用何种模式修改变量");
		GELEM_VAR_INIT(float,f,0.0f);
			GELEM_EDITVAR("浮点数值",GVT_F,GSem_Float,"浮点数值");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,b,FALSE);
			GELEM_EDITVAR("布尔值",GVT_S,GSem_Boolean,"布尔值");
			GELEM_BVR();
		GELEM_VAR_INIT(int,n,0);
			GELEM_EDITVAR("整数值",GVT_S,GSem_Interger,"整数值");
			GELEM_BVR();
		GELEM_VAR_INIT( StringID,idStr,StringID_Invalid);	
			GELEM_EDITVAR("字符串ID", GVT_U, GSem(GSem_StringID,"变量值字符串"), "" );
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("Skill ID", GVT_U, GSem(GSem_RecordID,"skills"), "" );
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff ID", GVT_U, GSem(GSem_RecordID,"buffs"), "" );
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("Item ID", GVT_U, GSem(GSem_RecordID,"items"), "" );
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idRes,RecordID_Invalid);
			GELEM_EDITVAR("Resource ID", GVT_U, GSem(GSem_RecordID,"resources"), "" );
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("Unit ID", GVT_U, GSem(GSem_RecordID,"units"), "" );
			GELEM_BVR();
		GELEM_VAR_INIT(LevelObjID,idObj,LevelObjID_Invalid);
			GELEM_EDITVAR("游戏对象ID", GVT_U, GSem_Interger, "" );
			GELEM_BVR();
		GELEM_VAR_INIT(LevelGUID,idGUID,LevelGUID_Invalid);
			GELEM_EDITVAR("GUID", GVT_U, GSem_Interger, "" );
			GELEM_BVR();
			//XXXXX:more BehaviorMemType
    END_GOBJ();    

public: //当作protected

	StringID nm;
	int mode;

	DEFINE_BVR(float,f);
	DEFINE_BVR(BOOL,b);
	DEFINE_BVR(int,n);
	DEFINE_BVR(StringID,idStr);
	DEFINE_BVR(RecordID,idBuff);
	DEFINE_BVR(RecordID,idItem);
	DEFINE_BVR(RecordID,idUnit);
	DEFINE_BVR(RecordID,idSkill);
	DEFINE_BVR(RecordID,idRes);
	DEFINE_BVR(LevelObjID,idObj);
	DEFINE_BVR(LevelGUID,idGUID);
	//XXXXX:more BehaviorMemType
};


class CBgn_ModVar:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ModVar);

	CBgn_ModVar()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
