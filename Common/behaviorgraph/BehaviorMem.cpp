/********************************************************************
	created:	2013/6/8 
	author:		cxi
	
	purpose:	Behavior Mem
*********************************************************************/
#include "stdh.h"
#include "BehaviorMem.h"
#include "BehaviorValue.h"


////////////////////////////////////////////////////////////////////////
//CBehaviorMemDesc

BOOL CBehaviorMemDesc::Exist(StringID nm)
{
	std::unordered_map<StringID,VarEntry>::iterator it=_vars.find(nm);
	if (it!=_vars.end())
		return TRUE;
	return FALSE;
}

void CBehaviorMemDesc::AddBit(StringID nm,BehaviorMemFlag flag,int vInit)
{
	DWORD szData=_szTotal-(_szBits+7)/8;

	VarEntry e;
	e.nm=nm;
	e.tp=BehaviorMemType_Bit;
	e.vInit=vInit;
	e.flag=flag;

	if (flag&BehaviorMemFlag_Persist)
		_nPersist++;
	if (flag&BehaviorMemFlag_Sync)
	{
		e.idxSync=_nSync;
		_nSync++;
	}


	e.idxByte=_szBits/8;
	e.idxBit=_szBits%8;

	_vars[nm]=e;

	_szBits++;
	_szTotal=szData+(_szBits+7)/8;
}

void CBehaviorMemDesc::AddNumber(StringID nm,BehaviorMemFlag flag,int vInit)
{
	DWORD szData=_szTotal-(_szBits+7)/8;

	VarEntry e;
	e.nm=nm;
	e.tp=BehaviorMemType_Integer;
	e.vInit=vInit;
	e.flag=flag;

	if (flag&BehaviorMemFlag_Persist)
		_nPersist++;
	if (flag&BehaviorMemFlag_Sync)
	{
		e.idxSync=_nSync;
		_nSync++;
	}

	e.idxByte=szData;
	e.idxBit=0;

	_vars[nm]=e;

	szData+=2;
	_szTotal=szData+(_szBits+7)/8;
}

void CBehaviorMemDesc::AddID(StringID nm,BehaviorMemType tpID,BehaviorMemFlag flag,int vInit)
{
	DWORD szData=_szTotal-(_szBits+7)/8;

	VarEntry e;
	e.nm=nm;
	e.tp=tpID;
	e.vInit=vInit;
	e.flag=flag;

	if (flag&BehaviorMemFlag_Persist)
		_nPersist++;
	if (flag&BehaviorMemFlag_Sync)
	{
		e.idxSync=_nSync;
		_nSync++;
	}

	e.idxByte=szData;
	e.idxBit=0;

	_vars[nm]=e;

	szData+=4;
	_szTotal=szData+(_szBits+7)/8;
}

void CBehaviorMemDesc::AddPos(StringID nm,BehaviorMemFlag flag)
{
	DWORD szData=_szTotal-(_szBits+7)/8;

	VarEntry e;
	e.nm=nm;
	e.tp=BehaviorMemType_Pos;
	e.vInit=0;
	e.flag=flag;

	if (flag&BehaviorMemFlag_Persist)
		_nPersist++;
	if (flag&BehaviorMemFlag_Sync)
	{
		e.idxSync=_nSync;
		_nSync++;
	}

	e.idxByte=szData;
	e.idxBit=0;

	_vars[nm]=e;

	szData+=8;
	_szTotal=szData+(_szBits+7)/8;
}

void CBehaviorMemDesc::AddFloat(StringID nm,BehaviorMemFlag flag,float fInit)
{
	DWORD szData=_szTotal-(_szBits+7)/8;

	VarEntry e;
	e.nm=nm;
	e.tp=BehaviorMemType_Float;
	e.fInit=fInit;
	e.flag=flag;

	if (flag&BehaviorMemFlag_Persist)
		_nPersist++;
	if (flag&BehaviorMemFlag_Sync)
	{
		e.idxSync=_nSync;
		_nSync++;
	}

	e.idxByte=szData;
	e.idxBit=0;

	_vars[nm]=e;

	szData+=4;
	_szTotal=szData+(_szBits+7)/8;
}

void CBehaviorMemDesc::AddObj(StringID nm,BehaviorMemFlag flag)
{
	VarEntry e;
	e.nm=nm;
	e.tp=BehaviorMemType_Obj;
	e.fInit=0.0f;
//	flag=(BehaviorMemFlag)(flag&(BehaviorMemFlag)(~BehaviorMemFlag_Persist));//Obj 不支持 persist标志
	e.flag=flag;

	if (flag&BehaviorMemFlag_Persist)
		_nPersist++;

	if (flag&BehaviorMemFlag_Sync)
	{
		e.idxSync=_nSync;
		_nSync++;
	}

	e.idxByte=0;
	e.idxBit=0;


	_vars[nm]=e;
}




void CBehaviorMemDesc::AddState(PadID idPad,StringID nmState,BehaviorMemFlag flag)
{
	StateEntry e;
	e.flag=flag;
	e.nm=nmState;

	_states[idPad]=e;
}

void CBehaviorMemDesc::_BuildInitVarData()
{
	if (_szTotal==_dataInit.size())
		return;//Build 过了

	CBehaviorMem memT;
	memT._desc=this;
	memT._data.resize(_szTotal);

	std::unordered_map<StringID,VarEntry>::iterator it;
	for (it=_vars.begin();it!=_vars.end();it++)
	{
		VarEntry *e=&(*it).second;


		switch(e->tp)
		{
			case BehaviorMemType_Bit:
			{
				memT.SetBit(e->nm,e->vInit);
				break;
			}
			case BehaviorMemType_Integer:
			{
				memT.SetNumber(e->nm,e->vInit);
				break;
			}
			case BehaviorMemType_StringID:
			case BehaviorMemType_SkillRecord:
			case BehaviorMemType_BuffRecord:
			case BehaviorMemType_ItemRecord:
			case BehaviorMemType_UnitRecord:
			case BehaviorMemType_ResourceRecord:
			case BehaviorMemType_ObjID:
			case BehaviorMemType_GUID:
			{
				memT.SetID(e->nm,e->tp,e->vInit);
				break;
			}
			case BehaviorMemType_Pos:
			{
				memT.SetPos(e->nm,i_math::vector2df());
				break;
			}
			case BehaviorMemType_Float:
			{
				memT.SetFloat(e->nm,e->fInit);
				break;
			}
			case BehaviorMemType_Obj:
			{
				//什么也不做
				break;
			}
			//XXXXX:more BehaviorMemType
		}
	}

	_dataInit.swap(memT._data);
	memT.Clear();
}

BehaviorMemType CBehaviorMemDesc::GetVarType(StringID nm)
{
	std::unordered_map<StringID,VarEntry>::iterator it=_vars.find(nm);
	if(it==_vars.end())
		return BehaviorMemType_None;
	return (*it).second.tp;
}


////////////////////////////////////////////////////////////////////////
//CBehaviorMem
void CBehaviorMem::Init(CBehaviorMemDesc *desc)
{
	_desc=desc;
	_desc->_BuildInitVarData();
	_data=_desc->_dataInit;

	assert(desc->_nSync<=64);
	_flagsSyncDirty.resize(desc->_nSync);
	_flagsSyncDirty.setAll();//全部设置成dirty
}

void CBehaviorMem::Clear()
{
	_data.clear();
	_states.clear();

	if (TRUE)
	{
		std::unordered_map<StringID,CBehaviorMemObj *>::iterator it;
		for (it=_objs.begin();it!=_objs.end();it++)
		{
			CBehaviorMemObj *e=(*it).second;
			Safe_Class_Delete(e);
		}
		_objs.clear();
	}

	Zero();
}


void CBehaviorMem::_Save(CDataPacket *dp,BehaviorMemFlag flag)
{
	//先保存States
	if (TRUE)
	{
		BYTE *pc=(BYTE*)dp->GetCurBufferPointer();
		dp->Data_NextByte();
		if (pc)
			*pc=0;
		std::unordered_set<PadID>::iterator it;
		for (it=_states.begin();it!=_states.end();it++)
		{
			std::unordered_map<PadID,CBehaviorMemDesc::StateEntry>::iterator it2=_desc->_states.find(*it);
			if (it2==_desc->_states.end())
				continue;

			CBehaviorMemDesc::StateEntry *e=&(*it2).second;

			if (!(e->flag&flag))
				continue;

			if (flag&BehaviorMemFlag_Sync)
				dp->Data_WriteSimple(e->nm);//网络同步,我们传送State的名称
			else
				dp->Data_WriteSimple(*it);
			if (pc)
				(*pc)++;
		}
	}

	//再保存Vars
	if (flag&BehaviorMemFlag_Persist)
	{
		dp->Data_NextByte()=_desc->_nPersist>0?1:0;
		if (_desc->_nPersist<=0)
			return;
	}
	else
	{
		assert(flag&BehaviorMemFlag_Sync);
		dp->Data_NextByte()=_desc->_nSync>0?1:0;
		if (_desc->_nSync<=0)
			return;
	}

	BYTE *c2=&dp->Data_NextByte();
	*c2=0;

	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it;
	for (it=_desc->_vars.begin();it!=_desc->_vars.end();it++)
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (!(e->flag&flag))
			continue;

		if (flag==BehaviorMemFlag_Sync)
		{
			assert(e->idxSync>=0);

			if (!_flagsSyncDirty.test(e->idxSync))
				continue;
		}

		switch(e->tp)
		{
			case BehaviorMemType_Bit:
			{
				DWORD v=_data[e->idxByte]&(1<<e->idxBit)?1:0;
				v|=2;
				v<<=14;

				v|=(e->nm&0xffff3fff);

				dp->Data_NextDword()=v;

				*c2=*c2+1;
				break;
			}
			case BehaviorMemType_Integer:
			{
				DWORD v=0;
				v<<=14;
				v|=(e->nm&0xffff3fff);

				dp->Data_NextDword()=v;
				dp->Data_NextShort()=*(short*)&_data[(_desc->_szBits+7)/8+e->idxByte];

				*c2=*c2+1;
				break;
			}
			case BehaviorMemType_StringID:
			case BehaviorMemType_SkillRecord:
			case BehaviorMemType_BuffRecord:
			case BehaviorMemType_ItemRecord:
			case BehaviorMemType_UnitRecord:
			case BehaviorMemType_ResourceRecord:
			case BehaviorMemType_ObjID:
			case BehaviorMemType_GUID:
			{
				DWORD v=1;
				v<<=14;
				v|=(e->nm&0xffff3fff);

				dp->Data_NextDword()=v;
				dp->Data_NextByte()=(BYTE)(e->tp);
				dp->Data_NextDword()=*(DWORD*)&_data[(_desc->_szBits+7)/8+e->idxByte];

				*c2=*c2+1;
				break;
			}
			case BehaviorMemType_Pos:
			{
				DWORD v=1;
				v<<=14;
				v|=(e->nm&0xffff3fff);

				dp->Data_NextDword()=v;
				dp->Data_NextByte()=(BYTE)(e->tp);
				i_math::vector2df *pos=(i_math::vector2df*)&_data[(_desc->_szBits+7)/8+e->idxByte];
				dp->Data_WriteSimpleR(*pos);

				*c2=*c2+1;
				break;
			}
			case BehaviorMemType_Float:
			{
				DWORD v=1;
				v<<=14;
				v|=(e->nm&0xffff3fff);

				dp->Data_NextDword()=v;
				dp->Data_NextByte()=(BYTE)(e->tp);
				float*f=(float*)&_data[(_desc->_szBits+7)/8+e->idxByte];
				dp->Data_WriteSimpleR(*f);

				*c2=*c2+1;
				break;
			}
			//XXXXX:more BehaviorMemType
		}
	}

	//Obj
	if (TRUE)
	{
		BYTE *c2=&dp->Data_NextByte();
		*c2=0;
		std::unordered_map<StringID,CBehaviorMemObj *>::iterator it;
		for (it=_objs.begin();it!=_objs.end();it++)
		{
			std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it2=_desc->_vars.find((*it).first);
			if (it2!=_desc->_vars.end())
			{
				CBehaviorMemDesc::VarEntry *e=&(*it2).second;
				if (e->flag&flag)
				{
					if (flag==BehaviorMemFlag_Sync)
					{
						assert(e->idxSync>=0);

						if (!_flagsSyncDirty.test(e->idxSync))
							continue;
					}

					dp->Data_WriteSimple(e->nm);

					CBehaviorMemObj *pObj=(*it).second;
					if (pObj)
					{
						dp->Data_WriteStringSH(pObj->GetClass()->GetName());

						CDataPacket &dpR=*dp;
						DP_PreSafeSave(dpR);
						SaveGObj(*dp,pObj->GetGObj());
						DP_PostSafeSave();
					}
					else
						dp->Data_WriteStringSH("");

					*c2=*c2+1;
				}
			}
		}
	}

}


void CBehaviorMem::_Load(CDataPacket *dp,BehaviorMemFlag flag)
{
	assert(_desc);
	assert(flag==BehaviorMemFlag_Persist);

	//先载入States
	if (TRUE)
	{
		BYTE c=dp->Data_NextByte();
		for (int i=0;i<c;i++)
		{
			PadID idPad;
			dp->Data_ReadSimple(idPad);
			std::unordered_map<PadID,CBehaviorMemDesc::StateEntry>::iterator it2=_desc->_states.find(idPad);
			if (it2==_desc->_states.end())
				continue;

			CBehaviorMemDesc::StateEntry *e=&(*it2).second;

			if (!(e->flag&flag))
				continue;

			_states.insert(idPad);
		}
	}

	//再载入Vars
	if (dp->Data_NextByte()==1)
	{
		BYTE c2=dp->Data_NextByte();
		for (int i=0;i<c2;i++)
		{
			DWORD v=dp->Data_NextDword();
			StringID nm=v&0xffff3fff;

			BehaviorMemType tp=BehaviorMemType_None;
			BOOL b;
			short n;
			DWORD id;
			i_math::vector2df pos;
			float f;
			v=(v>>14)&0x00000003;
			if (v&2)
			{
				tp=BehaviorMemType_Bit;
				b=v&1;
			}
			else
			{
				if (v==0)
				{
					tp=BehaviorMemType_Integer;
					n=dp->Data_NextShort();
				}
				else
				{
					tp=(BehaviorMemType)dp->Data_NextByte();
					switch(tp)
					{
						case BehaviorMemType_StringID:
						case BehaviorMemType_SkillRecord:
						case BehaviorMemType_BuffRecord:
						case BehaviorMemType_ItemRecord:
						case BehaviorMemType_UnitRecord:
						case BehaviorMemType_ResourceRecord:
						case BehaviorMemType_ObjID:
						case BehaviorMemType_GUID:
							id=dp->Data_NextDword();
							break;
						case BehaviorMemType_Pos:
							dp->Data_ReadSimple(pos);
							break;
						case BehaviorMemType_Float:
							dp->Data_ReadSimple(f);
							break;
						default:
							assert(FALSE);
							//XXXXX:more BehaviorMemType
					}
				}
			}

			std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
			if (it!=_desc->_vars.end())
			{
				CBehaviorMemDesc::VarEntry *e=&(*it).second;
				if (e->flag&flag)
				{
					if (e->tp==tp)
					{
						switch(tp)
						{
							case BehaviorMemType_Bit:
							{
								if (b)
									_data[e->idxByte]|=(1<<e->idxBit);
								else
									_data[e->idxByte]&=(~(1<<e->idxBit));
								break;
							}
							case BehaviorMemType_Integer:
							{
								*(short*)&_data[(_desc->_szBits+7)/8+e->idxByte]=n;
								break;
							}
							case BehaviorMemType_StringID:
							case BehaviorMemType_SkillRecord:
							case BehaviorMemType_BuffRecord:
							case BehaviorMemType_ItemRecord:
							case BehaviorMemType_UnitRecord:
							case BehaviorMemType_ResourceRecord:
							case BehaviorMemType_ObjID:
							case BehaviorMemType_GUID:
							{
								*(DWORD*)&_data[(_desc->_szBits+7)/8+e->idxByte]=id;
								break;
							}
							case BehaviorMemType_Pos:
							{
								*(i_math::vector2df*)&_data[(_desc->_szBits+7)/8+e->idxByte]=pos;
								break;
							}
							case BehaviorMemType_Float:
							{
								*(float*)&_data[(_desc->_szBits+7)/8+e->idxByte]=f;
								break;
							}
							//XXXXX:more BehaviorMemType
						}
					}
				}
			}
		}

		if (TRUE)
		{
			std::string nmClass;
			BYTE c=dp->Data_NextByte();
			for (int i=0;i<c;i++)
			{
				StringID nm;
				dp->Data_ReadSimple(nm);

				dp->Data_ReadStringSH(nmClass);
				if (!nmClass.empty())
				{
					CDataPacket &dpR=*dp;
					DP_PreSafeLoad(dpR);

					CBehaviorMemObj *pObj=(CBehaviorMemObj *)CClass::New(nmClass.c_str());
					if (pObj)
					{
						LoadGObj(*dp,pObj->GetGObj(),NULL);
						_objs[nm]=pObj;
					}

					DP_PostSafeLoad();
				}
				else
					_objs[nm]=NULL;
			}
		}
	}
}

#define BehaviorMem_CurVer 1

void CBehaviorMem::SavePersist(CDataPacket *dp)
{
	dp->Data_NextByte()=BehaviorMem_CurVer;
	_Save(dp,BehaviorMemFlag_Persist);
}

void CBehaviorMem::LoadPersist(CDataPacket *dp)
{
	BYTE ver=dp->Data_NextByte();
	_Load(dp,BehaviorMemFlag_Persist);
}

void CBehaviorMem::SetSyncDirty()	
{
	_bSyncDirty=1;

	_flagsSyncDirty.setN(_desc->_nSync);
}


void CBehaviorMem::SaveSync(CDataPacket *dp)
{
	_Save(dp,BehaviorMemFlag_Sync);
}

void CBehaviorMem::LoadSync(CDataPacket *dp)
{
	_Load(dp,BehaviorMemFlag_Sync);
}

BOOL CBehaviorMem::FillBehaviorValue(StringID nm,BhvVal &v)
{
	switch(v.tp.tpMem)
	{
		case BehaviorMemType_Bit:
		{
			BOOL b;
			if (GetBit(nm,b))
			{
				v.SetInt(b);
				return TRUE;
			}
			break;
		}
		case BehaviorMemType_Integer:
		{
			short n;
			if (GetNumber(nm,n))
			{
				v.SetInt((int)n);
				return TRUE;
			}
			break;
		}
		case BehaviorMemType_Float:
		{
			float f;
			if (GetFloat(nm,f))
			{
				v.SetFloat(f);
				return TRUE;
			}
			break;
		}
		default:
		{
			std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
			if (it!=_desc->_vars.end())
			{
				CBehaviorMemDesc::VarEntry *e=&(*it).second;
				if (e->tp==v.tp.tpMem)
				{
					switch(v.tp.tpMem)
					{
						case BehaviorMemType_StringID:
						case BehaviorMemType_SkillRecord:
						case BehaviorMemType_BuffRecord:
						case BehaviorMemType_ItemRecord:
						case BehaviorMemType_UnitRecord:
						case BehaviorMemType_ResourceRecord:
						case BehaviorMemType_ObjID:
						case BehaviorMemType_GUID:
						{
							v.data.resize(4);
							_GetID(e,*(DWORD*)&v.data[0]);
							return TRUE;
						}
						case BehaviorMemType_Pos:
						{
							v.data.resize(8);
							_GetPos(e,*(i_math::vector2df*)&v.data[0]);
							return TRUE;
						}
						case BehaviorMemType_Obj:
						{
							assert(FALSE);
							return TRUE;
						}
						//XXXXX:more BehaviorMemType
					}
				}
			}
		}
	}
	return FALSE;
}


BOOL CBehaviorMem::GetBit(StringID nm,BOOL &b)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (e->tp==BehaviorMemType_Bit)
		{
			_GetBit(e,b);
			return TRUE;
		}

		if (e->tp==BehaviorMemType_Integer)
		{
			short n;
			_GetNumber(e,n);
			b=(n!=0);
			return TRUE;
		}

		if (e->tp==BehaviorMemType_Float)
		{
			float f;
			_GetFloat(e,f);
			b=(f!=0.0f);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CBehaviorMem::GetNumber(StringID nm,short &n)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (e->tp==BehaviorMemType_Integer)
		{
			_GetNumber(e,n);
			return TRUE;
		}
		if (e->tp==BehaviorMemType_Bit)
		{
			BOOL b;
			_GetBit(e,b);
			if (b)
				n=1;
			else
				n=0;
			return TRUE;
		}

		if (e->tp==BehaviorMemType_Float)
		{
			float f;
			_GetFloat(e,f);
			n=(short)f;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CBehaviorMem::GetID(StringID nm,BehaviorMemType tp,DWORD &id)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (e->tp==tp)
		{
			_GetID(e,id);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CBehaviorMem::GetPos(StringID nm,i_math::vector2df &pos)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (e->tp==BehaviorMemType_Pos)
		{
			_GetPos(e,pos);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CBehaviorMem::GetFloat(StringID nm,float&f)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (e->tp==BehaviorMemType_Float)
		{
			_GetFloat(e,f);
			return TRUE;
		}

		if (e->tp==BehaviorMemType_Integer)
		{
			short n;
			_GetNumber(e,n);
			f=(float)n;
			return TRUE;
		}
		if (e->tp==BehaviorMemType_Bit)
		{
			BOOL b;
			_GetBit(e,b);
			if (b)
				f=1.0f;
			else
				f=0.0f;
			return TRUE;
		}

	}
	return FALSE;
}


BOOL CBehaviorMem::_SetBit(CBehaviorMemDesc::VarEntry *e,BOOL b)
{
	if (e->tp==BehaviorMemType_Bit)
	{
		BOOL bOld;
		if (_data[e->idxByte]&(1<<e->idxBit))
			bOld=TRUE;
		else
			bOld=FALSE;

		if (bOld!=b)
		{
			if (b)
				_data[e->idxByte]|=(1<<e->idxBit);
			else
				_data[e->idxByte]&=~(1<<e->idxBit);
			if (e->flag&BehaviorMemFlag_Persist)
				_bPersistDirty=1;
			if (e->flag&BehaviorMemFlag_Sync)
			{
				_bSyncDirty=1;
				_flagsSyncDirty.set(e->idxSync);
			}
		}
		return TRUE;
	}
	return FALSE;
}



BOOL CBehaviorMem::SetBit(StringID nm,BOOL b)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;
		if (_SetBit(e,b))
			return TRUE;
		if (_SetNumber(e,(short)b))
			return TRUE;

		return _SetFloat(e,(float)b);
	}
	return FALSE;
}

BOOL CBehaviorMem::_SetNumber(CBehaviorMemDesc::VarEntry *e,short v)
{
	if (e->tp==BehaviorMemType_Integer)
	{
		short *p=(short*)&_data[(_desc->_szBits+7)/8+e->idxByte];
		if ((*p)!=v)
		{
			*p=v;
			if (e->flag&BehaviorMemFlag_Persist)
				_bPersistDirty=1;
			if (e->flag&BehaviorMemFlag_Sync)
			{
				_bSyncDirty=1;
				_flagsSyncDirty.set(e->idxSync);
			}
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CBehaviorMem::SetNumber(StringID nm,short n)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;
		if (_SetNumber(e,n))
			return TRUE;
		if (_SetFloat(e,(float)n))
			return TRUE;
		return _SetBit(e,(BOOL)(n!=0));
	}
	return FALSE;
}

BOOL CBehaviorMem::SetID(StringID nm,BehaviorMemType tp,DWORD id)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (e->tp==tp)
		{
			DWORD *p=(DWORD*)&_data[(_desc->_szBits+7)/8+e->idxByte];
			if ((*p)!=id)
			{
				*p=id;
				if (e->flag&BehaviorMemFlag_Persist)
					_bPersistDirty=1;
				if (e->flag&BehaviorMemFlag_Sync)
				{
					_bSyncDirty=1;
					_flagsSyncDirty.set(e->idxSync);
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CBehaviorMem::SetPos(StringID nm,i_math::vector2df &pos)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (e->tp==BehaviorMemType_Pos)
		{
			i_math::vector2df*p=(i_math::vector2df*)&_data[(_desc->_szBits+7)/8+e->idxByte];
			if ((*p)!=pos)
			{
				*p=pos;
				if (e->flag&BehaviorMemFlag_Persist)
					_bPersistDirty=1;
				if (e->flag&BehaviorMemFlag_Sync)
				{
					_bSyncDirty=1;
					_flagsSyncDirty.set(e->idxSync);
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CBehaviorMem::_SetFloat(CBehaviorMemDesc::VarEntry *e,float f)
{
	if (e->tp==BehaviorMemType_Float)
	{
		float*p=(float*)&_data[(_desc->_szBits+7)/8+e->idxByte];
		if ((*p)!=f)
		{
			*p=f;
			if (e->flag&BehaviorMemFlag_Persist)
				_bPersistDirty=1;
			if (e->flag&BehaviorMemFlag_Sync)
			{
				_bSyncDirty=1;
				_flagsSyncDirty.set(e->idxSync);
			}
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CBehaviorMem::SetFloat(StringID nm,float f)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;

		if (_SetFloat(e,f))
			return TRUE;
		
		if (_SetNumber(e,(short)FloatToNearestInt(f)))
			return TRUE;

		return _SetBit(e,(BOOL)(f!=0.0f));
	}
	return FALSE;
}



void CBehaviorMem::AddState(PadID id,BehaviorMemFlag flag)
{
	if (_states.find(id)!=_states.end())
		return;//已经存在了
	_states.insert(id);
	if (flag&BehaviorMemFlag_Persist)
		_bPersistDirty=1;
	if (flag&BehaviorMemFlag_Sync)
		_bSyncDirty=1;
}

void CBehaviorMem::RemoveState(PadID id)
{
	std::unordered_set<PadID>::iterator it=_states.find(id);
	if (it!=_states.end())
	{
		_states.erase(it);

		std::unordered_map<PadID,CBehaviorMemDesc::StateEntry>::iterator it;
		it=_desc->_states.find(id);
		if (it!=_desc->_states.end())
		{
			CBehaviorMemDesc::StateEntry *e=&(*it).second;
			if (e->flag&BehaviorMemFlag_Persist)
				_bPersistDirty=1;
			if (e->flag&BehaviorMemFlag_Sync)
				_bSyncDirty=1;
		}
	}
}

BOOL CBehaviorMem::CheckState(PadID id)
{
	if (_states.find(id)!=_states.end())
		return TRUE;//已经存在了

	return FALSE;
}

BOOL CBehaviorMem::DepositObj(StringID nm,CBehaviorMemObj *obj)
{
	std::unordered_map<StringID,CBehaviorMemDesc::VarEntry>::iterator it=_desc->_vars.find(nm);
	if (it!=_desc->_vars.end())
	{
		CBehaviorMemDesc::VarEntry *e=&(*it).second;
		if (e->tp==BehaviorMemType_Obj)
		{
			std::unordered_map<StringID,CBehaviorMemObj *>::iterator it2=_objs.find(nm);
			if (it2==_objs.end())
			{
				if (e->flag&BehaviorMemFlag_Persist)
					_bPersistDirty=1;
				if (e->flag&BehaviorMemFlag_Sync)
				{
					_bSyncDirty=1;
					_flagsSyncDirty.set(e->idxSync);
				}
				_objs[nm]=obj;
			}
			else
			{
				if (e->flag&BehaviorMemFlag_Persist)
					_bPersistDirty=1;
				if (e->flag&BehaviorMemFlag_Sync)
				{
					_bSyncDirty=1;
					_flagsSyncDirty.set(e->idxSync);
				}

				if (obj!=(*it2).second)
				{
					Safe_Class_Delete((*it2).second);
					(*it2).second=obj;
				}
			}
			return TRUE;
		}
	}

	return FALSE;
}

CBehaviorMemObj *CBehaviorMem::GetObj(StringID nm)
{
	std::unordered_map<StringID,CBehaviorMemObj *>::iterator it=_objs.find(nm);
	if (it!=_objs.end())
		return (*it).second;

	return NULL;
}
