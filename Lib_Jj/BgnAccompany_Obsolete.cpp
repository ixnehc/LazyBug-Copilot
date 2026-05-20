/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelUtil.h"
#include "LevelPlayer.h"
#include "LevelObjMap.h"
#include "LevelBehavior.h"

#include "BgnAccompany_Obsolete.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "attr/attr.h"


#include "LevelSkillDriver.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Accompany_Obsolete
BIND_BGN_CLASS(CBgn_Accompany_Obsolete,CBgp_Accompany_Obsolete);

void CBgn_Accompany_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Accompany_Obsolete*pad=_GetPad<CBgp_Accompany_Obsolete>();


}


void CBgn_Accompany_Obsolete::Destroy()
{
}


void CBgn_Accompany_Obsolete::Break(BGNOutputs &outputs)
{
	Destroy();
}

