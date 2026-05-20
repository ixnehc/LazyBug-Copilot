/********************************************************************
	created:	2011/12/03
	file base:	Skill
	author:		cxi
	
	purpose:	技能
*********************************************************************/
#include "stdh.h"

#include "LevelOp.h"
#include "LevelClasses.h"

#include "LevelRecordPosture.h"
#include "LevelRecords.h"


#include "datapacket/BitPacket.h"


inline void WriteSkillTarget(CBitPacket *bp,LevelSkillTarget &target)
{
	bp->Bits_Write(target.tp,3);
	
	if (target.bOrg)
	{
		bp->Bit_Write_1();
		bp->Data_WriteSimple(target.org);
	}
	else
		bp->Bit_Write_0();

	DWORD sz=target.GetBufSize();
	bp->Data_WriteData(target.buf,sz);
}

inline void ReadSkillTarget(CBitPacket *bp,LevelSkillTarget &target)
{
	target.tp=(LevelSkillTarget::Type)bp->Bits_Read(3);

	if (bp->Bit_Read())
	{
		target.bOrg=1;
		target.org=bp->Data_ReadSimple<LevelPos>();
	}
	else
		target.bOrg=0;

	DWORD sz=target.GetBufSize();
	bp->Data_ReadData(target.buf,sz);
}

CLevelOp *NewLevelOp(LevelOpDesc &desc)
{
	CLevelOp *op=NewLevelOp(desc.uid);
	if (op)
		op->GetDesc()=desc;

	return op;
}

void WriteStrike(CBitPacket *bp,LevelStrike &strike)
{
	strike.Save(bp);
}

void ReadStrike(CBitPacket *bp,LevelStrike &strike)
{
	strike.Load(bp);
}


//////////////////////////////////////////////////////////////////////////
//LevelOpDesc
void LevelOpDesc::Save(CBitPacket *bp)
{
	bp->Bits_Write(uid,6);
	bp->Bits_Write(tpOwner,2);
	bp->Data_WriteSimple(link.id);
	if (link.iSerial==LevelOpLinkSerial_Invalid)
		bp->Bit_Write_0();
	else
	{
		bp->Bit_Write_1();
		bp->Data_WriteSimple(link.iSerial);
	}
	if (link.t==ANIMTICK_INFINITE)
		bp->Bit_Write_0();
	else
	{
		bp->Bit_Write_1();
		bp->Data_EncodeDword(link.t);
	}
	if (tpOwner!=LevelOpDesc::None)
	{
		if (tpOwner==LevelOpDesc::Obj)
			bp->Data_WriteSimple(idOwner);
		else
			bp->Data_WriteSimple((WORD)idOwner);
	}
}

void LevelOpDesc::Load(CBitPacket *bp)
{
	uid=(WORD)bp->Bits_Read(6);
	tpOwner=(WORD)bp->Bits_Read(2);
	bp->Data_ReadSimple(link.id);
	if (bp->Bit_Read())
		bp->Data_ReadSimple(link.iSerial);
	if (bp->Bit_Read())
		link.t=bp->Data_DecodeDword();
	if (tpOwner!=LevelOpDesc::None)
	{
		if (tpOwner==LevelOpDesc::Obj)
			bp->Data_ReadSimple(idOwner);
		else
			bp->Data_ReadSimple((WORD&)idOwner);
	}
}

////////////////////////////////////////////////////////////////////////
//LavMod
void LavMod::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(ver);
	bp->Data_WriteSimple(delta);
	bp->Bit_Write(bInstant);
	bp->Bit_Write(bModMax);
}

void LavMod::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(ver);
	bp->Data_ReadSimple(delta);
	bInstant=bp->Bit_Read();
	bModMax=bp->Bit_Read();
}


//////////////////////////////////////////////////////////////////////////
//LevelOp_StartSkill

void LevelOp_StartSkill::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(idRec);
	WriteSkillTarget(bp,target);
	bp->Data_WriteSimple(grd);
	arg.Save(bp);
	bp->Data_WriteSimple(idClient);
}

void LevelOp_StartSkill::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(idRec);
	ReadSkillTarget(bp,target);
	bp->Data_ReadSimple(grd);
	arg.Load(bp);
	bp->Data_ReadSimple(idClient);
}


//////////////////////////////////////////////////////////////////////////
//LevelOp_HPDamage
void LevelOp_HPMod::Save(CBitPacket *bp)
{
	mod.Save(bp);
	strike.Save(bp);
}

void LevelOp_HPMod::Load(CBitPacket *bp)
{
	mod.Load(bp);
	strike.Load(bp);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_AddBuff
void LevelOp_AddBuff::Save(CBitPacket *bp)
{
	data.Save(bp);
}

void LevelOp_AddBuff::Load(CBitPacket *bp)
{
	data.Load(bp);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_ReplaceBuff
void LevelOp_ModBuff::Save(CBitPacket *bp)
{
	assert(removes.size()<256);
	bp->Data_NextByte()=(BYTE)removes.size();
	bp->Data_WriteData(removes.data(),removes.size()*sizeof(LevelBuffID));
	bp->Bit_Write(!data.IsEmpty());
	if (!data.IsEmpty())
		data.Save(bp);
}

void LevelOp_ModBuff::Load(CBitPacket *bp)
{
	removes.resize(bp->Data_NextByte());
	bp->Data_ReadData(removes.data(),removes.size()*sizeof(LevelBuffID));
	if (bp->Bit_Read())
		data.Load(bp);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_SkillTeleport
void LevelOp_SkillTeleport::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(id);
	bp->Data_WriteSimple(target);
	bp->Data_WriteSimple(face);
	bp->Data_WriteSimple(dur);
	bp->Data_WriteSimple(flag);
}

void LevelOp_SkillTeleport::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(id);
	bp->Data_ReadSimple(target);
	bp->Data_ReadSimple(face);
	bp->Data_ReadSimple(dur);
	bp->Data_ReadSimple(flag);
}


//////////////////////////////////////////////////////////////////////////
//LevelOp_SPMod
void LevelOp_SPMod::Save(CBitPacket *bp)
{
	mod.Save(bp);
}

void LevelOp_SPMod::Load(CBitPacket *bp)
{
	mod.Load(bp);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_FullSPMod
void LevelOp_FullSPMod::Save(CBitPacket *bp)
{
	mod.Save(bp);
}

void LevelOp_FullSPMod::Load(CBitPacket *bp)
{
	mod.Load(bp);
}


//////////////////////////////////////////////////////////////////////////
//ExpressedEquips
void ExprEquips::Write(CBitPacket *bp)
{
	for (int i=0;i<ARRAY_SIZE(items);i++)
	{
		if (items[i])
		{
			bp->Bits_Write((DWORD)i,5);
			bp->Data_WriteSimple(items[i]);
		}
	}
	bp->Bits_Write((DWORD)EquipPart_MaxExpress,5);

	if (wpnActive==EquipPart_Invalid)
		bp->Bit_Write(0);
	else
	{
		bp->Bit_Write(1);
		bp->Bits_Write((DWORD)wpnActive,5);
	}

}

void ExprEquips::Read(CBitPacket *bp)
{
	while(1)
	{
		DWORD part;
		part=bp->Bits_Read(5);
		if (part==(DWORD)EquipPart_MaxExpress)
			break;
		RecordID id;
		bp->Data_ReadSimple(id);

		if (part<ARRAY_SIZE(items))
			items[part]=id;
	}

	if (bp->Bit_Read())
		wpnActive=(EquipPart)bp->Bits_Read(5);
	else
		wpnActive=EquipPart_Invalid;
}

BOOL ExprEquips::CheckShieldActive(CLevelRecords *records)
{
	if (items[EquipPart_Shield]==RecordID_Invalid)
		return FALSE;

	if (CheckMagicItemActive())
		return TRUE;

	if (wpnActive!=EquipPart_Invalid)
	{
		LevelRecordPosture *recPosture=records->GetPostureOfItem(items[wpnActive]);
		if (recPosture)
		{
			switch(recPosture->tp)
			{
				case LevelPosture_ShortWpn:
				case LevelPosture_LongWpn:
				//XXXXX:more LevelPostureType
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL ExprEquips::CheckMagicItemActive()
{
	if (items[EquipPart_MagicItem]!=RecordID_Invalid)
		return TRUE;
	return FALSE;
}




//////////////////////////////////////////////////////////////////////////
//LevelOp_ExpEquipMod

void LevelOp_ExprEquip::Save(CBitPacket *bp)
{
	equips.Write(bp);
}

void LevelOp_ExprEquip::Load(CBitPacket *bp)
{
	equips.Read(bp);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_FixPos
void LevelOp_FixPosEuler::Save(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(pos);
	bp->Data_WriteSimple(euler);
}

void LevelOp_FixPosEuler::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(pos);
	bp->Data_ReadSimple(euler);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_CancelReside
void LevelOp_CancelReside::Save(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(pos);
	bp->Data_WriteSimple(idTeleport);
	bp->Data_WriteSimple(idBroken);
}

void LevelOp_CancelReside::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(pos);
	bp->Data_ReadSimple(idTeleport);
	bp->Data_ReadSimple(idBroken);
}

////////////////////////////////////////////////////////////////////////
//LevelOp_ModBuffDur
void LevelOp_ModBuffDur::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(durNew);
}

void LevelOp_ModBuffDur::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(durNew);
}

////////////////////////////////////////////////////////////////////////
//LevelOp_ItemBirth

void LevelOp_ItemBirth::Save(CBitPacket *bp)
{
}

void LevelOp_ItemBirth::Load(CBitPacket *bp)
{
}


////////////////////////////////////////////////////////////////////////
//LevelOp_ResouceMod
void LevelOp_ResouceMod::Save(CBitPacket *bp)
{
	bp->Bits_Write(tpRes,3);//XXXXX:More LevelResourceType
	mod.Save(bp);
}
void LevelOp_ResouceMod::Load(CBitPacket *bp)
{
	tpRes=(LevelResourceType)bp->Bits_Read(3);//XXXXX:More LevelResourceType
	mod.Load(bp);
}

////////////////////////////////////////////////////////////////////////
//LevelOp_Revive

void LevelOp_Revive::Save(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(posRevive);
	bp->Data_WriteSimpleR(idTeleport);
}

void LevelOp_Revive::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(posRevive);
	bp->Data_ReadSimple(idTeleport);
}


////////////////////////////////////////////////////////////////////////
//LevelOp_Miss
void LevelOp_Miss::Save(CBitPacket *bp)
{
	bp->Bits_Write(reason,3);
}

void LevelOp_Miss::Load(CBitPacket *bp)
{
	reason=(BYTE)bp->Bits_Read(3);
}


////////////////////////////////////////////////////////////////////////
//LevelOp_EoBirth

void LevelOp_EoBirth::Save(CBitPacket *bp)
{
	bp->Data_NextDword()=tOwnerSkillCastTime;
}

void LevelOp_EoBirth::Load(CBitPacket *bp)
{
	tOwnerSkillCastTime=bp->Data_NextDword();
}

////////////////////////////////////////////////////////////////////////
//LevelOp_GradeMod

void LevelOp_GradeMod::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(grd);
}

void LevelOp_GradeMod::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(grd);
}



////////////////////////////////////////////////////////////////////////
//LevelOp_SyncBuffData
void LevelOp_SyncBuffData::Save(CBitPacket *bp)
{
	bp->Bits_Write(szData,5);
	bp->Data_WriteData(data,szData);
	bp->Bits_Write(szBitsData,5);
	bp->Data_WriteData(bits,szBitsData);
}

void LevelOp_SyncBuffData::Load(CBitPacket *bp)
{
	szData=(BYTE)bp->Bits_Read(5);
	bp->Data_ReadData(data,szData);
	szBitsData=(BYTE)bp->Bits_Read(5);
	bp->Data_ReadData(bits,szBitsData);
}



//////////////////////////////////////////////////////////////////////////
//LevelOp_StartSkill

void LevelOp_CombineSkill::Save(CBitPacket *bp)
{
	WriteSkillTarget(bp,target);
}

void LevelOp_CombineSkill::Load(CBitPacket *bp)
{
	ReadSkillTarget(bp,target);
}


////////////////////////////////////////////////////////////////////////
//LevelOp_SyncSkillData
void LevelOp_SyncSkillData::Save(CBitPacket *bp)
{
	bp->Bits_Write(szData,5);
	bp->Data_WriteData(data,szData);
	bp->Bits_Write(szBitsData,5);
	bp->Data_WriteData(bits,szBitsData);
}

void LevelOp_SyncSkillData::Load(CBitPacket *bp)
{
	szData=(BYTE)bp->Bits_Read(5);
	bp->Data_ReadData(data,szData);
	szBitsData=(BYTE)bp->Bits_Read(5);
	bp->Data_ReadData(bits,szBitsData);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_CancelMount
void LevelOp_CancelMount::Save(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(pos);
	bp->Data_WriteSimple(idTeleport);
	bp->Data_WriteSimple(idBroken);
}

void LevelOp_CancelMount::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(pos);
	bp->Data_ReadSimple(idTeleport);
	bp->Data_ReadSimple(idBroken);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_CancelSkill
void LevelOp_CancelSkill::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(idSkill);
	if (bStopAct)
		bp->Bit_Write_1();
	else
		bp->Bit_Write_0();
}

void LevelOp_CancelSkill::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(idSkill);
	bStopAct=bp->Bit_Read();
}


////////////////////////////////////////////////////////////////////////
//LevelOp_MBResouceMod
void LevelOp_MBResouceMod::Save(CBitPacket *bp)
{
	bp->Bits_Write(tpRes,3);
	mod.Save(bp);
}
void LevelOp_MBResouceMod::Load(CBitPacket *bp)
{
	tpRes=(MBResourceType)bp->Bits_Read(3);
	mod.Load(bp);
}


////////////////////////////////////////////////////////////////////////
//LevelOp_Path
void LevelOp_Path::Save(CBitPacket *bp)
{
	ksPos.Save(*bp->GetDP());
	ksFace.Save(*bp->GetDP());
	bp->Data_WriteSimple(dur);
}
void LevelOp_Path::Load(CBitPacket *bp)
{
	ksPos.Load_(*bp->GetDP());
	ksFace.Load_(*bp->GetDP());
	bp->Data_ReadSimple(dur);
}


////////////////////////////////////////////////////////////////////////
//LevelOp_SpeedMod
void LevelOp_SpeedMod::Save(CBitPacket *bp)
{
	bp->Bits_Write(ims,9);
	bp->Bits_Write(ias,9);
}

void LevelOp_SpeedMod::Load(CBitPacket *bp)
{
	ims=(short)bp->Bits_Read(9);
	ias=(short)bp->Bits_Read(9);
}


////////////////////////////////////////////////////////////////////////
//LevelOp_SpeedMod
void LevelOp_ShapeMod::Save(CBitPacket *bp)
{
	bp->Bits_Write(op,2);
	if (op==2)//Set name
		bp->Data_WriteSimple(nm);
}

void LevelOp_ShapeMod::Load(CBitPacket *bp)
{
	op=bp->Bits_Read(2);
	if (op==2)//Set name
		bp->Data_ReadSimple(nm);
}

//////////////////////////////////////////////////////////////////////////
//LevelDmgAbort
void LevelDmgAbort::Save(CBitPacket *bp)
{
	bp->Bits_Write(tp,2);
	if (tp==ShieldMask)
		strike.Save(bp);
	if (tp==ShieldAmulet)
		bp->Data_WriteSimple(idEo);
}

void LevelDmgAbort::Load(CBitPacket *bp)
{
	tp=(Type)bp->Bits_Read(2);
	if (tp==ShieldMask)
		strike.Load(bp);
	if (tp==ShieldAmulet)
		bp->Data_ReadSimple(idEo);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_HonorMod
void LevelOp_HonorMod::Save(CBitPacket *bp)
{
	bp->Data_EncodeDword(hnr);
}

void LevelOp_HonorMod::Load(CBitPacket *bp)
{
	hnr=bp->Data_DecodeDword();
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_VitaMod
void LevelOp_VitaMod::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(delta);
	if (delta>0)
		bp->Data_WriteSimple(idSrcOwner);
}

void LevelOp_VitaMod::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(delta);
	if (delta>0)
		bp->Data_ReadSimple(idSrcOwner);

}

//////////////////////////////////////////////////////////////////////////
//LevelOp_WormMod
void LevelOp_WormMod::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(delta);
}

void LevelOp_WormMod::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(delta);
}


//////////////////////////////////////////////////////////////////////////
//LevelOp_ChainedHammer
void LevelOp_ChainedHammer::Save(CBitPacket *bp)
{
	bp->Bits_Write(op,3);
	bp->Data_WriteSimple(dur);
}

void LevelOp_ChainedHammer::Load(CBitPacket *bp)
{
	op=(Op)bp->Bits_Read(3);
	bp->Data_ReadSimple(dur);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_Spore
void LevelOp_Spore::Save(CBitPacket *bp)
{
	bp->Bits_Write(op,2);
	bp->Data_WriteSimple(handle);
	if (op==LevelOp_Spore::Spawn)
		bp->Data_WriteSimpleR(pos);
}

void LevelOp_Spore::Load(CBitPacket *bp)
{
	op=(Op)bp->Bits_Read(2);
	bp->Data_ReadSimple(handle);
	if (op==LevelOp_Spore::Spawn)
		bp->Data_ReadSimple(pos);
}


//////////////////////////////////////////////////////////////////////////
//LevelOp_FireFly
void LevelOp_FireFly::Save(CBitPacket *bp)
{
	bp->Bits_Write(op,2);
	if (op==LevelOp_FireFly::StartFlee)
	{
		bp->Bits_Write(nPos,5);
		bp->Data_WriteData(PosBuf(),sizeof(PosBuf()[0])*nPos);
	}
	if (op==LevelOp_FireFly::EnterTorch)
	{
		bp->Bits_Write(nPos,5);
		bp->Data_WriteData(Pos3DBuf(),sizeof(Pos3DBuf()[0])*nPos);
		bp->Data_WriteSimple(idTorch);
	}
	if (op==LevelOp_FireFly::LeaveTorch)
	{
		bp->Bits_Write(nPos,5);
		bp->Data_WriteData(Pos3DBuf(),sizeof(Pos3DBuf()[0])*nPos);
		bp->Data_WriteSimple(idPlayer);
	}
}

void LevelOp_FireFly::Load(CBitPacket *bp)
{
	op=(Op)bp->Bits_Read(2);
	if (op==LevelOp_FireFly::StartFlee)
	{
		nPos=bp->Bits_Read(5);
		bp->Data_ReadData(PosBuf(),sizeof(PosBuf()[0])*nPos);
	}
	if (op==LevelOp_FireFly::EnterTorch)
	{
		nPos=bp->Bits_Read(5);
		bp->Data_ReadData(Pos3DBuf(),sizeof(Pos3DBuf()[0])*nPos);
		bp->Data_ReadSimple(idTorch);
	}

	if (op==LevelOp_FireFly::LeaveTorch)
	{
		nPos=bp->Bits_Read(5);
		bp->Data_ReadData(Pos3DBuf(),sizeof(Pos3DBuf()[0])*nPos);
		bp->Data_ReadSimple(idPlayer);
	}


}

//////////////////////////////////////////////////////////////////////////
//LevelOp_TempleMod
void LevelOp_TempleMod::Save(CBitPacket *bp)
{
	bp->Bits_Write(tp,4);
	bp->Bits_Write(iAltar,7);
}

void LevelOp_TempleMod::Load(CBitPacket *bp)
{
	tp=(LevelTempleType)bp->Bits_Read(4);
	iAltar=(BYTE)bp->Bits_Read(7);
}


//////////////////////////////////////////////////////////////////////////
//LevelOp_StrengthMod
void LevelOp_StrengthMod::Save(CBitPacket *bp)
{
	bp->Data_NextWord()=str;
}

void LevelOp_StrengthMod::Load(CBitPacket *bp)
{
	str=bp->Data_NextWord();
}


//////////////////////////////////////////////////////////////////////////
//LevelOp_EnableBody
void LevelOp_EnableBody::Save(CBitPacket *bp)
{
	bp->Bit_Write(bEnable);
}

void LevelOp_EnableBody::Load(CBitPacket *bp)
{
	bEnable=bp->Bit_Read();
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_MagicMod
void LevelOp_MagicMod::Save(CBitPacket *bp)
{
	bp->Data_NextWord()=magic;
}

void LevelOp_MagicMod::Load(CBitPacket *bp)
{
	magic=bp->Data_NextWord();
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_PainMod
void LevelOp_PainMod::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(pain);
	bp->Data_WriteSimple(tServer);
}

void LevelOp_PainMod::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(pain);
	bp->Data_ReadSimple(tServer);
}

//////////////////////////////////////////////////////////////////////////
//LevelOp_Dummy
void LevelOp_Dummy::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(t);
}
void LevelOp_Dummy::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(t);
}

