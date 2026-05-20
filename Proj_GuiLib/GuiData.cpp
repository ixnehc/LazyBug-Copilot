
#include "stdh.h"

#include "GuiData.h"

#include "RenderSystem/IRenderSystem.h"

#include "RenderSystem/ITools.h"
#include "RenderSystem/IRenderPort.h"


#include "WorldSystem/ITrrn.h"
#include "WorldSystem/IEntitySystem.h"

#include "Log/LogDump.h"

//////////////////////////////////////////////////////////////////////////
//GuiData_Camera
void GuiData_Camera::Clear()
{
	for (int i=0;i<ARRAY_SIZE(cams);i++)
		SAFE_RELEASE(cams[i]);
	Zero();
}


//////////////////////////////////////////////////////////////////////////
//GuiData_Trrn
ITrrnMap * GuiData_Trrn::GetTrrnMap()	
{		
	return pES->FindTrrn();	
}

ITrrnMapEditor * GuiData_Trrn::GetTrrnMapEditor()
{
	ITrrnMap *t=pES->FindTrrn();
	if (t)
		return t->GetEditor();
	return NULL;
}

ITrrnBrushLib * GuiData_Trrn::GetBrushLib()
{
	if(pES)
		return pES->FindTrrnBrushLib();
	return NULL;
}

BOOL GuiData_Trrn::GetHitPos(int x,int y,IRenderPort *rp,i_math::vector3df &pos)
{
	if (!rp)
		return FALSE;

	HitProbe probe;
	if (rp->CalcHitProbe(x,y,probe))
	{
		if (GetTrrnMap())
		{
			ITrrnMapEditor *editor=GetTrrnMapEditor();
			return editor->GetHitPos(probe,TRUE,pos);
		}
	}
	return FALSE;
}



