/********************************************************************
	created:	1:3:2009   8:48
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	defines for Asset Renderer
*********************************************************************/

#pragma once

#define MAX_ASSETRENDERER_EXTENT 2000

//////////////////////////////////////////////////////////////////////////
//Asset Renderer
enum RenderStyle
{
	RenderStyle_Normal,
	RenderStyle_Reflect,
	RenderStyle_ShadowMask,
	RenderStyle_VShadowMask,//variance shadow mask
	RenderStyle_RefractDepth,
	RenderStyle_Glow,
	RenderStyle_PassPrepare,
	RenderStyle_Outline,
};

enum RagentType
{
	Ragent_Default=0,	
	Ragent_LightMap,
	Ragent_Liquid,
	Ragent_Trrn,
	Ragent_Tree,
	Ragent_Defer,
	Ragent_PostProcess,
	Ragent_Light,
	Ragent_Sky,
	Ragent_Patches,
	Ragent_Helper,
	Ragent_Vegetable,
	Ragent_Shore,
	Ragent_Shell,
	Ragent_HotBlur,
	Ragent_Halo,
	Ragent_Road,
	Ragent_Grass,
	Ragent_Decal,
	Ragent_Lichen,
	//XXXXX:more ragent
	RagentType_Max,
};


enum GlobalLightingMethod
{
	GLM_None,
	GLM_Simple,
	GLM_Static,
	GLM_Dynamic,
	GLM_StaticTrrn,//静态光照,使用Terrain的LightMap
};

enum ShadowCast
{
	ShadowCast_None,//不接受阴影,不投射阴影
	ShadowCast_All,
	ShadowCast_Static,
	ShadowCast_Dyn,
};

enum SeeThruType
{
	SeeThru_None,
	SeeThru_Permanent,
};

typedef void* SeeThruTargetHandle;
#define SeeThruTargetHandle_Null (0)

typedef void *RatomID;

typedef void* IRatoms;

typedef void *RenvID;



enum RenvType
{
	Renv_None=0,
	Renv_DynLocalLight=1,

	RenvType_Max,
};

#define RenvFlag_Enum 1


typedef BYTE RenderLayorOnTrrnLevel;
#define RenderLayorOnTrrnLevel_Default (0)


//所谓envelope是指一些三角形/line信息,用来在编辑时显示在某个ratom外面,来表示它被选中了
class IMesh;
class IMatrice43;
struct Envelope
{
	void Clear()
	{
		meshes.clear();
		lines.clear();
	}
	struct Mesh
	{
		Mesh()
		{
			matrices=NULL;
			mesh=NULL;
			b2Sided=FALSE;
		}
		i_math::matrix43f mat;
		IMesh *mesh;//临时指针
		IMatrice43 *matrices;//临时指针
		BOOL b2Sided;
	};
	std::vector<Mesh> meshes;
	std::vector<i_math::vector3df> lines;
};

typedef void * OccluderID;
#define OccluderID_Null 0

//Asset Renderer的统计数据
struct PassStats
{
	PassStats()
	{
		memset(this,0,sizeof(*this));
	}
	char name[64];
	DWORD nRatoms;
	DWORD nOccs;
	DWORD nCullInc;
	DWORD nCullExc;
	DWORD nTrrnDP;
};
struct AdrStats
{

	AdrStats()
	{
		nPasses=0;
		nDefBatched=0;
	}

	PassStats passes[32];
	DWORD nPasses;

	DWORD nDefBatched;//CRagentDefault中被归到batch中的ratom个数

	void Clear()
	{
		nPasses=0;
		nDefBatched=0;
	}

	PassStats *NewPassStats(const char *name)
	{
		if(nPasses>=ARRAY_SIZE(passes)-1)
			return NULL;
		PassStats t;
		strcpy(t.name,name);
		passes[nPasses]=t;
		nPasses++;
		return &passes[nPasses-1];
	}

};


enum AssetRendererPart
{
	AdrPart_All,
	AdrPart_NotShell,
	AdrPart_Shell,
};

typedef i_math::rect_sh ShellRect;
typedef i_math::pos2d_sh ShellPos;
typedef i_math::size2d_sh ShellSize;

