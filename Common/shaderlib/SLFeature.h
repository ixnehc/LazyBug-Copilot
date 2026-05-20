#pragma once



//Feature code symbols
//NOTE: should be syncronized with g_FeatureNames

#define FC_MASK(c) (((unsigned __int64)1)<<(c))

#define FC_none FeatureCode(0)
#define FC_base FeatureCode(FC_MASK(0))
#define FC_col FeatureCode(FC_MASK(1))
#define FC_colmod	FeatureCode(FC_MASK(23))
#define FC_vtxcol FeatureCode(FC_MASK(39))
#define FC_bones1 FeatureCode(FC_MASK(2)) 
#define FC_bones2 FeatureCode(FC_MASK(3))
#define FC_bones3 FeatureCode(FC_MASK(4))
#define FC_bones4 FeatureCode(FC_MASK(5))
#define FC_spec FeatureCode(FC_MASK(6))
#define FC_envspec FeatureCode(FC_MASK(37))
#define FC_illum FeatureCode(FC_MASK(7))
#define FC_diffusemap FeatureCode(FC_MASK(8))
#define FC_specularmap FeatureCode(FC_MASK(9))
#define FC_illummap FeatureCode(FC_MASK(16))
#define FC_dirlight_p FeatureCode(FC_MASK(11))
#define FC_pointlight  FeatureCode(FC_MASK(53))
#define FC_uvBaseXform FeatureCode(FC_MASK(12))
#define FC_uvExtXform FeatureCode(FC_MASK(13))
#define FC_normalmap FeatureCode(FC_MASK(14))
#define FC_alphamap FeatureCode(FC_MASK(15))
#define FC_shadowmap FeatureCode(FC_MASK(18)) //shadow map
#define FC_shdwmap_hw FeatureCode(FC_MASK(40))//hardware shadow map
#define FC_lightmap FeatureCode(FC_MASK(19))
#define FC_dirlight_ac		FeatureCode(FC_MASK(56))

#define FC_clipplane	FeatureCode(FC_MASK(28))
#define FC_fog FeatureCode(FC_MASK(46))
#define FC_fog_ground FeatureCode(FC_MASK(47))
#define FC_clipfog FeatureCode(FC_MASK(45))


#define FC_alphatest FeatureCode(FC_MASK(43))//pixel shader alpha test
																			//NOTE:few cards support HW alpha test for a FP RT(such
																			//as D3DFMT_R32F),so we need to do this in pixel shader

#define FC_lm_g FeatureCode(FC_MASK(31))
#define FC_lm_l	FeatureCode(FC_MASK(58))
#define FC_lm_ul FeatureCode(FC_MASK(44))
#define FC_lm_dot3		FeatureCode(FC_MASK(57))
#define FC_lm_shdw	FeatureCode(FC_MASK(26))
#define FC_lm_fullshdw FeatureCode(FC_MASK(32)) 

#define FC_gbuf_x2	FeatureCode(FC_MASK(29))
#define FC_gbuf_x4	FeatureCode(FC_MASK(52))
#define FC_wbuffer	FeatureCode(FC_MASK(30))
#define FC_smmake FeatureCode(FC_MASK(41)) //generate shadow map
#define FC_vsmmake FeatureCode(FC_MASK(42))//generate variance shadow map
#define FC_wobuffer FeatureCode(FC_MASK(63))//warp offset buffer,rg有效

#define FC_warp			FeatureCode(FC_MASK(50))//扭曲效果

#define FC_pointlight_x2 	FeatureCode(FC_MASK(54))
#define FC_pointlight_xN		FeatureCode(FC_MASK(59))

#define FC_sight				FeatureCode(FC_MASK(60))//朝向某个指定方向

#define FC_depthopacity				FeatureCode(FC_MASK(61))//基于深度的的透明度(主要用于soft particle)


//通用的Features
#define FC_common01 FeatureCode(FC_MASK(20)) 
#define FC_common02 FeatureCode(FC_MASK(17)) 
#define FC_common03 FeatureCode(FC_MASK(33)) 
#define FC_common04 FeatureCode(FC_MASK(48))
#define FC_common05 FeatureCode(FC_MASK(49))
#define FC_common06 FeatureCode(FC_MASK(36))
#define FC_common07 FeatureCode(FC_MASK(35))
#define FC_common08 FeatureCode(FC_MASK(34))
#define FC_common09 FeatureCode(FC_MASK(55))

//Default
#define FC_trrnlm FC_common01 //用地表的光照图
#define FC_lichen	FC_common02//地丝效果
#define FC_lichen_ground	FC_common03//地丝地表效果

//地表
#define FC_trrnnmlmap FC_common01 //用来提供地表法线信息的贴图,(注意,这张法线贴图是粗糙的法线贴图(法线位于世界空间内),区别于表现细节的FC_normalmap)
#define FC_trrn2layor FC_common02 //2层贴图
#define FC_trrn3layor FC_common03 //3层贴图
#define FC_trrn4layor FC_common04	//4层贴图
#define FC_trrn1ns FC_common05	//1层法线贴图
#define FC_trrn2ns FC_common06	//2层法线贴图
#define FC_trrn3ns FC_common07//3层法线贴图
#define FC_trrn4ns FC_common08//4层法线贴图
#define FC_trrndraft FC_common09//草图

//Speed Tree
#define FC_spt_wind	FC_common01
#define FC_spt_trunk	FC_common02
#define FC_spt_vbillboard	FC_common03
#define FC_spt_hbillboard FC_common04
#define FC_spt_leafCard FC_common05
#define FC_spt_leafMesh FC_common06

//粒子系统
#define FC_parBB FC_common01//朝向camera
#define FC_parFacing FC_common02//朝向某个指定方向
#define FC_parSpdAlign FC_common03//速度对齐
#define FC_parFree FC_common04//自由旋转
#define FC_parTexAnim FC_common05
#define FC_parTexAnimS FC_common06//S 代表smooth
#define FC_parWarpMap FC_common07//
#define FC_parMesh FC_common08

//water
#define FC_water_depth FC_common01
#define FC_water_rtreflect FC_common02
#define FC_water_vtxwave FC_common03
#define FC_water_in FC_common04



//XXXXX more features


//Features available


#define FC_parFree_obsolete				FeatureCode(FC_MASK(62))//自由旋转
#define FC_parTexAnim_obsolete	FeatureCode(FC_MASK(21))
#define FC_parTexAnimS_obsolete	FeatureCode(FC_MASK(22))//S 代表smooth
#define FC_water_depth_obsolete		FeatureCode(FC_MASK(24))
#define FC_water_rtreflect_obsolete	FeatureCode(FC_MASK(25))
#define FC_water_vtxwave_obsolete	FeatureCode(FC_MASK(27))

#define FC_warpML_obsolete		FeatureCode(FC_MASK(51))//多层扭曲效果


#define FC_NAME(fc) (fc.ToName())

