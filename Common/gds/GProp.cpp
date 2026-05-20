#pragma once
#include "stdh.h"

#include <string>

#include "GProp.h"

IMPLEMENT_CLASS(Prop_Void);
IMPLEMENT_CLASS(Prop_String);


IMPLEMENT_CLASS(Prop_S);
IMPLEMENT_CLASS(Prop_U);
IMPLEMENT_CLASS(Prop_F);
IMPLEMENT_CLASS(Prop_Fx2);
IMPLEMENT_CLASS(Prop_Fx3);
IMPLEMENT_CLASS(Prop_Fx4);
IMPLEMENT_CLASS(Prop_Fx6);
IMPLEMENT_CLASS(Prop_Fx12);
IMPLEMENT_CLASS(Prop_Fx16);
IMPLEMENT_CLASS(Prop_Sx4);
IMPLEMENT_CLASS(Prop_Sx2);
IMPLEMENT_CLASS(Prop_Bx4);
//XXXXX:more simple type property


#define SEM_PROP(code,proptype,def)														\
case code:																										\
{																														\
	static proptype v=proptype(def);																\
	return &v;																									\
}

GProperty *DefaultPropFromSem(GSemCode code)
{
	switch(code)
	{
		SEM_PROP(	GSem_Pos,								Prop_Fx3,				i_math::vector3df(0.0f,0.0f,0.0f))
		SEM_PROP(	GSem_Normal,						Prop_Fx3,				i_math::vector3df(0.0f,1.0f,0.0f))
		SEM_PROP(	GSem_World,							Prop_Fx12,			i_math::matrix43f())
		SEM_PROP(	GSem_View,							Prop_Fx16,			i_math::matrix44f())
		SEM_PROP(	GSem_Proj,								Prop_Fx16,			i_math::matrix44f())
		SEM_PROP(	GSem_ViewProj,						Prop_Fx16,			i_math::matrix44f())
		SEM_PROP(	GSem_TexturePath,				Prop_String,		"")
		SEM_PROP(	GSem_Shiness,						Prop_F,				1.0f)
		SEM_PROP(	GSem_ShineStr,						Prop_F,				1.0f)
		SEM_PROP(	GSem_Alpha,							Prop_F,				1.0f)
		SEM_PROP(	GSem_UVXform,						Prop_Fx12,			i_math::matrix43f())
		SEM_PROP(	GSem_UVAddr,						Prop_Sx4,			i_math::vector4di(1,1,1,0))
		SEM_PROP(	GSem_TexelLength,				Prop_F,				1.0f)
		SEM_PROP(	GSem_Boolean,						Prop_S,				1)
		SEM_PROP(	GSem_Name,							Prop_String,		"")
		SEM_PROP(	GSem_MeshPath,					Prop_String,		"")
		SEM_PROP(	GSem_MtrlPath,						Prop_String,		"")
		SEM_PROP(	GSem_Enum,							Prop_S,				0)
		SEM_PROP(	GSem_Plane,							Prop_Fx4,				i_math::vector4df(0,1,0,0))
		SEM_PROP(	GSem_Interger,						Prop_S,				0)
		SEM_PROP(	GSem_Float,							Prop_F,				0.0f)
		SEM_PROP(	GSem_FolderPath,					Prop_String,		"")
		SEM_PROP(	GSem_TextureSize,					Prop_Fx2,				i_math::vector2df(256.0f,256.0f))
		SEM_PROP(	GSem_TexturePartPath,			Prop_String,		"")
		SEM_PROP(	GSem_Rect,								Prop_Sx4,			i_math::vector4di(0,0,64,64))
		SEM_PROP(	GSem_Point,							Prop_Sx2,			i_math::vector2di(0,0))
		SEM_PROP(	GSem_Size,								Prop_Sx2,			i_math::vector2di(64,64))

		SEM_PROP(	GSem_BoneAnimPath,			Prop_String,		"")
		SEM_PROP(	GSem_XformAnimPath,			Prop_String,		"")
		SEM_PROP(	GSem_MtrlColorAnimPath,	Prop_String,		"")
		SEM_PROP(	GSem_UvAnimPath,				Prop_String,		"")
		SEM_PROP(	GSem_DummiesPath,			Prop_String,		"")
		SEM_PROP(	GSem_SptPath,						Prop_String,		"")
		SEM_PROP(	GSem_MoppPath,					Prop_String,		"")
		SEM_PROP(	GSem_SpgPath,						Prop_String,		"")
		SEM_PROP(	GSem_AnimTreePath,			Prop_String,		"")
		SEM_PROP(	GSem_BoneAnim2Path,			Prop_String,		"")
		SEM_PROP(	GSem_MtrlExtPath,				Prop_String,		"")
		SEM_PROP(	GSem_SoundPath,					Prop_String,		"")
		SEM_PROP(	GSem_RecordsPath,				Prop_String,		"")
		SEM_PROP(	GSem_RagdollPath,				Prop_String,		"")
		SEM_PROP(	GSem_DtrPath,						Prop_String,		"")
		SEM_PROP(	GSem_BehaviorGraphPath,				Prop_String,		"")
		//XXXXX:more res type

		SEM_PROP(	GSem_MapFilePath,				Prop_String,		"")
		SEM_PROP(	GSem_TrrnBrushLibPath,		Prop_String,		"")
		SEM_PROP(	GSem_SptLibPath_obsolete,	Prop_String,		"")
		SEM_PROP(	GSem_SpgLibPath_obsolete,	Prop_String,		"")
		SEM_PROP(	GSem_ProtoPath,					Prop_String,		"")
		SEM_PROP(	GSem_BrushLibPath,				Prop_String,		"")

		// modify mark begin
//		SEM_PROP(	GSem_ValueSet,
		// end

//		SEM_PROP(	GSem_Flags,

		SEM_PROP(	GSem_StringID,						Prop_U,				0)

		SEM_PROP(	GSem_Range,							Prop_Fx2,				i_math::vector2df(0,1));
		SEM_PROP(	GSem_Xform,							Prop_Fx12,			i_math::matrix43f());

		SEM_PROP(	GSem_ColorAlpha,					Prop_Fx4,				i_math::vector4df(1.0f,1.0f,1.0f,1.0f))
		SEM_PROP(	GSem_ColorAlphaU,				Prop_Bx4,			i_math::vector4db(255,255,255,255))

	}

	return NULL;
}