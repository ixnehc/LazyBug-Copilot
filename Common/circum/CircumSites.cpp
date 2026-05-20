/********************************************************************
	created:	2013/2/6 
	author:		cxi
	
	purpose:	周围位点随机生成器
*********************************************************************/
#include "stdh.h"

#include "../commondefines/general_stl.h"

#include "CircumSites.h"

#include "../Random/Random.h"


void CCircumSites::Clear()
{
	_sites.clear();
	_rings.clear();
	Zero();
}

void CCircumSites::Build(DWORD nRing,float radiusStart,float gap)
{
	float radStart;
	if (radiusStart<gap)
		radiusStart=gap;
	float radius=radiusStart;
	for (int i=0;i<nRing;i++)
	{
		radStart=CSysRandom::RandRange<float>(0.0f,i_math::Pi*2.0f);

		float step=gap/radius;
		int nSteps=(int)((i_math::Pi*2.0f)/step);
		step=i_math::Pi*2.0f/(float)nSteps;

		_rings.push_back(_sites.size());

		for (int i=0;i<nSteps;i++)
		{
			float rad=radStart+step*(float)i;

			CSysRandom::RandVary(rad,step*0.1f);

			_sites.push_back(i_math::vector2df(cosf(rad)*radius,sinf(rad)*radius));
		}

		radius+=gap;
	}
}

void CCircumSites::AddRing(i_math::vector2df *pos,DWORD nPos)
{
	if (nPos<=0)
		return;
	_rings.push_back(_sites.size());
	VEC_APPEND_BUFFER(_sites,pos,nPos);
}

void CCircumSites::AddRing(i_math::matrix43f *mats,DWORD nMats)
{
	if (nMats<=0)
		return;

	_rings.push_back(_sites.size());
	for (int i=0;i<nMats;i++)
	{
		i_math::vector2df pos;
		pos.set(mats[i].getTranslationP()->x,mats[i].getTranslationP()->z);
		_sites.push_back(pos);
	}
}

int CCircumSites::RingFromSite(DWORD iSite)
{
	for (int i=1;i<_rings.size();i++)
	{
		if (_rings[i]>iSite)
			return i-1;
	}
	return _rings.size()-1;
}

i_math::vector2df *CCircumSites::GetRingSites(DWORD iRing,DWORD &c)
{
	c=0;
	if (iRing>=_rings.size())
		return NULL;

	c=_GetRingSitesCount(iRing);
	return &_sites[_rings[iRing]];
}


WORD CCircumSites::_GetRingSitesCount(DWORD iRing)
{
	if (iRing>=_rings.size())
		return 0;
	int c=0;
	if (iRing<_rings.size()-1)
		c=(_rings[iRing+1]-_rings[iRing]);
	else
		c=(_sites.size()-_rings[iRing]);
	if (c<0)
		c=0;
	return (WORD)c;
}


void CCircumSites::BeginGen(float rateChoose)
{
	_rateChoose=rateChoose;
	_iRing=0;
	extern int GenPrimeStep();
	_step=GenPrimeStep();
	if (_rings.size()>0)
		_iPos=_step%_GetRingSitesCount(_iRing);
	else
		_iPos=0;
	_nGen=0;
}

i_math::vector2df *CCircumSites::Gen()
{
	if (_iRing>=_rings.size())
		return NULL;

	i_math::vector2df *ret=&_sites[_rings[_iRing]+_iPos];
	_nGen++;
	BOOL bNextRing=FALSE;
	if (_nGen>=_GetRingSitesCount(_iRing))
		bNextRing=TRUE;
	else
	{
		if ((((float)_nGen)/(float)_GetRingSitesCount(_iRing))>_rateChoose)
			bNextRing=TRUE;
	}
	if (bNextRing)
	{//下一个Ring
		_iPos=0;
		_nGen=0;
		_iRing++;
		if (_iRing<_rings.size())
			_iPos=_step%_GetRingSitesCount(_iRing);
	}
	else
		_iPos=(_iPos+_step)%_GetRingSitesCount(_iRing);

	return ret;
}
