#pragma once

#include "GuiLib.h"

#include "protograph.h"

struct GuiLib_Api GuiData_ProtoLogic:public GeData
{
	virtual const char *GetName()	{		return "proto_logic";	}
	GuiData_ProtoLogic()
	{
		Zero();
	}

	void Zero()
	{
		bAssetOrProto=FALSE;
	}

	void Clear()
	{
		graph.Clear();
		Zero();
	}

	CProtoGraph graph;

	std::string drops;//drop的protoes的路径
	i_math::pos2di ptDrop;//drop的起始位置
	BOOL bAssetOrProto;

};
