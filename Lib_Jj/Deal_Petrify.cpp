#include "stdh.h"

#include "Deal_Petrify.h"

#include "LevelStrike.h"
#include "LevelOSB.h"

#include "Level.h"

#include "Buff_Petrify.h"
#include "Buff_Petrified.h"
#include "LevelRecords.h"

#include "Log/LogDump.h"
#include "LevelRecordBuff.h"

BIND_DEAL(Deal_Petrify);

extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);

void Deal_Petrify::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	CLevel *level=osbSrc.GetLevel();
	if (!level)
		return;
	extern BOOL LevelUtil_CheckDamageImmune(CLevelObj *lo);
	if (LevelUtil_CheckDamageImmune(loTarget))
		return;
	CLevelRecords *records=level->GetRecords();

	if (idPetrified!=RecordID_Invalid)
	{
		if (LevelUtil_FindBuffByRecordID(loTarget,idPetrified))
			return;//已经凝固了
	}

	CLevelBuff *buff=NULL;

	if (idPetrify!=RecordID_Invalid)
	{
		buff=LevelUtil_FindBuffByRecordID(loTarget,idPetrify);
		if (buff)
		{
			if (!buff->GetClass()->IsSameWith(Class_Ptr2(Buff_Petrify)))
			{
				LevelRecordBuff *rec=records->GetBuff(idPetrify);
				assert(rec);
				LOG_DUMP_1P("Deal_Petrify",Log_Error,"使用错误的石化Buff:%s",rec->Name.c_str());
				return;
			}
		}

		if (!buff)
		{
			BuffArg_Petrify argBuff;
			argBuff.str=str;
			level->GetDecider()->MakeBuff(osbSrc,loTarget,idPetrify,0,&argBuff,arg.link);
			buff=LevelUtil_FindBuffByRecordID(loTarget,idPetrify);
		}
		else
		{
			((Buff_Petrify*)buff)->IncStr(str);
		}
	}

	BOOL bPetrified=FALSE;
	if (buff)
		bPetrified=((Buff_Petrify*)buff)->IsPetrified();

	if (bPetrified)
	{//凝固了
		((Buff_Petrify*)buff)->StopDamp();
		if (idPetrified!=RecordID_Invalid)
		{
			BuffArg_Petrified argBuff;
			level->GetDecider()->MakeBuff(osbSrc,loTarget,idPetrified,0,&argBuff,arg.link);
		}
	}


}
