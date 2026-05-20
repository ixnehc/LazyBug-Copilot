
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/IEntitySystemDefines.h"
#include "protograph.h"

#include "GuiAgent_2DTransform.h"

#include "GuiAgent_3dnodeedit.h"

class IMesh;
class IMtrl;
class ILight;

class CRichGrid_AstUIDSetItem;
class CRichGrid;



class CGuiAgent_AstUIDSet:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_AstUIDSet()
	{
		_bWorking=FALSE;
		_bAccum=FALSE;
	}

	struct Binding
	{
		Binding()
		{
			grid=NULL;
			item=NULL;
		}
		BOOL IsValid();
		std::vector<DWORD> *GetBindUIDs();

		void OnBeginChange();
		void OnChange();
		void OnEndChange();

		CRichGrid * grid;
		CRichGrid_AstUIDSetItem *item;

	};

	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	virtual BOOL OnDraw();
	virtual BOOL OnTimer(int dt,DWORD flag);
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnLButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag);

protected:

	void _UpdateCur();
	Binding _GetCurBinding();
	void _Select(int x,int y);
	CRichGrid_AstUIDSetItem *_GetCurItem();
	CRichGrid *_GetCurGrid();

	std::string _curGrid;
	std::string _curItem;

	void _Sel(ProtoNodeID *inrects,DWORD c);
	std::vector<DWORD>_initials;
	i_math::pos2di _start;
	i_math::recti _rcDraw;
	BOOL _bAccum;


	BOOL _bWorking;

};

