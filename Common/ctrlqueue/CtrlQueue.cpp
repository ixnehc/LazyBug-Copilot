/********************************************************************
created:	2010/09/19
filename: 	d:\IxEngine\Common\ctrlqueue\CtrlQueue.cpp
author:		chenxi

purpose:	CtrlQueue
*********************************************************************/
#include "stdh.h"
#include "CtrlQueue.h"

//////////////////////////////////////////////////////////////////////////
//CCtrlQueue::Ctrl
void CCtrlQueue::Ctrl::Stop(AnimTick t)
{
	if (tOff!=0)
		return;
	if (t<tFadeIn)
	{//如果正在fadein,我们要转化为正在fade out
		tDur=0;
		if (tFadeIn>0)
		{
			float r=((float)t)/((float)tFadeIn);
			r=1.0f-r;
			tOff=(AnimTick)(((float)tFadeOut)*r)+tFadeIn+tDur-t;
		}
	}
	else
	{
		if ((t<tFadeIn+tDur)||(tDur==ANIMTICK_INFINITE))
			tDur=t-tFadeIn;
	}

}


//////////////////////////////////////////////////////////////////////////
//CCtrlQueue



void CCtrlQueue::AddCtrl(CCtrlQueue::Ctrl*ctrl)
{
	if (ctrl)
	{
		ctrl->AddRef();
		ctrl->nOccupied++;

		std::deque<Ctrl*>::iterator it=_ctrls.begin();
		while(it!=_ctrls.end())
		{
			if ((*it)->priority>ctrl->priority)
				it++;
			else
				break;
		}
		_ctrls.insert(it,ctrl);
	}
}


void CCtrlQueue::_UpdateDiscard()
{
	std::deque<Ctrl*>::iterator it,it2;
	Ctrl *ctrl;
	it2=_ctrls.begin();
	for (it=_ctrls.begin();it!=_ctrls.end();it++)
	{
		ctrl=(*it);
		if (ctrl->wt[1-_flip]>-100.0f)
		{
			(*it2)=ctrl;
			it2++;
			continue;
		}

		//上一帧要被丢弃
		ctrl->nOccupied--;
		ctrl->OnDiscard();//通知被discard了
		SAFE_RELEASE(ctrl);
	}
	_ctrls.erase(it2,_ctrls.end());
}


//更新方式:下层thread的weight** 受 **上层thread所影响,当thread完全结束后,把这个thread丢弃掉
void CCtrlQueue::_Update1(AnimTick t0)
{
	float wt=1.0f;
	AnimTick tTotal,t;
	std::deque<Ctrl*>::iterator it;
	Ctrl *ctrl;
	BOOL bInstantFadeIn=FALSE;
	for (it=_ctrls.begin();it!=_ctrls.end();it++)
	{
		ctrl=(*it);

		if (wt>0.0f)
		{
			t=t0+ctrl->tOff;
			tTotal=ctrl->tStart;
			if (t<tTotal)
			{
				ctrl->wt[_flip]=0.0f;
				continue;//这个thread尚未开始
			}
			tTotal+=ctrl->tFadeIn;
			if (t<tTotal)
			{//这个thread进入了fade in阶段
				ctrl->wt[_flip]=wt*((float)(t-ctrl->tStart))/(float)ctrl->tFadeIn;
				wt-=ctrl->wt[_flip];
			}
			else
			{
				if (ctrl->tDur==ANIMTICK_INFINITE)
					tTotal=ANIMTICK_INFINITE;
				else
					tTotal+=ctrl->tDur;
				if (t<tTotal)
				{//这个thread处于完全表现出来的阶段
					ctrl->wt[_flip]=wt;
					if (ctrl->tFadeIn==0)
					{
						ctrl->wt[1-_flip]=wt;
						bInstantFadeIn=TRUE;
					}
					wt=0.0f;
				}
				else
				{
					if (tTotal!=ANIMTICK_INFINITE)
						tTotal+=ctrl->tFadeOut;
					if (t<tTotal)
					{//这个thread处于fade out的阶段
						ctrl->wt[_flip]=wt*((float)(tTotal-t))/(float)ctrl->tFadeOut;
						wt-=ctrl->wt[_flip];
					}
					else
						ctrl->wt[_flip]=-1000.0f;
				}
			}
		}
		else
		{
			ctrl->wt[_flip]=0.0f;
			if (bInstantFadeIn)
			{
				ctrl->wt[1-_flip]=0.0f;
				if (!ctrl->bPersist)
					ctrl->wt[1-_flip]=-1000.0f;
			}
			if (!ctrl->bPersist)
				ctrl->wt[_flip]=-1000.0f;
		}
	}
	
	_UpdateDiscard();
}

//更新方式:下层thread的weight** 不受 **上层thread所影响,当thread完全结束后,把这个thread丢弃掉
void CCtrlQueue::_Update2(AnimTick t0)
{
	AnimTick tTotal,t;
	std::deque<Ctrl*>::iterator it,it2;
	Ctrl *ctrl;
	BOOL bForceDiscard=FALSE;
	for (it=_ctrls.begin();it!=_ctrls.end();it++)
	{
		ctrl=(*it);
		if (bForceDiscard)
		{
			ctrl->wt[_flip]=-1000.0f;
			ctrl->wt[1-_flip]=-1000.0f;
			continue;
		}
		t=t0+ctrl->tOff;
		tTotal=ctrl->tStart;
		if (t<tTotal)
		{
			ctrl->wt[_flip]=0.0f;
			continue;//这个thread尚未开始
		}
		tTotal+=ctrl->tFadeIn;
		if (t<tTotal)
		{//这个thread进入了fade in阶段
			ctrl->wt[_flip]=((float)(t-ctrl->tStart))/(float)ctrl->tFadeIn;
			continue;
		}
		if (ctrl->tDur==ANIMTICK_INFINITE)
			tTotal=ANIMTICK_INFINITE;
		else
			tTotal+=ctrl->tDur;
		if (t<tTotal)
		{//这个thread处于完全表现出来的阶段
			ctrl->wt[_flip]=1.0f;
			if (ctrl->tFadeIn==0)
				ctrl->wt[1-_flip]=1.0f;
			if (ctrl->CanDiscardBelow(t0))
				bForceDiscard=TRUE;
			continue;
		}
		if (tTotal!=ANIMTICK_INFINITE)
			tTotal+=ctrl->tFadeOut;
		if (t<tTotal)
		{//这个thread处于fade out的阶段
			ctrl->wt[_flip]=((float)(tTotal-t))/(float)ctrl->tFadeOut;
			continue;
		}
		ctrl->wt[_flip]=-1000.0f;//标记为需要扔掉
	}

	_UpdateDiscard();
}
