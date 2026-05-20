
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"



#include "../ILuaMachine.h"


class IAnimNode;
class IMano;

struct Prop_Mano:public GProperty
{
public:
	DEFINE_CLASS(Prop_Mano);


	virtual GProperty *Clone();
	virtual void DeleteThis();
	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_Mano;	}

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(Prop_Mano,1);
		GELEM_VAR_INIT(IMano*,mn,NULL);
	END_GOBJ();    

	IMano*mn;
};

