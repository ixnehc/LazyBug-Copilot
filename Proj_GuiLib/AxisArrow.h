#pragma once
#include "../Interfaces/RenderSystem/IRenderSystem.h"
#include "RenderPortBase.h"
#include "GuiLib.h"
class GuiLib_Api  CAxisArrow
{
private:
	CAxisArrow  * _instance;
	IMesh  * _mesh;
	IMtrl  * _mtrl;
public:
	CAxisArrow(void);
	~CAxisArrow(void);
	BOOL Init(IRenderSystem *pRS);
	void Release();
	void Draw(IRenderer * render,i_math::matrix43f * matrix=NULL,BOOL bHightLight =FALSE);
	BOOL HitTest(i_math::line3df & line ,i_math::matrix43f *mat);
};
