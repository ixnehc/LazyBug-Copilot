
#pragma once

#include "GuiEditor.h"

#include "RenderSystem/IRenderSystem.h"

#include "GuiAgent_3dnodeedit.h"

class IProbeCubeMapEditor;
class CGuiAgent_ProbeMove :public CGuiAgent_3DNodeMatEdit
{
public:
	CGuiAgent_ProbeMove(Matrix_EditMode mode);
	~CGuiAgent_ProbeMove(void);
	
	virtual BOOL OnDraw();
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);

protected:
	void _DrawAbbCorner(i_math::aabbox3df &abb);
	BOOL _TouchRes();
	void _BindMat();
	
	void _EndMatrixEdit(i_math::matrix43f *mat);
	//3DNode MatEdit
	virtual  void*_GetSelBuf();
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual i_math::pos2di *_GetBlock(H3DNode node);//返回这个node位于那个block内
	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);	
	IProbeCubeMapEditor * _GetEditor();
	DWORD * _GetVer();
	float _Clamp(float v,float ov);
private:
	i_math::vector3df _corners[8]; //角点信息
	int _iCorSel;	//当前选中得角点
	IMtrl * _mtrl;
	IMesh * _mesh;
	ILight * _light;
	i_math::matrix43f _matTemp;
	i_math::pos2di _ptBlkTemp;
};



