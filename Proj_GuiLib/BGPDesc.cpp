/********************************************************************
	created:	2012/12/2 
	author:		cxi
	
	purpose:	一些CBehaviorGraphPad的Desc
*********************************************************************/
#include "stdh.h"
 
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/Behavior.h"
#include "../Lib_Jj/BgnAttack.h"
#include "../Lib_Jj/BgnTeleport.h"
#include "behaviorgraph/BgnState.h"
//#include "behaviorgraph/BgnController.h"
#include "behaviorgraph/BgnRelay.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IRecords.h"
#include "records/records.h"

#include "graphicsgraph.h"

#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IAssetSystem.h"

#include "GuiLib.h"

#define DEFINE_BGP(clss,pad)															\
if (!pad0->GetClass()->IsSameWith(Class_Ptr2(clss)))					\
	return FALSE;																					\
clss*pad=(clss*)pad0;


class CBehaviorGraphPad;

////////////////////////////////////////////////////////////////////////
//Fill



////////////////////////////////////////////////////////////////////////
//Draw

BOOL DrawDesc_CBgp_ActivateStates(GraphicsGraph *gg,CBehaviorGraphPad*pad0,std::string &desc,i_math::recti &rcDesc)
{
	DEFINE_BGP(CBgp_ActivateStates,pad);

	i_math::recti rc=rcDesc;

	BOOL bDrawn=FALSE;
	for (int i=0;i<pad->_nms.size();i++)
	{
		StringID id=pad->_nms[i];
		if (id==StringID_Invalid)
			continue;

		i_math::size2di sz=gg->MessureText(StrLib_GetStr(id),1000);
		sz.w+=4;
		rc.Right()=rc.Left()+sz.w;
		rc.Bottom()=rc.Top()+sz.h;

		gg->DrawRoundCornerRect(rc,6,RGB(255,0,0),RGB(128,0,0));
		gg->FrameRoundCornerRect(rc,6,RGB(0,0,0),RGB(0,0,0));
		gg->DrawText(StrLib_GetStr(id),rc,DT_LEFT,FALSE,0xffffffff);

		rc.Top()+=sz.h;

		bDrawn=TRUE;
	}

	return bDrawn;
}


BOOL DrawDesc_CBgp_SwitchState(GraphicsGraph *gg,CBehaviorGraphPad*pad0,std::string &desc,i_math::recti &rcDesc)
{
	DEFINE_BGP(CBgp_SwitchState,pad);

	i_math::recti rc=rcDesc;

	BOOL bDrawn=FALSE;
	if(TRUE)
	{
		StringID id=pad->_nm;
		if (id!=StringID_Invalid)
		{

			i_math::size2di sz=gg->MessureText(StrLib_GetStr(id),1000);
			sz.w+=4;
			rc.Right()=rc.Left()+sz.w;
			rc.Bottom()=rc.Top()+sz.h;

			gg->DrawRoundCornerRect(rc,6,RGB(255,0,0),RGB(128,0,0));
			gg->FrameRoundCornerRect(rc,6,RGB(0,0,0),RGB(0,0,0));
			gg->DrawText(StrLib_GetStr(id),rc,DT_LEFT,FALSE,0xffffffff);

			rc.Top()+=sz.h;

			bDrawn=TRUE;
		}
	}
	return bDrawn;
}


BOOL DrawDesc_CBgp_StartRelay(GraphicsGraph *gg,CBehaviorGraphPad*pad0,std::string &desc,i_math::recti &rcDesc)
{
	DEFINE_BGP(CBgp_StartRelay,pad);

	i_math::recti rc=rcDesc;

	BOOL bDrawn=FALSE;
	if (TRUE)
	{
		StringID id=pad->_nm;
		if (id==StringID_Invalid)
			return FALSE;

		i_math::size2di sz=gg->MessureText(StrLib_GetStr(id),1000);
		sz.w+=4;
		rc.Right()=rc.Left()+sz.w;
		rc.Bottom()=rc.Top()+sz.h;

		gg->DrawRoundCornerRect(rc,6,RGB(64,128,64),RGB(32,64,32));
		gg->FrameRoundCornerRect(rc,6,RGB(0,0,0),RGB(0,0,0));
		gg->DrawText(StrLib_GetStr(id),rc,DT_LEFT,FALSE,0xffffffff);

		rc.Top()+=sz.h;

		bDrawn=TRUE;
	}
	return bDrawn;
}
