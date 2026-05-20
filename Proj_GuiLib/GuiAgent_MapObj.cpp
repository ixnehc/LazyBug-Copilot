

#include "stdh.h"

#include "GuiAgent_MapObj.h"

#include "MapObjUtil.h"

i_math::pos2di * CGuiAgent_MapObjOP::_GetBlock(H3DNode node)
{
	IObjMapEditor * editor = _GetEditor();
	if(editor&&editor->GetMapFileBlk(HMapObj(node),_ptBlk))
		return &_ptBlk;
	else
		return NULL;
}

H3DNode CGuiAgent_MapObjOP::_HitTest(i_math::line3df &ray)
{
	HMapObj hObj = INVALID_HMAPOBJ;
	IObjMapEditor * editor = _GetEditor();
	
	if(editor)
		hObj = editor->HitTest(ray);
	
	return H3DNode(hObj);
}

BOOL CGuiAgent_MapObjOP::_Remove(H3DNode node)
{
	HMapObj hObj = HMapObj(node);

	IObjMapEditor * editor = _GetEditor();
	if(editor)
		return editor->Delete(hObj);

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
i_math::pos2di * CGuiAgent_MapObjTransform::_GetBlock(H3DNode node)
{
	IObjMapEditor * editor = _GetEditor();
	if(editor&&editor->GetMapFileBlk(HMapObj(node),_ptBlk))
		return &_ptBlk;
	else
		return NULL;
}




