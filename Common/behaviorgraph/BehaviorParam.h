#pragma once

#include "BehaviorDefines.h"
#include "../strlib/strlibdefines.h"
#include "../records/recordsdefine.h"
#include "../gds/gobj.h"

template <typename T,int T_tp=0>
struct BehaviorParamRef
{
	BehaviorParamRef()
	{
		bRef=FALSE;
		nmRef=StringID_Invalid;
	}
	BehaviorParamRef(T vDef)
	{
		bRef=FALSE;
		v=vDef;
		nmRef=StringID_Invalid;
	}
	BOOL IsEmpty()
	{
		if (!bRef)
			return v==0;
		return nmRef==StringID_Invalid;
	}
	BOOL bRef;
	StringID nmRef;//变量名或常量名
	T v;
};

struct BPR_CustomObjBase
{
	BPR_CustomObjBase()
	{
		GConstructor();
		Zero(TRUE);
	}


	BEGIN_GOBJ( BPR_CustomObjBase, 1 );
	END_GOBJ();

	void Zero(BOOL bIntuitive)
	{
		bRef=FALSE;
		nmRef=StringID_Invalid;
	}

	void Clear()
	{
		Zero(FALSE);
	}

	void Copy(BPR_CustomObjBase *src)
	{
		bRef=src->bRef;
		nmRef=src->nmRef;
		GetEmbedGObj()->Copy(src->GetEmbedGObj());
	}

	void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=1;//version
		dp.Data_WriteSimple(bRef);
		dp.Data_WriteSimple(nmRef);
		GetEmbedGObj()->Save(dp,TRUE);
	}

	BOOL Load(CDataPacket &dp)
	{
		BYTE ver=dp.Data_NextByte();//version
		dp.Data_ReadSimple(bRef);
		dp.Data_ReadSimple(nmRef);
		if (ver>0)
			GetEmbedGObj()->Load(dp,TRUE,NULL);
		else
		{
			if (!bRef)
				GetEmbedGObj()->Load(dp,TRUE,NULL);
		}

		return TRUE;
	}

	BOOL IsValid()
	{
		if (!bRef)
			return TRUE;
		if (nmRef!=StringID_Invalid)
			return TRUE;
		return FALSE;
	}

	virtual GObjBase *GetEmbedGObj()=0;
	virtual void *GetEmbedObjOwner()=0;
	BOOL bRef;
	StringID nmRef;
};

template <typename T>
struct BPR_CustomObj:public BPR_CustomObjBase
{
	virtual GObjBase *GetEmbedGObj()
	{
		return obj.GetGObj();
	}
	virtual void *GetEmbedObjOwner()
	{
		return &obj;
	}

	void SaveDelta(CDataPacket &dp,BPR_CustomObj<T> *pRef)
	{
		Save(dp);
	}

	void LoadDelta(CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		Load(dp);
		if (ptrsDelta)
			ptrsDelta->push_back(this);
	}


	T obj;
};

struct FillDescAssist;

//////////////////////////////////////////////////////////////////////////
//BPR_Bool
typedef BehaviorParamRef<BOOL> BPR_Bool;
#define GELEM_BPR_BOOL(varname,vDef,showname,desc)						\
	GELEM_VAR_INIT(BPR_Bool,varname,BPR_Bool(vDef));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_Bool"),desc);
extern const char *GetBPRDesc(BPR_Bool &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_Bool(BOOL v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_Int
typedef BehaviorParamRef<int,1> BPR_Int;
#define GELEM_BPR_INT(varname,vDef,showname,desc)						\
	GELEM_VAR_INIT(BPR_Int,varname,BPR_Int(vDef));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_Int"),desc);
extern const char *GetBPRDesc(BPR_Int &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_Int(int v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_Float
typedef BehaviorParamRef<float> BPR_Float;
#define GELEM_BPR_FLOAT(varname,vDef,showname,desc)						\
	GELEM_VAR_INIT(BPR_Float,varname,BPR_Float(vDef));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_Float"),desc);
extern const char *GetBPRDesc(BPR_Float &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_Float(float v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_SkillID
typedef BehaviorParamRef<RecordID,1> BPR_SkillID;
#define GELEM_BPR_SKILLID(varname,showname,desc)						\
	GELEM_VAR_INIT(BPR_SkillID,varname,BPR_SkillID(RecordID_Invalid));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_SkillID"),desc);
extern const char *GetBPRDesc(BPR_SkillID &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_SkillID(RecordID v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_BuffID
typedef BehaviorParamRef<RecordID,2> BPR_BuffID;
#define GELEM_BPR_BUFFID(varname,showname,desc)						\
	GELEM_VAR_INIT(BPR_BuffID,varname,BPR_BuffID(RecordID_Invalid));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_BuffID"),desc);
extern const char *GetBPRDesc(BPR_BuffID &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_BuffID(RecordID v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_StringID
typedef BehaviorParamRef<DWORD,6> BPR_StringID;
#define GELEM_BPR_STRINGID(varname,constraint,showname,desc)						\
	GELEM_VAR_INIT(BPR_StringID,varname,BPR_StringID(0));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_StringID:"constraint),desc);
extern const char *GetBPRDesc(BPR_StringID &bpr,FillDescAssist *assist);
const char *GetBVRDesc_StringID(StringID v,StringID nmRef,FillDescAssist *assist);
#define BVRDESC_StringID(nm) GetBVRDesc_StringID(BVR_ARG(nm),assist)


//////////////////////////////////////////////////////////////////////////
//BPR_ItemID
typedef BehaviorParamRef<RecordID,3> BPR_ItemID;
#define GELEM_BPR_ITEMID(varname,showname,desc)						\
	GELEM_VAR_INIT(BPR_ItemID,varname,BPR_ItemID(RecordID_Invalid));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_ItemID"),desc);
extern const char *GetBPRDesc(BPR_ItemID &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_ItemID(RecordID v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_Custom
typedef BehaviorParamRef<int,4> BPR_Custom;
#define GELEM_BPR_CUSTOM(varname,clss,showname,desc)						\
	GELEM_VAR_INIT(BPR_Custom,varname,BPR_Custom(0));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_Custom:" ## #clss),desc);
extern const char *GetBPRDesc(BPR_Custom &bpr,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_UnitID
typedef BehaviorParamRef<RecordID,7> BPR_UnitID;
#define GELEM_BPR_UNITID(varname,showname,desc)						\
	GELEM_VAR_INIT(BPR_UnitID,varname,BPR_UnitID(RecordID_Invalid));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_UnitID"),desc);
extern const char *GetBPRDesc(BPR_UnitID &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_UnitID(RecordID v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_ResourceID
typedef BehaviorParamRef<RecordID,9> BPR_ResourceID;
#define GELEM_BPR_RESOURCEID(varname,showname,desc)						\
	GELEM_VAR_INIT(BPR_ResourceID,varname,BPR_ResourceID(RecordID_Invalid));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_ResourceID"),desc);
extern const char *GetBPRDesc(BPR_ResourceID &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_ResourceID(RecordID v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_EoID
typedef BehaviorParamRef<RecordID,8> BPR_EoID;
#define GELEM_BPR_EOID(varname,showname,desc)						\
	GELEM_VAR_INIT(BPR_EoID,varname,BPR_EoID(RecordID_Invalid));							\
	GELEM_EDITVAR(showname,GVT_None,GSem(GSem_Unknown,"BPR_EoID"),desc);
extern const char *GetBPRDesc(BPR_EoID &bpr,FillDescAssist *assist);
extern const char *GetBVRDesc_EoID(RecordID v,StringID nmRef,FillDescAssist *assist);

//////////////////////////////////////////////////////////////////////////
//BPR_CustomObj
#define GELEM_BPR_CUSTOMOBJ(varname,clssObj,showname,desc)						\
	GELEM_OBJVAR(BPR_CustomObj<clssObj>,varname);							\
	GELEM_EDITOBJ_EX(showname,GVT_None,GSem(GSem_Unknown,"BPR_CustomObj:" ## #clssObj));
extern const char *GetBPRDesc(BPR_CustomObjBase &bpr,FillDescAssist *assist);



//XXXXX:more BehaviorMemType
//XXXXX:more BPR

#define GELEM_BEHAVIORMEM_OBJ(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);															\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), desc );

#define GELEM_BEHAVIORMEM_OBJID(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);															\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:ObjID"), desc );

#define GELEM_BEHAVIORMEM_NUMBER(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																						\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:AllNumber"), desc );

#define GELEM_BEHAVIORMEM_INTERGER(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																			\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Interger"), desc );

#define GELEM_BEHAVIORMEM_POS(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																			\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Pos"), desc );

#define GELEM_BEHAVIORMEM_OBJID(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																			\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:ObjID"), desc );

#define GELEM_BEHAVIORMEM_ITEMRECORD(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																			\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:ItemRecord"), desc );

#define GELEM_BEHAVIORMEM_SKILLRECORD(varname,showname,desc)											\
GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																						\
GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:SkillRecord"), desc );

#define GELEM_BEHAVIORMEM_BUFFRECORD(varname,showname,desc)											\
	GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																						\
	GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:BuffRecord"), desc );

#define GELEM_BEHAVIORMEM_EORECORD(varname,showname,desc)											\
	GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																			\
	GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:EoRecord"), desc );

#define GELEM_BEHAVIORMEM_RESOURCERECORD(varname,showname,desc)											\
	GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																			\
	GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:ResRecord"), desc );

#define GELEM_BEHAVIORMEM_STRINGID(varname,showname,desc)											\
	GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																						\
	GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:StringID"), desc );

#define GELEM_BEHAVIORMEM_ALL(varname,showname,desc)											\
	GELEM_VAR_INIT( StringID,varname,StringID_Invalid);															\
	GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"行为图内存变量名称:All"), desc );

//XXXXX:more BehaviorMemType


#define GELEM_BEHAVIOR_TROOP(varname,showname,desc)											\
	GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																						\
	GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"Troop名称"), desc );

#define GELEM_BEHAVIOR_TROOPREF(varname,showname,desc)											\
	GELEM_VAR_INIT( StringID,varname,StringID_Invalid);																						\
	GELEM_EDITVAR( showname, GVT_U, GSem(GSem_StringID,"Troop名称引用"), desc );

