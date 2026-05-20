#pragma once

#include "../gds/GDefines.h"

//Effect Param Flag
#define EPF_MtrlEdit 1//this param is a mtrl param and could be edit in res editor
#define EPF_MtrlView 2//this param is a mtrl param and could be viewed(but NOT editable) in res editor

//effect param symbols
//NOTE:EP_xxxx ,the xxxx should be the same with in g_EPInfo[]
enum EffectParam
{
	EP_None=-1,
	EP_colDif,
	EP_colSpec,
	EP_colIllum,
	EP_fogparam,
	EP_fogcolor,
	EP_fogparam_w,//w代表Water,rgb为颜色,a为水的可视距离
	EP_shiness,
	EP_shinestr,
	EP_diffusemap,
	EP_specmap,
	EP_illummap,
	EP_world,
	EP_view,
	EP_proj,
	EP_viewproj,
	EP_proj2D,//proj for 2D drawing
	EP_camerapos,
	EP_nearfardist,
	EP_dirDL,
	EP_ambDL,
	EP_difDL,
	EP_specDL,
	EP_alphatestref,
	EP_uvBaseXform,
	EP_uvExtXform,
	EP_uvBaseAddr,
	EP_uvExtAddr,
	EP_normalmap,
	EP_shadowmap,
	EP_tlShadowMap,
	EP_shadowdist,//shadow 距离
	EP_alphamap,

	EP_difmul,
	EP_difadd,

	EP_lm_0,
	EP_lm_1,
	EP_lm_2,
	EP_lm_shdw,
	EP_lm_color_global,
	EP_lm_color_local,
	EP_lm_scale,

	EP_envlightmap,

	EP_lisproj,

	EP_clipplane,

	EP_ambcube,

	EP_scrnmap,//Screen Map
	EP_scrndepth,//Screen Depth Map

	EP_warpmap,
	EP_warpblend,//Warp混合模式的值,见ShaderBlendMode
	EP_warpstr,//扭曲的强度

	EP_sight,
	EP_sightmap,

	EP_rngDepthOpacity,

	EP_time,

	//Post Process maps
	EP_rtsize,//render target size
	EP_ppmap01,
	EP_ppmap02,
	EP_ppmap03,
	EP_ppmap04,
	EP_ppmap05,
	EP_ppmap06,
	EP_ppmap07,
	EP_ppmap08,  

	EP_ppmapsize01,
	EP_ppmapsize02,
	EP_ppmapsize03,
	EP_ppmapsize04,
	EP_ppmapsize05,
	EP_ppmapsize06,
	EP_ppmapsize07,
	EP_ppmapsize08,

	EP_paramsPL,
	EP_countPL,

//General use params
	EPG_f_01,
	EPG_f_02,

	EPG_fx3_01,
	EPG_fx3_02,
	EPG_fx3_03,

	EPG_fx4_01,
	EPG_fx4_02,

	EPG_fx16_01,
	EPG_fx16_02,

	EPG_map_01,
	EPG_map_02,

	EPG_s_01,//signed int
	EPG_s_02,
	
	EPG_fx2_01,
	EPG_fx2_02,

	EPG_f_array_01,//float array
	EPG_f_array_02,

//XXXXX more effect params
	EP_Max,
};


extern DWORD SizeOfEPInfo();
extern EffectParam EPfromName(const char *name);
extern int EPInfo_IndexFromEP(EffectParam ep);
extern const char *EPInfo_GetEPName(DWORD idx);
extern const char *EPInfo_GetName(DWORD idx);
extern const char *EPInfo_GetShowName(DWORD idx);
extern const char *EPInfo_GetDesc(DWORD idx);
extern EffectParam EPInfo_GetEP(DWORD idx);
extern int EPInfo_GetArraySize(DWORD idx);
extern GVarType EPInfo_GetVarType(DWORD idx);
extern GSem& EPInfo_GetVarSem(DWORD idx);
extern DWORD EPInfo_GetFlag(DWORD idx);

