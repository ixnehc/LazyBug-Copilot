
#include "stdh.h"

#include "GuiAgent_PaintRoadOpacity.h"

#include "GuiData_Road.h"

#include "AgentCmdID.h"

#include "MapObjUtil.h"

#include "WorldSystem/ITrrn.h"

#include "GuiData.h"

#include "spline/CubicSpline.h"

BOOL CGuiAgent_PaintRoadOpacity::OnBeginDrag(int x, int y, DWORD flag)
{
    GuiData_Road *data = (GuiData_Road *)FindData("road");
    if (!data)
        return FALSE;

    if (data->hObjSel == INVALID_HMAPOBJ)
        return FALSE;

	if (!data->bPaintingOpacity)
		return FALSE;

    IRoadEditor * editor = data->GetEditor();

    if (editor)
	{
        editor->BeginModOpacity(data->hObjSel);
	}

    return TRUE;

}

void CGuiAgent_PaintRoadOpacity::OnEndDrag(int x, int y, DWORD flag)
{
    GuiData_Road *data = (GuiData_Road *)FindData("road");
    if (!data)
        return;

    if (data->hObjSel == INVALID_HMAPOBJ)
		return;

    IRoadEditor * editor = data->GetEditor();

    if (editor)
    {
        editor->EndModOpacity(data->hObjSel);
		CommitMapObjMod(_GetModMgr(),GetView(),data->hObjSel,editor);

    }

}

void CGuiAgent_PaintRoadOpacity::OnDrag(int x, int y, DWORD flag)
{
    GuiData_Road *data = (GuiData_Road *)FindData("road");
    if (!data)
        return;
    GuiData_Trrn *dataTrrn = (GuiData_Trrn *)FindData("terrain");
    if (!dataTrrn)
        return;

    if (data->hObjSel == INVALID_HMAPOBJ)
        return;

    IRoadEditor * editor = data->GetEditor();

    if (editor)
    {
        HitProbe probe;
        if (GetRP()->CalcHitProbe(x, y, probe))
        {
            ITrrnMapEditor *editorTrrn = dataTrrn->GetTrrnMapEditor();
            if (editorTrrn)
            {
                i_math::vector3df vHit;
                if (editorTrrn->GetHitPos(probe, TRUE,vHit))
                {
                    editor->ModOpacity(data->hObjSel, vHit, data->deltaPaintOpacity, data->radiusPaintOpacityInner, data->radiusPaintOpacityOutter);

//                    CommitMapObjMod(_GetModMgr(), GetView(), data->hObjSel, editor);
                }
            }
        }
    }

}

BOOL CGuiAgent_PaintRoadOpacity::OnMouseMove(int x, int y, DWORD flag)
{
    BOOL bRet = __super::OnMouseMove(x, y, flag);

    return bRet;
}


BOOL CGuiAgent_PaintRoadOpacity::OnDraw()
{
    GuiData_Road *data = (GuiData_Road *)FindData("road");
    if (!data)
        return TRUE;

    if (!data->bPaintingOpacity)
        return TRUE;

	i_math::pos2di posCursor;
	_GetCursorPos(posCursor);

	GuiData_Trrn *dataTrrn = (GuiData_Trrn *)FindData("terrain");
	if (data&&dataTrrn)
	{
		if (data->bPaintingOpacity)
		{
			HitProbe probe;
			if (GetRP()->CalcHitProbe(posCursor.x, posCursor.y, probe))
			{
				ITrrnMapEditor *editorTrrn = dataTrrn->GetTrrnMapEditor();
				if (editorTrrn)
				{
					i_math::vector3df vHit;
					if (editorTrrn->GetHitPos(probe, TRUE,vHit))
					{
						TrrnSeedMapArg arg;
						arg.purpose = TrrnSeedMapArg::Purpose_AddHt;
						arg.vCenter = vHit;
						arg.radius = data->radiusPaintOpacityInner;
						arg.radius2 = data->radiusPaintOpacityOutter;

						editorTrrn->CalcSeedMap(_seedmap, arg);
					}
				}
			}
		}
	}


    if (_seedmap.IsEmpty())
        return TRUE;

    IRenderPort * rp = GetRP();

    std::vector<i_math::vector3df> b0, b1;
    b0.resize(_seedmap.boundary.size());
    b1.resize(_seedmap.boundary2.size());

    if (TRUE)
    {
        for (int i = 0;i < b0.size();i++)
        {
            b0[i] = _seedmap.boundary[i];
            b0[i].y += 0.2f;
        }
        for (int i = 0;i < b1.size();i++)
        {
            b1[i] = _seedmap.boundary2[i];
            b1[i].y += 0.2f;
        }
    }

    rp->Lines(b1.data(), b1.size() / 2, ColorAlpha(0xffff00, 0xff));
    rp->Lines(b0.data(), b0.size() / 2, ColorAlpha(0x00ff00, 0xff));


	return TRUE;
}

BOOL CGuiAgent_PaintRoadOpacity::OnRButtonClick(int x, int y, DWORD flag)
{

    GuiData_Road *data = (GuiData_Road *)FindData("road");
    if (!data)
        return TRUE;

	if (data->hObjSel == INVALID_HMAPOBJ)
		return TRUE;

	if (data->bPaintingOpacity)
	{
		data->bPaintingOpacity=FALSE;
		return  FALSE;
	}

    if (data->bPaintingOpacity&&data->deltaPaintOpacity > 0.0f)
        _AddMenu("Inc Opacity", ID_AGENT_INCROADOPACITY, MF_ENABLED | MF_STRING | MF_CHECKED);
    else
        _AddMenu("Inc Opacity", ID_AGENT_INCROADOPACITY, MF_ENABLED | MF_STRING );

    if (data->bPaintingOpacity&&data->deltaPaintOpacity < 0.0f)
        _AddMenu("Dec Opacity", ID_AGENT_DECROADOPACITY, MF_ENABLED | MF_STRING | MF_CHECKED);
    else
        _AddMenu("Dec Opacity", ID_AGENT_DECROADOPACITY, MF_ENABLED | MF_STRING);


    return TRUE;
}

BOOL CGuiAgent_PaintRoadOpacity::OnCommand(DWORD idCmd)
{
    GuiData_Road *data = (GuiData_Road *)FindData("road");
    if (!data)
        return TRUE;

    if (idCmd == ID_AGENT_INCROADOPACITY)
    {
        if (data->bPaintingOpacity&&data->deltaPaintOpacity > 0.0f)
            data->bPaintingOpacity = FALSE;
        else
        {
            data->bPaintingOpacity = TRUE;
            data->deltaPaintOpacity = fabsf(data->deltaPaintOpacity);
        }

        _Redraw(FALSE);
    }

    if (idCmd == ID_AGENT_DECROADOPACITY)
    {
        if (data->bPaintingOpacity&&data->deltaPaintOpacity < 0.0f)
            data->bPaintingOpacity = FALSE;
        else
        {
            data->bPaintingOpacity = TRUE;
            data->deltaPaintOpacity = -fabsf(data->deltaPaintOpacity);
        }

        _Redraw(FALSE);
    }
    return TRUE;
}


BOOL CGuiAgent_PaintRoadOpacity::OnMouseWheel(int delta,DWORD flag)
{
	GuiData_Road *data = (GuiData_Road *)FindData("road");
	if (!data)
		return TRUE;

	if (data->bPaintingOpacity&&data->hObjSel!=INVALID_HMAPOBJ)
	{
		if ((flag&CtrlOpFlag_CtrlDown)&&(!(flag&CtrlOpFlag_ShiftDown)))
		{
			data->radiusPaintOpacityOutter+=delta*0.002f;
			if (data->radiusPaintOpacityOutter<0.1f)
				data->radiusPaintOpacityOutter=0.1f;
			if (data->radiusPaintOpacityInner>data->radiusPaintOpacityOutter)
				data->radiusPaintOpacityInner=data->radiusPaintOpacityOutter;

			_Redraw(TRUE);
			return FALSE;
		}
		if ((flag&CtrlOpFlag_ShiftDown)&&(!(flag&CtrlOpFlag_CtrlDown)))
		{
			data->radiusPaintOpacityInner+=delta*0.002f;
			if (data->radiusPaintOpacityInner<0.1f)
				data->radiusPaintOpacityInner=0.1f;
			if (data->radiusPaintOpacityOutter<data->radiusPaintOpacityInner)
				data->radiusPaintOpacityOutter=data->radiusPaintOpacityInner;
			_Redraw(TRUE);
			return FALSE;
		}
	}

	return TRUE;

}
