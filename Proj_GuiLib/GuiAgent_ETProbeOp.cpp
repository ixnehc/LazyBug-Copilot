
#include "stdh.h"

#include "GuiAgent_ETProbeOp.h"

#include "GuiData_ETProbe.h"

void * CGuiAgent_ETProbeOp::_GetSelBuf()
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(!data)
		return NULL;
	return &(data->hObjSels);
}

DWORD * CGuiAgent_ETProbeOp::_GetVer()
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(!data)
		return NULL;
	return &(data->ver);
}

i_math::pos2di * CGuiAgent_ETProbeOp::_GetBlock(H3DNode node)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	if(!editor)
		return NULL;

	if(editor->GetMapFileBlk(HMapObj(node),_ptBlk))
		return &_ptBlk;

	return NULL;
}

H3DNode CGuiAgent_ETProbeOp::_HitTest(i_math::line3df &ray)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	if(!editor)
		return H3DNode_Invalid;
	
	float mind = 9999999.0f;
	i_math::vector3df posProj;
	int sel = -1;

	for(int i = 0;i<data->nodes.size();i++){
		HMapObj & hObj = data->nodes[i];
		const ETProbeInfo * info = editor->GetETProbeInfo(hObj);
		if(info){
			ray.getProjectionPoint(info->pos,posProj);
			if(info->pos.getDistanceFromSQ(posProj)<0.25f){
				float d = (float)(info->pos - ray.start).getLengthSQ();
				if(d<mind){
					sel = i;
					mind = d;
				}
			}
		}
	}

	if(sel>=0)
		return H3DNode(data->nodes[sel]);

	return H3DNode_Invalid;
}

BOOL CGuiAgent_ETProbeOp::_Remove(H3DNode node)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	if(editor)
		editor->Delete(HMapObj(node));
	
	return TRUE;
}







