#include "stdh.h"

#include "resource.h"

#include "GuiAgent_2DTransform.h"

#include "graphicsgraph.h"

#include "GuiData.h"

#pragma  warning(disable:4244)


CGuiAgent_2DTransform::CGuiAgent_2DTransform()
{
	_fMaxZoomIn = 50.0f;
	_fMaxZoomOut = 50.0f;
}

CGuiAgent_2DTransform::~CGuiAgent_2DTransform(void)
{

}

void CGuiAgent_2DTransform::_CalcTransform(i_math::matrix43f & mat)
{
	i_math::pos2df offGG,scaleGG;
	_GetTransformGG(offGG,scaleGG);

	mat.setScale(scaleGG.x,scaleGG.y,1.0f);
	mat.addTranslation(offGG.x,offGG.y,0.0f);

}


BOOL CGuiAgent_2DTransform::OnBeginDrag(int x,int y,DWORD flag)
{
	OnSetCursor(x,y,flag);
	_x = x;
	_y = y;

	_CalcTransform(_mat);
	
	return TRUE;
}

void CGuiAgent_2DTransform::OnEndDrag(int x,int y,DWORD flag)
{	
	OnSetCursor(x,y,flag);

	i_math::matrix43f mat;
	mat.setTranslation((x-_x),(y-_y),0.0f);
	mat = _mat*mat;

	_mat.makeIdentity();
	_x = 0;
	_y = 0;

	_UpdateTransform(mat);
}

void CGuiAgent_2DTransform::OnDrag(int x,int y,DWORD flag)
{
	GraphicsGraph * grp = GetGG();
	
	i_math::matrix43f mat;
	mat.setTranslation((x-_x),(y-_y),0.0f);
	mat = _mat*mat;	
	
	_UpdateTransform(mat);
}

BOOL CGuiAgent_2DTransform::OnSetCursor(int x,int y,DWORD flag)
{
	if(_bInDrag)
		_SetCursor(IDC_CURSORMOVE);
	else
		_SetCursor(NULL);
	return TRUE;
}

BOOL CGuiAgent_2DTransform::OnMouseWheel(int delta,DWORD flag)
{
	i_math::pos2di pt;

	i_math::matrix43f matOld;
	_CalcTransform(matOld);

	float scale =(delta>0)?1.2f:0.9f;
	// 将缩放限制在一定范围之内
	if( scale < 1.0f && matOld.getScale().x < ( 1.0f / _fMaxZoomOut ) )
		return TRUE;
	if( scale > 1.0f && matOld.getScale().x > _fMaxZoomIn )
		return TRUE;

	CGuiView * view = (CGuiView *)GetView();
	// 
	i_math::matrix43f matInvse,mat,temp;
	
	i_math::recti rc;
	_GetClientRect(rc);
	_GetCursorPos(pt);
// 	pt = rc.getCenter();

	i_math::vector3df pos;
	pos.x = pt.x;
	pos.y = pt.y;
	
	mat.setTranslation(-pos.x,-pos.y,0.0f);
	temp.setScale(scale,scale,1.0f);
	mat = mat*temp;
	temp.setTranslation(pos.x,pos.y,0.0f);
	mat = mat*temp;

	mat = matOld*mat;	
	_UpdateTransform(mat);

	return TRUE;
}

void CGuiAgent_2DTransform::_UpdateTransform(i_math::matrix43f & mat)
{
	i_math::pos2df off,scale;
		
	off.x = mat.getTranslation().x;
	off.y =  mat.getTranslation().y;
	scale.x = mat.getScale().x;
	scale.y = mat.getScale().y;

	_SetTransformGG(off,scale);
	
	OnUpdateTransform(off, scale );
	
	_Redraw(FALSE);
}

