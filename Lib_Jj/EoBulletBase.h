#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LoEffectObj.h"

#include "BulletBase.h"

struct BulletAbsorbSetting
{
	std::vector<RecordID> units;
	float radius;
	float angle;
	std::vector<DealEntry> deals;

	BEGIN_GOBJ_PURE(BulletAbsorbSetting,1);

		GELEM_VARVECTOR_INIT(RecordID,units,RecordID_Invalid);GELEM_UID(1);
			GELEM_EDITVAR("吸收者单位类型",GVT_U,GSem(GSem_RecordID,"units"),"单位类型");
		GELEM_VAR_INIT(float,radius,4.0f);GELEM_UID(2);
			GELEM_EDITVAR("吸收半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"吸收半径");
		GELEM_VAR_INIT(float,angle,60.0f);GELEM_UID(3);
			GELEM_EDITVAR("吸收角度",GVT_F,GSem(GSem_Float,"0.1,180,0.05"),"吸收者在Bullet前进方向上的角度范围");
		GELEM_OBJVECTOR(DealEntry,deals); GELEM_UID(4);
			GELEM_EDITOBJ("命中结算列表","命中吸收者后的结算");

	END_GOBJ();




};

struct EoParamBulletBase:public LevelEoParam
{
	enum InitialXfmMode
	{
		UseDefault=0,
		LocalOffsetToHostAim,

		ForceDword=0xffffffff,
	};

	InitialXfmMode modeInitialXfm;
	float xLocalOff;
	float yLocalOff;
	float zLocalOff;
	BOOL bMH;
	float radius;
	float fall;
	float speed;
	float speedVary;
	float distIgnoreStatic;
	BOOL bIgnoreHost;
	std::vector<DealEntry> dealsStaticHit;
	std::vector<DealEntry> dealsReach;
	std::vector<BulletAbsorbSetting> absorbs;
};

#define GELEM_EOPARAMBULLETBASE \
		GELEM_VAR_INIT(int,modeInitialXfm,0);GELEM_UID(27);\
			GELEM_EDITVAR("初始位置模式",GVT_S,GSem(GSem_Interger,\
				"默认位置/旋转:0"		"|角色局部空间内偏移(X轴)&角色局部空间内偏移(Y轴)&角色局部空间内偏移(Z轴),"\
				"局部偏移-->Host对象Aim点"	""\
				),"初始位置模式");\
		GELEM_VAR_INIT(float,xLocalOff,0.0f);GELEM_UID(28);\
			GELEM_EDITVAR("角色局部空间内偏移(X轴)",GVT_F,GSem(GSem_Float,"0.0,5.0,0.01"),"角色局部空间内偏移(X轴)");\
		GELEM_VAR_INIT(float,yLocalOff,1.6f);GELEM_UID(29);\
			GELEM_EDITVAR("角色局部空间内偏移(Y轴)",GVT_F,GSem(GSem_Float,"0.0,5.0,0.01"),"角色局部空间内偏移(Y轴)");\
		GELEM_VAR_INIT(float,zLocalOff,0.0f);GELEM_UID(30);\
			GELEM_EDITVAR("角色局部空间内偏移(Z轴)",GVT_F,GSem(GSem_Float,"0.0,5.0,0.01"),"角色局部空间内偏移(Z轴)");\
		GELEM_VAR_INIT(BOOL,bMH,FALSE); GELEM_UID(1);\
			GELEM_EDITVAR("是否穿透",GVT_S,GSem_Boolean,"是否穿透");\
		GELEM_VAR_INIT(float,radius,0.2f); GELEM_UID(2);\
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"半径"); \
		GELEM_VAR_INIT(float,fall,1.0f); GELEM_UID(31);\
			GELEM_EDITVAR("下探",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"向下额外检测的距离"); \
		GELEM_VAR_INIT(float,speed,5.0f); GELEM_UID(3);\
			GELEM_EDITVAR("速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"速度"); \
		GELEM_VAR_INIT(float,speedVary,0.0f); GELEM_UID(32);\
			GELEM_EDITVAR("速度浮动值",GVT_F,GSem(GSem_Float,"0.0,100,0.1"),"速度浮动值"); \
		GELEM_VAR_INIT(float,distIgnoreStatic,0.0f); GELEM_UID(4);\
			GELEM_EDITVAR("忽视静态障碍的距离",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"在子弹开始移动的多少距离内忽视静态障碍"); \
		GELEM_OBJVECTOR(DealEntry,dealsStaticHit); GELEM_UID(5);\
			GELEM_EDITOBJ("静态障碍结算列表","碰到静态障碍时的结算");\
		GELEM_OBJVECTOR(DealEntry,dealsReach); GELEM_UID(34);\
			GELEM_EDITOBJ("到达后结算列表","到达后结算列表");\
		GELEM_OBJVECTOR(BulletAbsorbSetting,absorbs); GELEM_UID(33);\
			GELEM_EDITOBJ("子弹吸收设置","子弹吸收设置");\
		GELEM_VAR_INIT(BOOL,bIgnoreHost,TRUE); GELEM_UID(26);\
			GELEM_EDITVAR("是否忽略Host",GVT_S,GSem_Boolean,"是否忽略Host");

class EoBulletBase:public CLoEffectObj
{
public:
	EoBulletBase()
	{
		_core=NULL;
		_t=0;
		_tFinish=ANIMTICK_INFINITE;
		_bNeedSyncAbsorb=FALSE;
	}
protected:

	virtual void _OnPostCreate();
	virtual void _OnDetroy();


	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync();

	virtual void _OnUpdate();
	virtual BOOL _NeedOps()	{		return TRUE;	}

	virtual CBulletBase *_CreateBullet()=0;
	virtual void _DestroyBullet(CBulletBase *bullet)=0;

	void _WriteHits(CBitPacket *bp,BOOL &bContent);
	void _WriteAbsorb(CBitPacket *bp,BOOL &bContent);

	BulletAbsorbSetting *_FindAbsorbSetting(EoParamBulletBase*param,CLevelObj *loTarget);

	CBulletBase *_core;
	AnimTick _t;

	AnimTick _tFinish;
	LevelObjHits _hitsToSend;
	BulletStaticHit _hitStaticToSend;
	BOOL _bNeedSyncAbsorb;

};


