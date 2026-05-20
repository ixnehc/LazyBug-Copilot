#pragma once
#include "UVAnimPanel.h"
#include "GuiLib.h"


class GuiLib_Api CColAnimPanel : public CUVAnimPanel
{
public:
	CColAnimPanel(void);
	~CColAnimPanel(void);

	virtual void Draw(IRenderPort *rp);
};
