/********************************************************************
	created:	2016/01/09 
	author:		cxi
	
	purpose:	 Behavior Value
*********************************************************************/
#include "stdh.h"

#include "BehaviorValue.h"


//////////////////////////////////////////////////////////////////////////
//BhvValDeclare
void BhvValDeclare::AssignDefault(BhvVal &e)
{
	e.tp=tp;
	e.nm=nm;
	e.nmRef=StringID_BhvValInvalidRef;
	e.data=dataDef;
}

BOOL BhvValDeclare::CheckDefault(BhvVal &e)
{
	if (!(e.tp==tp))
		return FALSE;
	if (e.nm!=nm)
		return FALSE;
	if (e.nmRef!=StringID_BhvValInvalidRef)
		return FALSE;
	if (!(e.data==dataDef))
		return FALSE;

	return TRUE;
}

BOOL BhvValDeclare::IsCompatible(BhvValDeclare &other)
{
	return tp.IsCompatible(other.tp);
}

//////////////////////////////////////////////////////////////////////////
//BhvValType
void BhvValType::From(GElemBase *elem)
{
	tp=(BYTE)elem->GetTypeID();
	gvt=(BYTE)elem->GetVarType();
	subname=elem->subtype;
	codeSem=(BYTE)elem->sem.code;
	constraintSem=elem->sem.constraint;

	if (TRUE)
	{
		tpMem=BehaviorMemType_None;
		if (tp==0)
		{
			switch(codeSem)
			{
				case GSem_Boolean:
					tpMem=BehaviorMemType_Bit;
					break;
				case GSem_Interger:
					tpMem=BehaviorMemType_Integer;
					break;
				case GSem_Float:
					tpMem=BehaviorMemType_Float;
					break;
				case GSem_StringID:
					tpMem=BehaviorMemType_StringID;
					break;
				case GSem_RecordID:
				{
					if (constraintSem=="skills")
						tpMem=BehaviorMemType_SkillRecord;
					else if (constraintSem=="units")
						tpMem=BehaviorMemType_UnitRecord;
					else if (constraintSem=="buffs")
						tpMem=BehaviorMemType_BuffRecord;
					else if (constraintSem=="items")
						tpMem=BehaviorMemType_ItemRecord;
					else if (constraintSem=="resources")
						tpMem=BehaviorMemType_ResourceRecord;
					break;
				}
				case GSem_Point:
				{
					if (gvt==GVT_Fx2)
						tpMem=BehaviorMemType_Pos;
					break;
				}
				case GSem_ObjID:
				{
					if (gvt==GVT_U)
						tpMem=BehaviorMemType_ObjID;
					break;
				}
				//XXXXX:more BehaviorMemType
			}
		}
	}

}


//////////////////////////////////////////////////////////////////////////
//BhvVal

void BhvVal::Clear()
{
	tp.GClear();
	data.clear();
	Zero();
}

void BhvVal::CopyFrom(BhvVal &src)
{
	nm=src.nm;
	tp=src.tp;
	nmRef=src.nmRef;
	data=src.data;
}

void BhvVal::Save(CDataPacket &dp)
{
	dp.Data_NextWord()=0;//ver
	dp.Data_WriteSimple(nm);
	tp.GSave(dp);
	dp.Data_WriteSimple(nmRef);
	if (nmRef==StringID_BhvValInvalidRef)
	{
		DP_WriteVector(dp,data);
	}
}

void BhvVal::Load(CDataPacket &dp)
{
	WORD ver=dp.Data_NextWord();
	dp.Data_ReadSimple(nm);
	tp.GLoad(dp);
	dp.Data_ReadSimple(nmRef);
	if (nmRef==StringID_BhvValInvalidRef)
	{
		DP_ReadVector(dp,data);
	}
}

BOOL BhvVal::Equals(BhvVal &entryOther)
{
	if (nm!=entryOther.nm)
		return FALSE;
	if (!(tp==entryOther.tp))
		return FALSE;
	if (nmRef!=entryOther.nmRef)
		return FALSE;
	if (nmRef==StringID_BhvValInvalidRef)
	{
		if (data!=entryOther.data)
			return FALSE;
	}
	return TRUE;
}

BOOL BhvVal::IsCompatible(BhvVal &entryOther)
{
	return tp.IsCompatible(entryOther.tp);
}

void BhvVal::SetInt(int v)
{
	switch(tp.gvt)
	{
		case GVT_B:
		{
			data.resize(1);
			*(BYTE*)data.data() =(BYTE)v;
			break;
		}
		case GVT_SS:
		{
			data.resize(2);
			*(short*)data.data() =(short)v;
			break;
		}
		case GVT_SU:
		{
			data.resize(2);
			*(unsigned short*)data.data() =(unsigned short)v;
			break;
		}
		case GVT_S:
		{
			data.resize(4);
			*(int*)data.data() =(int)v;
			break;
		}
		case GVT_U:
		{
			data.resize(4);
			*(unsigned int*)data.data() =(unsigned int)v;
			break;
		}
	}
}

void BhvVal::SetFloat(float v)
{
	if (tp.gvt==GVT_F)
	{
		data.resize(4);
		*(float*)data.data() =v;
	}

}
