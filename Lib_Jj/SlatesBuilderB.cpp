
#include "stdh.h"

#include "SlatesBuilderB.h"

#include "Random/Random.h"
#include "commondefines/general_stl.h"


BOOL CSlatesBuilderB::CheckReveal(LevelSlateType tp,int xOff,int yOff)
{
	if(xOff==0&&yOff==0)
		return FALSE;

	switch(tp)
	{
		case LevelSlateTypeB_Cross:
		{
			if ((xOff*yOff==0)&&(abs(xOff)+abs(yOff)==1))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Cross_x2:
		{
			if ((xOff*yOff==0)&&(abs(xOff)+abs(yOff)==2))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Ver:
		{
			if((xOff==0)&&(yOff!=0))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Hor:
		{
			if((xOff!=0)&&(yOff==0))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Ring:
		{
			if ((abs(xOff)<=1)&&(abs(yOff)<=1))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Ring_x2:
		{
			if ((abs(xOff)<=2)&&(abs(yOff)<=2))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Right:
		{
			if (xOff>=0)
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Left:
		{
			if (xOff<=0)
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Down:
		{
			if (yOff<=0)
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Up:
		{
			if (yOff>=0)
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Ascend:
		{
			if (xOff==yOff)
				return TRUE;
			break;
		}
		case LevelSlateTypeB_Descend:
		{
			if (xOff==-yOff)
				return TRUE;
			break;
		}
		case LevelSlateTypeB_LeftUp:
		{
			if ((xOff<=0)&&(yOff>=0))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_LeftDown:
		{
			if ((xOff<=0)&&(yOff<=0))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_RightUp:
		{
			if ((xOff>=0)&&(yOff>=0))
				return TRUE;
			break;
		}
		case LevelSlateTypeB_RightDown:
		{
			if ((xOff>=0)&&(yOff<=0))
				return TRUE;
			break;
		}
		//XXXXX:MoreSlateTypeB
	}

	return FALSE;
}

CSlatesBuilderB::LockAffects CSlatesBuilderB::_GetLockAffects(i_math::pos2di &pt,LevelSlateType tp)
{
	LockAffects affects=0;
	for (int i=0;i<_locks.c;i++)
	{
		Lock &lock=_locks.buf[i];
		if (CheckReveal(tp,lock.pt.x-pt.x,lock.pt.y-pt.y))
			affects|=(1<<i);
	}
	return affects;
}

CSlatesBuilderB::LockAffects CSlatesBuilderB::_GetLockAffects(Slate &slate)
{
	if (slate.tp==LevelSlateType_None)
		return 0;
	return _GetLockAffects(slate.pt,slate.tp);
}


void CSlatesBuilderB::Init(int w,int h,int lenMin)
{
	_slates.resize(w*h);
	VEC_SET(_slates,0);
	for (int i=0;i<w;i++)
	for (int j=0;j<h;j++)
		_slates[j*w+i].pt.set(i,j);

	_w=w;
	_h=h;

	_nLength=0;
	_lenMin=lenMin;
}

void CSlatesBuilderB::AddLock(int x,int y)
{
	_locks.Add(x,y);

	_GetSlate(i_math::pos2di(x,y)).bLock=TRUE;
}

CSlatesBuilderB::Slate &CSlatesBuilderB::_GetSlate(i_math::pos2di &pt)
{
	return _slates[pt.y*_w+pt.x];
}

BOOL CSlatesBuilderB::_Stamp(i_math::pos2di &ptStamp)
{

	Slate&slateStamp=_GetSlate(ptStamp);

	if (slateStamp.bLock)
		return FALSE;
	if (slateStamp.nVisits>=2)
		return FALSE;


	LockAffects affectsStamp=0;

	if (slateStamp.tp!=LevelSlateType_None)
	{
		affectsStamp=_GetLockAffects(slateStamp);
		Locks locks;
		locks.CopyFrom(_locks);
		locks.ApplyStamp(ptStamp,affectsStamp);
		if (_locks.IsSameWith(locks))
		{
			slateStamp.nVisits++;
			return TRUE;
		}
		return FALSE;
	}

	//尝试不同的types
	LevelSlateType tps[]={
		LevelSlateTypeB_Cross,
		LevelSlateTypeB_Cross_x2,
		LevelSlateTypeB_Ver,
		LevelSlateTypeB_Hor,
//		LevelSlateTypeB_Full,
		LevelSlateTypeB_Ring,
		LevelSlateTypeB_Ring_x2,
// 		LevelSlateTypeB_Right,
// 		LevelSlateTypeB_Left,
// 		LevelSlateTypeB_Up,
// 		LevelSlateTypeB_Down,
		LevelSlateTypeB_Ascend,
		LevelSlateTypeB_Descend,
// 		LevelSlateTypeB_LeftUp,
// 		LevelSlateTypeB_LeftDown,
// 		LevelSlateTypeB_RightDown,
	};

	BOOL b1stStamp=TRUE;
	if (!_locks1stStamp.empty())
		b1stStamp=FALSE;

	_entriesType.clear();
	_entriesType.reserve(64);

	for (int i=0;i<ARRAYSIZE(tps);i++)
	{
		Locks locks;
		locks.CopyFrom(_locks);
		LevelSlateType tp=tps[i];
		affectsStamp=_GetLockAffects(ptStamp,tp);
		locks.ApplyStamp(ptStamp,affectsStamp);

		if (affectsStamp==((1<<_locks.c)-1))
			continue;//单次stamp打开了所有lock

		if (b1stStamp)
		{
			if (!locks.IsAnyUnlock())
				continue;//第一次stamp,一个lock都无法打开
		}
		else
		{
			BOOL bLost=TRUE;
			for (int i=0;i<_locks1stStamp.size();i++)
			{
				if (locks.buf[_locks1stStamp[i]].nStamps>0)
				{
					bLost=FALSE;
					break;
				}
			}
			if (bLost)
				continue;//把第一次stamp打开的lock全部关闭
		}

// 		if ((tp==LevelSlateTypeB_Descend)||(tp==LevelSlateTypeB_Ascend))
// 		{
// 			if ((ptStamp.x>=3)&&(ptStamp.x<=4)&&(ptStamp.y>=3)&&(ptStamp.y<=4))
// 				continue;
// 		}

		if (_nLength<_lenMin)
		{
			if (locks.IsAllUnlock())
				continue;//尚未达到最小步长就全打开了
		}


		WeightedSlateType tpWeight;
		tpWeight.tp=tp;
		tpWeight.wt=1.0f;
		if (TRUE)
		{

			DWORD nExpectedUnlocked=1;

			float progress=((float)_nLength)/(float)_lenMin;

			progress=i_math::clamp_f(progress,0.0f,1.0f);

			float progressUnlock=((float)locks.GetUnlockCount())/(float)_locks.GetCount();
			progressUnlock=i_math::clamp_f(progressUnlock,0.0f,1.0f);

			float diff=fabsf(progressUnlock-progress);
			if (diff<0.1f)
				diff=0.1f;

			tpWeight.wt/=diff;
		}
		_entriesType.push_back(tpWeight);
	}

	if (_entriesType.size()<=0)
		return FALSE;

	CSysRandom::Srand();
	WeightedSlateType *entry=CSysRandom::RollWeighted(_entriesType);
	if (!entry)
		return FALSE;

	if (TRUE)
	{
		affectsStamp=_GetLockAffects(ptStamp,entry->tp);
		_locks.ApplyStamp(ptStamp,affectsStamp);
		if (b1stStamp)
		{
			//记录下所有第一次stamp被打开的lock
			for (int i=0;i<_locks.c;i++)
			{
				if (_locks.buf[i].nStamps>0)
					_locks1stStamp.push_back(i);
			}
		}
	}

	slateStamp.tp=entry->tp;
	slateStamp.nVisits=1;

	_nLength++;

	return TRUE;
}

void CSlatesBuilderB::_Reset()
{
	for (int i=0;i<_slates.size();i++)
	{
		_slates[i].tp=LevelSlateType_None;
		_slates[i].nVisits=0;
	}

	_locks1stStamp.clear();
	_locks.ResetLock();
	
}

BOOL CSlatesBuilderB::_FindPath()
{
	CSysRandom::Srand();

	i_math::pos2di ptCur;
	while(1)
	{
		i_math::pos2di pt;
		pt.x=CSysRandom::RandRangeInt(0,_w);
		pt.y=CSysRandom::RandRangeInt(0,_h);

		if (_Stamp(pt))
		{
			ptCur=pt;
			break;
		}
	}

	std::vector<int>indices;
	static i_math::pos2di offsets[]=
	{
		i_math::pos2di(-1,0),i_math::pos2di(1,0),i_math::pos2di(0,1),i_math::pos2di(0,-1)
	};

	while(1)
	{
		CSysRandom::Srand();
		CSysRandom::GenRandomIndices(indices,4);
		i_math::pos2di ptNext;

		BOOL bStamped=FALSE;
		for (int i=0;i<indices.size();i++)
		{
			i_math::pos2di off=offsets[indices[i]];
			ptNext=ptCur+off;
			if ((ptNext.x<0)||(ptNext.y<0)||(ptNext.x>=_w)||(ptNext.y>=_h))
				continue;

			if (!_Stamp(ptNext))
				continue;

			bStamped=TRUE;
			break;
		}

		if (!bStamped)
			return FALSE;

		if (_locks.IsAllUnlock())
			break;

		ptCur=ptNext;
	}

	for (int i=0;i<_slates.size();i++)
	{
		if ((_slates[i].tp!=LevelSlateType_None)&&(!_slates[i].bLock))
			_slates[i].bPath=TRUE;
	}

	BOOL bCanEnter=FALSE;
	for (int i=0;i<_slates.size();i++)
	{
		if (_slates[i].bPath)
		{
			i_math::pos2di &pt=_slates[i].pt;
			if (pt.y<=0)
			{
				bCanEnter=TRUE;
				break;
			}
			if (pt.y<=1)
			{
				if (!_slates[(pt.y-1)*_w+pt.x].bLock)
				{
					bCanEnter=TRUE;
					break;
				}
			}
		}
	}

	if (!bCanEnter)
		return FALSE;


	return TRUE;
}

DWORD LockAffects_GetCount(CSlatesBuilderB::LockAffects affects)
{
	DWORD c=0;
	for (int i=0;i<16;i++)
	{
		if (affects&(1<<i))
			c++;
	}
	return c;

}

void CSlatesBuilderB::_FillBg()
{
	_temp.reserve(128);

	//Runes
	if (TRUE)
	{
		std::vector<LevelSlateType> buf;
		LevelSlateType tp=LevelSlateTypeB_Rune01;
		for (int i=0;i<_slates.size();i++)
		{
			if (_slates[i].bLock)
			{
				buf.push_back(tp);
				tp=(LevelSlateType)(((int)tp)+1);
			}
		}

		std::vector<int> indices;
		CSysRandom::GenRandomIndices(indices,buf.size());

		int idx=0;
		for (int i=0;i<_slates.size();i++)
		{
			if (_slates[i].bLock)
			{
				_slates[i].tp=buf[indices[idx]];
				idx++;
			}
		}
	}

	for (int i=0;i<_slates.size();i++)
	{
		if (_slates[i].tp!=LevelSlateType_None)
			_temp.push_back(_slates[i].pt);
	}

	for (int k=0;k<_slates.size();k++)
	{
		Slate &slate=_slates[k];
		if (slate.tp==LevelSlateType_None)
		{
			int distMin=100000;
			for (int j=0;j<_temp.size();j++)
			{
				int dist;
				dist=abs(slate.pt.x-_temp[j].x)+abs(slate.pt.y-_temp[j].y);
				if (dist<distMin)
					distMin=dist;
			}

			if (distMin<=0)
				continue;

			float unlocksMax=0.6f*(float)distMin;

			LevelSlateType tps[]={
				LevelSlateTypeB_Cross,
				LevelSlateTypeB_Cross_x2,
				LevelSlateTypeB_Ver,
				LevelSlateTypeB_Hor,
				LevelSlateTypeB_Ring,
				LevelSlateTypeB_Ring_x2,
				LevelSlateTypeB_Ascend,
				LevelSlateTypeB_Descend,
			};

			_entriesType.clear();
			for (int i=0;i<ARRAYSIZE(tps);i++)
			{
				LevelSlateType tp=tps[i];
				LockAffects affects=_GetLockAffects(slate.pt,tp);
				DWORD c=LockAffects_GetCount(affects);

				float wt=1.0f;
				if (((float)c)>unlocksMax)
					wt-=((float)c)-unlocksMax;
				if (wt<=0.0f)
					continue;

				WeightedSlateType tpWeight;
				tpWeight.tp=tp;
				tpWeight.wt=1.0f;

				_entriesType.push_back(tpWeight);
			}

			if (_entriesType.size()<=0)
			{
				WeightedSlateType tpWeight;
				tpWeight.tp=LevelSlateTypeB_Cross_x2;
				tpWeight.wt=1.0f;

				_entriesType.push_back(tpWeight);
			}

			WeightedSlateType *entry=CSysRandom::RollWeighted(_entriesType);

			slate.tp=entry->tp;
		}
	}


}


void CSlatesBuilderB::Build()
{
	while(1)
	{
		if (_FindPath())
			break;
		_Reset();
	}

	CSysRandom::Srand();

	_FillBg();

// 	for (int i=0;i<_slates.size();i++)
// 	{
// 		if (_slates[i].tp==LevelSlateType_None)
// 			_slates[i].tp=LevelSlateTypeB_Full;
// 	}

}

LevelSlateType CSlatesBuilderB::GetResultType(int x,int y)
{
	return _slates[y*_w+x].tp;
}

void CSlatesBuilderB::Dump(SlatesBData &data)
{
	data.w=_w;
	data.h=_h;

	data.buf.resize(_slates.size());
	for (int i=0;i<_slates.size();i++)
	{
		data.buf[i].tp=(BYTE)_slates[i].tp;
		data.buf[i].bLock=(BYTE)_slates[i].bLock;
		data.buf[i].bPath=(BYTE)_slates[i].bPath;
	}
}
