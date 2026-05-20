
#pragma once
#include "GuiLib.h"

#include "resource.h"

#include "SlideTab.h"



class GuiLib_Api CResEditTab:public CSlideTab
{
public:
	BOOL Create(CWnd *pParent,CRect &rc,int id);
protected:
	CImageList _imglist;

};