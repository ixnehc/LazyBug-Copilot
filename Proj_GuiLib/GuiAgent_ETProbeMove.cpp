
#include "stdh.h"

#include "GuiAgent_ETProbeMove.h"

#include "GuiData_ETProbe.h"

CGuiAgent_ETProbeMove::CGuiAgent_ETProbeMove(void)
:CGuiAgent_3DNodeMatEdit(EditMode_Move)
{

}
void * CGuiAgent_ETProbeMove::_GetSelBuf()
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(!data)
		return NULL;
	return &(data->hObjSels);
}

DWORD * CGuiAgent_ETProbeMove::_GetVer()
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(!data)
		return NULL;
	return &(data->ver);
}

i_math::pos2di * CGuiAgent_ETProbeMove::_GetBlock(H3DNode node)
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
i_math::matrix43f * CGuiAgent_ETProbeMove::_GetMat(H3DNode node)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	if(editor){
		const ETProbeInfo * info = editor->GetETProbeInfo(HMapObj(node));
		if(info){
			_mat.setTranslation(info->pos);
			return &_mat;
		}
	}
	
	return NULL;
}
void CGuiAgent_ETProbeMove::_Move(H3DNode &node,i_math::matrix43f &mat)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();

	if(editor){
		const ETProbeInfo * info = editor->GetETProbeInfo(HMapObj(node));
		if(info){
			ETProbeInfo infoCopy = *info;
			i_math::vector3df pos = mat.getTranslation();
			infoCopy.pos = pos;
			HMapObj hObj = editor->SetETProbe(HMapObj(node),infoCopy);
			node = H3DNode(hObj);
		}
	}
}












