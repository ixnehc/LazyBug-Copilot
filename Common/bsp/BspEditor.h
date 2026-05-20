#pragma once



/***
*memmove - Copy source buffer to destination buffer
*
*Purpose:
*       memmove() copies a source memory buffer to a destination memory buffer.
*       This routine recognize overlapping buffers to avoid propogation.
*       For cases where propogation is not a problem, memcpy() can be used.
*
*Entry:
*       void *dst = pointer to destination buffer
*       const void *src = pointer to source buffer
*       size_t count = number of bytes to copy
*
*Exit:
*       Returns a pointer to the destination buffer
*
*Exceptions:
*******************************************************************************/
#include "BspPoly.h"
#include "BspModel.h"

extern class CBspEditor * GEditor;
class FEditorVisibility;
typedef void (FEditorVisibility::*PORTAL_FUNC)(CBspPoly&,s32,s32,s32,s32);
#define WORLD_MAX			524288.0	/* Maximum size of the world */
#define HALF_WORLD_MAX		262144.0	/* Half the maximum size of the world */
#define HALF_WORLD_MAX1		262143.0	/* Half the maximum size of the world - 1*/
#define MIN_ORTHOZOOM		250.0		/* Limit of 2D viewport zoom in */
#define MAX_ORTHOZOOM		16000000.0	/* Limit of 2D viewport zoom out */
#define DEFAULT_ORTHOZOOM	10000		/* Default 2D viewport zoom */
// Quality level for rebuilding Bsp.
enum EBspOptimization
{
	BSP_Lame,
	BSP_Good,
	BSP_Optimal
};
enum EPolyNodeFilter
{
	F_OUTSIDE				= 0, // Leaf is an exterior leaf (visible to viewers).
	F_INSIDE					= 1, // Leaf is an interior leaf (non-visible, hidden behind backface).
	F_COPLANAR_OUTSIDE		= 2, // Poly is coplanar and in the exterior (visible to viewers).
	F_COPLANAR_INSIDE		= 3, // Poly is coplanar and inside (invisible to viewers).
	F_COSPATIAL_FACING_IN	= 4, // Poly is coplanar, cospatial, and facing in.
	F_COSPATIAL_FACING_OUT	= 5, // Poly is coplanar, cospatial, and facing out.
};
enum ENodePlace
{
	NODE_Back		= 0, // Node is in back of parent              -> Bsp[iParent].iBack.
	NODE_Front		= 1, // Node is in front of parent             -> Bsp[iParent].iFront.
	NODE_Plane		= 2, // Node is coplanar with parent           -> Bsp[iParent].iPlane.
	NODE_Root		= 3, // Node is the Bsp root and has no parent -> Bsp[0].
};
class FCoplanarInfo
{
public:
	s32	    iOriginalNode;
	s32     iBackNode;
	int	    BackNodeOutside;
	int	    FrontLeafOutside;
	int     ProcessingBack;
};

typedef void (*BSP_FILTER_FUNC)(CBspModel			*Model,s32			    iNode,CBspPoly			*EdPoly, EPolyNodeFilter Leaf,ENodePlace 	ENode_Place );
//
// The visibility calculator class.
//
//
// A portal.
//
//
class FPortal : public CBspPoly
{
public:
	// Variables.
	s32	    iBackLeaf, iNode;
	s32	    iFrontLeaf;
	FPortal *GlobalNext, *FrontLeafNext, *BackLeafNext, *NodeNext;
	BYTE	IsTesting, ShouldTest;
	s32		FragmentCount;
	s32	    iZonePortalSurf;

	// Constructor.
	FPortal( CBspPoly &InPoly, s32 iInFrontLeaf, s32 iInBackLeaf, s32 iInNode, FPortal *InGlobalNext, FPortal *InNodeNext, FPortal *InFrontLeafNext, FPortal *InBackLeafNext )
		:	CBspPoly			(InPoly),
		iFrontLeaf		(iInFrontLeaf),
		iBackLeaf		(iInBackLeaf),
		iNode			(iInNode),
		GlobalNext		(InGlobalNext),
		NodeNext		(InNodeNext),
		FrontLeafNext	(InFrontLeafNext),
		BackLeafNext	(InBackLeafNext),
		IsTesting		(0),
		ShouldTest		(0),
		FragmentCount	(0),
		iZonePortalSurf (INDEX_NONE)
	{}

	// Get the leaf on the opposite side of the specified leaf.
	s32 GetNeighborLeafOf( s32 iLeaf )
	{
		//check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		if     ( iFrontLeaf == iLeaf )	return iBackLeaf;
		else							return iFrontLeaf;
	}

	// Get the next portal for this leaf in the linked list of portals.
	FLOAT Area()
	{
		vector3df Cross(0,0,0);
		for( s32 i=2; i<NumVertices; i++ )
			//Cross += (Vertex[i-1]-Vertex[0]) ^ (Vertex[i]-Vertex[0]);
			Cross += (Vertex[i-1].Position -Vertex[0].Position ) .crossProduct (Vertex[i].Position -Vertex[0].Position );
		return Cross.Size();
	}
	FPortal* Next( s32 iLeaf )
	{
		//check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		if     ( iFrontLeaf == iLeaf )	return FrontLeafNext;
		else							return BackLeafNext;
	}

	// Return this portal polygon, facing outward from leaf iLeaf.
	void GetPolyFacingOutOf( s32 iLeaf, CBspPoly &Poly )
	{
		//check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		Poly = *(CBspPoly*)this;
		if( iLeaf == iFrontLeaf ) Poly.Reverse();
	}

	// Return this portal polygon, facing inward to leaf iLeaf.
	void GetPolyFacingInto( s32 iLeaf, CBspPoly &Poly)
	{
		//check( iLeaf==iFrontLeaf || iLeaf==iBackLeaf );
		Poly = *(CBspPoly*)this;
		if( iLeaf == iBackLeaf ) Poly.Reverse();
	}
};

class FEditorVisibility
{
public:
	// Constants.
	enum {MAX_CLIPS=16384};
	enum {CLIP_BACK_FLAG=0x40000000};


	// Constructor.
	//FEditorVisibility( ULevel* InLevel, CBspModel* InModel, s32 InDebug );
	FEditorVisibility( CBspModel* InModel, s32 InExtra )
		:	
		Model			(InModel),
		NumPortals		(0),
		NumClips		(0),
		NumClipTests	(0),
		NumPassedClips	(0),
		NumUnclipped	(0),
		NumBspPortals	(0),
		MaxFragments	(0),
		NumZonePortals	(0),
		NumZoneFragments(0),
		Extra			(InExtra),
		FirstPortal		(NULL),
		NodePortals		(NULL),
		LeafPortals		(NULL)	{}

	// Destructor.
	~FEditorVisibility()
	{
		for( FPortal* Portal=FirstPortal; Portal;  )
		{
			 FPortal* pTempPortal=Portal->GlobalNext ;
			SAFE_DELETE(Portal);
			Portal=pTempPortal;
		}
			
		SAFE_DELETE_ARR(LeafPortals);
		SAFE_DELETE_ARR(NodePortals);
	}

	// Portal functions.
	void AddPortal( CBspPoly &Poly, s32 iFrontLeaf, s32 iBackLeaf, s32 iGeneratingNode, s32 iGeneratingBase );
	void BlockPortal( CBspPoly &Poly, s32 iFrontLeaf, s32 iBackLeaf, s32 iGeneratingNode, s32 iGeneratingBase );
	void TagZonePortalFragment( CBspPoly &Poly, s32 iFrontLeaf, s32 iBackLeaf, s32 iGeneratingNode, s32 iGeneratingBase );
	void FilterThroughSubtree( s32 Pass, s32 iGeneratingNode, s32 iGeneratingBase, s32 iParentLeaf, s32 iNode, CBspPoly Poly, PORTAL_FUNC Func, s32 iBackLeaf );
	void MakePortalsClip( s32 iNode, CBspPoly Poly, s32 Clip, PORTAL_FUNC Func );
	void MakePortals( s32 iNode );
	void AssignLeaves( s32 iNode, s32 Outside );

	// Zone functions.
	void FormZonesFromLeaves();
	void AssignAllZones( s32 iNode, int Outside );
	void BuildConnectivity();
	
	/*-----------------------------------------------------------------------------
	Volume visibility test.
	-----------------------------------------------------------------------------*/
	void TestVisibility();


	CBspModel*			Model;
	s32				Clips[MAX_CLIPS];
	s32				NumPortals, NumLogicalLeaves;
	s32				NumClips, NumClipTests, NumPassedClips, NumUnclipped;
	s32				NumBspPortals, MaxFragments, NumZonePortals, NumZoneFragments;
	s32				Extra;
	s32				iZonePortalSurf;
	FPortal*		FirstPortal;
	FPortal**		NodePortals;
	FPortal**		LeafPortals;
};
class CBspEditor:public IBspEditor
{
public:
	//IMPLEMENT_REFCOUNT_OVERRIDE;
	//void OnRelease()
	//{};
	IMPLEMENT_REFCOUNT;
	 virtual void BulidBspFromIMesh(IRenderSystem *pRS,IMeshSnapshot *pMS,IMtrl *pMtrl,s32	PolyFlags);

	virtual IBspModel * GetBspModel()
	{
		 return m_pBspModel;
	}
	virtual void Init()
	{
		if(m_pBspBrush==NULL)
		{
			m_pBspBrush=new CBspBrush;
			m_pBspBrush->Init();
		} 
		GEditor = this;
	}
	virtual void BuildPortaFromBsp()
	{
		csgRebuild_makePortal(m_pBspModel);
		//SAFE_DELETE(m_pBspBrush);
		//SAFE_DELETE(m_pBspModel);
	};
	void BuildPortal()
	{};
	//---------------------------------------------------------------------------------------
	CBspEditor()
	{
		m_pBspModel=NULL;
		m_pBspBrush=NULL;
	};
	~CBspEditor()
	{
		SAFE_DELETE(m_pBspBrush);
		SAFE_DELETE(m_pBspModel);
	};

	//---------------------------------------------------------------------------------------------------
	

	virtual void SetModel(IBspModel    *pBspModel)
	{
		m_pBspModel=(CBspModel   *)pBspModel;
	}
	virtual void SetBrush(IRenderSystem *pRS, IMeshSnapshot *pMS, IMtrl *pMtrl,bool ifDealWithCOPLANAR, u8 CsgOper, s32	PolyFlags )
	{	
		m_pBspBrush->GetBrushFromMesh(pRS,pMS,pMtrl,ifDealWithCOPLANAR,CsgOper,PolyFlags);
	}

	void BulidBsp();

	void FilterEdPoly
		(
		BSP_FILTER_FUNC	FilterFunc, 
		CBspModel				*Model,
		s32							iNode, 
		CBspPoly					*EdPoly, 
		FCoplanarInfo			CoplanarInfo, 
		s32							Outside
		);
	//
	// Handle a piece of a polygon that was filtered to a leaf.
	//
	void FilterLeaf
		(
		BSP_FILTER_FUNC FilterFunc, 
		CBspModel*			Model,
		s32			    iNode, 
		CBspPoly*			EdPoly, 
		FCoplanarInfo	CoplanarInfo, 
		s32				LeafOutside, 
		ENodePlace		ENodePlace
		);
	
	s32 bspAddNode(CBspModel*		Model, s32         iParent, ENodePlace	NodePlace,DWORD		NodeFlags, CBspPoly*		EdPoly);
	//INT  bspAddPoint( CBspModel *Model, vector3df *V, int Exact );
	INT  bspAddPoint( CBspModel *Model, FModelVertex *V, int Exact );
	int   bspNodeToFPoly
		(
		CBspModel	*Model,
		INT	    iNode,
		CBspPoly	*EdPoly
		);
	void  bspCleanup( CBspModel *Model );
	void bspRefresh( CBspModel *Model, int NoRemapSurfs );
	void bspBuildBounds( CBspModel* Model );
	void BspFilterFPoly( BSP_FILTER_FUNC FilterFunc, CBspModel *Model, CBspPoly *EdPoly );
	INT bspAddVector( CBspModel *Model, vector3df *V, int Normal );
	void TestVisibility( CBspModel* Model, int A, int B );
	void bspRepartition( CBspModel *Model, INT iNode, INT Simple );
	void bspBuild
		(
		CBspModel*				Model, 
		EBspOptimization	Opt, 
		INT					Balance, 
		INT					PortalBias,
		INT					RebuildSimplePolys,
		INT					iNode
		);
	void bspMergeCoplanars( CBspModel* Model, bool RemapLinks, bool MergeDisparateTextures );
	void bspBuildFPolys( CBspModel* Model, bool SurfLinks, INT iNode );
	void csgRebuild_makePortal( CBspModel *Model );
	virtual void BulidBspRenderSurf();
protected:
	CBspModel    *m_pBspModel;
	CBspBrush    *m_pBspBrush;
	u32				FastRebuild:1;
};
