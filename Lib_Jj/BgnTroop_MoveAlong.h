#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

struct AttrNodeFloat;
class CBgpTroop_MoveAlong:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_MoveAlong);

	virtual const char *GetTypeName()	{		return "移动Troop(沿路径)";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"命令Troop(%s)中的单位沿路径移动",GetBVRDesc_StringID(BVR_ARG(_troop),assist));
	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_MoveAlong,1);
		GELEM_OBJ(BccRoute,_route);
			GELEM_EDITOBJ("路径","路径");
			GELEM_BVR();
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","移动哪个Troop");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,_troop);
	DEFINE_BVR(BccRoute,_route);

};


class CBgnTroop_MoveAlong:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_MoveAlong);

	struct Entry
	{
		Entry()
		{
			lo=NULL;
			idxNode=0;
			tStartMove=0;
			bReached=FALSE;
			distToGo=0.0f;
			anSpeed=NULL;
		}
		CLevelObj *lo;
		AnimTick tStartMove;
		int idxNode;//正在走向route上的哪一个路点
		float distToGo;
		DWORD bReached:1;

		AttrNodeFloat *anSpeed;

	};

	CBgnTroop_MoveAlong()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	BOOL _CanControl(Entry &e);

	void _OccupyTroopControl();
	void _DiscardTroopControl();

	void _StartMove(Entry &e,BccRoute *bccRoute,AnimTick t);

	std::vector<Entry> _entries;
	std::vector<float>_distsRoute;

};

