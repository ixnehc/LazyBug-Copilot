#pragma once

#include "GuiLib.h"

#include <map>
#include <string>

class CxImage;

class GuiLib_Api CxImageWnd:public CStatic
{
public:
	CxImageWnd();
	~CxImageWnd();
	void SetBlank();
	void SetError();
	void SetImage(const char * namefile);
	CxImage * GetImage(){return _image;}
public:
	DECLARE_MESSAGE_MAP()

protected:
	CxImage *_image;
public:
	afx_msg void OnPaint();
};

