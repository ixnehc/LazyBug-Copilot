#include "stdh.h"
#include ".\guiagent_2dmove.h"
#include "graphicsgraph.h"

#pragma  warning(disable:4244)

CGuiAgent_2DMove::CGuiAgent_2DMove(void)
{
}

CGuiAgent_2DMove::~CGuiAgent_2DMove(void)
{

}
BOOL CGuiAgent_2DMove::OnBeginDrag(int x,int y,DWORD flag)
{
	_x = x;
	_y = y;
	return TRUE;
}
void CGuiAgent_2DMove::OnEndDrag(int x,int y,DWORD flag)
{	

	i_math::matrix43f mat;
	mat.setTranslation((x-_x),(y-_y),0.0f);
	mat = _mat*mat;

	_mat = mat;
	_x = 0;
	_y = 0;
}
void CGuiAgent_2DMove::OnDrag(int x,int y,DWORD flag)
{
	GraphicsGraph * grp = GetGG();
	
	i_math::matrix43f mat;
	mat.setTranslation((x-_x),(y-_y),0.0f);
	mat = _mat*mat;	
	
	_UpdateTransform(mat);
}
BOOL CGuiAgent_2DMove::OnMouseWheel(int delta,DWORD flag)
{
	i_math::pos2di pt;

	float scale =(delta>0)?1.2f:0.9f;
	if(scale<1.0f&&_mat.getScale().x<0.02f)
		return TRUE;

	CGuiView * view = (CGuiView *)GetView();
	// 
	i_math::matrix43f matInvse,mat,temp;
	
	i_math::recti rc;
	_GetClientRect(rc);
	pt = rc.getCenter();

	i_math::vector3df pos;
//	_mat.transformVect(pos,pos);
	pos.x = pt.x /*- pos.x*/;
	pos.y = pt.y /*- pos.y*/;
	
	mat.setTranslation(-pos.x,-pos.y,0.0f);
	temp.setScale(scale,scale,1.0f);
	mat = mat*temp;
	temp.setTranslation(pos.x,pos.y,0.0f);
	mat = mat*temp;

	mat = _mat*mat;	
	_UpdateTransform(mat);

	_mat = mat;

	return TRUE;
}
void CGuiAgent_2DMove::_UpdateTransform(i_math::matrix43f & mat)
{
	i_math::pos2df posMove;
	
	i_math::xformf xf;
	xf.fromMatrix(mat);
	
	int x = xf.pos.x /*mat.getTranslation().x*/;
	int y = xf.pos.y /*mat.getTranslation().y*/;
	float scale = xf.scale.x /*mat.getScale().x*/;

	posMove.set(x,y);

	_TransformGG(posMove,scale);
	_Redraw();
}






