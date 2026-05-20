
#pragma once

#include "WorldSystem/IBrushLib.h"

#include "NodeTreeCtrl.h"

#include "LyObjGrid.h"

#include "BrushLibSscBtn.h"

class CBrushLibTVCtrl
{
public:
	CBrushLibTVCtrl(void);

	void Create(CWnd * pParent,DWORD idCtrlTree,DWORD idCtrlGrid,DWORD idBnt,const char * name,BOOL bSupportChecked);
	
	void SetSsc(CSscSystemWrapper * ssc){_ssc = ssc;}

	void Bind(IBrushLib * pLib);
	
	const IBrush * GetSelBrush();

	BRUID GetSelUID();
	
	void SetSelUID(const BRUID &uid);

	class CBrushLibGrid :public CLyObjGrid<IBrush>
	{
		virtual void OnItemChange(CXTPPropertyGridItem *item);
		virtual void OnEndItemChange(CXTPPropertyGridItem *item); 	
		CBrushLibTVCtrl * _owner;
		friend class CBrushLibTVCtrl;
	};
	
	class CBrushLibTreeCtrl:public CNodeTreeCtrl
	{
		virtual DWORD _GetImageIdx(NodeHandle hNode,SscState state);
		virtual UINT  _GetImageID();
		virtual BOOL _SupportCheckBox(){return _bSupportCheckBox;}
		void PreSubclassWindow();
		friend class CBrushLibTVCtrl;
		BOOL _bSupportCheckBox;
	};

	struct ItemData
	{
		DWORD itemData;
		BRUID uid;
	};
	
	BOOL _OnLoad();
	BOOL _OnSave();
	BOOL _CheckEditable(); //检查库是否可以修改 

	friend class CBrushLibList;
	friend class CBrushLibGrid;

protected:
	CBrushLibGrid		_wndGrid;
	CBrushLibTreeCtrl	_wndTree;
	CBrushLibSscBtn			_sscBt;
	IBrushLib *		_pLib;
	DWORD _ver;
	CSscSystemWrapper * _ssc;
};




