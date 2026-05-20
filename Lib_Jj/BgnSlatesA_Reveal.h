#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSlatesA_Reveal:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSlatesA_Reveal);

	enum Op
	{
		Op_None=0,
		Op_RevealNearBy,
		Op_RevealAll,

		Op_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "Reveal周围石板";	}
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
		if (_op==Op_RevealNearBy)
		{
			FormatString(s,"Reveal周围%d层的石板",_radius);
		}
		if (_op==Op_RevealAll)
		{
			s="Reveal所有";
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpSlatesA_Reveal,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(Op,_op,Op_RevealNearBy);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,
				"Reveal周围石板:1"		","
				"Reveal所有石板:2"	"|周围层数"
				),"操作");
		GELEM_VAR_INIT(DWORD,_radius,2);
			GELEM_EDITVAR("周围层数",GVT_U,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"层数");
    END_GOBJ();    

public: //当作protected

	Op _op;
	int _radius;

};


class CBgnSlatesA_Reveal:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSlatesA_Reveal);

	CBgnSlatesA_Reveal()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
