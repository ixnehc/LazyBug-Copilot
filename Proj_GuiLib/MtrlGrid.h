
#pragma once
#include "GuiLib.h"

// #include "resource.h"
#include <vector>

#include "resdata/MtrlData.h"

#include "ResEditCtrl.h"

#include "RichGrid.h"

struct MtrlDataItemInfo
{
	FeatureParamA *fp;
	KeyType kt;
	GSem sem;
};


class CResEditPanel;
struct ResEditPanelState;

struct Reps_Mtrl;
struct MtrlData;

class CMtrlGrid:public CRichGrid,public CResEditCtrl
{
public:
	CMtrlGrid()
	{
	}

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	Reps_Mtrl *GetState()	{		return (Reps_Mtrl *)_state;	}

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);
	
	virtual void EnableCtrl(BOOL bActive=TRUE);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnGridNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnAnimNone();
	afx_msg void OnAnimValueSet();
	afx_msg void OnAnimRes();


	virtual MtrlData *_GetMtrlData()	{		return (MtrlData*)_GetResData();	}
	virtual RGState &_GetRGState();
	virtual const char *_GetGridName()	{		return "Material Data";	}
	virtual BOOL _NeedUniFeature()	{		return TRUE;}
	virtual BOOL _NeedMtrlExt()	{		return TRUE;}
	virtual BOOL _NeedRefPath()	{		return TRUE;}//是否要将参数里的绝对路径转为相对路径

	void _InsertLod(MtrlData::Lod&lod,DWORD iLod);

	void _Repair();



	MtrlDataItemInfo*_FindItemData(CXTPPropertyGridItem *item);

	void _SwitchAnimType(int at);

	std::string _pathOwner;
	std::map<CXTPPropertyGridItem*,MtrlDataItemInfo> _itemdata;

};
