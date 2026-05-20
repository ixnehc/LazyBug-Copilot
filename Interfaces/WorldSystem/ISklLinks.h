/********************************************************************
	created:	8:4:2010   17:28
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	interface for Skleton Links
*********************************************************************/
#pragma once

#include "IMano.h"

class IAnimNode;
class IDummies;
class ISklLinks
{
public:
	INTERFACE_REFCOUNT;

	virtual IDummies * GetDummies()=0;
	virtual void SetDummies(IDummies *dummies)=0;
	virtual IAnimNode *Obtain(const char *dummy,BOOL bResetScale=FALSE)=0;//加一个引用计数
	virtual IAnimNode *ObtainMcPart(const char *part)=0;//加一个引用计数,如果没有,新增一个
	virtual IMano *GetBaseMano()=0;//不加引用计数
	virtual IMano *ObtainBaseMano()=0;//加一个引用计数,如果没有,创建一个
	virtual BOOL AddMtrlCtrl(const char *part,ManoChannel ch,MtrlCtrl *ctrl)=0;
	virtual void AddEvent(AnimTick t,StringID nmEvent)=0;
	virtual BOOL FetchEvent(StringID nmEvent)=0;
	virtual BOOL FindEvent(StringID nmEvent,AnimTick &t)=0;

	virtual void Update(AnimTick t)=0;
	virtual void RegisterSub(const char *nm,IAnimNode *an)=0;
	virtual IAnimNode *GetSub(const char *nm)=0;//不加引用计数
};
