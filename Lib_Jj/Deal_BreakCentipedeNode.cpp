#include "stdh.h"

#include "Deal_BreakCentipedeNode.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "behaviorgraph/BehaviorMem.h"
#include "LevelBehavior.h"

#include "Level.h"

#include "LoCentipede.h"

BIND_DEAL(Deal_BreakCentipedeNode);


void Deal_BreakCentipedeNode::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (level) 
	{
		CLevelBehavior *bhv=loTarget->GetBehaviorAI();

		if (varCentipedeAgent!=StringID_Invalid)
		{
			if (bhv)
			{
				CBehaviorMem *mem=bhv->GetMem(0);
				if (mem)
				{
					LevelObjID idCentipedeAgent;
					if (mem->GetID(varCentipedeAgent,BehaviorMemType_ObjID,idCentipedeAgent))
					{
						extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
						CLevelObj *loAgent=LevelUtil_GetAliveLo(level,idCentipedeAgent);
						if (loAgent)
						{
							if (loAgent->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
								((CLoCentipede*)loAgent)->Break(loTarget->GetID(),arg.link);
						}
					}
				}
			}
		}
	}
}
