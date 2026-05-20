
#pragma once

#include "GridItemSpin.h"

#include "anim/animbase.h"


class CRichGrid_FloatItem:public CGridItemSpin
{
public:
	CRichGrid_FloatItem(CString strCaption):CGridItemSpin(strCaption)
	{
		_f=NULL;
	}
	void Bind(float *f,float min,float max);

	virtual void OnBeginValueChange()	;
	virtual void OnValueChanged(SlideSpinValue v);
	virtual void OnEndValueChange();

protected:
	float *_f;
};


class CRichGrid_AnimTickItem:public CRichGrid_FloatItem
{
public:
	CRichGrid_AnimTickItem(CString strCaption):CRichGrid_FloatItem(strCaption)
	{
		_v=NULL;
		_fv=0.0f;
	}
	void Bind(AnimTick *v,float min,float max);

	virtual void OnValueChanged(SlideSpinValue v);

protected:
	float _fv;
	AnimTick *_v;
};

