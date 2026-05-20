#include "stdh.h"

#include "AssetCtrlRenderer.h"

#include "RenderSystem/IRenderSystem.h"

#include "ShellText.h"
#include "ShellImage.h"

#include "stringparser/stringparser.h"


void CAssetCtrlRenderer::DoRender(void)
{	
	if (!_ratoms)
		return;
	if (TestDirty(DirtyFlag_Init))
	{
		InitRender();
		ClearDirty(DirtyFlag_Init);
	}

	RefreshRender();
	Render();
}



void CAssetCtrlRenderer::_UpdateAlpha(RatomID ratom)
{
	CAssetCtrl * ctrl = GetWindow();
	if(ctrl&&ratom)
	{
		BYTE alpha = ctrl->GetAlpha();
		_ratoms->UpdateAlpha(ratom,((float)alpha)/255.0f);
	}
}

void CAssetCtrlRenderer::_UpdateColor(RatomID ratom,DWORD color)
{
	_ratoms->UpdateColor(ratom,color);
	_ratoms->UpdateAlpha(ratom,((float)ColorAlpha_Alpha(color))/255.0f);
}


void CAssetCtrlRenderer::_UpdateTextRect(RatomID &ratomText,TextStyle *style,ShellPos *ptOff)
{

	if(ratomText)
	{
		CAssetCtrl * ctrl = GetWindow();
		ShellRect rcSrc = ctrl->GetScreenRect();
		ShellRect rcClip = ctrl->GetClippedScreenRect();

		if (style)
			rcSrc.inflate(-style->margin.Left(),-style->margin.Top(),-style->margin.Right(),-style->margin.Bottom());
		ShellPos pt;
		DWORD align=style?style->align:0;
		switch(align)
		{
		case 0:
			pt=rcSrc.UpperLeftCorner; break;
		case 1://center
			pt.set((rcSrc.UpperLeftCorner.x+rcSrc.LowerRightCorner.x)/2,rcSrc.UpperLeftCorner.y); break;
		case 2://right
			pt.set(rcSrc.LowerRightCorner.x,rcSrc.UpperLeftCorner.y); break;
		}

		if (ptOff)
			pt+=*ptOff;
		_ratoms->UpdateLoc(ratomText,pt);
		_ratoms->UpdateClip(ratomText,(ShellRect&)rcClip);
	}
}


void CAssetCtrlRenderer::_UpdateTextRect(RatomID &ratomText,CShellText *text,ShellPos *ptOff)
{
	_ValidateTextRatom(ratomText,text);

	_UpdateTextRect(ratomText,text->GetStyle(),ptOff);
}

void CAssetCtrlRenderer::_ValidateTextRatom(RatomID &ratomText,ITextPiece*tp)
{
	if (tp)
	{
		if(!ratomText)
			ratomText = _ratoms->Register(ShellRatom_Text);
	}
	else
	{
		_ratoms->UnRegister(ratomText);
		ratomText=0;
	}

}

void CAssetCtrlRenderer::_UpdateTextContent(RatomID &ratomText,ITextPiece*tp)
{
	_ValidateTextRatom(ratomText,tp);
	_ratoms->UpdateText(ratomText,tp);
}

void CAssetCtrlRenderer::_UpdateTextContent(RatomID &ratomText,CShellText *text)
{	
	_UpdateTextContent(ratomText,text?text->GetTextPiece():(ITextPiece*)NULL);
}


void CAssetCtrlRenderer::_ValidateTextRatom(RatomID &ratomText,CShellText *text)
{
	_ValidateTextRatom(ratomText,text?text->GetTextPiece():(ITextPiece*)NULL);
}

void CAssetCtrlRenderer::_UpdateBtnImage(RatomID &ratom,const char *path,DWORD idx)
{
	CAssetCtrl * ctrl = GetWindow();
	if (path[0]&&(idx>=0))
	{
		if(!ratom)
			ratom= _ratoms->Register(ShellRatom_Image);

		ShellRect rcClip = ctrl->GetClippedScreenRect();
		ShellRect rcScr = ctrl->GetScreenRect();
		_ratoms->UpdateLoc(ratom,(ShellRect &)rcScr);
		_ratoms->UpdateClip(ratom,(ShellRect &)rcClip);

		ShellRect rcTex;
		std::string pathTex;
		ParseShellImageStr(path, pathTex, rcTex);

		int ah = rcTex.getHeight() / 4; // average height,4ÊÇII_MAX
		rcTex.Top() += idx* ah;
		rcTex.Bottom() = rcTex.Top() + ah;

		_ratoms->UpdateImage(ratom, pathTex.c_str(), rcTex);	
	}
	else
	{
		_ratoms->UnRegister(ratom);
		ratom=0;
	}
}

void CAssetCtrlRenderer::_UpdateBlankImage(RatomID &ratom)
{
	CAssetCtrl * ctrl = GetWindow();
	if(!ratom)
		ratom= _ratoms->Register(ShellRatom_Image);

	ShellRect rcClip = ctrl->GetClippedScreenRect();
	ShellRect rcScr = ctrl->GetScreenRect();
	_ratoms->UpdateLoc(ratom,(ShellRect &)rcScr);
	_ratoms->UpdateClip(ratom,(ShellRect &)rcClip);
}


void CAssetCtrlRenderer::_UpdateComboImage(RatomID &ratom,ImageCombo *combo,ShellRect * rc)
{
	CAssetCtrl * ctrl = GetWindow();

	if (!combo)
		combo=ctrl->_comboImage;

	BOOL bContent=FALSE;
	if (combo)
	{
		if (combo->path.c_str()[0])
			bContent=TRUE;
	}

	if (bContent)
	{
		if(!ratom)
			ratom= _ratoms->Register(ShellRatom_Image);

		ShellRect rcClip = ctrl->GetClippedScreenRect();
		ShellRect rcScr = (rc)? (*rc):ctrl->GetScreenRect();
		_ratoms->UpdateLoc(ratom,(ShellRect &)rcScr);
		_ratoms->UpdateClip(ratom,(ShellRect &)rcClip);

		_ratoms->UpdateImage(ratom,combo->path.c_str());
		_ratoms->UpdateImageCombo(ratom,(ShellImageCombo)combo->combo,ShellSize(combo->szEdge.w,combo->szEdge.h));
	}
	else
	{
		_ratoms->UnRegister(ratom);
		ratom=0;
	}

}