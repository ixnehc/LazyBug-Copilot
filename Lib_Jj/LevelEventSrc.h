#pragma once

#include "commondefines/general_stl.h"

#include "LevelDefines.h"
#include "LevelEvents.h"

#include "ringbuf/RingBuf.h"

class CLevelBuff;
class CLevelEventSrc
{
public:
	DEFINE_CLASS(CLevelEventSrc);
	struct EventSrc
	{
		LevelEventType tp;
		LevelObjID id;
		AnimTick t;
	};

	struct StunSrc
	{
		StunSrc()
		{
			memset(this,0,sizeof(*this));
		}
		BOOL IsFinished()
		{
			return owner==NULL;
		}
		RecordID idBrokenSkill;//打断了哪个技能
		StringID idBrokenSkillStage;//技能处于哪个stage
		CLevelBuff*owner;
		DWORD count;//连锁次数
		AnimTick t;
	};
	void Add(LevelEventType tp,LevelObjID id,AnimTick t)
	{
		EventSrc v;
		v.tp=tp;
		v.id=id;
		v.t=t;
		srces.PushBack(v);
	}
	void AddStun(CLevelBuff *buff,RecordID idBrokenSkill,StringID idBrokenSkillStage,AnimTick t)
	{
		//看能否连锁
		StunSrc *recentToCombine=NULL;
		if (srcesStun.GetCount()>0)
		{
			StunSrc *src=&srcesStun.GetAt(srcesStun.GetCount()-1);
			if (!src->IsFinished())
				recentToCombine=src;
			else
			{
				if (src->t==t)//和上一个Stun结束的同一帧开始,我们连锁它们
					recentToCombine=src;
			}
		}

		if (recentToCombine)
		{
			recentToCombine->t=t;
			recentToCombine->owner=buff;
			recentToCombine->count++;
			return;
		}

		StunSrc srcNew;
		srcNew.idBrokenSkill=idBrokenSkill;
		srcNew.idBrokenSkillStage=idBrokenSkillStage;
		srcNew.t=t;
		srcNew.count=1;
		srcNew.owner=buff;
		srcesStun.PushBack(srcNew);
	}
	void NofityStunFinish(CLevelBuff *buff,AnimTick t)
	{
		StunSrc *recentUnfinished=NULL;
		if (srcesStun.GetCount()>0)
		{
			StunSrc *src=&srcesStun.GetAt(srcesStun.GetCount()-1);
			if (!src->IsFinished())
				recentUnfinished=src;
		}

		if (recentUnfinished)
		{
			if (recentUnfinished->owner==buff)
			{
				recentUnfinished->owner=NULL;
				recentUnfinished->t=t;
			}
		}

	}

	void Clear()
	{
		srces.Clear();
	}
	BOOL Exist(LevelEventType tp,LevelObjID id,AnimTick tAfter)//检测是否有在t之后的指定id的event src
	{
		int c=srces.GetCount();
		for (int i=0;i<c;i++)
		{
			EventSrc &v=srces.GetAt(i);
			if ((v.id==id)&&(v.tp==tp))
			{
				if (v.t>=tAfter)
					return TRUE;
			}
		}
		return FALSE;
	}

	BOOL Exist(LevelEventType tp,AnimTick tAfter)//检测是否有在t之后的damage src
	{
		int c=srces.GetCount();
		for (int i=c-1;i>=0;i--)
		{
			EventSrc &v=srces.GetAt(i);
			if (v.tp==tp)
			{
				if (v.t>=tAfter)
					return TRUE;
			}
		}
		return FALSE;
	}

	BOOL ExistWithMask(LevelEventTypeMask mask,AnimTick tAfter,LevelObjID *idRet)
	{
		int c=srces.GetCount();
		for (int i=c-1;i>=0;i--)
		{
			EventSrc &v=srces.GetAt(i);
			if (((DWORD)(1<<(DWORD)v.tp))&mask)
			{
				if (v.t>=tAfter)
				{
					if (idRet)
						*idRet=v.id;
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	BOOL ExistWithMask(LevelEventTypeMask mask,LevelObjID id,AnimTick tAfter)
	{
		int c=srces.GetCount();
		for (int i=c-1;i>=0;i--)
		{
			EventSrc &v=srces.GetAt(i);
			if (v.id==id)
			{
				if (((DWORD)(1<<(DWORD)v.tp))&mask)
				{
					if (v.t>=tAfter)
						return TRUE;
				}
			}
		}
		return FALSE;
	}

	BOOL ExistStun()//当前时刻有没有Stun
	{
		int c=srcesStun.GetCount();
		if (c>0)
		{
			StunSrc &v=srcesStun.GetAt(c-1);
			if (!v.IsFinished())
				return TRUE;
		}
		return FALSE;
	}

	BOOL ExistStun(int countStun,RecordID idBrokenSkill,std::vector<StringID> *nmsBrokenSkillStage,AnimTick tAfter)
	{
		int c=srcesStun.GetCount();
		for (int i=c-1;i>=0;i--)
		{
			StunSrc &v=srcesStun.GetAt(i);
			if ((!v.IsFinished())||(v.t>tAfter))
			{
				if (idBrokenSkill!=RecordID_Invalid)
				{
					if (v.idBrokenSkill!=idBrokenSkill)
						continue;
				}

				if (nmsBrokenSkillStage)
				{
					if (nmsBrokenSkillStage->size()>0)
					{
						int idx;
						VEC_FIND(*nmsBrokenSkillStage,v.idBrokenSkillStage,idx);
						if (idx==-1)
							continue;
					}
				}

				if ((v.count==countStun)||(countStun==0))
					return TRUE;
			}
		}
		return FALSE;
	}


protected:
	CRingBuf<EventSrc,4> srces;
	CRingBuf<StunSrc,4> srcesStun;


};
