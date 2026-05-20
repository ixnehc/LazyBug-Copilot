#pragma once
#include "ResEditPanel.h"
#include "ResAnchor.h"
#include "DummyBarList.h"
#include "AxisArrow.h"
#include "SKeletonModel.h"
#include "DummyPropertiesWnd.h"
#include "GuiAgent_MatrixEdit.h"
#include "GuiAgent_DummiesSelect.h"

struct Reps_Dummies :public ResEditPanelState
{
	Reps_Dummies()
	{ 
		editMode =0 ;
		arrowMesh = NULL;
		dummyIdx = -1;
	}
	virtual void Copy(ResEditPanelState &src)
	{
		this->dummyIdx=((Reps_Dummies *)(&src))->dummyIdx;
		ResEditPanelState::Copy(src);
	}
	int dummyIdx;
	CAxisArrow * arrowMesh;
	DWORD editMode;
};

class GuiLib_Api  CDummiesEditPanel : public CResEditPanel
{
public:
	CDummiesEditPanel(void);
	~CDummiesEditPanel(void);

	//Override function
	virtual UINT GetIDD(){return IDD_DUMMIESPANEL;};

	//3d 
	virtual void Init3d();
	virtual void Clear3d();	

	//anchor releted
	virtual void OnResDataChange(ResData *dataNew);
	
	//draw
	virtual void Draw(IRenderPort *rp)	;//should be overidden to draw something in subclass

	virtual void OnSelect();


	//serialize
	//virtual ResEditPanelState *NewState();
	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual BOOL StateToFile(ResEditPanelState *state);//Save all the need-saving part of the state(generally it's the resdata the panel is editing)
	virtual ResEditPanelState *_NewState();
	//UI relative
public:	
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL OnInitDialog();

public:
	virtual void BeginMatrixEdit(i_math::matrix43f *matrix);
	virtual void OnMatrixEdit(i_math::matrix43f *matrix);
	virtual void EndMatrixEdit(i_math::matrix43f *matrix);
protected:
	void _BindStateMatrix();

private:
	CDummyBarList   _dummiesListWnd;
	CDummyPropertiesWnd  _dummyPropertiesWnd;
	CAxisArrow  _meshDummy;
	ILight  * _light;
	CSkeletonModel  _skeletonModel;
	IRenderPort  *_rp;
	CGuiAgent_MatrixEdit * _pAgentMatrixEdit;
	CDummiesSelectAgent * _pAgentDummiesSelect;
};

