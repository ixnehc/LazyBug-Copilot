
#include "stdh.h"
#include "valueset.h"

float ValueSet::GetFloat(AnimTick t)
{
	if ( IsEmpty() )
		return 0.0f;

	_AdjustT(t);

	Key_f ret;
	CalcKey<Key_f>(t,&ret);
	return ret.v;
}

float ValueSet::GetMaxFloat()
{
	if ( IsEmpty() )
		return 0.0f;

	float max=-100000.0f;
	int c=GetKeyCount();
	for (int i=0;i<c;i++)
	{
		float v=((Key_f*)GetKey(i))->v;
		if (v>max)
			max=v;
	}
	return max;
}


DWORD ValueSet::GetColor(AnimTick t)
{
	if ( IsEmpty() )
		return 0xffffffff;
	_AdjustT(t);

	Key_col ret;
	CalcKey<Key_col>(t,&ret);
	return ret.color;
}


float ValueSet::GetStart()
{
	if (IsEmpty())
		return 0.0f;
	return ((Key_f*)GetKey(0))->v;
}

AnimTick ValueSet::GetDur()
{
	int c=GetKeyCount();
	if (c==0)
		return 0;
	if (_idxLoop>=0)
	{
		if (_idxLoop<c)
			return ANIMTICK_INFINITE;
	}

	return ((Key *)GetKey(c-1))->t;//最后一个Key的时间
}

void ValueSet::Reset()
{
	Clean();
	_idxLoop=-1;
}


bool ValueSet::SetLoopIndex( int nIndex )
{
	if ( nIndex >= (((int)GetKeyCount())-1) )
		return false;
	_idxLoop = nIndex;
	return true;
}

int ValueSet::GetLoopIndex()
{
	return _idxLoop;
}

void ValueSet::ResetFloat(float v)
{
	_hd.type=KT_Float;
	_hd.szKey=sizeof(Key_f);
	_hd.keycount=0;
	_buf.clear();
	AddFloat(0,v);
}

void ValueSet::ResetColor(DWORD col)
{
	_hd.type=KT_Color;
	_hd.szKey=sizeof(Key_f);
	_hd.keycount=0;
	_buf.clear();
	AddColor(0,col);
}


BOOL ValueSet::AddFloat(float t,float v)
{
	if (_hd.type!=KT_Float)
		return FALSE;
	Key_f k;
	k.t=ANIMTICK_FROM_SECOND(t);
	k.v=v;
	InsertKey(GetKeyCount(),k);
	return TRUE;
}

BOOL ValueSet::AddColor(float t,DWORD col)
{
	if (_hd.type!=KT_Color)
		return FALSE;
	Key_col k;
	k.t=ANIMTICK_FROM_SECOND(t);
	k.color=col;
	InsertKey(GetKeyCount(),k);
	return TRUE;
}
