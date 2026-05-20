
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_Weaks.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Weaks,BuffParam_Weaks,BuffArg_Weaks);

DWORD ParseWeaks(DamageAttrMask &weaks0,DamageAttrMask *bufWeaks)
{
	DamageAttrMask weaks=weaks0;
	DWORD c=0;

	DWORD flag=1;
	while(weaks!=0)
	{
		if (weaks&flag)
			bufWeaks[c++]=(DamageAttrMask)flag;
		weaks&=~flag;
		flag=flag<<1;
	}
	return c;
}

void Buff_Weaks::_OnCreate(LevelBuffArg *arg)
{
	BuffParam_Weaks *param=(BuffParam_Weaks *)_param;
	for (int i=LevelWeakCategory_Start;i<LevelWeakCategory_Count;i++)
	{
		LevelWeakCategory category=((LevelWeakCategory)i);
		if (param->categories&LevelWeakCategory_GetMask(category))
		{
			DamageAttrMask weaks=0;
			switch(category)
			{
				case LevelWeakCategory_StunFwd:
					weaks=param->stunFwd;
					break;
				case LevelWeakCategory_StunSide:
					weaks=param->stunSide;
					break;
				case LevelWeakCategory_StunBack:
					weaks=param->stunBack;
					break;
				case LevelWeakCategory_KB:
					weaks=param->kb;
					break;
			}

			DamageAttrMask bufWeaks[DamageAttrType_Max];

			DWORD nWeaks=ParseWeaks(weaks,bufWeaks);

			for (int j=0;j<nWeaks;j++)
			{
				WeakStatus status;
				status.category=category;
				status.weak=bufWeaks[j];

				status.nBroken=0;
				status.tRecentBroken=ANIMTICK_INFINITE;
				_statusWeaks.push_back(status);
			}

		}
	}
}

void Buff_Weaks::_OnDestroy()
{
	_statusWeaks.clear();
}

void Buff_Weaks::HandleEvent(LevelEvent &e0)
{
	BuffParam_Weaks *param=(BuffParam_Weaks *)_param;

	__super::HandleEvent(e0);
	if (e0.bHandled)
		return;

	AnimTick tCur=_GetLevel()->GetT_();

	if (e0.GetType()==LET_ModWeaks)
	{
		LeModWeaks &e=(LeModWeaks &)e0;
		if (e.loTarget==_GetOwner())
		{
			for (int i=0;i<_statusWeaks.size();i++)
			{
				WeakStatus &status=_statusWeaks[i];

				BOOL bPassivated=FALSE;
				if (param->nPsvtReq>0)//可以钝化
				{
					if (status.nBroken>=param->nPsvtReq)
					{
						if (param->durPsvtCD>0)//钝化后可以恢复
						{
							if (tCur<status.tRecentBroken+param->durPsvtCD)
								bPassivated=TRUE;//钝化后尚未恢复
						}
						else
							bPassivated=TRUE;//钝化后永不恢复
					}
				}

				if (!bPassivated)
					e.attrWeaks->Cur().weaks[status.category]|=status.weak;
			}
		}
	}

	if (e0.GetType()==LET_NotifyWeaksBroken)
	{
		LeNotifyWeaksBroken &e=(LeNotifyWeaksBroken&)e0;
		if (e.loTarget==_GetOwner())
		{
			for (int i=0;i<_statusWeaks.size();i++)
			{
				WeakStatus &status=_statusWeaks[i];
				if (status.category==e.category)
				{
					if (status.weak&e.weaks)
					{
						if (status.nBroken>0)
						{
							if (param->durPsvtCD>0)
							{
								if (tCur>status.tRecentBroken+param->durPsvtCD)
									status.nBroken=0;
							}
						}
						status.nBroken++;
						status.tRecentBroken=tCur;
					}
				}
			}
		}
	}


}
