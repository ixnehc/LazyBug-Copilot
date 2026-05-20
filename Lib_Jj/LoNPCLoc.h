#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObjSrc.h"

struct LosNPCLoc:public CLevelObjSrc
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosNPCLoc,11);

	BEGIN_GOBJ_PURE(LosNPCLoc,1);

		GELEM_ALLOWDISABLE();

		GELEM_VAR_INIT(RecordID,idNPC,RecordID_Invalid);
			GELEM_EDITVAR("NPC表格项",GVT_U,GSem(GSem_RecordID,"npcs"),"指明是哪个NPC的产生位置");
	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return FALSE;	}

	RecordID idNPC;


};
