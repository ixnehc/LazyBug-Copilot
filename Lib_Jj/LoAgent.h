#pragma once

#include "LevelObj.h"

#include "LevelOps.h"

#include "LevelObjSrc.h"



#define GELEM_AGENTRECORD()																				\
		GELEM_VAR_INIT(RecordID,_idRec,RecordID_Invalid);															\
			GELEM_EDITVAR("表格项",GVT_U,GSem(GSem_RecordID,"agents"),"关联的表格项");

#define GELEM_AGENTGUID()																							\
		GELEM_VAR_INIT(LevelGUID,_guid,LevelGUID_Invalid);															\
			GELEM_EDITVAR("GUID",GVT_U,GSem_GUID,"全局唯一标识符");


struct LosAgent:public CLevelObjSrc
{
};

struct AgentSrc
{
	DEFINE_CLASS(AgentSrc);
	AgentSrc()
	{
		rec=NULL;
	}
	void Write(CBitPacket *bp);
	i_math::matrix43f mat;
	LevelRecordAgent *rec;
};



struct SheetRecordAgent;
class CUnit;
class CLoAgent:public CLevelObj
{
public:
	CLoAgent()
	{
		_srcA=NULL;
		_unitGround=NULL;
	}

	virtual void OnDestroy();

	virtual BOOL IsServerOnly();

	virtual LevelObjType GetType()	{		return LevelObjType_Agent;	}
	virtual const char *GetShowName()	{		return "未知功能单位";	}
	virtual LevelGUID GetGUID()	override{		return _GetGUID();	}

	virtual LevelObjID GetRootOwnerID()	{		return GetID();	}
	virtual CLevelSkill *GetOwnerSkill()	{		return NULL;	}

	virtual LoMiscFlags*GetMiscFlags() override;

	virtual LevelPos GetFramePos() override	{		return _pos;	}
	virtual LevelPos3D GetFramePos3D() override;
	virtual void PostCreate();
	virtual void PostCreate(LevelPos &pos,float rad,RecordID idRec,LevelPlayerID idPlayer);
	virtual void PostCreate(i_math::matrix43f &mat,RecordID idRec,LevelPlayerID idPlayer);
	virtual CLevelOps *GetOps()	{		return &_ops;	}

	virtual float GetInvokeRange();
	virtual void Invoke(CLevelObj *loFrom)	{	}

	DWORD GetBuffID_Dead()override;

	void WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	void WriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void WriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void PostWriteSync()override;

	float GetRadius_() override;
	float GetHeight() override;

	virtual CUnit *GetUnit()	{		return _unitGround;	}

	virtual LevelPos3D GetBriefCenter()	{		return GetFramePos3D();	}

	LevelRecordAgent *GetRec()	{		return _GetRec();	}
	i_math::matrix43f *GetMat()	{		return _GetMat();	}

protected:

	LevelRecordAgent *_GetRec();
	LevelGUID _GetGUID();
	i_math::matrix43f *_GetMat();

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)	{	}
	virtual void _OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer){}
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer){}
	virtual void _OnPostWriteSync(){}



	LevelPos _pos;

	CLevelOps _ops;

	AgentSrc *_srcA;//A代表Alternative,如果这个指针不为空,表示这个Agent是动态创建出来的(区别于根据地图信息创建的Agent)

	CUnit *_unitGround;

};


