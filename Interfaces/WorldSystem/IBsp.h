
#pragma once
class IBspModel
{
public:
	INTERFACE_REFCOUNT;
	virtual void WriteToPKG()=0;
	virtual void ReadFromPKG()=0;
	//virtual void GetVolumeMatchingZoneID (CBspVisibilitySet *pBVS)=0;//传入视锥参数,得到(被portal 剪裁过的视锥+zone ID)组.
};
class IBspEditor
{
public:
	INTERFACE_REFCOUNT;
	virtual void BulidBspFromIMesh(IMeshSnapshot *pMS,s32	PolyFlags)=0;
	virtual void GetBspModel(IBspModel *pBM)=0;
	virtual void BuildPortaFromBsp()=0;
};

