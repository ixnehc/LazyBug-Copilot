
#include "stdh.h"

#include "ItemDataEditListBox.h"

#include "WndBase.h"

#include "resource.h"

void CItemDataEditListBox::Create(CWnd * pParent,DWORD idCtrl,const char * nameHeader)
{
	CRect rc;
	GET_CONTROL_RECT(pParent,idCtrl,rc);
	CXTEditListBoxEx::Create(WS_VISIBLE|WS_CHILD|LBS_NOTIFY,rc,pParent,idCtrl);
	CXTEditListBoxEx::Initialize();
	CXTEditListBoxEx::SetListEditStyle(fromMBCS(nameHeader), LBS_XT_DEFAULT);
}
void CItemDataEditListBox::OnSelChange()
{
	LBItemData data = GetSelData();
	OnSelChange(data);
}
void CItemDataEditListBox::OnNewItem( int iItem,const char * name)
{
	if(iItem==-1&&name!=NULL){  //新建
		LBItemData data = OnNewItem(name);
		if(data!=LBItemData_Invalid){
			int count = GetCount();
			_itemDatas.push_back(data);
			SetItemData(count-1,data);
			SetCurSel(count-1);
		}
	}
	else{//修改名称
		CString strName;
		int len = GetTextLen(iItem);
		if(len>0){
			GetText(iItem,strName.GetBuffer(len));
			OnNameChange(GetSelData(), toMBCS(strName));
			strName.ReleaseBuffer();
		}
	}
}
DWORD CItemDataEditListBox::GetSelData()
{
	LBItemData itemData = LBItemData_Invalid;
	int iSel = GetCurSel();
	if(iSel>=0)
		itemData = (LBItemData)GetItemData(iSel);
	return itemData;
}

void CItemDataEditListBox::Bind(LBItemData* itemDatas,DWORD count)
{	
	if(!itemDatas){
		ResetContent();
		_itemDatas.clear();
		return;
	}

	DWORD sz = _itemDatas.size();

	//检查是否发生改变
	BOOL bItemDataChange = TRUE;
	if(sz==count){
		int i = 0;
		for(;i<count;i++){
			if(_itemDatas[i]!=itemDatas[i])
				break;
		}

		//没有任何的改变
		if(i>=count)
			bItemDataChange = FALSE;
	}

	BOOL bShowNameChange = FALSE;
	std::string nameShow;
	for(int i = 0;i<_itemDatas.size();i++)
	{
		if(GetShowName(_itemDatas[i],nameShow))
		{
			CString strName;
			int len = GetTextLen(i);
			if(len!=0)
			{
				GetText(i,strName.GetBuffer(len));
				if (nameShow.compare(toMBCS(strName)) != 0)
				{
					bShowNameChange = TRUE;
					break;
				}
				strName.ReleaseBuffer();
			}
			else
			{
				if(!nameShow.empty())
				{
					bShowNameChange = TRUE;
					break;
				}
			}
		}
	}
	
	//没有发生任何的变化不更新
	if((!bItemDataChange)
		&&(!bShowNameChange)
		&&count==GetCount())
		return;


	LBItemData dataPtrOld = GetSelData(); //上一次数据的指向

	
	int sel = GetCurSel();
	LBItemData curItemData = LBItemData_Invalid;
	if(sel>=0)
		curItemData = (LBItemData)GetItemData(sel);
	
	int selNew = -1;
	int nItem = 0;

	ResetContent();
	_itemDatas.clear();
	std::string showName;
	for(int i = 0;i<count;i++)
	{
		if(GetShowName(itemDatas[i],showName))
		{
			if(curItemData==itemDatas[i])
				selNew = nItem;
			_itemDatas.push_back(itemDatas[i]);
			AddString(fromMBCS(showName.c_str()));
		}
	}

	for(int i = 0;i<_itemDatas.size();i++)
		SetItemData(i,_itemDatas[i]);
	
	if(selNew<0&&!_itemDatas.empty())
		selNew = 0;

	if(selNew>=0)
		SetCurSel(selNew);

	LBItemData dataPtrCur = GetSelData();
	if(dataPtrCur!=dataPtrOld)
		OnSelChange(dataPtrCur);
}

void CItemDataEditListBox::SetSelData(LBItemData data)
{
	DWORD count = GetCount();
	for(int i = 0;i<count;i++){
		LBItemData dataItem = (LBItemData)GetItemData(i);
		if(dataItem==data){
			SetCurSel(i);
			break;
		}
	}
}



