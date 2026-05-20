#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"
#include "LevelSlateDefinesB.h"



class CSlatesBuilderB
{
public:
	typedef DWORD LockAffects;

	CSlatesBuilderB()
	{
		_w=_h=0;
		_nLength=0;
	}
	struct Lock
	{
		Lock()
		{
			nStamps=0;
		}
		void CopyFrom(Lock &src)
		{
			pt=src.pt;
			nStamps=src.nStamps;
			memcpy(stamps,src.stamps,nStamps*sizeof(stamps[0]));
		}
		void AddStamp(i_math::pos2di &pt)
		{
			if (nStamps>=SLATESB_MAX_STAMP)
				return;
			stamps[nStamps]=pt;
			nStamps++;
		}
		void RemoveStamp()
		{
			if (nStamps>0)
				nStamps--;
		}
		BOOL ExistsStamp(i_math::pos2di &pt)
		{
			for (int i=0;i<nStamps;i++)
			{
				if (pt==stamps[i])
					return TRUE;
			}
			return FALSE;
		}
		BOOL IsSameWith(Lock &other)
		{
			if (pt==other.pt)
			{
				if (nStamps==other.nStamps)
				{
					for (int i=0;i<nStamps;i++)
					{
						if (stamps[i]!=other.stamps[i])
							return FALSE;
					}
					return TRUE;
				}
			}
			return FALSE;
		}
		BOOL IsUnlock()
		{
			return nStamps>0;
		}
		i_math::pos2di pt;
		DWORD nStamps;
		i_math::pos2di stamps[SLATESB_MAX_STAMP];
	};
	struct Locks
	{
		Locks()
		{
			c=0;
		}

		DWORD GetCount()		{			return c;		}

		void ResetLock()
		{
			for (int i=0;i<c;i++)
				buf[i].nStamps=0;
		}

		void Add(int x,int y)
		{
			buf[c].pt.x=(short)x;
			buf[c].pt.y=(short)y;
			c++;
		}

		void CopyFrom(Locks &src)
		{
			c=src.c;
			for (int i=0;i<c;i++)
				buf[i].CopyFrom(src.buf[i]);
		}
		void ApplyStamp(i_math::pos2di &pt,LockAffects affects)
		{
			for (int i=0;i<c;i++)
			{
				if (affects&(1<<i))
				{
					if (!buf[i].ExistsStamp(pt))
						buf[i].AddStamp(pt);
				}
				else
					buf[i].RemoveStamp();
			}
		}
		BOOL IsSameWith(Locks &other)
		{
			if (c!=other.c)
				return FALSE;
			for (int i=0;i<c;i++)
			{
				if (!buf[i].IsSameWith(other.buf[i]))
					return FALSE;
			}
			return TRUE;
		}

		DWORD GetUnlockCount()
		{
			DWORD nUnlocks=0;
			for (int i=0;i<c;i++)
			{
				if (buf[i].IsUnlock())
					nUnlocks++;
			}
			return nUnlocks;
		}

		BOOL IsAnyUnlock()
		{
			for (int i=0;i<c;i++)
			{
				if (buf[i].IsUnlock())
					return TRUE;
			}
			return FALSE;
		}

		BOOL IsAllUnlock()		{			return GetUnlockCount()>=c;		}


		DWORD c;
		Lock buf[8];//Big enough
	};

	struct Slate
	{
		i_math::pos2di pt;
		BOOL bLock;
		BOOL bPath;

		LevelSlateType tp;
		int nVisits;
	};

	void Init(int w,int h,int lenMin);
	void AddLock(int x,int y);

	void Build();
	void Dump(SlatesBData &data);

	LevelSlateType GetResultType(int x,int y);

	static BOOL CheckReveal(LevelSlateType tp,int xOff,int yOff);

public:

	void _Reset();
	BOOL _FindPath();

	void _FillBg();


	struct WeightedSlateType
	{
		LevelSlateType tp;
		float wt;
	};

	Slate &_GetSlate(i_math::pos2di &pt);

	BOOL _Stamp(i_math::pos2di &ptStamp);

	LockAffects _GetLockAffects(i_math::pos2di &pt,LevelSlateType tp);
	LockAffects _GetLockAffects(Slate &slate);

	int _w,_h;
	std::vector<Slate> _slates;
	Locks _locks;
	std::vector<int> _locks1stStamp;

	DWORD _nLength;
	DWORD _lenMin;

	std::vector<WeightedSlateType> _entriesType;
	std::vector<i_math::pos2di> _temp;

};