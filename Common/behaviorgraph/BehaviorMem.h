#pragma once

#include "../class/class.h"
#include "../strlib/strlibdefines.h"
#include "../linkpad/LinkPadDefines.h"

#include "bitset/bitset.h"

#include <unordered_map>

#include "BehaviorDefines.h"


class CBehaviorMemDesc
{
public:
	DEFINE_CLASS(CBehaviorMemDesc);
	CBehaviorMemDesc()
	{
		Zero();
	}
	~CBehaviorMemDesc()
	{
		Clear();
	}
	void Zero()
	{
		_szTotal=0;
		_szBits=0;

		_nPersist=0;
		_nSync=0;
	}

	void Clear()
	{
		_vars.clear();
		_states.clear();
		Zero();
	}


	struct VarEntry
	{
		VarEntry()
		{
			memset(this,0,sizeof(*this));
			idxSync=-1;
		}
		StringID nm;
		BehaviorMemType tp;
		int vInit;//初始值
		float fInit;
		BehaviorMemFlag flag;
		DWORD idxByte;
		DWORD idxBit;
		int idxSync;//第几个要Sync的变量
	};

	struct StateEntry
	{
		BehaviorMemFlag flag;
		StringID nm;
	};


	BOOL IsEmpty()	{		return (_vars.size()<=0)&&(_states.size()<=0);	}

	BOOL Exist(StringID nm);

	void AddBit(StringID nm,BehaviorMemFlag flag,int vInit=0);
	void AddNumber(StringID nm,BehaviorMemFlag flag,int vInit=0);
	void AddID(StringID nm,BehaviorMemType tp,BehaviorMemFlag flag,int vInit=0);
	void AddPos(StringID nm,BehaviorMemFlag flag);
	void AddFloat(StringID nm,BehaviorMemFlag flag,float fInit=0);
	void AddObj(StringID nm,BehaviorMemFlag flag);//注意Raw不支持BehaviorMemFlag_Persist

	void AddState(PadID idPad,StringID nmState,BehaviorMemFlag flag);

	BehaviorMemType GetVarType(StringID nm);

protected:
	void _BuildInitVarData();

	//Vars
	std::unordered_map<StringID,VarEntry>_vars;
	DWORD _szTotal;//以Byte为单位,整个数据块的大小
	DWORD _szBits;//一共有几位

	BYTE _nPersist;
	BYTE _nSync;

	std::vector<BYTE> _dataInit;

	//States
	std::unordered_map<PadID,StateEntry>_states;

	friend class CBehaviorMem;
};

struct BhvVal;
class CBehaviorMem
{
public:
	DEFINE_CLASS(CBehaviorMem);
	CBehaviorMem()
	{
		Zero();
	}

	void Zero()
	{
		_desc=NULL;
		_bPersistDirty=0;
		_bSyncDirty=0;
		_flagsSyncDirty.resetAll();
	}

	void Init(CBehaviorMemDesc *desc);
	void Clear();

	void SavePersist(CDataPacket *dp);
	void LoadPersist(CDataPacket *dp);
	BOOL IsPersistDirty()	{		return _bPersistDirty;	}
	void SetPersistDirty()	{		_bPersistDirty=1;	}
	void ClearPersistDirty()	{		_bPersistDirty=0;	}

	void SaveSync(CDataPacket *dp);
	void LoadSync(CDataPacket *dp);
	BOOL IsSyncDirty()	{		return _bSyncDirty;	}
	void SetSyncDirty()	;
	void ClearSyncDirty()	{		_bSyncDirty=0;_flagsSyncDirty.resetAll();	}


	BOOL GetBit(StringID nm,BOOL &b);
	BOOL GetNumber(StringID nm,short &n);
	BOOL GetID(StringID nm,BehaviorMemType tp,DWORD &id);
	BOOL GetPos(StringID nm,i_math::vector2df &pos);
	BOOL GetFloat(StringID nm,float&f);
	BOOL FillBehaviorValue(StringID nm,BhvVal &v);//

	BOOL SetBit(StringID nm,BOOL b);
	BOOL SetNumber(StringID nm,short n);
	BOOL SetID(StringID nm,BehaviorMemType tp,DWORD id);
	BOOL SetPos(StringID nm,i_math::vector2df&pos);
	BOOL SetFloat(StringID nm,float f);

	BOOL DepositObj(StringID nm,CBehaviorMemObj *obj);
	CBehaviorMemObj *GetObj(StringID nm);
	template <typename T>
	T *GetObj(StringID nm)
	{
		CBehaviorMemObj *p=GetObj(nm);
		if (p)
		{
			if (p->GetClass()->IsSameWith(Class_Ptr(T)))
				return (T*)p;
		}
		return NULL;
	}

	void AddState(PadID id,BehaviorMemFlag flag);
	void RemoveState(PadID id);
	BOOL CheckState(PadID id);

protected:

	void _Save(CDataPacket *dp,BehaviorMemFlag flag);
	void _Load(CDataPacket *dp,BehaviorMemFlag flag);

	BOOL _SetBit(CBehaviorMemDesc::VarEntry *e,BOOL b);
	BOOL _SetNumber(CBehaviorMemDesc::VarEntry *e,short v);
	BOOL _SetFloat(CBehaviorMemDesc::VarEntry *e,float f);

	void _GetBit(CBehaviorMemDesc::VarEntry *e,BOOL &b)
	{
		if (_data[e->idxByte]&(1<<e->idxBit))
			b=TRUE;
		else
			b=FALSE;
	}
	void _GetNumber(CBehaviorMemDesc::VarEntry *e,short &n)
	{
		n=*(short*)&_data[(_desc->_szBits+7)/8+e->idxByte];
	}
	void _GetID(CBehaviorMemDesc::VarEntry *e,DWORD &id)
	{
		id=*(DWORD*)&_data[(_desc->_szBits+7)/8+e->idxByte];
	}
	void _GetPos(CBehaviorMemDesc::VarEntry *e,i_math::vector2df &pos)
	{
		pos=*(i_math::vector2df*)&_data[(_desc->_szBits+7)/8+e->idxByte];
	}
	void _GetFloat(CBehaviorMemDesc::VarEntry *e,float&f)
	{
		f=*(float*)&_data[(_desc->_szBits+7)/8+e->idxByte];
	}


	CBehaviorMemDesc *_desc;
	std::vector<BYTE>_data;
	std::unordered_map<StringID,CBehaviorMemObj *>_objs;

	std::unordered_set<PadID>_states;

	DWORD _bPersistDirty:1;
	DWORD _bSyncDirty:1;
	Bitset<2> _flagsSyncDirty;//最多支持64个Var

	friend class CBehaviorMemDesc;
	friend class CBehavior;
};
