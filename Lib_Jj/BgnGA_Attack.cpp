/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnGA_Attack.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"

#include "LoGeneralAgent.h"

#include "Log/LogDump.h"
#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgnGA_AttackPos
BIND_BGN_CLASS(CBgnGA_AttackPos,CBgpGA_AttackPos);


void CBgnGA_AttackPos::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_AttackPos*pad=_GetPad<CBgpGA_AttackPos>();
	CLevelObj *lo=_GetLo();

	LevelBehaviorContext *ctx=_GetCtx();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	BOOL bCasted=FALSE;

	if (TRUE)
	{
		LevelBehaviorContext *ctx=_GetCtx();

		BOOL bLS;
		std::vector<i_math::matrix43f>*sites=NULL;
		BP_MatSet *bp=&pad->sites;
		if (bp)
		{
			sites=&bp->data;
			bLS=!bp->bWS;
		}
		if (sites)
		{
			//立即开始攻击
			LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
			if (recSkill)
			{
				int nChooses=pad->nChooses;
				if (nChooses==0)
					nChooses=sites->size();
				if (nChooses>sites->size())
					nChooses=sites->size();
				LevelSkillArg arg;
				arg.sites.resize(nChooses);
				std::vector<int>indices;
				CSysRandom::GenRandomIndices(indices,nChooses);
				for (int i=0;i<nChooses;i++)
				{
					i_math::matrix43f mat=(*sites)[indices[i]];
					if (bLS)
						mat=mat*lo->GetLos()->GetMat();
					LevelPos pos(mat.getTranslationP()->x,mat.getTranslationP()->z);
					arg.sites[i]=pos;
				}
				LevelSkillTarget target;
				driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,&arg);

				bCasted=TRUE;
			}
		}
	}

	if(!bCasted)
	{
		LOG_DUMP_1P("CBgnGA_AttackPos",Log_Error,"无法施放技能(没有指定技能ID或者无法找到施放对象!)(行为图:%s)",StrLib_GetStr(ctx->bg->GetName()));

		_OutputOk(outputs,1,"结束");
		return;
	}
}

void CBgnGA_AttackPos::Update(BGNOutputs &outputs)
{
	CBgpGA_AttackPos*pad=_GetPad<CBgpGA_AttackPos>();

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		if (!driver->IsWorking())
		{
			_OutputOk(outputs,1,"结束");
			return;
		}
	}
}
