#include "stdh.h"
#include "resource.h"
#include "WndBase.h"

#include "DebugPanel_Output.h"

#include "stringparser/stringparser.h"

#include "GuiData.h"
#include "GuiData_FrameProxy.h"
#include "GuiData_Debugger.h"

#include "Scintilla.h"


//////////////////////////////////////////////////////////////////////////
//COutputWnd

BEGIN_MESSAGE_MAP(COutputWnd,CScintillaWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void COutputWnd::OnLButtonDown(UINT flag,CPoint pt)
{
	static DWORD tickLast=0;

	DWORD tick=GetTickCount();
	DWORD dTick=tick-tickLast;
	tickLast=tick;
	if (dTick<::GetDoubleClickTime())
	{
		CDbgPanel_Output*parent=(CDbgPanel_Output*)GetParent();

//		int pos=PositionFromPoint(pt.x,pt.y);
		int line=GetCurrentLine();

		parent->OnDblClickLine(line);
		return;
	}

	CScintillaWnd::OnLButtonDown(flag,pt);
}

void COutputWnd::OnLButtonDblClk(UINT flag,CPoint pt)
{
}





//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CDbgPanel_Output,CGuiPanel)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(SCN_DOUBLECLICK,10000,OnDblClick)
	ON_COMMAND(ID_COMMON_DELETE,OnClear)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



CDbgPanel_Output::CDbgPanel_Output(CWnd* pParent):CGuiPanel(IDD_DEBUG_OUTPUT, pParent)
{
	_wnd=NULL;

}

BOOL CDbgPanel_Output::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_DEBUG_OUTPUT,pParent);	
}

void CDbgPanel_Output::_SetDefaultFormat()
{
// 	_wnd->SetBackground(STYLE_DEFAULT,0);
// 	_wnd->SetForeground(STYLE_DEFAULT,0xffffffff);
// 	_wnd->SetBackground(STYLE_LINENUMBER,0);
// 
// 
// 	_wnd->SetLexer(SCLEX_LUA);
// 
// 	int size=9;
// 	const char *face="微软雅黑";
// 	_wnd->SetStyle(SCE_LUA_DEFAULT, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_IDENTIFIER, 0xffffff,0x000000,size,face);	
// 
// 	_wnd->SetStyle(SCE_LUA_COMMENT, 0xffffff,0x000000,size,face);
// 	_wnd->SendMessage(SCI_STYLESETCHANGEABLE,SCE_LUA_COMMENT,false);
// 
// 	_wnd->SetStyle(SCE_LUA_COMMENTLINE, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_COMMENTDOC, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_NUMBER, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_WORD, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_STRING, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_CHARACTER, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_PREPROCESSOR, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_OPERATOR, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_IDENTIFIER, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_STRINGEOL, 0xffffff,0x000000,size,face);
// 	_wnd->SetStyle(SCE_LUA_WORD2, 0xffffff,0,size,face);
// 	_wnd->SetStyle(SCE_LUA_WORD3, 0xffffff,0,size,face);
// 
// 	_wnd->SetCaretFore(0xffffff);
// 
// 	_wnd->SetSelColor(0x0,0xafafaf);
// 
// 	_wnd->SetMarginWidth(8);
// 
// 	_wnd->SetTipFore(0);
// 
// 	_wnd->SetReadOnly(TRUE);
// 
// 	_wnd->EnablePopupMenu(FALSE);
}


BOOL CDbgPanel_Output::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	_wnd=new COutputWnd;
	if (!_wnd)
		return FALSE;

	if (!_wnd->Create(_T("Title"), WS_CHILD | WS_VISIBLE, CRect(0,0,1,1), this, 10000)) // hb - todo autogenerate id
		return FALSE;

	_SetDefaultFormat();

	_RecalcLayout();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDbgPanel_Output::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	if (_wnd)
	{
		i_math::recti rc;

		GetClientRect((LPRECT)&rc);

		SetWindowPos(_wnd,rc);
	}
}

void CDbgPanel_Output::OnDestroy()
{
	SAFE_DELETE(_wnd);

	CGuiPanel::OnDestroy();
}




void CDbgPanel_Output::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CDbgPanel_Output::Reset()
{
}



void CDbgPanel_Output::UpdateUI()
{
	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
	assert(dataDebugger);

	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;

	IDebugger *dbgr=dataDebugger->context->dbgr;

}

void CDbgPanel_Output::OnDebugOutput(DebugOutput &o)
{
	if (_wnd)
	{
		int iCurLine=_wnd->GetCurrentLine();
		int nLineOld=_wnd->GetLineCount();
		_lines.resize(nLineOld);

		GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");

		std::string s;
		if ((o.line!=-1)||(o.pathProto[0])&&(o.pathProtoNode[0]))
			FormatString(s,"%s . %s (line %d) :  %s\n",o.pathProto,o.pathProtoNode,o.line,o.content);
		else
			FormatString(s,"%s\n",o.content);


		_wnd->SetReadOnly(FALSE);
		_wnd->AppendText(s.c_str());
		_wnd->SetReadOnly(TRUE);

		int nLineNew=_wnd->GetLineCount();
		_lines.resize(nLineNew);

		for (int i=nLineOld;i<nLineNew;i++)
		{
			_lines[i].protoid=o.protoid;
			_lines[i].nodeid=o.nodeid;
			_lines[i].iLine=o.line;
		}

		if (iCurLine>=nLineOld-2)
			_wnd->GotoLine(nLineNew);
	}
}

void CDbgPanel_Output::OnDblClickLine(int line)
{
	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;

	line++;
	if (line<_lines.size())
	{
		LineInfo *p=&_lines[line];
		if (p->protoid!=ProtoID_Null)
			proxy->GotoLuaSrc(p->protoid,p->nodeid,p->iLine); 
	}
}


void CDbgPanel_Output::OnDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
// 	SCNotification*scn= (SCNotification*)pNMHDR;
// 	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;
// 
// 	DWORD line=scn->line+1;
// 	if (line<_lines.size())
// 	{
// 		LineInfo *p=&_lines[line];
// 		if (p->protoid!=ProtoID_Null)
// 		proxy->GotoLuaSrc(p->protoid,p->nodeid,p->iLine); 
// 	}
// 


	*pResult=0;
}


void CDbgPanel_Output::OnContextMenu(CWnd*, CPoint pt)
{
	CMenu menu;
	menu.CreatePopupMenu();

	ScreenToClient(&pt);

	int idx=0;

	menu.InsertMenu(idx++, MF_ENABLED | MF_STRING, ID_COMMON_DELETE, _T("Clear"));


	ClientToScreen(&pt);

	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x,pt.y,this,NULL );

}

void CDbgPanel_Output::OnClear()
{
	_wnd->SetReadOnly(FALSE);
	_wnd->SetText("");
	_wnd->SetReadOnly(TRUE);
	_lines.clear();
}

void CDbgPanel_Output::ClearOutput()
{
	OnClear();
}
