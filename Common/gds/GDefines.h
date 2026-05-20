#pragma once

#include <string>

enum GVarType
{
	GVT_None=0,
	GVT_S,//Signed,4 byte
	GVT_U,//Unsigned,4 byte
	GVT_F,//Float,4 byte
	GVT_Fx3,//Float,4 byte,x3
	GVT_Fx4,//Float,4 byte,x4
	GVT_Fx12,//Float,4 byte,x12
	GVT_Fx16,//Float,4 byte,x16
	GVT_Sx4,//Signed,4 byte,x4
	GVT_Sx2,//Signed,4 byte,x2
	GVT_Ux4,//Unsigned,4 byte,x4
	GVT_Ux2,//Unsigned,4 byte,x2
	GVT_String,
	GVT_Raw,//Raw data
	GVT_Fx2,//Float,4 byte,x2
	GVT_Void,
	GVT_Bx4,//BYTE,1byte,x4
	GVT_Fx6,//Float,4byte,x6

	GVT_SS,//Short Signed,2 byte
	GVT_SU,//Short Unsigned,2 byte
	GVT_B,		//Byte, 1 byte
	GVT_Bx2,		//Byte, 1byte,x2

	GVT_Bx8,//Unsigned,8 byte,x8
	
	//XXXXX: more GVarType
	GVT_Max,
};

typedef struct {float v[3];} GFloat3;
typedef struct {float v[4];} GFloat4;
typedef struct {float v[12];} GFloat10;
typedef struct {float v[12];} GFloat12;
typedef struct {float v[16];} GFloat16;
typedef struct {int v[2];} GInt2;
typedef struct {int v[4];} GInt4;
typedef struct {unsigned int v[2];} GUInt2;
typedef struct {unsigned int v[4];} GUInt4;
typedef struct {float v[2];} GFloat2;
typedef struct {BYTE v[4];} GByte4;
typedef struct {float v[6];} GFloat6;
typedef struct {BYTE v[2];} GByte2;
typedef struct {BYTE v[8];} GUInt8;

//XXXXX: more GVarType


//注意,需要和g_SemMap保持一致
//注意,新加入的语义必须添加在末尾
enum GSemCode
{
	GSem_Unknown=0,
	GSem_Void,
	GSem_Color_obsolete,
	GSem_ColorU_obsolete,
	GSem_Pos,
	GSem_Normal,
	GSem_World,
	GSem_View,
	GSem_Proj,
	GSem_ViewProj,
	GSem_TexturePath,
	GSem_Shiness,
	GSem_ShineStr,
	GSem_Alpha,
	GSem_UVXform,
	GSem_UVAddr,
	GSem_TexelLength,
	GSem_Boolean,
	GSem_Name,
	GSem_MeshPath,
	GSem_MtrlPath,
	GSem_Enum,
	GSem_Plane,
	GSem_Interger,
	GSem_Float,
	GSem_FolderPath,
	GSem_TextureSize,
	GSem_TexturePartPath,
	GSem_Rect,
	GSem_Point,
	GSem_Size,

	GSem_BoneAnimPath,//动画资源路径
	GSem_XformAnimPath,//动画资源路径
	GSem_MtrlColorAnimPath,//动画资源路径
	GSem_UvAnimPath,//动画资源路径
	GSem_DummiesPath,//dummies资源路径
	GSem_SptPath,//speed tree资源路径
	GSem_MoppPath,//mopp 资源路径
	GSem_SpgPath,//speed grass 资源路径
	GSem_AnimTreePath,
	GSem_BoneAnim2Path,

	GSem_MapFilePath,
	GSem_TrrnBrushLibPath,
	GSem_SptLibPath_obsolete,//speed tree lib 的路径
	GSem_SpgLibPath_obsolete,
	GSem_ProtoPath,

	// modify mark begin
	GSem_ValueSet,
	// end

	GSem_Flags,

	GSem_StringID,

	GSem_Range,
	GSem_Xform,

	GSem_ColorAlpha,
	GSem_ColorAlphaU,

	GSem_FilePath,

	GSem_BrushLibPath,

	GSem_Aabb,

	GSem_MtrlExtPath,
	GSem_SoundPath,
	GSem_RecordsPath,

	GSem_RecordID,
	GSem_AnimTick,

	GSem_RagdollPath,
	GSem_DtrPath,
	GSem_BehaviorGraphPath,

	GSem_GUID,
	GSem_ObjID,
};
//XXXXX:more res type,注意新加的语意必须加在末尾

struct GSem
{
	GSem()
	{
		code=GSem_Unknown;
	}
	GSem(GSemCode code_)
	{
		code=code_;
	}
	GSem(GSemCode code_,const char *constraint_)
	{
		code=code_;
		constraint=constraint_;
	}
	GSem &operator=(const GSem&src)
	{
		code=src.code;
		constraint=src.constraint;
		return *this;
	}
	void operator=(GSemCode code_)
	{
		code=code_;
		constraint="";
	}
	GSemCode code;
	std::string constraint;//用于编辑时提供额外的信息
};

enum GFlag
{
	GF_None=0,
	GF_Editable=1,


	GF_ForceDword=0xffffffff,
};

int SizeFromVarType(GVarType gvt);//如果不是定长的变量类型,返回-1
const char *NameFromVarType(GVarType gvt);
const char *PropNameFromVarType(GVarType gvt);
GVarType VarTypeFromPropName(const char *name);

DWORD GetSemCount();
GSem& GetSem(DWORD idx);
const char *GetSemDesc(GSem &sem);
GVarType GetSemVarType(GSem &sem);
const char *GetSemList();//得到所有Sem的字符串列表,用","分隔,可以用于语意的Constraint
GSemCode SemCodeFromName(const char *name);

extern BOOL VerifySemVarType(GSem& sem,GVarType gvt);