
#pragma once

#include "datapacket/DataPacket.h"

class CtrlPoint
{
public:
	virtual ~CtrlPoint(void){}

	virtual void Clone(const CtrlPoint * p){	pos = p->pos;}

	virtual void Lerp(CtrlPoint * p0,CtrlPoint * p1,float r){ pos = (1-r)*p0->pos + r*p1->pos;}
	
	virtual BOOL Equals(CtrlPoint * p){	return (pos==p->pos);}

	virtual void Load(CDataPacket &dp) {dp.Data_ReadData(&pos,sizeof(pos));}
	
	virtual void Save(CDataPacket &dp) {dp.Data_WriteData(&pos,sizeof(pos));}

public:

	i_math::vector3df pos; //基本的元素位置
};


class ICtrlPointPack
{
public:
	virtual BOOL Remove(DWORD idx) = 0;	//
	virtual BOOL Remove(DWORD *idx,DWORD n) = 0; //删除多个
	virtual BOOL Insert(DWORD idx,const CtrlPoint * cp) = 0;
	virtual void Push(const CtrlPoint *cp) = 0;
	virtual CtrlPoint * At(DWORD idx) = 0;
	virtual BOOL Set(DWORD idx,const CtrlPoint * p) = 0;
	virtual DWORD GetNumberOfCP() = 0;
	virtual void Clean() = 0;
	virtual void Clone(ICtrlPointPack * pp,int s = -1,int e = -1) = 0;
	
	virtual BOOL IsEmpty() = 0;
	virtual CtrlPoint * Back() = 0;
	
	virtual CtrlPoint * NewCP() = 0;
	virtual void DeleteCP(CtrlPoint *p) = 0;
	
	virtual void DeleteMe() = 0;

	virtual i_math::vector3df GetCenter() = 0;
	virtual i_math::aabbox3df GetAABB() = 0;

	virtual BOOL Equals(ICtrlPointPack *pp) = 0;


	//extra-function , flag and so on
	virtual BOOL IsClosed() = 0;
	virtual void SetClosed(BOOL bClosed) = 0;

	virtual BOOL IsDoubleSide() = 0;
	virtual void SetDoubleSide(BOOL bDoubleSide) = 0;

	virtual DWORD GetSequence(DWORD *&seq) = 0;	//考虑到控制点序列 考虑到Flag标志的影响
};






