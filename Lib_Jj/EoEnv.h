#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"
#include "datapacket/BitPacket.h"

#include "sparsearray/sparsearray2D.h"

#include "behaviorgraph/BehaviorCustomConst.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Env 46

struct EoParamEnv:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamEnv);

	BEGIN_GOBJ_PURE(EoParamEnv,1);

		GELEM_VAR_INIT(LevelTick,durLichenDealCycle,ANIMTICK_FROM_SECOND(0.2f)); GELEM_UID(1);
			GELEM_EDITVAR("地丝作用周期",GVT_U,GSem(GSem_AnimTick,"0.1,10,0.1"),"地丝的效果多久作用一次");
		GELEM_VAR_INIT(LevelTick,durLichenTrailSampleCycle,ANIMTICK_FROM_SECOND(0.2f)); GELEM_UID(2);
			GELEM_EDITVAR("地丝拖尾采样周期",GVT_U,GSem(GSem_AnimTick,"0.1,10,0.1"),"地丝拖尾采样周期");
		GELEM_VAR_INIT(LevelTick,durLichenTrailFI,ANIMTICK_FROM_SECOND(1.0f)); GELEM_UID(3);
			GELEM_EDITVAR("地丝拖尾淡入时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"地丝拖尾淡入时间");
		GELEM_VAR_INIT(LevelTick,durLichenTrail,ANIMTICK_FROM_SECOND(1.0f)); GELEM_UID(4);
			GELEM_EDITVAR("地丝拖尾时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"地丝拖尾时间");
		GELEM_VAR_INIT(LevelTick,durLichenTrailFO,ANIMTICK_FROM_SECOND(1.0f));GELEM_UID(5);
			GELEM_EDITVAR("地丝拖尾淡出时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"地丝拖尾淡出时间");
		GELEM_OBJVECTOR(DealEntry,dealsLichenEnemy); GELEM_UID(6);
			GELEM_EDITOBJ("地丝结算列表(敌人)","多个结算");
		GELEM_OBJVECTOR(DealEntry,dealsLichenNative); GELEM_UID(7);
			GELEM_EDITOBJ("地丝结算列表(本方)","多个结算");
		GELEM_VAR_INIT(float,radiusSporeExplode,1.0f);
			GELEM_EDITVAR("孢子爆炸半径",GVT_F,GSem(GSem_Float,"0.01,10.0,0.01"),"孢子爆炸半径");
		GELEM_OBJVECTOR(DealEntry,dealsSpore); GELEM_UID(8);
			GELEM_EDITOBJ("孢子结算列表(敌人)","多个结算");
		GELEM_VAR_INIT(LevelTick,durSporeGrow,ANIMTICK_FROM_SECOND(8.0f)); GELEM_UID(9);
			GELEM_EDITVAR("孢子生长时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"孢子生长时间");
		GELEM_VAR_INIT(LevelTick,durSporeUndetonatable,ANIMTICK_FROM_SECOND(2.0f)); GELEM_UID(10);
			GELEM_EDITVAR("孢子不可被引爆时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"孢子不可被引爆时间");
	END_GOBJ();

	LevelTick durLichenDealCycle;
	LevelTick durLichenTrailSampleCycle;
	LevelTick durLichenTrail;
	LevelTick durLichenTrailFI;
	LevelTick durLichenTrailFO;
	std::vector<DealEntry> dealsLichenEnemy;
	std::vector<DealEntry> dealsLichenNative;

	float radiusSporeExplode;
	std::vector<DealEntry> dealsSpore;
	LevelTick durSporeGrow;
	LevelTick durSporeUndetonatable;

};

typedef DWORD EoEnvLichenHandle;
#define EoEnvLichenHandle_Invalid (0)

typedef DWORD EoEnvSporeHandle;
#define EoEnvSporeHandle_Invalid (0)


struct EoEnvOp
{
	virtual CClass *GetClass()=0;
	enum Type
	{
		None,
		LichenStart,
		LichenStop,
		RequestDestroy,
	};
	virtual EoEnvOp::Type GetType()=0;
	virtual void Save(CBitPacket &bp)=0;
	virtual void Load(CBitPacket &bp)=0;
};

struct EoEnvOp_LichenStart:public EoEnvOp
{
	DEFINE_CLASS(EoEnvOp_LichenStart);
	EoEnvOp_LichenStart()
	{
		hLichen=EoEnvLichenHandle_Invalid;
		radius=0.0f;
		durFI=0;
		durFO=0;
		idHost=LevelObjID_Invalid;
		bTrail=FALSE;
		bDispel=FALSE;
	}
	virtual EoEnvOp::Type GetType()	{		return LichenStart;	}

	virtual void Save(CBitPacket &bp)
	{
		bp.Data_WriteSimple(hLichen);
		bp.Bit_Write(bTrail);
		bp.Bit_Write(bDispel);
		bp.Data_WriteSimple(idHost);
		bp.Data_WriteSimpleR(pos);
		bp.Data_WriteSimple(radius);
		if (bDispel)
			bp.Data_WriteSimple(radiusDispelCore);
		bp.Data_WriteSimple(durFI);
		bp.Data_WriteSimple(durFO);
	}
	virtual void Load(CBitPacket &bp)
	{
		bp.Data_ReadSimple(hLichen);
		bTrail=bp.Bit_Read();
		bDispel=bp.Bit_Read();
		bp.Data_ReadSimple(idHost);
		bp.Data_ReadSimple(pos);
		bp.Data_ReadSimple(radius);
		if (bDispel)
			bp.Data_ReadSimple(radiusDispelCore);
		bp.Data_ReadSimple(durFI);
		bp.Data_ReadSimple(durFO);
	}

	EoEnvLichenHandle hLichen;
	BOOL bTrail;
	BOOL bDispel;
	LevelObjID idHost;
	LevelPos pos;
	float radius;
	float radiusDispelCore;
	LevelTick durFI;
	LevelTick durFO;
};

struct EoEnvOp_LichenStop:public EoEnvOp
{
	DEFINE_CLASS(EoEnvOp_LichenStop);
	EoEnvOp_LichenStop()
	{
		hLichen=EoEnvLichenHandle_Invalid;
	}
	virtual EoEnvOp::Type GetType()	{		return LichenStop;	}

	virtual void Save(CBitPacket &bp)
	{
		bp.Data_WriteSimple(hLichen);
	}
	virtual void Load(CBitPacket &bp)
	{
		bp.Data_ReadSimple(hLichen);
	}

	EoEnvLichenHandle hLichen;
};

struct EoEnvOp_RequestDestroy:public EoEnvOp
{
	DEFINE_CLASS(EoEnvOp_RequestDestroy);
	EoEnvOp_RequestDestroy()
	{
	}
	virtual EoEnvOp::Type GetType()	{		return RequestDestroy;	}

	virtual void Save(CBitPacket &bp)
	{
	}
	virtual void Load(CBitPacket &bp)
	{
	}

};



class EoEnv:public CLoEffectObj
{
public:
	EoEnv()
	{
		_nLichenDeals=0;
		_nLichenSamples=0;

		_bFenceDestroyed=FALSE;

		_bPendingDestroy=FALSE;
	}
	~EoEnv()
	{
		_ClearSpores();
	}
	DEFINE_LEVELOBJ_CLASS(EoEnv,CLASSUID_Env);

	virtual const char *GetShowName()	{		return "战斗环境";	}

	virtual BOOL IsGlobalSight()	{		return TRUE;	}//是否全局视野(全局游戏对象),即玩家在任何地方都能看到它

	void RequestDestroy();

	void SetArea(BccArea &area);
	BccArea &GetArea()	{		return _area;	}

	EoEnvLichenHandle StartLichen(LevelPos &pos,float radius,BOOL bDispel,float radiusDispelCore,float durFI=1.0f,float durFO=1.0f);
	EoEnvLichenHandle StartLichen(LevelObjID idHost,float radius,BOOL bDispel,float radiusDispelCore,float durFI=1.0f,float durFO=1.0f);
	EoEnvLichenHandle StartLichenTrail(LevelObjID idHost,float radius);
	void StopLichen(EoEnvLichenHandle hLichen);
	void UpdateLichenPos(EoEnvLichenHandle hLichen,LevelPos &posNew);

	void SpawnSpore(LevelPos &pos,LevelOpLink &link);
	void DetonateSpore(LevelOSB &osb,LevelPos &pos,float radius,LevelOpLink &link);

	void SetFenceDestroyed()	{		_bFenceDestroyed=TRUE;	}
	BOOL CheckFenceDestroyed()	{		return _bFenceDestroyed;	}

protected:

	virtual void _OnDetroy() override;

	virtual BOOL _NeedOps() override	{		return TRUE;	}

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	virtual void _OnPostWriteSync() override;

	virtual void _OnUpdate() override;

	//Lichen
	void _UpdateLichenDeal();

	struct LichenEntry
	{
		LichenEntry()
		{
			handle=EoEnvLichenHandle_Invalid;
			bTrail=FALSE;

			idHost=LevelObjID_Invalid;
			radius=0.0f;
			
			tStart=ANIMTICK_INFINITE;
			durFI=0;
			tStop=ANIMTICK_INFINITE;
			durFO=0;

			bDispel=FALSE;
			radiusDispelCore=0.0f;
		}

		struct TrailSample
		{
			LevelTick tStart;
			LevelPos pos;
		};

		BOOL IsStarted()		{			return tStart!=ANIMTICK_INFINITE;		}
		BOOL IsStopped()		{			return tStop!=ANIMTICK_INFINITE;		}

		EoEnvLichenHandle handle;

		LevelPos pos;
		LevelObjID idHost;
		float radius;

		LevelTick tStart;
		LevelTick durFI;
		LevelTick tStop;
		LevelTick durFO;

		BOOL bTrail;
		std::deque<TrailSample> trail;

		BOOL bDispel;
		float radiusDispelCore;

	};
	std::unordered_map<EoEnvLichenHandle,LichenEntry> _lichens;
	void _ClearLichens();
	void _SaveLichens(CBitPacket &bp,BOOL &bContent);
	void _SaveLichenEntry(CBitPacket &bp,LichenEntry &e);
	void _CollectLichenAffects(LevelPos &pos,float radius,float wt);
	std::map<CLevelObj *,float> _affectsLichenEnemy;
	std::map<CLevelObj *,float> _affectsLichenNative;

	int _nLichenDeals;
	int _nLichenSamples;

	//Spore
	void _UpdateSporeDeal();

	struct SporeEntry
	{
		EoEnvSporeHandle handle;
		LevelPos pos;
		LevelTick tStart;
		SporeEntry *next;
	};
	struct SporesEntry
	{
		SporeEntry *head;//链表
	};
	SparseArray2D<SporesEntry,8> _mpSpores;
	CMemPool<SporeEntry> _poolSpores;
	void _ClearSpores();

	void _ClearEnvOps();
	void _SaveEnvOps(CBitPacket &bp,BOOL &bContent);
	std::vector<EoEnvOp*> _opsEnv;

	BOOL _bFenceDestroyed;
	BccArea _area;


	std::vector<LevelPos> _temp;

	BOOL _bPendingDestroy;
};
