#include "stdh.h"

#include "ResourceListCtrl.h"

#include "ResSelectorDlg.h"

#include "FileSystem/IFileSystem.h"

#include "resource.h"
#include "stringparser/stringparser.h"

CResourceListCtrl::CResourceListCtrl(IRenderSystem* rs) : 
	_pRS(rs)
{
}

CResourceListCtrl::~CResourceListCtrl()
{
}

BEGIN_MESSAGE_MAP(CResourceListCtrl, CTreeCtrl)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CREATE()
END_MESSAGE_MAP()

void CResourceListCtrl::_CreateImageList()
{
	extern BOOL CreateImageList(CImageList& il, UINT nID,int w,int h);

	CreateImageList(_imageList,IDB_RESTEXICON,16,16);

	SetImageList(&_imageList,TVSIL_NORMAL);		
}
int CResourceListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(NULL== _imageList.GetSafeHandle())
		_CreateImageList();

	return CTreeCtrl::OnCreate(lpCreateStruct);
}
void CResourceListCtrl::SetRootDir(const char* root)
{
	SetBrowseDir(root);
}

void CResourceListCtrl::SetFilter(const char* filter)
{
	if (filter)
	{
		_filter = filter;
		strlwr(const_cast<char*>(_filter.c_str()));
	}
}

const char* CResourceListCtrl::GetSelResource() const
{
	return _selResource.c_str();
}

const char *CResourceListCtrl::GetSelResource2()
{
	if(_bLocked)
		return "";
	else{
		HTREEITEM item = GetSelectedItem();
		if(item)
		{
			DWORD dtype = (DWORD)GetItemData(item);
			if(dtype==RLCF_FILE)
			{
				std::string str = toMBCS(GetItemText(item));
				_selResource = _browseDir;
				_selResource.append("\\");
				_selResource.append(str);
			}
			else
				_selResource.clear();
		}
	}

	return _selResource.c_str();
}

#define DIR_UP ".."
void CResourceListCtrl::SetBrowseDir(const char* root,const char *sel)
{
	if(NULL == GetImageList(TVSIL_NORMAL))	{
		SetImageList(&_imageList,TVSIL_NORMAL);
	}

	_bLocked = TRUE;
	SetRedraw(FALSE);

	DeleteAllItems();


	IFileSystem* pFS = NULL;
	if (_pRS)
		pFS = _pRS->GetFS();
	
	assert(pFS);

	_browseDir = root;

	HTREEITEM hItemSel=NULL;

	pFS->EnumBegin(root, FALSE);

	std::string item;
	DWORD count = pFS->EnumGetFolderCount();
	DWORD i = 0;
	if (strcmp(_pSelector->GetRootDir(), root) != 0)
	{
		item = DIR_UP;
		AddItem(item.c_str(), RLCF_DIR);
	}

	for (i = 0; i < count; i++)
	{
		item = pFS->EnumGetFolder(i);
		AddItem(item.c_str(), RLCF_DIR);
	}
	count = pFS->EnumGetFileCount();
	for (i = 0; i < count; i++)
	{
		item = pFS->EnumGetFile(i);
		if (IsPassFilter(item.c_str()))
		{
			HTREEITEM hItem=AddItem(item.c_str(), RLCF_FILE);
			if (sel[0])
			{
				if (StringEqualNoCase(item.c_str(),sel))
					hItemSel=hItem;
			}
		}
	}
	pFS->EnumEnd();


	GetParent()->PostMessage(WM_RL_BROWSEDIR);
	
	_bLocked = FALSE;
	SetRedraw(TRUE);
	ShowWindow(SW_SHOW);
	Invalidate(TRUE);
	UpdateWindow();

	if (hItemSel)
		Select(hItemSel,TVGN_CARET);

}


HTREEITEM CResourceListCtrl::AddItem(LPCSTR lpszFileName, DWORD userData)
{
	// Draw the icon.
	std::string strPath = _browseDir;
	strPath += "\\";
	strPath += lpszFileName;

	HTREEITEM item = NULL;
	int iil = (userData==RLCF_DIR)?0:1;

	item = InsertItem(fromMBCS(lpszFileName), iil, iil);
	SetItemData(item,userData);
	return item;
	
}
void CResourceListCtrl::PreSubclassWindow() 
{
	if(NULL== _imageList.GetSafeHandle())
		_CreateImageList();
	// TODO: Add your specialized code here and/or call the base class
}

void CResourceListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
//	BOOL bOutside = TRUE;
//	UINT nHitItem = ItemFromPoint(point, bOutside);
	
	HTREEITEM item = GetSelectedItem();
	std::string str = toMBCS(GetItemText(item));

	if(NULL!=item)
	{
		if (_browseDir != _pSelector->GetRootDir()&&str.compare(DIR_UP)==0)
		{
			std::string::size_type pos = _browseDir.find_last_of('\\');
			if (pos != std::string::npos)
			{
				std::string dir = _browseDir.substr(0, pos);
				SetBrowseDir(dir.c_str());
				return;
			}
		}
		else{
			DWORD dtype = (DWORD)GetItemData(item);
			std::string dir = _browseDir;
			dir.append("\\");
			dir.append(str.c_str());

			if(dtype==RLCF_DIR)
			{
				SetBrowseDir(dir.c_str());
			}
			else
			{
				_selResource = dir;
				((CResSelectorDlg*)GetParent())->OnLbnDblClick();
			}
		}
	}
}

BOOL CResourceListCtrl::IsPassFilter(const char* file) const
{
	char* p = (char*)strrchr(file, '.');
	if (p)
	{
		return (_filter.empty() ? TRUE : (_filter.find(strlwr(p)) != std::string::npos));
	}
	return FALSE;
}
