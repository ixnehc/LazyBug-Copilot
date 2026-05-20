#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "records/recordsdefine.h"

#include "LevelDefines.h"

#include "LevelRecordEO.h"

#include "LevelObj.h"

#include "LevelOSB.h"

#include "LevelOps.h"

#define DEFINE_EOPARAM_CLASS(clss)											\
	DEFINE_CLASS(clss);																	\
	virtual CClass*GetEoClass()																				\
	{																																\
		extern CClass *GetEoClass_##clss();																				\
		return GetEoClass_##clss();																				\
	}


#define BIND_EOPARAM(clssEo,clssEoParam)													\
	CClass *GetEoClass_##clssEoParam()																				\
	{																													\
		return Class_Ptr2(clssEo);																\
	}																													\
	CClass *GetClass_##clssEoParam()														\
	{																													\
		return Class_Ptr2(clssEoParam);															\
	}


struct LevelRecordEo;
struct LevelOp_EoBirth;

struct LevelEoParam
{
	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;
	virtual CClass*GetEoClass()=0;
};

struct LevelEoDetectHitArg
{
	LevelEoDetectHitArg()
	{
		radius=0.2f;
		fall=1.0f;
		idIgnore=idSpecify=LevelObjID_Invalid;
		bUnit=TRUE;
		bAgent=FALSE;
	}
	float radius;
	float fall;
	LevelObjID idIgnore;
	LevelObjID idSpecify;
	BOOL bUnit;
	BOOL bAgent;
};

class CLevelSkill;
class CLevelObjHistory;
class CLevelBehavior;
class CLoEffectObj:public CLevelObj
{
public:

	CLoEffectObj()
	{
		Zero();
	}
	void Zero()
	{
		_rec=NULL;
		_grd=0;
		_tCreate=0;

		_eZone=NULL;

		_idSrcOwner=LevelObjID_Invalid;

		_opBirth=NULL;

		_bhv=NULL;
	}

	
	virtual LevelObjType GetType()	 override{		return LevelObjType_Eo;	}
	virtual LevelObjID GetRootOwnerID() override	{		return _idSrcOwner;	}
	virtual CLevelSkill *GetRootSkill() override;


	void PostCreate(LevelPlayerID idPlayer,CRecord *rec,i_math::xformf &xfmInital,AnimEventZone *eZone,LevelGrade grd,LevelOSB &osb,LevelOpLink &link);
	void PostCreate(LevelPlayerID idPlayer,RecordID idRec,i_math::xformf &xfmInital,AnimEventZone *eZone,LevelGrade grd,LevelOSB &osb,LevelOpLink &link);
	void PostCreate(LevelPlayerID idPlayer,RecordID idRec,LevelPos&pos,LevelPos &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link);
	void PostCreate(LevelPlayerID idPlayer,RecordID idRec,LevelPos3D&pos,LevelPos3D &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link);
	void PostCreate(LevelPlayerID idPlayer,CRecord *rec,LevelPos&pos,LevelPos &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link);
	void PostCreate(LevelPlayerID idPlayer,CRecord *rec,LevelPos3D&pos,LevelPos3D &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link);
	virtual void OnDestroy();

	void SetHost(LevelObjID idHost)	{		_idHost=idHost;	}
	LevelObjID GetHost()	{		return _idHost;	}

	LevelPos3D GetInitialPos3D()	{		return _GetInitialPos3D();	}

	LevelPos3D GetInitialDir3D()	{		return _GetInitialDir3D();	}

	virtual LevelPos GetFramePos()override	{		return _GetInitialPos();	}
	virtual LevelFace GetFrameFace() override	{		return LevelFaceFromQuat(_xfmInitial.rot);	}

	LevelRecordEo *GetRec()	{		return _rec;	}

	template <typename T>
	T *GetParam()
	{
		if (!_rec)
			return NULL;
		return _rec->GetParam<T>();
	}

	CLevelBehavior *GetBhv()	{		return _bhv;	}
	CBehaviorMem *GetMem();


	virtual void WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void WriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void WriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void PostWriteSync();

	virtual void Update();

	virtual CLevelOps *GetOps()	{		return _NeedOps()?&_ops:NULL;	}

	virtual void SetObliterateArg(LevelObliterateArg &arg){}

	BOOL NeedOps()	{		return _NeedOps();	}

	CLevelObj **DetectRange(LevelPos &pos,float radius,DWORD &c)	{		return _DetectRange(pos,radius,c);	}
	CLevelObj *DetectFirstInRange(LevelPos &pos,float radius)	{		return _DetectFirstInRange(pos,radius);	}

	CLevelObj *DetectHit(i_math::line3df &line,LevelEoDetectHitArg &argHit)	{		return _DetectHit(line,argHit);	}
	CLevelObj *DetectHit_ShieldAmulet(i_math::line3df &line,LevelEoDetectHitArg &argHit);
	void DetectHits(i_math::line3df &line,LevelEoDetectHitArg &argHit,LevelObjHits &hits,CLevelObjHistory &history);

	AnimTick GetCreateTime()	{		return _tCreate;	}


protected:

	virtual void _OnPostCreate(){}
	virtual void _OnDetroy(){}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)	{	}
	virtual void _OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer){}
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer){}
	virtual void _OnPostWriteSync(){}

	virtual void _OnUpdate()	{	}

	virtual BOOL _NeedOps()	{		return FALSE;	}

	AnimTick _GetT();
	AnimTick _GetAge();

	CLevelObj **_DetectRange(LevelPos &pos,float radius,DWORD &c);
	CLevelObj **_DetectRange_ShieldAmulet(LevelPos &pos,float radius,DWORD &c);
	CLevelObj **_DetectInAll(DWORD &c);
	CLevelObj *_DetectFirstInRange(LevelPos &pos,float radius);
	CLevelObj *_DetectHit(i_math::line3df &line,LevelEoDetectHitArg &argHit);
	BOOL _CheckCanHit(CLevelObj *lo,LevelEoDetectHitArg &argHit);

	void _MakeDeals(LevelPos3D &pos,DealArg&arg)	{		_MakeDeals(LevelOSB(this),pos,arg);	}
	void _MakeDeals(CLevelObj *loTarget,DealArg&arg)	{		_MakeDeals(LevelOSB(this),loTarget,arg);	}
	void _MakeRangeDeal(float radius,BOOL bIgnoreHost=FALSE)	{		_MakeRangeDeal(LevelOSB(this),radius,bIgnoreHost);	}
	void _MakeRangeDeal3D(float radius)	{		_MakeRangeDeal3D(LevelOSB(this),radius);	}

	void _MakeDeals(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg);
	void _MakeDeals(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg);
	void _MakeRangeDeal(LevelOSB &osbSrc,float radius,BOOL bIgnoreHost=FALSE);
	void _MakeRangeDeal(LevelOSB &osbSrc,float radius,DealArg &arg,BOOL bIgnoreHost=FALSE);
	void _MakeRangeDeal3D(LevelOSB &osbSrc,float radius);
	void _MakeZoneDeal(LevelOSB &osbSrc,DealArg &arg,BOOL bIgnoreHost=FALSE);

	CLevelObj *_GetOwner();
	CLevelSkill *_GetOwnerSkill();
	LevelSkillID _GetRootSkillID();
	AnimTick _GetSkillCastingTime();//得到OwnerSkill的casting时间,注意这个时间是根据IAS修正过的.如果失败,返回ANIMTICK_INFINITE
	BOOL _CheckSkillCastingEvent(StringID nmEvent);
 	AnimTick _GetSkillCastingEventTime(StringID nmEvent);//返回ANIMTICK_INFINITE表示失败
	BOOL _GetSkillCastingXfm(i_math::xformf &xfm);
	LevelSkillTarget* _GetSkillTarget();
	BOOL _CalcEZoneInfo(AnimEventZone::KeyFan &kFan,i_math::vector3df &pos,i_math::vector3df &dir,float &fov);

	LevelPos _GetInitialPos()	{		return _xfmInitial.pos.getXZ();	}
	LevelPos _GetInitialDir();
	LevelPos3D _GetInitialPos3D()	{		return _xfmInitial.pos;	}
	LevelPos3D _GetInitialDir3D();

	AnimTick _tCreate;

	i_math::xformf _xfmInitial;
	AnimEventZone *_eZone;

// 	LevelPos _posInitial;
// 	LevelPos _dirInitial;

	LevelGrade _grd;

	LevelRecordEo * _rec;

	LevelOp_EoBirth *_opBirth;

	LevelOSB _owner;
	LevelObjID _idSrcOwner;

	LevelObjID _idHost;//宿主

	//用于同步的SkillOps
	CLevelOps _ops;

	//Behavior
	CLevelBehavior *_bhv;

};
