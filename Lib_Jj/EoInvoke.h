#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Invoke 59

struct EoParamInvoke:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamInvoke);

	EoParamInvoke()
	{
	}

	~EoParamInvoke()
	{
	}


	BEGIN_GOBJ(EoParamInvoke,1);


	END_GOBJ();


};



class EoInvoke:public CLoEffectObj
{
public:
	EoInvoke()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoInvoke,CLASSUID_Invoke);

	virtual const char *GetShowName()	{		return "Invoke";	}

protected:
	virtual void _OnPostCreate();
	virtual void _OnUpdate();
	virtual void _OnDetroy();


};
