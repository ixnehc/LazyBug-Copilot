/********************************************************************
	created:	2010/4/19   13:31
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	Anim Tree Ctrl interfaces
*********************************************************************/

#pragma once

#include "anim/animdefines.h"
#include "linkpad/LinkPad.h"

#include "ctrlqueue/CtrlQueue.h"
#include "bitset/bitset.h"

class CClass;

class IAnim;
class IBoneAnim;
class IAnimTree;
class IMatrice43;
class ISkeleton;
struct AnimEvent;
class CAvtrStates;
class IAnimNode;
class CTuner;

struct BoneCtrls;
struct IKCtrls;

class IAnimTreeCtrl
{
public:
	INTERFACE_REFCOUNT;
	virtual ISkeleton *GetSkeleton()=0;
	virtual BOOL IsValid()=0;
	virtual void Reset(IBoneAnim **anims,DWORD nAnim,IAnimTree *at,AnimTick t)=0;

	virtual void SetTimeRate(float rate)=0;

	virtual void Stabilize(AnimTick dt)=0;
	virtual void StabilizeSwitch()=0;//保证状态切换完成

	virtual BOOL Tick(AnimTick t)=0;

	virtual BOOL CalcMats(AnimTick t,IMatrice43 *mats,const char *root="")=0;
	virtual BOOL CalcXfms(AnimTick t,i_math::xformf *xfms,DWORD count,const char *root="")=0;
	virtual BOOL CalcAndApplyBoneCtrls(AnimTick t,BoneCtrls &bonectrls,const char *root="")=0;
	virtual BOOL CalcAndApplyIKCtrls(AnimTick t,IKCtrls &ikctrls,const char *root="")=0;
	virtual BOOL CalcFloat(AnimTick t,float &v,const char *root="")=0;
	virtual BOOL CalcXfm(AnimTick t,i_math::xformf&v,const char *root="")=0;

	virtual void SetIgnoreCalcErr(BOOL bIgnore=TRUE)=0;//设定当调用Calc(..)失败时,要不要Dump出Error来

	virtual AnimEvent **GetEvents(DWORD &count)=0;
	virtual AnimEvent **FetchEvents(DWORD &count)=0;
	virtual DWORD GetEventCount()=0;
	virtual AnimEvent *GetEvent(DWORD idx)=0;
	virtual void RemoveEvent(DWORD idx)=0;

	virtual BOOL BindDbg(PadID id,IAnimNode *an)=0;
	virtual BOOL Bind(CAvtrStates *cs)=0;
	virtual BOOL BindIKEffector(StringID id,IAnimNode *an)=0;

	virtual BOOL BindTuner(StringID id,CTuner*tuner)=0;
	virtual BOOL BindTunerFloat(StringID id,IAnimNode *an)=0;
	virtual BOOL SetTuneFloat(StringID id,float v)=0;
	virtual BOOL SetTuneFloat(const char *name,float v)=0;
	virtual BOOL SetTuneStringID(StringID id,StringID idStr)=0;

	virtual float GetWeight(PadID id)=0;
	virtual float GetLinkWeight(PadID idTarget,const char *stbTarget)=0;
	virtual const char *GetDesc(PadID id)=0;//得到描述一个pad当前状态的字符串,主要用于编辑
	virtual BOOL IsAwake(PadID id)=0;

};
