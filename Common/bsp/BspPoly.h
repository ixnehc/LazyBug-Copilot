#pragma once
//
// A general-purpose polygon used by the editor.  An CBspPoly is a free-standing
// class which exists independently of any particular level, unlike the polys
// associated with Bsp nodes which rely on scads of other objects.  FPolys are
// used in UnrealEd for internal work, such as building the Bsp and performing
// boolean operations.
#include "../fvfex/fvfex.h"
class CBspModel;
class CBspBrush;

//#ifndef SAFE_DELETE
//#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
//#endif
//
#ifndef SAFE_DELETE_ARR
#define SAFE_DELETE_ARR(p)       { if(p) { delete [] (p);     (p)=NULL; } }
#endif
enum ESplitType
{
	SP_Coplanar		= 0, // Poly wasn't split, but is coplanar with plane
	SP_Front		= 1, // Poly wasn't split, but is entirely in front of plane
	SP_Back			= 2, // Poly wasn't split, but is entirely in back of plane
	SP_Split		= 3, // Poly was split into two new editor polygons
};

enum CBspPolyFlags
{
	// Regular in-game flags.
	PF_Invisible		= 0x00000001,	// Poly is invisible.
	PF_NotSolid			= 0x00000008,	// Poly is not solid, doesn't block.
	PF_Semisolid	  	= 0x00000020,	// Poly is semi-solid = collision solid, Csg nonsolid.
	PF_GeomMarked	  	= 0x00000040,	// Geometry mode sometimes needs to mark polys for processing later.
	PF_TwoSided			= 0x00000100,	// Poly is visible from both sides.
	PF_Portal			= 0x04000000,	// Portal between iZones.
	PF_Mirrored         = 0x20000000,   // Mirrored BSP surface.

	// Editor flags.
	PF_Memorized     	= 0x01000000,	// Editor: Poly is remembered.
	PF_Selected      	= 0x02000000,	// Editor: Poly is selected.

	// Internal.
	PF_EdProcessed 		= 0x40000000,	// FPoly was already processed in editorBuildFPolys.
	PF_EdCut       		= 0x80000000,	// FPoly has been split by SplitPolyWithPlane.

	// Combinations of flags.
	PF_NoEdit			= PF_Memorized | PF_Selected | PF_EdProcessed | PF_EdCut,
	PF_NoImport			= PF_NoEdit | PF_Memorized | PF_Selected | PF_EdProcessed | PF_EdCut,
	PF_AddLast			= PF_Semisolid | PF_NotSolid,
	PF_NoAddToBSP		= PF_EdCut | PF_EdProcessed | PF_Selected | PF_Memorized
};
struct FModelVertex
{
	vector3df			Position;
	void *              buffer;    //不包括position
	FVFEx              modelVF;
	f32                  bufferSize;

	vector3df	        Normal;//点向量
	f32					TexNum;
	f32					TexCoordx[8];
	f32					TexCoordy[8];

   inline bool operator==( FModelVertex Other )//根据buffer,modelVF重写
   {
		   if( *this != Other )
			   return 0;
	   return 1;
   }
   inline bool operator!=( FModelVertex Other )////根据buffer,modelVF重写
   {
		  /* if( Position != Other.Position||Normal!=  Other.Normal)
			   for(int i =0;i<TexNum;i++)
				   if(TexCoordx[i]!=Other.TexCoordx[i]||TexCoordy[i]!=Other.TexCoordy[i])
								 return 1;*/
	   if( FPointsAreSame(Position , Other.Position)&&modelVF== Other.modelVF )
	   {
		   DWORD  mySize= fvfSize(modelVF);
		   for (int i=0;i<mySize;i++)
		   {
			   if(((BYTE*)buffer)[i]!= ((BYTE*)Other.buffer)[i]) return 1;
		   }		
		   return 0;
	   }
	   return 1;
   }
	//FVector2D		TexCoord;
};
class CBspPoly
{
public:
	enum {MAX_VERTICES=16}; // Maximum vertices an CBspPoly may have.
	enum {VERTEX_THRESHOLD=MAX_VERTICES-2}; // Threshold for splitting into two.

	vector3df			    Normal;					// Normal of polygon.
	//vector3df				Vertex[MAX_VERTICES];	// Actual vertices.
	u32						PolyFlags;				// CBspPoly & Bsp poly bit flags (PF_).
	s32						NumVertices;			// Number of vertices.

	FModelVertex      Vertex[MAX_VERTICES];	// Actual vertices.had UV
	DWORD				MaterialID;
	DWORD				SurflID;
	vector3df				Normals[MAX_VERTICES];	// Actual vertices.

	//vector3df				Base;					// Base point of polygon.
	FModelVertex		Base;					// Base point of polygon.
	vector3df				TextureU;				// Texture U vector.
	vector3df				TextureV;				// Texture V vector.
	//string                   strMaterial;
		
	//ABrush*						Actor;					// Brush where this originated, or NULL.
	//UMaterialInstance*	Material;				// Material.
	//FName						ItemName;				// Item name.
	
	s32					iLink;					// iBspSurf, or brush fpoly index of first identical polygon, or MAXWORD.
	s32					iBrushPoly;				// Index of editor solid's polygon this originated from.
	u32					SmoothingMask;			// A mask used to determine which smoothing groups this polygon is in.  SmoothingMask & (1 << GroupNumber)
	f32					LightMapScale;			// The number of units/lightmap texel on this surface.

	//std::vector<s32>		SurfaceIDs;
	//std::vector<s32>		MeshIDs;

	// Custom functions.
	CBspPoly();
	void Init();
	void Reverse();
	void SplitInHalf(CBspPoly *OtherHalf);
	void Transform(const vector3df &PreSubtract,const vector3df &PostAdd);
	int Fix();
	int CalcNormal( bool bSilent = 0 );
	int SplitWithPlane(const vector3df &Base,const vector3df &Normal,CBspPoly *FrontPoly,CBspPoly *BackPoly,int VeryPrecise) const;
	int SplitWithNode(const CBspModel *Model,s32 iNode,CBspPoly *FrontPoly,CBspPoly *BackPoly,int VeryPrecise) const;
	int SplitWithPlaneFast(const plane3df Plane,CBspPoly *FrontPoly,CBspPoly *BackPoly) const;
	int Split(const vector3df &Normal, const vector3df &Base, int NoOverflow=0 );
	int RemoveColinears();
	int Finalize( CBspBrush* InOwner, int NoError );
	int Faces(const CBspPoly &Test) const;
	f32 Area();
	bool DoesLineIntersect( vector3df Start, vector3df End, vector3df* Intersect = NULL );
	bool OnPoly( vector3df InVtx );
	bool OnPlane( vector3df InVtx );
	void InsertVertex( s32 InPos, vector3df InVtx );
	void RemoveVertex( vector3df InVtx );
	bool IsCoplanar();
	s32 Triangulate(CBspBrush* InOwner );
	s32 GetVertexIndex( vector3df& InVtx );
	vector3df GetMidPoint();

	// Serializer.
	//friend FArchive& operator<<( FArchive& Ar, CBspPoly& Poly );

	// Inlines.
	int IsBackfaced( const vector3df &Point ) const
	//{return ((Point-Base) | Normal) < 0.f;}
	{return ((Point-Base.Position) .dotProduct(Normal) ) < 0.f;}
	int IsCoplanar( const CBspPoly &Test ) const
	//{return Abs((Base - Test.Base)|Normal)<0.01f && Abs(Normal|Test.Normal)>0.9999f;}
	{return Abs((Base.Position - Test.Base.Position) .dotProduct(Normal) )<0.01f && Abs(Normal .dotProduct(Test.Normal) )>0.9999f;}

	inline bool operator==( CBspPoly Other )
	{
		if( NumVertices != Other.NumVertices )
			return 0;

		for( int x = 0 ; x < NumVertices ; x++ )
			if( Vertex[x] != Other.Vertex[x] )
				return 0;

		return 1;
	}
	inline bool operator!=( CBspPoly Other )
	{
		if( NumVertices != Other.NumVertices )
			return 1;

		for( int x = 0 ; x < NumVertices ; x++ )
			if( Vertex[x] != Other.Vertex[x] )
				return 1;

		return 0;
	}
};



class FBox
{
public:
	// Variables.
	vector3df Min;
	vector3df Max;
	BYTE IsValid;

	// Constructors.
	FBox() {}
	FBox(INT) { Init(); }
	FBox( const vector3df& InMin, const vector3df& InMax ) : Min(InMin), Max(InMax), IsValid(1) {}
	FBox( const vector3df* Points, INT Count );

	// Accessors.
	vector3df& GetExtrema( int i )
	{
		return (&Min)[i];
	}
	const vector3df& GetExtrema( int i ) const
	{
		return (&Min)[i];
	}

	// Functions.
	void Init()
	{
		Min = Max = vector3df(0,0,0);
		IsValid = 0;
	}
	//FORCEINLINE 
	__forceinline  FBox& operator+=( const vector3df &Other )
	{
		if( IsValid )
		{
#if ASM
			__asm
			{
				mov		eax,[Other]
				mov		ecx,[this]

				movss	xmm3,[eax]vector3df.X
					movss	xmm4,[eax]vector3df.Y
					movss	xmm5,[eax]vector3df.Z

					movss	xmm0,[ecx]FBox.Min.X
					movss	xmm1,[ecx]FBox.Min.Y
					movss	xmm2,[ecx]FBox.Min.Z
					minss	xmm0,xmm3
					minss	xmm1,xmm4
					minss	xmm2,xmm5
					movss	[ecx]FBox.Min.X,xmm0
					movss	[ecx]FBox.Min.Y,xmm1
					movss	[ecx]FBox.Min.Z,xmm2

					movss	xmm0,[ecx]FBox.Max.X
					movss	xmm1,[ecx]FBox.Max.Y
					movss	xmm2,[ecx]FBox.Max.Z
					maxss	xmm0,xmm3
					maxss	xmm1,xmm4
					maxss	xmm2,xmm5
					movss	[ecx]FBox.Max.X,xmm0
					movss	[ecx]FBox.Max.Y,xmm1
					movss	[ecx]FBox.Max.Z,xmm2
			}
#else
			Min.X = min_( Min.X, Other.X );
			Min.Y = min_( Min.Y, Other.Y );
			Min.Z = min_( Min.Z, Other.Z );

			Max.X = max_( Max.X, Other.X );
			Max.Y = max_( Max.Y, Other.Y );
			Max.Z = max_( Max.Z, Other.Z );
#endif
		}
		else
		{
			Min = Max = Other;
			IsValid = 1;
		}
		return *this;
	}
	FBox operator+( const vector3df& Other ) const
	{
		return FBox(*this) += Other;
	}
	FBox& operator+=( const FBox& Other )
	{
		if( IsValid && Other.IsValid )
		{
#if ASM
			__asm
			{
				mov		eax,[Other]
				mov		ecx,[this]

				movss	xmm0,[ecx]FBox.Min.X
					movss	xmm1,[ecx]FBox.Min.Y
					movss	xmm2,[ecx]FBox.Min.Z
					minss	xmm0,[eax]FBox.Min.X
					minss	xmm1,[eax]FBox.Min.Y
					minss	xmm2,[eax]FBox.Min.Z
					movss	[ecx]FBox.Min.X,xmm0
					movss	[ecx]FBox.Min.Y,xmm1
					movss	[ecx]FBox.Min.Z,xmm2

					movss	xmm0,[ecx]FBox.Max.X
					movss	xmm1,[ecx]FBox.Max.Y
					movss	xmm2,[ecx]FBox.Max.Z
					maxss	xmm0,[eax]FBox.Max.X
					maxss	xmm1,[eax]FBox.Max.Y
					maxss	xmm2,[eax]FBox.Max.Z
					movss	[ecx]FBox.Max.X,xmm0
					movss	[ecx]FBox.Max.Y,xmm1
					movss	[ecx]FBox.Max.Z,xmm2
			}
#else
			Min.X = min_( Min.X, Other.Min.X );
			Min.Y = min_( Min.Y, Other.Min.Y );
			Min.Z = min_( Min.Z, Other.Min.Z );

			Max.X = max_( Max.X, Other.Max.X );
			Max.Y = max_( Max.Y, Other.Max.Y );
			Max.Z = max_( Max.Z, Other.Max.Z );
#endif
		}
		else *this = Other;
		return *this;
	}
	FBox operator+( const FBox& Other ) const
	{
		return FBox(*this) += Other;
	}
	vector3df& operator[]( INT i )
	{
		//check(i>-1);
		//check(i<2);
		if( i == 0 )		return Min;
		else				return Max;
	}
	FBox TransformBy( const FMatrix& M ) const;
	FBox ExpandBy( FLOAT W ) const
	{
		return FBox( Min - vector3df(W,W,W), Max + vector3df(W,W,W) );
	}
	// Returns the midpoint between the min and max points.
	vector3df GetCenter() const
	{
		return vector3df( ( Min + Max ) * 0.5f );
	}
	// Returns the extent around the center
	vector3df GetExtent() const
	{
		return 0.5f*(Max - Min);
	}

	void GetCenterAndExtents( vector3df & center, vector3df & Extents ) const
	{
		Extents = GetExtent();
		center = Min + Extents;
	}

	bool Intersect( const FBox & other ) const
	{
		if( Min.X > other.Max.X || other.Min.X > Max.X )
			return false;
		if( Min.Y > other.Max.Y || other.Min.Y > Max.Y )
			return false;
		if( Min.Z > other.Max.Z || other.Min.Z > Max.Z )
			return false;
		return true;
	}
	// Checks to see if the location is inside this box
	bool IsInside( vector3df& In )
	{
		return ( In.X > Min.X && In.X < Max.X
			&& In.Y > Min.Y && In.Y < Max.Y 
			&& In.Z > Min.Z && In.Z < Max.Z );
	}


	// Serializer.

};

