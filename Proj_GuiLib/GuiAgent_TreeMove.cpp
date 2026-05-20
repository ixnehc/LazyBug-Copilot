
#include "stdh.h"

#include "GuiAgent_TreeMove.h"

#include "GuiData_forest.h"

#include "WorldSystem/ISpt.h"

void * CGuiAgent_TreeMove::_GetSelBuf()
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(data)
		return &(data->hTreeSels);
	return NULL;
}
DWORD *CGuiAgent_TreeMove::_GetVer()
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(data)
		return &(data->ver);
	return NULL;
}

i_math::matrix43f * CGuiAgent_TreeMove::_GetMat(H3DNode node)
{
	IForestEditor *editor = (IForestEditor *)_GetEditor();
	
	if(editor){
		const TreeInfo * info = editor->GetTreeInfo(HMapObj(node));
		if(info)
			_matWork = info->GetTransform();
	}

	return &_matWork;
}

IObjMapEditor *CGuiAgent_TreeMove::_GetEditor()
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	IForestEditor *editor = NULL;
	if(data)
		editor = data->GetEditor();
	return editor;
}

void CGuiAgent_TreeMove::_Move(H3DNode &node,i_math::matrix43f &mat)
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	IForestEditor *editor = NULL;
	if(data)
		editor = data->GetEditor();
	
	if(editor){
		TreeInfo feature;

		i_math::vector3df test;
		test = mat.getTranslation();	// get translation
		feature.pos = test;

		if(TRUE) //get scale.
		{
			test.set(0.0f,1.0f,0.0f);
			mat.transformVect(test,test); 
			i_math::vector3df v0;
			v0.set(0.0f,0.0f,0.0f);
			mat.transformVect(v0,v0);
			feature.scale  = float(test.getDistanceFrom(v0));
		}

		// get rotate Y
		i_math::vector3df x0,x,z0,z;
		z0.set(0.0f,0.0f,1.0f);
		x0.set(1.0f,0.0f,0.0f);

		i_math::matrix43f matRot = mat;
		matRot.m03 = matRot.m13 = matRot.m23 = 0.0f;

		matRot.transformVect(x0,x);
		matRot.transformVect(z0,z);

		i_math::vector3df projX = x;

		x.y = 0;
		x.normalize();
		float dot = x.dotProduct(x0);

		float angel = acos(dot);

		if(x.dotProduct(z0)>0)
			feature.rotY = - angel;
		else
			feature.rotY = angel;

		projX.normalize();
		BOOL bVerse = projX.dotProduct(z0)<0; 

		const float pi = 3.14159265358979323846f;

		if((matRot.m11<0.999999f&&(abs(matRot.m12)<0.0000001f)))
		{	
			if(bVerse&&matRot.m02<0)
				feature.rotY -= pi;

			if(!bVerse&&matRot.m02>0)
				feature.rotY += pi;
		}

		HMapObj hObjNew = editor->SetTreeInfo(HMapObj(node),feature);
		node = H3DNode(hObjNew);
	}
}





