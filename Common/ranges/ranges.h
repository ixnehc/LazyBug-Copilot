
#pragma once

#include "../math/range.h"

#include <deque>


class CRanges
{
public:
	void AddRange(float lo,float hi)	{		return AddRange(i_math::rangef(lo,hi));	}
	virtual void AddRange(i_math::rangef &rng);
	float Rand();

	virtual BOOL FindClosestInRange(float v,float &vClosest);

protected:
	void _Collapse(int iStart);
	std::deque<i_math::rangef> _rngs;
};

class CCircumRanges:public CRanges
{
public:
	void AddRange(i_math::rangef &rng) override;

	void Invert();
	float GetCoverRate();

	virtual BOOL FindClosestInRange(float v,float &vClosest);



};