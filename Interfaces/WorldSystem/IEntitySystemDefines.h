/********************************************************************
	created:	2008/4/9   14:24
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	entity system defines
*********************************************************************/

#pragma once


#include "math/imath_all.h"

#include "editor/ctrlop.h"
#include "fastdelegate/FastDelegate.h"

#include "IDestroyCache.h"


//////////////////////////////////////////////////////////////////////////
//Proto
typedef unsigned __int64 ProtoID;
#define ProtoID_Null 0

typedef unsigned __int64 ProtoNodeID;
#define ProtoNodeID_Null 0



#define PROTO_SUFFIX "prt"

#define PROTO_PATH_SYS "_sys"
#define PROTO_PATH_EDITOR "_sys\\editor"
#define PROTO_PATH_GE "_sys\\ge"
#define PROTO_PATH_ERR "_sys\\err"
#define PROTO_PATH_PLAY "_sys\\play\\play"

#define PROTO_AUTONAME_PREFIX '$'

enum ProtoNodeType
{
	PN_Asset,
	PN_Entity,
	PN_LuaObj,
};

struct ProtoStubInfo
{
	ProtoStubInfo()
	{
		idInner=ProtoNodeID_Null;
		name=nameInner="";
	}

	const char*name;
	ProtoNodeID idInner;
	const char *nameInner;
	i_math::pos2di pos;//graph pos
};

struct PNConnect
{
	PNConnect()
	{
		id[0]=id[1]=ProtoNodeID_Null;
		name[0]=name[1]="";
	}
	ProtoNodeID id[2];
	const char *name[2];
};

typedef WORD ProtoCommentID;
#define ProtoCommentID_Invalid (0)

struct ProtoComment
{
	const char *str;
	i_math::recti rc;
};

typedef BYTE PNDeferGrp;
#define PNDeferGrp_None 0
#define PNDeferGrp_Dyn  0xfe	//dynamic proto node的组
#define PNDeferGrp_Invalid 0xff
#define PNDeferGrp_Max 9


struct StubArg
{
	StubArg()
	{
		type=0;//GStub_Property
		bConnectable=TRUE;
	}
	std::string name;
	std::string nameGVT;//数据类型,如果为空,表示为一个"百搭"类型
	int type;//GStubType value
	int sem;//GSemCode value
	std::string constaint;//GSem 的constraint string
	std::string desc;
	BOOL bConnectable;
};

struct ProtoBreakPoint
{
	ProtoBreakPoint()
	{
		line=-1;
		flag=None;
	}
	enum Flag
	{
		None=0,
		Disable=1,
	};
	int line;
	Flag flag;
};

struct LuaTblInfo
{
	struct Entry
	{
		std::string name;
		int tp;//a LUA_TXXXX value
		ProtoNodeID nodeid;
	};
	std::vector<Entry> entries;
};



//////////////////////////////////////////////////////////////////////////
//Entity Bit(bits used in the EntitySystem internally)
typedef WORD EntityBit;
#define EntityBit_Alive 1
#define EntityBit_Resided 2//the entity has been added into CEntityMap
//#define EntityBit_Global 4//the entity has been added into CEntityGlobal
#define EntityBit_Dying 8//the entity is going to be destroyed
#define EntityBit_Enum 0x8000


//////////////////////////////////////////////////////////////////////////
//EntityMap
typedef unsigned __int64 EntityAddress;//representing all the information to 
																	//locate an entity in the entity map
#define EntityAddress_Null 0


//////////////////////////////////////////////////////////////////////////
//LuaMachine
#define BEGIN_FUNC_HELP(name) static const char *HelpFunc_##name(){	return ""

#define END_FUNC_HELP() ;}

#define ADD_LIB_FUNC(funcname,realname)						\
	AddLibFunc(funcname,realname);									\
	AddLibFuncHelp(funcname,HelpFunc_##realname());

#define ADD_LIB_FUNC2(funcname,realname)						\
	lm->AddLibFunc(funcname,realname);									\
	lm->AddLibFuncHelp(funcname,HelpFunc_##realname());
	

//////////////////////////////////////////////////////////////////////////
//system


class IAsset;
class IProtoNode;
struct EntityCreateArg
{
	EntityCreateArg()
	{
		parent=NULL;
		protonode=NULL;
		bLab=0;
		bEditMode=0;
		bDesc=0;
		bAllowEditHelper=0;
	}
	IProtoNode *protonode;
	IAsset *parent;
	DWORD bLab:1;
	DWORD bEditMode:1;
	DWORD bDesc:1;
	DWORD bAllowEditHelper:1;
};


struct EntitySystemInput
{
	EntitySystemInput()
	{
		dt=0.0f;
		szRP.set(1024,768);
	}
	void AddOp(CtrlOp &op)	{		ops.push_back(op);	}
	void ClearOp()	{		ops.clear();	}
	void SetDt(DWORD dMS)	{		dt=((float)dMS)/1000.0f;	}
	void SetDt(float d)	{		dt=d;	}
	void SetCursorPos(int x,int y)	{		ptCursor.set(x,y);	}
	void SetRPSize(i_math::size2di &sz)	{		szRP=sz;	}
	std::vector<CtrlOp>ops;//界面的操作
	i_math::pos2di ptCursor;//鼠标在窗口里的位置
	float dt;//时间增量,in second
	i_math::size2di szRP;//渲染屏幕的大小
};

class IAnimNode;
struct EntitySystemCenter
{
	EntitySystemCenter()
	{
		anCenter=NULL;
	}
	std::string pathMap;
	i_math::vector3df center;
	IAnimNode *anCenter;
};

typedef fastdelegate::FastDelegate0<BOOL> ESProgressCallBack;





class IProtoLib;
class IEntityMap;
class IEntityGlobal;
class IDebugger;
class IEntitySystem;
class IWorldSystem;
class IAssetSystem;
class IAssetPackage;
class IMapFile;
class IGameSystem;
class IClient;
class IEntity;

class CConfig;

struct EntitySystemState
{
	EntitySystemState()
	{
		Zero();
	}
	void Zero()
	{
		pES=NULL;

		pWS=NULL;
		pAS=NULL;
		pLabAS=NULL;
		pAPs=NULL;
		mf=NULL;

		pGS=NULL;
		pClient=NULL;

		lib=NULL;
		mp=NULL;
		global=NULL;
		debugger=NULL;

		cfg=NULL;
	}
	BOOL IsValid()	{		return pES!=NULL;	}
	IProtoLib *lib;
	IEntityMap *mp;
	IEntityGlobal *global;

	IDebugger *debugger;

	IEntitySystem *pES;	//if this is NULL, the EntitySystemState is not valid

	IWorldSystem *pWS;
	IAssetSystem *pAS;
	IAssetSystem *pLabAS;
	IAssetPackage *pAPs;
	IMapFile *mf;

	IGameSystem *pGS;
	IClient *pClient;

	DestroyCache<IEntity,EntityBit_Dying> dc;

	EntitySystemCenter center;

	ESProgressCallBack cbProgress;

	CConfig *cfg;
};
