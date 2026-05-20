
#pragma once
#include "IResource.h"

#include "anim/animbase.h"
#include "strlib/strlibdefines.h"

#include "bitset/bitset.h"

struct KeySet;
struct Key;
struct AnimPiece;
struct AnimData;
class IAnim:public IResource
{
public:
	virtual KeyType GetKeyType()=0;

	virtual DWORD GetAnimPieceCount()=0;
	virtual StringID GetAnimPieceName(DWORD idx)=0;//if fail,return ""
	virtual int FindAnimPiece(StringID name)=0;//if not found,return -1

	virtual KeySet *GetKeySet()=0;
	virtual BOOL CalcKey(Key *key,DWORD idxAP,AnimTick t)=0;

	//得到某个AnimPiece的key,nKeys用来指定key指针指向多大的buffer,并且用来返回有多少key被返回了
	virtual BOOL GetKeys(Key *key,DWORD &nKeys,DWORD idxAP)=0;

	virtual DWORD GetKeyCount(DWORD idxAP)=0;
	virtual Key *GetKey(DWORD idxAP,DWORD iKey)=0;

	//Note:return the inclusive range(that is to say, the range is [tStart,tEnd]),
	virtual AnimTick GetAPDuration(DWORD idxAP)=0;
	virtual BOOL GetAPRange(DWORD idxAP,AnimTick &tStart,AnimTick &tEnd)=0;
	virtual AnimEvent *GetEvents(DWORD idxAP,DWORD &count)=0;
	virtual BOOL GetAPParam(DWORD idxAP,DWORD iParam,float &v)=0;
};

class IAnimMgr:public IResourceMgr
{

};
struct AnimData;
class IDynAnimMgr:public IResourceMgr
{
public:
	virtual IAnim *Create(AnimData *data)=0;
};

struct KeySet;
struct AnimPiece;
class ISkeleton;
class IBoneAnim :public IResource
{
public:
	//access bone skeleton
	virtual ISkeleton*GetSkeleton()=0;
	virtual BOOL CalcKey(AnimTick t,i_math::xformf *xfms,DWORD nXfm,DWORD idxAP,Bitset<8>*mask=NULL) = 0;//xfms: provide memory. nXfm: number of xfms array. 
	//access animpiece
	virtual DWORD GetAnimPieceCount()=0;
	virtual StringID GetAnimPieceName(DWORD idx)=0;//if fail,return ""
	virtual int FindAnimPiece(StringID name)=0;//if not found,return -1

	virtual BOOL GetAPRange(DWORD idxAP,AnimTick &tStart,AnimTick &tEnd)=0;
	virtual AnimEvent *GetEvents(DWORD idxAP,DWORD &count)=0;
	virtual BOOL GetAPParam(DWORD idxAP,DWORD iParam,float &v) = 0;

	virtual BOOL GetAnimRange(AnimTick &tStart,AnimTick &tEnd) = 0;;
};

class IBoneAnimMgr:public IResourceMgr
{
};
struct ResData;
class IDynBoneAnimMgr:public IResourceMgr
{
public:
	virtual IBoneAnim *Create(ResData *data)=0;
};




//Anim Thread Flag
#define ATF_Reverse 1 //the anim is playing from end to begin

#define AnimThreadChannel_Top 0xff//目前最上面的那个channel
#define AnimThreadChannel_AboveTop 0xfe//目前最上面的那个channel之上
#define AnimThreadChannel_TopMost 0xfd//最最上面

typedef DWORD SpeedRate;
#define SPEEDRATE_UNIT 1000 
#define SPEEDRATE_MIN 1
#define SPEEDRATE_MAX 1000000

struct AnimThreadParam
{
	void Reset()//reset to default
	{
		tOff=0;
		tFadeIn=0;
		tFadeOut=0;
		tLocalStart=0;
		tLocalEnd=ANIMTICK_INFINITE;
		nLoop=1;
		rSpeed=SPEEDRATE_UNIT;
		flag=0;
		label=StringID_Invalid;
		id=0;
		ch=AnimThreadChannel_Top;
	}
	AnimThreadParam()	{		Reset();	}
	void SetOff(AnimTick off)	{		tOff=off;	}
	void SetFadeIn(AnimTick t)	{		tFadeIn=t;	}
	void SetFadeOut(AnimTick t)	{		tFadeOut=t;	}
	void SetLocalStart(AnimTick t)	{		tLocalStart=t;	}
	void SetLocalEnd(AnimTick t)	{		tLocalEnd=t;	}
	void SetLoop(DWORD n=0xffffffff) 	{		nLoop=n;	}
	void SetSpeedRate(DWORD count)	//count of 1/1000,clamped by [SPEEDRATE_MIN,SPEEDRATE_MAX]
	{		
		if (count<SPEEDRATE_MIN) 
			count=SPEEDRATE_MIN;
		if (count>SPEEDRATE_MAX) 
			count=SPEEDRATE_MAX;
		rSpeed=count;	
	}
	void SetReverse()	{		flag|=ATF_Reverse;	}
	void SetChannel(int ch_)	{		ch=ch_;	}
	void SetLabel(StringID labelThread)	{		label=labelThread;	}
	void SetID(DWORD idThread)	{		id=idThread;	}

	AnimTick tOff;
	AnimTick tFadeIn;//not affected by rSpeed
	AnimTick tFadeOut;//only valid when any thread exists on the below channel,not affected by rSpeed
	AnimTick tLocalStart;//指定一个时刻,从这个anim piece的这个时刻开始播放动画
	AnimTick tLocalEnd;//指定一个时刻,这个anim piece播放的动画到这个时刻结束
	DWORD nLoop;
	short ch;
	WORD flag;//ATF_XXXX
	SpeedRate rSpeed;//count of 1/1000 
	StringID label;
	DWORD id;
	//IMPORTANT: need to add a event argument,indicate that the thread will be added
	//when the specified event occurs
};

struct AnimThreadInfo
{
	StringID labelThread;
	DWORD idThread;

	DWORD nLoop;
	short ch;
	WORD flag;

};

struct Key;


class IAnimPlayer
{
public:
	INTERFACE_REFCOUNT;

	virtual void Reset(IAnim *anim,DWORD idxAP,AnimTick tStart,BOOL bLoop)=0;
	virtual void Reset(AnimTick tStart)=0;

	virtual IAnim *GetAnim()=0;
	virtual int GetAPIdx()=0;

	virtual void SetSpeedRate(float rate)=0;

	virtual BOOL Tick(AnimTick t)=0;
	virtual Key* Calc(AnimTick t)=0;

	virtual float GetRatio(AnimTick t)=0;

	//if event is ignored,no met event will be add to queue during IncTick()/DecTick()
	virtual void SetIgnoreEvent(BOOL bIgnore=TRUE)=0;
	virtual BOOL FetchEvent(AnimEvent &e)=0;
};

