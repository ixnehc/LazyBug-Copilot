/********************************************************************
	created:	2008/05/30
	created:	30:5:2008   15:50
	filename: 	d:\IxEngine\Proj_WorldEditor2\ProtoLuaSource.cpp
	file path:	d:\IxEngine\Proj_WorldEditor2
	file base:	ProtoLuaSource
	file ext:	cpp
	author:		cxi
	
	purpose:	lua script编辑窗口
*********************************************************************/

#include "stdh.h"
#include "commondefines/general_stl.h"
#include "CppView.h"

#include "ChildView.h"

#include "stringparser/stringparser.h"
#include "gds/GStub.h"


#pragma warning(disable:4312)

#define MARKER_BREAKPOINT 2
#define MARKER_EXE_INDICATOR 3
#define MARKER_EXE_INDICATOR2 4

//Lua 类型定义,copied from lua.h
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8


/////////////////////////////////////////////////////////////////////////////
// CCppView

IMPLEMENT_DYNCREATE(CCppView, CView)

CCppView::CCppView()
{
	_bLoading=FALSE;
	_bStdAC=FALSE;
}

CCppView::~CCppView()
{
}


BEGIN_MESSAGE_MAP(CCppView, CScintillaView)
	//{{AFX_MSG_MAP(CCppView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_SAVE,OnFileSave)
	ON_COMMAND(ID_EDIT_UNDO,OnUndo)
	ON_COMMAND(ID_EDIT_REDO,OnRedo)
	ON_COMMAND(ID_EDIT_COPY,OnCopy)
	ON_COMMAND(ID_EDIT_PASTE,OnPaste)
	ON_COMMAND(ID_EDIT_CUT,OnCut)
	ON_WM_CONTEXTMENU()

	// Standard printing commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCppView drawing

void CCppView::_SetDefaultFormat()
{
	CScintillaView::_SetDefaultFormat();
	const char keywords[] =

		"and break do else elseif "
		"end false for function if "
		"in local nil not or "
		"repeat return then true until "
		"while "
		"_me _owner _ge _gt _st _pub "
		"_base _str _math _sys _gam "
		;

	_wnd->SetWordChar("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

	_wnd->SetKeywords(0,keywords);

	_keywords=keywords;

	_wnd->DefineBookmark(MARKER_BREAKPOINT,RGB(255,0,0),RGB(255,0,0),0);//SC_MARK_CIRCLE
	_wnd->DefineBookmark(MARKER_EXE_INDICATOR,RGB(0,0,0),RGB(255,255,0),4);//SC_MARK_ARROW
	_wnd->DefineBookmark(MARKER_EXE_INDICATOR2,RGB(0,0,0),RGB(0,255,0),2);//SC_MARK_ARROW

	_wnd->SetDwellTime(600);

	_wnd->EnablePopupMenu(FALSE);

	_wnd->AC_SetMaxHeight(10);
}


void CCppView::OnDraw(CDC*)
{
}


void CCppView::OnFileSave()
{
// 	((CChildView *)(GetParent()->GetParent()))->SaveLuaSrc(this);
}

void CCppView::OnUndo()
{
	CScintillaView::OnEditUndo();
}
void CCppView::OnRedo()
{
	CScintillaView::OnEditRedo();
}

void CCppView::OnCopy()
{
	CScintillaView::OnEditCopy();
}

void CCppView::OnPaste()
{
	CScintillaView::OnEditPaste();
}
void CCppView::OnCut()
{
	CScintillaView::OnEditCut();
}

void CCppView::_OnModified()
{
	if (!_bLoading)
	{
		OnFileSave();
	}
}


void CCppView::SetExeIndicator(int iLine,BOOL bOnStack)
{
	int iOld=_wnd->FindNextBookmark(1<<MARKER_EXE_INDICATOR,0,FALSE);
	if (iOld>=0)
	{
		if (iOld!=iLine)
			_wnd->DeleteBookmark(iOld,MARKER_EXE_INDICATOR);
	}
	iOld=_wnd->FindNextBookmark(1<<MARKER_EXE_INDICATOR2,0,FALSE);
	if (iOld>=0)
	{
		if (iOld!=iLine)
			_wnd->DeleteBookmark(iOld,MARKER_EXE_INDICATOR2);
	}

	if (iLine>=0)
		_wnd->AddBookmark(iLine,bOnStack?MARKER_EXE_INDICATOR2:MARKER_EXE_INDICATOR);
}

void SortStringVec(std::vector<std::string> &buf)
{
	std::map<std::string,int>sort;
	for (int i=0;i<buf.size();i++)
		sort[buf[i]]=1;
	buf.clear();
	std::map<std::string,int>::iterator it;
	for (it=sort.begin();it!=sort.end();it++)
		buf.push_back((*it).first);
}

void CCppView::_LoadStdAssist()
{
// 	if (_bStdAC)
// 		return;
// 	IProtoLib *lib=_GetProtoLib();
// 	if (!lib)
// 		return;
// 
// 	ILuaMachine *lm=lib->GetLuaMachine();
// 	DWORD c=lm->GetLibCount();
// 	for (int i=0;i<c;i++)
// 	{
// 		std::string name=lm->GetLibName(i);
// 		std::string funclist=lm->GetLibFuncs(i);
// 		std::vector<std::string>buf;
// 		SplitStringBy(",",funclist,&buf);
// 		SortStringVec(buf);
// 		LinkStringBy(" ",funclist,&buf);
// 		_ac[name]=funclist;
// 	}
// 	_bStdAC=TRUE;
}

//例如GetSubPart("abc","abc.def")的返回值是"def",
//例如GetSubPart("abc","abc.def.ghi")的返回值是"",
static const char *GetSubPart(const char *name,const char *nameSub)
{
	char *p=(char*)name;
	char *q=(char*)nameSub;
	while((*p)==(*q)&&(*p)&&(*q))
	{
		p++;
		q++;
	}
	if (((*p)==0)&&((*q)=='.'))
	{
		q++;
		const char *ret=q;
		while(*q)
		{
			if ((*q)=='.')
				return "";
			q++;
		}
		return ret;
	}
	return "";
}


void CCppView::UpdateSyntaxAssist()
{
	_LoadStdAssist();

// 	if (proto->GetVer()!=_ver)
// 	{
// 		IProtoNode *node=proto->GetNode(_id);
// 		
// 		_UpdateSyntaxAssist(_editor->_dataProto.lib,proto,node);
// 
// 		_ver=proto->GetVer();
// 	}
}

// void CCppView::_UpdateSyntaxAssist(IProtoLib *lib,IProto *proto,IProtoNode *node)
// {
// 	CNodeTree *ntree=proto->GetNodeTree()->GetTree();
// 	if (!ntree)
// 		return;
// 
// 	_acPNs.clear();
// 	_nodenames="";
// 	if (TRUE)
// 	{
// 		DWORD c;
// 		NodeHandle *handles=ntree->Enum(NodeHandle_Root,NodeType_None,c);
// 		std::string name;
// 		for (int i=0;i<c;i++)
// 		{
// 			name=ntree->GetName(handles[i]);
// 			if (name.c_str()[0]==PROTO_AUTONAME_PREFIX)
// 				continue;//自动命名的node
// 			_nodenames+=name;
// 			_nodenames+=" ";
// 			IProtoNode *p=proto->FindNode(ntree->GetPath(handles[i]));
// 			if (p)
// 			{
// 				if (p->GetID()==_id)
// 					_AddNodeAC("_me",p);
// 				else
// 					_AddNodeAC(name.c_str(),p);
// 			}
// 		}
// 	}
// 
// 	_wnd->SetKeywords(1,_nodenames.c_str());
// 
// 	_funcnames="";
// 	if (TRUE)
// 	{
// 		ILuaMachine *lm=lib->GetLuaMachine();
// 
// 		std::string s;
// 		DWORD c=lm->GetLibCount();
// 		for (int i=0;i<c;i++)
// 		{
// 			s=ReplaceString(lm->GetLibFuncs(i),","," ");
// 			_funcnames=_funcnames+s+" ";
// 		}
// 		s="print setmetatable tonumber getmetatable tostring";
// 		_funcnames=_funcnames+s+" ";
// 	}
// 	_wnd->SetKeywords(2,_funcnames.c_str());
// 
// 	_wnd->Refresh();
// }


const char *CCppView::GetCurWord(int pos)
{
	return _wnd->GetCurWord(pos,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
}


void CCppView::_OnUpdateUI()
{
}

void CCppView::UpdateUI()
{
}



inline BOOL IsWordChar(int c)
{
	if (c<=0)
		return FALSE;
	return isalnum(c)||(c=='_');
}

void CCppView::_CullVar(int pos0,std::string &s)
{
	int pos=pos0;
	int c;

	//往后找
	while(IsWordChar(c=_wnd->GetCharAt(pos++)))
		s+=c;

	if (s=="")
		return;

	//往前找Word|dot|blank
	pos=pos0-1;
	while(1)
	{
		c=_wnd->GetCharAt(pos--);

		if (IsWordChar(c)||(c==' ')||(c=='.'))
			s=std::string("")+(char)c+s;
		else
			break;
	}

	std::vector<std::string>buf,buf2;
	std::list<std::string>buf3;

	//用dot分割
	SplitStringBy(".",s,&buf);

	std::string ss;
	//从后到前遍历
	for (int i=buf.size()-1;i>=0;i--)
	{
		ss=buf[i];
		//分割后的每个部分切去首尾
		RemoveTailBlank(ss);
		RemoveHeadBlank(ss);
		if (ss=="")
			break;

		//用空格分开
		SplitStringBy(" ",ss,&buf2);

		//分割后的最后一块添加到结果字串的前部
		buf3.push_front(buf2[buf2.size()-1]);

		//如果分割出多块,则不用继续了
		if (buf2.size()>1)
			break;
	}

	LinkStringBy(".",s,&buf3);

}

void CCppView::_CullPrevVar(int pos,std::string &s)
{
	int c;
	while(1)
	{
		c=_wnd->GetCharAt(pos--);

		if (IsWordChar(c))
			s=std::string("")+(char)c+s;
		else
			break;
	}
	
}

void CCppView::_OnDwellStart(int pos)
{
	std::string s;
	_CullVar(pos,s);

}

void CCppView::_OnDwellEnd(int pos)
{
	_wnd->HideTip();
}


DWORD CCppView::_ParseLineHead(int iLine,std::string &head)
{
	int pos=_wnd->PositionFromLine(iLine);
	std::string pad;
	DWORD c=0;
	while(_wnd->GetCharAt(pos)=='\t')
	{
		pos++;
		c++;
	}
	head=GetCurWord(pos);
	return c;
}

//注意:此函数尚未测试过
BOOL CCppView::_ParseLineTail(int iLine,std::string &tail)
{
	tail="";
	int pos=_wnd->EndPosFromLine(iLine);
	int posStart=_wnd->PositionFromLine(iLine);
	while(isspace(_wnd->GetCharAt(pos)))
	{
		pos--;
		if (pos<posStart)
			break;
	}
	tail=GetCurWord(pos);
	return TRUE;
}


static BOOL IsBlockStart(const char *str)
{
	std::string s=str;
	if ((s=="if")||(s=="function")||(s=="for")||(s=="else")||(s=="while"))
		return TRUE;
	return FALSE;
}

static BOOL IsBlockEnd(const char *str)
{
	if (str==std::string("end")||str==std::string("else"))
		return TRUE;
	return FALSE;
}

void CCppView::_RepairExpect(int iLine,const char *expect)
{
	BOOL bFound=FALSE;

	if (TRUE)
	{
		int pos=_wnd->PositionFromLine(iLine-1);
		int flagsOld=_wnd->GetSearchflags();
		_wnd->SetSearchflags(2|4);//SCFIND_WHOLEWORD|SCFIND_MATCHCASE);
		int posFound=_wnd->SearchForward((char*)expect,pos,FALSE);
		if (posFound!=-1)
		{
			int iLineFound=_wnd->LineFromPosition(posFound);
			if ((iLineFound<=iLine)&&(iLineFound>=iLine-1))//只能在当前行和上一行中找
				bFound=TRUE;
		}
		_wnd->SetSearchflags(flagsOld);
	}

	if (!bFound)
	{
		std::string s=" ";
		s+=expect;
		_wnd->InsertText(s.c_str(),_wnd->EndPosFromLine(iLine-1));
	}

}


void CCppView::_OnCharAdded(int ch)
{
// 	//如果是回车的话,我们要做一些额外的工作
// 	if (ch==13)
// 	{
// 		int iLine=_wnd->GetCurrentLine();
// 		if (iLine>0)
// 		{
// 			DWORD nTab;
// 			int pos=_wnd->PositionFromLine(iLine-1);
// 			std::string head;
// 			nTab=_ParseLineHead(iLine-1,head);
// 
// 			std::string pad;
// 			for (int i=0;i<nTab;i++)
// 				pad+='\t';
// 			if (IsBlockStart(head.c_str()))
// 				pad+='\t';
// 
// 			_wnd->AddText(pad.c_str());
// 
// 			//如果上一行是一个block的开始的话,我们看看是否需要添加这个block的结尾部分
// 			if (IsBlockStart(head.c_str()))
// 			{
// 				//尝试找block的结尾
// 				int nLine=_wnd->GetLineCount();
// 				BOOL bFound=FALSE;
// 				std::string head2;
// 				DWORD nTab2;
// 				for (int i=iLine;i<nLine;i++)
// 				{
// 					nTab2=_ParseLineHead(i,head2);
// 					if (nTab==nTab2)
// 					{
// 						if (IsBlockEnd(head2.c_str()))
// 						{
// 							bFound=TRUE;
// 							break;
// 						}
// 						if (IsBlockStart(head2.c_str()))
// 							break;
// 					}
// 				}
// 				if (!bFound)
// 				{
// 					std::string tail;
// 					tail="\r\n";
// 					for (int i=0;i<nTab;i++)
// 						tail+='\t';
// 					tail+="end";
// 					_wnd->InsertText(tail.c_str());
// 				}
// 			}
// 
// 			if (head=="if")
// 				_RepairExpect(iLine,"then");
// 			if (head=="while")
// 				_RepairExpect(iLine,"do");
// 
// 		}
// 
// 		
// 	}
// 
// 	if (IsWordChar(ch)||(ch=='.'))
// 	{
// 		if ((!_wnd->AC_IsActive())||(ch=='.'))
// 		{
// 			int pos=_wnd->GetCurrentPosition();
// 
// 			pos--;
// 
// 			//先往前找到一个最近的'.'
// 			std::string part;
// 			BOOL bFoundDot=FALSE;
// 			BOOL bFoundSpace=FALSE;
// 			int cWord=0;
// 			while(pos>=0)
// 			{
// 				char c=_wnd->GetCharAt(pos);
// 				if (c=='.')
// 				{
// 					bFoundDot=TRUE;
// 					break;
// 				}
// 				if (IsWordChar(c))
// 				{
// 					if (!bFoundSpace)
// 					{
// 						part=std::string()+c+part;
// 						cWord++;
// 					}
// 				}
// 				else
// 				{
// 					if (c!=' ')//注意:如果是空格的话,我们还要往前找有没有dot,所以不能跳出循环
// 					{
// 						bFoundSpace=TRUE;
// 						break;
// 					}
// 				}
// 				pos--;
// 			}
// 
// 			std::string word;
// 			if (bFoundDot)
// 			{
// 				if(pos>0)
// 					_CullVar(pos-1,word);
// 			}
// 
// 			if(word=="")
// 				bFoundDot=FALSE;
// 
// 			std::string list;
// 			if (bFoundDot)
// 			{
// 				std::map<std::string,std::string>*acs[]=
// 				{
// 					&_ac,&_acGT,&_acPNs,
// 				};
// 				for (int i=0;i<ARRAY_SIZE(acs);i++)
// 				{
// 					std::map<std::string,std::string>::iterator it=acs[i]->find(word);
// 					if (it!=acs[i]->end())
// 						list=(*it).second;
// 					if (!list.empty())
// 						break;
// 				}
// 			}
// 			else
// 				list=_keywords+_nodenames+"print setmetatable tonumber getmetatable tostring ";
// 			if (!list.empty())
// 			{
// 				RemoveTailBlank(list);
// 				std::vector<std::string>pieces;
// 				SplitStringBy(" ",list,&pieces);
// 
// 				SortStringVec(pieces);
// 				LinkStringBy(" ",list,&pieces);
// 
// 				//看看我们是不是真的有匹配
// 				BOOL bSomeMatch=FALSE;
// 				for (int i=0;i<pieces.size();i++)
// 				{
// 					if (part.length()>pieces[i].length())
// 						continue;
// 					if (memcmp(part.c_str(),pieces[i].c_str(),part.length())!=0)
// 						continue;
// 					bSomeMatch=TRUE;
// 					break;
// 				}
// 
// 				if (bSomeMatch)
// 					_wnd->AC_Show(cWord,list.c_str());
// 			}
// 		}
// 	}
}



void CCppView::OnContextMenu(CWnd*, CPoint pt)
{
	CMenu menu;
	menu.CreatePopupMenu();

	ScreenToClient(&pt);

	if (!_wnd->GetSelectedText()[0])
		_wnd->SendMessage(WM_LBUTTONDOWN,0,MAKELPARAM(pt.x,pt.y));


	int idx=0;

	menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_EDIT_COPY,"Copy");
	menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_EDIT_CUT,"Cut");
	menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_EDIT_PASTE,"Paste");

//	menu.InsertMenu(idx++,MF_ENABLED|MF_SEPARATOR);



	ClientToScreen(&pt);

	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x,pt.y,this,NULL );
	
}

