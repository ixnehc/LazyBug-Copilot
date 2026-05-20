
#pragma once

#include "bitset/bitset.h"

enum SkeletonMatchLevel
{
	SklMatch_None,//Totally not matched
	SklMatch_PartialTopo,//patially matched in topology
	SklMatch_Partial,//patially matched in both topology and name
	SklMatch_FullTopo,//fully matched in topology (but not in name)
	SklMatch_Full,//Totally matched
};

struct BoneCtrl
{
	i_math::vector3df off;
	i_math::quatf rot;
	float scale;
};

struct BoneCtrls
{
	BoneCtrls()
	{
		bcs_=NULL;
		nBC=0;
		xfms=NULL;
		matBase=NULL;
	}

	i_math::xformf *xfms;
	BoneCtrl *bcs_;
	DWORD nBC;
	i_math::matrix43f *matBase;

	Bitset<8> bonemask;
};

struct IKCtrls
{
	IKCtrls()
	{
		xfms=NULL;
		matBase=NULL;
	}

	i_math::xformf *xfms;
	i_math::matrix43f *matBase;
};


class SkeletonInfo;
class IMatrice43;
//三种matrice:
//1.bone matrice:matrix在它的parent空间里
//2.skeleton matrice:matrix在世界坐标系里
//3.skin matrice:用于直接计算蒙皮的矩阵(包含了offset)
class ISkeleton
{
public:
	INTERFACE_REFCOUNT;
	virtual DWORD GetBoneCount()=0;
	virtual BOOL GetBoneDefXform(DWORD iBone,i_math::xformf &xformDef)=0;
	virtual BOOL GetBoneOffMat(DWORD iBone,i_math::matrix43f &matOff)=0;
	virtual BOOL GetBoneParent(DWORD iBone,int &iParent)=0;//if no parent,iParent will be filled with -1
	virtual SkeletonInfo*GetSkeletonInfo()=0;//**stl used

	//根据bone matrice计算skeleton matrice
	virtual BOOL CalcSkeletonMatrice(IMatrice43*mats,i_math::xformf *xfms,i_math::matrix43f *matBase)=0;
	virtual BOOL CalcSkeletonXforms(i_math::xformf *xfms,DWORD c,i_math::matrix43f *matBase)=0;
	//根据skeletion matrice计算skin matrice
	virtual BOOL CalcSkinMatrice(IMatrice43*mats)=0;

	//find a match for sklSub, in me
	//validbones is an indice table indicating the bones to be checked in sklSub,if it's NULL
	//or EMPTY,use all the bones in sklSub to check
	//resultindice returns the result indice to the bones of this ISkeleton,for each bone in 
	//sklSub(if validbones is NULL or EMPTY),or for each bone in validbones
	//NOTE:if fully matched(including SklMatch_FullTopo),resultindice will be cleared to empty.
	virtual SkeletonMatchLevel FindMatch(std::vector<DWORD>&resultindice,
					ISkeleton *sklSub,std::vector<DWORD>*validbones)=0;//**stl used
};

//matrix43 array
class IMatrice43
{
public:
	INTERFACE_REFCOUNT;
	virtual DWORD GetVer()=0;
	virtual BOOL Set(i_math::matrix43f &mat)=0;
	virtual BOOL Set(i_math::matrix43f *mats,DWORD count)=0;
	virtual DWORD GetCount()=0;
	virtual BOOL SetCount(DWORD count)=0;
	//get the ptr for reading(will NOT increase version)
	//the returned pointer should not be kept
	virtual i_math::matrix43f * GetPtr()=0;
	virtual i_math::matrix43f *QueryPtr()=0;//query the ptr to modify(will increase version)
};

class IShader;

struct FeatureCode;
class ICamera
{
public:
	INTERFACE_REFCOUNT;
	virtual BOOL IsPerspective()=0;
	virtual BOOL IsOrtho()=0;
	virtual BOOL SetPosTarget(i_math::vector3df &eye,i_math::vector3df &at,i_math::vector3df &up)=0;
	virtual BOOL SetPosTarget(i_math::vector3df &eye,i_math::vector3df &at)=0;//use 0,1,0 as up
	virtual BOOL SetNearFar(i_math::f32 distNear,i_math::f32 distFar)=0;
	virtual BOOL GetNearFar(i_math::f32 &distNear,i_math::f32 &distFar)=0;
	virtual BOOL SetFov(i_math::f32 fov)=0;//angle in rad
	virtual BOOL SetAspectRatio(i_math::f32 r)=0;//ver/hor
	virtual BOOL SetOffCenterOrtho(i_math::f32 l,i_math::f32 t,i_math::f32 r,i_math::f32 b)=0;
	virtual BOOL SetOffCenterOrtho(i_math::rectf &rc)=0;
	virtual BOOL GetOffCenterOrtho(i_math::rectf &rc)=0;
	virtual BOOL Bind(IShader *shader)=0;//bind to shader
	virtual void TransPos(i_math::vector3df &pos)=0;//transform a position from world space to projected space
	virtual void Clone(ICamera *camSrc)=0;//copy content from camSrc
	virtual BOOL GetEyePos(i_math::vector3df &eye)=0;
	virtual BOOL GetEyeLookAt(i_math::vector3df &at)=0;
	virtual BOOL GetEyeDir(i_math::vector3df &dir)=0;//return a normalized dir
	virtual BOOL GetEyeMat(i_math::matrix43f &mat)=0;
	virtual BOOL GetFov(i_math::f32 &fov)=0;//angle in rad
	virtual BOOL GetAspectRatio(i_math::f32 &r)=0;//ver/hor
	virtual BOOL GetViewFrustum(i_math::volumeCvxf &vol)=0;
	virtual BOOL GetViewProj(i_math::matrix44f &mat)=0;
	virtual BOOL GetViewProj_Raw(i_math::matrix44f &mat)=0;
	virtual BOOL GetView(i_math::matrix44f &mat)=0;
	virtual BOOL GetProj(i_math::matrix44f &mat)=0;
	virtual BOOL GetXAxis(i_math::vector3df &dir)=0;
	virtual BOOL GetYAxis(i_math::vector3df &dir)=0;
	virtual BOOL GetZAxis(i_math::vector3df &dir)=0;
	virtual void TransPosInverse(i_math::vector3df &pos)=0;//transform a position from projected space to world space
	virtual BOOL CalcHitProbe(i_math::line3df &probe,int x,int y,i_math::recti &rcViewport,float length)=0;
	virtual BOOL EnableClipPlane(BOOL bEnable,i_math::plane3df *plane)=0;
	virtual i_math::plane3df *GetClipPlane()=0;//if oblique clip is not enabled ,return NULL

	virtual BOOL GetProjScaleMask(i_math::recti &rcScrn,i_math::vector3df &pos,i_math::matrix43f &matScale)=0;

	virtual FeatureCode *GetFC(DWORD &nFC)=0;//the first is intended one,the others are fallbacks
	virtual void GetFrustumCorners(i_math::vector3df * corners/* i_math::vector3df corners[8]*/) =0;
	/*
		Font: 微软雅黑(msyh),16
		1----0			5----4
		|		 |			|		|
		2----3			6----7
		// near			far
	*/
};



enum LightMode
{
	LM_None,
	LM_Dir,
	LM_Point,
	//		LT_Spot,
};

enum LightFlag
{
	LF_Dyn=1,
	LF_ShadowMap=2,
	LF_ShadowVolume=4,

	forcedword=0xffffffff,
};


struct LightInfo
{
	LightInfo()
	{
		mode=LM_None;
	}

	LightMode mode;

	i_math::vector3df pos;
	i_math::vector3df dir;//LT_Dir
	i_math::color4df amb;
	i_math::color4df diff;
	i_math::color4df spec;
};

class ITexture;
class ILight
{
public:
	INTERFACE_REFCOUNT;

	virtual LightMode GetMode()=0;
	virtual LightFlag GetFlag()=0;
	virtual void SetFlag(LightFlag flag)=0;
	virtual void ModFlag(LightFlag flagToAdd,LightFlag flagToRemove)=0;
	virtual FeatureCode *GetFC(DWORD &nFC)=0;//the first is intended one,the others are fallbacks

	virtual void SetDirLight(i_math::vector3df &dir,DWORD amb,DWORD diff,DWORD spec)=0;
	virtual LightInfo *QueryInfo()=0;
	virtual LightInfo *GetInfo()=0;

	virtual void Clone(ILight *lgtSrc)=0;

	virtual void SetShadowMap(ITexture *shamap)=0;

	//Bind the light to the shader(set the light' related effect param to the shader)
	//generally for a certain type of light,the shader may maintain an array of light param
	//iSlot indicate which element this light should be put into that array
	virtual BOOL Bind(IShader *shader,DWORD iSlot)=0;


};



class ICvxVolume
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL BuildFromCamera(ICamera *cam)=0;

	virtual BOOL GetVolume(i_math::volumeCvxf &vol)=0;
	virtual BOOL SetVolume(i_math::volumeCvxf &vol)=0;

	//the returned pointers should NOT be kept for later use
	//edges will contain nEdge*2 DWORD
	virtual BOOL GetFrame(i_math::vector3df *&corners,DWORD &nCorner,
		DWORD *&edges,DWORD &nEdge)=0;

	//the returned pointers should NOT be kept for later use
	virtual BOOL GetCorners(i_math::vector3df *&corners,DWORD &nCorner)=0;

	//将volume朝着某个平面的方向拉伸出去,并和这个平面构成一个新的volume
	//pl必须在当前的volume的外部
	virtual BOOL ExtrudeToPlane(i_math::plane3df &pl)=0;

	//v should be outside the volume,and the current volume should not be empty
	virtual BOOL AddCorner(i_math::vector3df &v)=0;


};
