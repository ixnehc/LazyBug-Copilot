#pragma once

#include "class/class.h"

typedef DWORD UnitGestureUID;
#define UnitGestureUID_Invalid 0

class CUnit3D;
class CUnit;
class CUnitGesture
{
public:
	INTERFACE_REFCOUNT;
	virtual CClass *GetClass()=0;

	virtual void Destroy()=0;
	virtual void Update(CUnit3D *unit,float dt)=0;
	virtual void Update(CUnit *unit,float dt)=0;
	virtual BOOL IsFinished()=0;
	virtual UnitGestureUID GetUID()=0;

protected:
};


struct UnitGestureInfo
{
	UnitGestureInfo()
	{
		Zero();
	}

	void Zero()
	{
		gesture=NULL;
		tGestureAge=0.0f;
	}

	void Clear()
	{
		SAFE_DESTROY(gesture);	
		tGestureAge=0.0f;
	}

	void SetGesture(CUnitGesture *gesture_)
	{
		SAFE_DESTROY(gesture);
		SAFE_REPLACE(gesture,gesture_);
		tGestureAge=0.0f;
	}
	CUnitGesture *GetGesture()	{		return gesture;	}
	CUnitGesture *FetchGesture()
	{
		tGestureAge=0.0f;
		CUnitGesture *ret=gesture;
		gesture=NULL;
		return ret;
	}
	UnitGestureUID GetGestureUID()	{		return gesture?gesture->GetUID():0;	}
	float GetGestureAge()	{		return tGestureAge;	}

	BOOL IsValid()	{		return gesture!=NULL;	}

	void UpdateState(CUnit3D *unit,float dt)
	{
		if (gesture)
		{
			tGestureAge+=dt;
			if (!gesture->IsFinished())
				gesture->Update(unit,dt);
			if (gesture->IsFinished())
			{
				SAFE_DESTROY(gesture);
				tGestureAge=0.0f;
			}
		}
	}

	void UpdateState(CUnit*unit,float dt)
	{
		if (gesture)
		{
			tGestureAge+=dt;
			if (!gesture->IsFinished())
				gesture->Update(unit,dt);
			if (gesture->IsFinished())
			{
				SAFE_DESTROY(gesture);
				tGestureAge=0.0f;
			}
		}
	}


	CUnitGesture *gesture;
	float tGestureAge;

};