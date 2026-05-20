
#pragma once

#include "gds/GDefines.h"

enum GVarTypeEx
{
	GVTEx_Start=GVT_Max,

	GVTEx_AEL,
	GVTEx_ProtoNode,
	GVTEx_CppTbl_obsolete,
	GVTEx_Sheet,
	GVTEx_SheetRow,

	GVTEx_BoneLink,
	GVTEx_AnimNode,

	GVTEx_Input,

	GVTEx_PhysEvent,
	GVTEx_AnimEvent,

	GVTEx_CameraLink,

	GVTEx_Max,
};
