
#pragma once

#include "WorldSystem/IBrushLib.h"

#include "ItemDataEditListBox.h"

#include "LyObjGrid.h"

#include "BrushLibSscBtn.h"

class CBrushLibLVCtrl
{
public:
	CBrushLibLVCtrl(void);

	void Create(CWnd * pParent,DWORD idCtrlList,DWORD idCtrlGrid,DWORD idBnt,const char * name);
	
	void SetSsc(CSscSystemWrapper * ssc){_ssc = ssc;}

	void Bind(IBrushLib * pLib);
	
	const IBrush * GetSelBrush();

	BRUID GetSelUID();
	
	void SetSelUID(const BRUID &uid);

	class CBrushLibGrid :public CLyObjGrid<IBrush>
	{
		virtual void OnItemChange(CXTPPropertyGridItem *item);
		virtual void OnEndItemChange(CXTPPropertyGridItem *item); 	
		CBrushLibLVCtrl * _owner;
		friend class CBrushLibLVCtrl;
	};
	
	class CBrushLibList: public CItemDataEditListBox
	{
		virtual LBItemData OnNewItem(const char * name);
		virtual BOOL GetShowName(LBItemData itemData,std::string &name);
		virtual void OnNameChange(LBItemData itemData,const char * name);//名称发生改变
		virtual void OnDeleteItem();
		CBrushLibLVCtrl * _owner;
		friend class CBrushLibLVCtrl;
	};

	struct ItemData
	{
		DWORD itemData;
		BRUID uid;
	};
	
	BOOL _OnLoad();
	BOOL _OnSave();
	BOOL _CheckEditable(); //检查库是否可以修改 

	BRUID _FromItemData(DWORD itemData);

	friend class CBrushLibList;
	friend class CBrushLibGrid;

protected:
	CBrushLibList   _wndList;
	CBrushLibGrid	_wndGrid;
	CBrushLibSscBtn			_sscBt;
	IBrushLib *		_pLib;
	
	CSscSystemWrapper * _ssc;
	std::vector<ItemData> _itemTable;
};




