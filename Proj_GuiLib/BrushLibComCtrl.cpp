
#include "stdh.h"

#include "BrushLibComCtrl.h"

#include "stringparser/stringparser.h"

#include "SscBase.h"

CBrushLibComCtrl::CBrushLibComCtrl(void)
{
	_pLib = NULL;
	_ssc = NULL;
}

void CBrushLibComCtrl::Create(CWnd * pParent,DWORD idCtrlList,DWORD idCtrlGrid,DWORD idBnt,const char * name)
{
	_wndGrid.Create(pParent,idCtrlGrid);
	_wndGrid.SetWindowText(name);
	_wndList.Create(pParent,idCtrlList,name);
	
	//创建source safe 功能Button
	CRect rc;
	GET_CONTROL_RECT(pParent,idBnt,rc);
	_sscBt.Create("",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,rc,pParent,IDC_BT_SSC);

	SscBtnCallback dlgbt;
	dlgbt.bind(this,CBrushLibComCtrl::_OnLoad);
	_sscBt.BindNotifyLoad(dlgbt);
	dlgbt.bind(this,CBrushLibComCtrl::_OnSave);
	_sscBt.BindNotifySave(dlgbt);

	_wndGrid._owner = this;
	_wndList._owner = this;
}

BOOL CBrushLibComCtrl::_OnLoad()
{
	if(_pLib)
		_pLib->ReLoad();
	return TRUE;
}

BOOL CBrushLibComCtrl::_OnSave()
{
	if(_pLib)
		_pLib->Save();
	return TRUE;
}

void CBrushLibComCtrl::Bind(IBrushLib * pLib)
{
	if(!pLib){
		_pLib = NULL;
		_wndGrid.BindData(NULL);
		_wndList.Bind(NULL,0);
		_wndList.EnableWindow(FALSE);
		_wndGrid.EnableWindow(FALSE);
		_sscBt.Bind("",NULL);
	}
	else{
		_pLib = pLib;
		BRUID * pIDX = NULL;
		DWORD c = 0;
		
		pLib->Enum(pIDX,c);
		_itemTable.resize(c);
		std::vector<DWORD> idx(c);
		for(DWORD i = 0;i< c;i++){
			idx[i] = i;
			_itemTable[i].uid = pIDX[i];
			_itemTable[i].itemData = i;
		}
		_wndList.Bind(&idx[0],c);
		_sscBt.Bind(_pLib->GetPath(),_ssc);
		
		SscState state;
		if(_ssc&&_ssc->GetState(_pLib->GetPath(),state)
			&&!(state==SSC_CHECKEDOUT_ME||state==SSC_NOTCONTROLLED)){
				_wndList.EnableWindow(FALSE);
				_wndGrid.EnableWindow(FALSE);
		}
		else{
			_wndList.EnableWindow(TRUE);
			_wndGrid.EnableWindow(TRUE);
		}
	}

	

	const IBrush * br = GetSelBrush();
	_wndGrid.BindData(br);
}

void CBrushLibComCtrl::SetSelUID(const BRUID &uid)
{
	for(int i = 0;i<_itemTable.size();i++){
		if(_itemTable[i].uid==uid){
			_wndList.SetSelData(_itemTable[i].itemData);
			break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void CBrushLibComCtrl::CBrushLibGrid::OnItemChange(CXTPPropertyGridItem *item)
{
	if(_owner->_pLib){
		IBrush  * br = GetData();
		_owner->_pLib->Set(br->GetUID(),br);
	}
}

void CBrushLibComCtrl::CBrushLibGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	if(_owner->_pLib){
		IBrush * br = GetData();
		if(br)
			_owner->_pLib->Set(br->GetUID(),br);
		_owner->_pLib->Save();
	}
	CLyObjGrid<IBrush>::OnEndItemChange(item);
}

//////////////////////////////////////////////////////////////////////////

DWORD  CBrushLibComCtrl::CBrushLibList::OnNewItem(const char * name)
{
	DWORD itemData = LBItemData_Invalid;

	if(_owner->_pLib&&name){
		if(strcmp(name,"")!=0){
			IBrush * br = _owner->_pLib->New();
			br->SetName(name);
			BRUID uid = br->GetUID();
			DWORD sz = _owner->_itemTable.size();
			_owner->_itemTable.resize(sz+1);
			_owner->_itemTable.back().itemData = sz;
			_owner->_itemTable.back().uid = uid;
			_owner->_pLib->Save();
		}
		else{
			::MessageBox(GetSafeHwnd(),"名称不可为空！","Error",MB_OK);
		}
	}
	
	_owner->Bind(_owner->_pLib);//更新

	return itemData;
}

BRUID CBrushLibComCtrl::_FromItemData(DWORD itemData)
{
	BRUID uid = INVALID_BRUID;

	for(DWORD i = 0;i<_itemTable.size();i++){
		if(_itemTable[i].itemData == itemData)	{
			uid = _itemTable[i].uid;
			break;
		}
	}

	return uid;
}

const IBrush * CBrushLibComCtrl::GetSelBrush() 
{
	const IBrush * br = NULL;

	if(_pLib){
		LBItemData itemData = _wndList.GetSelData();
		BRUID uid = _FromItemData(itemData);
		br = _pLib->Get(uid);
	}

	return br;
}

BRUID CBrushLibComCtrl::GetSelUID()
{
	DWORD itemData = _wndList.GetSelData();
	return _FromItemData(itemData);
}

BOOL CBrushLibComCtrl::CBrushLibList::GetShowName(LBItemData  itemData,std::string &name)
{
	if(_owner->_pLib){
		BRUID brID = _owner->_FromItemData(itemData);
		const IBrush * br = _owner->_pLib->Get(brID);
		if(br){
			name = br->GetName();
			return TRUE;
		}
	}
	return FALSE;
}

void CBrushLibComCtrl::CBrushLibList::OnNameChange(LBItemData itemData,const char * name)
{
	if(_owner->_pLib&&name){
		if(strcmp(name,"")!=0){
			BRUID brID = _owner->_FromItemData(itemData);
			IBrush * br = (IBrush *)_owner->_pLib->Get(brID);
			if(br)
				br->SetName(name);
			_owner->_pLib->Save();
		}
		else{
			::MessageBox(GetSafeHwnd(),"名称不可为空！","Error",MB_OK);
		}
	}
}

void  CBrushLibComCtrl::CBrushLibList::OnDeleteItem()
{
	if(_owner->_pLib){
		LBItemData  itemData = GetSelData();
		BRUID uid = _owner->_FromItemData(itemData);
		const IBrush * br = _owner->_pLib->Get(uid);
		if(br){
			std::string msgDump;
			FormatString(msgDump,"你确认删掉该项{ %s }?",br->GetName());
			if(::MessageBox(GetSafeHwnd(),msgDump.c_str(),"Waring",MB_YESNO)!=IDYES)
				return;
		}
		_owner->_pLib->Delete(uid);
	}
}




