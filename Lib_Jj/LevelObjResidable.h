#pragma once

#include "class/class.h"


#include "LevelDefines.h"

struct LosAgent;


class CLevelObjResidable
{
public:
	virtual CClass *GetClass()=0;

	virtual void Clear()=0;

	virtual BOOL CanPreserve()=0;
	virtual BOOL CanOccupy()=0;
	virtual LevelObjSeatToken Preserve()=0;
	virtual void Cancel(LevelObjSeatToken token)=0;
	virtual LevelObjSeatToken Occupy(LevelObjID id)=0;

	virtual void Discard(LevelObjSeatToken token)=0;

	virtual LevelPos3D GetSeatPos(LevelObjSeatToken token)=0;

};

//只有一个seat的Residable
class CLevelObjResidable_Single:public CLevelObjResidable
{
public:
	DEFINE_CLASS(CLevelObjResidable_Single);
	CLevelObjResidable_Single()
	{
		Zero();
	}
	void Zero()
	{
		_bEnable=TRUE;
		_bPreserve=0;
		_bOccupy=0;

	}

	void Init(i_math::matrix43f &mat,LevelPos3D &posSeat);//posSeat为local的位点
	virtual void Clear()	{		Zero();	}

	BOOL CanPreserve()
	{
		if (!_bEnable)
			return FALSE;
		if (_bPreserve)
			return FALSE;
		if (_bOccupy)
			return FALSE;
		return TRUE;
	}
	BOOL CanOccupy()
	{
		if (!_bEnable)
			return FALSE;
		if (_bOccupy)
			return FALSE;
		return TRUE;
	}

	virtual LevelObjSeatToken Preserve()
	{
		if (!CanPreserve())
			return LevelObjSeatToken_Invalid;
		_bPreserve=TRUE;
		return LevelObjSeatToken_Common;
	}
	virtual void Cancel(LevelObjSeatToken token)
	{
		if (token==LevelObjSeatToken_Common)
			_bPreserve=FALSE;
	}

	virtual LevelObjSeatToken Occupy(LevelObjID id)
	{
		if (!_bEnable)
			return LevelObjSeatToken_Invalid;
		if (_bOccupy)
			return LevelObjSeatToken_Invalid;
		_bOccupy=TRUE;
		_idObj=id;
		return LevelObjSeatToken_Common;
	}

	virtual void Discard(LevelObjSeatToken token)
	{
		if (token==LevelObjSeatToken_Common)
		{
			_bOccupy=FALSE;
			_idObj=LevelObjID_Invalid;
		}
	}

	virtual LevelPos3D GetSeatPos(LevelObjSeatToken token)
	{
		return _posSeat;
	}

	void Enable(BOOL bEnable)
	{
		_bEnable=bEnable;
	}

	LevelObjID GetOccupingObj()	{		return _idObj;	}


protected:
	BOOL _bEnable;
	DWORD _bPreserve;
	DWORD _bOccupy;
	LevelPos3D _posSeat;

	LevelObjID _idObj;

};

//有无数个seat的Residable
class CLevelObjResidable_Infinite:public CLevelObjResidable
{
public:
	DEFINE_CLASS(CLevelObjResidable_Infinite);
	CLevelObjResidable_Infinite()
	{
		Zero();
	}
	void Zero()
	{
		_count=0;
	}

	void Init(i_math::matrix43f &mat,LevelPos3D &posSeat);//posSeat为local的位点
	virtual void Clear()	{		Zero();	}

	virtual BOOL CanPreserve()	{		return TRUE;	}
	virtual BOOL CanOccupy()	{		return TRUE;	}

	virtual LevelObjSeatToken Preserve()	{		return LevelObjSeatToken_Common;	}
	virtual void Cancel(LevelObjSeatToken token)	{	}

	virtual LevelObjSeatToken Occupy(LevelObjID id)	{		_count++;return LevelObjSeatToken_Common;	}

	virtual void Discard(LevelObjSeatToken token)	{_count--;	}
	virtual LevelPos3D GetSeatPos(LevelObjSeatToken token)	{		return _posSeat;	}

protected:
	LevelPos3D _posSeat;
	int _count;

};