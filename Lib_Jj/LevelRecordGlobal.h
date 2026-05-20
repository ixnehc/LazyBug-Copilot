#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"
#include "LevelDeal.h"

struct StdMiniIcons
{
	std::string main;
	std::string checkmark;

	BEGIN_GOBJ_PURE(StdMiniIcons,1);
		GELEM_STRING_INIT(main,"");
			GELEM_EDITVAR("主角的小地图图标",GVT_String,GSem_TexturePartPath,"主角的小地图图标");
		GELEM_STRING_INIT(checkmark,"");
			GELEM_EDITVAR("打勾标记",GVT_String,GSem_TexturePartPath,"小地图上的打勾标记");
	END_GOBJ();
};

struct StdItems
{
	std::vector<RecordID> golds;
	std::vector<RecordID> gems;
	std::vector<RecordID> crystals;
	unsigned __int64 crystalDisappear;

	BEGIN_GOBJ_PURE(StdItems,1);
		GELEM_VARVECTOR_INIT(RecordID,golds,RecordID_Invalid);
			GELEM_EDITVAR("金粒",GVT_U,GSem(GSem_RecordID,"items"),"金粒的Item");
		GELEM_VARVECTOR_INIT(RecordID,gems,RecordID_Invalid);
			GELEM_EDITVAR("宝石",GVT_U,GSem(GSem_RecordID,"items"),"宝石的Item");
		GELEM_VARVECTOR_INIT(RecordID,crystals,RecordID_Invalid);
			GELEM_EDITVAR("魔晶",GVT_U,GSem(GSem_RecordID,"items"),"魔晶的Item");
		GELEM_VAR_INIT(unsigned __int64,crystalDisappear,0);
			GELEM_EDITVAR("魔晶消失",GVT_Bx8,GSem_ProtoPath,"魔晶消失效果的Proto路径");
	END_GOBJ();
};

struct StdBubbles
{
	unsigned __int64 dmgHP;
	unsigned __int64 hpMax;
	unsigned __int64 spMax;
	unsigned __int64 spFull;

	unsigned __int64 gold;
	unsigned __int64 gem;
	unsigned __int64 labor;
	unsigned __int64 crystal;
	unsigned __int64 demonblood;
	//XXXXX:More LevelResourceType
	
	BEGIN_GOBJ_PURE(StdBubbles,1);
		GELEM_VAR_INIT(unsigned __int64,dmgHP,0);
			GELEM_EDITVAR("伤害泡泡",GVT_Bx8,GSem_ProtoPath,"伤害泡泡的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,hpMax,0);
			GELEM_EDITVAR("最大HP泡泡",GVT_Bx8,GSem_ProtoPath,"最大HP泡泡的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,spMax,0);
			GELEM_EDITVAR("最大SP泡泡",GVT_Bx8,GSem_ProtoPath,"最大SP泡泡的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,spFull,0);
			GELEM_EDITVAR("FullSP泡泡",GVT_Bx8,GSem_ProtoPath,"FullSP泡泡的Proto路径");

		GELEM_VAR_INIT(unsigned __int64,gold,0);
			GELEM_EDITVAR("金钱泡泡",GVT_Bx8,GSem_ProtoPath,"金钱泡泡的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,gem,0);
			GELEM_EDITVAR("宝石泡泡",GVT_Bx8,GSem_ProtoPath,"宝石泡泡的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,labor,0);
			GELEM_EDITVAR("Labor泡泡",GVT_Bx8,GSem_ProtoPath,"Labor泡泡的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,crystal,0);
			GELEM_EDITVAR("魔晶泡泡",GVT_Bx8,GSem_ProtoPath,"魔晶泡泡的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,demonblood,0);
			GELEM_EDITVAR("魔血石泡泡",GVT_Bx8,GSem_ProtoPath,"魔血石的Proto路径");
		//XXXXX:More LevelResourceType
	END_GOBJ();
};

struct ArtifactChargeIconSet
{
	std::vector<std::string> pathes;
	std::vector<std::string> normal;
	std::vector<std::string> disabled;
	BEGIN_GOBJ_PURE(ArtifactChargeIconSet,1);
	GELEM_STRINGVECTOR(normal);
			GELEM_EDITVAR("不同充能数量的图标路径(激活)",GVT_String,GSem_TexturePartPath,"不同充能数量的图标路径");
		GELEM_STRINGVECTOR(disabled);
			GELEM_EDITVAR("不同充能数量的图标路径(失活)",GVT_String,GSem_TexturePartPath,"不同充能数量的图标路径");
	END_GOBJ();

};

struct Sights
{
	unsigned __int64 main;//主角
	unsigned __int64 rtnu;//随从

	BEGIN_GOBJ_PURE(Sights,1);
		GELEM_VAR_INIT(unsigned __int64,main,0);
			GELEM_EDITVAR("主角视野",GVT_Bx8,GSem_ProtoPath,"主角视野的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,rtnu,0);
			GELEM_EDITVAR("随从视野",GVT_Bx8,GSem_ProtoPath,"随从视野的Proto路径");
	END_GOBJ();
};

struct LichenAttract
{
	unsigned __int64 main;//主角
	unsigned __int64 rtnu;//随从

	BEGIN_GOBJ_PURE(LichenAttract,1);
		GELEM_VAR_INIT(unsigned __int64,main,0);
			GELEM_EDITVAR("主角地丝吸引器",GVT_Bx8,GSem_ProtoPath,"主角地丝吸引器的Proto路径");
		GELEM_VAR_INIT(unsigned __int64,rtnu,0);
			GELEM_EDITVAR("随从地丝吸引器",GVT_Bx8,GSem_ProtoPath,"随从地丝吸引器的Proto路径");
	END_GOBJ();
};

struct FootRings
{
	unsigned __int64 main;
	unsigned __int64 rtnu;//随从
	unsigned __int64 rtnuGuard;//随从(Guard模式)

	BEGIN_GOBJ_PURE(FootRings,1);
		GELEM_VAR_INIT(unsigned __int64,main,0);
			GELEM_EDITVAR("主角光圈",GVT_Bx8,GSem_ProtoPath,"主角脚下的光圈");
		GELEM_VAR_INIT(unsigned __int64,rtnu,0);
			GELEM_EDITVAR("随从光圈",GVT_Bx8,GSem_ProtoPath,"随从脚下的光圈");
		GELEM_VAR_INIT(unsigned __int64,rtnuGuard,0);
			GELEM_EDITVAR("随从光圈(Guard模式)",GVT_Bx8,GSem_ProtoPath,"随从脚下的光圈(Guard模式)");
	END_GOBJ();

};

struct CollarInfo
{
	StringID idEP;
	DWORD colMe;
	DWORD colEnemy;

	BEGIN_GOBJ_PURE(CollarInfo,1);
		GELEM_VAR_INIT(StringID,idEP,StringID_Invalid);
			GELEM_EDITVAR( "袖标颜色参数名称", GVT_U, GSem(GSem_StringID,"EffectParam定义"), "代表袖标颜色的材质参数名称" );
		GELEM_VAR_INIT(DWORD,colMe,0x00ff00ff);
			GELEM_EDITVAR("本方颜色",GVT_Bx4,GSem_ColorAlphaU,"本方的袖标颜色");
		GELEM_VAR_INIT(DWORD,colEnemy,0xff0000ff);
			GELEM_EDITVAR("敌方颜色",GVT_Bx4,GSem_ColorAlphaU,"敌方的袖标颜色");
	END_GOBJ();

};


struct SlatesInfo
{
	unsigned __int64 coverNormal;
	unsigned __int64 coverSilver;
	unsigned __int64 coverGold;

	unsigned __int64 edgelink;

	unsigned __int64 switchlock;
	unsigned __int64 switch_;
	unsigned __int64 switchpointer;

	unsigned __int64 buttonlock;
	unsigned __int64 button;

	std::vector<unsigned __int64> gems;


	BEGIN_GOBJ_PURE(SlatesInfo,1);
		GELEM_VAR_INIT(unsigned __int64,coverNormal,0);
			GELEM_EDITVAR("普通",GVT_Bx8,GSem_ProtoPath,"普通Slate");
		GELEM_VAR_INIT(unsigned __int64,coverSilver,0);
			GELEM_EDITVAR("银色",GVT_Bx8,GSem_ProtoPath,"银色Slate");
		GELEM_VAR_INIT(unsigned __int64,coverGold,0);
			GELEM_EDITVAR("金色",GVT_Bx8,GSem_ProtoPath,"金色Slate");
		GELEM_VAR_INIT(unsigned __int64,edgelink,0);
			GELEM_EDITVAR("EdgeLink",GVT_Bx8,GSem_ProtoPath,"EdgeLink");
		GELEM_VAR_INIT(unsigned __int64,switchlock,0);
			GELEM_EDITVAR("SwitchLock",GVT_Bx8,GSem_ProtoPath,"SwitchLock");
		GELEM_VAR_INIT(unsigned __int64,switch_,0);
			GELEM_EDITVAR("Switch",GVT_Bx8,GSem_ProtoPath,"Switch");
		GELEM_VAR_INIT(unsigned __int64,switchpointer,0);
			GELEM_EDITVAR("SwitchPointer",GVT_Bx8,GSem_ProtoPath,"SwitchPointer");
		GELEM_VAR_INIT(unsigned __int64,buttonlock,0);
			GELEM_EDITVAR("ButtonLock",GVT_Bx8,GSem_ProtoPath,"ButtonLock");
		GELEM_VAR_INIT(unsigned __int64,button,0);
			GELEM_EDITVAR("Button",GVT_Bx8,GSem_ProtoPath,"Button");
		GELEM_VARVECTOR_INIT(unsigned __int64,gems,0);
			GELEM_EDITVAR("Gems",GVT_Bx8,GSem_ProtoPath,"Gems");
	END_GOBJ();
	//XXXXX:MoreSlateCover

};

struct BellySetting
{
	BEGIN_GOBJ_PURE(BellySetting,1);

		GELEM_VAR_INIT(RecordID,idEnvEo,RecordID_Invalid);
			GELEM_EDITVAR("战斗环境",GVT_U,GSem(GSem_RecordID,"eos"),"战斗环境");
		GELEM_VAR_INIT(RecordID,idUnit_Belly,RecordID_Invalid);
			GELEM_EDITVAR("Belly单位",GVT_U,GSem(GSem_RecordID,"units"),"Belly单位");
		GELEM_VAR_INIT(RecordID,idUnit_BellyQueen,RecordID_Invalid);
			GELEM_EDITVAR("BellyQueen单位",GVT_U,GSem(GSem_RecordID,"units"),"BellyQueen单位");
		GELEM_VAR_INIT(float,htBellyQueen,3.0f);
			GELEM_EDITVAR("BellyQueen离地高度",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"BellyQueen离地高度");
		GELEM_VAR_INIT(RecordID,idUnit_BellyKing,RecordID_Invalid);
			GELEM_EDITVAR("BellyKing单位",GVT_U,GSem(GSem_RecordID,"units"),"BellyKing单位");
		GELEM_VAR_INIT(RecordID,idUnit_BellyMinion,RecordID_Invalid);
			GELEM_EDITVAR("BellyMinion单位",GVT_U,GSem(GSem_RecordID,"units"),"BellyMinion单位");
		GELEM_VAR_INIT(RecordID,idBuff_SacredOrb,RecordID_Invalid);
			GELEM_EDITVAR("SacredOrb",GVT_U,GSem(GSem_RecordID,"buffs"),"SacredOrb");

	END_GOBJ();

	RecordID idEnvEo;
	RecordID idUnit_Belly;
	RecordID idUnit_BellyQueen;
	float htBellyQueen;
	RecordID idUnit_BellyKing;
	RecordID idUnit_BellyMinion;

	RecordID idBuff_SacredOrb;
};

struct StarPlateSetting
{
	BEGIN_GOBJ_PURE(StarPlateSetting,1);

		GELEM_VAR_INIT(unsigned __int64,idPlate,0);GELEM_UID(16);
			GELEM_EDITVAR("Plate Proto",GVT_Bx8,GSem_ProtoPath,"Plate Proto");
		GELEM_VAR_INIT(unsigned __int64,idSite,0);GELEM_UID(1);
			GELEM_EDITVAR("Site Proto",GVT_Bx8,GSem_ProtoPath,"Site Proto");
		GELEM_VAR_INIT(unsigned __int64,idLink,0);GELEM_UID(2);
			GELEM_EDITVAR("Link Proto",GVT_Bx8,GSem_ProtoPath,"Link Proto");
		GELEM_VAR_INIT(unsigned __int64,idChip,0);GELEM_UID(14);
			GELEM_EDITVAR("Chip Proto",GVT_Bx8,GSem_ProtoPath,"Chip Proto");
		GELEM_VAR_INIT(unsigned __int64,idLinkBeam,0);GELEM_UID(3);
			GELEM_EDITVAR("Link光线Proto",GVT_Bx8,GSem_ProtoPath,"Link光线Proto");
		GELEM_VAR_INIT(unsigned __int64,idObelisk,0);GELEM_UID(17);
			GELEM_EDITVAR("Obelisk Proto",GVT_Bx8,GSem_ProtoPath,"Proto");
		GELEM_VAR_INIT(RecordID,idAgentSite,RecordID_Invalid);GELEM_UID(13);
			GELEM_EDITVAR("Site Agent",GVT_U,GSem(GSem_RecordID,"agents"),"Agent");
		GELEM_VAR_INIT(RecordID,idAgentObelisk,RecordID_Invalid);GELEM_UID(18);
			GELEM_EDITVAR("Obelisk Agent",GVT_U,GSem(GSem_RecordID,"agents"),"Agent");
		GELEM_OBJVECTOR(DealEntry,dealsKillingKing); GELEM_UID(19);
			GELEM_EDITOBJ("结算列表(杀死King)","多个结算");
		GELEM_VAR_INIT(float,durKillingKingDelay,2.0f);GELEM_UID(20);
			GELEM_EDITVAR("杀死King的延后时间",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"杀死King的延后时间");
		GELEM_VAR_INIT(float,durCDSite,20.0f);GELEM_UID(4);GELEM_VERSION(2);
			GELEM_EDITVAR("Site CD时长",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"Site CD时长");
		GELEM_VAR_INIT(float,distBridgeMin,3.0f);GELEM_UID(7);
			GELEM_EDITVAR("最短星桥距离",GVT_F,GSem(GSem_Float,"0.01,100.0,0.01"),"最短星桥距离");
		GELEM_VAR_INIT(float,distBridgeMax,3.0f);GELEM_UID(8);
			GELEM_EDITVAR("最长星桥距离",GVT_F,GSem(GSem_Float,"0.01,100.0,0.01"),"最长星桥距离");
		GELEM_VAR_INIT(int,countBridgeMin,5);GELEM_UID(9);
			GELEM_EDITVAR("最少星桥个数",GVT_S,GSem_Interger,"最少星桥个数");
		GELEM_VAR_INIT(int,countBridgeMax,6);GELEM_UID(10);
			GELEM_EDITVAR("最多星桥个数",GVT_S,GSem_Interger,"最多星桥个数");
		GELEM_VAR_INIT(float,radiusSense,2.0f);GELEM_UID(11);
			GELEM_EDITVAR("星点感应范围",GVT_F,GSem(GSem_Float,"0.01,10.0,0.01"),"星点感应范围");

		GELEM_VAR_INIT(RecordID,idBuff_SacredOrb,RecordID_Invalid);GELEM_UID(12);
			GELEM_EDITVAR("SacredOrb",GVT_U,GSem(GSem_RecordID,"buffs"),"SacredOrb");

	END_GOBJ();

	unsigned __int64 idPlate;
	unsigned __int64 idSite;
	unsigned __int64 idLink;
	unsigned __int64 idChip;
	unsigned __int64 idLinkBeam;
	unsigned __int64 idObelisk;
	RecordID idAgentSite;
	RecordID idAgentObelisk;
	std::vector<DealEntry> dealsKillingKing;
	float durKillingKingDelay;
	float durCDSite;
	float distBridgeMin;
	float distBridgeMax;
	int countBridgeMin;
	int countBridgeMax;
	float radiusSense;

	RecordID idBuff_SacredOrb;

};

struct MagicCircuitSetting
{
	BEGIN_GOBJ_PURE(MagicCircuitSetting,1);

		GELEM_VAR_INIT( StringID,idBG,StringID_Invalid);	
			GELEM_EDITVAR( "行为图", GVT_U, GSem(GSem_StringID,"行为图名称"), "行为图" );

		GELEM_VAR_INIT(RecordID,idAgentRelay,RecordID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("Relay Agent",GVT_U,GSem(GSem_RecordID,"agents"),"Agent");
		GELEM_VAR_INIT(RecordID, idBuffRelayHit, RecordID_Invalid);GELEM_UID(17);
			GELEM_EDITVAR("Relay Hit Buff", GVT_U, GSem(GSem_RecordID, "buffs"), "Relay Hit Buff");
		GELEM_VAR_INIT(RecordID,idAgentBollard,RecordID_Invalid);GELEM_UID(16);
			GELEM_EDITVAR("Bollard Agent",GVT_U,GSem(GSem_RecordID,"agents"),"Agent");
		GELEM_VAR_INIT(RecordID, idBuffTeleport, RecordID_Invalid);GELEM_UID(20);
			GELEM_EDITVAR("Teleport Buff", GVT_U, GSem(GSem_RecordID, "buffs"), "Teleport Buff");
		GELEM_VARVECTOR_INIT(RecordID,idsFuseReady,RecordID_Invalid);GELEM_UID(15);
			GELEM_EDITVAR("FuseReady Buffs",GVT_U,GSem(GSem_RecordID,"buffs"),"Agent");
		GELEM_VAR_INIT(RecordID,idFuseBurn,RecordID_Invalid);GELEM_UID(14);
			GELEM_EDITVAR("FuseBurn Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Agent");
		GELEM_VAR_INIT( StringID,signalActivatePipe,StringID_Invalid);	GELEM_UID(3);
			GELEM_EDITVAR( "Activate Pipe信号", GVT_U, GSem(GSem_StringID,"信号名称"), "信号名称" );
		GELEM_VAR_INIT( StringID,signalDeactivatePipe,StringID_Invalid);	GELEM_UID(4);
			GELEM_EDITVAR( "Deactivate Pipe信号", GVT_U, GSem(GSem_StringID,"信号名称"), "信号名称" );
		GELEM_VAR_INIT( StringID,signalSpotOn,StringID_Invalid);	GELEM_UID(10);
			GELEM_EDITVAR( "SpotOn信号", GVT_U, GSem(GSem_StringID,"信号名称"), "信号名称" );
		GELEM_VAR_INIT( StringID,signalSpotOff,StringID_Invalid);	GELEM_UID(11);
			GELEM_EDITVAR( "SpotOff信号", GVT_U, GSem(GSem_StringID,"信号名称"), "信号名称" );
		GELEM_VAR_INIT( StringID,signalSpotFlash,StringID_Invalid);	GELEM_UID(12);
			GELEM_EDITVAR( "SpotFlash信号", GVT_U, GSem(GSem_StringID,"信号名称"), "信号名称" );
		GELEM_VAR_INIT( StringID,signalActivateTeleport,StringID_Invalid);	GELEM_UID(18);
			GELEM_EDITVAR( "Activate Teleport信号", GVT_U, GSem(GSem_StringID,"信号名称"), "信号名称" );
		GELEM_VAR_INIT( StringID,signalDeactivateTeleport,StringID_Invalid);	GELEM_UID(19);
			GELEM_EDITVAR( "Deactivate Teleport信号", GVT_U, GSem(GSem_StringID,"信号名称"), "信号名称" );
		GELEM_OBJVECTOR(DealEntry,dealsSummonCrystal);GELEM_UID(5);
			GELEM_EDITOBJ("召唤水晶结算列表","多个结算");
		GELEM_OBJVECTOR(DealEntry,dealsSummonRelayBird);GELEM_UID(6);
			GELEM_EDITOBJ("召唤RelayBird结算列表","多个结算");
		GELEM_OBJVECTOR(DealEntry,dealsSummonRailGuard);GELEM_UID(21);
			GELEM_EDITOBJ("召唤RailGuard结算列表","多个结算");
		GELEM_VAR_INIT(unsigned __int64,idFocus,0);GELEM_UID(7);
			GELEM_EDITVAR("Focus Proto",GVT_Bx8,GSem_ProtoPath,"Proto");
		GELEM_VAR_INIT(unsigned __int64,idFocusHit,0);GELEM_UID(8);
			GELEM_EDITVAR("Focus Hit Proto",GVT_Bx8,GSem_ProtoPath,"Proto");
		GELEM_VAR_INIT(unsigned __int64,idTailOrb,0);GELEM_UID(9);
			GELEM_EDITVAR("Tail Orb Proto",GVT_Bx8,GSem_ProtoPath,"Proto");

			//Next UID: 22
	END_GOBJ();

	StringID idBG;

	RecordID idAgentRelay;
	RecordID idBuffRelayHit;
	RecordID idAgentBollard;
	RecordID idBuffTeleport;
	StringID signalActivatePipe;
	StringID signalDeactivatePipe;
	StringID signalSpotOn;
	StringID signalSpotOff;
	StringID signalSpotFlash;
	StringID signalActivateTeleport;
	StringID signalDeactivateTeleport;
	std::vector<RecordID> idsFuseReady;
	RecordID idFuseBurn;
	std::vector<DealEntry> dealsSummonCrystal;
	std::vector<DealEntry> dealsSummonRelayBird;
	std::vector<DealEntry> dealsSummonRailGuard;
	unsigned __int64 idFocus;
	unsigned __int64 idFocusHit;
	unsigned __int64 idTailOrb;

};


struct EffectParamNames
{
	StringID epe_uvrot;

	BEGIN_GOBJ_PURE(EffectParamNames,1);

		GELEM_VAR_INIT( StringID,epe_uvrot,StringID_Invalid);	
			GELEM_EDITVAR( "epe_uvrot", GVT_U, GSem(GSem_StringID,"EffectParam定义"), "名称" );

	END_GOBJ();

};

struct SlidewaySetting
{
	StringID nmReachedSignal;
	float radiusSignal;

	BEGIN_GOBJ_PURE(SlidewaySetting,1);

		GELEM_VAR_INIT( StringID,nmReachedSignal,StringID_Invalid);	
			GELEM_EDITVAR( "抵达信号", GVT_U, GSem(GSem_StringID,"信号名称"), "名称" );
		GELEM_VAR_INIT(float,radiusSignal,10.0f);
			GELEM_EDITVAR("信号广播半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"广播半径");

	END_GOBJ();

};

struct LevelRecordGlobal:public CRecord
{
	DEFINE_CLASS(LevelRecordGlobal);

	RecordID idPlayerUnit;
	RecordID idDefBuff_Stun;
	RecordID idDefBuff_SkillStun;
	RecordID idDefBuff_KB;
	RecordID idDefBuff_Dizzy;
	RecordID idDefBuff_Dead;
	RecordID idDefBuff_PB;
	RecordID idDefBuff_InSlates;
	RecordID idDefBuff_Bleed;
	RecordID idDefBuff_Teleport;

	RecordID idDefPosture;

	RecordID idExhaustedSkill;

	RecordID idStartMap;

	RecordID AgentInvoker;
	RecordID ItemInvoker;

	RecordID eoDropSoul;
	RecordID eoDropMP;
	RecordID eoDropDemonBlood;

	std::vector<RecordID> itemsInitial;//默认的技能

	StdItems itemsStd;
	ArtifactChargeIconSet iconsetCharge;

	StdMiniIcons mis;

	StdBubbles bubblesStd;

	Sights sights;
	LichenAttract attractLichen;
	FootRings rings;

	CollarInfo collarinfo;

	SlatesInfo slatesinfo;

	BellySetting bellysetting;
	StarPlateSetting starplatesetting;
	MagicCircuitSetting magiccircuitsetting;

	EffectParamNames epnames;

	SlidewaySetting slidewaysetting;

	BEGIN_GOBJ_PURE(LevelRecordGlobal,1);

		GELEM_VAR_INIT(RecordID,idPlayerUnit,RecordID_Invalid);
			GELEM_EDITVAR("主角Unit",GVT_U,GSem(GSem_RecordID,"units"),"主角使用的Unit");

		GELEM_VAR_INIT(RecordID,idStartMap,RecordID_Invalid);
			GELEM_EDITVAR( "起始关卡", GVT_U, GSem(GSem_RecordID,"maps"), "起始的关卡" );

		GELEM_VARVECTOR_INIT(RecordID,itemsInitial,RecordID_Invalid);
			GELEM_EDITVAR("初始道具",GVT_U,GSem(GSem_RecordID,"items"),"初始道具");

		GELEM_VAR_INIT(RecordID,idDefPosture,RecordID_Invalid);
			GELEM_EDITVAR("缺省的姿势",GVT_U,GSem(GSem_RecordID,"postures"),"单位没有装备任何武器时使用的Posture");

		GELEM_VAR_INIT(RecordID,idExhaustedSkill,RecordID_Invalid);
			GELEM_EDITVAR("筋疲力尽时的缺省技能",GVT_U,GSem(GSem_RecordID,"skills"),"筋疲力尽时的缺省技能");

		GELEM_VAR_INIT(RecordID,idDefBuff_Stun,RecordID_Invalid);
			GELEM_EDITVAR("缺省的硬直 Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位Stun时缺省的使用哪个Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_SkillStun,RecordID_Invalid);
			GELEM_EDITVAR("缺省的Skill硬直 Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位Skill硬直时缺省的使用哪个Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_KB,RecordID_Invalid);
			GELEM_EDITVAR("缺省的击退Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位被击退时缺省的使用哪个Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_Dizzy,RecordID_Invalid);
			GELEM_EDITVAR("缺省的眩晕Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位被眩晕时缺省的使用哪个Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_PB,RecordID_Invalid);
			GELEM_EDITVAR("缺省的推开Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位被推开时缺省的使用哪个Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_Dead,RecordID_Invalid);
			GELEM_EDITVAR("缺省的死亡Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位死亡时缺省的使用哪个Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_InSlates,RecordID_Invalid);
			GELEM_EDITVAR("缺省的石板迷宫Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位在石板迷宫内时缺省使用哪个Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_Bleed,RecordID_Invalid);
			GELEM_EDITVAR("缺省的流血Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"缺省的流血Buff");
		GELEM_VAR_INIT(RecordID,idDefBuff_Teleport,RecordID_Invalid);
			GELEM_EDITVAR("缺省的传送Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"缺省的传送Buff");

		GELEM_VAR_INIT(RecordID,AgentInvoker,RecordID_Invalid);
			GELEM_EDITVAR("触发功能单位技能",GVT_U,GSem(GSem_RecordID,"skills"),"触发功能单位技能");
		GELEM_VAR_INIT(RecordID,ItemInvoker,RecordID_Invalid);
			GELEM_EDITVAR("拾取道具的技能",GVT_U,GSem(GSem_RecordID,"skills"),"拾取道具的技能");

		GELEM_VAR_INIT(RecordID,eoDropSoul,RecordID_Invalid);
			GELEM_EDITVAR("吸取掉落Soul的Eo",GVT_U,GSem(GSem_RecordID,"eos"),"吸取掉落Soul的Eo");
		GELEM_VAR_INIT(RecordID,eoDropMP,RecordID_Invalid);
			GELEM_EDITVAR("吸取掉落MP的Eo",GVT_U,GSem(GSem_RecordID,"eos"),"吸取掉落MP的Eo");
		GELEM_VAR_INIT(RecordID,eoDropDemonBlood,RecordID_Invalid);
			GELEM_EDITVAR("吸取掉落魔血石的Eo",GVT_U,GSem(GSem_RecordID,"eos"),"吸取掉落MP的Eo");

		GELEM_OBJ(StdItems,itemsStd)
			GELEM_EDITOBJ("标准的道具","标准的道具");

		GELEM_OBJ(ArtifactChargeIconSet,iconsetCharge)
			GELEM_EDITOBJ("Artifact的充能图标集","Artifact的充能图标集");

		GELEM_OBJ(StdMiniIcons,mis)
			GELEM_EDITOBJ("小地图图标","标准的小地图图标");

		GELEM_OBJ(StdBubbles,bubblesStd)
			GELEM_EDITOBJ("标准的泡泡","标准的泡泡");

		GELEM_OBJ(Sights,sights)
			GELEM_EDITOBJ("视野效果","视野效果");
		GELEM_OBJ(LichenAttract,attractLichen)
			GELEM_EDITOBJ("地丝吸引器效果","地丝吸引器效果");
		GELEM_OBJ(FootRings,rings)
			GELEM_EDITOBJ("脚下光圈","脚下光圈");
		GELEM_OBJ(CollarInfo,collarinfo)
			GELEM_EDITOBJ("袖标信息","袖标信息");

		GELEM_OBJ(SlatesInfo,slatesinfo)
			GELEM_EDITOBJ("石板效果","石板效果");

		GELEM_OBJ(BellySetting,bellysetting)
			GELEM_EDITOBJ("Belly信息","Belly信息");

		GELEM_OBJ(StarPlateSetting,starplatesetting)
			GELEM_EDITOBJ("StarPlate信息","StarPlate信息");

		GELEM_OBJ(MagicCircuitSetting,magiccircuitsetting)
			GELEM_EDITOBJ("MagicCircuit信息","MagicCircuit信息");

		GELEM_OBJ(EffectParamNames,epnames)
			GELEM_EDITOBJ("EffectParam名称","EffectParam名称");

		GELEM_OBJ(SlidewaySetting,slidewaysetting)
			GELEM_EDITOBJ("滑槽设定","滑槽设定");

	END_GOBJ();


};
