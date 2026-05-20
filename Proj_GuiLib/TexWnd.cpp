/********************************************************************
	created:	2007/2/6   12:56
	filename: 	e:\IxEngine\Proj_GuiLib\TexWnd.cpp
	author:		cxi
	
	purpose:	a wnd to display a texture
*********************************************************************/

#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "ximage.h"


#include "RenderSystem/IRenderSystem.h"



#include "resource.h"

#include "TexWnd.h"


CTexWnd::CTexWnd()
{
	_tex=NULL;
	_iFrame=0;
}


BOOL CTexWnd::SetTex(ITexture *tex,DWORD iFrame)
{
	if (tex==NULL)
	{
		SetBlank();
		return TRUE;
	}

	extern CxImage *ImageFromTex(ITexture*tex,CxImage *container);
	ImageFromTex(tex,_image);

	SetError();

	return FALSE;
}

