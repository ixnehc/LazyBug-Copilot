#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "records/recordsdefine.h"

#include "LevelObj.h" 

#include "BuffCalc.h"

class CLevelBuff;
inline void buff_verify(CLevelBuff*c) {}

#define DEFINE_BUFF_CLASS(clss,uid)															\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_LevelBuff;															\
		instance._uid=uid;																					\
		{clss *p=NULL;buff_verify(p);}																	\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;


#define DEFINE_BUFFPARAM_CLASS(clss)											\
	DEFINE_CLASS(clss);																	\
	virtual CClass*GetBuffClass()																				\
	{																																\
		extern CClass *GetBuffClass_##clss();																				\
		return GetBuffClass_##clss();																				\
	}																																	\
	virtual CClass*GetArgClass()																				\
	{																																\
		extern CClass *GetArgClass_##clss();																				\
		return GetArgClass_##clss();																				\
	}

#define BIND_BUFFPARAM(clssBuff,clssBuffParam,clssArg)													\
	CClass *GetBuffClass_##clssBuffParam()																				\
	{																													\
		return Class_Ptr2(clssBuff);																\
	}																													\
	CClass *GetClass_##clssBuffParam()														\
	{																													\
		return Class_Ptr2(clssBuffParam);															\
	}																													\
	CClass *GetArgClass_##clssBuffParam()														\
	{																													\
		return Class_Ptr2(clssArg);															\
	}


struct LevelBuffParam
{
	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;
	virtual CClass*GetBuffClass()=0;
	virtual CClass*GetArgClass()=0;
};


struct LevelBuffArg
{
	virtual CClass *GetClass()=0;
	template <typename T> 
	T* ToPtr()
	{
		if (IsClass2(this,T))
		{
			return (T*)this;
		}
		return NULL;
	}
};


struct LevelBuffData
{
	LevelBuffData()
	{
		id=LevelBuffID_Invalid;
	}
	BOOL IsEmpty()	{		return id==LevelBuffID_Invalid;	}

	WORD bClassUID;
	LevelBuffID id;
	WORD uid;//如果bClassUID是0,uid是一个SimpleRecordID,否则是一个ClassUID
	WORD dur;//1/10秒为单位,如果为0xffff,表示为无限时间
	BYTE szData;
	BYTE szBitsData;//in Byte
	BYTE data[MAX_BUFF_DATA];//最多MAX_BUFF_DATA个字节
	BYTE bits[MAX_BUFF_DATA];//最多MAX_BUFF_DATA个字节

	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);

};

class CBehaviorMem;
class CLevelBehavior;

class CLevel;
struct LevelRecordBuff;
class CLevelBuffs;
class CLevelOp;
struct LevelOp_AddBuff;
class CLevelReactors;
struct DealArg;
class CLevelBuff:public CBuffFactor
{
public:
	IMPLEMENT_REFCOUNT_C
	CLevelBuff()
	{
		Zero();
	}
	void Zero()
	{
		_id=LevelBuffID_Invalid;
		_buffs=NULL;
		_rec=NULL;
		_dur=0;
		_tUpdate=0;
		_tAge=0;
		_param=NULL;
		_bhv=NULL;
		_reactors=NULL;
	}

	enum ConflictResult
	{
		Conflict_None,
		Conflict_Replace,
		Conflict_Forbid,
	};

	virtual CClass *GetClass()=0;
	virtual BOOL NeedSync()	{		return FALSE;	}//是否需要同步给客户端(客户端是否要知道这个Buff),注意这个函数的返回值必须是固定不变的
	virtual BOOL NeedSyncTimeUp()	{		return FALSE;	}//时间到结束时是否需要同步给客户端,注意如果为TRUE,则这个Buff的_dur为ANIMTICK_INFINITE
	virtual BOOL NeedSyncTimeUpIP()	{		return FALSE;	}//对于Player来说(If Player),时间到结束时是否需要同步给客户端
	virtual LevelOp_AddBuff *AccuireSyncOp()	{		return NULL;	}//某些Buff在同步(主要是在FirstSync时)给客户端时,
																											//会希望以LevelOp_AddBuff的形式进行同步,这个函数检查这种情况
																											//注意返回的指针由Buff自己管理,外部不会负责删除这个指针
	virtual BOOL NeedSyncBigData()	{		return FALSE;	}//是否有大量数据要进行同步(超过MAX_BUFF_DATA)

	virtual BOOL Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)	{		return FALSE;	}
	virtual ConflictResult CheckConflict(CLevelBuff *buffExist);//

	virtual LevelBuffMask GetForbiddingBuffs(){ return 0 ;}//得到那些会阻止我的Buff
	virtual LevelBuffMask GetReplaceBuffs(){return 0 ;}//得到那些我可以取代的Buff

	virtual void HandleEvent(LevelEvent &e);

	//SkillPath
	virtual void StartSkillPath(LevelPos &pos,float ht)	{	}
	virtual void StopSkillPath()	{	}
	virtual BOOL CalcSkillPathXfm(AnimTick t,LevelPos &pos,float &ht,LevelFace &face)	{		return FALSE;	}

	LevelBuffID GetID()	{		return _id;	}
	BOOL IsAlive()	{		return _id!=LevelBuffID_Invalid;	}
	LevelRecordBuff *GetRec()	{		return _rec;	}
	LevelBuffParam *GetParam()	{		return _param;	}
	AnimTick GetDur()	{		return _dur;	}
	AnimTick GetAge()	{		return _tAge;	}

	CLevelBehavior *GetBhv()	{		return _bhv;	}
	CBehaviorMem *GetMem();

	void ToData(LevelBuffData &data);


	CLevelOp *NewOp(CClass *clss,LevelOpLink &link);

	template <typename T>
	T *NewOp()
	{
		return (T*)NewOp(Class_Ptr2(T),LevelOpLink());
	}

	template <typename T>
	T *NewOp(LevelOpLink &link)
	{
		return (T*)NewOp(Class_Ptr2(T),link);
	}



	void Create(CLevelBuffs *buffs,LevelBuffID id,LevelRecordBuff *rec,AnimTick dur,LevelBuffArg *arg);
	void Create_Teleport(CLevelBuffs *buffs,LevelBuffID id,LevelRecordBuff *rec,AnimTick dur,CLevelBuff *buffOrg);
	void Destroy();

	
	virtual BOOL CanTeleport();//返回这个Buff在所属的LevelObj进行Teleport时能否跟着一块Teleport
	virtual void LoadTeleport(CLevelBuff *buffOrg)	{	}//从buffOrg中得到可以Teleport的数据,填充到自己里面

	void SetDur(AnimTick dur)	{		_dur=dur;	}
	void MergeDur(AnimTick durNew);

	void Update(AnimTick dt);

	CLevel *GetLevel()	{		return _GetLevel();	}
	CLevelObj *GetOwner()	{		return _GetOwner();	}

protected:
	void _SetParam(LevelBuffParam *param)
	{
		_param=param;
	}

	virtual void _OnCreate(LevelBuffArg *param){}
	virtual void _OnDestroy(){}


	virtual void _OnUpdate(AnimTick dt){}

	virtual void _WriteData(CBitPacket *dp)	{}//最多写入MAX_BUFF_DATA个字节
	virtual void _WriteBigData(CDataPacket *dp)	{}

	void _MakeDeals(LevelPos3D &pos,DealArg&arg);
	void _MakeDeals(CLevelObj *loTarget,DealArg&arg);

	CLevelObj **_DetectRange(LevelPos &pos,float radius,DWORD &c);
	void _MakeRangeDeals(float radius);


	CLevelObj*_GetOwner();
	CLevel*_GetLevel();

	void _AddSyncDataOp();

	LevelBuffParam *_param;

	CLevelBuffs *_buffs;
	LevelRecordBuff *_rec;
	LevelBuffID _id;
	AnimTick _tUpdate;
	AnimTick _dur;//剩余的持续时间,如果为ANIMTICK_INFINITE,表示无限时间
	AnimTick _tAge;//已经持续多久了

	CLevelBehavior *_bhv;

	CLevelReactors *_reactors;


	friend class CLevelBuffs;
	friend class CGameBuff;

};

class CLevelBuffIDPool
{
public:
	CLevelBuffIDPool()
	{
		_seed=1;
	}
	void Init(CLevel *level)	
	{	
		_level=level;
	}
	void Clear()
	{
		_seed=1;
		_frees.clear();
	}
	LevelBuffID Alloc();
	void Free(LevelBuffID id);

	struct FreeNode
	{
		LevelTick t;
		LevelBuffID id;
	};

protected:
	CLevel *_level;

	LevelBuffID _seed;
	std::deque<FreeNode>_frees;

};

class CLevelBuffs:public CBuffFormular
{
public:
	CLevelBuffs()
	{
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
		_idpool=NULL;
		_bNeedFlushDead=FALSE;
	}
	void Init(CLevelObj*owner,CLevelBuffIDPool *idpool);
	void Init_Teleport(CLevelObj*owner,CLevelBuffIDPool *idpool,CLevelBuffs *buffs);
	void Clear();


	CLevelObj*GetOwner()	{		return _owner;	}
	CLevelBuffIDPool *GetIDPool()	{		return _idpool;	}

	CLevelBuff *CreateBuff(LevelRecordBuff *rec,AnimTick dur,LevelBuffArg *param);
	CLevelBuff *CreateBuff(CClass *clss,AnimTick dur,LevelBuffArg *param);
	CLevelBuff *CreateBuff_Teleport(CLevelBuff *buffOrg);
	void Update(AnimTick t);

	CLevelBuff *FindBuff(CClass *clssBuff);
	CLevelBuff *FindBuffByID(LevelBuffID idBuff);
	CLevelBuff *FindBuffByRecordID(RecordID idBuff);

	CLevelBuff **GetBuffs(DWORD &c)
	{
		c=_buffs.size();
		return _buffs.data();
	}


	void HandleEvent(LevelEvent &e);


	//读写FirstSync
	void WriteFirstSync(CBitPacket *bp);
	void WriteSyncL(CBitPacket *bp,BOOL &bContent);
	void PostWriteSync();

protected:
	void _FlushDead();

	void _WriteBigData(CLevelBuff *buff,CBitPacket *bp);

	std::vector<CLevelBuff*>_buffs;
	CLevelObj*_owner;
	CLevelBuffIDPool *_idpool;
	BOOL _bNeedFlushDead;

	friend class CLevelDecider;
};