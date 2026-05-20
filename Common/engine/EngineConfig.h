
#pragma once

struct EngineCfgEntry
{
	EngineCfgEntry(const char *name_,const char *str_,GSemCode code_,const char *constraint_="",const char *desc_="")
	{
		name=name_;
		desc=desc_;
		tp=1;
		str=str_;
		code=code_;
		constraint=constraint_;
	}
	EngineCfgEntry(const char *name_,int num_,GSemCode code_,const char *constraint_="",const char *desc_="")
	{
		name=name_;
		desc=desc_;
		tp=2;
		num=num_;
		code=code_;
		constraint=constraint_;
	}
	const char *name;
	const char *desc;
	int tp;
	const char *str;
	int num;
	GSemCode code;
	const char *constraint;
};

#define HelperLayor_SemConstraint \
	"路径:1,碰撞体:2,地表印刻:4,对齐边界:8,缩略模型:16,位点:32,游戏对象:64,遮挡体:128,骨骼系统:256,贴花空间:512,HitTest碰撞体:1024"
//XXXXX:more Helper Layors



#define _E EngineCfgEntry
inline EngineCfgEntry *GetEngineConfigEntries(DWORD &c)
{
	static EngineCfgEntry entries[]=
	{

		_E( "Engine.Common.ViewDist",			500,				GSem_Interger,"",												"视野距离(单位为米)"),
		_E( "Engine.Renderer.Mechanism",			0,					GSem_Interger,"Forward,Deferred",					"绘制的基本方式"),
		_E( "Engine.Renderer.Warp",		FALSE,			GSem_Boolean,"",													"是否开启(单层)扭曲效果"),
		_E( "Engine.Renderer.Warp(MultiLayor)",		FALSE,			GSem_Boolean,"",													"是否开启(多层)扭曲效果"),
		_E( "Engine.Renderer.HDR",		FALSE,			GSem_Boolean,"",													"是否开启 HDR效果"),
		_E( "Engine.Renderer.Glow",					TRUE,			GSem_Boolean,"",										"是否开启Glow效果"),
		_E( "Engine.Renderer.Halo",					FALSE,			GSem_Boolean,"",										"是否开启Halo效果"),
		_E( "Engine.Renderer.DepthOfField",	FALSE,			GSem_Boolean,"",										"是否开启景深效果"),
		_E( "Engine.Renderer.MotionBlur",	FALSE,			GSem_Boolean,"",										"是否开启运动模糊效果"),
		_E( "Engine.Renderer.DynShadow",			FALSE,			GSem_Interger,"不开启,低精度,高精度",					"动态阴影效果级别"),
		_E( "Engine.Renderer.SoftParticle",					TRUE,			GSem_Boolean,"",										"是否开启软粒子效果"),
		_E( "Engine.Renderer.ShowSeeThru",					FALSE,			GSem_Boolean,"",										"是否显示视线穿透物件"),
		_E( "Engine.Renderer.ShowStaticShdwCaster",					FALSE,			GSem_Boolean,"",										"是否显示静态阴影投射体"),
		_E( "Engine.Trrn.DetailLevel",				1,					GSem_Interger,"低精度,中等精度,高精度",			"地表贴图的精度"),
		_E( "Engine.Trrn.NormalMap",				TRUE,			GSem_Boolean,"",												"地表绘制是否使用法线贴图"),
		_E( "Engine.Trrn.SpecMap",					TRUE,			GSem_Boolean,"",												"地表绘制是否使用高光贴图"),
		_E( "Engine.SpeedTree.Wind",			TRUE,				GSem_Boolean,	"",								"SpeedTree树木是否表现自然风效果"),
		_E( "Engine.SpeedTree.NormalMap",		TRUE,				GSem_Boolean,	"",								"SpeedTree树木是否表现法线贴图细节纹理效果"),
		_E( "Engine.SpeedTree.SelfShadow",		TRUE,				GSem_Boolean,	"",								"SpeedTree树木是否表现自阴影"),
		_E( "Engine.Vegetable.DetailRange",		200,				GSem_Interger,	"",								"植被可视稠密区的范围"),
		_E( "Engine.Vegetable.EnableLodFade",		1,				GSem_Interger,	"",								"是否表现Lod的过渡效果"),
		_E( "Engine.Water.DepthEffect",			TRUE,				GSem_Boolean,	"",								"是否表现水下深度效果"),
		_E( "Engine.Water.RealTimeReflect",		TRUE,				GSem_Boolean,	"",								"是否表现准确的实时反射"),
		_E( "Engine.Water.VertexWave",			TRUE,				GSem_Boolean,	"",								"是否表现水面波动"),
		_E( "Engine.Water.DetailLevel",			TRUE,				GSem_Interger,	"Low:0,Middle:1,High:2",		"波动效果级别"),
		_E( "Engine.Helper.NeverShow",			FALSE,				GSem_Boolean,	"",								"是否永远不显示Helper"),
		_E( "Engine.Helper.Layors",			0x7fffffff,				GSem_Flags,	HelperLayor_SemConstraint,								"Helper的Layor"),
		_E( "Engine.Sky.Show",			TRUE,				GSem_Boolean,	"",												"是否绘制天空"),
	};

	c=ARRAY_SIZE(entries);
	return entries;
}
#undef _E