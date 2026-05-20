/********************************************************************
	created:	2010/03/15
	filename: 	d:\IxEngine\Interfaces\WorldSystem\stubparams\param_mano.cpp
	author:		chenxi
	
	purpose:	·â×°IManoµÄparam
*********************************************************************/

#include "stdh.h"

#include "../IMano.h"

#include "param_mano.h"


//////////////////////////////////////////////////////////////////////////
//Prop_Mano

void Prop_Mano::DeleteThis()
{
	SAFE_RELEASE(mn);
	Class_Delete(this);
};

GProperty *Prop_Mano::Clone()
{
	Prop_Mano *p=Class_New2(Prop_Mano);
	p->mn=mn;
	SAFE_ADDREF(p->mn);
	return p;
}
