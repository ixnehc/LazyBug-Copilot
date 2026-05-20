/********************************************************************
	created:	2019/12/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_Centipede_SpawnCyst.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoCentipede.h"


#include "Skill_GeneralAdvS.h"

#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgn_Centipede_SpawnCyst
BIND_BGN_CLASS(CBgn_Centipede_SpawnCyst,CBgp_Centipede_SpawnCyst);

void CBgn_Centipede_SpawnCyst::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Centipede_SpawnCyst*pad=_GetPad<CBgp_Centipede_SpawnCyst>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
	{
		CLoCentipede *loCentipede=(CLoCentipede *)ctx->lo;

		if (loCentipede->GetWorkingMode()==CentipedeWorkingMode_Combat)
		{
			CCentipedeState_Combat &stateCombat=loCentipede->GetCombatState();

			BOOL bLeft=TRUE;

			bLeft=loCentipede->IsLeftArm();
			if (pad->bInner)
				bLeft=!bLeft;

			DWORD nUnbroken=stateCombat.GetUnbroken();

			int idx=-1;
			if (TRUE)
			{

				int head=3;
				int tail=nUnbroken-2;

				if (head<=tail)
				{
					float wt=0;
					float sumWeight=0.0f;
					for (int i=tail;i>=head;i--)
					{
						wt+=1.0f;
						sumWeight+=wt;
					}

					float seed=CSysRandom::RandRange(0.0f,sumWeight);

					wt=0;
					sumWeight=0.0f;
					for (int i=tail;i>=head;i--)
					{
						wt+=1.0f;
						sumWeight+=wt;
						if (sumWeight>=seed)
						{
							idx=i;
							break;
						}
					}
					if (idx==-1)
						idx=head;
				}
			}

			if (idx>=0)
			{
				stateCombat.SpawnCyst(nUnbroken-1-idx,bLeft);
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}

