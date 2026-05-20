#pragma once
#include "../Interfaces/WorldSystem/IBsp.h"

#include "BspPoly.h"
#include<map>

class FVert
{
public:
	// Variables.
	s32 	pVertex;	// Index of vertex.
	s32		iSide;		// If shared, index of unique side. Otherwise INDEX_NONE.
	FVert()
	{
		pVertex=0;
		iSide=0;
	}
};

class CBspPolys 
{
	public:
		std::vector<CBspPoly* > Element;
	// Constructors.
		CBspPolys(){}//Element.resize(0);
	//
	~CBspPolys()
	{
		CleanPloys();
	}
	BOOL INIPolysFromMesh( IRenderSystem *pRS,IMeshSnapshot *snapshot, IMtrl *mtrl,s32	PolyFlags);
	void CleanPloys()
	{
		for (int i=0;i<Element.size();i++)
		{
			SAFE_DELETE(Element[i]);
		}
		Element.resize(0);
	}
	void AddOnePoly();
	void DealWithCOPLANAR(){}
};
enum EBspNodeFlags
{
	// Flags.
	NF_NotCsg			= 0x01, // Node is not a Csg splittr, i.e. is a transparent poly.
	NF_NotVisBlocking   = 0x04, // Node does not block visibility, i.e. is an invisible collision hull.
	NF_BrightCorners	= 0x10, // Temporary.
	NF_IsNew 		 	= 0x20, // Editor: Node was newly-added.
	NF_IsFront     		= 0x40, // Filter operation bounding-sphere precomputed and guaranteed to be front.
	NF_IsBack      		= 0x80, // Guaranteed back.
};
enum ECsgOper
{
	CSG_Active              =0,
	CSG_Add                 =1,
	CSG_Subtract            =2,
	CSG_Intersect           =3,
	CSG_Deintersect         =4,
	CSG_MAX                 =5,
};
struct FZoneSet
{
	FZoneSet(): MaskBits(0) {}

	// Pre-defined sets.

	static FZoneSet IndividualZone(s32 ZoneIndex) { return FZoneSet(((u64)1) << ZoneIndex); }
	static FZoneSet AllZones() { return FZoneSet(~(u64)0); }
	static FZoneSet NoZones() { return FZoneSet(0); }

	// Accessors.

	bool ContainsZone(s32 ZoneIndex) const
	{
		return (MaskBits & (((u64)1) << ZoneIndex)) != 0;
	}

	void AddZone(s32 ZoneIndex)
	{
		MaskBits |= (((u64)1) << ZoneIndex);
	}

	void RemoveZone(s32 ZoneIndex)
	{
		MaskBits &= ~(((u64)1) << ZoneIndex);
	}

	bool IsEmpty() const { return MaskBits == 0; }
	bool IsNotEmpty() const { return MaskBits != 0; }

	// Operators.

	friend FZoneSet operator|(const FZoneSet& A,const FZoneSet& B)
	{
		return FZoneSet(A.MaskBits | B.MaskBits);
	}

	friend FZoneSet& operator|=(FZoneSet& A,const FZoneSet& B)
	{
		A.MaskBits |= B.MaskBits;
		return A;
	}

	friend FZoneSet operator&(const FZoneSet& A,const FZoneSet& B)
	{
		return FZoneSet(A.MaskBits & B.MaskBits);
	}

	// Serialization.

	/*friend FArchive& operator<<(FArchive& Ar,FZoneSet& S)
	{
	return Ar << S.MaskBits;
	}*/

private:

	FZoneSet(u64 InMaskBits): MaskBits(InMaskBits) {}

	/** A mask containing a bit representing set inclusion for each of the 64 zones. */
	u64	MaskBits;
};
struct FBspNode // 58 bytes
{
	enum {MAX_NODE_VERTICES=16};	// Max vertices in a Bsp node, pre clipping.
	enum {MAX_FINAL_VERTICES=24};	// Max vertices in a Bsp node, post clipping.
	enum {MAX_ZONES=64};			// Max zones per level.

	// Persistent information.
	plane3df		Plane;			// 16 Plane the node falls into (X, Y, Z, W).
	FZoneSet	ZoneMask;		// 8  Bit mask for all zones at or below this node (up to 64).
	s32			iVertPool;		// 4  Index of first vertex in vertex pool, =iTerrain if NumVertices==0 and NF_TerrainFront.
	s32			iSurf;			// 4  Index to surface information.
	s32			iRenderSurf;

	DWORD	MaterialID;
	DWORD	SurflID;
	//DWORD	ZoneID;		//

	// iBack:  4  Index to node in front (in direction of Normal).
	// iFront: 4  Index to node in back  (opposite direction as Normal).
	// iPlane: 4  Index to next coplanar poly in coplanar list.
	union 
	{ 
		s32 iBack; 
		s32 iChild[1]; 
	};
	s32 iFront;
	s32 iPlane;

	s32		iCollisionBound;// 4  Collision bound.

	u8		iZone[2];		// 2  Visibility zone in 1=front, 0=back.
	u8		NumVertices;	// 1  Number of vertices in node.
	u8		NodeFlags;		// 1  Node flags.
	s32		iLeaf[2];		// 8  Leaf in back and front, INDEX_NONE=not a leaf.

	// Functions.
	bool IsCsg( u32 ExtraFlags=0 ) const
	{
		return (NumVertices>0) && !(NodeFlags & (NF_IsNew | NF_NotCsg | ExtraFlags));
	}
	bool ChildOutside( s32 iChild, bool Outside, u32 ExtraFlags=0 ) const
	{
		return iChild ? (Outside || IsCsg(ExtraFlags)) : (Outside && !IsCsg(ExtraFlags));
	}
	//friend FArchive& operator<<( FArchive& Ar, FBspNode& N );
};


struct FBspSurf
{
public:

	//UMaterialInstance*	Material;		// 4 Material.
	u32					PolyFlags;		// 4 Polygon flags.//Portal或实体
	s32							vNormal;		// 4 Index to polygon normal.
	plane3df						Plane;			// 16 The plane this surface lies on.

	// Functions.
	//friend FArchive& operator<<( FArchive& Ar, FBspSurf& Surf );

	s32					pBase;			// 4 Polygon & texture base point index (where U,V==0,0).
	s32					vTextureU;		// 4 Texture U-vector index.
	s32					vTextureV;		// 4 Texture V-vector index.
	s32					iBrushPoly;		// 4 Editor brush polygon index.
	//ABrush*			Actor;			// 4 Brush actor owning this Bsp surface.
	f32					LightMapScale;	// 4 The number of units/lightmap texel on this surface.
};


struct FLeaf
{
	s32		iZone;          // The zone this convex volume is in.

	// Functions.
	FLeaf()
	{}
	FLeaf(s32 iInZone):
	iZone(iInZone)
	{}
	//friend FArchive& operator<<( FArchive& Ar, FLeaf& L )
	//{
	//	Ar << L.iZone;
	//	return Ar;
	//}
};

struct FConvexVolume
{
public:
	std::vector<plane3df>	Planes;
	bool ClipPolygon(CBspPoly& Polygon) const;
};




//
//	FZoneProperties
//

struct FZoneProperties
{
public:
	// Variables.
	//AZoneInfo*	ZoneActor;		// Optional actor defining the zone's property.
	f32		LastRenderTime;	// Most recent level TimeSeconds when rendered.
	FZoneSet	Connectivity;	// (Connect[i]&(1<<j))==1 if zone i is adjacent to zone j.
	FZoneSet	Visibility;		// (Connect[i]&(1<<j))==1 if zone i can see zone j.

};
class CBspBrush
{
public:
	CBspBrush()
	{
		Polys=NULL;
	};
	~CBspBrush()
	{
		SAFE_DELETE(Polys);
	};
	//释放pBspBrush
	virtual void Init()
	{
		if(Polys==NULL)
			Polys=new CBspPolys;
	}
	void GetBrushFromMesh( IRenderSystem *pRS,IMeshSnapshot *pMS, IMtrl *pMtrl,bool bDealWithCOPLANAR, u8 u8CsgOper, s32 s32PolyFlags)
	{
		mCsgOper=u8CsgOper;
		mPolyFlags=s32PolyFlags;
	    Polys->INIPolysFromMesh(pRS,pMS,pMtrl,s32PolyFlags);
		if(bDealWithCOPLANAR)
		{
			Polys->DealWithCOPLANAR();
		}
	}
public:
	u8					mCsgOper;
	s32					mPolyFlags;
	CBspPolys*			Polys;	
};

//
//	FPointRegion - Identifies a unique convex volume in the world.
//

struct FPointRegion
{
	// Variables.
	//class AZoneInfo* Zone;			// Zone actor.
	INT				 iLeaf;			// Bsp leaf.
	BYTE             ZoneNumber;	// Zone number.

	// Constructors.

	FPointRegion( )
		:	iLeaf(INDEX_NONE), ZoneNumber(0)
	{}
	FPointRegion( INT InLeaf, BYTE InZoneNumber )
		:	iLeaf(InLeaf), ZoneNumber(InZoneNumber)
	{}
};
//struct Point_Old_New
//{
//	WORD indexInBspModelPoint;
//	WORD indexInRenderSurf;
//
//	// Constructors.
//
//	Point_Old_New()
//		:	indexInBspModelPoint(INDEX_NONE),indexInRenderSurf(INDEX_NONE)
//	{}
//	Point_Old_New(WORD bsp,WORD surf)
//		:	indexInBspModelPoint(bsp),indexInRenderSurf(surf)
//	{}
//};
class CRenderSurf
{
	public:
	CRenderSurf()
	{
		i_pVB=NULL;
		//i_pMtrl=NULL;
	}
	~CRenderSurf()
	{
		clearVB();
		//SAFE_RELEASE(i_pMtrl);
	}
	clearVB()
	{
		SAFE_RELEASE(i_pVB);
	}
	setVB(IVertexBuffer * pVB)
	{
		i_pVB=pVB;
	}
	DWORD getMaterialID()
	{
		return  MaterialID;
	}


	IVertexBuffer *i_pVB;
	DWORD MaterialID;			//IMtrl ID,
	DWORD MaterialSurfID;	//IMtrl SurfID   
	//IMtrl 's layer1-n 的 tex Num 必须和VB 中的tex Num匹配,否则绘制错误.
	//mesh snap shot 每个surface 的 vertex 的 TexNum 相同
	DWORD SurflID;		//
	DWORD ZoneID;		//
	DWORD ZoneLen;    //起始位置记录长度，其余为0.

	std::vector<WORD> renderSurfPoints;//Point index
	std::vector<WORD> renderSurfPointIndices;
	//std::vector<Point_Old_New> renderSurfPointIndices---->std::vector<WORD> renderSurfPointIndices  
	//
	//renderSurfPointIndices  (n,-1)->(n,m)
	//>renderSurfPoints-> 一一修正

	//maybe no need  Point_Old_New  only DWORD 
	//>renderSurfPoints 也无须修正  在生成renderSurfPointIndices，仅记录下标即可。

	//每个surfs		都有一个AABB
	//surface			按zoneID 排列
	//game	状态	zoneID ----- volumeCvx
	//edit状态		volumeCvx 检测所有 surface
	//在bspmodel	加入 vector<zoneINsufBegin> 对应在render Surf  中起始位置
};

class CBspModel:public IBspModel
{
public:
	/*IMPLEMENT_REFCOUNT_OVERRIDE;
	void OnRelease();*/
	CBspModel()
	{
		Polys=NULL;
		PolysTemp=NULL;
		RenderSurfIndex=NULL;
	}
	IMPLEMENT_REFCOUNT;
	virtual void WriteToPKG(){}
	virtual void ReadFromPKG(){}
	~CBspModel()
	{
		SAFE_DELETE(Polys);
		SAFE_DELETE(PolysTemp);

		for (int i=0;i<Nodes.size();i++)
		{
			SAFE_DELETE(Nodes[i]);
		}
		Nodes.resize(0);

		for (int i=0;i<Surfs.size();i++)
		{
			SAFE_DELETE(Surfs[i]);
		}
		Surfs.resize(0);

		for (int i=0;i<Leaves.size();i++)
		{
			SAFE_DELETE(Leaves[i]);
		}
		Leaves.resize(0);

		for (int i=0;i<Mtrls.size();i++)
		{
			SAFE_DELETE(Mtrls[i]);
		}	
		SAFE_DELETE(RenderSurfIndex);
	}
	//void GetVolumeMatchingZoneID(CBspVisibilitySet *pBVS);//传入视锥参数,得到(被portal 剪裁过的视锥+zone ID)组.
	// Arrays and subobjects.
	//CBspPoly							TempPoly;
	CBspPolys*							Polys;
	CBspPolys*							PolysTemp;//用于递归调用时的临时分配内存
	std::vector<FBspNode*>		Nodes;
	std::vector<FVert>				Verts;

	std::vector< vector3df>			Vectors;//面向量
	//Verts 记录 Points的索引
	std::vector< FModelVertex >	Points;//need FModelVertex struct x,y,z, u,v,Normal;
	std::vector< CRenderSurf >		RenderSurfs;//<---------------------bsp生成后再，生成。
	std::vector <IMtrl *>				Mtrls;
	static std::vector <std::string >            strBspMtrls;
	matrix43f									matrixBsp;				  
	//bool  IsInRenderSurf( std::string pathin)
	 bool  IsInMtrls( IMtrl* MaterialIn,DWORD & MaterialID)
	{
		std::string pathin;//pathMy,
	    pathin=MaterialIn->GetPath();
		for( int i=0; i<RenderSurfs.size(); i++ ) 
		{
			//pathMy=Mtrls[i]->GetPath();
			if(Mtrls[i]->GetPath()==pathin)
			{
				MaterialID=i;
				return true;
			}				
		}
		//add to 
		return false;
	}
    //surface   Vectors/面向量  Normals/点向量  Points

	std::vector<FBspSurf*>				Surfs;
	//std::vector<FRenderSurf*>				RenderSurfs;
	std::vector<s32>							LeafHulls;
	std::vector<FLeaf*>					Leaves;
	std::vector<s32>							PortalNodes;
	// Other variables.
	bool						RootOutside;
	bool						Linked;
	s32						MoverLink;
	s32						NumSharedSides;
	s32						NumZones;
	FZoneProperties				Zones[FBspNode::MAX_ZONES];
	DWORD *              RenderSurfIndex;
	//FBoxSphereBounds			Bounds;


	

	f32 FindNearestVertex
		
		(
		const vector3df	&SourcePoint,
		vector3df				&DestPoint,
		f32						MinRadius,
		s32						&pVertex
		) 
	const;
	FPointRegion PointRegion( vector3df Location ) const;
	void EmptyModel( INT EmptySurfInfo, INT EmptyPolys );

	virtual bool BuildRenderSurf(IRenderSystem *pRS );
	virtual DWORD * GetRenderSurfIndexArray(DWORD & sizeNum)
	{
		SAFE_DELETE(RenderSurfIndex);
        RenderSurfIndex= new  DWORD[1];
		return RenderSurfIndex;
	}
	virtual void  Get_1VB_2Mtrl(DWORD  index,IVertexBuffer * & pVB,IMtrl * & pMtrlID ,DWORD  &surfIndex)
	{
	
	}

};









class CBspVisibilitySet
{
public:
	// Constructor.
	CBspVisibilitySet(CBspModel* InModel,const plane3df& InViewOrigin,const FZoneSet& InIgnoreZones,bool InUsePortals):
	  pBspModel(InModel),
		  ViewOrigin(InViewOrigin),
		  VisibleZones(FZoneSet::NoZones()),
		  IgnoreZones(InIgnoreZones),
		  UsePortals(InUsePortals)
	  {}
	  /* Adds a visibility volume to a specific zone.
	  * Extends the portals visible through the new visibility volume and recurses with the extended visibility volumes.
	  *
	  * @param	ZoneIndex - The index of the zone the Volume is a visibility volume for.
	  *
	  * @param	Volume - The visibility volume
	  */
	  void AddVisibilityVolume(INT ZoneIndex,const FConvexVolume& Volume);
protected:
	CBspModel	*pBspModel;
	typedef std::pair <s32,FConvexVolume> Int_Pair;
	std::multimap<s32,FConvexVolume> VisibilityVolumes;

	/** The set of zones which are possibly visible. */
	FZoneSet VisibleZones;

	/** The set of zones which should be entirely ignored. */
	FZoneSet IgnoreZones;

	/**
	* The point which visibility is being computed for.  Uses a homogenous vector to represent both finite points
	* and infinitely distant points(directions).
	*/
	plane3df ViewOrigin;

	/** True if the viewer can see through portals. */
	bool UsePortals;
};

