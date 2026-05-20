/********************************************************************
	created:	2013/3/25 
	author:		cxi
	
	purpose:	Level Strike
*********************************************************************/
#include "stdh.h"

#include "LevelStrike.h"
#include "datapacket/BitPacket.h"

void LevelStrike::Save(CBitPacket *bp)
{
// 	bp->Bits_Write(tp,5);
// 	if (tp!=Attack_None)
// 	{
		bp->Bits_Write(maskDmg,16);	//XXXXX: More DamageAttrType

		bp->Bits_Write(str,7);
		bp->Bits_Write(angle_,9);
		if (idSrc==LevelObjID_Invalid)
			bp->Bit_Write_0();
		else
		{
			bp->Bit_Write_1();
			bp->Data_WriteSimple(idSrc);
		}
		bp->Bits_Write(bInstant,1);
// 	}
}

void LevelStrike::Load(CBitPacket *bp)
{
// 	tp=(LevelAttackType)bp->Bits_Read(5);
// 	if (tp!=Attack_None)
// 	{
		maskDmg=(BYTE)bp->Bits_Read(16); //XXXXX: More DamageAttrType
		str=(BYTE)bp->Bits_Read(7);
		angle_=(WORD)bp->Bits_Read(9);
		if (bp->Bit_Read())
			bp->Data_ReadSimple(idSrc);
		bInstant=(BYTE)bp->Bits_Read(1);
// 	}
}

void LevelStrike::SetDir(float x,float y)
{
	float r=atan2f(y,x);
	r=i_math::wrap_radian(r);

	angle_=(WORD)(r*i_math::GRAD_PI);
}

void LevelStrike::SetDir(i_math::vector2df &dir)
{
	SetDir(dir.x,dir.y);
}

i_math::vector2df LevelStrike::GetDir()
{
	float r=((float)angle_)*(float)i_math::GRAD_PI2;
	return i_math::vector2df(cosf(r),sinf(r));
}

LevelFace LevelStrike::GetFace()
{
	return ((float)angle_)*(float)i_math::GRAD_PI2;
}

void LevelStrike::SetStr(float str_)
{
	str=(BYTE)i_math::clamp_f(str_*10.0f,0.0f,100.0f);
}

float LevelStrike::GetStr()
{
	return ((float)str)/10.0f;
}
