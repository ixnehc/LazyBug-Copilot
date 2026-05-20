// ***************************************************************
//  GSem   version:  1.0   ? date: 05/29/2008
//  -------------------------------------------------------------
//  author:		ixnehc
//  -------------------------------------------------------------
//  Copyright (C) 2008 - All Rights Reserved
// ***************************************************************
//  Purpose: 
// ***************************************************************
#include "stdh.h"
#include "GDefines.h"

#include <assert.h>

struct GVarTypeEntry
{
	GVarType gvt;
	int sz;
	const char *name;
	const char *nameProp;
};

GVarTypeEntry g_VarMap[]=
{																
	{GVT_None,			-1,			"<None>",							""},
	{GVT_S,				4,				"int",									"Prop_S"},//Signed,4 byte
	{GVT_U,				4,				"unsigned int",					"Prop_U"},//Unsigned,4 byte
	{GVT_F,				4,				"float",									"Prop_F"},//Float,4 byte
	{GVT_Fx3,			12,			"float3",								"Prop_Fx3"},//Float,4 byte,x3
	{GVT_Fx4,			16,			"float4",								"Prop_Fx4"},//Float,4 byte,x4
	{GVT_Fx12,			48,			"float12",								"Prop_Fx12"},//Float,4 byte,x12
	{GVT_Fx16,			64,			"float16",								"Prop_Fx16"},//Float,4 byte,x16
	{GVT_Sx4,			16,			"int4",									"Prop_Sx4"},//Signed,4 byte,x4
	{GVT_Sx2,			8,				"int2",									"Prop_Sx2"},//Signed,4 byte,x2
	{GVT_Ux4,			16,			"unsigned int4",					""},//Unsigned,4 byte,x4
	{GVT_Ux2,			8,				"unsigned int2",					""},//Unsigned,4 byte,x2
	{GVT_String,		-1,			"String",								"Prop_String"},
	{GVT_Raw,			-1,			"binary",								""},//Raw data
	{GVT_Fx2,			8,				"float2",								"Prop_Fx2"},
	{GVT_Void,			0,				"void",									"Prop_Void"},
	{GVT_Bx4,			4,				"byte[4]",								"Prop_U"},
	{GVT_Fx6,			4,				"float6",								"Prop_Fx6"},
	{GVT_SS,				2,				"short",								""},
	{GVT_SU,				2,				"Word",								""},
	{GVT_B,				1,				"Byte",									""},
	{GVT_Bx2,			2,				"byte[2]",								""},
	{GVT_Bx8,			8,				"unsigned64",						""},
	//XXXXX: more GVarType
};

//如果不是定长的变量类型,返回-1
int SizeFromVarType(GVarType gvt)
{
	assert(g_VarMap[gvt].gvt==gvt);
	return g_VarMap[gvt].sz;
}


const char *NameFromVarType(GVarType gvt)
{
	assert(g_VarMap[gvt].gvt==gvt);
	return g_VarMap[gvt].name;
}

const char *PropNameFromVarType(GVarType gvt)
{
	assert(g_VarMap[gvt].gvt==gvt);
	return g_VarMap[gvt].nameProp;
}

GVarType VarTypeFromPropName(const char *name)
{
	for (int i=0;i<ARRAY_SIZE(g_VarMap);i++)
	{
		if (strcmp(g_VarMap[i].nameProp,name)==0)
			return g_VarMap[i].gvt;
	}
	return GVT_None;
}



struct GSemEntry
{
	GSem sem;
	const char *name;
	GVarType gvt;
	const char *desc;
};


//注意,这个表格的顺序和个数必须和GSemCode的枚举值保持一致
//注意,描述字串中不要有逗号
#define GSEM_ENTRY(nm,gvt,desc)	{nm,#nm,gvt,desc}

GSemEntry g_SemMap[]=
{
	GSEM_ENTRY(GSem_Unknown,					GVT_None,				""),
	GSEM_ENTRY(GSem_Void,							GVT_Void,				"无类型"),
	GSEM_ENTRY(GSem_Color_obsolete,			GVT_Fx3,					"颜色(3xfloat)(已作废)"),
	GSEM_ENTRY(GSem_ColorU_obsolete,		GVT_Bx4,					"颜色(24bit)(已作废)"),
	GSEM_ENTRY(GSem_Pos,								GVT_Fx3,					"空间位置"),
	GSEM_ENTRY(GSem_Normal,						GVT_Fx3,					"法线"),
	GSEM_ENTRY(GSem_World,							GVT_Fx12,				"世界空间转换矩阵"),
	GSEM_ENTRY(GSem_View,							GVT_Fx16,				"Camera转换矩阵"),
	GSEM_ENTRY(GSem_Proj,								GVT_Fx16,				"投影转换矩阵"),
	GSEM_ENTRY(GSem_ViewProj,						GVT_Fx16,				"Camerax投影转换矩阵"),
	GSEM_ENTRY(GSem_TexturePath,				GVT_String,				"贴图路径"),
	GSEM_ENTRY(GSem_Shiness,						GVT_F,						"光亮度"),
	GSEM_ENTRY(GSem_ShineStr,						GVT_F,						"高光强度"),
	GSEM_ENTRY(GSem_Alpha,							GVT_F,						"透明度"),
	GSEM_ENTRY(GSem_UVXform,						GVT_Fx12,				"贴图坐标转换矩阵"),
	GSEM_ENTRY(GSem_UVAddr,						GVT_Sx4,					"贴图寻址模式"),
	GSEM_ENTRY(GSem_TexelLength,				GVT_F,						"像素单位长度"),
	GSEM_ENTRY(GSem_Boolean,						GVT_S,						"布尔值"),
	GSEM_ENTRY(GSem_Name,							GVT_String,				"普通字符串"),
	GSEM_ENTRY(GSem_MeshPath,					GVT_String,				"模型资源的路径"),
	GSEM_ENTRY(GSem_MtrlPath,						GVT_String,				"材质资源的路径"),
	GSEM_ENTRY(GSem_Enum,							GVT_S,						"枚举值"),
	GSEM_ENTRY(GSem_Plane,							GVT_Fx4,					"平面方程系数"),
	GSEM_ENTRY(GSem_Interger,						GVT_S,						"普通整数"),
	GSEM_ENTRY(GSem_Float,							GVT_F,						"普通浮点数"),
	GSEM_ENTRY(GSem_FolderPath,					GVT_String,				"目录路径"),
	GSEM_ENTRY(GSem_TextureSize,					GVT_Fx2,					"贴图大小"),
	GSEM_ENTRY(GSem_TexturePartPath,			GVT_String,				"描述贴图上某一部分的字符串"),
	GSEM_ENTRY(GSem_Rect,							GVT_Sx4,					"二维方形区域"),
	GSEM_ENTRY(GSem_Point,							GVT_Sx2,					"二维点"),
	GSEM_ENTRY(GSem_Size,								GVT_Sx2,					"二维大小"),

	GSEM_ENTRY(GSem_BoneAnimPath,			GVT_String,				"骨骼动画资源的路径"),
	GSEM_ENTRY(GSem_XformAnimPath,			GVT_String,				"路径动画资源的路径"),
	GSEM_ENTRY(GSem_MtrlColorAnimPath,	GVT_String,				"材质颜色资源的路径"),
	GSEM_ENTRY(GSem_UvAnimPath,				GVT_String,				"uv动画资源的路径"),
	GSEM_ENTRY(GSem_DummiesPath,			GVT_String,				"dummies资源路径"),
	GSEM_ENTRY(GSem_SptPath,						GVT_String,				"speed tree资源路径"),
	GSEM_ENTRY(GSem_MoppPath,					GVT_String,				"Mopp资源文件路径"),
	GSEM_ENTRY(GSem_SpgPath,						GVT_String,				"speed grass资源文件路径"),
	GSEM_ENTRY(GSem_AnimTreePath,			GVT_String,				"AnimTree资源文件路径"),
	GSEM_ENTRY(GSem_BoneAnim2Path,			GVT_String,				"骨骼动画资源文件路径"),

	GSEM_ENTRY(GSem_MapFilePath,				GVT_String,				"map 文件路径"),
	GSEM_ENTRY(GSem_TrrnBrushLibPath,		GVT_String,				"地表笔刷库文件路径"),
	GSEM_ENTRY(GSem_SptLibPath_obsolete,	GVT_String,				"SpeedTree库文件路径"),
	GSEM_ENTRY(GSem_SpgLibPath_obsolete,	GVT_String,				"SpeedGrass库文件路径"),
	GSEM_ENTRY(GSem_ProtoPath,					GVT_String,				"Proto的路径"),
	
	GSEM_ENTRY(GSem_ValueSet,						GVT_None,						"数值折线" ),

	GSEM_ENTRY(GSem_Flags,							GVT_U,					"标志位"),
	GSEM_ENTRY(GSem_StringID,						GVT_U,					"字符串ID"),

	GSEM_ENTRY(GSem_Range,							GVT_Fx2,					"取值范围"),
	GSEM_ENTRY(GSem_Xform,							GVT_Fx12,				"转换矩阵"),

	GSEM_ENTRY(GSem_ColorAlpha,					GVT_Fx4,		"颜色-Alpha(4xfloat)"),
	GSEM_ENTRY(GSem_ColorAlphaU,				GVT_Bx4,			"颜色-Alpha(32bit)"),

	GSEM_ENTRY(GSem_FilePath,						GVT_String,				"(任意)文件路径"),

	GSEM_ENTRY(GSem_BrushLibPath,				GVT_String,				"笔刷库文件路径"),

	GSEM_ENTRY(GSem_Aabb,							GVT_Fx6,					"包围盒"),
	GSEM_ENTRY(GSem_MtrlExtPath,		GVT_String,				"材质扩展资源文件路径"),
	GSEM_ENTRY(GSem_SoundPath,			GVT_String,				"声音资源文件路径"),
	GSEM_ENTRY(GSem_RecordsPath,			GVT_String,				"Records资源文件路径"),

	GSEM_ENTRY(GSem_RecordID,			GVT_U,				"RecordID"),
	GSEM_ENTRY(GSem_AnimTick,			GVT_U,				"AnimTick"),

	GSEM_ENTRY(GSem_RagdollPath,			GVT_String,				"Radgoll资源文件路径"),
	GSEM_ENTRY(GSem_DtrPath,			GVT_String,				"Destructable资源文件路径"),
	GSEM_ENTRY(GSem_BehaviorGraphPath,			GVT_String,				"BehaviorGraph资源文件路径"),
	GSEM_ENTRY(GSem_GUID,			GVT_U,				"独一无二的ID"),
	GSEM_ENTRY(GSem_ObjID,			GVT_U,				"游戏对象ID"),
};
//XXXXX:more res type


DWORD GetSemCount()
{
	return ARRAY_SIZE(g_SemMap);
}

GSem& GetSem(DWORD idx)
{
	if (idx>=GetSemCount())
		return g_SemMap[0].sem;
	return g_SemMap[idx].sem;
}

const char *GetSemDesc(GSem &sem)
{
	return g_SemMap[sem.code].desc;
}

GVarType GetSemVarType(GSem &sem)
{
	return g_SemMap[sem.code].gvt;
}


BOOL VerifySemVarType(GSem& sem,GVarType gvt)
{
	//一些特例
	if (sem.code==GSem_Interger)
	{
		if((gvt==GVT_S)||(gvt==GVT_U)||(gvt==GVT_SS)||(gvt==GVT_SU)||(gvt==GVT_B))
			return TRUE;
		return FALSE;
	}
	if (sem.code==GSem_Flags)
	{
		if((gvt==GVT_S)||(gvt==GVT_U))
			return TRUE;
		return FALSE;
	}

	if (sem.code==GSem_RecordID)
	{
		if((gvt==GVT_S)||(gvt==GVT_U))
			return TRUE;
		return FALSE;
	}

	if (sem.code==GSem_Boolean)
	{
		if((gvt==GVT_S)||(gvt==GVT_U)||(gvt==GVT_B))
			return TRUE;
		return FALSE;
	}

	if (sem.code==GSem_Size)
	{
		if ((gvt==GVT_Bx2)||(gvt==GVT_Sx2))
			return TRUE;
		return FALSE;
	}

	if (sem.code==GSem_Point)
	{
		if ((gvt==GVT_Bx2)||(gvt==GVT_Sx2))
			return TRUE;
		return FALSE;
	}

	if (sem.code==GSem_ProtoPath)
	{
		if ((gvt==GVT_String)||(gvt==GVT_Bx8))
			return TRUE;
		return FALSE;
	}

	if (sem.code==GSem_Range)
	{
		if ((gvt==GVT_Fx2)||(gvt==GVT_Sx2))
			return TRUE;
		return FALSE;
	}

	//XXXXX: more GVarType

	GVarType t=GetSemVarType(sem);
	if (t==GVT_None)
		return TRUE;


	return (t==gvt);
}

//得到所有Sem的字符串列表,用","分隔,可以用于语意的Constraint
const char *GetSemList()
{
	static std::string s;

	if (s=="")
	{
		s="<未知>";
		int n=GetSemCount();
		for (int i=1;i<n;i++)//从1开始是为了略过GSem_Unknown
		{
			s+=",";
			s+=GetSemDesc(GSem((GSemCode)i));
		}
	}
	return s.c_str();
}

GSemCode SemCodeFromName(const char *name)
{
	int c=GetSemCount();
	for (int i=1;i<c;i++)
	{
		if (strcmp(g_SemMap[i].name,name)==0)
			return g_SemMap[i].sem.code;
	}
	return GSem_Unknown;
}
