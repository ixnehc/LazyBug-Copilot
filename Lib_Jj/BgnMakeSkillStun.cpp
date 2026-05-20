/********************************************************************
	created:	2019/10/21 
	author:		cxi
	
	purpose:	快闪
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnMakeSkillStun.h"

#include "LevelOSB.h"
 
#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "Buff_SkillStun.h"

#include "Skill_GeneralAdvS.h"


////////////////////////////////////////////////////////////////////////
//CBgn_MakeSkillStun
BIND_BGN_CLASS(CBgn_MakeSkillStun,CBgp_MakeSkillStun);

  
void CBgn_MakeSkillStun::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_MakeSkillStun*pad=_GetPad<CBgp_MakeSkillStun>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelDecider *decider=ctx->level->GetDecider();
	CLevelObj *lo=ctx->lo;

	CLevelDecider::MakeSkillStunContext *ctxMakeSkillStun=decider->GetSkillStunContext();

	if (pad->_idSkill!=RecordID_Invalid)
	{
		if (ctxMakeSkillStun)
		{
			RecordID idRecBuff=ctx->level->GetRecords()->GetGlobal()->idDefBuff_SkillStun;
			if (idRecBuff!=RecordID_Invalid)
			{
				CLevelObj *loStunSrc=NULL;
				if (ctxMakeSkillStun->osbSrc)
					loStunSrc=ctxMakeSkillStun->osbSrc->GetRootOwner();

				BOOL bValidTarget=FALSE;
				LevelSkillTarget targetSkill;
				switch(pad->_tpTarget)
				{
					case CBgp_MakeSkillStun::TargetType_None:
					{
						bValidTarget=TRUE;
						break;
					}
					case CBgp_MakeSkillStun::TargetType_StunSrc:
					{
						if (loStunSrc)
						{
							targetSkill.SetObjID(loStunSrc->GetID());
							bValidTarget=TRUE;
						}
						break;
					}
					case CBgp_MakeSkillStun::TargetType_StunSrcPos:
					{
						if (loStunSrc)
						{
							targetSkill.SetPos(loStunSrc->GetFramePos());
							bValidTarget=TRUE;
						}
						break;
					}
					case CBgp_MakeSkillStun::TargetType_BackToStunSrc:
					{
						if (loStunSrc)
						{
							LevelPos posAim=lo->GetFramePos()*2.0f-loStunSrc->GetFramePos();
							targetSkill.SetAim(posAim);
							bValidTarget=TRUE;
						}
						break;
					}
				}

//				if (targetSkill.tp!=LevelSkillTarget::Target_None)
				if (bValidTarget)
				{
					extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
					CLevelSkill *skillCur=LevelUtil_GetCastingSkill(lo);
					if (skillCur)
					{
						LevelRecordSkill *recSkill=skillCur->GetRec();
						SkillParam_GeneralAdvS *param=recSkill->GetParam<SkillParam_GeneralAdvS>();
						if (param)
						{
							if (param->stages.size()==1)
							{
								int v=0;
								v++;
							}
						}
					}
					BuffArg_SkillStun arg;
					LevelBuffID idBuff=decider->MakeBuff(*(ctxMakeSkillStun->osbSrc),ctx->lo,idRecBuff,ANIMTICK_INFINITE,&arg,*ctxMakeSkillStun->link);
					if (idBuff!=LevelBuffID_Invalid)
					{
						CLevelSkillDriver *driver=ctx->lo->GetSkillDriver();
						if (driver)
							driver->StartCast(LevelSkillType(pad->_idSkill),targetSkill,1,NULL,ctxMakeSkillStun->link);

						ctxMakeSkillStun->idResultBuff=idBuff;
					}
				}

			}
		}
	}

	_OutputOk(outputs,1,"结束");
}
