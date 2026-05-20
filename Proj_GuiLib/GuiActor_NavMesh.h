
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/INavMesh.h"

#include "PinControls.h"


class GuiLib_Api CGuiPanel_NavMesh :public CGuiPanel
{
public:
	CGuiPanel_NavMesh(CWnd * pParent = NULL);
	virtual ~CGuiPanel_NavMesh();
	virtual BOOL Create(CWnd *pParent);
	virtual const char *GetName(){ return "navmesh";}
	virtual const char *_GetModMgrName()	{		return "world";	}

	virtual void UpdateUI();
	virtual void OnEnterActivity();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	void Reset(){}
	INavMeshEditor * GetEditor();

	DECLARE_MESSAGE_MAP() 

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnParamsChange(NMHDR * pNotifyStruct,LRESULT *pResult);
	afx_msg void OnOpChange(unsigned int idCtrl);
	afx_msg void OnExport();

private:
	void _LoadParam();
	//Cell
	CPinboardEdit	_edCellSize;
	CPinSpinner		_spCellSize;

	CPinboardEdit	_edCellHeight;
	CPinSpinner		_spCellHeight;
	
	//Agent
	CPinboardEdit	_edAgentHeight;
	CPinSpinner		_spAgentHeight;

	CPinboardEdit	_edAgentRadius;
	CPinSpinner		_spAgentRadius;

	CPinboardEdit	_edAgentMaxClimb;
	CPinSpinner		_spAgentMaxClimb;

	CPinboardEdit	_edAgentMaxSlope;
	CPinSpinner		_spAgentMaxSlope;
	
	//Region
	CPinboardEdit	_edMinRegion;
	CPinSpinner		_spMinRegion;
	
	CPinboardEdit	_edMergeRegion;
	CPinSpinner		_spMergeRegion;
	
	//Trianglization
	CPinboardEdit	_edMaxEdgeLen;
	CPinSpinner		_spMaxEdgeLen;

	CPinboardEdit	_edMaxEdgeError;
	CPinSpinner		_spMaxEdgeError;
	
	CPinboardEdit	_edVerPerPoly;
	CPinSpinner		_spVerPerPoly;

	CPinboardEdit	_edSampleDist;
	CPinSpinner		_spSampleDist;

	CPinboardEdit	_edMaxSampleError;
	CPinSpinner		_spMaxSampleError;
};




