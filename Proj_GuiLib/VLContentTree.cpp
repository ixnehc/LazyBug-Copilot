#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "assert.h"

#include "VLContentTree.h"

//should be synchronized with the value in vegebrush.cpp
#define NODETYPE_FOLDER 1
#define NODETYPE_CLASS_BEGIN 2
#define NODETYPE_CLASS_END 1002
#define NODETYPE_ISCLASS(type) (((type)>=NODETYPE_CLASS_BEGIN)&&((type)<=NODETYPE_CLASS_END))

UINT CVLContentTree::_GetImageID()
{
	return IDB_RESTREEICON;
}


DWORD CVLContentTree::_GetImageIdx(NodeType type)
{
	if (type==NODETYPE_FOLDER)
		return 0;
	if (NODETYPE_ISCLASS(type))
		return 1;
	assert(FALSE);
	return 0;
}


NodePtr CVLContentTree::GetCurSel()
{
	HTREEITEM hItem=GetFirstSelectedItem();
	if (!hItem)
		return NULL;
	if (GetNextSelectedItem(hItem))
		return NULL;//More than 1 is selected
	NodeHandle hNode=(NodeHandle)GetItemData(hItem);
	if (hNode==NodeHandle_Null)
		return NULL;

	return _tree->GetPtr(hNode);
}
