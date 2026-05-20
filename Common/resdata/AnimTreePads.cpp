/********************************************************************
	created:	14:4:2010   12:19
	file path:	d:\IxEngine\Common\resdata
	author:		chenxi
	
	purpose:	所有的Anim Tree Pad
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "AnimTreePads.h"

#include "stringparser/stringparser.h"

#include <assert.h> 

BYTE CAnimTreePads::_CalcClassCode()
{
	BYTE *p=NULL;
	p+=((BYTE*)(Class_Ptr(CAtpRoot))-p);
	p+=((BYTE*)Class_Ptr(CAtpSequence)-p);
	p+=((BYTE*)Class_Ptr(CAtpSequenceWH)-p);
	p+=((BYTE*)Class_Ptr(CAtpSequenceSD)-p);
	p+=((BYTE*)Class_Ptr(CAtpSequenceST)-p);
	p+=((BYTE*)Class_Ptr(CAtpBlend)-p);
	p+=((BYTE*)Class_Ptr(CAtpBlendX)-p);
	p+=((BYTE*)Class_Ptr(CAtpSpeedBlend)-p);
	p+=((BYTE*)Class_Ptr(CAtpSwitch)-p);
	p+=((BYTE*)Class_Ptr(CAtpPartialBlend)-p);
	p+=((BYTE*)Class_Ptr(CAtpPartialSwitch)-p);
	p+=((BYTE*)Class_Ptr(CAtpSwitch_ActSub)-p);
	p+=((BYTE*)Class_Ptr(CAtpSwitch_AvtrLoco)-p);
	p+=((BYTE*)Class_Ptr(CAtpRotOnSpotBlend)-p);
	p+=((BYTE*)Class_Ptr(CAtpMoveStartRotBlend)-p);
	p+=((BYTE*)Class_Ptr(CAtpMoveRotBlend)-p);
	p+=((BYTE*)Class_Ptr(CAtpBoneCtrlChainStretch)-p);
	p+=((BYTE*)Class_Ptr(CAtpBoneCtrlEel)-p);
	p+=((BYTE*)Class_Ptr(CAtpIKCtrl_Chain)-p);
	p+=((BYTE*)Class_Ptr(CAtpIKCtrl_Simple)-p);
	p+=((BYTE*)Class_Ptr(CAtpIKCtrl_Custom)-p);
	//XXXXX:more AnimTreePad

	return FORCE_TYPE(BYTE,p);

}




#define BEGIN_STUB() 										\
switch(idx)															\
{

#define STUB_IN(__idx,__name)							\
		case __idx:													\
			return PadStub(__name,PadStub_In,1)

#define STUB_OUT(__idx,__name)						\
		case __idx:													\
			return PadStub(__name,PadStub_Out,0)


		
#define END_STUB()											\
		default:assert(FALSE);								\
}																			\
return PadStub();


//32 is big enough
const char *GetDefaultChildName(int idx)
{
	static char *names[30]=
	{
		"Child0",	"Child1",	"Child2",	"Child3",	"Child4",	"Child5",	"Child6",	"Child7",	"Child8",	"Child9",
		"Child10",	"Child11",	"Child12",	"Child13",	"Child14",	"Child15",	"Child16",	"Child17",	"Child18",	"Child19",
		"Child20",	"Child21",	"Child22",	"Child23",	"Child24",	"Child25",	"Child26",	"Child27",	"Child28",	"Child29",
	};
	if (idx<ARRAY_SIZE(names))
		return names[idx];
	return "";
}

//////////////////////////////////////////////////////////////////////////
//NameCases
DWORD NameCases::GetStubCount()
{
	return nms.size()+1;//nms.size()+out
}
PadStub NameCases::GetStub(DWORD idx)
{
	if (idx<nms.size())
	{
		const char *name=StrLib_GetStr(nms[idx]);
		if (!name[0])
			name=GetDefaultChildName(idx);
		return PadStub(name,PadStub_In,1);
	}
	else
		return PadStub("输出",PadStub_Out	,0);
}

DWORD NameCases::GetChildCount()
{
	return GetStubCount()-1;
}

StringID *NameCases::GetDbgNames(DWORD &count)
{
	static std::vector<StringID>buf;
	buf.clear();
	buf.push_back(StringID_Invalid);
	VEC_APPEND(buf,nms);
	count=buf.size();
	return buf.data();
}



//////////////////////////////////////////////////////////////////////////
//
PadStub CAnimTreePad::GetStub(DWORD idx)
{
	PadStub t;
	if (idx<_childs.size())
	{
		t.bSingleLink=1;
		t.name=_childs[idx].name.c_str();
		if (!t.name[0])
			t.name=GetDefaultChildName(idx);
		t.type=PadStub_In;
	}
	else
	{
		t.bSingleLink=0;
		t.name="输出";
		t.type=PadStub_Out;
	}

	return t;
}

BOOL CAnimTreePad::SetChildName(DWORD idx,const char *name,CLinkPads *owner)
{
	if (_childs[idx].name==name)
		return TRUE;
	if (name[0])//非空
	{
		int idx;
		VEC_FIND_BY_ELEMENT(_childs,name,name,idx);
		if (idx!=-1)
			return FALSE;//不是唯一的
	}
	owner->_ToPersist();
	_childs[idx].name=name;
	_ClearIdxCache();
	owner->_FromPersist();
	return TRUE;
}

BOOL CAnimTreePad::AddChild(CLinkPads *owner)	
{		
	owner->_ToPersist();
	 _childs.push_back(AtpChild());
	_ClearIdxCache();
	owner->_FromPersist();
	return TRUE;
}

void CAnimTreePad::RemoveChild(DWORD idx,CLinkPads *owner)
{		
	owner->_ToPersist();
	_childs.erase(_childs.begin()+idx);	
	_ClearIdxCache();
	owner->_FromPersist();
}


//XXXXX:more AnimTreePad
//////////////////////////////////////////////////////////////////////////
//CAtpRoot

IMPLEMENT_CLASS(CAtpRoot);


DWORD CAtpRoot::GetStubCount()
{
	return 3;
}

PadStub CAtpRoot::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"输入");
		STUB_IN(1,"输入(BoneCtrl)");
		STUB_IN(2,"输入(IKCtrl)");
	END_STUB()
}


//////////////////////////////////////////////////////////////////////////
//CAtpSequence
IMPLEMENT_CLASS(CAtpSequence);


DWORD CAtpSequence::GetStubCount()
{
	return 1;
}

PadStub CAtpSequence::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_OUT(0,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpSequenceWH
IMPLEMENT_CLASS(CAtpSequenceWH);


//////////////////////////////////////////////////////////////////////////
//CAtpSequenceSD
IMPLEMENT_CLASS(CAtpSequenceSD);

//////////////////////////////////////////////////////////////////////////
//CAtpSequenceTSD
IMPLEMENT_CLASS(CAtpSequenceTSD);

//////////////////////////////////////////////////////////////////////////
//CAtpSequenceST
IMPLEMENT_CLASS(CAtpSequenceST);


//////////////////////////////////////////////////////////////////////////
//CAtpBlend
IMPLEMENT_CLASS(CAtpBlend);


DWORD CAtpBlend::GetStubCount()
{
	return 3;
}

PadStub CAtpBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"Child0");
		STUB_IN(1,"Child1");
		STUB_OUT(2,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpBlendX
IMPLEMENT_CLASS(CAtpBlendX);

DWORD CAtpBlendX::GetStubCount()
{
	return _nChilds+1;
}
PadStub CAtpBlendX::GetStub(DWORD idx)
{
	static const char *names[]=	{	"Child0","Child1","Child2","Child3","Child4","Child5","Child6","Child7","Child8","Child9"	};
	if (idx<_nChilds)
	{
		if (idx<ARRAY_SIZE(names))
			return PadStub(names[idx],PadStub_In,1);
		else
			return PadStub("<overflow>",PadStub_In,1);
	}
	else
		return PadStub("输出",PadStub_Out	,0);
}

DWORD CAtpBlendX::GetChildCount()
{
	return _nChilds;
}


//////////////////////////////////////////////////////////////////////////
//CAtpSpeedBlend
IMPLEMENT_CLASS(CAtpSpeedBlend);


DWORD CAtpSpeedBlend::GetStubCount()
{
	return 5;
}

PadStub CAtpSpeedBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"Lvl0");
		STUB_IN(1,"Lvl1");
		STUB_IN(2,"Lvl2");
		STUB_IN(3,"Lvl3");
		STUB_OUT(4,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpDirBlend
IMPLEMENT_CLASS(CAtpShiftBlend);

DWORD CAtpShiftBlend::GetStubCount()
{
	return 5;
}

PadStub CAtpShiftBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
	STUB_IN(0,"向前");
	STUB_IN(1,"向右");
	STUB_IN(2,"向后");
	STUB_IN(3,"向左");
	STUB_OUT(4,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpMoveRotBlend
IMPLEMENT_CLASS(CAtpMoveRotBlend);

DWORD CAtpMoveRotBlend::GetStubCount()
{
	return 4;
}

PadStub CAtpMoveRotBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"左转");
		STUB_IN(1,"前方");
		STUB_IN(2,"右转");
		STUB_OUT(3,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpMoveStartRotBlend
IMPLEMENT_CLASS(CAtpMoveStartRotBlend);

DWORD CAtpMoveStartRotBlend::GetStubCount()
{
	return 4;
}

PadStub CAtpMoveStartRotBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"左转");
		STUB_IN(1,"前方");
		STUB_IN(2,"右转");
		STUB_OUT(3,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpRotOnSpotBlend
IMPLEMENT_CLASS(CAtpRotOnSpotBlend);

DWORD CAtpRotOnSpotBlend::GetStubCount()
{
	return 7;
}

PadStub CAtpRotOnSpotBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"左转180度");
		STUB_IN(1,"左转X度");
		STUB_IN(2,"左转0度");
		STUB_IN(3,"右转0度");
		STUB_IN(4,"右转X度");
		STUB_IN(5,"右转180度");
		STUB_OUT(6,"输出");
	END_STUB()
}


//////////////////////////////////////////////////////////////////////////
//CAtpSwitch

IMPLEMENT_CLASS(CAtpSwitch);
IMPLEMENT_CLASS(CAtpSwitch_TunerString);
IMPLEMENT_CLASS(CAtpSwitch_Move);
IMPLEMENT_CLASS(CAtpSwitch_Fly);
IMPLEMENT_CLASS(CAtpSwitch_Jump);
IMPLEMENT_CLASS(CAtpSwitch_Turn);
IMPLEMENT_CLASS(CAtpSwitch_Slide);
IMPLEMENT_CLASS(CAtpSwitch_Posture);
IMPLEMENT_CLASS(CAtpSwitch_Act);
IMPLEMENT_CLASS(CAtpSwitch_Auto);

IMPLEMENT_CLASS(CAtpSwitch_ActSub);


DWORD CAtpSwitch_ActSub::GetStubCount()
{
	return _count+1;
}
PadStub CAtpSwitch_ActSub::GetStub(DWORD idx)
{
	static const char *names[]=	{	"0","1","2","3","4","5","6","7","8","9"	};
	if (idx<_count)
	{
		if (idx<ARRAY_SIZE(names))
			return PadStub(names[idx],PadStub_In,1);
		else
			return PadStub("<overflow>",PadStub_In,1);
	}
	else
		return PadStub("输出",PadStub_Out	,0);
}

DWORD CAtpSwitch_ActSub::GetChildCount()
{
	return _count;
}

IMPLEMENT_CLASS(CAtpSwitch_PostureTrans);
PadStub CAtpSwitch_PostureTrans::GetStub(DWORD idx)
{		
	if (idx<_cases.GetStubCount()-1)
		return _cases.GetStub(idx);

	idx-=_cases.GetStubCount()-1;
	if (idx<_transes.size())
	{
		PostureTrans *trans=&_transes[idx];
		return PadStub(trans->nm.c_str(),PadStub_In,1);
	}
	return PadStub("输出",PadStub_Out	,0);
}


IMPLEMENT_CLASS(CAtpSwitch_AutoX);

DWORD CAtpSwitch_AutoX::GetStubCount()
{
	return _nLevels+1;
}
PadStub CAtpSwitch_AutoX::GetStub(DWORD idx)
{
	static const char *names[]=	{	"Lvl0","Lvl1","Lvl2","Lvl3","Lvl4","Lvl5","Lvl6","Lvl7","Lvl8","Lvl9"	};
	if (idx<_nLevels)
	{
		if (idx<ARRAY_SIZE(names))
			return PadStub(names[idx],PadStub_In,1);
		else
			return PadStub("<overflow>",PadStub_In,1);
	}
	else
		return PadStub("输出",PadStub_Out	,0);
}

DWORD CAtpSwitch_AutoX::GetChildCount()
{
	return _nLevels;
}


//////////////////////////////////////////////////////////////////////////
//CAtpSwitch_AvtrMove
IMPLEMENT_CLASS(CAtpSwitch_AvtrLoco);

DWORD CAtpSwitch_AvtrLoco::GetStubCount()
{
	return AvtrLocoChild_Count+1;
}

PadStub CAtpSwitch_AvtrLoco::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(AvtrLocoChild_NotMove,"未移动");
		STUB_IN(AvtrLocoChild_StartFw,"起步(前方)");
		STUB_IN(AvtrLocoChild_StartL,"起步(左转)");
		STUB_IN(AvtrLocoChild_StartR,"起步(右转)");
		STUB_IN(AvtrLocoChild_MoveL,"移动(左转起步)");
		STUB_IN(AvtrLocoChild_MoveR,"移动(右转起步)");
		STUB_IN(AvtrLocoChild_StopLPass,"停步(LeftPass)");
		STUB_IN(AvtrLocoChild_StopL,"停步(Left)");
		STUB_IN(AvtrLocoChild_StopRPass,"停步(RightPass)");
		STUB_IN(AvtrLocoChild_StopR,"停步(Right)");
		STUB_IN(AvtrLocoChild_RotateOnSpot,"原地转身");
		STUB_OUT(AvtrLocoChild_Count,"输出");
	END_STUB()
}

StringID CAtpSwitch_AvtrLoco::GetDbgName_NotMove()
{
	static StringID nm=StringID_Invalid;
	if (nm==StringID_Invalid)
		nm=StrLib_Get()->FindStr(0,"NotMove","AvtrMoveType");
	return nm;
}

StringID CAtpSwitch_AvtrLoco::GetDbgName_Start()
{
	static StringID nm=StringID_Invalid;
	if (nm==StringID_Invalid)
		nm=StrLib_Get()->FindStr(0,"Start","AvtrMoveType");
	return nm;
}

StringID CAtpSwitch_AvtrLoco::GetDbgName_Move()
{
	static StringID nm=StringID_Invalid;
	if (nm==StringID_Invalid)
		nm=StrLib_Get()->FindStr(0,"Move","AvtrMoveType");
	return nm;
}

StringID CAtpSwitch_AvtrLoco::GetDbgName_Stop()
{
	static StringID nm=StringID_Invalid;
	if (nm==StringID_Invalid)
		nm=StrLib_Get()->FindStr(0,"Stop","AvtrMoveType");
	return nm;
}

StringID CAtpSwitch_AvtrLoco::GetDbgName_RotOnSpot()
{
	static StringID nm=StringID_Invalid;
	if (nm==StringID_Invalid)
		nm=StrLib_Get()->FindStr(0,"RotOnSpot","AvtrMoveType");
	return nm;
}

StringID *CAtpSwitch_AvtrLoco::GetDbgNames(DWORD &count)
{
	static StringID nms[5];
	static BOOL bInit=FALSE;
	if (!bInit)
	{
		bInit=TRUE;
		nms[0]=GetDbgName_NotMove();
		nms[1]=GetDbgName_Start();
		nms[2]=GetDbgName_Move();
		nms[3]=GetDbgName_Stop();
		nms[4]=GetDbgName_RotOnSpot();
	}

	count=ARRAY_SIZE(nms);
	return nms;
}



//////////////////////////////////////////////////////////////////////////
//CAtpPartialBlend

IMPLEMENT_CLASS(CAtpPartialBlend);


DWORD CAtpPartialBlend::GetStubCount()
{
	return 3;
}

PadStub CAtpPartialBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"Full");
		STUB_IN(1,"Partial");
		STUB_OUT(2,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpPartialSwitch

IMPLEMENT_CLASS(CAtpPartialSwitch);


DWORD CAtpPartialSwitch::GetStubCount()
{
	return 6;
}

PadStub CAtpPartialSwitch::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"Full");
		STUB_IN(1,"Partial0");
		STUB_IN(2,"Partial1");
		STUB_IN(3,"Partial2");
		STUB_IN(4,"Partial3");
		STUB_OUT(5,"输出");
	END_STUB()
}

IMPLEMENT_CLASS(CAtpPartialSwitch_Auto);

//////////////////////////////////////////////////////////////////////////
//CAtpBoneCtrl
IMPLEMENT_CLASS(CAtpBoneCtrl);
DWORD CAtpBoneCtrl::GetStubCount()
{
	return 2;
}

PadStub CAtpBoneCtrl::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"Child0");
		STUB_OUT(1,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpBoneCtrlBlend
IMPLEMENT_CLASS(CAtpBoneCtrlBlend);


DWORD CAtpBoneCtrlBlend::GetStubCount()
{
	return 3;
}

PadStub CAtpBoneCtrlBlend::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"Child0");
		STUB_IN(1,"Child1");
		STUB_OUT(2,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpBoneCtrlMerge

IMPLEMENT_CLASS(CAtpBoneCtrlMerge);

DWORD CAtpBoneCtrlMerge::GetStubCount()
{
	return _nChild+1;
}
PadStub CAtpBoneCtrlMerge::GetStub(DWORD idx)
{
	static const char *names[]=	{	"Child0","Child1","Child2","Child3","Child4","Child5","Child6","Child7","Child8","Child9"	};
	if (idx<_nChild)
	{
		if (idx<ARRAY_SIZE(names))
			return PadStub(names[idx],PadStub_In,1);
		else
			return PadStub("<overflow>",PadStub_In,1);
	}
	else
		return PadStub("输出",PadStub_Out	,0);
}

DWORD CAtpBoneCtrlMerge::GetChildCount()
{
	return _nChild;
}

//////////////////////////////////////////////////////////////////////////
//CAtpBoneCtrlChainStretch
IMPLEMENT_CLASS(CAtpBoneCtrlChainStretch);
DWORD CAtpBoneCtrlChainStretch::GetStubCount()
{
	return 2;
}

PadStub CAtpBoneCtrlChainStretch::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_IN(0,"Child0");
	STUB_OUT(1,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpBoneCtrlEel
IMPLEMENT_CLASS(CAtpBoneCtrlEel);
DWORD CAtpBoneCtrlEel::GetStubCount()
{
	return 1;
}

PadStub CAtpBoneCtrlEel::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_OUT(0,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpIKCtrl_Base

DWORD CAtpIKCtrl_Base::GetStubCount()
{
	return 1;
}

PadStub CAtpIKCtrl_Base::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_OUT(0,"输出");
	END_STUB()
}



//////////////////////////////////////////////////////////////////////////
//CAtpIKCtrl_Chain
IMPLEMENT_CLASS(CAtpIKCtrl_Chain);

//////////////////////////////////////////////////////////////////////////
//CAtpIKCtrl_Simple
IMPLEMENT_CLASS(CAtpIKCtrl_Simple);

//////////////////////////////////////////////////////////////////////////
//CAtpIKCtrl_Custom
IMPLEMENT_CLASS(CAtpIKCtrl_Custom);

//////////////////////////////////////////////////////////////////////////
//CAtpFloatST
IMPLEMENT_CLASS(CAtpFloatST);

DWORD CAtpFloatST::GetStubCount()
{
	return 1;
}

PadStub CAtpFloatST::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_OUT(0,"输出");
	END_STUB()
}

//////////////////////////////////////////////////////////////////////////
//CAtpPath
IMPLEMENT_CLASS(CAtpPath);


DWORD CAtpPath::GetStubCount()
{
	return 1;
}

PadStub CAtpPath::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_OUT(0,"输出");
	END_STUB()
}

void CAtpPath::CollectRefs(std::vector<std::string>&buf)
{
	if (!_path.empty())
	{
		UNIQUE_VEC_ADD(buf,_path);
	}
}


//////////////////////////////////////////////////////////////////////////
//CAtpPathST
IMPLEMENT_CLASS(CAtpPathST);

//////////////////////////////////////////////////////////////////////////
//CAtpCombo_Act
IMPLEMENT_CLASS(CAtpCombo_Act);
DWORD CAtpCombo_Act::GetStubCount()
{
	return 1;
}

PadStub CAtpCombo_Act::GetStub(DWORD idx)
{
	BEGIN_STUB()
		STUB_OUT(0,"输出");
	END_STUB()
}

StringID *CAtpCombo_Act::GetDbgNames(DWORD &count)
{
	static std::vector<StringID>nms;
	nms.clear();
	nms.push_back(StringID_Invalid);
	for (int i=0;i<_acts.size();i++)
		nms.push_back(_acts[i].act);
	count=nms.size();
	return nms.data();
		
}
