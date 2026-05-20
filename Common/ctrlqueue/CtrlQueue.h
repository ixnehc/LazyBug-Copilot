#pragma once

#include "../anim/animbase.h"

#include <deque>


class CCtrlQueue
{
public:
	struct Ctrl
	{
		Ctrl()
		{
			tStart=tDur=tFadeIn=tFadeOut=tOff=0;
			wt[0]=wt[1]=0.0f;
			priority=100;
			bPersist=1;
			bInitial=0;
			nOccupied=0;
		}

		INTERFACE_REFCOUNT;
		virtual void OnDiscard()	{	}//当这个ctrl被从某个queue中discard时,会调用这个函数
		virtual BOOL CanDiscardBelow(AnimTick t)		{			return FALSE;		}

		float GetWeight(DWORD flip,float lerp)
		{
			float t1=wt[flip];
			if (t1<0.0f)
				t1=0.0f;
			float t2=wt[1-flip];
			if (t2<0.0f)
				t2=0.0f;
			return i_math::lerp(t2,t1,lerp);
		}

		void Stop(AnimTick t);


		AnimTick tStart;//起始时间
		AnimTick tDur;//维持时间
		AnimTick tFadeIn;
		AnimTick tFadeOut;
		AnimTick tOff;
		BYTE priority;
		BYTE bPersist:1;//如果不是persist的,这个ctrl会在weight<=0的时候被丢弃
		BYTE bInitial:1;//表示这个Ctrl提供的值是否是初始值

		//以下变量为只读,不要修改
		short nOccupied;//表示这个ctrl存在于几个Ctrl队列中,如果为0,表示这个ctrl已经不起任何作用了
		float wt[2];//weight

	};
	enum Mode
	{
		Mode1,
		Mode2,
	};

	CCtrlQueue()
	{
		_mode=Mode1;
		_flip=0;
	}
	~CCtrlQueue()
	{
		std::deque<Ctrl*>::iterator it;
		for (it=_ctrls.begin();it!=_ctrls.end();it++)
		{
			if (*it)
				(*it)->Release();
		}
		_ctrls.clear();
	}
	void SetMode(Mode mode)	{		_mode=mode;	}

	BOOL IsEmpty()	{		return _ctrls.size()<=0;	}
	void AddCtrl(Ctrl*ctrl);


	virtual void Update(AnimTick t)
	{
		_flip=1-_flip;
		switch(_mode)
		{
			case Mode1:				_Update1(t);				break;
			case Mode2:				_Update2(t);				break;
		}
	}

protected:

	void _Update1(AnimTick t);
	void _Update2(AnimTick t);


	void _UpdateDiscard();

	std::deque<Ctrl*> _ctrls;
	BYTE _mode;
	BYTE _flip;
};

