
#pragma once

#include "../ref/ref.h"

typedef unsigned __int64 RocerMask;

typedef DWORD RocerType;
#define RocerType_Invalid 0xffffffff

typedef DWORD RocEvent;

struct RocerEntry
{
	DEFINE_CLASS(RocerEntry);
	RocerType type;
	Ref *refRocer;

	RocerEntry *next;
};



//Right of Control
struct Roc
{
	DEFINE_REF();

	Roc()
	{
		rocers=0;
		entries=NULL;
	}

	void Clear()
	{
		RocerEntry *e=entries;
		while(e)
		{
			SAFE_RELEASE(e->refRocer);
			RocerEntry *next=e->next;
			Class_Delete(e);
			e=next;
		}
		entries=NULL;

		BreakRef();

	}

	inline void SendEvent(RocEvent re);

	RocerEntry *entries;
	RocerMask rocers;
};

class CRocer
{
public:
	CRocer()
	{
		_refRoc=NULL;
		_bInLost=FALSE;
	}
	DEFINE_REF();
	void Clear()//注意,Clear()不会放弃控制权,会保留原来的状态,如果你要放弃控制权,请调用Discard()
	{
		SAFE_RELEASE(_refRoc);
		BreakRef();
	}
	void Discard()	{		DiscardAndUpdate(RocerType_Invalid);	}
	void DiscardAndUpdate(RocerType type)
	{
		if (!_refRoc)
			return;

		Roc *roc=(Roc *)_refRoc->GetStuff();
		if (roc)
		{
			Ref *refMe=GetRef();

			RocerEntry **e=&roc->entries;

			while(*e)
			{
				RocerEntry **p=e;
				e=&(*e)->next;

				if ((*p)->refRocer==refMe)
				{
					SAFE_RELEASE((*p)->refRocer);
					RocerMask t=(1<<((*p)->type));
					roc->rocers&=~t;

					RocerEntry *tp=(*p);
					(*p)=(*p)->next;
					Class_Delete(tp);
					break;
				}
			}

			if (RocerType_Invalid!=type)
				roc->rocers|=(1<<type);
		}
		SAFE_RELEASE(_refRoc);
	}
	BOOL Occupy(Ref *refRoc,RocerType type)
	{
		if(_bInLost)
			return FALSE;//正在lost的时候,不能抢占
		if (_refRoc)
			Discard();

		Roc *roc=(Roc *)refRoc->GetStuff();
		if (!roc)
			return FALSE;


		Ref *refMe=GetRef();

		//判断自己是否可以抢占控制权
		if (!_TestOccupy(type,roc->rocers))
			return FALSE;

		//得到需要通知哪些即将失去控制权的rocer,并去掉这些原来的抢占者
		RocerMask discards=_GetDiscards(type,roc->rocers);
		Ref*notifies[64];//big enough
		DWORD nNotifies=0;
		if (TRUE)
		{
			RocerEntry **e=&roc->entries;

			while(*e)
			{
				RocerEntry **p=e;
				e=&(*e)->next;

				CRocer *rocer=NULL;
				if ((1<<(*p)->type)&discards)
				{
					rocer=(CRocer *)(*p)->refRocer->GetStuff();

					//从roc里去掉rocer
					SAFE_RELEASE((*p)->refRocer);
					RocerEntry *tp=(*p);
					(*p)=(*p)->next;
					Class_Delete(tp);
				}
				if (rocer)
				{
					if (rocer->_refRoc)
					{
						SAFE_RELEASE(rocer->_refRoc);//从rocer里去掉roc
						notifies[nNotifies++]=rocer->ObtainRef();
					}
				}
			}
			roc->rocers&=~discards;
		}

		//抢占
		if (TRUE)
		{
			RocerEntry *entry=Class_New2(RocerEntry);
			entry->type=type;
			entry->refRocer=refMe;
			entry->refRocer->AddRef();

			entry->next=roc->entries;
			roc->entries=entry;

			roc->rocers|=(1<<type);

			_refRoc=refRoc;
			refRoc->AddRef();

		}


		//通知那些被抢占者
		_bInLost=TRUE;
		for (int i=0;i<nNotifies;i++)
		{
			CRocer *rocer=(CRocer *)notifies[i]->GetStuff();
			if (rocer)
				rocer->OnLost();
			SAFE_RELEASE(notifies[i]);
		}
		_bInLost=FALSE;

		return TRUE;
	}

	BOOL Update(RocerType type)
	{
		return Occupy(_refRoc,type);
	}

	virtual void OnEvent(RocEvent re){}
	virtual void OnLost()	{	}

protected:

	virtual BOOL _TestOccupy(DWORD type,RocerMask rocersOld)=0;
	virtual RocerMask _GetDiscards(DWORD type,RocerMask rocersOld)=0;
	Ref *_refRoc;
	BOOL _bInLost;
	

};

void Roc::SendEvent(RocEvent re)
{
	RocerEntry *e=entries;
	Ref*notifies[64];//big enough
	DWORD nNotifies=0;
	while(e)
	{
		if (e->refRocer)
		{
			e->refRocer->AddRef();
			notifies[nNotifies++]=e->refRocer;
		}
		e=e->next;
	}

	for (int i=0;i<nNotifies;i++)
	{
		CRocer *rocer=(CRocer *)notifies[i]->GetStuff();
		if (rocer)
			rocer->OnEvent(re);
		SAFE_RELEASE(notifies[i]);
	}
}
