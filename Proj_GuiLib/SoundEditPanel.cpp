/********************************************************************
	created:	2012/02/23
	file base:	SoundEditPanel
	author:		cxi
	
	purpose:	Sound Edit Panel
*********************************************************************/
#include "stdh.h"

#include "RenderSystem/ISound.h"

#include ".\SoundEditPanel.h"

#include "resdata/ResData.h"
#include "GuiEditor_res.h"

#include "WndBase.h"

#include "RenderSystem/IRenderSystem.h"

#include <assert.h>

	
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////////
//CSoundEditPanel
CSoundEditPanel::CSoundEditPanel()
{
	_anchor.SetResType(Res_Sound);
	_anchor.SetLabel("SoundAnchor");
}


void CSoundEditPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOUNDANCHOR, _anchor);
}

BEGIN_MESSAGE_MAP(CSoundEditPanel, CResEditPanel)
	ON_COMMAND(IDC_PLAY,OnPlay)
END_MESSAGE_MAP()



// CSoundEditPanel 消息处理程序

BOOL CSoundEditPanel::OnInitDialog()
{
	if (FALSE==CResEditPanel::OnInitDialog())
		return FALSE;


	return TRUE;
}

void CSoundEditPanel::Init3d()
{
}


void CSoundEditPanel::Clear3d()
{
}

void CSoundEditPanel::OnResDataChange(ResData *data)
{
	_stateToMod->SetData(data);

	OnPlay();

}

ResEditPanelState *CSoundEditPanel::_NewState()
{
	return (ResEditPanelState *)new Reps_Sound;
}


//Update the controls in the panel to reflect the state
BOOL CSoundEditPanel::StateToControl(ResEditPanelState *state0)
{
	if (!CResEditPanel::StateToControl(state0))
		return FALSE;

	return TRUE;
}

//Save all the need-saving part of the state(generally it's the resdata the panel is editing)
BOOL CSoundEditPanel::StateToFile(ResEditPanelState *state)
{
	return FALSE;//永远不会存储
}


void CSoundEditPanel::Draw(IRenderPort *rp)
{
}

void CSoundEditPanel::OnSelect()
{
	ClearAgent();
}


void CSoundEditPanel::OnPlay()
{
	std::string path;
	path=_anchor.GetRelativePath();
	if (path=="")
		return;
	ISound *sound=(ISound *)g_ssGuiLib.pRS->GetSoundMgr()->ObtainRes(path.c_str());
	ISoundPlay *play=sound->Play2D(FALSE);
	SAFE_RELEASE(play);
	SAFE_RELEASE(sound);
}



