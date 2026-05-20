#pragma once

#include "gds/GObj.h"
#include "class/class.h"
#include "anim/KeySet.h"


// 折线
struct ValueSet:public KeySet
{	
public:

	DEFINE_CLASS( ValueSet);

	ValueSet()
	{
		GConstructor();
		_idxLoop=-1;
		_bVisible=0;
	}

	void Zero(BOOL bIntuitive)
	{
		_idxLoop=-1;
		_bVisible=0;
	}
	BEGIN_GOBJ( ValueSet, 1 );
	END_GOBJ();

	void Clear()
	{
		KeySet::Clean();
		Zero(FALSE);
	}

	void Copy(ValueSet *src)
	{
		KeySet::CopyFrom(*(KeySet *)src);
		_idxLoop=src->_idxLoop;
		_bVisible=src->_bVisible;
	}

	void Save(CDataPacket &dp)
	{
		DP_WriteVar(dp,_hd);
		DP_WriteVectorN(dp,_buf);
		DP_WriteVar(dp,_idxLoop);
		DP_WriteVar(dp,_bVisible);
	}


	BOOL Load(CDataPacket &dp)
	{
		DP_ReadVar(dp,_hd);
		DP_ReadVectorN(dp,_buf);
		DP_ReadVar(dp,_idxLoop);
		DP_ReadVar(dp,_bVisible);
		if (_hd.szKey==12)
		{//旧版本的keyset,修补一下
			_hd.szKey=8;

			for (int i=0;i<_hd.keycount;i++)
			{
				DWORD *p,*q;
				p=(DWORD*)&_buf[i*8];
				q=(DWORD*)&_buf[i*12];

				p[0]=q[0];
				p[1]=q[2];
			}

		}
		return TRUE;
	}

	void SaveDelta(CDataPacket &dp,ValueSet*pRef)
	{
		Save(dp);
	}

	void LoadDelta(CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		Load(dp);
		if (ptrsDelta)
			ptrsDelta->push_back(this);
	}

	
	float	GetFloat( AnimTick t);
	float GetMaxFloat();
	DWORD GetColor(AnimTick t);
	float	GetStart();// 获取第一个控制点的值 x = 0.0f
	AnimTick GetDur();//获取ValueSet的值会发生变化的时间范围,如果有循环,返回ANIMTICK_INFINITE
	void	Reset();// 重置
	
	bool	SetLoopIndex( int nIndex );// 设置循环区域开始控制点
	int GetLoopIndex();//获得循环区域开始控制点

	void ResetFloat(float v);
	BOOL AddFloat(float t,float v);

	void ResetColor(DWORD col);
	BOOL AddColor(float t,DWORD col);

public://take it as protected

	short _idxLoop;// 循环的索引 默认为-1 表示不包含循环区域
	WORD _bVisible;//这个标志用于编辑
	void _AdjustT(AnimTick &t)
	{
		if (_idxLoop>=0)
		{
			if (_idxLoop<GetKeyCount()-1)
			{
				AnimTick tStart=GetKey(_idxLoop)->t;
				AnimTick range=GetKey(GetKeyCount()-1)->t-tStart;
				if (range>0)
				{
					if (t>tStart)
						t=tStart+(t-tStart)%range;
				}
			}
		}
	}

};

