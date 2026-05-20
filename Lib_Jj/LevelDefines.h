#pragma once

#include "class/class.h"

#include "anim/animdefines.h"
#include "strlib/strlibdefines.h"

#include "records/recordsdefine.h"

#define AssertAliveObj(pObj) assert(pObj);assert((pObj)->IsAlive());


typedef DWORD LevelObjID;
#define LevelObjID_Invalid ((LevelObjID)(0))

#define VerifyLevelObjAlive(lo)					\
if (lo)															\
{																	\
	if (!lo->IsAlive())										\
		SAFE_RELEASE(lo);								\
}

enum LevelObjType
{
	LevelObjType_None,
	LevelObjType_Agent,
	LevelObjType_Unit,
	LevelObjType_Item,
	LevelObjType_Eo,
};

//外形模拟方式
enum LevelObjShapeType
{
	LevelObjShape_SingleCircle,//用一个圆模拟
	LevelObjShape_MultiCircle,//用多个圆模拟
};

enum PlayerClass
{
	PlayerClass_None=0,
	PlayerClass_Hunter,
	PlayerClass_Magician,
	PlayerClass_Knight,
};


typedef DWORD LevelGUID;//GUID
#define LevelGUID_Invalid ((LevelGUID)0)

typedef WORD LevelPlayerMask;

typedef BYTE LevelPlayerID;
#define LevelPlayerID_Invalid (0xff)

#define LevelPlayerID_Wild (15)
#define LevelPlayerID_NeutralWild (14)
#define LevelPlayerID_PlayerWild (13) //wild,but ally for all the players

typedef double AnimSecond;
#define ANIMTICK_FROM_SECOND(s) (AnimTick)(((float)ANIMTICK_PER_SECOND)*(float)(s))

typedef double ServerSecond;

//Level的时间
typedef AnimTick LevelTick;

#define LevelFatalError() 

#define MAX_ACCOUNT_NAME (16)
#define MAX_ACCOUNTPASSWORD_NAME (16)

#define MAX_PLAYER_NAME (16) //角色名字最大的字符个数

#define MAX_LEVEL_NAME (16) //Level名字的最大字符个数


#define LEVEL_MAX_PLAYER (8)

//Aov代表area of view,视野
#define LEVEL_AOVMAP_BLOCKLEN (8.0f) //单位为米
#define LEVEL_AOV_RADIUS (64.0f) //单位为米
typedef i_math::pos2di AovCenter;
#define AovCenter_Invalid (i_math::pos2di(-10000,-10000))

//Aoa代表area of activating,激活区域
#define LEVEL_AOA_BLOCKLEN (32.0f) //单位为米
#define LEVEL_AOA_RADIUS (96.0f) //单位为米
typedef i_math::pos2di AoaCenter;
#define AoaCenter_Invalid (i_math::pos2di(-10000,-10000))


#define ClassF_LevelObjSrc 2
#define ClassF_LevelObj 4
#define ClassF_LevelItem 8
#define ClassF_LevelOp 16
#define ClassF_LevelBuff 32
#define ClassF_LevelSkill 64
#define ClassF_LevelObjParam 128
#define ClassF_LevelGesture 256

//LevelPos以米为单位
typedef i_math::vector2df LevelPos;
#define LevelPos_Invalid (LevelPos(-10000.0f,-10000.0f))

typedef i_math::vector3df LevelPos3D;
#define LevelPos3D_Invalid (LevelPos3D(-10000.0f,-10000.0f,-10000.0f))

#define LevelPos3DFrom2D(pos3D__,pos2D__,gtm__)												\
	(pos3D__).set((pos2D__).x,gtm__->GetHeight((pos2D__).x,(pos2D__).y),(pos2D__).y);


//LevelPosInt以cm为单位,Int代表Interger
typedef i_math::pos2di LevelPosInt;
#define LevelPosInt_Invalid (LevelPosInt(-10000000,-10000000))

inline LevelPosInt LevelPosToInt(LevelPos pos)
{
	LevelPosInt t;
	t.x=(int)floor(pos.x*32.0f);
	t.y=(int)floor(pos.y*32.0f);
	return t;
}

inline LevelPosInt LevelPosToInt(float x,float y)
{
	LevelPosInt t;
	t.x=(int)floor(x*32.0f);
	t.y=(int)floor(y*32.0f);
	return t;
}

inline LevelPos LevelPosFromInt(LevelPosInt pos)
{
	LevelPos t;
	t.x=(1.0f/32.0f)*(float)pos.x;
	t.y=(1.0f/32.0f)*(float)pos.y;
	return t;
}

typedef float LevelFace;
typedef float LevelFaceYaw;//向左为负,向右为正
typedef BYTE LevelFaceInt;
inline LevelFaceInt LevelFaceToInt(float face)
{
	face=i_math::wrap_radian(face);
	return (LevelFaceInt)i_math::clamp_i((int)(face/(i_math::Pi*2.0f)*255.0f),0,255);
}

inline float LevelFaceFromInt(LevelFaceInt face)
{
	return ((float)face)/255.0f*i_math::Pi*2.0f;
}

inline float LevelFaceToEuler(LevelFace face)
{
	return i_math::Pi/2.0f-face;
	i_math::vector3df euler;
	euler.x=cosf(face);
	euler.z=sinf(face);
	euler.toEuler();
	return euler.x;
}
inline LevelFace LevelFaceFromEuler(float xEuler)
{
	return i_math::Pi/2.0f-xEuler;
}
inline void LevelFaceToQuat(LevelFace face,i_math::quatf &rot)
{
	i_math::vector3df euler;
	euler.x=LevelFaceToEuler(face);
	rot.fromEuler(euler);
}
inline LevelFace LevelFaceFromQuat(i_math::quatf &rot)
{
	i_math::vector3df euler;
	rot.toEuler(euler);
	return LevelFaceFromEuler(euler.x);
}

inline LevelPos LevelFaceToDir(LevelFace face)
{
	return LevelPos(cosf(face),sinf(face));
}

inline LevelPos3D LevelFaceToDir3D(LevelFace face)
{
	return LevelPos3D(cosf(face),0.0f,sinf(face));
}


inline LevelFace LevelFaceFromDir(LevelPos &dir)
{
	return atan2f(dir.y,dir.x);
}

inline LevelFace LevelFaceFromDir_XZ(LevelPos3D &dir)
{
	return atan2f(dir.z,dir.x);
}


inline LevelFace LevelFaceLerp(LevelFace faceFrom,LevelFace faceTo,float r)
{
	float gap=i_math::get_radian_dist(faceFrom,faceTo);
	gap*=r;
	float face=faceFrom;
	i_math::rotate_limited(face,faceTo,gap);
	return face;
}

 
inline void LevelFaceApplyYaw(LevelFace &face,LevelFaceYaw yaw)
{
	face+=-yaw;
}

//返回的yaw, apply到faceBase上得到faceTarget
//返回值(-Pi,+Pi)
inline LevelFaceYaw LevelFaceCalcYaw(LevelFace faceBase,LevelFace faceTarget)
{
	return normalize_radian(-(faceTarget-faceBase));
}


struct LevelXfm
{
	LevelPos pos;
	LevelFace face;
};


//LevelVUS,13 bits
#define LevelVUS_BaseMtr (-32) //in meter
#define LevelVUS_MaxMeterRange (63)//63
#define LevelVUS_VuPerMtr 128
#define LevelVUS_Base (LevelVUS_BaseMtr*LevelVUS_VuPerMtr) //in VU
#define LevelVUS_MaxVu (LevelVUS_MaxMeterRange*LevelVUS_VuPerMtr)//in VU

typedef float LevelHeight;
typedef WORD LevelHeightVu;//vertical unit

#define LevelHeightToVu(ht)							\
((LevelHeightVu)(i_math::clamp_i(FloatToNearestInt(((ht)-(float)LevelVUS_BaseMtr)*(float)LevelVUS_VuPerMtr),0,LevelVUS_MaxVu)))

#define LevelHeightFromVu(vu) (((float)(vu)/(float)LevelVUS_VuPerMtr)+(float)LevelVUS_BaseMtr)


typedef WORD LevelRandomSeed;
#define LevelRandomSeed_Invalid (0)

struct LevelObjCircle
{
	LevelPos center;
	float radius;
};

//对LevelObj的一些特定需求
enum LevelObjRequire
{
	LevelObjRequire_None,
	LevelObjRequire_Attackable,
	LevelObjRequire_Dormant,
};
#define LevelObjRequire_SemConstraint "没有需求,可攻击对象,休眠对象"




typedef DWORD LevelItemID;
#define LevelItemID_Invalid ((LevelItemID)(0))

typedef DWORD FlockTypeID;
#define FlockTypeID_Invalid (0)

//LevelOp
typedef WORD LevelOpLinkID_;
#define LevelOpLinkID_Invalid (0)

#define LevelOpLinkSerial_Invalid (0xff)

struct LevelOpLink
{
	LevelOpLink()
	{
		id=LevelOpLinkID_Invalid;
		iSerial=LevelOpLinkSerial_Invalid;
		t=ANIMTICK_INFINITE;
	}
	BOOL IsValid()
	{
		return id!=LevelOpLinkID_Invalid;
	}
	LevelOpLinkID_ id;
	BYTE iSerial;
	AnimTick t;
};

struct LevelOpDesc
{
	enum OwnerType
	{
		None=0,//没有owner,或者说,这个op的owner是这个op所属的LevelObj
		Skill=1,
		Buff=2,
		Obj=3,//另外的LevelObj
	};
	LevelOpDesc()
	{
		uid=0;
	}

	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);

	BOOL IsEmpty()	{		return uid==0;	}

	WORD uid; //根据这个uid可以创建出一个指定类型的CLevelOp对象,9bits
	LevelOpLink link;

	WORD tpOwner;//Owner是什么类型(OwnerType),2bits

	DWORD idOwner;//Owner的id,16 bits或32 bits,根据tpOwner可以是LevelBuffID或者LevelSkillID或者LevelObjID
};


//Skill

// typedef DWORD LevelSkillTypeID_;
// #define LevelSkillTypeID_Invalid (0)

#define MAX_LEVEL_SKILL_STACK (1000000)

typedef WORD LevelSkillID;
#define LevelSkillID_Invalid (0)

//Player施放技能时,Client提供的ID
typedef WORD ClientSkillID;
#define ClientSkillID_Invalid (0)

#define SkillTarget_CheckType(mask,tp) ((mask)&(1<<(tp)))

//技能等级
typedef BYTE LevelSkillGrade;
#define LevelSkillGrade_Invalid (0)

//技能快捷键的个数
#define LevelSkillMaxFast (8) 

//LevelAbility
enum LevelAbilityType
{
	LevelAbilityType_None=0,

	LevelAbilityType_First=1,

	//Weapon
	LevelAbilityType_Unarmed=LevelAbilityType_First,
	LevelAbilityType_Fire=2,
	LevelAbilityType_UtumTide=3,
	LevelAbilityType_FlashSwing=4,
	LevelAbilityType_DeathCall=5,
	LevelAbilityType_FlameBlade=6,
	LevelAbilityType_LightningBow=7,
	LevelAbilityType_HonorSword=8,
	LevelAbilityType_SkullSword=9,
	LevelAbilityType_TeleportSword=10,
	LevelAbilityType_PhantomDagger=11,
	LevelAbilityType_BloodTeeth=12,
	LevelAbilityType_ObliterateBow=13,
	LevelAbilityType_Nameless=57,

	//Armor
	LevelAbilityType_WolfSkin=14,
	LevelAbilityType_TalBless=15,
	LevelAbilityType_AnWeep=16,
	LevelAbilityType_BlackSteel=17,
	LevelAbilityType_HunterPlate=18,
	LevelAbilityType_SimCurse=19,
	LevelAbilityType_WhirlWind=20,
	LevelAbilityType_HonorPlate=21,
	LevelAbilityType_7sonLeather=22,
	LevelAbilityType_Frost=23,
	LevelAbilityType_EliPromise=24,

	//Shield
	LevelAbilityType_StormEye=49,

	//Spells
	LevelAbilityType_Spell_FireBall=31,

    //Artifacts
    LevelAbilityType_ExplodeOil=32,
	LevelAbilityType_WeaponInductionStone=33,
	LevelAbilityType_ToeStone=34,
	LevelAbilityType_SacredArrow=35,
	LevelAbilityType_Bomb=36,
	LevelAbilityType_MagicRing=37,
	LevelAbilityType_MoneyBag=38,
	LevelAbilityType_GemPot=39,
	LevelAbilityType_HPAmulet=40,
	LevelAbilityType_SPAmulet=41,
	LevelAbilityType_SPPotion=42,
	LevelAbilityType_HPPotion=43,
	LevelAbilityType_VampireRing=44,
	LevelAbilityType_ShieldMask=45,
	LevelAbilityType_Whetstone=46,
	LevelAbilityType_HonorBook=47,
	LevelAbilityType_ShieldAmulet=48,
	LevelAbilityType_HPFlusk=50,

	//Poems
	LevelAbilityType_Poem_ChartI=51,
	LevelAbilityType_Poem_ChartII=52,
	LevelAbilityType_Poem_ChartIII=53,

	//Banners
	LevelAbilityType_Banner_Fire=54,
	LevelAbilityType_Banner_Wolf=55,

	//Special
	LevelAbilityType_PushSlideway=56,
    //XXXXX:More Ability

	//注意新添加的类型一定要加在最下面

	LevelAbilityType_Max=58,//XXXXX:More Ability

	LevelAbilityType_ForceDword=0xffffffff,
};
#define LevelAbilityConstraintStr \
	"n/a:0,空手:1,(武器)火焰术:2,(武器)乌图姆潮水:3,(武器)飞速斩:4,(武器)死亡召唤:5,(武器)火焰刀:6,(武器)闪电弓:7,(武器)荣耀之剑:8,(武器)骷髅剑:9,(武器)闪动剑:10,(武器)幻影匕首:11,(武器)血牙剑:12,(武器)尸爆弓:13,(武器)无名:57"\
	"(护甲)野狼皮:14,(护甲)塔尔的护佑:15,(护甲)安的啜泣:16,(护甲)黑钢甲:17,(护甲)猎人护甲:18,(护甲)西姆的诅咒:19,(护甲)旋风甲:20,(护甲)荣耀战甲:21,(护甲)七子甲:22,(护甲)寒霜甲:23,(护甲)艾利的守护:24,"\
	"(咒语)火球术:31,"\
	"(宝物)ExplodeOil:32,(宝物)武器感应石:33,(宝物)脚尖石:34,(宝物)圣箭:35,(宝物)炸弹:36,(宝物)魔法戒指:37,(宝物)钱包:38,(宝物)宝石罐:39,(宝物)HP项链:40,(宝物)SP项链:41,(宝物)SP药水:42,(宝物)HP药水:43,"\
	"(宝物)吸血戒指:44,(宝物)魔盾面具:45,(宝物)磨刀石:46,(宝物)荣耀之书:47,(宝物)魔盾护身符:48,(宝物)生命宝瓶:50,"\
	"(盾牌)风暴之眼:49,"\
	"(圣诗卷轴)圣诗卷轴I:51,(圣诗卷轴)圣诗卷轴II:52,(圣诗卷轴)圣诗卷轴III:53,"\
	"(圣旗)火焰:54,(圣旗)生命之狼:55,"\
	"(特殊)推动滑槽:56"

//XXXXX:More Ability

#define LEVELABILITY_INFINITE_CHARGE 10000

enum LevelAbilityAction
{
	LevelAbilityAction_None=0,
	LevelAbilityAction_First=1,

	LevelAbilityAction_AttackA=LevelAbilityAction_First,
	LevelAbilityAction_AttackA_Dash,
	LevelAbilityAction_AttackA_RunningDash,
	LevelAbilityAction_AttackB,
	LevelAbilityAction_AttackB_Dash,
	LevelAbilityAction_AttackC,
	LevelAbilityAction_AttackC_Dash,
	LevelAbilityAction_AttackD,
	LevelAbilityAction_AttackD_Dash,
	LevelAbilityAction_EvadeB,
	LevelAbilityAction_FuryA,
	LevelAbilityAction_EvadeF,
	LevelAbilityAction_EvadeL,
	LevelAbilityAction_EvadeR,
	LevelAbilityAction_AttackPreB,
	LevelAbilityAction_AttackPreC,
	LevelAbilityAction_AttackPreD,
	LevelAbilityAction_TeleLeftA,
	LevelAbilityAction_TeleRightA,
	LevelAbilityAction_TeleBackA,
	LevelAbilityAction_JumpF,
	LevelAbilityAction_AttackPreAR,
	LevelAbilityAction_AttackAR,
	LevelAbilityAction_AttackAR_Dash,
	LevelAbilityAction_FuryB,
	LevelAbilityAction_FuryC,
	LevelAbilityAction_FuryD,
	LevelAbilityAction_MissileA,
	LevelAbilityAction_Guard,
	LevelAbilityAction_ShieldAttack,
	//注意新增的Action必须添加在末尾
	//XXXXX:More LevalAbilityAction

	LevelAbilityAction_Max,

	LevelAbilityAction_ForceDword=0xffffffff
};

#define LevelAbilityActionConstraintStr \
	"n/a:0,"\
	"AttackA:1,"\
	"AttackA_Dash:2,"\
	"AttackA_RunningDash:3,"\
	"AttackPreB:15,"\
	"AttackB:4,"\
	"AttackB_Dash:5,"\
	"AttackPreC:16,"\
	"AttackC:6,"\
	"AttackC_Dash:7,"\
	"AttackPreD:17,"\
	"AttackD:8,"\
	"AttackD_Dash:9,"\
	"AttackPreAR:22,"\
	"AttackAR:23,"\
	"AttackAR_Dash:24,"\
	"EvadeB:10,"\
	"EvadeF:12,"\
	"EvadeL:13,"\
	"EvadeR:14,"\
	"TeleLeftA:18,"\
	"TeleRightA:19,"\
	"TeleBackA:20,"\
	"JumpF:21,"\
	"FuryA:11,"\
	"FuryB:25,"\
	"FuryC:26,"\
	"FuryD:27,"\
	"MissileA:28,"\
	"Guard:29,"\
	"ShieldAttack:30"
//XXXXX:More LevalAbilityAction


typedef BYTE LevelAbilityGrade;

struct LevelSkillType
{
	LevelSkillType()
	{
		Zero();
	}
	LevelSkillType(RecordID idSkill_)
	{
		Zero();
		idSkill=idSkill_;
	}
	BOOL IsEmpty()
	{
		return (idSkill==RecordID_Invalid)&&(tpAbility_==LevelAbilityType_None);
	}
	void Zero()
	{
		idSkill=RecordID_Invalid;
		tpAbility_=LevelAbilityType_None;
		actionAbility=LevelAbilityAction_None;
	}
	BOOL Equals(LevelSkillType &other)
	{
		return (idSkill==other.idSkill)&&(tpAbility_==other.tpAbility_)&&(actionAbility==other.actionAbility);
	}
	RecordID idSkill;
	LevelAbilityType tpAbility_;
	LevelAbilityAction actionAbility;
};



//Fast相关
#define LEVELHOTKEYS "QWERASDFZXCV1234567890"
inline BOOL CheckLevelHotKey(char c)
{
	const char *p=LEVELHOTKEYS;
	while(*p)
	{
		if ((*p)==c)
			return TRUE;;
		p++;
	}
	return FALSE;
}

struct LevelFastTarget
{
	typedef DWORD PackID;
	LevelFastTarget()
	{
		Zero();
	}
	enum Mode
	{
		None,
		AbilityMelee,
		AbilityToggle,
		AbilityConsume,
		WeaponFury,
		ShieldGuard,
		ShootArrow,
		AbilityMissile,
	};
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsEmpty()
	{
		return mode==None;
	}
	static LevelFastTarget GetNone()
	{
		LevelFastTarget target;
		return target;
	}
	static LevelFastTarget GetWeaponFury()
	{
		LevelFastTarget target;
		target.mode=WeaponFury;
		return target;
	}
	static LevelFastTarget GetShieldGuard()
	{
		LevelFastTarget target;
		target.mode=ShieldGuard;
		return target;
	}
	PackID Pack()
	{
		return (((DWORD)mode)<<16)|(DWORD)tp;
	}
	void UnPack(PackID id)
	{
		tp=(LevelAbilityType)(id&0xffff);
		mode=(Mode )(id>>16);
	}
	BOOL Equals(LevelFastTarget &other)
	{
		if (mode!=other.mode)
			return FALSE;
		if ((mode==AbilityMelee)||
			(mode==AbilityMissile)||
			(mode==AbilityToggle)||
			(mode==AbilityConsume)||
			(mode==ShootArrow))
		{
			if (tp!=other.tp)
				return FALSE;
		}
		return TRUE;
	}
	Mode mode;
	LevelAbilityType tp;
};


struct LevelSkillTarget
{
	typedef DWORD TypeMask;
	enum Type
	{
		Target_None=0,
		Target_DefObj=1,

		Target_Pos=2,
		Target_Aim=3,
		Target_FixPosAndObj=4,//位置+对象(到某处对某对象释放技能)
		Target_Pos3D=5,
		Target_ObjPos=6,//针对对象所在位置施放技能

		ForceDword=0xffffffff,
	};

	LevelSkillTarget()
	{
		Zero();
	}
	void Zero()
	{
		tp=Target_None;
		bOrg=0;
		arg=0;
	}
	void SetOrg(LevelPos &pos)
	{
		bOrg=1;
		org=pos;
	}
	void SetObjID(LevelObjID id)
	{
		tp=Target_DefObj;
		ObjID()=id;
	}

	void SetPos(LevelPos &pos)
	{
		tp=Target_Pos;
		Pos()=pos;
	}
	void SetPos3D(LevelPos3D &pos)
	{
		tp=Target_Pos3D;
		Pos3D()=pos;
	}

	void SetAim(LevelPos &aim)
	{
		tp=Target_Aim;
		Aim()=aim;
	}
	void SetFixPosAndObj(LevelPos &pos,LevelFace face,LevelObjID id)
	{
		tp=Target_FixPosAndObj;
		Pos()=pos;
		ObjID()=id;
		Face()=face;
	}
	void SetObjPos(LevelPos &pos,LevelObjID id)
	{
		tp=Target_ObjPos;
		Pos()=pos;
		ObjID()=id;
	}
	void SetNone()
	{
		tp=Target_None;
	}
	void SetArg(DWORD arg_)
	{
		arg=arg_;
	}
	DWORD GetBufSize()
	{
		switch(tp)
		{
			case Target_None:
				return 0;
			case Target_DefObj:
				return sizeof(LevelObjID);
			case Target_FixPosAndObj:
			case Target_ObjPos:
				return sizeof(LevelPos)+sizeof(LevelObjID)+sizeof(float);
			case Target_Pos3D:
				return sizeof(LevelPos3D);
			default:
				return sizeof(LevelPos);
		}
		return 0;
	}
	BOOL Equals(LevelSkillTarget &other)
	{
		if (tp!=other.tp)
			return FALSE;
		if (arg!=other.arg)
			return FALSE;
		if (tp==Target_None)
			return TRUE;

		DWORD sz=GetBufSize();
		if (memcmp(buf,other.buf,sz)==0)
			return TRUE;
		return FALSE;
	}

	LevelObjID &ObjID()
	{
		if ((tp!=Target_FixPosAndObj)&&(tp!=Target_ObjPos))
			return *(LevelObjID *)buf;
		else
			return *(LevelObjID *)(buf+sizeof(LevelPos));
	}


	LevelPos &Pos()
	{
		return *(LevelPos*)buf;
	}

	LevelPos3D &Pos3D()
	{
		return *(LevelPos3D*)buf;
	}


	LevelPos &Aim()
	{
		return *(LevelPos*)buf;
	}

	LevelFace &Face()
	{
		return *(float *)(buf+sizeof(LevelPos)+sizeof(LevelObjID));
	}

	LevelPos org;
	BYTE buf[16];

	BYTE tp:3;
	BYTE bOrg:1;
	BYTE arg:4;//用户自定义的信息
};

enum LevelSkillTargetFacingMode
{
	LevelSkillTargetFacingMode_None,
	LevelSkillTargetFacingMode_FaceTarget,
	LevelSkillTargetFacingMode_FaceTargetFixedPos,

	LevelSkillTargetFacingMode_ForceDword=0xffffffff,
};

#define LevelSkillTargetFacingMode_SemConstraint "不变:0,朝向目标位置:1,朝向目标固定位置(用于从固定位置对目标对象施放的情况):2"



//附加的技能参数
struct LevelSkillArg
{
	DEFINE_CLASS(LevelSkillArg);

	LevelSkillArg()
	{
		Zero();
	}
	void Zero()
	{
		seedRnd=LevelRandomSeed_Invalid;
	}

	void Clear()
	{
		objs.clear();
		sites.clear();
		data.clear();
		Zero();
	}

	LevelSkillArg*Clone()
	{
		LevelSkillArg *arg=Class_New2(LevelSkillArg);
		arg->CopyFrom(this);
		return arg;
	}
	BOOL IsEmpty()
	{
		return (objs.size()==0)&&(sites.size()==0)&&(data.size()==0)&&(seedRnd==LevelRandomSeed_Invalid);
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
	void CopyFrom(LevelSkillArg* src)
	{
		if (src)
		{
			objs=src->objs;
			sites=src->sites;
			data=src->data;
			seedRnd=src->seedRnd;
		}
		else
			Clear();
	}
	std::vector<LevelObjID> objs;
	std::vector<LevelPos> sites;
	std::vector<BYTE> data;
	LevelRandomSeed seedRnd;

	//XXXXX:more LevelSkillArg members
};



enum LevelSkillState
{
	SkillState_None,
	SkillState_Fail,//施放失败
	SkillState_Casting,//技能正在施放过程中,此时不能释放另一个技能
	SkillState_Casted,//技能施放过程已经结束,但技能仍没有结束,有持续效果
	SkillState_Finished,//已经结束,可以Finish了
};

#define MAX_SKILL_DATA 128


//Buff
typedef DWORD LevelBuffTypeID_;
#define LevelBuffTypeID_Invalid (0)


typedef WORD LevelBuffID;
#define LevelBuffID_Invalid (0)

typedef unsigned __int64 LevelBuffMask;//一个LevelBuffMask可以包含多个LevelBuff类型

#define MAX_BUFF_DATA 32

// struct LevelBuffArg
// {
// 	char Char0()	{		return (char)buf[0];	}
// 	char Char1()	{		return (char)buf[1];	}
// 	char Char2()	{		return (char)buf[2];	}
// 	char Char3()	{		return (char)buf[3];	}
// 
// 	BYTE Byte0()	{		return (char)buf[0];	}
// 	BYTE Byte1()	{		return (char)buf[1];	}
// 	BYTE Byte2()	{		return (char)buf[2];	}
// 	BYTE Byte3()	{		return (char)buf[3];	}
// 
// 	WORD Word0()	{		return *(WORD*)buf.data();	}
// 	WORD Word1()	{		return *(WORD*)&buf[2];	}
// 
// 	short Short0()	{		return *(short*)buf.data();	}
// 	short Short1()	{		return *(short*)&buf[2];	}
// 
// 	int Int()	{		return *(int*)buf.data();	}
// 	float Float()	{		return *(float*)buf.data();	}
// 
// 	BYTE buf[4];
// };

struct LevelTeleportTarget
{
	enum Type
	{
		None,
		ReturnPoint,

		MaxType,
		ForceDword=0xffffffff,
	};

	LevelTeleportTarget()
	{
		idMap=RecordID_Invalid;
		nmSite=StringID_Invalid;
		tp=None;
	}
	Type tp;
	RecordID idMap;
	StringID nmSite;
	LevelPos posSite;
};

//Teleport ID
typedef WORD LevelTeleportID;
#define LevelTeleportID_Invalid ((LevelTeleportID)(0))

//时间
#define LEVEL_FRAME_INTERVAL (0.1f)
#define LEVEL_SUBFRAME_COUNT (4)
#define LEVEL_SUBFRAME_INTERVAL						(LEVEL_FRAME_INTERVAL/(float)LEVEL_SUBFRAME_COUNT)
#define LEVEL_FRAME_TICK								ANIMTICK_FROM_SECOND(LEVEL_FRAME_INTERVAL)
#define LEVEL_SUBFRAME_TICK						ANIMTICK_FROM_SECOND(LEVEL_SUBFRAME_INTERVAL)
#define LEVEL_BUFF_UPDATE_TICK LEVEL_FRAME_TICK
#define LEVEL_SKILL_UPDATE_TICK LEVEL_SUBFRAME_TICK
#define LEVEL_BUFF_UPDATE_INTERVAL LEVEL_FRAME_INTERVAL
#define LEVEL_SKILL_UPDATE_INTERVAL LEVEL_SUBFRAME_INTERVAL

//LevelRelation
enum LevelRelation
{
	LevelRelation_Native,//自己人
	LevelRelation_Ally,//友军
	LevelRelation_Enemy,//敌军
	LevelRelation_Neutral,//中立
};

typedef DWORD LevelRelationMask; //=1<<(relation-1)
#define LevelRelationMask_None 0
#define LevelRelationMask_Native (1)
#define LevelRelationMask_Ally (2)
#define LevelRelationMask_Enemy (4)
#define LevelRelationMask_Neutral (8)


//单位等级
typedef BYTE LevelGrade;
#define LevelGradeBase_SemConstraint "n/a,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50"
#define LevelGradeVary_SemConstraint "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"

struct LevelUpgradeValue
{
	LevelUpgradeValue()
	{
		base=0.0f;
		perGrade=0.0f;
	}
	float base;
	float perGrade;
};


enum LevelMoveStage
{
	LevelMoveStage_None=0,
	LevelMoveStage_NotMove,
	LevelMoveStage_RotateOnSpot,
	LevelMoveStage_Faced,
	LevelMoveStage_Reached,//到达目标停止
	LevelMoveStage_Abort,//未到达目标停止
	LevelMoveStage_StartFw,
	LevelMoveStage_StartRot,
	LevelMoveStage_Move,
	LevelMoveStage_Stop,

	//XXXXX:More Move Stage
};

#define LevelMoveStage_BitCount (4)

enum LevelMoveMethod
{
	LevelMoveMethod_None=0,
	LevelMoveMethod_Ground=1,
	LevelMoveMethod_Resided_=2,
	LevelMoveMethod_Flying=3,
	LevelMoveMethod_Floating=4,//漂浮
	LevelMoveMethod_Mount=5,
};

typedef DWORD LevelMoveMethodMask; //=1<<(method-1)
#define LevelMoveMethodMask_None 0
#define LevelMoveMethodMask_All (0xffffffff)
#define LevelMoveMethodMask_Ground 1
#define LevelMoveMethodMask_Resided 2
#define LevelMoveMethodMask_Flying 4
#define LevelMoveMethodMask_Float 8
#define LevelMoveMethodMask_Mount 16

typedef DWORD LevelMoveSession;
#define LevelMoveSession_Invalid 0;

struct LevelMoveStep
{
	LevelMoveStep()
	{
		bReaching=FALSE;
		dist=0.0f;
		speed=0.0f;
	}
	LevelPos GetFinalPos()
	{
		return pos+dir*dist;
	}
	LevelPos pos;
	LevelPos dir;//normalized
	float dist;
	BOOL bReaching;
	float speed;//固有速度
};

enum DetectSightType
{
	DetectSightType_Me,
	DetectSightType_Owner,
	DetectSightType_Troop,
	DetectSightType_Custom,

	DetectSightType_ForceDword=0xffffffff,
};


enum EquipPart
{
	EquipPart_HeadGear=0,//头盔
	EquipPart_Shield,
	EquipPart_Weapon,
	EquipPart_Glove,
	EquipPart_Armor,
	EquipPart_Boot,
	EquipPart_Head,
	EquipPart_Accessory01,
	EquipPart_Accessory02,
	EquipPart_Accessory03,
	EquipPart_Accessory04,
	EquipPart_Weapon2nd,
	EquipPart_MagicItem,
	EquipPart_Reserve03,
	EquipPart_Reserve04,

	EquipPart_MaxExpress,//以上为所有从外观上可以看到的装备

	EquipPart_Amulet=EquipPart_MaxExpress,
	EquipPart_ReserveEx01,
	EquipPart_ReserveEx02,
	EquipPart_ReserveEx03,
	EquipPart_ReserveEx04,
	EquipPart_ReserveEx05,
	EquipPart_ReserveEx06,
	EquipPart_ReserveEx07,
	EquipPart_ReserveEx08,

	EquipPart_Max,

	EquipPart_Invalid=1000,

	//注意,如果这个enum如果有修改,意味着角色的装备的数据库数据格式变了,旧有的数据将无法再读入
};

typedef char EquipSetPick;
#define EquipSetPick_None (-1)

enum LevelItemCategory
{
	LevelItemCategory_None=0,
	LevelItemCategory_Weapon,
	LevelItemCategory_Armor,
	LevelItemCategory_Accessory,
	LevelItemCategory_Resource,//资源
	LevelItemCategory_Debris,//残片
	LevelItemCategory_Artifact,//神器

	LevelItemCategory_ForceDword=0xffffffff,
};

enum LevelArtifactType
{
	LevelArtifact_None=0,
	LevelArtifact_VesselOfUtuum=1,
	LevelArtifact_RingOfFire=2,
	LevelArtifact_SwordOfFeather=3,
	LevelArtifact_WingOfDeath=4,
	LevelArtifact_ArmorOfVigor=5,
	LevelArtifact_StaffOfElf=6,
	LevelArtifact_ShieldOfVigor=7,
	LevelArtifact_FlameBlade=8,
	LevelArtifact_LightningBow=9,
	LevelArtifact_HonorSword=10,
	LevelArtifact_SkullSword=11,
	LevelArtifact_TeleportSword=12,
	LevelArtifact_PhantomDagger=13,
	LevelArtifact_BloodTeeth=14,
	LevelArtifact_ObliterateBow=15,
	LevelArtifact_Nameless=86,

	LevelArtifact_WolfSkin=16,
	LevelArtifact_TalBless=17,
	LevelArtifact_AnWeep=18,
	LevelArtifact_BlackSteel=19,
	LevelArtifact_HunterPlate=20,
	LevelArtifact_SimCurse=21,
	LevelArtifact_WhirlWind=22,
	LevelArtifact_HonorPlate=23,
	LevelArtifact_7sonLeather=24,
	LevelArtifact_Frost=25,
	LevelArtifact_EliPromise=26,

    LevelArtifact_ExplodeOil=27,
    LevelArtifact_WeaponInductionStone_A=28,
	LevelArtifact_WeaponInductionStone_B=29,
	LevelArtifact_WeaponInductionStone_C=30,
	LevelArtifact_ToeStone=31,
	LevelArtifact_StrenghPotion=32,
	LevelArtifact_MagicPowerPotion=33,
	LevelArtifact_HunterRing=34,
	LevelArtifact_SoulStone=35,
	LevelArtifact_SacredArrow=36,
	LevelArtifact_Bomb=37,
	LevelArtifact_EyeOfQueen=38,
	LevelArtifact_CrystalHeart=39,
	LevelArtifact_MagicRing=40,
	LevelArtifact_MoneyBag=41,
	LevelArtifact_GemPot=42,
	LevelArtifact_HPAmulet=43,
	LevelArtifact_SPAmulet=44,
	LevelArtifact_HPPotion=45,
	LevelArtifact_SPPotion=46,
	LevelArtifact_VampireRing=47,
	LevelArtifact_ShieldMask=48,
	LevelArtifact_Whetstone=49,
	LevelArtifact_CrystalEye=50,
	LevelArtifact_HonorBook=51,
	LevelArtifact_SkeletonKingCrown=52,
	LevelArtifact_ShieldAmulet=53,

	LevelArtifact_StormEye=54,
	LevelArtifact_HPFlusk=55,

	LevelArtifact_Poem_ChartI=56,
	LevelArtifact_Poem_ChartII=57,
	LevelArtifact_Poem_ChartIII=58,
	LevelArtifact_Poem_ChartIV_Reserved=59,
	LevelArtifact_Poem_ChartV_Reserved=60,
	LevelArtifact_Poem_ChartVI_Reserved=61,
	LevelArtifact_Poem_ChartVII_Reserved=62,
	LevelArtifact_Poem_ChartVIII_Reserved=63,
	LevelArtifact_Poem_ChartX_Reserved=64,
	LevelArtifact_Poem_ChartXI_Reserved=65,
	LevelArtifact_Poem_ChartXII_Reserved=66,
	LevelArtifact_Poem_ChartXIII_Reserved=67,

	LevelArtifact_Rune_A=68,
	LevelArtifact_Rune_B=69,
	LevelArtifact_Rune_C=70,
	LevelArtifact_Rune_D=71,
	LevelArtifact_Rune_E=72,
	LevelArtifact_Rune_F=73,
	LevelArtifact_Rune_G=74,
	LevelArtifact_Rune_H=75,
	LevelArtifact_Rune_I=76,
	LevelArtifact_Rune_J_Reserved=77,
	LevelArtifact_Rune_K_Reserved=78,
	LevelArtifact_Rune_L_Reserved=79,
	LevelArtifact_Rune_M_Reserved=80,
	LevelArtifact_Rune_N_Reserved=81,
	LevelArtifact_Rune_O_Reserved=82,
	LevelArtifact_Rune_P_Reserved=83,
	LevelArtifact_Rune_Q_Reserved=84,
	LevelArtifact_Rune_R_Reserved=85,

	LevelArtifact_Max=87,

	//XXXXX:more LevelArtifactType
	//新增LevelArtifactType必须添加在末尾
	LevelArtifact_ForceDword=0xffffffff,
};
#define LevelArtifactConstraintStr "n/a:0,乌图之瓶:1,炽焰之戒:3,斩羽之剑:3,死神之翼:4,活力护甲:5,精灵权杖:6,活力护盾:7,火焰刀:8,闪电弓:9,荣耀之剑:10,骷髅剑:11,闪动剑:12,幻影匕首:13,血牙剑:14,尸爆弓:15,无名之剑:86,"\
	"野狼皮甲:16,塔尔的护佑:17,安的啜泣:18,黑钢甲:19,猎人护甲:20,西姆的诅咒:21,旋风甲:22,荣耀战甲:23,七子甲:24,寒霜甲:25,艾利的守护:26"\
    ",ExplodeOil:27,武器感应石A:28,武器感应石B:29,武器感应石C:30,脚尖石:31,力量药水:32,魔力药水:33,猎人戒指:34,灵魂石:35,圣箭:36,炸弹:37,母虫之眼:38,魔晶之心:39,魔法戒指:40,钱包:41,宝石罐:42"\
	",HP项链:43,SP项链:44,HP药水:45,SP药水:46,吸血戒指:47,魔盾面具:48,磨刀石:49,水晶眼:50,荣耀之书:51,骷髅王之冠:52,魔盾护身符:53,风暴之眼:54,HP宝瓶:55"\
	",圣诗卷轴I:56,圣诗卷轴II:57,圣诗卷轴III:58"\
	",圣诗卷轴IV(预留位置):59,圣诗卷轴V(预留位置):60,圣诗卷轴VI(预留位置):61,圣诗卷轴VII(预留位置):62,圣诗卷轴VIII(预留位置):63"\
	",圣诗卷轴X(预留位置):64,圣诗卷轴XI(预留位置):65,圣诗卷轴XII(预留位置):66,圣诗卷轴XIII(预留位置):67"\
	",符文A:68,符文B:69,符文C:70,符文D:71,符文E:72,符文F:73,符文G:74,符文H:75,符文I:76"\
	",符文J(预留位置):77,符文K(预留位置):78,符文L(预留位置):79,符文M(预留位置):80,符文N(预留位置):81,符文O(预留位置):82,符文P(预留位置):83,符文Q(预留位置):84,符文R(预留位置):85"

#define LevelArtifactConstraintStr_Poems "n/a:0,圣诗卷轴I:56,圣诗卷轴II:57,圣诗卷轴III:58"
#define LevelArtifactConstraintStr_Runes "n/a:0,符文A:68,符文B:69,符文C:70,符文D:71,符文E:72,符文F:73,符文G:74,符文H:75,符文I:76"
//XXXXX:more LevelArtifactType



//Posture
//注意修改后,要和UnitDefAttacks内同步
//n/a,徒手,单手盾,单手短兵器+盾,单手短兵器,单手长兵器+盾,单手长兵器,双手剑,弓,弩,双手矛,双手斧,双手杖
#define LevelPostureFlagsStr "徒手:1,单手盾:2,单手短兵器+盾:4,单手短兵器:8,单手长兵器+盾:16,单手长兵器:32,双手剑:64,弓:128,弩:256,双手矛:512,双手斧:1024,双手杖:2048,双手拖刀:4096,双持剑:8192,单手魔法物件+盾:16384,单手魔法物件:32768"
//XXXXX:more LevelPostureType
enum LevelPostureType
{
	LevelPosture_None=0,
	LevelPosture_Unarmed,//徒手
	LevelPosture_Shield,//盾
	LevelPosture_ShortWpnWithShield,//单手短兵器+盾
	LevelPosture_ShortWpn,//单手短兵器
	LevelPosture_LongWpnWithShield,//单手长兵器+盾
	LevelPosture_LongWpn,//单手长兵器
	LevelPosture_2HandSword,//双手剑武器
	LevelPosture_Bow,//弓
	LevelPosture_Crossbow,//弩
	LevelPosture_2HandLance,//双手矛武器
	LevelPosture_2HandAxe,//双手斧武器
	LevelPosture_2HandStaff,//双手杖武器

	LevelPosture_2HandDrag,//双手拖刀
	LevelPosture_2SwordInHand,//双持
	LevelPosture_MagicItemWithShield,
	LevelPosture_MagicItem,

	LevelPosture_Max,

	//XXXXX:more LevelPostureType
};


struct LevelEvent
{
	LevelEvent()
	{
		bHandled=FALSE;
	}
	virtual DWORD GetType()=0;
	void SetHandled()
	{
		bHandled=TRUE;
	}
	BOOL bHandled;
};

typedef DWORD LevelEventMapFlag;


//Residable 相关
typedef WORD LevelObjSeatToken;
#define LevelObjSeatToken_Invalid (0)
#define LevelObjSeatToken_Common (0xffff) //通用的token


//Talk 相关
enum LevelTalkMode
{
	TalkMode_None,
	TalkMode_Exclusive,//同一时刻,只能有一个玩家对话
	TalkMode_Share,//同一时刻,可以有多个玩家对话

	TalkMode_ForceDword=0xffffffff,
};
#define MAX_TALK_CHOICES 8

enum LevelTalkState
{
	LevelTalk_None=0,
	LevelTalk_Query,//正在请求对话
	LevelTalk_Accept,//当前弹出的东西被确认了,结果被保存下来

	LevelTalk_Topic,//当前弹出topic
	LevelTalk_Speak,//当前弹出Speak
	LevelTalk_Prompt,//当前弹出Prompt
	LevelTalk_Dialog,//当前弹出用户自定义的对话框
	LevelTalk_DialogCmd,//当前有一个对话框的命令要处理

	LevelTalk_ForceDword=0xffffffff,
};

struct LevelTalkDlgCmd
{
	LevelTalkDlgCmd()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	StringID cmd;//各种对话框命令
	DWORD bClose:1;//是否要关闭对话框
	float param01;//第一个参数
	float param02;//第二个参数
};


//Level Persist
template <int T_size>
struct LevelPersistEntryFixedSize
{
	LevelPersistEntryFixedSize()
	{
		memset(this,0,sizeof(*this));
	}
	BYTE ver;
	BYTE szData;
	void AddByte(BYTE v)
	{
		memcpy(&data[szData],&v,1);
		szData++;
	}
	BYTE data[T_size-2];
};

#define MAX_PERSISTENTRY_AGENT_SIZE 510//修改这个值将需要清空数据库
typedef LevelPersistEntryFixedSize<512> LevelPersistEntry_Agent;

#define MAX_PERSISTENTRY_LEVELAI_SIZE 510//修改这个值将需要清空数据库
typedef LevelPersistEntryFixedSize<512> LevelPersistEntry_LevelAI;


//Simple var的StringID定义
#define LevelSimpleVarName_Max 64
#define LevelSimpleVarName_CheckDay 1
#define LevelSimpleVarName_Byte 2
#define LevelSimpleVarName_Word 3
#define LevelSimpleVarName_Flag0 4
#define LevelSimpleVarName_Flag1 5
#define LevelSimpleVarName_Flag2 6
#define LevelSimpleVarName_Flag3 7
//XXXXX:more simple var
//注意,新加的Var添加在末尾(不要插在前面)

struct BhvVal;
struct LevelSimpleMem
{
	LevelSimpleMem()
	{
		memset(this,0,sizeof(*this));
	}

	void SetPersistDirty()	{		bPersistDirty=1;	}
	void ClearPersistDirty()	{		bPersistDirty=0;	}
	void CopyContent(LevelSimpleMem &from);
	BOOL EqualContent(LevelSimpleMem &other);
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	BOOL GetValue(StringID nm,DWORD &value);
	BOOL SetValue(StringID nm,DWORD value);

	BOOL FillBehaviorValue(StringID nm,BhvVal &v);//

	BYTE iCheckDay:5;//最多31天
	BYTE flag0:1;
	BYTE flag1:1;
	BYTE flag2:1;
	BYTE b;
	WORD w;

	BYTE bPersistDirty;
	BYTE bSyncDirty;

};

//S代表Simple
struct LevelPersistEntry_AgentS
{
	LevelSimpleMem mem_;
};





//Hook
#define LevelHookPriority_Low(v) (0+(v))
#define LevelHookPriority_Medium(v) (100+(v))
#define LevelHookPriority_Hi(v) (200+(v))
#define LevelHookPriority_Default LevelHookPriority_Medium(50)


struct LevelHook
{
	virtual int GetType()=0;

};

//Resource
enum LevelResourceType
{
	LevelResource_None=0,
	LevelResource_Gold,//金子
	LevelResource_Gem,//宝石
	LevelResource_Soul,//亡魂
	LevelResource_Labor,//劳力
	LevelResource_Crystal,//魔晶
	LevelResource_DemonBlood,//魔血

	//XXXXX:More LevelResourceType

	LevelRes_Max,

	LevelResource_ForceDword=0xffffffff,
};

#define LevelResourceType_SemConstraint "n/a,金子,宝石,亡魂,乌图姆,魔晶,魔血石" 	//XXXXX:More LevelResourceType

inline const char *NameFromResourceType(LevelResourceType tp)
{
	switch(tp)
	{
		case LevelResource_Gold:			return "金子";
		case LevelResource_Gem:			return "宝石";
		case LevelResource_Soul:			return "亡魂";
		case LevelResource_Labor:			return "乌图姆";
		case LevelResource_Crystal:			return "魔晶";
		case LevelResource_DemonBlood:			return "魔血石";
		//XXXXX:More LevelResourceType
	}
	return "n/a";
}



//伤害结算

// enum LevelAttackType
// {
// 	Attack_None=0,
// 	Attack_MeleePhys,
// 	Attack_RangePhys,
// 	Attack_Fire,
// 	Attack_Electricity,
// 	Attack_Cold,
// 	Attack_Poison,
// 	Attack_Cure,
// 
// 	Attack_Max,
// 
// 	Attack_ForceDword=0xffffffff,
// };

// struct LevelAttack
// {
// 	LevelAttack()
// 	{
// 		memset(this,0,sizeof(*this));
// 	}
// 	LevelAttackType tp;
// 	short atkLo,atkHi;
// 	short accu;
// 	short stun;
// 	LevelGrade grdOwner;
// };
// 
// struct LevelAttackAddOn
// {
// 	LevelAttackAddOn()
// 	{
// 		atkDelta=0;
// 		atkMultiply=1.0f;
// 		accuDelta=0;
// 		accuMultiply=1.0f;
// 	}
// 	void Apply(LevelAttack &atk)
// 	{
// 		atk.atkLo=(short)(atkMultiply*(float)atk.atkLo)+atkDelta;
// 		atk.atkHi=(short)(atkMultiply*(float)atk.atkHi)+atkDelta;
// 		atk.accu=(short)(accuMultiply*(float)atk.accu)+accuDelta;
// 		atk.stun=(short)(stunMultiply*(float)atk.stun)+stunDelta;
// 	}
// 	short atkDelta;
// 	float atkMultiply;
// 	short accuDelta;
// 	float accuMultiply;
// 	short stunDelta;
// 	float stunMultiply;
// };
// 
// struct LevelDefence
// {
// 	LevelDefence()
// 	{
// 		tp=Attack_None;
// 		memset(this,0,sizeof(*this));
// 	}
// 	LevelAttackType tp;
// 	short def;
// 	short evade;
// 	short stunresist;
// 	LevelGrade grdOwner;
// 	BYTE bCanStun:1;
// };

//NPC
enum NPCState
{
	NPCState_Default=0,
	NPCState_Retinue,
	NPCState_RetinueDead,//死去的随从
	
};


typedef DWORD LevelPriority;

//Award
struct LevelAwardSeed
{
	LevelAwardSeed()
	{
		v1=v2=0;
	}
	DWORD v1;
	DWORD v2;
};
struct LevelAward
{
	LevelAward()
	{
		memset(this,0,sizeof(*this));
	}
	enum Type
	{
		Item,
		Upgrade,
		Resource,
		LevelUp_Weapon,

		ForceDword=0xffffffff,
	};
	union
	{
		RecordID idRec;
		LevelResourceType tpRes;
	};

	BOOL MergeFrom(LevelAward &other)
	{
		if (bValid&&other.bValid)
		{
			if (tp==Resource)
			{
				if (other.bValid)
				{
					if (tpRes==other.tpRes)
					{
						count+=other.count;
						bExpendable=0;
						return TRUE;
					}
				}
			}
		}
		return FALSE;
	}

	LevelAwardSeed seed;

	DWORD tp:4;
	DWORD bValid:1;
	DWORD bExpendable:1;
	DWORD count:26;

};

struct LevelAwardPrice
{
	LevelAwardPrice()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsValid()
	{
		return (count>0)&&(tpRes>0);
	}
	DWORD tpRes:4;
	DWORD bAffordable:1;//能不能付得起
	DWORD count:27;
};

struct LevelResFileHeader
{
	DWORD type:8;
	DWORD off:24;//ResData完整数据的偏移(距离文件起始位置)
};

enum LevelMapLayor
{
	LevelMapLayor_None,
	LevelMapLayor_Ground,
	LevelMapLayor_Underground,

	LevelMapLayor_ForceDword=0xffffffff,
};

struct LevelAgentGuid
{
	RecordID idMap;
	LevelGUID guid;
};


struct LevelObjHits
{
	LevelObjHits()
	{
		c=0;
	}
	void Zero()
	{
		c=0;
	}
	BOOL IsEmpty()
	{
		return c==0;
	}
	void Add(LevelObjID id)
	{
		if (c<ARRAY_SIZE(ids))
		{
			ids[c]=id;
			c++;
		}
	}
	LevelObjID ids[4];
	int c;
};


typedef float LevelEoqPower;//EyeOfQueen 控制力
#define LevelEoqPower_Invalid (-1.0f)
#define LevelEoqPower_IsValid(v) ((v)>=0.0f)


#define LEVEL_MAX_VITA (64)


//Weak

enum LevelWeakCategory
{
	LevelWeakCategory_None=0,

	LevelWeakCategory_Start=1,

	LevelWeakCategory_StunFwd=LevelWeakCategory_Start,
	LevelWeakCategory_StunSide=2,
	LevelWeakCategory_StunBack=3,
	LevelWeakCategory_KB=4,
	LevelWeakCategory_Jink=5,
	//XXXXX: more LevelWeakCategory

	LevelWeakCategory_Count,

	LevelWeakCategory_ForceDword=0xffffffff,
};

typedef DWORD LevelWeakCategoryMask;

inline LevelWeakCategoryMask LevelWeakCategory_GetMask(LevelWeakCategory category)
{
	if (category==0)
		return 0;
	return (LevelWeakCategoryMask)(1<<(category-1));
}




//LevelService
typedef StringID LevelServiceType;
#define LevelServiceType_None (StringID_Invalid)


//Temple
enum LevelTempleType
{
	LevelTemple_None=0,
	LevelTemple_Sun,
	LevelTemple_Moon,
	LevelTemple_Fire,
	LevelTemple_Star,
	LevelTemple_Sand,
	LevelTemple_Craft,

	//XXXXX:More LevelTempleType
	LevelTemple_Max,

	LevelTemple_ForceDword=0xffffffff,
};

#define LevelTempleType_SemConstraint "n/a,光之圣殿,月之圣殿,火之圣殿,星之圣殿,沙之祭坛,工匠祭坛"
inline const char *NameFromTempleType(LevelTempleType tp)
{
	switch(tp)
	{
		case LevelTemple_Sun:			return "光之圣殿";
		case LevelTemple_Moon:			return "月之圣殿";
		case LevelTemple_Fire:			return "火之圣殿";
		case LevelTemple_Star:			return "星之圣殿";
		case LevelTemple_Sand:			return "沙之圣殿";
		case LevelTemple_Craft:			return "工匠圣殿";
		//XXXXX:More LevelTempleType
	}
	return "n/a";
}

struct LevelTempleInfo
{
	LevelTempleInfo()
	{
		maskAltars=0;
	}
	DWORD maskAltars;

	BOOL TestAltar(int iAltar)
	{
		return maskAltars&(1<<iAltar);
	}
	void SetAltar(int iAltar)
	{
		maskAltars|=(1<<iAltar);
	}
	DWORD GetAltarCount()
	{
		unsigned int tmp;
		tmp = maskAltars - ((maskAltars>>1) & 033333333333) - ((maskAltars>>2) & 011111111111);
		return ((tmp + (tmp >> 3)) & 030707070707) % 63;
// 		u32 slRet;
// 		__asm 
// 		{	
// 			popcnt eax,n;
// 			mov   dword ptr [slRet],eax
// 		}
// 		return slRet;

// 		DWORD c=0;
// 		DWORD mask=maskAltars;
// 		while(mask)
// 		{
// 			c++;
// 			mask &= (mask - 1) ;
// 		}
// 		return c;
	}
};


extern void GetMainGameCameraAxes(i_math::vector3df &axisX,i_math::vector3df &axisY,i_math::vector3df &axisZ);

class CLevelExploreMap;
struct LevelExploreMaps
{
	LevelExploreMaps()
	{
		sttc=dyn=NULL;
	}
	BOOL IsEmpty()
	{
		return sttc==NULL&&dyn==NULL;
	}
	CLevelExploreMap *sttc;//static
	CLevelExploreMap *dyn;

};

struct LevelPain
{
	LevelPain()
	{
		v=0;
		t=0;
	}
	float v;
	LevelTick t;
};


enum LevelUniqueObjType
{
	LevelUniqueObj_None,
	LevelUniqueObj_MagicBoard,
	LevelUniqueObj_SnailP1,
	LevelUniqueObj_Belly,
	LevelUniqueObj_StarPlate,
	LevelUniqueObj_MagicCircuit,

	LevelUniqueObj_Max,
};