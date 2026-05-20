#include "stdh.h"

#include ".\GuiActor_OverallMap.h"

#include "resource.h"

#include "ToolBase.h"

#include "GuiAgent_2DTransform.h"

#include "GuiData.h"
#include "GuiData_OverallMap.h"

#include "maptooldefines.h"

#include "FileSystem/IMapFile.h"

CGuiActor_OverallMap::CGuiActor_OverallMap(CWnd * pParent/* = NULL*/)
:CGuiPanel(IDD_ACPANEL_MAP,pParent)
{
}
CGuiActor_OverallMap::~CGuiActor_OverallMap(void)
{	
	_tools.Release();
}
//////////////////////////////////////////////////////////////////////////

class CGuiAgent_OverallMap2DTransform :public CGuiAgent_2DTransform
{
public:
	CGuiAgent_OverallMap2DTransform()
		: CGuiAgent_2DTransform()
	{
	}
	virtual void OnUpdateTransform(const i_math::pos2df & pos,const i_math::pos2df &scale)
	{
		GuiData_OverallMap * data = (GuiData_OverallMap *)FindData("overallmap");
		data->ptOff.set((float)pos.x,(float)pos.y);
		data->scale = scale.x;
		_Redraw();
	}
};
//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CGuiActor_OverallMap,CGuiPanel)
	ON_CONTROL(CBN_SELCHANGE,IDC_COMBO_TOOLS,OnToolChange)
END_MESSAGE_MAP()
void CGuiActor_OverallMap::OnToolChange()
{
	_UpdateMode();
}
BOOL CGuiActor_OverallMap::Create(CWnd * pParent)
{
	if(FALSE ==CDialog::Create(IDD_ACPANEL_MAP,pParent))
		return FALSE;
	return TRUE;
}
BOOL CGuiActor_OverallMap::OnInitDialog()
{
	if(FALSE == CGuiPanel::OnInitDialog())
		return FALSE;

	CComboBox * pCombo = (CComboBox * )GetDlgItem(IDC_COMBO_TOOLS);
	pCombo->ResetContent();
	pCombo->AddString(_T("none"));
	
	_tools.InitializeTools(TOOL_MAPCONTROL);

	return TRUE;
}
void CGuiActor_OverallMap::_UpdateTools()
{
	CComboBox * pCombo = (CComboBox * )GetDlgItem(IDC_COMBO_TOOLS);
	pCombo->ResetContent();

	for(int i = 0;i<_tools.GetNumberOfTools();i++)
	{
		CToolBase * pTool = _tools.GetTool(i);
		for(int m = 0;m<pTool->NumOfModes();m++)
		{
			CToolBase::Mode * mode = pTool->GetMode(m);
			int idx = pCombo->AddString(fromMBCS(mode->name.c_str()));
			pCombo->SetItemData(idx,DWORD_PTR(mode));
		}
	}
}
void CGuiActor_OverallMap::OnLeaveActivity()
{

}
void CGuiActor_OverallMap::OnEnterActivity()
{
	CGeView * view = FindView("overallmap");
	
	view->DiscardLevels(1);
	view->AttachActor(0,this);
	view->AddAgent(0,new CGuiAgent_OverallMap2DTransform());

	_UpdateTools();

	CComboBox * pCombo = (CComboBox * )GetDlgItem(IDC_COMBO_TOOLS);
	if(pCombo->GetCount()>0)
	{
		pCombo->SetCurSel(0);	
		_UpdateMode();
	}

	//加载简略图数据
	GuiData_System * dataSys = (GuiData_System *)FindData("system");
	if(!dataSys||!dataSys->mf)
		return;

	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	if(!dataMap)
		return;

	BYTE  * pData = NULL;
	DWORD szData = 0;
	if (dataSys->mf->LoadUnique(TRRNDRAFT_DATA,pData,szData))
	{
		//略过前面的类型
		pData+=sizeof(DWORD);
		szData-=sizeof(DWORD);
		
		Image * pImage = (Image *)(dataMap->pImage);
		SAFE_DELETE(pImage);//销毁旧的数据

		dataMap->pImage = RebuildImage(pData,szData);
	}
}

void CGuiActor_OverallMap::_UpdateMode()
{
	CComboBox * pCombo = (CComboBox * )GetDlgItem(IDC_COMBO_TOOLS);
	if(pCombo->GetCount() == 0)
		return;
	
	for(int i = 0;i<pCombo->GetCount();i++)
	{
		CToolBase::Mode * mode = (CToolBase::Mode *)pCombo->GetItemData(i);
		if(!mode) continue;
		mode->owner->EndParam(mode->mode);
	}
	CWnd * pToolPanel = GetDlgItem(IDC_STATIC_PANELTOOL);
	int idx = pCombo->GetCurSel();

	CToolBase::Mode * mode = (CToolBase::Mode *)pCombo->GetItemData(idx);
	if(mode)
		mode->owner->BeginParam(pToolPanel,mode->mode,this,0,"overallmap");
}




