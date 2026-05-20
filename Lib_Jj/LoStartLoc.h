#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObjSrc.h"

struct LosStartLoc:public CLevelObjSrc
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosStartLoc,2);

	BEGIN_GOBJ_PURE(LosStartLoc,1);

	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return FALSE;	}

};
