/********************************************************************
	created:	2022/2/12 
	purpose:	GA功能:交换道具
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "LevelUtil.h"

#include "BgnGA_ExchangeItem.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ExchangeItem
BIND_BGN_CLASS(CBgnGA_ExchangeItem,CBgpGA_ExchangeItem);


void CBgnGA_ExchangeItem::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ExchangeItem*pad=_GetPad<CBgpGA_ExchangeItem>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (pad->varItem)
	{
		RecordID idItem;
		if (_GetMem()->GetID(pad->varItem,BehaviorMemType_ItemRecord,idItem))
		{
			CLevelPlayer *player=_GetTalkPlayer();
			if (player)
			{
				LevelPlayerStates *lps=player->GetLPS();

				if (idItem!=RecordID_Invalid)
				{
					LevelRecordItem *recItem=_GetLevel()->GetRecords()->GetItem(idItem);
					if (recItem)
					{
						if (recItem->tpArtifact!=LevelArtifact_None)
						{
							RecordID idItemExchanged=RecordID_Invalid;
							if(TRUE)
							{
								EquipPart part=LevelUtil_GetItemEquipPart(level,idItem);
								if (part==EquipPart_Weapon)
								{
									RecordID idEquip,idCarry;
									idEquip=LevelUtil_GetEquippingWeapon(_GetTalkLo(),&idCarry);
									if ((idEquip!=RecordID_Invalid)&&(idCarry!=RecordID_Invalid))
										idItemExchanged=idEquip;
								}
								if ((part==EquipPart_Armor)||(part==EquipPart_Shield))
									idItemExchanged=LevelUtil_GetEquippingNonWeapon(_GetTalkLo(),part);
							}
							if (idItemExchanged!=RecordID_Invalid)
								LevelUtil_RemoveEquip(player,idItemExchanged);
							LevelUtil_AddArtifact(player,idItem,1);
							_GetMem()->SetID(pad->varItem,BehaviorMemType_ItemRecord,idItemExchanged);
						}
					}
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
