#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_ShootBullet 35



struct EoParamShootBullet:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamShootBullet);

	enum DirMode
	{
		OwnerPosToTargetPos,
		ShootPosToTargetPos,
		OriginalDir,
		ShootPosToTargetPosKeepPitch,
		ShootPosToAimPosKeepPitch,

		ForceDword=0xffffffff,
	};

	enum DirRandomMode
	{
		DirRandomMode_NoRandom, 
		DirRandomMode_2D,
		DirRandomMode_3D,
		DirRandomMode_InEZone,

		DirRandomMode_ForceDword=0xffffffff,
	};

	BEGIN_GOBJ_PURE(EoParamShootBullet,1);

		GELEM_VAR_INIT(DirMode,modeDir,OwnerPosToTargetPos);
		GELEM_EDITVAR("计算方向方式",GVT_U,GSem(GSem_Interger,
			"原始方向:2,Owner脚下位置到目标脚下位置:0,发射点位置到目标脚下位置:1,发射点位置到目标脚下位置(保持Pitch):3,发射点位置到目标瞄准位置(保持Pitch):4"),"如何计算发射方向");
		GELEM_VAR_INIT(DirRandomMode,modeDirRandom,DirRandomMode_NoRandom);
			GELEM_EDITVAR("方向随机模式",GVT_S,GSem(GSem_Interger,
				"不随机:0"		"|随机角度范围&随机子弹个数,"
				"事件区域内随机:3" "|随机角度范围,"
				"2D:1"				","
				"3D:2"				""
				),"方向随机模式");
		GELEM_VAR_INIT(float,angleDirOff,30.0f);
			GELEM_EDITVAR("随机角度范围",GVT_F,GSem(GSem_Float,"0.0,180.0,0.05"),"角度随机范围");
		GELEM_VAR_INIT(int,countRandom,1);
			GELEM_EDITVAR("随机子弹个数",GVT_S,GSem_Interger,"子弹个数");
	END_GOBJ();

	DirMode modeDir;
	DirRandomMode modeDirRandom;
	float angleDirOff;
	int countRandom;

};



class EoShootBullet:public CLoEffectObj
{
public:
	EoShootBullet()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoShootBullet,CLASSUID_ShootBullet);

	virtual const char *GetShowName()	{		return "发射子弹";	}
	BOOL _NeedOps() override	{		return TRUE;	}

protected:
	void _OnPostCreate() override;	
	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;

	void _OnUpdate() override;


};
