
#pragma once

#include "../common/math/xform.h"

struct KinectDesc
{
	//Skeleton information
	int nBones;
	BYTE BoneParents[256];
	const char *BoneNames[256];
};

class IKinectDevice
{
public:
	enum MonitorChannel
	{
		Monitor_None=0,
		Monitor_Skeleton,
	};

	virtual BOOL Acquire()=0;
	virtual void Abandon()=0;

	KinectDesc *GetDesc()=0;

	void StartMonitor(MonitorChannel ch)=0;
	void EndMonitor(MonitorChannel ch)=0;
	BOOL IsResultReady(MonitorChannel ch)=0;
	i_math::xformf *FetchSkeleton()=0;

};

