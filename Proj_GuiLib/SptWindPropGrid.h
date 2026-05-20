#pragma once

#include "RichGrid.h"
#include "GuiLib.h"
#include "WndBase.h"
#include "resdata/ResDataDefines.h"

class GuiLib_Api CSptWindPropGrid :public CRichGrid ,public CUIEvent
{
public:
	enum
	{
		Event_EndChange,
	};
	CSptWindPropGrid(void);
	~CSptWindPropGrid(void);
public:
	void BindData(const SptWndCfg * cfg);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);
	SptWndCfg & GetWindCfg(){return _cfg; }
private:
	SptWndCfg _cfg;
};
