
#pragma once

#include <assert.h>

//Should be synchronize with g_RGIBInfo
#define ID_RGIB_MoveUp (1<<0)
#define ID_RGIB_MoveDown (1<<1)
#define ID_RGIB_Remove (1<<2)
#define ID_RGIB_Clone (1<<3)
#define ID_RGIB_Clear (1<<4)
#define ID_RGIB_Browse (1<<5)
#define ID_RGIB_New (1<<6)
#define ID_RGIB_Menu (1<<7)
#define ID_RGIB_Reserve1 (1<<8)
#define ID_RGIB_Reserve2 (1<<9)
#define ID_RGIB_Reserve3 (1<<10)

#define ID_RGIB_Visible (1<<11)
#define ID_RGIB_Reserve4 (1<<12)
#define ID_RGIB_Reserve5 (1<<13)
#define ID_RGIB_Reserve6 (1<<14)

#define RDIB_COUNT 15

//Rich Grid Item Button
struct RGIB_Info
{
	DWORD id;
	int idxImage;
	BOOL bCheckBox;
};



class CRichGrid_ButtonItem;

class CRichGridItemButton : public CXTButton
{
public:
	CImageList *GetImageList();

	DECLARE_MESSAGE_MAP()
protected:
	CRichGrid_ButtonItem* _pItem;


	static CImageList _imagelist;

	friend class CRichGrid_ButtonItem;
public:
	afx_msg void OnBnClicked();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};

class CRichGrid_ButtonItem : public CXTPPropertyGridItem
{
public:
	CRichGrid_ButtonItem (CString strCaption);

	void ClearButtonMask();
	void AddButtonMask(DWORD maskBtns);

	virtual void OnButtonClick(DWORD idButton);
	virtual void OnButtonMenuCmd(DWORD idCmd);

	BOOL Bind(DWORD idButton,BOOL *b);

protected:
	virtual void OnDeselect();
	virtual void OnSelect();
	virtual CRect GetValueRect();

protected:
	void _GetButtonCount(DWORD &cLeft,DWORD &cRight);

	CRichGridItemButton *_GetButtonFromID(DWORD idButton);

protected:
	DWORD _maskBtn;
	CRichGridItemButton _buttons[RDIB_COUNT];
	BOOL *_binds[RDIB_COUNT];
	void _CreateButtons();
	void _ArrangeButtons();

};

template <class T>
class CRichGrid_VectorItem:public CRichGrid_ButtonItem
{
public:
	CRichGrid_VectorItem(CString strCaption):CRichGrid_ButtonItem(strCaption)
	{
		_vec=NULL;
	}

	BOOL Bind(std::vector<T>*vec)
	{
		_vec=vec;
		return TRUE;
	}

	virtual void OnButtonClick(DWORD idButton)
	{
		if ((!(idButton&(ID_RGIB_Clear|ID_RGIB_New)))||
			(!_vec))
		{
			CRichGrid_ButtonItem::OnButtonClick(idButton);
			return;
		}

		CRichGrid *grid=GetRichGrid(this);
		std::string path;
		grid->OnBeginItemChange(this);

		if (idButton==ID_RGIB_Clear)
		{
			_vec->clear();
			grid->OnItemChange(this);
		}
		if (idButton==ID_RGIB_New)
		{
			path=grid->PathFromItem(this);
			_vec->resize(_vec->size()+1);
			grid->OnItemChange(this);
		}

		grid->OnEndItemChange(this);

		//NOTE:after calling OnEndItemChange(..),the grid has been reset and this item 
		//is no longer valid,so never use "this" pointer in the following code

		if (idButton==ID_RGIB_New)
		{//select&expand the new added item
			CXTPPropertyGridItem *item=grid->ItemFromPath(path.c_str());
			if (item)
			{
				item->Expand();
				//select&expand the last child of item
				if (item->GetChilds()->GetCount()>0)
				{
					item=item->GetChilds()->GetAt(item->GetChilds()->GetCount()-1);
					item->Select();
					item->Expand();
				}
			}
		}

		return;
	}

protected:
	std::vector<T>*_vec;
};

template <class T>
class CRichGrid_VectorElemItem:public CRichGrid_ButtonItem
{
public:
	CRichGrid_VectorElemItem(CString strCaption):CRichGrid_ButtonItem(strCaption)
	{
		_vec=NULL;
		_iElem=0;
	}

	BOOL Bind(std::vector<T>*vec,DWORD iElem)
	{
		_vec=vec;
		_iElem=iElem;
		return TRUE;
	}

	virtual void OnButtonClick(DWORD idButton)
	{
		if ((!(idButton&(ID_RGIB_MoveDown|ID_RGIB_MoveUp|ID_RGIB_Remove|ID_RGIB_Clone)))||
			(!_vec))
		{
			CRichGrid_ButtonItem::OnButtonClick(idButton);
			return;
		}

		assert(_iElem<_vec->size());

		std::string path,path2;
		CRichGrid *grid=GetRichGrid(this);

		if (idButton&(ID_RGIB_MoveUp|ID_RGIB_MoveDown))
		{
			CXTPPropertyGridItem *item;
			if (idButton==ID_RGIB_MoveUp)
				item=grid->GetPrevItem(this);
			else
				item=grid->GetNextItem(this);
			if (!item)
				return;
			path=grid->PathFromItem(this);
			path2=grid->PathFromItem(item);
		}


		grid->OnBeginItemChange(this);
		switch (idButton)
		{
			case ID_RGIB_Remove:
			{
				_vec->erase(_vec->begin()+_iElem);
				break;
			}
			case ID_RGIB_MoveUp:
			{
				if (_iElem<=0)
					return;
				T t;
				t=(*_vec)[_iElem];
				(*_vec)[_iElem]=(*_vec)[_iElem-1];
				(*_vec)[_iElem-1]=t;
				break;
			}
			case ID_RGIB_MoveDown:
			{
				if (_iElem>=_vec->size()-1)
					return;
				T t;
				t=(*_vec)[_iElem];
				(*_vec)[_iElem]=(*_vec)[_iElem+1];
				(*_vec)[_iElem+1]=t;
				break;
			}
			case ID_RGIB_Clone:
			{
				path=grid->PathFromItem(GetParentItem());
				_vec->resize(_vec->size()+1);
				(*_vec)[_vec->size()-1]=(*_vec)[_iElem];
				break;
			}
			default:
				assert(FALSE);
		}

		grid->OnItemChange(this);
		grid->OnEndItemChange(this);

		//NOTE:after calling OnEndItemChange(..),the grid has been reset and this item 
		//is no longer valid,so never use "this" pointer in the following code
		if (idButton&(ID_RGIB_MoveUp|ID_RGIB_MoveDown))
		{
			CXTPPropertyGridItem *item;
			item=grid->ItemFromPath(path.c_str());
			item->Collapse();
			item=grid->ItemFromPath(path2.c_str());
			item->Collapse();
			item->Select();
		}

		if (idButton==ID_RGIB_Clone)
		{//select&expand the new added item
			CXTPPropertyGridItem *item=grid->ItemFromPath(path.c_str());
			if (item)
			{
				item->Expand();
				//select&expand the last child of item
				if (item->GetChilds()->GetCount()>0)
				{
					item=item->GetChilds()->GetAt(item->GetChilds()->GetCount()-1);
					item->Select();
					item->Expand();
				}
			}
		}

		return;
	}

protected:
	std::vector<T>*_vec;
	DWORD _iElem;
};

