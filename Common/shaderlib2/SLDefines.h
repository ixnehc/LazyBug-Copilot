#pragma once

#include <vector>
#include <string>



enum ShaderVarStage
{
	SVS_None,
	SVS_VsIn,
	SVS_Vs,
	SVS_PsIn,
	SVS_Ps,
	SVS_Out,
};

enum ShaderVarType
{
	SVT_None,
	SVT_int,
	SVT_intx2,
	SVT_intx3,
	SVT_intx4,
	SVT_float,
	SVT_floatx2,
	SVT_floatx3,
	SVT_floatx4,
	SVT_matrix43,
	SVT_matrix44,
};

enum ShaderVarSem
{
	SVSem_NONE,
	SVSem_POSITION0,
	SVSem_POSITION1,
	SVSem_NORMAL0,
	SVSem_NORMAL1,
	SVSem_BINORMAL0,
	SVSem_BINORMAL1,
	SVSem_TANGENT0,
	SVSem_TANGENT1,
	SVSem_PSIZE0,
	SVSem_COLOR0,
	SVSem_COLOR1,
	SVSem_BLENDWEIGHT0,
	SVSem_BLENDINDICES0,
	SVSem_TEXTURE0,
	SVSem_TEXTURE1,
	SVSem_TEXTURE2,
	SVSem_TEXTURE3,
	SVSem_TEXTURE4,
	SVSem_TEXTURE5,
	SVSem_TEXTURE6,
	SVSem_TEXTURE7,
};

enum ShaderPlatform
{
	ShaderPlatform_None=0,
	ShaderPlatform_DX9=1,
	ShaderPlatform_DX10=2,
	ShaderPlatform_DX11=4,

	ShaderPlatform_forcedword=0xffffffff,
};

enum ShaderFeatureFlag
{
	SFF_None=0,
	SFF_Base=1,//An Always-be-present feature
	SFF_MtrlEdit=2,//this feature could be edit when editing a mtrl
	SFF_MtrlRepair=4,

	SFF_All=0xffffffff,

	ShaderFeatureFlag_forcedword=0xffffffff,
};

