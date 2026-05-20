#pragma once

#include "EditListBoxEx.h"

typedef DWORD LBItemData;
#define LBItemData_Invalid 0xffffffff

//为定时刷新定制控件 每个项对应了一个数据的索引 认为该项对应的索引发生改变则该项发生改变 
class CItemDataEditListBox :public CXTEditListBoxEx
{
public:
	void Create(CWnd * pParent,DWORD idCtrl,const char * nameHeader);
	void  Bind(LBItemData* dataes,DWORD count);  //Bind every time
	LBItemData GetSelData();
	void SetSelData(LBItemData data);
	virtual LBItemData OnNewItem(const char * name){return LBItemData_Invalid;}
	virtual void  OnSelChange(LBItemData data){}
	virtual BOOL GetShowName(LBItemData data,std::string &name) = 0;
	virtual void OnNameChange(LBItemData data,const char * name){} //名称发生改变
	
private:
	//CXTEditListBoxEx overriding
	virtual void OnNewItem( int iItem,const char * name);
	virtual void OnSelChange();
private:
	std::vector<DWORD> _itemDatas;
};

