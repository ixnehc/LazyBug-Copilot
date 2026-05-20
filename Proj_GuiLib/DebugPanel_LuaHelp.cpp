#include "stdh.h"
#include <map>
#include "resource.h"

#include "commondefines/general_stl.h"
#include "WndBase.h"
#include "WorldSystem/ILuaMachine.h"

#include "DebugPanel_LuaHelp.h"

#include "GuiData.h"
#include "GuiData_FrameProxy.h"
#include "GuiData_Debugger.h"
#include ".\debugpanel_luahelp.h"

#include "Scintilla.h"

#include "stringparser/stringparser.h"

#include "log/LogDump.h"



//////////////////////////////////////////////////////////////////////////
//CDbgPanel_LuaHelp

#define ID_HELP_CONTENT 10000
#define ID_LIB_START 100
#define ID_LIB_END 200

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CDbgPanel_LuaHelp,CGuiPanel)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(SCN_HOTSPOTCLICK,ID_HELP_CONTENT,OnHotSpotClick)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedLib)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedBack)
	ON_COMMAND_RANGE(ID_LIB_START,ID_LIB_END,OnLibCommand)
END_MESSAGE_MAP()



CDbgPanel_LuaHelp::CDbgPanel_LuaHelp(CWnd* pParent):CGuiPanel(IDD_DEBUG_LUAHELP, pParent)
{
	_wndHelp=NULL;
	_keyHelp="";

	_verHelp=0xffffffff;

	_bHotSpotClick=FALSE;
}

BOOL CDbgPanel_LuaHelp::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_DEBUG_LUAHELP,pParent);	
}

void CDbgPanel_LuaHelp::_SetDefaultFormat(CScintillaWnd *wnd)
{
// 	wnd->SetBackground(STYLE_DEFAULT,0);
// 	wnd->SetForeground(STYLE_DEFAULT,0xffffffff);
// 	wnd->SetBackground(STYLE_LINENUMBER,0);
// 
// 	wnd->SetLexer(SCLEX_LUA);
// 
// 	int size=12;
// 	const char *face="微软雅黑";
// 	wnd->SetStyle(SCE_LUA_DEFAULT, 0xffffff,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_IDENTIFIER, 0xffffff,0x000000,size,face);	
// 
// 	wnd->SetStyle(SCE_LUA_COMMENT, 0xffffff,0x000000,size,face);
// //	wnd->SendMessage(SCI_STYLESETCHANGEABLE,SCE_LUA_COMMENT,true);
// 
// 	wnd->SetStyle(SCE_LUA_COMMENTLINE, 0xafafaf,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_COMMENTDOC, 0xafafaf,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_NUMBER, 0x00ff00,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_WORD, 0x00ffff,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_STRING, 0x0000ff,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_CHARACTER, 0x0000ff,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_PREPROCESSOR, 0x00FFFF,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_OPERATOR, 0xffff00,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_IDENTIFIER, 0xffffff,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_STRINGEOL, 0x0000ff,0x000000,size,face);
// 	wnd->SetStyle(SCE_LUA_WORD2, 0x7f7fff,0,size,face);
// 	wnd->SetStyle(SCE_LUA_WORD3, 0xffc000,0,size,face);
// 	wnd->SetStyle(SCE_LUA_WORD4, 0x00c0ff,0,size,face);
// 	wnd->SetUnderline(SCE_LUA_WORD4, 1);
// 	wnd->SetHotSpot(SCE_LUA_WORD4, 1);
// 	wnd->SetUnderline(SCE_LUA_WORD3, 1);
// 	wnd->SetHotSpot(SCE_LUA_WORD3, 1);
// 
// 	wnd->SetCaretFore(0xffffff);
// 
// 	wnd->SetSelColor(0x0,0xafafaf);
// 
// 	wnd->SetMarginWidth(4);
// 
// 	wnd->SetTipFore(0);
// 
// 	wnd->SetReadOnly(TRUE);
// 
// 	wnd->EnablePopupMenu(FALSE);
}

BOOL CDbgPanel_LuaHelp::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	_wndHelp=new CHelp;
	if (!_wndHelp)
		return FALSE;

	if (!_wndHelp->Create(_T("Title"), WS_CHILD | WS_VISIBLE, CRect(0,0,1,1), this, ID_HELP_CONTENT)) 
		return FALSE;

	_SetDefaultFormat(_wndHelp);

	_RecalcLayout();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDbgPanel_LuaHelp::OnDestroy()
{
	SAFE_DELETE(_wndHelp);
	CGuiPanel::OnDestroy();
}


void CDbgPanel_LuaHelp::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;

	GetClientRect((LPRECT)&rc);

	rc.Top()+=21;
 	SetWindowPos(_wndHelp,rc);
}

void CDbgPanel_LuaHelp::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);


	_RecalcLayout();
}

IProtoLib *CDbgPanel_LuaHelp::_GetProtoLib()
{
	GuiData_Debugger*data=(GuiData_Debugger*)FindData("debugger");

	return data->context->pES->GetProtoLib();

}

ILuaMachine *CDbgPanel_LuaHelp::_GetLM()
{
	return _GetProtoLib()->GetLuaMachine();
}

void CDbgPanel_LuaHelp::_SetKeywords(CScintillaWnd *wnd,ILuaMachine *lm)
{
	std::string s,ss,sss;

	ss="";
	if (TRUE)
	{
		DWORD c=lm->GetLibCount();
		for (int i=0;i<c;i++)
		{
			sss+=lm->GetLibName(i);
			sss+=" ";
			s=ReplaceString(lm->GetLibFuncs(i),","," ");
			ss=ss+s+" ";
		}
		s="print setmetatable tonumber getmetatable tostring";
		ss=ss+s+" ";
	}
	wnd->SetKeywords(0,sss.c_str());

	wnd->SetKeywords(2,ss.c_str());

	ss=ReplaceString(lm->GetTypes(),","," ");
	wnd->SetKeywords(3,ss.c_str());
	
}


void CDbgPanel_LuaHelp::Reset()
{
	ILuaMachine *lm=_GetLM();

	_SetKeywords(_wndHelp,lm);


	_CollectKeys();

}

void CDbgPanel_LuaHelp::_CollectKeys()
{
	_funcnames.clear();
	_funclib.clear();
	_typenames.clear();

	ILuaMachine *lm=_GetLM();

	std::vector<std::string>pieces;
	for (int i=0;i<lm->GetLibCount();i++)
	{
		const char *funcs=lm->GetLibFuncs(i);

		SplitStringBy(",",std::string(funcs),&pieces);

		for (int j=0;j<pieces.size();j++)
		{
			_funcnames.push_back(pieces[j]);
			_funclib.push_back(i);
		}
	}

	const char *types=lm->GetTypes();
	SplitStringBy(",",std::string(types),&pieces);

	for (int j=0;j<pieces.size();j++)
		_typenames.push_back(pieces[j]);
}


void CDbgPanel_LuaHelp::UpdateUI()
{
	_UpdateHotSpotClick();
	_UpdateHelp();
}

void CDbgPanel_LuaHelp::_UpdateHotSpotClick()
{
	if (_bHotSpotClick)
	{
		if (GetCapture()==_wndHelp)
			return;
		int pos=_wndHelp->GetCurrentPosition();
		if (pos==_posClick)
		{
			GuiData_Debugger*data=(GuiData_Debugger*)FindData("debugger");
			if (data)
				data->SetHelpKey(_strClick.c_str());
		}

		_bHotSpotClick=FALSE;
	}
}



void CDbgPanel_LuaHelp::OnHotSpotClick(NMHDR*p, LRESULT*result)
{
	SCNotification *scn=(SCNotification *)p;
	GuiData_Debugger*data=(GuiData_Debugger*)FindData("debugger");
	if (!data)
		return;

	const char *sel=_wndHelp->GetCurWord(scn->position,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

	_bHotSpotClick=TRUE;
	_strClick=sel;
	_posClick=scn->position;

	*result=0;
}

void CDbgPanel_LuaHelp::_UpdateHelp()
{
	GuiData_Debugger*data=(GuiData_Debugger*)FindData("debugger");
	if (_verHelp==data->verHelp)
		return;//没有变化

	if (_keyHelp==data->keyHelp)
		return;//没有变化

	ILuaMachine *lm=_GetLM();

	_verHelp=data->verHelp;

	std::string s;
	const char *help="";

	//try functions
	int idx;
	if (!help[0])
	{
		VEC_FIND(_funcnames,data->keyHelp,idx);
		if (idx!=-1)
		{
			help=lm->GetLibFuncHelp(data->keyHelp.c_str(),_funclib[idx]);
			if (!help[0])
				help="~目前没有内容~";
		}
	}
	//try types
	if (!help[0])
	{
		VEC_FIND(_typenames,data->keyHelp,idx);
		if (idx!=-1)
		{
			help=lm->GetTypeHelp(data->keyHelp.c_str());
			if (!help[0])
				help="~目前没有内容~";
		}
	}
	if (!help[0])
	{
		//try libs
		std::string ss;
		ILuaMachine *lm=_GetLM();
		for (int i=0;i<lm->GetLibCount();i++)
		{
			if (data->keyHelp==lm->GetLibName(i))
			{
				FormatString(s,"< %s >有以下函数:\n",lm->GetLibName(i));

				std::map<std::string,std::string> sort;

				for (int k=0;k<_funcnames.size();k++)
				{
					if (_funclib[k]==i)
					{
						ss=_funcnames[k];
						StringUpper(ss);
						sort[ss]=_funcnames[k];
					}
				}

				std::map<std::string,std::string>::iterator it;
				for (it=sort.begin();it!=sort.end();it++)
				{
					s+="  ";
					s+=(*it).second.c_str();
					s+="(..)\n";
				}

				help=s.c_str();

				break;
			}
		}
	}
	if (!help[0])
	{//try asset package classes
		IProtoLib *protolib=_GetProtoLib();
		if (protolib)
			help=protolib->GetClassHelp(help);
	}

	if (!help[0])
		return;

	_wndHelp->SetReadOnly(FALSE);
	_wndHelp->SetText(help,FALSE);
	_wndHelp->Refresh();
	_wndHelp->SetReadOnly(TRUE);

	if (_keyHelp!="")
		_keyhistory.push_back(_keyHelp);
	_keyHelp=data->keyHelp;

}


void CDbgPanel_LuaHelp::OnBnClickedLib()
{
	CMenu menu;
	menu.CreatePopupMenu();

	ILuaMachine *lm=_GetLM();

	for (int i=0;i<lm->GetLibCount();i++)
		menu.AppendMenu(MF_ENABLED | MF_STRING, ID_LIB_START + i, fromMBCS(lm->GetLibName(i)));


	CPoint pt;
	GetCursorPos(&pt);

	XTFuncContextMenu(&menu,TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,this,IDR_TOOLBAREXT);
	//		menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x,point.y,this,NULL );

}

void CDbgPanel_LuaHelp::OnLibCommand(UINT idCmd)
{
	GuiData_Debugger*data=(GuiData_Debugger*)FindData("debugger");
	if (!data)
		return;

	int iLib=idCmd-ID_LIB_START;

	ILuaMachine *lm=_GetLM();
	data->SetHelpKey(lm->GetLibName(iLib));

	_UpdateHelp();

	_wndHelp->SetFocus();

}

void CDbgPanel_LuaHelp::OnBnClickedBack()
{
	GuiData_Debugger*data=(GuiData_Debugger*)FindData("debugger");
	if (data)
	{
		if (_keyhistory.size()>0)	
		{
			data->SetHelpKey(_keyhistory[_keyhistory.size()-1].c_str());
			_keyHelp="";
			_keyhistory.pop_back();
		}
	}
}
