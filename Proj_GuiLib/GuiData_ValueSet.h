#pragma once


#include "GuiLib.h"
#include "editor/editor.h"

#include "ref/ref.h"

#include "ruler/ruler.h"

#include <unordered_map>

#define RULERX_THICK 16
#define RULERY_THICK 50

#define KEY_RADIUS 6

#define COLOR_BAND_THICK 16
#define COLOR_BAND_GAP 14
#define COLOR_BAND_EDGE 8

struct ValueSet;
class CRichGrid_ValueSetItem;
struct ValueSetEntry
{
	ValueSetEntry()
	{
		ref=NULL;
	}
	ValueSet *GetValueSet(std::string *path);
	CRichGrid_ValueSetItem*GetItem();

	Ref *ref;
};

class CRichGrid;
class CXTPPropertyGridItem;
struct ValueSetGroup
{
	DEFINE_CLASS(ValueSetGroup);
	ValueSetGroup()
	{
		refGrid=NULL;
		ver=0;
	}
	~ValueSetGroup()
	{
		SAFE_RELEASE(refGrid);
	}

	void AddEntry(CRichGrid *grid,CXTPPropertyGridItem*item,const char *classname);
	void ClearEntries();

	i_math::rectf CalcBound(const char *selentry,BOOL bSelOnly,BOOL bClose);

	Ref *refGrid;

	std::vector<ValueSetEntry> entries;
	DWORD ver;
};


class CRichGrid_ValueSetItem;
class CColorAlphaPage;
class CGuiData_ValueSet : public GeData
{	
public:
	struct SelKey
	{
		std::string grp;
		std::string entry;
		int idxKey;
	};
	CGuiData_ValueSet();
	~CGuiData_ValueSet();
	virtual const char *GetName()	{		return "ValueSet";	}
	void	Clear();

	BOOL UpdateSelGrp();//返回有没有发生变化

	void SetGrpGrid(const char *grp,CRichGrid *grid);
	CRichGrid *GetGrid();
	void AddEntry(const char *grp,CRichGrid *grid,CXTPPropertyGridItem*item,const char *classname);
	void ClearGroup(const char *grp);

	ValueSetGroup *GetSelGroup()	{		return FindGroup(_selgrp.c_str());	}
	ValueSetGroup *FindGroup(const char *grp);
	ValueSetEntry*GetSelEntry();
	int GetSelKey();
	void SetSelKey(int idxKey);

public:
	ValueSetGroup * _EnsureGrp(const char *grp);
	void _UpdateSelKey();

	std::unordered_map<std::string,ValueSetGroup*>_grps;
	std::string _selgrp;//当前选中的group
	std::string _selentry;//当前选中的entry
	SelKey _selkey;//当前选中的key

	CColorAlphaPage *_colpage;


	friend class CValueSetGrid;
	friend class CValueSetDialog;
	friend class CGuiView_ValueSet;
};


