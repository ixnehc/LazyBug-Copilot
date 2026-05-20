
#pragma once
#include "GuiLib.h"
#include "math/range.h"
#include "strlib/strlib.h"
#include "valueset/valueset.h"
#include "ColorSet/ColorSet.h"
#include <vector>
#include <map>
#include <set>

#include "ref/ref.h"

struct GVar;
class IProtoLib;

class IRenderSystem;

#define RG_ADDCHILDITEM(item,itemclass,cap,desc)																				\
	if (!_GetStackTop())																																		\
		return NULL;																																			\
	if (_IsCaptionHide(cap))																																\
		return NULL;																																			\
	if (!_IsCaptionShow(cap))																																\
		return NULL;																																			\
	itemclass *item=(itemclass *)_GetStackTop()->AddChildItem(new itemclass(fromMBCS(cap)));							\
	_itemLast=item;																																			\
	_itemLast->SetDescription(fromMBCS(desc));																												\
	if (_bReadOnly)																																			\
		item->SetReadOnly(_bReadOnly);																										\
	if (_GetHook())																																			\
		_GetHook()->PostInsertItem(this,item,#itemclass);


struct RGItemState
{
	BOOL bExpand;
	BOOL bSel;
	BOOL bReadOnly;
};

class RGState:public std::map<std::string,RGItemState>
{
public:
	int iTopLine;
	RGState &operator=(RGState &src)
	{
		(std::map<std::string,RGItemState>&)(*this)=(std::map<std::string,RGItemState>&)src;
		iTopLine=src.iTopLine;
		return *this;
	}
};



struct GSem;
class ISscSystem;

class CRichGrid;
class CXTPPropertyGridItem;
struct RichGridHook
{
	virtual void PostResetContent(CRichGrid *grid)	{	}
	virtual void PostInsertItem(CRichGrid *grid,CXTPPropertyGridItem *item,const char *itemclass)	{	}
};

struct KeySet;
struct MtrlData;
class GuiLib_Api CRichGrid: public CXTPPropertyGrid
{
public:
	DEFINE_REF()

	CRichGrid()
	{
		_hook2=NULL;
		_bReadOnly=FALSE;
	}
	virtual ~CRichGrid()
	{
		BreakRef();
	}

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);

	void ResetContent();

	void SetReadOnly(BOOL bReadOnly);
	BOOL GetReadOnly()	{		return _bReadOnly;	}

	void BeginInsert(CXTPPropertyGridItem *item=NULL);
	void EndInsert();

	void PushInsert();
	void PopInsert();



	void RecordState(RGState &state);
	void RestoreState(RGState &state);

	const char *PathFromItem(CXTPPropertyGridItem *item);
	CXTPPropertyGridItem *ItemFromPath(const char *path);

	//Note:not working for a top level category item
	CXTPPropertyGridItem *GetPrevItem(CXTPPropertyGridItem *item);
	CXTPPropertyGridItem *GetNextItem(CXTPPropertyGridItem *item);

	void ExpandAll();
	void Expand();

	void LockPaint();
	void UnLockPaint();

	static void SetHook(RichGridHook *hook)	{		_hook=hook;	}
	void SetHook2(RichGridHook *hook)	{		_hook2=hook;	}

	CXTPPropertyGridItem *SeekItem(const char *cap);

	CXTPPropertyGridItem *InsertCategory(const char *cap,const char *desc,CXTPPropertyGridItem *item=NULL);
	CXTPPropertyGridItem *InsertItem(const char *cap,const char *desc,const char *value);
	CXTPPropertyGridItem *InsertBoolItem(const char *cap,const char *desc,BOOL *b,const char *constraint="");
	CXTPPropertyGridItem *InsertBoolItem(const char *cap,const char *desc,BYTE *b,const char *constraint="");
	CXTPPropertyGridItem *InsertBoolItem(const char *cap,const char *desc,DWORD *flag,DWORD mask,const char *constraint="");
	CXTPPropertyGridItem *InsertNameItem(const char *cap,const char *desc,std::string *s);
	CXTPPropertyGridItem *InsertFloatItem(const char *cap,const char *desc,float *f,float min,float max,float slideSpeed = 0.0f);
	CXTPPropertyGridItem *InsertAnimTickItem(const char *cap,const char *desc,AnimTick *v,float min,float max,float slideSpeed = 0.0f);
	CXTPPropertyGridItem *InsertUVXformItem(const char *cap,const char *desc,float *f);
	CXTPPropertyGridItem *InsertUVAddrItem(const char *cap,const char *desc,int *v);
	CXTPPropertyGridItem *InsertColorItem(const char *cap,const char *desc,float *col);
	CXTPPropertyGridItem *InsertColorItem(const char *cap,const char *desc,DWORD*col);
	CXTPPropertyGridItem *InsertTexItem(const char *cap,const char *desc,std::string *s,BOOL bSelPart=FALSE);
	CXTPPropertyGridItem *InsertResItem(const char *cap,const char *desc,std::string *s,int restype);
	CXTPPropertyGridItem *InsertComboItem(const char *cap,const char *desc,std::string *str,std::vector<std::string>&constraints);
	template<typename T>
	CXTPPropertyGridItem *InsertComboItem(const char *cap,const char *desc,T *idx,std::vector<std::string>&constraints)
	{
		RG_ADDCHILDITEM(item,CRichGrid_ComboItem<T>,cap,desc);

		item->Bind(idx,constraints,NULL);
		_AddItemBind(item,idx);

		return item;
	}
	CXTPPropertyGridItem *InsertButtonItem(const char *cap,const char *desc,DWORD IDs);
	CXTPPropertyGridItem *InsertFlagItem(const char *cap,const char *desc,std::string *sFlag,const char *sConstraint);
	CXTPPropertyGridItem *InsertFlagItem(const char *cap,const char *desc,DWORD*flag,const char *sConstraint);
	CXTPPropertyGridItem *InsertFlagItem(const char *cap,const char *desc,WORD*flag,const char *sConstraint);
	CXTPPropertyGridItem *InsertNumItem(const char *cap,const char *desc,int*v);
	CXTPPropertyGridItem *InsertNumItem(const char *cap,const char *desc,short*v);
	CXTPPropertyGridItem *InsertNumItem(const char *cap,const char *desc,WORD*v);
	CXTPPropertyGridItem *InsertNumItem(const char *cap,const char *desc,BYTE*v);
	CXTPPropertyGridItem *InsertRandomItem(const char *cap,const char *desc,DWORD*v);
	CXTPPropertyGridItem *InsertFolderItem(const char *cap,const char *desc,std::string*v,const char *pathRoot="");
	CXTPPropertyGridItem *InsertFileItem(const char *cap,const char *desc,std::string*v,const char *pathRoot="",const char *suffix="",const char *filter="");
	CXTPPropertyGridItem *InsertVec4Item(const char *cap,const char *desc,i_math::vector4df *vec);
	CXTPPropertyGridItem *InsertVec3Item(const char *cap,const char *desc,i_math::vector3df *vec);
	CXTPPropertyGridItem *InsertVec2Item(const char *cap,const char *desc,i_math::vector2df *vec);
	CXTPPropertyGridItem *InsertAbbItem(const char *cap,const char *desc,i_math::aabbox3df *aabb);
	CXTPPropertyGridItem *InsertRectItem(const char *cap,const char *desc,i_math::recti*rc,BOOL bShowSize,BOOL bNoRepair,int gran=1);
	CXTPPropertyGridItem *InsertSizeItem(const char *cap,const char *desc,i_math::vector2di*sz);
	CXTPPropertyGridItem *InsertSizeItem(const char *cap,const char *desc,i_math::vector2db*sz);
	CXTPPropertyGridItem *InsertPointItem(const char *cap,const char *desc,i_math::vector2di*sz);
	CXTPPropertyGridItem *InsertPointItem(const char *cap,const char *desc,i_math::vector2db*sz);
	CXTPPropertyGridItem *InsertRangeItem(const char *cap,const char *desc,i_math::rangef*range);
	CXTPPropertyGridItem *InsertRangeItem(const char *cap,const char *desc,i_math::rangei*range);
	CXTPPropertyGridItem *InsertProtoItem(const char *cap,const char *desc,std::string*v,IProtoLib *protolib,BOOL bLuaOnly=FALSE);
	CXTPPropertyGridItem *InsertProtoItem(const char *cap,const char *desc,unsigned __int64*v,IProtoLib *protolib,BOOL bLuaOnly=FALSE);
	CXTPPropertyGridItem *InsertSscUIDItem(const char *cap,const char *desc,DWORD*v);
	CXTPPropertyGridItem *InsertStringIDItem(const char *cap,const char *desc,StringID*v,const char *grp,DWORD iCategory);
	CXTPPropertyGridItem *InsertRecordIDItem(const char *cap,const char *desc,DWORD*v,const char *nameRecord);
	CXTPPropertyGridItem *InsertDynObjItem(const char *cap,const char *desc,void**v,std::unordered_map<std::string,CClass*>&classes,std::unordered_map<std::string,std::string>&names);
	// add by yuyang for RichGrid
	CXTPPropertyGridItem *InsertValueSetItem( const char* cap, const char* desc, ValueSet *p,i_math::rectf &rcLimit);
	// end
	CXTPPropertyGridItem *InsertMatSetItem( const char* cap, const char* desc, std::vector<i_math::matrix43f> *mats,BOOL bLS,const char *mode);
	CXTPPropertyGridItem *InsertMatSetItem( const char* cap, const char* desc, std::vector<i_math::vector3df> *vecs,BOOL bLS,const char *mode);
	CXTPPropertyGridItem *InsertMatSetItem( const char* cap, const char* desc, std::vector<i_math::spheref> *sphs,BOOL bLS,const char *mode);

	CXTPPropertyGridItem *InsertAstUIDSetItem( const char* cap, const char* desc, std::vector<DWORD> *uids);

	//edit by star
	CXTPPropertyGridItem *InsertXformItem(const char * cap,const char * desc,i_math::matrix43f * matrix);

	template <class T>
	CXTPPropertyGridItem *InsertVectorItem(const char *cap,const char *desc,std::vector<T> *vec,DWORD IDs)
	{
		RG_ADDCHILDITEM(item,CRichGrid_VectorItem<T>,cap,desc);

		item->AddButtonMask(IDs);
		item->Bind(vec);
		_AddItemBind(item,vec);

		return item;
	}

	template <class T>
	CXTPPropertyGridItem *InsertVectorElemItem(const char *cap,const char *desc,std::vector<T> *vec,DWORD iElem,DWORD IDs)
	{
		RG_ADDCHILDITEM(item,CRichGrid_VectorElemItem<T>,cap,desc);

		item->AddButtonMask(IDs);
		item->Bind(vec,iElem);
		if (vec)
			_AddItemBind(item,&(*vec)[iElem]);

		return item;
	}

	template <class T>
	CXTPPropertyGridItem *InsertIntItem(const char *cap,const char *desc,T *v,T min,T max)
	{
		RG_ADDCHILDITEM(item,CRichGrid_IntItem<T>,cap,desc);
		item->Bind(v,min,max);
		_AddItemBind(item,v);
		return item;
	}



	CXTPPropertyGridItem *InsertGVar(const char *cap,const char *desc,GVar &var,GSem &sem,IRenderSystem *pRS);
	virtual CXTPPropertyGridItem *InsertVar(void *var,const char *cap,const char *desc,GVarType vt,GSem &sem);

	void AddCaptionHide(const char *caption);
	void AddCaptionShow(const char *caption);

	//Called from the item in this grid
	virtual void OnBeginItemChange(CXTPPropertyGridItem *item)	{	}
	virtual void OnItemChange(CXTPPropertyGridItem *item)	{	}
	virtual void OnEndItemChange(CXTPPropertyGridItem *item)	{	}

	virtual void RedrawDueToItemChange()	{	}

	virtual void OnItemCommand(CXTPPropertyGridItem *item,DWORD idCmd)	{	}

protected:
	virtual BOOL _NeedHook()	{		return TRUE;	}

	RichGridHook *_GetHook();

	CXTPPropertyGridItem*_GetStackTop()
	{
		if (_stackItem.size()<=0)
			return NULL;
		return _stackItem[_stackItem.size()-1];
	}
	CXTPPropertyGridItem* _itemLast;
	std::vector<CXTPPropertyGridItem*>_stackItem;

	BOOL _IsCaptionHide(const char *caption);
	std::vector<std::set<std::string> >_stackCaptionHides;
	BOOL _IsCaptionShow(const char *caption);
	std::vector<std::set<std::string> >_stackCaptionShows;

	CXTPPropertyGridItem *_ItemFromPath(CXTPPropertyGridItems*items,const char *path);

	BOOL _bReadOnly;


	//for recording item state

	void _RecordItemState(CXTPPropertyGridItems*items,RGState &state);
	void _RestoreItemState(CXTPPropertyGridItems*items,RGState &state);

	void _SetReadOnly(CXTPPropertyGridItems *items,BOOL bReadOnly);

	void _ExpandItemR(CXTPPropertyGridItem*item);
	void _CollapseItemR(CXTPPropertyGridItem*item);

	const char *_GetPathSep()	{		return "\\";	}

	//ItemBinds
	void _AddItemBind(CXTPPropertyGridItem*item,void *ptr);
	std::unordered_map<void*,CXTPPropertyGridItem*> _binds;

	RichGridHook *_hook2;
	static RichGridHook *_hook;

};

extern CRichGrid *GetRichGrid(CXTPPropertyGridItem *item);
