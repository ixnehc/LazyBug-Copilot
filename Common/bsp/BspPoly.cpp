
#include "stdh.h"
#include "BspPoly.h"
#include "BspModel.h"

 CBspPoly::CBspPoly()
{}
//
// Initialize everything in an  editor polygon structure to defaults.
//
void CBspPoly::Init()
{
	//Base	.Position		= vector3df(0,0,0);
	Normal			= vector3df(0,0,0);
	TextureU		= vector3df(0,0,0);
	TextureV		= vector3df(0,0,0);
	PolyFlags       = 0;
	//Actor			= NULL;
	//Material        = NULL;
	//ItemName        = NAME_None;
	NumVertices     = 0;
	iLink           = INDEX_NONE;
	iBrushPoly		= INDEX_NONE;
	SmoothingMask	= 0;
	LightMapScale	= 8.0f;
}
//
// Reverse an CBspPoly by reversing the normal and reversing the order of its
// vertices.
//
void CBspPoly::Reverse()
{
	vector3df Temp;
	int i,c;

	Normal *= -1;

	c=NumVertices/2;
	for( i=0; i<c; i++ )
	{
		// Flip all points except middle if odd number of points.
		Temp      = Vertex[i].Position ;
		Vertex[i].Position  = Vertex[(NumVertices-1)-i].Position ;
		Vertex[(NumVertices-1)-i].Position = Temp;
	}
}

//
// Fix up an editor poly by deleting vertices that are identical.  Sets
// vertex count to zero if it collapses.  Returns number of vertices, 0 or >=3.
//
int CBspPoly::Fix()
{
	int i,j,prev;

	j=0; prev=NumVertices-1;
	for( i=0; i<NumVertices; i++ )
	{
		if( !FPointsAreSame( Vertex[i].Position , Vertex[prev].Position  ) )
		{
			if( j != i )
				Vertex[j] = Vertex[i];
			prev = j;
			j    ++;
		}
		//else debugf( NAME_Warning, TEXT("CBspPoly::Fix: Collapsed a point") );
	}
	if (j>=3) NumVertices = j;
	else      NumVertices = 0;
	return NumVertices;
}

//
// Compute the 2D area.
//
f32 CBspPoly::Area()
{
	vector3df Side1,Side2;
	f32 Area;
	int i;

	Area  = 0.f;
	Side1 = Vertex[1].Position  - Vertex[0].Position ;
	for( i=2; i<NumVertices; i++ )
	{
		Side2 = Vertex[i].Position  - Vertex[0].Position ;
		Area += Side1.crossProduct(Side2).Size();
		//Area += (Side1 ^ Side2).Size();
		Side1 = Side2;
	}
	return Area;
}

//
// Split with plane. Meant to be numerically stable.
//
int CBspPoly::SplitWithPlane
(
 const vector3df	&PlaneBase,
 const vector3df	&PlaneNormal,
 CBspPoly			*FrontPoly,
 CBspPoly			*BackPoly,
 int				VeryPrecise
 ) const
{
	vector3df 	Intersection;
	f32   	Dist=0,MaxDist=0,MinDist=0;
	f32		PrevDist,Thresh;
	enum 	  	{V_FRONT,V_BACK,V_EITHER} Status,PrevStatus=V_EITHER;
	int     	i,j;

	if (VeryPrecise)	Thresh = THRESH_SPLIT_POLY_PRECISELY;	
	else				Thresh = THRESH_SPLIT_POLY_WITH_PLANE;

	// Find number of vertices.
	//check(NumVertices>=3);
	//check(NumVertices<=MAX_VERTICES);

	// See if the polygon is split by SplitPoly, or it's on either side, or the
	// polys are coplanar.  Go through all of the polygon points and
	// calculate the minimum and maximum signed distance (in the direction
	// of the normal) from each point to the plane of SplitPoly.
	for( i=0; i<NumVertices; i++ )
	{
		Dist = FPointPlaneDist( Vertex[i].Position , PlaneBase, PlaneNormal );

		if( i==0 || Dist>MaxDist ) MaxDist=Dist;
		if( i==0 || Dist<MinDist ) MinDist=Dist;

		if      (Dist > +Thresh) PrevStatus = V_FRONT;
		else if (Dist < -Thresh) PrevStatus = V_BACK;
	}
	if( MaxDist<Thresh && MinDist>-Thresh )
	{
		return SP_Coplanar;
	}
	else if( MaxDist < Thresh )
	{
		return SP_Back;
	}
	else if( MinDist > -Thresh )
	{
		return SP_Front;
	}
	else//<------------------切分MaxDist>Thresh && MinDist<-Thresh )
	{
		// Split.
		if( FrontPoly==NULL )
			return SP_Split; // Caller only wanted status.
		//if( NumVertices > MAX_VERTICES )
		//	appErrorf( TEXT("%s"), TEXT("CBspPoly::SplitWithPlane: Vertex overflow") );

		*FrontPoly = *this; // Copy all info.
		FrontPoly->PolyFlags |= PF_EdCut; // Mark as cut.
		FrontPoly->NumVertices =  0;

		*BackPoly = *this; // Copy all info.
		BackPoly->PolyFlags |= PF_EdCut; // Mark as cut.
		BackPoly->NumVertices = 0;

		j = NumVertices-1; // Previous vertex; have PrevStatus already.

		for( i=0; i<NumVertices; i++ )
		{
			PrevDist	= Dist;
			Dist		= FPointPlaneDist( Vertex[i].Position, PlaneBase, PlaneNormal );

			if      (Dist > +Thresh)  	Status = V_FRONT;
			else if (Dist < -Thresh)  	Status = V_BACK;
			else						Status = PrevStatus;//在误差之内，所以指定前后皆可。

			if( Status != PrevStatus )
			{
				// Crossing.  Either Front-to-Back or Back-To-Front.
				// Intersection point is naturally on both front and back polys.
				if( (Dist >= -Thresh) && (Dist < +Thresh) )//该点接近切分面。
				{
					// This point lies on plane.
					if( PrevStatus == V_FRONT )
					{
						FrontPoly->Vertex[FrontPoly->NumVertices++] = Vertex[i];
						BackPoly ->Vertex[BackPoly ->NumVertices++] = Vertex[i];
					}
					else
					{
						BackPoly ->Vertex[BackPoly ->NumVertices++] = Vertex[i];
						FrontPoly->Vertex[FrontPoly->NumVertices++] = Vertex[i];
					}
				}
				else if( (PrevDist >= -Thresh) && (PrevDist < +Thresh) )//上一点接近切分面。
				{
					// Previous point lies on plane.
					if (Status == V_FRONT)
					{
						FrontPoly->Vertex[FrontPoly->NumVertices++] = Vertex[j];
						FrontPoly->Vertex[FrontPoly->NumVertices++] = Vertex[i];
					}
					else
					{
						BackPoly ->Vertex[BackPoly ->NumVertices++] = Vertex[j];
						BackPoly ->Vertex[BackPoly ->NumVertices++] = Vertex[i];
					}
				}
				else//该点与上一点都不在切分面上。
				{
					FModelVertex tempVertex;			
					// Intersection point is in between.
					Intersection = FLinePlaneIntersection(Vertex[j].Position ,Vertex[i].Position ,PlaneBase,PlaneNormal);
					tempVertex.Position=Intersection;
					//对 uv  normal  插值。
					f64 longDistance =Vertex[i].Position.getDistanceFrom(Vertex[j].Position ) ;
					f64 shortDistance =Vertex[i].Position.getDistanceFrom(Intersection) ;	
					f64  temp=shortDistance/longDistance;				
					tempVertex.Normal= Vertex[i]. Normal+(Vertex[j]. Normal-Vertex[j]. Normal)*temp;		
					tempVertex.TexNum=Vertex[i].TexNum;
					assert(Vertex[i].TexNum==Vertex[j].TexNum);
					for(int q=0;q< Vertex[i].TexNum;q++)
					{
						tempVertex.TexCoordx[q]=Vertex[i]. TexCoordx[q]+(Vertex[j]. TexCoordx[q]-Vertex[j]. TexCoordx[q])*temp;
						tempVertex.TexCoordy[q]=Vertex[i]. TexCoordy[q]+(Vertex[j]. TexCoordy[q]-Vertex[j]. TexCoordy[q])*temp;
					}
		
					if( PrevStatus == V_FRONT )
					{
						//FrontPoly->Vertex[FrontPoly->NumVertices++].Position  = Intersection;
						//BackPoly ->Vertex[BackPoly ->NumVertices++].Position  = Intersection;
						//BackPoly ->Vertex[BackPoly ->NumVertices++].Position 	= Vertex[i].Position ;
						FrontPoly->Vertex[FrontPoly->NumVertices++] = tempVertex;
						BackPoly ->Vertex[BackPoly ->NumVertices++] = tempVertex;
						BackPoly ->Vertex[BackPoly ->NumVertices++] 	= Vertex[i] ;
					}
					else
					{
						//BackPoly ->Vertex[BackPoly ->NumVertices++].Position  = Intersection;
						//FrontPoly->Vertex[FrontPoly->NumVertices++].Position  = Intersection;
						//FrontPoly->Vertex[FrontPoly->NumVertices++].Position 	= Vertex[i].Position ;
						BackPoly ->Vertex[BackPoly ->NumVertices++] = tempVertex;
						FrontPoly->Vertex[FrontPoly->NumVertices++] = tempVertex;
						FrontPoly->Vertex[FrontPoly->NumVertices++] 	= Vertex[i];
					}
				}
			}
			else
			{
				if (Status==V_FRONT) FrontPoly->Vertex[FrontPoly->NumVertices++] = Vertex[i];
				else                 BackPoly ->Vertex[BackPoly ->NumVertices++] = Vertex[i];//<-----------需确认是否类copy
			}
			j  = i;
			PrevStatus = Status;
		}

		// Handle possibility of sliver polys due to precision errors.
		if( FrontPoly->Fix()<3 )
		{
			//debugf( NAME_Warning, TEXT("CBspPoly::SplitWithPlane: Ignored front sliver") );
			return SP_Back;
		}
		else if( BackPoly->Fix()<3 )
		{
			//debugf( NAME_Warning, TEXT("CBspPoly::SplitWithPlane: Ignored back sliver") );
			return SP_Front;
		}
		else return SP_Split;
	}
}

//
// Split with a Bsp node.
//
int CBspPoly::SplitWithNode
(
 const CBspModel	*Model,
 s32				iNode,
 CBspPoly			*FrontPoly,
 CBspPoly			*BackPoly,
 s32				VeryPrecise
 ) const
{
	const FBspNode * Node = Model->Nodes[iNode];
	const FBspSurf * Surf = Model->Surfs[Node->iSurf ];

	return SplitWithPlane
		(
		Model->Points [Model->Verts[Node->iVertPool].pVertex].Position,
		Model->Vectors[Surf->vNormal],
		FrontPoly, 
		BackPoly, 
		VeryPrecise
		);
}

//
// Split with plane quickly for in-game geometry operations.
// Results are always valid. May return sliver polys.
//
int CBspPoly::SplitWithPlaneFast
(
 const plane3df	Plane,
 CBspPoly*			FrontPoly,
 CBspPoly*			BackPoly
 ) const
{
	enum {V_FRONT=0,V_BACK=1} Status,PrevStatus,VertStatus[MAX_VERTICES],*StatusPtr;
	int Front=0,Back=0;

	StatusPtr = &VertStatus[0];
	for( int i=0; i<NumVertices; i++ )
	{
		f32 Dist = Plane.getDistanceTo(Vertex[i].Position );

		//f32 Dist = Plane.PlaneDot(Vertex[i]);
		if( Dist >= 0.f )
		{
			*StatusPtr++ = V_FRONT;
			if( Dist > +THRESH_SPLIT_POLY_WITH_PLANE )
				Front=1;
		}
		else
		{
			*StatusPtr++ = V_BACK;
			if( Dist < -THRESH_SPLIT_POLY_WITH_PLANE )
				Back=1;
		}
	}
	if( !Front )
	{
		if( Back ) return SP_Back;
		else       return SP_Coplanar;
	}
	if( !Back )
	{
		return SP_Front;
	}
	else
	{
		// Split.
		if( FrontPoly )
		{
			const vector3df *V  = &Vertex[0].Position ;
			const vector3df *W  = &Vertex[NumVertices-1].Position ;
			vector3df *V1       = &(FrontPoly->Vertex [0].Position);
			vector3df *V2       = &(BackPoly ->Vertex [0].Position);
			PrevStatus        = VertStatus         [NumVertices-1];
			StatusPtr         = &VertStatus        [0];

			int N1=0, N2=0;
			for( int i=0; i<NumVertices; i++ )
			{
				Status = *StatusPtr++;
				if( Status != PrevStatus )
				{
					// Crossing.
					*V1++ = *V2++ = FLinePlaneIntersection( *W, *V, Plane );
					if( PrevStatus == V_FRONT )	{*V2++ = *V; N1++; N2+=2;}
					else {*V1++ = *V; N2++; N1+=2;};
				}
				else if( Status==V_FRONT ) {*V1++ = *V; N1++;}
				else {*V2++ = *V; N2++;};

				PrevStatus = Status;
				W          = V++;
			}
			FrontPoly->NumVertices	= N1;
			FrontPoly->Base			= Base;
			FrontPoly->Normal		= Normal;
			FrontPoly->PolyFlags	= PolyFlags;

			BackPoly->NumVertices	= N2;
			BackPoly->Base			= Base;
			BackPoly->Normal		= Normal;
			BackPoly->PolyFlags		= PolyFlags;
		}
		return SP_Split;
	}
}

//
// Split an CBspPoly in half.
//
void CBspPoly::SplitInHalf( CBspPoly *OtherHalf )
{
	int m = NumVertices/2;
	int i;

	//if( (NumVertices<=3) || (NumVertices>MAX_VERTICES) )
	//	appErrorf( TEXT("CBspPoly::SplitInHalf: %i Vertices"), NumVertices );

	*OtherHalf = *this;

	OtherHalf->NumVertices = (NumVertices-m) + 1;
	NumVertices            = (m            ) + 1;

	for( i=0; i<(OtherHalf->NumVertices-1); i++ )
	{
		OtherHalf->Vertex[i] = Vertex[i+m];
	}
	OtherHalf->Vertex[OtherHalf->NumVertices-1] = Vertex[0];

	PolyFlags            |= PF_EdCut;
	OtherHalf->PolyFlags |= PF_EdCut;

}

//
// Compute normal of an CBspPoly.  Works even if CBspPoly has 180-degree-angled sides (which
// are often created during T-joint elimination).  Returns nonzero result (plus sets
// normal vector to zero) if a problem occurs.
//
int CBspPoly::CalcNormal( bool bSilent )
{
	Normal = vector3df(0,0,0);
	for( int i=2; i<NumVertices; i++ )
		//Normal += (Vertex[i-1] - Vertex[0]) ^ (Vertex[i] - Vertex[0]);
	    //
		Normal += (Vertex[i-1] .Position- Vertex[0].Position) .crossProduct(Vertex[i].Position - Vertex[0].Position );

	if( Normal.SizeSquared() < (f32)THRESH_ZERO_NORM_SQUARED )
	{
		//if( !bSilent )
		//	debugf( NAME_Warning, TEXT("CBspPoly::CalcNormal: Zero-area polygon") );
		return 1;
	}

	Normal.normalize();
	//Normal.Normalize();
	return 0;
}

//
// Transform an editor polygon with a coordinate system, a pre-transformation
// addition, and a post-transformation addition:
//
void CBspPoly::Transform
(
 const vector3df&		PreSubtract,
 const vector3df&		PostAdd
 )
{
	vector3df 	Temp;
	int 		i;

	TextureU = TextureU;
	TextureV = TextureV;

	Base.Position = (Base.Position - PreSubtract) + PostAdd;
	for( i=0; i<NumVertices; i++ )
		Vertex[i].Position   = (Vertex[i] .Position - PreSubtract) + PostAdd;

	// Transform normal.  Since the transformation coordinate system is
	// orthogonal but not orthonormal, it has to be renormalized here.
	Normal = Normal.SafeNormal();

}

//
// Remove colinear vertices and check convexity.  Returns 1 if convex, 0 if
// nonconvex or collapsed.
//移去凹点。1已构成了凸多边型。2 没有凸点或已被合并。
s32 CBspPoly::RemoveColinears()
{
	vector3df  SidePlaneNormal[MAX_VERTICES];
	vector3df  Side;
	s32      i,j;

	for( i=0; i<NumVertices; i++ )
	{
		j=i-1; if (j<0) j=NumVertices-1;

		// Create cutting plane perpendicular to both this side and the polygon's normal.
		Side = Vertex[i].Position  - Vertex[j].Position ;
		//SidePlaneNormal[i] = Side ^ Normal;
		SidePlaneNormal[i] = Side.crossProduct(Normal);


		if( !SidePlaneNormal[i].Normalize() )
		{
			// Eliminate these nearly identical points.去除同位置点
			appMemcpy( &Vertex[i], &Vertex[i+1], (NumVertices-(i+1)) * sizeof (vector3df) );
			if (--NumVertices<3) {NumVertices = 0; return 0;}; // Collapsed.
			i--;
		}
	}
	for( i=0; i<NumVertices; i++ )
	{
		j=i+1; if (j>=NumVertices) j=0;

		if( FPointsAreNear(SidePlaneNormal[i],SidePlaneNormal[j],FLOAT_NORMAL_THRESH) )
		{
			// Eliminate colinear points.去除凹点
			appMemcpy (&Vertex[i],&Vertex[i+1],(NumVertices-(i+1)) * sizeof (vector3df));
			appMemcpy (&SidePlaneNormal[i],&SidePlaneNormal[i+1],(NumVertices-(i+1)) * sizeof (vector3df));
			if (--NumVertices<3) {NumVertices = 0; return 0;}; // Collapsed.
			i--;
		}
		else
		{
			for( j=0; j<NumVertices; j++ )
			{
				if (j != i)
				{
					switch( SplitWithPlane (Vertex[i].Position,SidePlaneNormal[i],NULL,NULL,0) )
					{
					case SP_Front: return 0; // Nonconvex + Numerical precision error
					case SP_Split: return 0; // Nonconvex
						// SP_BACK: Means it's convex
						// SP_COPLANAR: Means it's probably convex (numerical precision)
					}
				}
			}
		}
	}
	return 1; // Ok.
}

// Checks to see if the specified line intersects this poly or not.  If "Intersect" is
// a valid pointer, it is filled in with the intersection point.
bool CBspPoly::DoesLineIntersect( vector3df Start, vector3df End, vector3df* Intersect )
{
	// If the ray doesn't cross the plane, don't bother going any further.
	float DistStart, DistEnd;
	DistStart = FPointPlaneDist( Start, Vertex[0].Position, Normal );
	DistEnd = FPointPlaneDist( End, Vertex[0].Position, Normal );
	if( (DistStart < 0 && DistEnd < 0) || (DistStart > 0 && DistEnd > 0 ) )
	{
		return 0;
	}

	// Get the intersection of the line and the plane.
	vector3df Intersection = FLinePlaneIntersection(Start,End,Vertex[0].Position ,Normal);
	if( Intersect )	*Intersect = Intersection;
	if( Intersection == Start || Intersection == End )
	{
		return 0;
	}

	// Check if the intersection point is actually on the poly.
	return OnPoly( Intersection );
}
//
// Checks to see if the specified vertex is on this poly.  Assumes the vertex is on the same
// plane as the poly and that the poly is convex.
//
// This can be combined with FLinePlaneIntersection to perform a line-fpoly intersection test.
//
bool CBspPoly::OnPoly( vector3df InVtx )
{
	vector3df  SidePlaneNormal;
	vector3df  Side;

	for( s32 x = 0 ; x < NumVertices ; x++ )
	{
		// Create plane perpendicular to both this side and the polygon's normal.
		Side = Vertex[x].Position  - Vertex[(x-1 < 0 ) ? NumVertices-1 : x-1 ].Position ;
		//SidePlaneNormal = Side ^ Normal;
		SidePlaneNormal = Side .crossProduct(Normal);
		SidePlaneNormal.normalize();

		// If point is not behind all the planes created by this polys edges, it's outside the poly.
		if( FPointPlaneDist( InVtx, Vertex[x].Position, SidePlaneNormal ) > THRESH_POINT_ON_PLANE )
			return 0;
	}

	return 1;
}

// Inserts a vertex into the poly at a specific position.
//
void CBspPoly::InsertVertex( s32 InPos, vector3df InVtx )
{
	//check( InPos <= NumVertices );

	std::vector<vector3df> NewVerts;

	// Copy the existing vertices to a temp array
	for( s32 x = 0 ; x < NumVertices ; x++ )
	{
		if(x!=InPos)
			NewVerts.push_back(Vertex[x].Position );
		else
		{
			NewVerts.push_back(InVtx);
			x--;
		}
	}
	// Insert the new vertex
	/*NewVerts.Insert( InPos );
	NewVerts( InPos ) = InVtx;*/

	// Copy the temp array into the real vertex list and clean up
	for( s32 x = 0 ; x < NewVerts.size(); x++ )
		Vertex[x].Position  = NewVerts[x];

	NumVertices++;
}

// Removes a vertex from the polygons list of vertices
//
void CBspPoly::RemoveVertex( vector3df InVtx )
{
	// Copy all vertices that are not equal to InVtx into a temp array.

	std::vector<vector3df> NewVerts;

	for( s32 x = 0 ; x < NumVertices ; ++x )
	{
		if( Vertex[x].Position  != InVtx )
		{
			NewVerts.push_back(Vertex[x].Position );
		}
	}

	// Overwrite the vertex list information with the contents
	// of the temp array.

	NumVertices = NewVerts.size();

	for( s32 x = 0 ; x < NewVerts.size(); ++x )
	{
		Vertex[x].Position  = NewVerts[x];
	}
}

/**
* Checks to see if all the vertices on a polygon are coplanar.
*/

bool CBspPoly::IsCoplanar()
{
	// 3 or fewer vertices is automatically coplanar

	if( NumVertices < 3 )
	{
		return 1;
	}

	CalcNormal(1);

	for( s32 x = 0 ; x < NumVertices ; ++x )
	{
		if( !OnPlane( Vertex[x].Position  ) )
		{
			return 0;
		}
	}

	// If we got this far, the poly has to be coplanar.

	return 1;
}

/**
* Breaks up this polygon into separate triangles.
*
* NOTE: Assumes that the polygon is convex and breaks it up
* into a triangle strip configuration (in layout only - not polygon winding).
*
* NOTE: It is up to the caller to make sure this original
* polygon is removed from the brush afterwards!
*
* @param	InOwner		The CBspBrush we want to add the new triangles into
*
* @return	The number of triangles created
*/

s32 CBspPoly::Triangulate( CBspBrush* InOwner )
{
	if( NumVertices < 3 )
	{
		return 0;
	}

	CBspPoly* NewTriangle;
	s32 Count = 0;
	bool bFlip = 0;
	s32 Mid = NumVertices/2;
	vector3df v0, v1, v2;
	s32 Front,Back;

	v0 = Vertex[0].Position;
	v1 = Vertex[1].Position;
	Front = 2;
	Back = NumVertices-1;
	for( s32 x = 2 ; x < NumVertices ; ++x )
	{
		if( bFlip )
		{
			v2 = Vertex[ Back ].Position ;
		}
		else
		{
			v2 = Vertex[ Front ].Position ;
		}

		//NewTriangle = new( InOwner->Polys->Element )CBspPoly();//
		NewTriangle = new CBspPoly;//
		InOwner->Polys->Element.push_back(NewTriangle);


		NewTriangle->Init();
		NewTriangle->Vertex[0].Position  = v0;
		NewTriangle->Vertex[1] .Position = v1;
		NewTriangle->Vertex[2] .Position = v2;
		NewTriangle->NumVertices = 3;

		NewTriangle->CalcNormal();
		NewTriangle->TextureU = TextureU;
		NewTriangle->TextureV = TextureV;
		NewTriangle->Base = Base;
		NewTriangle->PolyFlags = PolyFlags;
		//NewTriangle->Material = Material;
		NewTriangle->PolyFlags &= ~PF_GeomMarked;

		Count++;

		if( bFlip )
		{
			v0 = v2;
			Back--;
		}
		else
		{
			v1 = v2;
			Front++;
		}

		bFlip = !bFlip;
	}

	//debugf( TEXT("CBspPoly::Triangulate : Created %d triangles"), Count );
	return Count;
}

/**
* Finds the index of the specific vertex.
*
* @param	InVtx	The vertex to find the index of
*
* @return	The index of the vertex, if found.  Otherwise INDEX_NONE.
*/

s32 CBspPoly::GetVertexIndex( vector3df& InVtx )
{
	s32 idx = INDEX_NONE;

	for( s32 v = 0 ; v < NumVertices ; ++v )
	{
		if( Vertex[v].Position  == InVtx )
		{
			idx = v;
			break;
		}
	}

	return idx;
}

/**
* Computes the mid point of the polygon (in local space).
*/

vector3df CBspPoly::GetMidPoint()
{
	vector3df mid(0,0,0);

	for( s32 v = 0 ; v < NumVertices ; ++v )
	{
		mid += Vertex[v].Position ;
	}

	return mid / NumVertices;
}

// Checks to see if the specified vertex lies on this polygons plane.
//
bool CBspPoly::OnPlane( vector3df InVtx )
{
	return ( ( FPointPlaneDist( InVtx, Vertex[0].Position, Normal ) > -THRESH_POINT_ON_PLANE )
		&& ( FPointPlaneDist( InVtx, Vertex[0].Position, Normal ) < THRESH_POINT_ON_PLANE ) );
}

//
// Split a poly and keep only the front half. Returns number of vertices,
// 0 if clipped away.
//
int CBspPoly::Split( const vector3df &Normal, const vector3df &Base, int NoOverflow )
{
	if( NoOverflow && NumVertices>=CBspPoly::VERTEX_THRESHOLD )
	{
		// Don't split it, just reject it.
		if( SplitWithPlaneFast( plane3df(Base,Normal), NULL, NULL )==SP_Back )
			return 0;
		else
			return NumVertices;
	}
	else
	{
		// Split it.
		CBspPoly Front, Back;
		Front.Init();
		Back.Init();
		switch( SplitWithPlaneFast( plane3df(Base,Normal), &Front, &Back ))
		{
		case SP_Back:
			return 0;
		case SP_Split:
			*this = Front;
			return NumVertices;
		default:
			return NumVertices;
		}
	}
}

//
// Compute all remaining polygon parameters (normal, etc) that are blank.
// Returns 0 if ok, nonzero if problem.
//
int CBspPoly::Finalize( CBspBrush* InOwner, int NoError )
{
	// Check for problems.
	Fix();
	if( NumVertices<3 )
	{
		// Since we don't have enough vertices, remove this polygon from the brush

		for( s32 p = 0 ; p < InOwner->Polys->Element.size() ; ++p )
		{
			if( *(InOwner->Polys->Element[p]) == *this )
			{
				//SAFE_DELETE
				InOwner->Polys->Element.erase(InOwner->Polys->Element.begin()+p);
				break;
			}
		}

		//debugf( NAME_Warning, TEXT("CBspPoly::Finalize: Not enough vertices (%i)"), NumVertices );
		if( NoError )
			return -1;
		else
		{
			//debugf( TEXT("CBspPoly::Finalize: Not enough vertices (%i) : polygon removed from brush"), NumVertices );
			return -2;
		}
	}

	// If no normal, compute from cross-product and normalize it.
	if( Normal.IsZero() && NumVertices>=3 )
	{
		if( CalcNormal() )
		{
			//debugf( NAME_Warning, TEXT("CBspPoly::Finalize: Normalization failed, verts=%i, size=%f"), NumVertices, Normal.Size() );
			if( NoError )
				return -1;
			//else
			//	appErrorf( TEXT("CBspPoly::Finalize: Normalization failed, verts=%i, size=%f"), NumVertices, Normal.Size() );
		}
	}

	// If texture U and V coordinates weren't specified, generate them.
	if( TextureU.IsZero() && TextureV.IsZero() )
	{
		for( int i=1; i<NumVertices; i++ )
		{
			//TextureU = ((Vertex[0] - Vertex[i]) ^ Normal).SafeNormal();
			//TextureV = (Normal ^ TextureU).SafeNormal();
			TextureU = ((Vertex[0].Position  - Vertex[i].Position) .crossProduct(Normal)).SafeNormal();
			TextureV = (Normal.crossProduct(TextureU)).SafeNormal();
			if( TextureU.SizeSquared()!=0 && TextureV.SizeSquared()!=0 )
				break;
		}
	}
	return 0;
}

/*---------------------------------------------------------------------------------------
FPolys object implementation.
---------------------------------------------------------------------------------------*/

//IMPLEMENT_CLASS(UPolys);

/*---------------------------------------------------------------------------------------
Backfacing.
---------------------------------------------------------------------------------------*/

//
// Return whether this poly and Test are facing each other.
// The polys are facing if they are noncoplanar, one or more of Test's points is in 
// front of this poly, and one or more of this poly's points are behind Test.
//
int CBspPoly::Faces( const CBspPoly &Test ) const
{
	// Coplanar implies not facing.
	if( IsCoplanar( Test ) )
		return 0;

	// If this poly is frontfaced relative to all of Test's points, they're not facing.
	for( int i=0; i<Test.NumVertices; i++ )
	{
		if( !IsBackfaced( Test.Vertex[i].Position ) )
		{
			// If Test is frontfaced relative to on or more of this poly's points, they're facing.
			for( i=0; i<NumVertices; i++ )
				if( Test.IsBackfaced( Vertex[i].Position ) )
					return 1;
			return 0;
		}
	}
	return 0;
}

//
//	FBox::TransformBy
//

inline FBox FBox::TransformBy(const FMatrix& M) const
{
	FBox	NewBox(0);

	for(INT X = 0;X < 2;X++)
		for(INT Y = 0;Y < 2;Y++)
			for(INT Z = 0;Z < 2;Z++)
				NewBox += M.TransformFVector(vector3df(GetExtrema(X).X,GetExtrema(Y).Y,GetExtrema(Z).Z));

	return NewBox;
}
/*---------------------------------------------------------------------------------------
The End.
---------------------------------------------------------------------------------------*/
