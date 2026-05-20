#include "stdh.h"

#include "LevelDefines.h"

#include "datapacket/BitPacket.h"

#include "behaviorgraph/BehaviorValue.h"

//////////////////////////////////////////////////////////////////////////
//LevelSkillArg

void LevelSkillArg::Save(CBitPacket *bp)
{
	DP_WriteVector(*bp,objs);
	DP_WriteVector(*bp,sites);
	DP_WriteVector(*bp,data);
	bp->Data_WriteSimple(seedRnd);
}

void LevelSkillArg::Load(CBitPacket *bp)
{
	DP_ReadVector(*bp,objs);
	DP_ReadVector(*bp,sites);
	DP_ReadVector(*bp,data);
	bp->Data_ReadSimple(seedRnd);
}


//////////////////////////////////////////////////////////////////////////
//LevelSimpleMem
void LevelSimpleMem::CopyContent(LevelSimpleMem &from)
{
	memcpy(this,&from,(&bPersistDirty)-(BYTE*)this);
}

BOOL LevelSimpleMem::EqualContent(LevelSimpleMem &other)
{
	return memcmp(this,&other,(&bPersistDirty)-(BYTE*)this)==0;
}


void LevelSimpleMem::Save(CDataPacket &dp)
{
	dp.Data_WriteData(this,(int)((&bPersistDirty)-(BYTE*)this));
}

void LevelSimpleMem::Load(CDataPacket &dp)
{
	dp.Data_ReadData(this,(int)((&bPersistDirty)-(BYTE*)this));

}


BOOL LevelSimpleMem::SetValue(StringID nmVar,DWORD value)
{
	switch(nmVar)
	{
		case LevelSimpleVarName_CheckDay:
		{
			iCheckDay=(BYTE)value;
			return TRUE;
		}
		case LevelSimpleVarName_Flag0:
		{
			if (value!=0)
				flag0=1;
			else
				flag0=0;
			return TRUE;
		}
		case LevelSimpleVarName_Flag1:
		{
			if (value!=0)
				flag1=1;
			else
				flag1=0;
			return TRUE;
		}
		case LevelSimpleVarName_Flag2:
		{
			if (value!=0)
				flag2=1;
			else
				flag2=0;
			return TRUE;
		}
		case LevelSimpleVarName_Byte:
		{
			b=(BYTE)value;
			return TRUE;
		}
		case LevelSimpleVarName_Word:
		{
			w=(WORD)value;
			return TRUE;
		}
		//XXXXX:more simple var
	}
	return FALSE;
}

BOOL LevelSimpleMem::GetValue(StringID nmVar,DWORD &value)
{
	switch(nmVar)
	{
		case LevelSimpleVarName_CheckDay:
			value=(DWORD)iCheckDay;
			return TRUE;
		case LevelSimpleVarName_Flag0:
			value=(DWORD)flag0;
			return TRUE;
		case LevelSimpleVarName_Flag1:
			value=(DWORD)flag1;
			return TRUE;
		case LevelSimpleVarName_Flag2:
			value=(DWORD)flag2;
			return TRUE;
		case LevelSimpleVarName_Byte:
			value=(DWORD)b;
			return TRUE;
		case LevelSimpleVarName_Word:
			value=(DWORD)w;
			return TRUE;
		//XXXXX:more simple var
	}
	return FALSE;
}

BOOL LevelSimpleMem::FillBehaviorValue(StringID nm,BhvVal &v)
{
	switch(v.tp.tpMem)
	{
		case BehaviorMemType_Bit:
		case BehaviorMemType_Integer:
		{
			DWORD n;
			if (GetValue(nm,n))
			{
				v.SetInt((int)n);
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}

BehaviorMemType ResolveSimpleVarType(StringID nmVar)
{
	if (nmVar==StringID_Invalid)
		return BehaviorMemType_None;

	if (nmVar>LevelSimpleVarName_Max)
		return BehaviorMemType_None;

	switch(nmVar)
	{
	case LevelSimpleVarName_Flag0:
	case LevelSimpleVarName_Flag1:
	case LevelSimpleVarName_Flag2:
	case LevelSimpleVarName_Flag3:
		return BehaviorMemType_Bit;
	case LevelSimpleVarName_CheckDay:
	case LevelSimpleVarName_Byte:
	case LevelSimpleVarName_Word:
		return BehaviorMemType_Integer;
		//XXXXX:more simple var
	}
	return BehaviorMemType_None;
}


const char *GetLevelAbilityActionName(LevelAbilityAction action)
{
	switch(action)
	{
		case LevelAbilityAction_AttackA:												return "AttackA";
		case LevelAbilityAction_AttackA_Dash:									return "AttackA_Dash";
		case LevelAbilityAction_AttackA_RunningDash:						return "AttackA_RunningDash";
		case LevelAbilityAction_AttackB:												return "AttackB";
		case LevelAbilityAction_AttackB_Dash:									return "AttackB_Dash";
		case LevelAbilityAction_AttackC:												return "AttackC";
		case LevelAbilityAction_AttackC_Dash:									return "AttackC_Dash";
		case LevelAbilityAction_AttackD:												return "AttackD";
		case LevelAbilityAction_AttackD_Dash:									return "AttackD_Dash";
		case LevelAbilityAction_EvadeB:												return "EvadeB";
		case LevelAbilityAction_EvadeF:												return "EvadeF";
		case LevelAbilityAction_EvadeL:												return "EvadeL";
		case LevelAbilityAction_EvadeR:												return "EvadeR";
		case LevelAbilityAction_AttackPreB:											return "AttackPreB";
		case LevelAbilityAction_AttackPreC:										return "AttackPreC";
		case LevelAbilityAction_AttackPreD:										return "AttackPreD";
		case LevelAbilityAction_TeleLeftA:											return "TeleLeftA";
		case LevelAbilityAction_TeleRightA:											return "TeleRightA";
		case LevelAbilityAction_TeleBackA:											return "TeleBackA";
		case LevelAbilityAction_JumpF:												return "JumpF";
		case LevelAbilityAction_AttackPreAR:										return "AttackPreAR";
		case LevelAbilityAction_AttackAR:											return "AttackAR";
		case LevelAbilityAction_AttackAR_Dash:									return "AttackAR_Dash";
		case LevelAbilityAction_FuryA:													return "FuryA";
		case LevelAbilityAction_FuryB:													return "FuryB";
		case LevelAbilityAction_FuryC:													return "FuryC";
		case LevelAbilityAction_FuryD:													return "FuryD";
		case LevelAbilityAction_MissileA:												return "MissileA";
		case LevelAbilityAction_Guard:													return "Guard";
		case LevelAbilityAction_ShieldAttack:										return "ShieldAttack";
		//XXXXX:More LevalAbilityAction
	}
	return "n/a";
}

BOOL IsInterruptibleComboAbilityAction(LevelAbilityAction action)
{
	switch(action)
	{
		case LevelAbilityAction_AttackPreB:	
		case LevelAbilityAction_AttackPreC:
		case LevelAbilityAction_AttackPreD:
		case LevelAbilityAction_AttackPreAR:
			return TRUE;
		//XXXXX:More LevalAbilityAction
	}
	return FALSE;

}


void GetMainGameCameraAxes(i_math::vector3df &axisX,i_math::vector3df &axisY,i_math::vector3df &axisZ)
{
	static i_math::vector3df vX,vY,vZ;
	static BOOL bInit=FALSE;
	if (!bInit)
	{
//		vZ=i_math::vector3df(0.0f,-0.70710683,0.70710683);
// 		vZ.normalize();
// 		vY.set(0,1,0);

		vZ=i_math::vector3df(0.0f,-1.0f,0.0f);
		vY.set(0,0,1);

		vX=vY.crossProduct(vZ);
		vX.normalize();
		vY=vZ.crossProduct(vX);
		vY.normalize();
		bInit=TRUE;
	}

	axisX=vX;
	axisY=vY;
	axisZ=vZ;
}