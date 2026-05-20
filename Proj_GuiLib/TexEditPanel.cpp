/********************************************************************
	created:	11/27/2009
	filename: 	TexEditPanel
	author:		chenxi
	
	purpose:	texture edit panel
*********************************************************************/
#include "stdh.h"

#include"resdata/ResDataDefines.h"

#include "RenderSystem/ITexture.h"
#include "RenderSystem/IRenderPort.h"

#include ".\TexEditPanel.h"


#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"

#include "WndBase.h"



#include <assert.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////////
//CTexEditPanel
CTexEditPanel::CTexEditPanel()
{
	_anchor.SetResType(Res_Texture);
	_anchor.SetLabel("TextureAnchor");
}


void CTexEditPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXANCHOR, _anchor);
}

BEGIN_MESSAGE_MAP(CTexEditPanel, CResEditPanel)
END_MESSAGE_MAP()



// CTexEditPanel 消息处理程序

BOOL CTexEditPanel::OnInitDialog()
{
	if (FALSE==CResEditPanel::OnInitDialog())
		return FALSE;


	return TRUE;
}

void CTexEditPanel::Init3d()
{
}


void CTexEditPanel::Clear3d()
{
}

void CTexEditPanel::OnResDataChange(ResData *data)
{
	_stateToMod->SetData(data);

}

ResEditPanelState *CTexEditPanel::_NewState()
{
	return (ResEditPanelState *)new Reps_Tex;
}


//Update the controls in the panel to reflect the state
BOOL CTexEditPanel::StateToControl(ResEditPanelState *state0)
{
	if (!CResEditPanel::StateToControl(state0))
		return FALSE;

	return TRUE;
}

//Save all the need-saving part of the state(generally it's the resdata the panel is editing)
BOOL CTexEditPanel::StateToFile(ResEditPanelState *state)
{
	return FALSE;//永远不会存储
}


void CTexEditPanel::Draw(IRenderPort *rp)
{
	ITexture *tex;
	std::string path;
	path=_anchor.GetRelativePath();
	if (path=="")
		return;
	tex=(ITexture*)g_ssGuiLib.pRS->GetTexMgr()->ObtainRes(path.c_str());

	DrawTextureArg arg;
	arg.SetDest(8,20);
	rp->DrawTexture(tex,arg);
	SAFE_RELEASE(tex);
}

void CTexEditPanel::OnSelect()
{
	ClearAgent();
}




