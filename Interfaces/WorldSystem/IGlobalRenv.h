/********************************************************************
	created:	2009/02/23
	created:	23:2:2009   21:01
	filename: 	d:\IxEngine\Interfaces\WorldSystem\IGlobalRenv.h
	file path:	d:\IxEngine\Interfaces\WorldSystem
	file base:	IGlobalRenv
	file ext:	h
	author:		cxi
	
	purpose:	exposed global render environment defines
*********************************************************************/
#pragma once

#include "math/vector3d.h"

class ITexture;


struct GlobalLight
{
	GlobalLight()
	{
		dir.set(1.0f,-1.0f,1.0f);
		dir.normalize();

		colDifDL.set(0.5f,0.5f,0.5f);
		colSpecDL.set(0.5f,0.5f,0.5f);
		colAmbDL.set(0.5f,0.5f,0.5f);

		colmodDL.set(1,1,1);
		colmodGlobal.set(1,1,1);
		bColMod=FALSE;

		eUnitDL.set(0.5f,0.5f,0.5f);
		eUnitEnv.set(0.5f,0.5f,0.5f);

		softness=0.5f;
		eFull=eUnitDL+eUnitEnv;
	}

	//直射光的方向,是一个normalized的向量
	i_math::vector3df dir;

	//传统方式绘制需要的参数
	i_math::vector3df GetDifDL_Modified()	{		return bColMod?colDifDL*colmodDL:colDifDL;	}
	i_math::vector3df GetSpecDL_Modified()	{		return bColMod?colSpecDL*colmodDL:colSpecDL;	}
	i_math::vector3df GetAmbDL_Modified()	{		return bColMod?colAmbDL*colmodGlobal:colAmbDL;	}
	i_math::vector3df colDifDL;//直射光的颜色(Diffuse)
	i_math::vector3df colSpecDL;//直射光的颜色(Specular)
	i_math::vector3df colAmbDL;//直射光的颜色(Ambient)

	//使用LightMap绘制需要的参数
	//以下两个值是用来Bake的(代表当全局光的总能量为1的时候,来自直射光和环境光的能量）
	i_math::vector3df eUnitDL;
	i_math::vector3df eUnitEnv;

	//全局光的总能量
	i_math::vector3df GetFullEnergy_Modified()
	{
		return bColMod?eFull*colmodGlobal:eFull;
	}
	i_math::vector3df eFull;

	//直射光与环境光的比例,如果为1.0表示全部是环境光,0表示全部是直射光
	float softness;

	//Color Mode
	BOOL bColMod;
	i_math::vector3df colmodDL;
	i_math::vector3df colmodGlobal;
};

struct FogParam
{
	enum Type
	{
		None,
		Fog_Standard,
	};

	FogParam()
	{
		type=None;
		distStart=10.0f;
		distEnd=200.0f;
		height=10.0f;
		strength=0.4f;

		col.set(1.0f,1.0f,1.0f);
	}

	BOOL CheckValidAtDist(float d)
	{
		float a=(1.0f-(distEnd-d)/(distEnd-distStart))*strength;
		return a>0.01f;
	}

	void ToVector4df(i_math::vector4df &v)
	{
		v.x=distStart;
		v.y=distEnd;
		v.z=height;
		v.w=strength;
	}

	Type type;

	float distStart;
	float distEnd;
	float height;
	float strength;

	i_math::vector3df col;
};

struct WaterEnv
{
	WaterEnv()
	{
		Zero();
	}
	enum State//当前Camera和水面的位置关系
	{
		None,
		Above,
		Intersect,
		Below,
	};
	void Zero()
	{
		state=None;
	}
	State state;

	i_math::vector4df clipplane;//水平面
	i_math::vector4df fogparam;//xyz为颜色,w为雾的可视距离
};

struct TrrnEnv
{
	TrrnEnv()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL bExist;//地表是否存在

	//LightMap info of terrain
	ITexture *texShdw;
	ITexture *texLM;
	i_math::vector4df uvShdwParam;
	i_math::vector4df uvLmParam;

};

struct SightInfo
{
	SightInfo()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsValid()
	{
		return bValid_||bForcedValid;
	}
	BOOL bValid_;
	BOOL bForcedValid;//强制Valid
	float rateForcedValid;//强制比率,1.0表示完全显示出来
	ITexture *texSight;
	i_math::vector4df uvf;//uv factor for texSight
};

//全局光阴影
class ICamera;
struct GlobalShadow
{
	GlobalShadow()
	{
		Zero();
	}
	void Zero()
	{
		camLis=NULL;
		shdwmap=NULL;
		bHWShdwMap=FALSE;
	}


	BOOL IsEmpty()
	{
		return shdwmap==NULL;
	}

	float dist;//shadow 的最远距离,离camera的距离
	float dist2;//dist*dist
	i_math::plane3df plShdwClip;//在这个平面之后的物件需要(作为受影体)绘制阴影
	ICamera *camLis;//用来绘制shadow map的camera
	i_math::volumeCvxf vol;
	i_math::recti rc;//在shadow map的位置
	i_math::matrix44f lisproj;//这个proj用于shadow map采样

	ITexture *shdwmap;
	i_math::vector2df tlShadowMap;//texel length
	BOOL bHWShdwMap;

};

//Global Render Environment
struct GlobalRenv
{
	struct ViewInfo
	{
		ViewInfo()
		{
			n=f=0;
		}
		~ViewInfo()
		{
		}
		i_math::recti rcViewport;
		//some fast cache value from cam
		i_math::vector3df eye;
		i_math::vector3df lookat;
		float n,f;
	};

	BOOL NeedFog()
	{
		if (fog.type==FogParam::None)
			return FALSE;
		return fog.CheckValidAtDist(viewinfo.f);
	}
	BOOL NeedWaterFog()
	{
		if ((envWater.state==WaterEnv::Below)||(envWater.state==WaterEnv::Intersect))
			return TRUE;
		return FALSE;
	}

	i_math::vector3df center;
	GlobalLight lgt;
	FogParam fog;
	WaterEnv envWater;
	TrrnEnv envTrrn;
	GlobalShadow shdw;
	SightInfo sight;

	ViewInfo viewinfo;
};
