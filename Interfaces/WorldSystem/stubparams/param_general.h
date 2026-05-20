
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"


#include "editor/ctrlop.h"
#include "../../PhysicsSystem/IPhysicsSystemDefines.h"
#include "../../RenderSystem/IRenderSystemDefines.h"

#include "../ILuaMachine.h"


struct PropInput:public GProperty
{
	DEFINE_CLASS(PropInput);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(PropInput,1);
		GELEM_VAR(CtrlOp,op);
	END_GOBJ();    

	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_Input;	}

	CtrlOp op;
};



