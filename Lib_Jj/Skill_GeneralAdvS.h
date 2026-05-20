#pragma once

#include "LevelSkill.h"

#include "LevelEventMap.h"

#include "LevelGesture.h"

#include "anim/KeySet.h"
#include "valueset/valueset.h"

#include "LevelAttrs_Weak.h"

#include "Skill_General.h"
#include "Skill_GeneralC.h"

#include <set>


struct SkillParam_GeneralAdvS:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_GeneralAdvS);

	enum ObstacleMethod
	{
		ObstacleMethod_NotCheck=0,
		ObstacleMethod_StopAtStaticObstacleOrEnemyObstacle,
		ObstacleMethod_StopAtStaticObstacle,
		ObstacleMethod_StopAtStaticObstacleAndBumpEnemyObstacle,
		ObstacleMethod_AvoidPhysObstacle,
		ObstacleMethod_StopAtStaticObstacleOrDynObstacle,

		ObstacleMethod_ForceDword=0xffffffff,
	};

	struct Window
	{
		StringID nm;
		RecordID idOpenEo;
		RecordID idCloseEo;

		BEGIN_GOBJ_PURE_NESTED(Window,SkillParam_GeneralAdvS::Window,1);

			GELEM_VAR_INIT(StringID,nm,StringID_Invalid);GELEM_UID(1);
				GELEM_EDITVAR("名称",GVT_U,GSem(GSem_StringID,"技能窗口名称"),"名称");
			GELEM_VAR_INIT(RecordID,idOpenEo,RecordID_Invalid);
				GELEM_EDITVAR("开启Eo",GVT_U,GSem(GSem_RecordID,"eos"),"开启Eo");
			GELEM_VAR_INIT(RecordID,idCloseEo,RecordID_Invalid);
				GELEM_EDITVAR("关闭Eo",GVT_U,GSem(GSem_RecordID,"eos"),"关闭Eo");
		END_GOBJ();

	};

	enum PhysObstacleAvoidDir
	{
		PhysObstacleAvoidDir_Undefined=-100000,
		PhysObstacleAvoidDir_CCW=-1,
		PhysObstacleAvoidDir_CW=1,
	};

	struct OpEntryBase
	{
		enum Op
		{
			Op_None,
			Op_AllowCancel,
			Op_OverrideWeaks,
			Op_CleanOverrideWeaks,
			Op_Landing,//着陆
			Op_TakeOff,//起飞
			Op_SetFacingMode_None,
			Op_SetPathFacingMode_None,
			Op_OpenWindow,
			Op_CloseWindow,
			Op_SetFacingMode_Target,
			Op_SetPathFacingMode_Target,
			Op_AddWeaks,
			Op_RemoveWeaks,
			Op_BgHandler,
			Op_DisableTargetMatching,
			Op_SetObstacleMethod,
			Op_AddBuff,
			Op_RemoveBuff,
			Op_SetPathFacingMode_TargetFixedPos,

			Op_ForceDword=0xffffffff,
		};

		StringID nm;
		BOOL bEnable;
		Op op;
		WeaksEx weaks;
		BOOL bWeaksCanTakeOver;
		StringID nmWindow;
		StringID nmBgHandler;
		ObstacleMethod methodObstacle;
		std::vector<RecordID> buffs;
		BOOL bManageBuffDur;
	};

	struct OpEntry:public OpEntryBase
	{
		BEGIN_GOBJ_PURE_NESTED(OpEntry,SkillParam_GeneralAdvS::OpEntry,1);

			GELEM_VAR_INIT(StringID,nm,StringID_Invalid);GELEM_UID(1);
				GELEM_EDITVAR("触发事件",GVT_U,GSem(GSem_StringID,"动画事件"),"触发Operation的事件");
			GELEM_VAR_INIT(BOOL,bEnable,TRUE);GELEM_UID(2);
				GELEM_EDITVAR("是否有效",GVT_S,GSem_Boolean,"是否有效");
			GELEM_VAR_INIT(Op,op,Op_None);GELEM_UID(4);
				GELEM_EDITVAR("Op",GVT_U,GSem(GSem_Interger,
					"n/a:0" "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"允许取消:1"  "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"重载弱点:2"   "|窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"添加弱点:12"   "|窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"去除弱点:13"   "|窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"取消弱点修改:3"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"着陆:4"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"起飞:5"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"设置为不改变朝向:6"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"设置为不改变路径朝向:7"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"设置为朝向目标:10"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"设置为路径朝向目标:11"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"设置为路径朝向目标固定位置:19"   "|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"开启窗口:8"		"|弱点&弱点可接管&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"关闭窗口:9"		"|弱点&弱点可接管&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"行为图处理:14"		"|弱点&弱点可接管&窗口名称&障碍处理方式&Buff列表&管理Buff生命周期,"
					"禁用目标匹配:15"		"|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&Buff列表&管理Buff生命周期,"
					"设置障碍处理方式:16"		"|弱点&弱点可接管&窗口名称&行为图处理入口&Buff列表&管理Buff生命周期,"
					"添加Buff:17"		"|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式,"
					"清除Buff:18"		"|弱点&弱点可接管&窗口名称&行为图处理入口&障碍处理方式&管理Buff生命周期"
					),"操作");
			//XXXXX: More SkillParam_GeneralAdvS::OpEntry::Op
			GELEM_OBJ(WeaksEx,weaks);GELEM_UID(5);
				GELEM_EDITOBJ("弱点","弱点");
			GELEM_VAR_INIT(BOOL,bWeaksCanTakeOver,TRUE);GELEM_UID(6);
				GELEM_EDITVAR("弱点可接管",GVT_S,GSem_Boolean,"弱点可接管");
			GELEM_VAR_INIT(StringID,nmWindow,StringID_Invalid);GELEM_UID(7);
				GELEM_EDITVAR("窗口名称",GVT_U,GSem(GSem_StringID,"技能窗口名称"),"名称");
			GELEM_VAR_INIT( StringID,nmBgHandler,StringID_Invalid);	GELEM_UID(8);
				GELEM_EDITVAR( "行为图处理入口", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "一个行为图中继的名称,用来执行一段更新逻辑" );
			GELEM_VAR_INIT(ObstacleMethod,methodObstacle,ObstacleMethod_NotCheck);GELEM_UID(9);
				GELEM_EDITVAR("障碍处理方式",GVT_U,GSem(GSem_Interger,
					"不检测障碍:0"		","
					"遇到Static障碍或Enemy障碍停止移动:1"	","
					"遇到Static障碍停止移动:2"		","
					"遇到Static障碍停止移动并撞开Enemy障碍:3"		","
					"遇到Static障碍或动态障碍停止移动:5"	""
					),"障碍处理方式");
			GELEM_VARVECTOR_INIT(RecordID,buffs,RecordID_Invalid);GELEM_UID(12);
				GELEM_EDITVAR("Buff列表",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
			GELEM_VAR_INIT(BOOL,bManageBuffDur,TRUE);GELEM_UID(11);
				GELEM_EDITVAR("管理Buff生命周期",GVT_S,GSem_Boolean,"管理Buff生命周期(自动删除)");
		END_GOBJ();
	};

	struct Stage
	{
		struct TargetMatchOld
		{
			BEGIN_GOBJ_PURE_NESTED_UID2(TargetMatchOld,SkillParam_GeneralAdvS::Stage::TargetMatchOld,498,1);
				GELEM_VAR_INIT(BOOL,bEnable,FALSE); GELEM_UID(1);
					GELEM_EDITVAR("可用",GVT_S,GSem(GSem_Boolean,"最小速度缩放,最大速度缩放,目标距离偏移"),"可用");
				GELEM_VAR_INIT(float,scaleSpeedMin,0.0f);GELEM_UID(2);
					GELEM_EDITVAR("最小速度缩放",GVT_F,GSem(GSem_Float,"-2,10.0,0.05"),"最小速度缩放");
				GELEM_VAR_INIT(float,scaleSpeedMax,2.0f);GELEM_UID(3);
					GELEM_EDITVAR("最大速度缩放",GVT_F,GSem(GSem_Float,"-2,10.0,0.05"),"最大速度缩放");
				GELEM_VAR_INIT(float,distOff,0.0f);GELEM_UID(4);
					GELEM_EDITVAR("目标距离偏移",GVT_F,GSem(GSem_Float,"-10,10.0,0.05"),"目标距离偏移(米");
			END_GOBJ();
			
			BOOL bEnable;
			float scaleSpeedMin;
			float scaleSpeedMax;
			float distOff;
		};

		struct TargetMatch
		{
			BEGIN_GOBJ_PURE_NESTED_UID2(TargetMatch,SkillParam_GeneralAdvS::Stage::TargetMatch,499,1);
				GELEM_VAR_INIT(BOOL,bEnable,FALSE); GELEM_UID(1);
					GELEM_EDITVAR("可用",GVT_S,GSem(GSem_Boolean,"名称,最小速度缩放,最大速度缩放"),"可用");
				GELEM_VAR_INIT( StringID,nmEvent,StringID_Invalid);	
					GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"动画事件"), "TargetMatcher名称" );
				GELEM_VAR_INIT(float,scaleSpeedMin,0.0f);GELEM_UID(2);
					GELEM_EDITVAR("最小速度缩放",GVT_F,GSem(GSem_Float,"-100.0,100.0,0.05"),"最小速度缩放");
				GELEM_VAR_INIT(float,scaleSpeedMax,2.0f);GELEM_UID(3);
					GELEM_EDITVAR("最大速度缩放",GVT_F,GSem(GSem_Float,"-100.0,100.0,0.05"),"最大速度缩放");
			END_GOBJ();

			BOOL bEnable;
			StringID nmEvent;

			float scaleSpeedMin;
			float scaleSpeedMax;
		};

		struct HeightAdjust
		{
			enum Mode
			{
				Default=0,
				BlendToTargetOverStage=1,
				BlendToTargetOverDur=2,

				Mode_ForceDword=0xffffffff,
			};

			HeightAdjust()
			{
				GConstructor();

				blend.ResetFloat(0.0f);
				blend.AddFloat(1.0f,1.0f);
			}

			~HeightAdjust()
			{
				GDestructor();
			}


			BEGIN_GOBJ(SkillParam_GeneralAdvS::Stage::HeightAdjust,1);
				GELEM_VAR_INIT(Mode,mode,SkillParam_GeneralAdvS::Stage::HeightAdjust::Default); GELEM_UID(1);
					GELEM_EDITVAR("调整模式",GVT_S,GSem(GSem_Interger,
						"缺省模式:0"		"|目标高度&过渡曲线&过渡时间,"
						"在整个阶段内过渡到目标高度:1"	"|过渡时间,"
						"在指定时间内过渡到目标高度:2"	""
						),"调整模式");
				GELEM_VAR_INIT(float,htTarget,2.0f);GELEM_UID(2);
					GELEM_EDITVAR("目标高度",GVT_F,GSem(GSem_Float,"0,10.0,0.05"),"目标高度");
				GELEM_VAR_INIT(AnimTick,durBlend,ANIMTICK_FROM_SECOND(0.2f));GELEM_UID(4)
					GELEM_EDITVAR("过渡时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.01"),"过渡时间");
				GELEM_OBJVAR( ValueSet, blend);GELEM_UID(3);
					GELEM_EDITOBJ_EX("过渡曲线","过渡曲线",GSem( GSem_Unknown, "0,0,1,1" ));
			END_GOBJ();

			Mode mode;
			float htTarget;
			AnimTick durBlend;
			ValueSet blend;
		};

		enum WorkingMode
		{
			WorkingMode_AnimDriven_Default_Old,
			WorkingMode_AnimDriven_RotateOnSpot,
			WorkingMode_BuffDriven_WS,//WS: World Space
			WorkingMode_CodeDriven_MoveAlong,
			WorkingMode_AnimDriven_Default,

			WorkingMode_ForceDword=0xffffffff, 
		};

		StringID nm;
		unsigned __int64 cast;
		RecordID idPathRes;
		RecordID idBuff;
		AnimTick dur;
		AnimTick durVar;

		WorkingMode modeWork;

		LevelSkillTargetFacingMode modeInitialFacing;
		float angleMaxInitialFacingAdjust;
		LevelSkillTargetFacingMode modeInitialPathFacing;
		float angleMaxInitialPathFacingAdjust;
		LevelSkillTargetFacingMode modeFacing;
		float speedMaxFacingAdjust;
		LevelSkillTargetFacingMode modePathFacing;
		float speedMaxPathFacingAdjust;
		LevelSkillTargetFacingMode modeRotateOnSpotFacing;

		StringID nmTargetAlignerEvent;

		float speedMoveAlong;

		TargetMatchOld targetmatchOld;
		TargetMatch targetmatch;
		HeightAdjust heightadjust;

		StringID nmUpdater;

		BEGIN_GOBJ_PURE_NESTED(Stage,SkillParam_GeneralAdvS::Stage,1);

			GELEM_VAR_INIT(StringID,nm,StringID_Invalid);GELEM_UID(17);
				GELEM_EDITVAR("阶段名称",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");

			GELEM_VAR_INIT(unsigned __int64,cast,0);
				GELEM_EDITVAR("技能释放效果",GVT_Bx8,GSem_ProtoPath,"技能释放效果");

			GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));GELEM_UID(19);
				GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,1000,0.1"),"如果为0,由路径持续时间决定");
			GELEM_VAR_INIT(AnimTick,durVar,ANIMTICK_FROM_SECOND(0.0f));GELEM_UID(22);
				GELEM_EDITVAR("持续时间浮动",GVT_U,GSem(GSem_AnimTick,"0,1000,0.1"),"持续时间浮动");

			GELEM_VAR_INIT(WorkingMode,modeWork,WorkingMode_AnimDriven_Default_Old); GELEM_UID(3);
				GELEM_EDITVAR("工作模式",GVT_U,GSem(GSem_Interger,
					"缺省(动画驱动)_Old:0" "|原地转身朝向模式&移动控制Buff&移动速度&TargetAligner名称&目标匹配,"
					"缺省(动画驱动):4" "|原地转身朝向模式&移动控制Buff&移动速度&目标匹配(Old),"
					"原地转身(动画驱动):1" "|移动控制Buff&初始朝向模式&最大初始朝向调整角度&初始路径朝向模式&最大初始路径朝向调整角度&朝向模式&最大朝向调整速度&路径朝向模式&最大路径朝向调整速度&目标匹配(Old)&目标匹配&移动速度&TargetAligner名称,"
					"缺省(Buff驱动):2" "|移动路径资源&原地转身朝向模式&初始朝向模式&最大初始朝向调整角度&初始路径朝向模式&最大初始路径朝向调整角度&朝向模式&最大朝向调整速度&路径朝向模式&最大路径朝向调整速度&目标匹配(Old)&目标匹配&移动速度&TargetAligner名称,"
					"移动到目标(Code驱动):3" "|原地转身朝向模式&移动控制Buff&移动路径资源&原地转身朝向模式&目标匹配(Old)&目标匹配&TargetAligner名称"),
					"工作模式");

			GELEM_VAR_INIT(RecordID,idPathRes,RecordID_Invalid); GELEM_UID(18);
				GELEM_EDITVAR("移动路径资源",GVT_U,GSem(GSem_RecordID,"resources"),"移动路径资源");

			GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid); GELEM_UID(23);
				GELEM_EDITVAR("移动控制Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"移动控制Buff");

			GELEM_VAR_INIT( StringID,nmTargetAlignerEvent,StringID_Invalid);	
				GELEM_EDITVAR( "TargetAligner名称", GVT_U, GSem(GSem_StringID,"动画事件"), "TargetAligner名称" );

			GELEM_VAR_INIT(LevelSkillTargetFacingMode,modeInitialFacing,LevelSkillTargetFacingMode_None); GELEM_UID(4);
				GELEM_EDITVAR("初始朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"初始朝向模式");
			GELEM_VAR_INIT(float,angleMaxInitialFacingAdjust,180.0f);GELEM_UID(5);
				GELEM_EDITVAR("最大初始朝向调整角度",GVT_F,GSem(GSem_Float,"0.0,180.0,0.05"),"最大初始朝向调整角度");
			GELEM_VAR_INIT(LevelSkillTargetFacingMode,modeInitialPathFacing,LevelSkillTargetFacingMode_None); GELEM_UID(6);
				GELEM_EDITVAR("初始路径朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"初始路径朝向模式");
			GELEM_VAR_INIT(float,angleMaxInitialPathFacingAdjust,180.0f);GELEM_UID(7);
				GELEM_EDITVAR("最大初始路径朝向调整角度",GVT_F,GSem(GSem_Float,"0.0,180.0,0.05"),"最大初始路径朝向调整角度");
			GELEM_VAR_INIT(LevelSkillTargetFacingMode,modeFacing,LevelSkillTargetFacingMode_None); GELEM_UID(8);
				GELEM_EDITVAR("朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"朝向模式");
			GELEM_VAR_INIT(float,speedMaxFacingAdjust,1800.0f);GELEM_UID(9);
				GELEM_EDITVAR("最大朝向调整速度",GVT_F,GSem(GSem_Float,"0.0,1800.0,0.05"),"最大朝向调整速度");
			GELEM_VAR_INIT(LevelSkillTargetFacingMode,modePathFacing,LevelSkillTargetFacingMode_None); GELEM_UID(10);
				GELEM_EDITVAR("路径朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"路径朝向模式");
			GELEM_VAR_INIT(float,speedMaxPathFacingAdjust,1800.0f);GELEM_UID(11);
				GELEM_EDITVAR("最大路径朝向调整速度",GVT_F,GSem(GSem_Float,"0.0,1800.0,0.05"),"最大路径朝向调整速度");
			GELEM_VAR_INIT(float,speedMoveAlong,5.0f);GELEM_UID(24);
				GELEM_EDITVAR("移动速度",GVT_F,GSem(GSem_Float,"0.0,100.0,0.05"),"移动速度");
			GELEM_OBJ(TargetMatchOld,targetmatchOld);GELEM_UID(13);
				GELEM_EDITOBJ("目标匹配(Old)","根据目标调整位置");
			GELEM_OBJ(TargetMatch,targetmatch);GELEM_UID(25);
				GELEM_EDITOBJ("目标匹配","根据目标调整位置");
			GELEM_OBJ(HeightAdjust,heightadjust);GELEM_UID(21);
				GELEM_EDITOBJ("高度调整","高度调整");
			GELEM_VAR_INIT(LevelSkillTargetFacingMode,modeRotateOnSpotFacing,LevelSkillTargetFacingMode_None); GELEM_UID(16);
				GELEM_EDITVAR("原地转身朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"原地转身朝向模式");
			GELEM_VAR_INIT( StringID,nmUpdater,StringID_Invalid);	GELEM_UID(20);
				GELEM_EDITVAR( "更新器", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "一个行为图中继的名称,用来执行一段更新逻辑" );

		//NEXT GELEM_UID(26);
		END_GOBJ();

	};

	BEGIN_GOBJ_PURE(SkillParam_GeneralAdvS,1); 

		GELEM_OBJVECTOR(Stage,stages);GELEM_UID(5);
			GELEM_EDITOBJ("各阶段参数","各阶段参数");
		GELEM_VAR_INIT( StringID,nmStageSwitcher,StringID_Invalid);	GELEM_UID(7);
			GELEM_EDITVAR( "阶段切换器名称", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "一个行为图中继的名称,用来执行一段切换阶段的逻辑" );
		GELEM_OBJVECTOR(GeneralSkillEoEntry,entriesEo);GELEM_UID(3);
			GELEM_EDITOBJ("EO参数","EO参数");
		GELEM_OBJVECTOR(OpEntry,entriesOp);GELEM_UID(4);
			GELEM_EDITOBJ("Op参数(Event)","Op参数");
		GELEM_OBJVECTOR(Window,windows);GELEM_UID(8);
			GELEM_EDITOBJ("窗口参数","窗口参数");
		GELEM_VAR_INIT(ObstacleMethod,methodObstacle,ObstacleMethod_NotCheck);GELEM_UID(6);
			GELEM_EDITVAR("障碍处理方式",GVT_U,GSem(GSem_Interger,
				"不检测障碍:0"		"|绕障距离&绕障方向,"
				"遇到Static障碍或Enemy障碍停止移动:1"	"|绕障距离&绕障方向,"
				"遇到Static障碍停止移动:2"		"|绕障距离&绕障方向,"
				"遇到Static障碍停止移动并撞开Enemy障碍:3"		"|绕障距离&绕障方向,"
				"绕开Phys障碍:4"			","
				"遇到Static障碍或动态障碍停止移动:5"	"|绕障距离&绕障方向"
				),"障碍处理方式");
		GELEM_VAR_INIT(float,distAvoid,2.0f);GELEM_UID(9);
			GELEM_EDITVAR("绕障距离",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"绕障距离");
		GELEM_VAR_INIT(PhysObstacleAvoidDir,dirAvoid,PhysObstacleAvoidDir_CCW); GELEM_UID(10);
			GELEM_EDITVAR("绕障方向",GVT_U,GSem(GSem_Interger,"逆时针:-1,顺时针:1"),"绕障方向");

	END_GOBJ();

	Stage *FindStage(StringID nm)
	{
		for (int i=0;i<stages.size();i++)
		{
			if (stages[i].nm==nm)
				return &stages[i];
		}
		return NULL;
	}

	StringID nmStageSwitcher;
	std::vector<Stage> stages;

	std::vector<GeneralSkillEoEntry> entriesEo;
	std::vector<OpEntry> entriesOp;
	std::vector<Window> windows;

	ObstacleMethod methodObstacle;
	float distAvoid;
	PhysObstacleAvoidDir dirAvoid;

};

struct LevelPathesEvent;
struct LevelPath;
class CSkillGesture_PathS;
class Skill_GeneralAdvS:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_GeneralAdvS,36);

	Skill_GeneralAdvS()
	{
		_ges=NULL;
		_tCasting=0;

		_tXfmAnim=0;

		_ht=0.0f;
		_face=0.0f;
		_faceTargetAligner=0.0f;

		_methodObstacle=SkillParam_GeneralAdvS::ObstacleMethod_NotCheck;

		_modeFacing=LevelSkillTargetFacingMode_None;
		_modePathFacing=LevelSkillTargetFacingMode_None;
		_yawPathFacing=0.0f;

		_scaleHt=1.0f;

		_dirPhysObstacleAvoid=SkillParam_GeneralAdvS::PhysObstacleAvoidDir_Undefined;

		_idRecentDynObstacle=LevelObjID_Invalid;

		_bAllowCancel=FALSE;

		_bFinishing=FALSE;

	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim)|(1<<LevelSkillTarget::Target_None)|(1<<LevelSkillTarget::Target_DefObj)|(1<<LevelSkillTarget::Target_FixPosAndObj)|(1<<LevelSkillTarget::Target_Pos);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Control;	}
	virtual AnimTick GetCastingTime()	{		return _tCasting;	}//返回经过IAS修正的casting time

	virtual BOOL CheckCastingEvent(StringID nmEvent)	{		return _events.CheckCastingEvent(nmEvent);}
	virtual AnimTick GetCastingEventTime(StringID nmEvent);

	virtual void StopCast(AnimTick tStop);

	void GetCastingPos(LevelPos &pos) override;
	void GetCastingPos3D(LevelPos3D &pos3D) override;
	LevelFace GetCastingFace() override;

	void SetPathFacingYaw(LevelFaceYaw yaw)	{		_yawPathFacing=yaw;	}
	void SetPhysObstacleAvoidDir(SkillParam_GeneralAdvS::PhysObstacleAvoidDir dir)	{		_dirPhysObstacleAvoid=dir;	}

	BOOL CheckEventWindow(StringID nmOpen,StringID nmClose);
	BOOL ExistStage()	{		return !_stage.IsEmpty();	}
	BOOL CheckInStage(StringID nmStage);
	SkillParam_GeneralAdvS::Stage *GetStageParam()	{		return _stage.param;	}
	StringID GetStageNameID();
	AnimTick GetStageAge();
	void OnEvent(StringID nmEvent,AnimEventZone *ezone,AnimTick t);

	void DoSwitchStage(StringID nmStage);
	void OnOp(SkillParam_GeneralAdvS::OpEntry &entryOp,AnimEventZone *ezone)	{		_OnOp(entryOp,ezone,NULL);	}

	LevelObjID GetRecentDynObstacle()	{		return _idRecentDynObstacle;	}

protected:
	void _OnStart_Internal();
	virtual void _OnStart() override;
	virtual void _OnStart(LevelOpLink &link) override;
	virtual void _OnBreak();
	virtual void _OnUpdate(AnimTick dt);

	virtual BOOL _WriteSyncData(CBitPacket *bp) override;

	virtual void _OnFinish()	{		_Finish();	}

	void _OnOp(SkillParam_GeneralAdvS::OpEntryBase &entryOp,AnimEventZone *ezone,LevelOpLink *link);
	void _CleanOverrideWeaks();

	virtual BOOL CanCancel()	 override{		return _bAllowCancel;	}
	virtual void Cancel() override;


	void _CalcAnimXfm(i_math::xformf &xfm,AnimTick t);
	LevelFace _AdjustFacing(LevelFace face,LevelSkillTargetFacingMode mode,float speedMaxAdjust,AnimTick dt,LevelFaceYaw yaw=0.0f);

	void _DoSwitchStage(int iStage);

	void _Finish();

	AnimTick _tCasting;

	AnimTick _tXfmAnim;
	i_math::xformf _xfmAnim;
	LevelPos _pos;
	float _ht;//相对于地表的高度,不是世界空间里的绝对高度
	LevelFace _face;
	LevelFace _faceTargetAligner;

	CSkillGesture_PathS *_ges;

	void _UpdateEvents();
	CSkillGeneralEvents _events;

	BOOL _bAllowCancel;
	SkillParam_GeneralAdvS::ObstacleMethod _methodObstacle;
	LevelSkillTargetFacingMode _modeFacing;
	LevelSkillTargetFacingMode _modePathFacing;
	BOOL _bTargetMatching;
	LevelFaceYaw _yawPathFacing;
	float _scaleHt;
	SkillParam_GeneralAdvS::PhysObstacleAvoidDir _dirPhysObstacleAvoid;

	//Recent Obstacle
	LevelObjID _idRecentDynObstacle;


	void _OpenWindow(StringID nmWindow);
	void _CloseWindow(StringID nmWindow);
	void _CloseAllWindows();
	SkillParam_GeneralAdvS::Window *_FindWindowParam(StringID nmWindow);
	std::unordered_set<StringID> _windows;

	BOOL _bFinishing;

	struct StageState
	{
		StageState()
		{
			Zero();
			ver=0;
		}
		void Zero()
		{
			param=NULL;
			idxParam=0;
			tStart=0;
			path=NULL;
			dur=0;
			scaleFaceRotate=1.0f;
			posStageStart.set(0.0f,0.0f);
			htStart=0.0f;
			idBuff=LevelBuffID_Invalid;
			tAutoCleanOverrideWeaks=0;
			ezoneTargetAligner=NULL;
			ezoneTargetMatcher=NULL;
		}
		BOOL IsEmpty()		{			return param==NULL;		}

		//Working data
		SkillParam_GeneralAdvS::Stage *param;
		int idxParam;
		LevelPath *path;
		AnimTick tStart;
		AnimTick dur;
		LevelPos posStageStart;
		float scaleFaceRotate;
		float htStart;
		LevelBuffID idBuff;
		AnimTick tAutoCleanOverrideWeaks;
		AnimEventZone * ezoneTargetAligner;
		AnimEventZone * ezoneTargetMatcher;
		struct EventBuff
		{
			EventBuff()
			{
				Zero();
			}
			void Zero()
			{
				memset(this,0,sizeof(*this));
			}
			LevelBuffID idBuff;
			AnimTick tAutoClean;
		};
		std::deque<EventBuff> buffsEvent;
		DWORD ver;
	};
	BOOL _TriggerBhvSwitcher();
	void _StartStage(SkillParam_GeneralAdvS::Stage *paramStage,int idxParam,AnimTick tStart);
	void _UpdateStage(AnimTick t,AnimTick dt);
	void _EndStage();
	void _ClearEventBuff(AnimTick t,StageState::EventBuff &entry);
	BOOL _CalcTargetAlignerFace(AnimTick t,LevelFace faceBase,LevelFace &face);

	StageState _stage;

	void _CallBehaviorRelay(StringID nmRelay,LevelOpLink *link);

};

