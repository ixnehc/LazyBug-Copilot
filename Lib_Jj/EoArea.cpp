
#include "stdh.h"

#include "Level.h"
#include "LevelUtil.h"

#include "EoArea.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoArea,EoParamArea);


void EoArea::_OnUpdate()
{
	EoParamArea *param=GetParam<EoParamArea>();
	if (!param)
		return;

	if (param->bBindSkill)
	{
		if (_GetSkillCastingTime()==ANIMTICK_INFINITE)
		{
			DeferDestroy();
			return;
		}
	}

	_iCycle++;
	if (_iCycle<param->cycle)
		return;

	_iCycle=0;

	if (TRUE)
	{
		LevelUtilDetectParam paramDetect;
		paramDetect.loSrc=this;
		paramDetect.pos=GetFramePos();
		paramDetect.rangeMin=0.0f;
		paramDetect.rangeMax=param->radius;
		paramDetect.flags=&param->flagsDetect[0];
		paramDetect.nFlags=param->flagsDetect.size();
		paramDetect.requires=&param->requires[0];
		paramDetect.nRequires=param->requires.size();

		DWORD c;
		CLevelObj **los=LevelUtil_Detect(paramDetect,NULL,c);

		for (int i=0;i<c;i++)
		{
			CLevelObj *loTarget=los[i];

			DealArg arg;
			arg.link.id=GetLevel()->GenOpLinkID();
			arg.grd=_grd;

			_MakeDeals(loTarget,arg);
		}
	}

	if (_level->GetT_()>_tCreate+param->dur+ANIMTICK_FROM_SECOND(1.0f))
	{
		DeferDestroy();
	}
}

