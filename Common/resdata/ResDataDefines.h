#pragma once

#include "gds/GVar.h"

#include "../math/matrix43.h"

#include "../math/sphere.h"

#include <assert.h>

#include "gds/GObj.h"

//the existing defines should not be modified,otherwise the data in res file will be invalid
enum ResType
{
	Res_None=0,

	//Folder
	Res_Folder=10,

	//所谓native的Res是指这中资源的文件格式是一种已有的,通用的类型
	Res_Native_Begin=50,
	Res_Texture=50,
	Res_Sheet,
	Res_Sound,
	Res_Ragdoll,

	//所谓standard的Res是指这种资源的文件格式是由我们自己定义的
	
	//standard res type
	Res_Standard_Begin=100,
	Res_Mesh=100,
	ResA_XForm,
	ResA_Bones_obsolete,
	Res_Mtrl,
	ResA_MtrlColor,
	ResA_MapCoord,
	Res_Dummies,
	Res_Spt,
	Res_Mopp,
	Res_Spg,
	Res_AnimTree,
	ResA_Bones2,
	Res_MtrlExt,
	Res_Records,
	Res_Dtr,
	Res_BehaviorGraph,
	//XXXXX:more res type

	//IMPORTANT: add new res type here.
};

#define Res_IsFolder(type)  ((type)==Res_Folder)
#define Res_IsNative(type) (((type)>=Res_Native_Begin)&&((type)<Res_Standard_Begin))
#define Res_IsStandard(type) ((type)>=Res_Standard_Begin)


#define MD_MaxSurf 16
#define MD_MaxLod 8


enum 
{
	MtlData_MaxLayor=4,
};

enum LayorFlag
{
	LF_Invisible=1,
	LF_IgnoreLight=2,

};

enum ShaderBlendMode
{
	Blend_Opaque,
	Blend_AlphaBlend,
	Blend_Additive,
	Blend_Modulate,
	Blend_Enlighten,
	Blend_Inverse,

	Blend_Invalid,
	//XXXXX:more blend mode
};


enum StencilMode
{
	Sten_Disable=0,
	Sten_Write,//No test,directly overwrite the value in the stencil buffer with ref value
	Sten_WriteNoColor,//No test,directly overwrite the value in the stencil buffer with ref value,and do not write color
	Sten_Filter,//write pixel if the value in the stencil buffer equals the ref value
	Sten_InvFilter,//write pixel if the value in the stencil buffer does NOT equal the ref value
	Sten_Inc,//No test,add 1 to the stencil buffer
	Sten_IncNoColor,//No test,add 1 to the stencil buffer,do not write color
	Sten_FilterWrite,//write pixel if the value in the stencil buffer equals the ref value,and for
								//the passed pixel,write the ref value into the stencil buffer
	Sten_InvFilterWrite,//write pixel if the value in the stencil buffer does NOT equal the ref
									//value,and for the passed pixel,write the ref value into the stencil buffer
	Sten_FilterInc,//write pixel if the value in the stencil buffer equals the ref
									//value,and for the passed pixel,add 1 to the stencil buffer

	Sten_Invalid,
	//XXXXX:more stencil mode
};


enum DepthMode
{
	Depth_Default=0,//compare and write
	Depth_NoCmp,//just write depth to depth buffer
	Depth_NoWrite,//just compare and do not write to depth buffer
	Depth_Disable,//既不比较也不写,
	Depth_FartherNoWrite,//绘制被挡住的部分,不写depth buffer

	Depth_Invalid,
};


enum AlphaTestMode
{
	AlphaTest_Disable=0,
	AlphaTest_Standard=1,

	AlphaTest_Invalid,
};

//describe which side(front side/back side/or both side) should be drawn
enum FacingMode
{
	Facing_Front=0,
	Facing_Back=1,
	Facing_Both=2,

	Facing_Invalid,
};


struct ShaderState
{
	ShaderState()
	{
		Zero();
	}

	void Zero()
	{
		modeBlend=(BYTE)Blend_Opaque;
		modeStencil=(BYTE)Sten_Disable;
		modeDepth=(BYTE)Depth_Default;
		modeAlphaTest=(BYTE)AlphaTest_Disable;
		modeFacing=(BYTE)Facing_Front;

		refStencil=0;
		maskStencil=0xffff;
		refDepth=0.0f;
		refAlphaTest=0;

		reserved=0;
	}

	void SetInvalid()
	{
		modeBlend=(BYTE)Blend_Invalid;
		modeStencil=(BYTE)Sten_Invalid;
		modeDepth=(BYTE)Depth_Invalid;
		modeAlphaTest=(BYTE)AlphaTest_Invalid;
		modeFacing=(BYTE)Facing_Invalid;
	}


	BYTE modeBlend;//a ShaderBlendMode value
	BYTE modeStencil;//a StencilMode value
	BYTE modeDepth;//a DepthMode
	BYTE modeAlphaTest;//an AlphaTestMode value

	WORD refStencil;
	WORD maskStencil;
	float refDepth;
	WORD refAlphaTest;

	BYTE modeFacing;//a FacingMode value
	BYTE reserved;
};


struct ShaderBlend
{
	DWORD mode;//an LayorBlendMode value
	DWORD ref;//not used yet
};

struct StencilOp
{
	DWORD mode;//a StencilMode
	WORD ref;
	WORD mask;
};

struct DepthMethod
{
	DWORD mode;
	float ref;//not used yet
};




// struct FeatureParam
// {
// 	FeatureParam()
// 	{
// 		memset(ep_name,0,sizeof(ep_name));
// 	}
// 	void SetEP(const char *name)
// 	{
// 		strncpy(ep_name,name,sizeof(ep_name)-1);
// 		ep_name[sizeof(ep_name)-1]=0;
// 	}
// 	char ep_name[32];
// 	GVar var;
// 
// 	//comparing by ep_name
// 	bool operator>(const FeatureParam&src)
// 	{
// 		return (strcmp(ep_name,src.ep_name)>0);
// 	}
// 
// 	bool operator==(const FeatureParam&src)
// 	{
// 		if (strcmp(ep_name,src.ep_name)!=0)
// 			return false;
// 		if (!var.Equal((GVar&)src.var))
// 			return false;
// 		return true;
// 	}
// 
// 	FeatureParam& operator=(const FeatureParam&src)
// 	{
// 		memcpy(ep_name,src.ep_name,sizeof(ep_name));
// 		var=src.var;
// 		return *this;
// 	}
// };

struct DummyInfo
{
	enum
	{
		BoundType_Sphere,
		BoundType_AABB,
		BoundType_Point,
		BoundType_Capsule,
		BoundType_UNKNOWN=0xffffffff,
	};
	DummyInfo()
	{
		bt=BoundType_UNKNOWN;
		idxBone=0;
	}

	i_math::spheref * getSphere()
	{	
		if(bt!=BoundType_Sphere)
			return NULL;
		return (i_math::spheref*)(data);
	}
	i_math::aabbox3df * getAAbb()
	{
		if(bt!=BoundType_AABB)
			return NULL;
		return (i_math::aabbox3df*)(data);
	}
	i_math::vector3df * getPoint()
	{
		if(bt!=BoundType_Point)
			return NULL;
		return (i_math::vector3df *)(data);
	}
	i_math::capsulef * getCapsule()
	{
		if(bt!=BoundType_Capsule)
			return NULL;
		return(i_math::capsulef *)(data);
	}
	void setType(DWORD dummytype)
	{
		switch(dummytype)
		{
		case BoundType_AABB:
			{
				assert(sizeof(data)>=sizeof(i_math::aabbox3df));
				
				i_math::aabbox3df aabb;

				if(bt==BoundType_Point)
				{
					i_math::vector3df * p = getPoint();
					i_math::vector3df len(0.5f,0.5f,0.5f);
					
					aabb.MaxEdge = len + *p;
					aabb.MinEdge = -len + *p;
				}
				else if (bt == BoundType_Sphere)
				{
					i_math::spheref * sph = getSphere();

					i_math::vector3df c = sph->center;
					
					float r = sph->radius;
					i_math::vector3df len(r,r,r);

					aabb.MinEdge = c - len;
					aabb.MaxEdge = c + len;
				}
				
				memcpy(data,&aabb,sizeof(i_math::aabbox3df));
			
				break;
			}
		case BoundType_Sphere:
			{
				assert(sizeof(data)>=sizeof(i_math::spheref));

				i_math::spheref sph;

				if(bt==BoundType_AABB)
				{
					i_math::aabbox3df * abb = getAAbb();
					
					sph.center = abb->getCenter();
					sph.radius = float(abb->MinEdge.getDistanceFrom(abb->MaxEdge))/2.0f;
				}
				else if(bt==BoundType_Point)
				{
					i_math::vector3df * center = getPoint();
					sph.setCenter(*center);
					sph.setRadius(0.5f);
				}
				
				memcpy(data,&sph,sizeof(i_math::spheref));

				break;
			}
		case BoundType_Point:
			{
				assert(sizeof(data)>=sizeof(i_math::vector3df));
				
				i_math::vector3df center;

				if(bt==BoundType_AABB)
				{
					i_math::aabbox3df * abb = getAAbb();
					center = abb->getCenter();
				}
				else if(bt==BoundType_Sphere)
				{
					i_math::spheref * sph = getSphere();
					center = sph->center;
				}

				memcpy(data,&center,sizeof(i_math::vector3df));

				break;
			}
		case BoundType_Capsule:
			{
				assert(sizeof(data)>=sizeof(i_math::capsulef));
				break;
			}
		default:
			break;
		}

		bt = dummytype;
	}
	
	DWORD & getBoundType(){return bt;}

	DWORD GetVersion(){return 2;}

	DWORD idxBone;
	i_math::matrix43f matOff;
	i_math::matrix43f matOffInv;
protected:
	float data[7];	
	DWORD bt;
};

#define MAX_LOD_LEVEL 10
#define MAX_BB_IMAGES 10
#define MAX_NUM_WINDS 10
#define MAX_NUM_WINDANGLES 10
#define MAX_NUM_WINDMATRICES 10
//SpeedWind的配置数据
struct SptWndCfg
{
	struct Range
	{
		void Set(float la,float ha,float ls,float hs)
		{
			fLowAngle=la;
			fHiAngle=ha;
			fLowSpeed=ls;
			fHiSpeed=hs;
		}
		float fLowAngle;
		float fHiAngle;
		float fLowSpeed;
		float fHiSpeed;

		BEGIN_GOBJ_PURE(Range,1);
			GELEM_VAR_INIT(float,fLowAngle,0.1f);
				GELEM_EDITVAR("最小偏角",GVT_F,GSem(GSem_Float,"0,360.0f,0.1f"),"最小的偏转角度");
			GELEM_VAR_INIT(float,fHiAngle,0.1f);
				GELEM_EDITVAR("最大偏角",GVT_F,GSem(GSem_Float,"0,360.0f,0.1f"),"最大的偏转角度");
			GELEM_VAR_INIT(float,fLowSpeed,0.1f);
				GELEM_EDITVAR("最小速度",GVT_F,GSem(GSem_Float,"0.01f,1.0f,0.01f"),"最小的角速度");
			GELEM_VAR_INIT(float,fHiSpeed,0.1f);
				GELEM_EDITVAR("最大速度",GVT_F,GSem(GSem_Float,"0.01f,1.0f,0.01f"),"最大的角速度");
		END_GOBJ();
	};

	struct Gust
	{
		//GetGusting()
		float fGustStrengthMin;
		float fGustStrengthMax;
		float fGustDurationMin;
		float fGustDurationMax;
		float fGustFrequency;
		
		BEGIN_GOBJ_PURE(Gust,1);
			GELEM_VAR_INIT(float,fGustFrequency,4.0f);
				GELEM_EDITVAR("频率",GVT_F,GSem(GSem_Float,"0,100,1.0f"),"强风出现的频率");
			GELEM_VAR_INIT(float,fGustStrengthMin,0.25f);
				GELEM_EDITVAR("最小强度",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"强风最小强度");
			GELEM_VAR_INIT(float,fGustStrengthMax,1.0f);
				GELEM_EDITVAR("最大强度",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"强风最大强度");
			GELEM_VAR_INIT(float,fGustDurationMin,2.0f);
				GELEM_EDITVAR("最小持续时间",GVT_F,GSem(GSem_Float,"0,100.0f,0.1f"),"强风的最小持续时间");
			GELEM_VAR_INIT(float,fGustDurationMax,7.0f);
				GELEM_EDITVAR("最大持续时间",GVT_F,GSem(GSem_Float,"0,100.0f,0.1f"),"强风的最大持续时间");
		END_GOBJ()

	};
	
	BEGIN_GOBJ_PURE(SptWndCfg,1)
	
		GELEM_OBJ(Gust,gust);
			GELEM_EDITOBJ_EX("强风","强风的参数",GSem_Unknown);
		
		GELEM_VAR_INIT(float,fBranchExponents,2.0f);
			GELEM_EDITVAR("树干指数",GVT_F,GSem(GSem_Float,"0.01f,20.0f,0.1f"),"树干风的强度指数曲线的指数");
		GELEM_OBJ(Range,branchHor);
			GELEM_EDITOBJ_EX("树干水平摆动","树干的水平摆动",GSem_Unknown);
		GELEM_OBJ(Range,branchVer);
			GELEM_EDITOBJ_EX("树干垂直摆动","树干的垂直摆动",GSem_Unknown);
		
		GELEM_VAR_INIT(float,fLeafExponents,2.0f);
			GELEM_EDITVAR("叶子指数",GVT_F,GSem(GSem_Float,"0.01f,20.0f,0.1f"),"叶子风的强度指数曲线的指数");
		GELEM_OBJ(Range,leafRustling)
			GELEM_EDITOBJ_EX("树叶旋动","树叶沿向屏幕轴向旋动",GSem_Unknown);
		GELEM_OBJ(Range,leafRocking)
			GELEM_EDITOBJ_EX("树叶摆动","树叶向屏幕内外摆动",GSem_Unknown);

		GELEM_VAR_INIT(float,fMaxBend,35.0f);
			GELEM_EDITVAR("弯曲",GVT_F,GSem(GSem_Float,"0,360.0f,1.0f"),"最大的弯曲角度");
		GELEM_VAR_INIT(float,fResponse,0.5f);
			GELEM_EDITVAR("风速",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"风强度改变的速率");
		GELEM_VAR_INIT(float,fResponseLimit,0.001f);
			GELEM_EDITVAR("调和",GVT_F,GSem(GSem_Float,"0,1.0f,0.001f"),"每次能改变的最大值，避免风的突变。");	
		
		GELEM_VAR_INIT(int,nMatrices,6);
		GELEM_VAR_INIT(int,nLeafAngles,8);
		GELEM_VAR_INIT(float,fStrength,1.0f);
		GELEM_VAR_INIT(i_math::vector3df,direction,i_math::vector3df(0,0,1.0f));

		END_GOBJ()

	Gust gust;

	Range branchHor;//GetBranchHorizontal()
	Range branchVer;//GetBranchVertical()
	Range leafRocking;//GetLeafRocking()
	Range leafRustling;//GetLeafRustling()

	//GetExponents()
	float fBranchExponents;
	float fLeafExponents;

	float fMaxBend;//GetMaxBendAngle()

	//GetWindResponse()
	float fResponse;
	float fResponseLimit;

	//GetQuntities()
	int nMatrices;
	int nLeafAngles;

	//strength and direction
	float fStrength;
	i_math::vector3df direction;

};
