/********************************************************************
	created:	2019/12/06   13:39
	author:		cxi
*********************************************************************/
#include "stdh.h"
#include "ranges.h"

#include "../commondefines/general_stl.h"

#include "Random/Random.h"


void CRanges::_Collapse(int iStart)
{
	int sz=iStart+1;
	for (int i=iStart+1;i<_rngs.size();i++)
	{
		if (0==_rngs[iStart].compare(_rngs[i]))
		{
			_rngs[iStart].merge(_rngs[i]);
			continue;
		}
		_rngs[sz]=_rngs[i];
		iStart=sz;
		sz++;
	}
	_rngs.resize(sz);
}


void CRanges::AddRange(i_math::rangef &rng)
{
	for (int i=0;i<_rngs.size();i++)
	{
		int v=rng.compare(_rngs[i]);
		if (v<0)
		{
			_rngs.insert(_rngs.begin()+i,rng);
			return;
		}
		if (v>0)
			continue;
		if (v==0)
		{
			if (_rngs[i].merge(rng))
				_Collapse(i);
			return;
		}
	}
	_rngs.push_back(rng);
}

float CRanges::Rand()
{
	float sum=0.0f;
	for (int i=0;i<_rngs.size();i++)
		sum+=_rngs[i].length();

	float v=CSysRandom::RandRange(0.0f,sum);

	for (int i=0;i<_rngs.size();i++)
	{
		if (v<=_rngs[i].length())
			return _rngs[i].low+v;
		v-=_rngs[i].length();
	}

	if (_rngs.size()>0)
		return _rngs[_rngs.size()-1].hi;
	return 0.0f;
}

BOOL CRanges::FindClosestInRange(float v,float &vClosest)
{
	BOOL bRet=FALSE;
	float distMin=1000000.0f;
	for (int i=0;i<_rngs.size();i++)
	{
		if (_rngs[i].isIn(v))
		{
			vClosest=v;
			return TRUE;
		}
		float dist=fabsf(_rngs[i].low-v);
		if (dist<distMin)
		{
			bRet=TRUE;
			distMin=dist;
			vClosest=_rngs[i].low;
		}

		dist=fabsf(_rngs[i].hi-v);
		if (dist<distMin)
		{
			bRet=TRUE;
			distMin=dist;
			vClosest=_rngs[i].hi;
		}
	}
	return bRet;
}



//////////////////////////////////////////////////////////////////////////
//CCircumRanges
void CCircumRanges::AddRange(i_math::rangef &rng)
{
	float lo=i_math::wrap_radian(rng.low);
	float hi=rng.hi+lo-rng.low;

	if (hi>=lo+i_math::Pi*2.0f)
	{
		_rngs.clear();
		_rngs.push_back(i_math::rangef(0.0f,i_math::Pi*2.0f));
		return;
	}

	if (hi>i_math::Pi*2.0f)
	{
		CRanges::AddRange(i_math::rangef(lo,i_math::Pi*2.0f));
		CRanges::AddRange(i_math::rangef(0.0f,hi-i_math::Pi*2.0f));
		return;
	}
	CRanges::AddRange(i_math::rangef(lo,hi));

}

void CCircumRanges::Invert()
{
	std::deque<i_math::rangef> rngsNew;

	float prev=0.0f;
	for (int i=0;i<_rngs.size();i++)
	{
		if (_rngs[i].low>prev)
			rngsNew.push_back(i_math::rangef(prev,_rngs[i].low));
		prev=_rngs[i].hi;
	}
	if (prev<i_math::Pi*2.0f)
		rngsNew.push_back(i_math::rangef(prev,i_math::Pi*2.0f));

	_rngs.swap(rngsNew);
}

float CCircumRanges::GetCoverRate()
{
	float v=0.0f;
	for (int i=0;i<_rngs.size();i++)
		v+=_rngs[i].hi-_rngs[i].low;

	return i_math::clamp_f(v/(i_math::Pi*2.0f),0.0f,1.0f);
}


BOOL CCircumRanges::FindClosestInRange(float v,float &vClosest)
{
	v=i_math::wrap_radian(v);

	BOOL bRet=FALSE;
	float distMin=1000000.0f;
	for (int i=0;i<_rngs.size();i++)
	{
		if (_rngs[i].isIn(v))
		{
			vClosest=v;
			return TRUE;
		}
		float dist=get_radian_dist(_rngs[i].low,v);
		if (dist<distMin)
		{
			bRet=TRUE;
			distMin=dist;
			vClosest=_rngs[i].low;
		}

		dist=get_radian_dist(_rngs[i].hi,v);
		if (dist<distMin)
		{
			bRet=TRUE;
			distMin=dist;
			vClosest=_rngs[i].hi;
		}
	}
	return bRet;
}
