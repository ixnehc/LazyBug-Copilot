#pragma  once
#include "RichGrid.h"
#include "ResEditCtrl.h"
#include "resdata/DummiesData.h"

class CDummyPropertiesWnd :public CRichGrid ,public CResEditCtrl
{
public:
	CDummyPropertiesWnd();
	~CDummyPropertiesWnd();

	 void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	 virtual void EnableCtrl(BOOL bActive=TRUE);

	 virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	 virtual void OnItemChange(CXTPPropertyGridItem *item);
	 virtual void OnEndItemChange(CXTPPropertyGridItem *item);

	 virtual void OnItemCommand(CXTPPropertyGridItem *item,DWORD idCmd);

	 DECLARE_MESSAGE_MAP()
protected:
	RGState state;
	CXTPPropertyGridItem * itemType;
	DWORD btType;
};


