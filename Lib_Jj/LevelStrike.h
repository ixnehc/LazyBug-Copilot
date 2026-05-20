#pragma once

#include "class/class.h"

#include "LevelDefines.h"



//记录一次击打的所有信息
struct LevelStrike
{
	LevelStrike()
	{
// 		tp=Attack_None;
		maskDmg=0;
		str=0;
		angle_=255;
		idSrc=LevelObjID_Invalid;
		bInstant=1;
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);

	void SetDir(i_math::vector2df &dir);
	void SetDir(float x,float y);

	void SetStr(float str);
	float GetStr();

	i_math::vector2df GetDir();
	LevelFace GetFace();

	BOOL IsInstant()	{		return bInstant;	}


	WORD maskDmg;//a DamageAttrMask value//XXXXX: More DamageAttrType
	BYTE str;//强度,0..100
	LevelObjID idSrc;//来自于谁,如果这个id为有效值,则根据这个idSrc和目标对象的位置来计算strike方向,而不使用dir
	WORD angle_;//朝向,0..360,255表示没有方向
	BYTE bInstant;//这个strike造成的数值变化是立即表现,还是慢慢过渡表现
};

