#include "stdh.h"

#include "ShellText.h"

#include "RenderSystem/IFont.h"
#include "RenderSystem/IRenderSystem.h"


#include "WorldSystem/assetcore/assetctrl/AssetCtrl.h"
#include "WorldSystem/assetcore/assetctrl/AssetCtrlRenderer.h"



void CShellText::Clear()
{
	SAFE_RELEASE(_tp);
	SAFE_RELEASE(_style);
}

void CShellText::Init(CAssetCtrl *owner,ShellTextArg &arg)
{
	_owner=owner;
	_tp=_owner->_ss->pRS->GetFontMgr()->MakeText("");
	SAFE_REPLACE(_style,arg.style);
	if (arg.style)
	{
		_tp->SetAlign(arg.style->align);
		_tp->SetSelColor(FORCE_TYPE(DWORD,arg.style->colSelFg),FORCE_TYPE(DWORD,arg.style->colSelBg));
		_tp->SetBgAlpha((DWORD)(arg.style->colSelBg.w));
		_tp->SetDefaultFormat(StrLib_GetStr(arg.style->fmt));
	}
	if (arg.idStr!=StringID_Invalid)
		_tp->SetFormatText(StrLib_GetStr(arg.idStr));
	else
		_tp->SetFormatText(arg.str.c_str());
	if (arg.bSingleLine)
		_tp->SetSingleLine(arg.bSingleLine);
	if (arg.bShowPassword)
		_tp->SetShowPassword('*');
	if ((arg.bShowLink)&&(arg.style))
		_tp->ShowLink(FORCE_TYPE(DWORD,arg.style->colLink),TRUE);
}

void CShellText::_NotifyTextChange()
{
	CAssetCtrlRenderer *rdr=_owner->GetWindowRenderer();
	if (rdr)
		rdr->OnTextChanged();
}

const char *CShellText::GetStr()
{
	if (_tp)
		return _tp->GetTextMB();
	return "";
}


void CShellText::Set(StringID id)	
{		
	if (id==StringID_Invalid)
		Set("");
	else
		Set(StrLib_GetStr(id));
}

void CShellText::Set(const char *s)
{
	if (_tp)
	{
		_tp->SetFormatText(s);
		_NotifyTextChange();
	}
}

void CShellText::SetWidthLimit(DWORD wLimit)
{
	if (!_tp)
		return;

	if (_style)
	{
		if (wLimit>(_style->margin.Left()+_style->margin.Right()))
			wLimit-=(_style->margin.Left()+_style->margin.Right());
		else
			wLimit=1;
	}
	_tp->SetWidthLimit(wLimit);
	_NotifyTextChange();
}

BOOL CShellText::IsSingleLine()
{	
	return _tp?_tp->IsSingleLine():TRUE;	
}

ShellSize CShellText::GetActualSize()
{
	if (!_tp)
		return ShellSize(0,0);
	ShellSize sz=_tp->GetActualSize();
	if (_style)
	{
		sz.w+=_style->margin.Left()+_style->margin.Right();
		sz.h+=_style->margin.Top()+_style->margin.Bottom();
	}
	return sz;
}
