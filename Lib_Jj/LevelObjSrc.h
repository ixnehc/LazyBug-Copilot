#pragma once

#include "class/class.h"

#include "strlib/strlibdefines.h"

#include "LevelDefines.h"

#include "records/recordsdefine.h"

#include <unordered_map>


class CLevelObjSrc;
inline void los_verify(CLevelObjSrc*c) {}

#define DEFINE_LEVELOBJSRC_CLASS(clss,uid)											\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_LevelObjSrc;														\
		instance._uid=uid;																					\
		{clss *p=NULL;los_verify(p);}																	\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;


class CLevelObjParam;
inline void lop_verify(CLevelObjParam*c) {}

#define DEFINE_LEVELOBJPARAM_CLASS(clss,uid)											\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_LevelObjParam;														\
		instance._uid=uid;																					\
		{clss *p=NULL;lop_verify(p);}																	\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;

#define GELEM_ALLOWDISABLE()																				\
	GELEM_VAR_INIT(BYTE,_bDisable,0);																	\
			GELEM_EDITVAR("是否禁用",GVT_B,GSem(GSem_Interger,"可用,禁用"),"是否禁用");

#define GELEM_ALLOWPLAYERID()												\
	GELEM_VAR_INIT(LevelPlayerID,_idPlayer,LevelPlayerID_NeutralWild);																	\
		GELEM_EDITVAR("PlayerID",GVT_B,GSem(GSem_Interger,"Wild:15,NeutalWild:14,PlayerWild:13"),"隶属于哪个Player");


class CLevelObj;
struct GObjBase;

class CLevelChanceData;
class CLevelChancer;
class CLevelRecords;
class CRecord;
struct LevelRecordAgent;
class CLevel;
class CLevelObjParam
{
public:
	CLevelObjParam()
	{
		_bDisable=0;
		_guid_=0;
		_idPlayer=LevelPlayerID_NeutralWild;
	}

	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;

	virtual void RegisterChance(CLevelChanceData *cd,CLevelRecords *records){}
	virtual BOOL CheckCreateChance(CLevel *level,CLevelObjSrc *los)	{		return TRUE;	}//返回LevelObj能否创建

	virtual StringID GetUniqueName()	{		return StringID_Invalid;	}

	BOOL IsDisable()	{		return _bDisable;	}
	LevelPlayerID GetPlayerID()	{		return _idPlayer;	}

	LevelGUID GetGUID_()	{		return _guid_;	}

protected:
	BYTE _bDisable;
	LevelGUID _guid_;
	LevelPlayerID _idPlayer;

	friend class CLevelSources;

};

//注意:CLevelObjSrc和CLevelObjParam都是CLevelObj的参数,它们两者的区别是
//CLevelObjSrc里包含相对固定的参数,可能很多CLevelObj会共享同一个CLevelObjSrc的参数
//而CLevelObjParam里包含变化很大的参数,每个CLevelObj的参数都会不一样
class CLevelObjSrc
{
public:
	CLevelObjSrc()
	{
		_idRec=RecordID_Invalid;
		_rec=NULL;
		_bDisable=0;
		_guid=0;
	}

	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;

	virtual BOOL NeedSyncGUID()=0;

	i_math::matrix43f&GetMat()	{		return _mat;	}
	LevelPos GetPos()
	{		
		i_math::vector3df *p=_mat.getTranslationP();
		return LevelPos(p->x,p->z);
	}
	RecordID GetRecID()	{		return _idRec;	}
 	LevelRecordAgent*GetRecord()	{		return _rec;	}
 	void SetRecord(LevelRecordAgent*rec)	{		_rec=rec;	}

	LevelGUID GetGUID()	{		return _guid;	}
	void SetGUID(LevelGUID guid)	{		_guid=guid;	}

	void CopyFrom(CLevelObjSrc *src);

	BOOL IsDisable()	{		return _bDisable;	}

	BOOL IsAttackable();

protected:
	i_math::matrix43f _mat;
	LevelGUID _guid;
	BYTE _bDisable;
	LevelRecordAgent* _rec;

	RecordID _idRec;//

	//注意加了新变量后,要记得在CopyFrom里增加复制的代码

	friend class CLevelSources;
	friend class CLevelBasis;

};


class CDataPacket;
class CLevelObjSrc;
class CLevelSources
{
public:
	DEFINE_CLASS(CLevelSources);
	struct Src
	{
		CLevelObjSrc *src;
		CLevelObjParam *param;
	};
	CLevelSources()
	{
		_bLookUpBuilt=FALSE;
	}
	void Clear();

	BOOL Save(CDataPacket &dp);
	BOOL Load(CDataPacket &dp);

	void Add(CLevelObjSrc *src,CLevelObjParam *param=NULL)
	{
		Src s;
		s.src=src;
		s.param=param;
		_buf.push_back(s);
		_bLookUpBuilt=FALSE;
	}

	CLevelObjSrc *FindLos(LevelGUID guid);
	CLevelObjParam *FindLop(LevelGUID guid);

	DWORD GetCount()	{		return _buf.size();	}
	CLevelObjSrc *GetLos(DWORD idx);
	CLevelObjParam *GetLop(DWORD idx);

protected:
	std::vector<Src> _buf;

	void _BuildLookUp();
	std::unordered_map<LevelGUID,Src*>_lookup;
	BOOL _bLookUpBuilt;


	friend class CLevel;
	friend class CLevelBasis;
	friend class CLevelSlatesBasis;
	friend class CWorldBasis;
};

