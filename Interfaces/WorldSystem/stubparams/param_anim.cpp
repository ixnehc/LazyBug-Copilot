/********************************************************************
	created:	31:10:2008   9:30
	filename: 	d:\IxEngine\Interfaces\WorldSystem\stubparams\param_anim.cpp
	author:		chenxi
	
	purpose:	params related to animation 
*********************************************************************/
#include "stdh.h"

#include "../IAnimNodes.h"
#include "../IAssetRenderer.h"
#include "../IMano.h"
   
#include "RenderSystem/IAnim.h"

#include "avtrstates/avtrstates.h"

#include "param_anim.h"


//////////////////////////////////////////////////////////////////////////
//Prop_AnimNode

void Prop_AnimNode::DeleteThis()
{
	SAFE_RELEASE(an);
	Class_Delete(this);
};

GProperty *Prop_AnimNode::Clone()
{
	Prop_AnimNode *p=Class_New2(Prop_AnimNode);
	p->an=an;
	SAFE_ADDREF(p->an);
	return p;
}


//////////////////////////////////////////////////////////////////////////
//Prop_AvtrStates
GProperty *Prop_AvatarStates::Clone()
{
	Prop_AvatarStates*p=Class_New2(Prop_AvatarStates);
	p->v=v;
	SAFE_ADDREF(p->v);
	return p;
}

void Prop_AvatarStates::DeleteThis()
{
	SAFE_RELEASE(v);
	Class_Delete(this);
}
