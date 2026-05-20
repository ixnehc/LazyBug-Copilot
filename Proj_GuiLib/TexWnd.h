#pragma once

#include "GuiLib.h"

#include "CxImageWnd.h"




class ITexture;

class GuiLib_Api CTexWnd:public CxImageWnd
{
public:
	CTexWnd();
	BOOL SetTex(ITexture *tex,DWORD iFrame);//pass tex as NULL to clear content


protected:
	ITexture *_tex;
	DWORD _iFrame;

};

