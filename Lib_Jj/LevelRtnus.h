#pragma once

#include "class/class.h"

#include "LevelRtnuDefines.h"

#include "LevelObj.h"


class CLoUnit;
class CLevelPlayer;
struct LPSRetinueData;
class CBehaviorPersist;
class CLevelBehavior;
class CLevelRtnu
{
public:
	DEFINE_CLASS(CLevelRtnu)
	CLevelRtnu()
	{
		Zero();
	}

	void Zero()
	{
		_loUnit=NULL;
		_uid=RetinueUID_Invalid;
		_idPlayer=LevelPlayerID_Invalid;
		_bhv=LevelRtnuBehavior_None;
	}

	//传入的lo由CLevelRetinue维护
	//bPersist表示这个Retinue是否要保存下来
	BOOL CreateNew(CLoUnit *lo,BOOL bPersist,CLevelPlayer *player);
	BOOL CreateFromData(LPSRetinueData *data,CLevelPlayer *player,LevelPos &pos);
	BOOL CreateTeleport(CLevelRtnu *rtnuOrg,CLevelPlayer *player,LevelPos &pos);
	CLoUnit *Dismiss();
	void Discard();
	void Destroy();
	void Update();

	LevelRtnuRank GetRank();

	void SetBhv(LevelRtnuBehavior bhv)	{		_bhv=bhv;	}
	LevelRtnuBehavior GetBhv()	{		return _bhv;	}

	RetinueUID GetUID()	{		return _uid;	}

	CLoUnit *GetLo()	{		return _loUnit;	}

protected:
	void _CreateFromData(LPSRetinueData *data,CLoUnit *lo,CLevelPlayer *player);

	CLoUnit *_loUnit;
	RetinueUID _uid;//_uid不为空,表示这个Retinue是要保存下来的
	LevelPlayerID _idPlayer;//属于哪一个Player
	LevelRtnuBehavior _bhv;

	friend class CLevelRtnus;
};

struct LevelRecordSkill;
class CLevelRtnus
{
public:
	DEFINE_CLASS(CLevelRtnus);
	CLevelRtnus()
	{
		Zero();
	}
	void Zero()
	{
		_idxRtnusGC=0;
		_owner=NULL;
	}
	void Init(CLevelPlayer *player);
	void Clear();

	BOOL Add_New(CLoUnit *lo,BOOL bPersist);
	BOOL Add_FromData(LPSRetinueData *data,LevelPos &pos);
	BOOL Add_Teleport(CLevelRtnu *rtnuOrg,LevelPos &pos);
	void Remove(CLevelObj *lo);//使lo不再作为这个Player的Retinue,注意,这个函数不会Destroy这个lo
	void RemoveAll();//清除所有player的retinue,注意,这个函数不会Destroy这些retinue的level obj
	CLevelRtnu**GetRetinues(DWORD &c);//注意返回的LevelObj不保证都有效,使用时要检测一下
	CLevelRtnu**GetValidRetinues(DWORD &c);//返回的LevelObj保证都有效,但比较慢
	DWORD GetRetinueCount(RecordID idUnit);
	void FetchValidRetinues(std::vector<CLevelRtnu*> &rtnus)
	{
		_UpdateRetinuesGC();
		_rtnus.swap(rtnus);
		_rtnus.clear();
	}
	void AddCoSkillCharge(LevelRecordSkill *skill,LevelSkillGrade grd,LevelSkillTarget &target);
	void UpdateAI();

	void Update();

protected:
	void _UpdateRetinuesGC();

	std::vector<CLevelRtnu*> _rtnus;//随从
	DWORD _idxRtnusGC;
	CLevelPlayer *_owner;

};