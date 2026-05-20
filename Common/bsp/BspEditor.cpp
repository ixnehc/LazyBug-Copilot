#include "stdh.h"
#include "BspEditor.h"
/*-----------------------------------------------------------------------------
Size of the world.
-----------------------------------------------------------------------------*/
class CBspEditor * GEditor;
void TagReferencedNodes( CBspModel *Model, INT *NodeRef, INT *PolyRef, INT iNode );

void CleanupNodes( CBspModel *Model, INT iNode, INT iParent );

s32 AddThing( std::vector< FModelVertex>& Vectors, vector3df& V, f32 Thresh, int Check );

void SubtractBrushFromWorldFunc( CBspModel* Model, s32 iNode, CBspPoly* EdPoly,	EPolyNodeFilter Filter, ENodePlace ENodePlace );

void AddBrushToWorldFunc( CBspModel* Model, s32 iNode, CBspPoly* EdPoly, EPolyNodeFilter Filter, ENodePlace ENodePlace );

FZoneSet BuildZoneMasks( CBspModel* Model, s32 iNode );

CBspPoly BuildInfiniteFPoly( CBspModel* Model, s32 iNode );

void SplitPartitioner
(
 CBspModel*	Model,
 CBspPoly**	PolyList,
 CBspPoly**	FrontList,
 CBspPoly**	BackList,
 INT		n,
 INT		nPolys,
 INT&	nFront, 
 INT&	nBack, 
 CBspPoly	InfiniteEdPoly
 );

void UpdateBoundWithPolys( FBox& Bound, CBspPoly** PolyList, INT nPolys );

void UpdateConvolutionWithPolys( CBspModel *Model, INT iNode, CBspPoly **PolyList, int nPolys );

void FilterBound
(
 CBspModel*			Model,
 FBox*			ParentBound,
 INT				iNode,
 CBspPoly**			PolyList,
 INT				nPolys,
 INT				Outside
 );
void TagReferencedNodes( CBspModel *Model, INT *NodeRef, INT *PolyRef, INT iNode )
{
	FBspNode * Node = Model->Nodes[iNode];

	NodeRef[iNode     ] = 0;
	PolyRef[Node->iSurf] = 0;

	if( Node->iFront != INDEX_NONE ) TagReferencedNodes(Model,NodeRef,PolyRef,Node->iFront);
	if( Node->iBack  != INDEX_NONE ) TagReferencedNodes(Model,NodeRef,PolyRef,Node->iBack );
	if( Node->iPlane != INDEX_NONE ) TagReferencedNodes(Model,NodeRef,PolyRef,Node->iPlane);
}
void CleanupNodes( CBspModel *Model, INT iNode, INT iParent )
{
	FBspNode *Node = Model->Nodes[iNode];

	// Transactionally empty vertices of tag-for-empty nodes.
	Node->NodeFlags &= ~(NF_IsNew | NF_IsFront | NF_IsBack);

	// Recursively clean up front, back, and plane nodes.
	if( Node->iFront != INDEX_NONE ) CleanupNodes( Model, Node->iFront, iNode );
	if( Node->iBack  != INDEX_NONE ) CleanupNodes( Model, Node->iBack , iNode );
	if( Node->iPlane != INDEX_NONE ) CleanupNodes( Model, Node->iPlane, iNode );

	// Reload Node since the recusive call aliases it.
	Node = Model->Nodes[iNode];

	// If this is an empty node with a coplanar, replace it with the coplanar.
	if( Node->NumVertices==0 && Node->iPlane!=INDEX_NONE )
	{
		//Model->Nodes.ModifyItem( Node->iPlane );<-------------------为undo 准备
		FBspNode* PlaneNode =  Model->Nodes[Node->iPlane] ;

		// Stick our front, back, and parent nodes on the coplanar.
		if(  (Node->Plane).dotProduct( PlaneNode->Plane.Normal) >= 0.0 )
		{
			PlaneNode->iFront  = Node->iFront;
			PlaneNode->iBack   = Node->iBack;
		}
		else
		{
			PlaneNode->iFront  = Node->iBack;
			PlaneNode->iBack   = Node->iFront;
		}

		if( iParent == INDEX_NONE )
		{
			// This node is the root.
			//Model->Nodes.ModifyItem( iNode );
			//*Node                  = *PlaneNode;   // Replace root.
			Node                  = PlaneNode;   // Replace root.
			PlaneNode->NumVertices = 0;            // Mark as unused.
		}
		else
		{
			// This is a chil node.
			//Model->Nodes.ModifyItem( iParent );
			FBspNode *ParentNode =  Model->Nodes[iParent];

			if      ( ParentNode->iFront == iNode ) ParentNode->iFront = Node->iPlane;
			else if ( ParentNode->iBack  == iNode ) ParentNode->iBack  = Node->iPlane;
			else if ( ParentNode->iPlane == iNode ) ParentNode->iPlane = Node->iPlane;
			else          NULL;
			//appErrorf( TEXT("CleanupNodes: Parent and child are unlinked") );
		}
	}
	else if( Node->NumVertices == 0 && ( Node->iFront==INDEX_NONE || Node->iBack==INDEX_NONE ) )
	{
		// Delete empty nodes with no fronts or backs.
		// Replace empty nodes with only fronts.
		// Replace empty nodes with only backs.
		INT iReplacementNode;
		if     ( Node->iFront != INDEX_NONE ) iReplacementNode = Node->iFront;
		else if( Node->iBack  != INDEX_NONE ) iReplacementNode = Node->iBack;
		else                                  iReplacementNode = INDEX_NONE;

		if( iParent == INDEX_NONE )
		{
			// Root.
			if( iReplacementNode == INDEX_NONE )
			{
				Model->Nodes.clear();
			}
			else
			{
				//Model->Nodes.ModifyItem( iNode );
				Node = Model->Nodes[iReplacementNode];
			}
		}
		else
		{
			// Regular node.
			FBspNode *ParentNode = Model->Nodes[iParent];
			//Model->Nodes.ModifyItem( iParent );

			if     ( ParentNode->iFront == iNode ) ParentNode->iFront = iReplacementNode;
			else if( ParentNode->iBack  == iNode ) ParentNode->iBack  = iReplacementNode;
			else if( ParentNode->iPlane == iNode ) ParentNode->iPlane = iReplacementNode;
			else NULL;
			//appErrorf( TEXT("CleanupNodes: Parent and child are unlinked") );
		}
	}
}
//s32 AddThing( std::vector< FModelVertex>& Vectors, vector3df& V, f32 Thresh, int Check )
s32 AddThing( std::vector< FModelVertex>& Vectors, FModelVertex& V, f32 Thresh, int Check )
{
	if( Check )
	{
		// See if this is very close to an existing point/vector.		
		for( s32 i=0; i<Vectors.size(); i++ )
		{
			const vector3df &TableVect = Vectors[i].Position;
			f32 Temp=(V.Position.X - TableVect.X);
			if( (Temp > -Thresh) && (Temp < Thresh) )
			{
				Temp=(V.Position.Y - TableVect.Y);
				if( (Temp > -Thresh) && (Temp < Thresh) )
				{
					Temp=(V.Position.Z - TableVect.Z);
					if( (Temp > -Thresh) && (Temp < Thresh) )
					{
						// Found nearly-matching vector.
						return i;
					}
				}
			}
		}
	}
	//FModelVertex temp=V;

	Vectors.push_back( V );

	return Vectors.size()-1;
}

s32 AddThing1( std::vector< vector3df>& Vectors, vector3df& V, f32 Thresh, int Check )
{
	if( Check )
	{
		// See if this is very close to an existing point/vector.		
		for( s32 i=0; i<Vectors.size(); i++ )
		{
			const vector3df &TableVect = Vectors[i];
			f32 Temp=(V.X - TableVect.X);
			if( (Temp > -Thresh) && (Temp < Thresh) )
			{
				Temp=(V.Y - TableVect.Y);
				if( (Temp > -Thresh) && (Temp < Thresh) )
				{
					Temp=(V.Z - TableVect.Z);
					if( (Temp > -Thresh) && (Temp < Thresh) )
					{
						// Found nearly-matching vector.
						return i;
					}
				}
			}
		}
	}
	vector3df temp;
	temp=V;
	Vectors.push_back( temp );

	return Vectors.size()-1;
}
void SubtractBrushFromWorldFunc( CBspModel* Model, s32 iNode, CBspPoly* EdPoly,	EPolyNodeFilter Filter, ENodePlace ENodePlace )
{
	switch (Filter)
	{
	case F_OUTSIDE:
	case F_COSPATIAL_FACING_OUT:
	case F_COSPATIAL_FACING_IN:
	case F_COPLANAR_OUTSIDE:
		break;
	case F_COPLANAR_INSIDE:
	case F_INSIDE:
		EdPoly->Reverse();
		GEditor->bspAddNode (Model,iNode,ENodePlace,NF_IsNew,EdPoly); // Add to Bsp back
		EdPoly->Reverse();
		break;
	}
}

void AddBrushToWorldFunc( CBspModel* Model, s32 iNode, CBspPoly* EdPoly, EPolyNodeFilter Filter, ENodePlace ENodePlace )
{
	switch( Filter )
	{
	case F_OUTSIDE:
	case F_COPLANAR_OUTSIDE:
		GEditor->bspAddNode (Model,iNode,ENodePlace,NF_IsNew,EdPoly);
		break;
	case F_COSPATIAL_FACING_OUT:
		if( !(EdPoly->PolyFlags & PF_Semisolid) )
			GEditor->bspAddNode (Model,iNode,ENodePlace,NF_IsNew,EdPoly);
		break;
	case F_INSIDE:
	case F_COPLANAR_INSIDE:
	case F_COSPATIAL_FACING_IN:
		break;
	}
}
FZoneSet BuildZoneMasks( CBspModel* Model, s32 iNode )
{
	FBspNode *Node = Model->Nodes[iNode];
	FZoneSet ZoneMask = FZoneSet::NoZones();

	if( Node->iZone[0]!=0 ) ZoneMask.AddZone(Node->iZone[0]);
	if( Node->iZone[1]!=0 ) ZoneMask.AddZone(Node->iZone[1]);

	if( Node->iFront != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node->iFront );
	if( Node->iBack  != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node->iBack );
	if( Node->iPlane != INDEX_NONE )	ZoneMask |= BuildZoneMasks( Model, Node->iPlane );

	Node->ZoneMask = ZoneMask;

	return ZoneMask;
}
CBspPoly BuildInfiniteFPoly( CBspModel* Model, s32 iNode )
{
	FBspNode *Node   = Model->Nodes [iNode ];
	FBspSurf   *Poly   = Model->Surfs  [Node->iSurf ];
	vector3df  Base    = (Poly->Plane * Poly->Plane.D).Normal;
	vector3df  Normal  = Poly->Plane.Normal;
	vector3df	 Axis1,Axis2;

	// Find two non-problematic axis vectors.
	Normal.FindBestAxisVectors( Axis1, Axis2 );

	// Set up the CBspPoly.
	CBspPoly EdPoly;
	EdPoly.Init();
	EdPoly.NumVertices = 4;
	EdPoly.Normal      = Normal;

	EdPoly.Base.Position        = Base;
	EdPoly.Base.Normal      =vector3df(0,0,0);
	EdPoly.Base.TexNum    =1;
	EdPoly.Base.TexCoordx[0]=0;
	EdPoly.Base.TexCoordy[0]=0;

	EdPoly.Vertex[0].Position    = Base + Axis1*WORLD_MAX + Axis2*WORLD_MAX;
	EdPoly.Vertex[0].Normal      =vector3df(0,0,0);
	EdPoly.Vertex[0].TexNum    =1;
	EdPoly.Vertex[0].TexCoordx[0]=0;
	EdPoly.Vertex[0].TexCoordy[0]=0;
	EdPoly.Vertex[1].Position   = Base - Axis1*WORLD_MAX + Axis2*WORLD_MAX;
	EdPoly.Vertex[1].Normal      =vector3df(0,0,0);
	EdPoly.Vertex[1].TexNum    =1;
	EdPoly.Vertex[1].TexCoordx[0]=0;
	EdPoly.Vertex[1].TexCoordy[0]=1;
	EdPoly.Vertex[2].Position    = Base - Axis1*WORLD_MAX - Axis2*WORLD_MAX;
	EdPoly.Vertex[2].Normal      =vector3df(0,0,0);
	EdPoly.Vertex[2].TexNum    =1;
	EdPoly.Vertex[2].TexCoordx[0]=0;
	EdPoly.Vertex[2].TexCoordy[0]=0;
	EdPoly.Vertex[3].Position   = Base + Axis1*WORLD_MAX - Axis2*WORLD_MAX;
	EdPoly.Vertex[3].Normal      =vector3df(0,0,0);
	EdPoly.Vertex[3].TexNum    =1;
	EdPoly.Vertex[3].TexCoordx[0]=1;
	EdPoly.Vertex[3].TexCoordy[0]=1;

	return EdPoly;
}
void SplitPartitioner
(
 CBspModel*	Model,
 CBspPoly**	PolyList,
 CBspPoly**	FrontList,
 CBspPoly**	BackList,
 INT		n,
 INT		nPolys,
 INT&	nFront, 
 INT&	nBack, 
 CBspPoly	InfiniteEdPoly
 )
{
	CBspPoly FrontPoly,BackPoly;
	while( n < nPolys )
	{
		if( InfiniteEdPoly.NumVertices >= CBspPoly::VERTEX_THRESHOLD )
		{
			CBspPoly Half;
			InfiniteEdPoly.SplitInHalf(&Half);
			SplitPartitioner(Model,PolyList,FrontList,BackList,n,nPolys,nFront,nBack,Half);
		}
		CBspPoly* Poly = PolyList[n];
		switch( InfiniteEdPoly.SplitWithPlane(Poly->Vertex[0].Position,Poly->Normal,&FrontPoly,&BackPoly,0) )
		{
		case SP_Coplanar:
			// May occasionally happen.
			//debugf( NAME_Log, TEXT("FilterBound: Got inficoplanar") );
			break;

		case SP_Front:
			// Shouldn't happen if hull is correct.
			//debugf( NAME_Log, TEXT("FilterBound: Got infifront") );
			return;

		case SP_Split:
			InfiniteEdPoly = BackPoly;
			break;

		case SP_Back:
			break;
		}
		n++;
	}

	CBspPoly* New= new CBspPoly;//<------------------不在此处释放
	Model->PolysTemp->Element.push_back(New);
	//= new(GMem)CBspPoly;
	*New = InfiniteEdPoly;
	New->Reverse();
	New->iBrushPoly |= 0x40000000;
	FrontList[nFront++] = New;

	New= new CBspPoly;//<------------------不在此处释放
	Model->PolysTemp->Element.push_back(New);
	//New = new(GMem)CBspPoly;
	*New = InfiniteEdPoly;
	BackList[nBack++] = New;
}
//
// Trys to merge two polygons.  If they can be merged, replaces Poly1 and emptys Poly2
// and returns 1.  Otherwise, returns 0.
//
int TryToMerge( CBspPoly *Poly1, CBspPoly *Poly2 )
{
	// Vertex count reasonable?
	if( Poly1->NumVertices+Poly2->NumVertices > CBspPoly::MAX_VERTICES )
		return 0;

	// Find one overlapping point.
	INT Start1=0, Start2=0;
	for( Start1=0; Start1<Poly1->NumVertices; Start1++ )
		for( Start2=0; Start2<Poly2->NumVertices; Start2++ )
			if( FPointsAreSame(Poly1->Vertex[Start1].Position ,Poly2->Vertex[Start2].Position ) &&Poly1->Vertex[Start1]==Poly2->Vertex[Start2])
				goto FoundOverlap;
	return 0;
FoundOverlap:

	// Wrap around trying to merge.
	INT End1  = Start1;
	INT End2  = Start2;
	INT Test1 = Start1+1; if (Test1>=Poly1->NumVertices) Test1 = 0;
	INT Test2 = Start2-1; if (Test2<0)                   Test2 = Poly2->NumVertices-1;
	if(
		//FPointsAreSame(Poly1->Vertex[Test1].Position ,Poly2->Vertex[Test2].Position ) 
		//&&
		Poly1->Vertex[Test1]==Poly2->Vertex[Test2])
	{
		End1   = Test1;
		Start2 = Test2;
	}
	else
	{
		Test1 = Start1-1; if (Test1<0)                   Test1=Poly1->NumVertices-1;
		Test2 = Start2+1; if (Test2>=Poly2->NumVertices) Test2=0;
		if( 
			//FPointsAreSame(Poly1->Vertex[Test1].Position ,Poly2->Vertex[Test2].Position ) 
			//&&  
			(Poly1->Vertex[Start1]==Poly2->Vertex[Start2])   )
			
		{
			Start1 = Test1;
			End2   = Test2;
		}
		else return 0;
	}

	// Build a new edpoly containing both polygons merged.
	//CBspPoly *NewPoly = new CBspPoly;

	CBspPoly NewPoly = *Poly1;//<--------------------------------有问题语句
	NewPoly.NumVertices = 0;
	INT Vertex = End1;
	for( INT i=0; i<Poly1->NumVertices; i++ )
	{
		NewPoly.Vertex[NewPoly.NumVertices++] = Poly1->Vertex[Vertex];//<----------默认=,操作符.需要重载
		if( ++Vertex >= Poly1->NumVertices )
			Vertex=0;
	}
	Vertex = End2;
	for( INT i=0; i<(Poly2->NumVertices-2); i++ )
	{
		if( ++Vertex >= Poly2->NumVertices )
			Vertex=0;
		NewPoly.Vertex[NewPoly.NumVertices++] = Poly2->Vertex[Vertex];//<----------默认=,操作符.需要重载
	}

	// Remove colinear vertices and check convexity.
	if( NewPoly.RemoveColinears() )
	{
		if( NewPoly.NumVertices <= FBspNode::MAX_NODE_VERTICES )
		{
			//SAFE_DELETE(Poly1);//<---------------需要正确释放
			*Poly1 = NewPoly;
			Poly2->NumVertices	= 0;
			//SAFE_DELETE(Poly2);
			return 1;
		}
		else return 0;
	}
	else return 0;
}

//
// Merge all polygons in coplanar list that can be merged convexly.
//
void MergeCoplanars( CBspModel* Model, INT* PolyList, INT PolyCount )
{
	INT MergeAgain = 1;
	while( MergeAgain )
	{
		MergeAgain = 0;
		for( INT i=0; i<PolyCount; i++ )
		{
			CBspPoly*Poly1 = Model->Polys->Element[PolyList[i]];
			if( Poly1->NumVertices > 0 )
			{
				for( INT j=i+1; j<PolyCount; j++ )
				{
					CBspPoly*Poly2 = Model->Polys->Element[PolyList[j]];
					if( Poly2->NumVertices > 0 )
					{
						if( TryToMerge( Poly1, Poly2 ) )
							MergeAgain=1;
					}
				}
			}
		}
	}
}
//
// Update a bounding volume by expanding it to enclose a list of polys.
//

void UpdateBoundWithPolys( FBox& Bound, CBspPoly** PolyList, INT nPolys )
{
	for( INT i=0; i<nPolys; i++ )
		for( INT j=0; j<PolyList[i]->NumVertices; j++ )
			Bound += PolyList[i]->Vertex[j].Position ;
}

//
// Update a convolution hull with a list of polys.
//

void UpdateConvolutionWithPolys( CBspModel *Model, INT iNode, CBspPoly **PolyList, int nPolys )
{
	FBox Box(0);

	FBspNode &Node = *(Model->Nodes[iNode]);
	Node.iCollisionBound = Model->LeafHulls.size();
	for( int i=0; i<nPolys; i++ )
	{
		if( PolyList[i]->iBrushPoly != INDEX_NONE )
		{
			int j;
			for( j=0; j<i; j++ )
				if( PolyList[j]->iBrushPoly == PolyList[i]->iBrushPoly )
					break;
			if( j >= i )
				Model->LeafHulls.push_back(PolyList[i]->iBrushPoly);

		}
		for( int j=0; j<PolyList[i]->NumVertices; j++ )
			Box += PolyList[i]->Vertex[j].Position ;
	}
	Model->LeafHulls.push_back(INDEX_NONE);

	// Add bounds.<---------------------------------------------------加用于碰撞检测，包裹的位置
	Model->LeafHulls.push_back( Box.Min.X );
	Model->LeafHulls.push_back( Box.Min.Y );
	Model->LeafHulls.push_back( Box.Min.Z );
	Model->LeafHulls.push_back( Box.Max.X );
	Model->LeafHulls.push_back( Box.Max.Y );
	Model->LeafHulls.push_back( Box.Max.Z );
	//Model->LeafHulls.push_back( *(INT*)&Box.Max.Z );

}
void FilterBound
(
	CBspModel*			Model,
	FBox*			ParentBound,
	INT				iNode,
	CBspPoly**			PolyList,
	INT				nPolys,
	INT				Outside
)
{
	//FMemMark Mark(GMem);
	FBspNode&	Node	= *(Model->Nodes[iNode]);
	FBspSurf&	Surf	= *(Model->Surfs  [Node.iSurf]);
	vector3df		Base = (Surf.Plane * Surf.Plane.D).Normal;
	vector3df&	Normal	= Model->Vectors[Surf.vNormal];
	FBox		Bound(0);

	Bound.Min.X = Bound.Min.Y = Bound.Min.Z = +WORLD_MAX;
	Bound.Max.X = Bound.Max.Y = Bound.Max.Z = -WORLD_MAX;

	// Split bound into front half and back half.
	CBspPoly** FrontList =new CBspPoly* [nPolys*2+16]; 
	memset(FrontList,0x00,sizeof(CBspPoly*)*(nPolys*2+16));
	//= new(GMem,nPolys*2+16)CBspPoly*;
	int nFront=0;

	CBspPoly** BackList =new CBspPoly* [nPolys*2+16]; 
	memset(BackList,0x00,sizeof(CBspPoly*)*(nPolys*2+16));
	//= new(GMem,nPolys*2+16)CBspPoly*;
	int nBack=0;

	CBspPoly* FrontPoly=new CBspPoly;
	Model->PolysTemp->Element.push_back(FrontPoly);
	//= new(GMem)CBspPoly;
	CBspPoly* BackPoly=new CBspPoly;
	Model->PolysTemp->Element.push_back(BackPoly);
	//= new(GMem)CBspPoly;

	for( INT i=0; i<nPolys; i++ )
	{
		CBspPoly *Poly = PolyList[i];
		switch( Poly->SplitWithPlane( Base, Normal, FrontPoly, BackPoly, 0 ) )
		{
		case SP_Coplanar:
			//debugf( NAME_Log, TEXT("FilterBound: Got coplanar") );
			FrontList[nFront++] = Poly;
			BackList[nBack++] = Poly;
			break;

		case SP_Front:
			FrontList[nFront++] = Poly;
			break;

		case SP_Back:
			BackList[nBack++] = Poly;
			break;

		case SP_Split:
			if( FrontPoly->NumVertices >= CBspPoly::VERTEX_THRESHOLD )
			{
				CBspPoly *Half =new CBspPoly;
				Model->PolysTemp->Element.push_back(Half);
				//= new(GMem)CBspPoly;
				FrontPoly->SplitInHalf(Half);
				FrontList[nFront++] = Half;//<-----------------------在FrontList中释放
			}
			FrontList[nFront++] = FrontPoly;

			if( BackPoly->NumVertices >= CBspPoly::VERTEX_THRESHOLD )
			{
				CBspPoly *Half=new CBspPoly;
				Model->PolysTemp->Element.push_back(Half);
				//= new(GMem)CBspPoly;
				BackPoly->SplitInHalf(Half);;//<-----------------------在BackList中释放
				BackList[nBack++] = Half;
			}
			BackList [nBack++] = BackPoly;

			//FrontPoly = new(GMem)CBspPoly;
			//BackPoly  = new(GMem)CBspPoly;
			FrontPoly = new CBspPoly;
			BackPoly  = new CBspPoly;
			Model->PolysTemp->Element.push_back(FrontPoly);
			Model->PolysTemp->Element.push_back(BackPoly);
			break;

			//default:
			//	appErrorf( TEXT("FZoneFilter::FilterToLeaf: Unknown split code") );
		}
	}
	//SAFE_DELETE(FrontPoly);
	//SAFE_DELETE(BackPoly);

	if( nFront && nBack )
	{
		// Add partitioner plane to front and back.
		CBspPoly InfiniteEdPoly = BuildInfiniteFPoly( Model, iNode );
		InfiniteEdPoly.iBrushPoly = iNode;

		SplitPartitioner(Model,PolyList,FrontList,BackList,0,nPolys,nFront,nBack,InfiniteEdPoly);
	}
	else
	{
		//if( !nFront ) debugf( NAME_Log, TEXT("FilterBound: Empty fronthull") );
		//if( !nBack  ) debugf( NAME_Log, TEXT("FilterBound: Empty backhull") );
	}

	// Recursively update all our childrens' bounding volumes.
	if( nFront > 0 )
	{
		if( Node.iFront != INDEX_NONE )
			FilterBound( Model, &Bound, Node.iFront, FrontList, nFront, Outside || Node.IsCsg() );
		else if( Outside || Node.IsCsg() )
			UpdateBoundWithPolys( Bound, FrontList, nFront );
		else
			UpdateConvolutionWithPolys( Model, iNode, FrontList, nFront );
	}
	if( nBack > 0 )
	{
		if( Node.iBack != INDEX_NONE)
			FilterBound( Model, &Bound,Node.iBack, BackList, nBack, Outside && !Node.IsCsg() );
		else if( Outside && !Node.IsCsg() )
			UpdateBoundWithPolys( Bound, BackList, nBack );
		else
			UpdateConvolutionWithPolys( Model, iNode, BackList, nBack );
	}

	// Update parent bound to enclose this bound.
	if( ParentBound )
		*ParentBound += Bound;

	//for (int i=0;i<nPolys*2+16;i++)
	//{
	//	SAFE_DELETE(FrontList[i]);
	//	SAFE_DELETE(BackList[i]);
	//}

	SAFE_DELETE_ARR(FrontList);
	SAFE_DELETE_ARR(BackList);
	//Mark.Pop();
}



//
// Find the best splitting polygon within a pool of polygons, and return its
// index (into the PolyList array).
//
CBspPoly *FindBestSplit
(
	INT					NumPolys,
	CBspPoly**				PolyList,
	EBspOptimization	Opt,
	INT					Balance,
	INT					InPortalBias
)
{
	//check(NumPolys>0);

	// No need to test if only one poly.
	if( NumPolys==1 )
		return PolyList[0];

	CBspPoly   *Poly, *Best=NULL;
	f32   Score, BestScore;
	int     i, Index, j, Inc;
	int     Splits, Front, Back, Coplanar, AllSemiSolids;

	//PortalBias -- added by Legend on 4/12/2000
	float	PortalBias = InPortalBias / 100.0f;
	Balance &= 0xFF;								// keep only the low byte to recover "Balance"
	//GLog->Logf( TEXT("Balance=%d PortalBias=%f"), Balance, PortalBias );

	if		(Opt==BSP_Optimal)  Inc = 1;					// Test lots of nodes.
	else if (Opt==BSP_Good)		Inc = max_(1,NumPolys/20);	// Test 20 nodes.
	else /* BSP_Lame */			Inc = max_(1,NumPolys/4);	// Test 4 nodes.

	// See if there are any non-semisolid polygons here.
	for( i=0; i<NumPolys; i++ )
		if( !(PolyList[i]->PolyFlags & PF_AddLast) )
			break;
	AllSemiSolids = (i>=NumPolys);

	// Search through all polygons in the pool and find:
	// A. The number of splits each poly would make.
	// B. The number of front and back nodes the polygon would create.
	// C. Number of coplanars.
	BestScore = 0;
	for( i=0; i<NumPolys; i+=Inc )
	{
		Splits = Front = Back = Coplanar = 0;
		Index = i-1;
		do
		{
			Index++;
			Poly = PolyList[Index];
		} while( Index<(i+Inc) && Index<NumPolys 
			&& ( (Poly->PolyFlags & PF_AddLast) && !(Poly->PolyFlags & PF_Portal) )
			&& !AllSemiSolids );
		if( Index>=i+Inc || Index>=NumPolys )
			continue;

		for( j=0; j<NumPolys; j+=Inc ) if( j != Index )
		{
			CBspPoly *OtherPoly = PolyList[j];
			switch( OtherPoly->SplitWithPlaneFast( plane3df( Poly->Vertex[0].Position , Poly->Normal), NULL, NULL ) )
			{
				case SP_Coplanar:
					Coplanar++;
					break;

				case SP_Front:
					Front++;
					break;

				case SP_Back:
					Back++;
					break;

				case SP_Split:
					// Disfavor splitting polys that are zone portals.
					if( !(OtherPoly->PolyFlags & PF_Portal) )
						Splits++;
					else
						Splits += 16;
					break;
			}
		}
		// added by Legend 1/31/1999
		// Score optimization: minimize cuts vs. balance tree (as specified in BSP Rebuilder dialog)
		Score = ( 100.0 - float(Balance) ) * Splits + float(Balance) * Abs( Front - Back );
		if( Poly->PolyFlags & PF_Portal )
		{
			// PortalBias -- added by Legend on 4/12/2000
			//
			// PortalBias enables level designers to control the effect of Portals on the BSP.
			// This effect can range from 0.0 (ignore portals), to 1.0 (portals cut everything).
			//
			// In builds prior to this (since the 221 build dating back to 1/31/1999) the bias
			// has been 1.0 causing the portals to cut the BSP in ways that will potentially
			// degrade level performance, and increase the BSP complexity.
			// 
			// By setting the bias to a value between 0.3 and 0.7 the positive effects of 
			// the portals are preserved without giving them unreasonable priority in the BSP.
			//
			// Portals should be weighted high enough in the BSP to separate major parts of the
			// level from each other (pushing entire rooms down the branches of the BSP), but
			// should not be so high that portals cut through adjacent geometry in a way that
			// increases complexity of the room being (typically, accidentally) cut.
			//
			Score -= ( 100.0 - float(Balance) ) * Splits * PortalBias; // ignore PortalBias of the split polys -- bias toward portal selection for cutting planes!
		}
		//debugf( "  %4d: Score = %f (Front = %4d, Back = %4d, Splits = %4d, Flags = %08X)", Index, Score, Front, Back, Splits, Poly->PolyFlags ); //LEC

		if( Score<BestScore || !Best )
		{
			Best      = Poly;
			BestScore = Score;
		}
	}
	//check(Best);
	return Best;
}

//
// Pick a splitter poly then split a pool of polygons into front and back polygons and
// recurse.
//
// iParent = Parent Bsp node, or INDEX_NONE if this is the root node.
// IsFront = 1 if this is the front node of iParent, 0 of back (undefined if iParent==INDEX_NONE)
//
void SplitPolyList
(
 CBspModel				*Model,
 INT                 iParent,
 ENodePlace			NodePlace,
 INT                 NumPolys,
 CBspPoly				**PolyList,
 EBspOptimization	Opt,
 INT					Balance,
 INT					PortalBias,
 INT					RebuildSimplePolys
 )
{
	//FMemMark Mark(GMem);

	// To account for big EdPolys split up.
	int NumPolysToAlloc = NumPolys + 8 + NumPolys/4;

	int NumFront=0; 
	CBspPoly **FrontList = new CBspPoly*[NumPolysToAlloc];
	memset(FrontList,0x00,sizeof(CBspPoly*)*(NumPolysToAlloc));

	int NumBack =0; 
	CBspPoly **BackList  = new CBspPoly*[NumPolysToAlloc];
	memset(BackList,0x00,sizeof(CBspPoly*)*(NumPolysToAlloc));



	CBspPoly *SplitPoly = FindBestSplit( NumPolys, PolyList, Opt, Balance, PortalBias );

	// Add the splitter poly to the Bsp with either a new BspSurf or an existing one.
	if( RebuildSimplePolys )
		SplitPoly->iLink = Model->Surfs.size();

	INT iOurNode	 = GEditor->bspAddNode(Model,iParent,NodePlace,0,SplitPoly);
	INT iPlaneNode = iOurNode;

	// Now divide all polygons in the pool into (A) polygons that are
	// in front of Poly, and (B) polygons that are in back of Poly.
	// Coplanar polys are inserted immediately, before recursing.

	// If any polygons are split by Poly, we ignrore the original poly,
	// split it into two polys, and add two new polys to the pool.

	CBspPoly *FrontEdPoly = new CBspPoly;
	CBspPoly *BackEdPoly  = new CBspPoly;
    Model->PolysTemp->Element.push_back(FrontEdPoly);
	Model->PolysTemp->Element.push_back(BackEdPoly);
	for( INT i=0; i<NumPolys; i++ )
	{
		CBspPoly *EdPoly = PolyList[i];
		if( EdPoly == SplitPoly )
			continue;

		switch( EdPoly->SplitWithPlane( SplitPoly->Vertex[0].Position , SplitPoly->Normal, FrontEdPoly, BackEdPoly, 0 ) )
		{
		case SP_Coplanar:
			if( RebuildSimplePolys )
				EdPoly->iLink = Model->Surfs.size()-1;
			iPlaneNode = GEditor->bspAddNode( Model, iPlaneNode, NODE_Plane, 0, EdPoly );

			break;

		case SP_Front:
			FrontList[NumFront++] = PolyList[i];
			break;

		case SP_Back:
			BackList[NumBack++] = PolyList[i];
			break;

		case SP_Split:

			// Create front & back nodes.
			FrontList[NumFront++] = FrontEdPoly;
			BackList [NumBack ++] = BackEdPoly;

			// If newly-split polygons have too many vertices, break them up in half.
			if( FrontEdPoly->NumVertices >= CBspPoly::VERTEX_THRESHOLD )
			{
				CBspPoly *Temp = new CBspPoly;
				FrontEdPoly->SplitInHalf(Temp);
				FrontList[NumFront++] = Temp;
			}
			if( BackEdPoly->NumVertices >= CBspPoly::VERTEX_THRESHOLD )
			{
				CBspPoly *Temp = new CBspPoly;
				BackEdPoly->SplitInHalf(Temp);
				BackList[NumBack++] = Temp;
			}
			FrontEdPoly = new CBspPoly;
			BackEdPoly  = new CBspPoly;
			Model->PolysTemp->Element.push_back(FrontEdPoly);
			Model->PolysTemp->Element.push_back(BackEdPoly);
			break;
		}
	}


	// Recursively split the front and back pools.
	if( NumFront > 0 ) SplitPolyList( Model, iOurNode, NODE_Front, NumFront, FrontList, Opt, Balance, PortalBias, RebuildSimplePolys );
	if( NumBack  > 0 ) SplitPolyList( Model, iOurNode, NODE_Back,  NumBack,  BackList,  Opt, Balance, PortalBias, RebuildSimplePolys );
	
	/*for (int i=0;i<NumPolysToAlloc;i++)
	{
		bool  bDelFront=true;
		bool  bDelBack=true;
		for( INT j=0; j<NumPolys;j++ )
		{  
			if(bDelFront)
				if(FrontList[i]==PolyList[j])
					bDelFront=false;

			if(bDelBack)	
				if(BackList[i]==PolyList[j])
					bDelBack=false;			
		}
		if(bDelFront)
			SAFE_DELETE(FrontList[i]);
		if(bDelBack)
			SAFE_DELETE(BackList[i]);
	}*/
	//SAFE_DELETE(FrontEdPoly);
	//SAFE_DELETE(BackEdPoly);
	SAFE_DELETE_ARR(FrontList);
	SAFE_DELETE_ARR(BackList);
	//Mark.Pop();
}

//
// Convert a Bsp node's polygon to an EdPoly, add it to the list, and recurse.
//
void MakeEdPolys( CBspModel* Model, INT iNode )
{
	FBspNode* Node = Model->Nodes[iNode];

	CBspPoly Temp;
	if( GEditor->bspNodeToFPoly(Model,iNode,&Temp) >= 3 )
	{
		CBspPoly* pTemp=new CBspPoly(Temp);
		Model->Polys->Element.push_back(pTemp);
	}

	if( Node->iFront!=INDEX_NONE ) MakeEdPolys( Model, Node->iFront );
	if( Node->iBack !=INDEX_NONE ) MakeEdPolys( Model, Node->iBack  );
	if( Node->iPlane!=INDEX_NONE ) MakeEdPolys( Model, Node->iPlane );
}


//
// Add a new vector to the model, merging near-duplicates,  and return its index.
//
INT CBspEditor::bspAddVector( CBspModel *Model, vector3df *V, int Normal )
{
	return AddThing1
		(
		Model->Vectors,
		*V,
		Normal ? THRESH_NORMALS_ARE_SAME : THRESH_VECTORS_ARE_NEAR,
		1
		);
}


//
// Perform visibility testing within the level.
//
void CBspEditor::TestVisibility( CBspModel* Model, int A, int B )
{
	if( Model->Nodes.size() )
	{
		// Test visibility.
		FEditorVisibility Visi( Model, A );
		Visi.TestVisibility();
	}
}
//
// Build EdPoly list from a model's Bsp. Not transactional.
//
void CBspEditor::bspBuildFPolys( CBspModel* Model, bool SurfLinks, INT iNode )
{
	Model->Polys=new  CBspPolys;
	//Model->Polys->CleanPloys();
	if( Model->Nodes.size() )
		MakeEdPolys( Model, iNode );
	if( !SurfLinks )
		for( INT i=0; i<Model->Polys->Element.size(); i++ )
			Model->Polys->Element[i]->iLink=i;

}
void CBspEditor::BulidBspFromIMesh (IRenderSystem *pRS, IMeshSnapshot *pMS, IMtrl *pMtrl,s32	PolyFlags )
{
	//SetModel(m_pBspModel);
	SetBrush(pRS,pMS,pMtrl,0,CSG_Add,PolyFlags);
	BulidBsp();
}
void CBspEditor::BulidBsp( )
{
	u8 CSGOper=m_pBspBrush->mCsgOper;
	// Pass the brush polys through the world Bsp.
	if( CSGOper==CSG_Intersect || CSGOper==CSG_Deintersect )
	{
		//// Empty the brush.
		//Brush->EmptyModel(1,1);

		//// Intersect and deintersect.
		//for( i=0; i<TempModel->Polys->Element.Num(); i++ )
		//{
		//	CBspPoly EdPoly = TempModel->Polys->Element(i);
		//	GModel = Brush;
		//	BspFilterFPoly( CSGOper==CSG_Intersect ? IntersectBrushWithWorldFunc : DeIntersectBrushWithWorldFunc, Model,  &EdPoly );
		//}
		//NumPolysFromBrush = Brush->Polys->Element.Num();
	}
	else
	{
		// Add and subtract.
		for( int i=0; i<m_pBspBrush->Polys->Element.size(); i++ )
		{
			CBspPoly  *EdPoly =m_pBspBrush->Polys->Element[i];//简单的缺省copy CBspPoly不含指针且没有专门析构

			// Mark the polygon as non-cut so that it won't be harmed unless it must
			// be split, and set iLink so that BspAddNode will know to add its information
			// if a node is added based on this poly.
			//	EdPoly.PolyFlags &= ~(PF_EdCut);
			//if( EdPoly->iLink == i )
			{
			//EdPoly.iLink = TempModel->Polys->Element(i).iLink = Model->Surfs.Num();
				EdPoly->iLink = m_pBspModel->Surfs.size();
			}
			//else
			{
			//EdPoly.iLink = TempModel->Polys->Element(EdPoly.iLink).iLink;
			}

			// Filter brush through the world.
			BspFilterFPoly( CSGOper==CSG_Add ? AddBrushToWorldFunc : SubtractBrushFromWorldFunc, m_pBspModel, EdPoly );
			//BspFilterFPoly((BSP_FILTER_FUNC)SubtractBrushFromWorldFunc, m_pBspModel, EdPoly );
		}
	}
}
//
// Build an CBspPoly representing an "infinite" plane (which exceeds the maximum
// dimensions of the world in all directions) for a particular Bsp node.
//

//----------------------------------------------------------------------------------------------------------


void CBspEditor::FilterEdPoly(BSP_FILTER_FUNC	FilterFunc, CBspModel				*Model,s32							iNode, CBspPoly					*EdPoly, FCoplanarInfo			CoplanarInfo, s32							Outside)
{
	s32            SplitResult,iOurFront,iOurBack;
	s32			   NewFrontOutside,NewBackOutside;

FilterLoop:
	if( EdPoly->NumVertices >= CBspPoly::VERTEX_THRESHOLD )
	{
		// Split EdPoly in half to prevent vertices from overflowing.
		CBspPoly Temp;
		EdPoly->SplitInHalf(&Temp);

		// Filter other half.
		FilterEdPoly( FilterFunc, Model, iNode, &Temp, CoplanarInfo, Outside );
	}

	// Split em.
	CBspPoly TempFrontEdPoly,TempBackEdPoly;
	int temp=Model->Surfs[Model->Nodes[iNode]->iSurf]->vNormal;
	temp=Model->Nodes[iNode]->iSurf;
	SplitResult = EdPoly->SplitWithPlane
		(
		Model->Points [Model->Verts[Model->Nodes[iNode]->iVertPool].pVertex].Position,
		Model->Vectors[Model->Surfs[Model->Nodes[iNode]->iSurf]->vNormal],
		&TempFrontEdPoly,
		&TempBackEdPoly,
		0
		);

	// Process split results.
	if( SplitResult == SP_Front )
	{
Front:

		FBspNode *Node = Model->Nodes[iNode];
		Outside        = Outside || Node->IsCsg();

		if( Node->iFront == INDEX_NONE )
		{
			FilterLeaf(FilterFunc,Model,iNode,EdPoly,CoplanarInfo,Outside,NODE_Front);
		}
		else
		{
			iNode = Node->iFront;
			goto FilterLoop;
		}
	}
	else if( SplitResult == SP_Back )
	{
		FBspNode *Node = Model->Nodes[iNode];
		Outside        = Outside && !Node->IsCsg();

		if( Node->iBack == INDEX_NONE )
		{
			FilterLeaf( FilterFunc, Model, iNode, EdPoly, CoplanarInfo, Outside, NODE_Back );
		}
		else
		{
			iNode=Node->iBack;
			goto FilterLoop;
		}
	}
	else if( SplitResult == SP_Coplanar )
	{
		if( CoplanarInfo.iOriginalNode != INDEX_NONE )
		{
			// This will happen once in a blue moon when a polygon is barely outside the
			// coplanar threshold and is split up into a new polygon that is
			// is barely inside the coplanar threshold.  To handle this, just classify
			// it as front and it will be handled propery.
			//GErrors++;
			//debugf( NAME_Warning, TEXT("FilterEdPoly: Encountered out-of-place coplanar") );
			goto Front;
		}
		CoplanarInfo.iOriginalNode        = iNode;
		CoplanarInfo.iBackNode            = INDEX_NONE;
		CoplanarInfo.ProcessingBack       = 0;
		CoplanarInfo.BackNodeOutside      = Outside;
		NewFrontOutside                   = Outside;

		// See whether Node's iFront or iBack points to the side of the tree on the front
		// of this polygon (will be as expected if this polygon is facing the same
		// way as first coplanar in link, otherwise opposite).
		if( (Model->Nodes[iNode]->Plane.Normal.dotProduct(EdPoly->Normal) ) >= 0.0 )
		{
			iOurFront = Model->Nodes[iNode]->iFront;
			iOurBack  = Model->Nodes[iNode]->iBack;

			if( Model->Nodes[iNode]->IsCsg() )
			{
				CoplanarInfo.BackNodeOutside = 0;
				NewFrontOutside              = 1;
			}
		}
		else
		{
			iOurFront = Model->Nodes[iNode]->iBack;
			iOurBack  = Model->Nodes[iNode]->iFront;

			if( Model->Nodes[iNode]->IsCsg() )
			{
				CoplanarInfo.BackNodeOutside = 1; 
				NewFrontOutside              = 0;
			}
		}

		// Process front and back.
		if ((iOurFront==INDEX_NONE)&&(iOurBack==INDEX_NONE))
		{
			// No front or back.
			CoplanarInfo.ProcessingBack		= 1;
			CoplanarInfo.FrontLeafOutside	= NewFrontOutside;
			FilterLeaf
				(
				FilterFunc,
				Model,
				iNode,
				EdPoly,
				CoplanarInfo,
				CoplanarInfo.BackNodeOutside,
				NODE_Plane
				);
		}
		else if( iOurFront==INDEX_NONE && iOurBack!=INDEX_NONE )
		{
			// Back but no front.
			CoplanarInfo.ProcessingBack		= 1;
			CoplanarInfo.iBackNode			= iOurBack;
			CoplanarInfo.FrontLeafOutside	= NewFrontOutside;

			iNode   = iOurBack;
			Outside = CoplanarInfo.BackNodeOutside;
			goto FilterLoop;
		}
		else
		{
			// Has a front and maybe a back.

			// Set iOurBack up to process back on next call to FilterLeaf, and loop
			// to process front.  Next call to FilterLeaf will set FrontLeafOutside.
			CoplanarInfo.ProcessingBack = 0;

			// May be a node or may be INDEX_NONE.
			CoplanarInfo.iBackNode = iOurBack;

			iNode   = iOurFront;
			Outside = NewFrontOutside;
			goto FilterLoop;
		}
	}
	else if( SplitResult == SP_Split )
	{
		// Front half of split.
		if( Model->Nodes[iNode]->IsCsg() )
		{
			NewFrontOutside = 1; 
			NewBackOutside  = 0;
		}
		else
		{
			NewFrontOutside = Outside;
			NewBackOutside  = Outside;
		}

		if( Model->Nodes[iNode]->iFront==INDEX_NONE )
		{
			FilterLeaf
				(
				FilterFunc,
				Model,
				iNode,
				&TempFrontEdPoly,
				CoplanarInfo,
				NewFrontOutside,
				NODE_Front
				);
		}
		else
		{
			FilterEdPoly
				(
				FilterFunc,
				Model,
				Model->Nodes[iNode]->iFront,
				&TempFrontEdPoly,
				CoplanarInfo,
				NewFrontOutside
				);
		}

		// Back half of split.
		if( Model->Nodes[iNode]->iBack==INDEX_NONE )
		{
			FilterLeaf
				(
				FilterFunc,
				Model,
				iNode,
				&TempBackEdPoly,
				CoplanarInfo,
				NewBackOutside,
				NODE_Back
				);
		}
		else
		{
			FilterEdPoly
				(
				FilterFunc,
				Model,
				Model->Nodes[iNode]->iBack,
				&TempBackEdPoly,
				CoplanarInfo,
				NewBackOutside
				);
		}
	}
}
//-------------------------------------------------------------------------------------------------
//
// Handle a piece of a polygon that was filtered to a leaf.
//
void CBspEditor::FilterLeaf
(
 BSP_FILTER_FUNC FilterFunc, 
 CBspModel*			Model,
 s32			    iNode, 
 CBspPoly*			EdPoly, 
 FCoplanarInfo	CoplanarInfo, 
 s32				LeafOutside, 
 ENodePlace		ENodePlace
 )
{
	EPolyNodeFilter FilterType;

	if( CoplanarInfo.iOriginalNode == INDEX_NONE )
	{
		// Processing regular, non-coplanar polygons.
		FilterType = LeafOutside ? F_OUTSIDE : F_INSIDE;
		FilterFunc( Model, iNode, EdPoly, FilterType, ENodePlace );
	}
	else if( CoplanarInfo.ProcessingBack )
	{
		// Finished filtering polygon through tree in back of parent coplanar.
DoneFilteringBack:
		if      ((!LeafOutside) && (!CoplanarInfo.FrontLeafOutside)) FilterType = F_COPLANAR_INSIDE;
		else if (( LeafOutside) && ( CoplanarInfo.FrontLeafOutside)) FilterType = F_COPLANAR_OUTSIDE;
		else if ((!LeafOutside) && ( CoplanarInfo.FrontLeafOutside)) FilterType = F_COSPATIAL_FACING_OUT;
		else if (( LeafOutside) && (!CoplanarInfo.FrontLeafOutside)) FilterType = F_COSPATIAL_FACING_IN;
		else
		{
			//appErrorf( TEXT("FilterLeaf: Bad Locs") );
			return;
		}
		FilterFunc( Model, CoplanarInfo.iOriginalNode, EdPoly, FilterType, NODE_Plane );
	}
	else
	{
		CoplanarInfo.FrontLeafOutside = LeafOutside;

		if( CoplanarInfo.iBackNode == INDEX_NONE )
		{
			// Back tree is empty.
			LeafOutside = CoplanarInfo.BackNodeOutside;
			goto DoneFilteringBack;
		}
		else
		{
			// Call FilterEdPoly to filter through the back.  This will result in
			// another call to FilterLeaf with iNode = leaf this falls into in the
			// back tree and EdPoly = the final EdPoly to insert.
			CoplanarInfo.ProcessingBack=1;
			FilterEdPoly( FilterFunc, Model, CoplanarInfo.iBackNode, EdPoly,CoplanarInfo, CoplanarInfo.BackNodeOutside );
		}
	}
}	
s32  CBspEditor::bspAddPoint( CBspModel *Model, FModelVertex *V, int Exact )
{
	FLOAT Thresh = Exact ? THRESH_POINTS_ARE_SAME : THRESH_POINTS_ARE_NEAR;//Exact=1 0.002f,Exact=0 0.015f,

	// Try to find a match quickly from the Bsp. This finds all potential matches
	// except for any dissociated from nodes/surfaces during a rebuild.
	vector3df Temp;
	s32 pVertex;
	FLOAT NearestDist = Model->FindNearestVertex(V->Position,Temp,Thresh,pVertex);
	if( (NearestDist >= 0.0) && (NearestDist <= Thresh) &&(*V==Model->Points[pVertex]))
	{
		// Found an existing point.
		//比较该点的normal 与UV
		return pVertex;
	}
	else
	{
		// No match found; add it slowly to find duplicates.
		return AddThing( Model->Points, *V, Thresh, !FastRebuild );
		//return AddThing( Model->Points, *V, Thresh, 1 );
		return 1;
	}
}
s32 CBspEditor::bspAddNode
(
 CBspModel*		Model, 
 s32         iParent, 
 ENodePlace	NodePlace,
 DWORD		NodeFlags, 
 CBspPoly*		EdPoly
 )
{
	if( NodePlace == NODE_Plane )
	{
		// Make sure coplanars are added at the end of the coplanar list so that 
		// we don't insert NF_IsNew nodes with non NF_IsNew coplanar children.
		while( Model->Nodes[iParent]->iPlane != INDEX_NONE )
			iParent = Model->Nodes[iParent]->iPlane;
	}
	FBspSurf* Surf = NULL;
	if( EdPoly->iLink == Model->Surfs.size() )
	{
		FBspSurf*pTempSurf=new  FBspSurf;
		Model->Surfs.push_back(pTempSurf);
		s32 NewIndex	= Model->Surfs.size();
		Surf  = Model->Surfs[NewIndex-1];

		//s32 NewIndex = Model->Surfs.AddZeroed();
		//Surf = &Model->Surfs(NewIndex);

		// This node has a new polygon being added by bspBrushCSG; must set its properties here.
		/*if(Model->Points.size()>0)
		{
			f32 TempRadius =EdPoly->Base.getDistanceFrom(Model->Points[0].Position);
			if(TempRadius>THRESH_POINTS_ARE_SAME||TempRadius<-THRESH_POINTS_ARE_SAME)
				Surf->pBase     = bspAddPoint  (Model,&EdPoly->Base,1);
			else
				Surf->pBase     = 0;
		}
		else
		{
			Surf->pBase     = bspAddPoint  (Model,&EdPoly->Base,1);
		}*/
		
		Surf->pBase     = bspAddPoint  (Model,&EdPoly->Base,1);
		Surf->vNormal   	= bspAddVector (Model,&EdPoly->Normal,1);
		//Surf->vTextureU 	= bspAddVector (Model,&EdPoly->TextureU,0);
		//Surf->vTextureV 	= bspAddVector (Model,&EdPoly->TextureV,0);
		//Surf->Material  	= EdPoly->Material;
		//Surf->Actor			= NULL;

		Surf->PolyFlags 	= EdPoly->PolyFlags & ~PF_NoAddToBSP;
		Surf->LightMapScale	= EdPoly->LightMapScale;

		//Surf->Actor	 		= EdPoly->Actor;
		Surf->iBrushPoly	= EdPoly->iBrushPoly;

		Surf->Plane			= plane3df(EdPoly->Base.Position,EdPoly->Normal);
	}
	else
	{
		//check(EdPoly->iLink!=INDEX_NONE);
		//check(EdPoly->iLink<Model->Surfs.Num());
		//Surf = Model->Surfs[EdPoly->iLink];
	}

	// Set NodeFlags.
	//if( Surf->PolyFlags & PF_NotSolid              ) NodeFlags |= NF_NotCsg;
	//if( Surf->PolyFlags & (PF_Invisible|PF_Portal) ) NodeFlags |= NF_NotVisBlocking;
	if(EdPoly->PolyFlags & ~PF_NoAddToBSP& (PF_Invisible|PF_Portal) ) NodeFlags |= NF_NotVisBlocking;;

	if( EdPoly->NumVertices > FBspNode::MAX_NODE_VERTICES )
	{
		// Split up into two coplanar sub-polygons (one with MAX_NODE_VERTICES vertices and
		// one with all the remaining vertices) and recursively add them.

		// Copy first bunch of verts.
		//FMemMark Mark(GMem);
		CBspPoly *EdPoly1 = new CBspPoly;
		*EdPoly1 = *EdPoly;
		EdPoly1->NumVertices = FBspNode::MAX_NODE_VERTICES;

		CBspPoly *EdPoly2 = new CBspPoly;
		*EdPoly2 = *EdPoly;
		EdPoly2->NumVertices = EdPoly->NumVertices + 2 - FBspNode::MAX_NODE_VERTICES;

		// Copy first vertex then the remaining vertices.
		appMemmove
			(
			&EdPoly2->Vertex[1],
			&EdPoly->Vertex [FBspNode::MAX_NODE_VERTICES - 1],
			(EdPoly->NumVertices + 1 - FBspNode::MAX_NODE_VERTICES) * sizeof (vector3df)
			);
		s32 iNode = GEditor->bspAddNode( Model, iParent, NodePlace, NodeFlags, EdPoly1 ); // Add this poly first.
		GEditor->bspAddNode( Model, iNode,   NODE_Plane, NodeFlags, EdPoly2 ); // Then add other (may be bigger).
		delete EdPoly1;
		delete EdPoly2;
		//Mark.Pop();
		return iNode; // Return coplanar "parent" node (not coplanar child)
	}
	else
	{
		// Add node.
		//if( NodePlace!=NODE_Root )
		//	Model->Nodes.ModifyItem( iParent );<-为undo 准备.
		FBspNode*pTempNode=new  FBspNode;
		int temp1=sizeof(FBspNode);
		memset(pTempNode,0x00,sizeof(FBspNode)) ;
		Model->Nodes.push_back(pTempNode);
		s32 iNode			 = Model->Nodes.size()-1;
		FBspNode& Node       = *(Model->Nodes[iNode]);

		// Tell transaction tracking system that parent is about to be modified.
		FBspNode* Parent=NULL;
		if( NodePlace!=NODE_Root )
			Parent = Model->Nodes[iParent];

		// Set node properties.
		Node.iSurf       	 = EdPoly->iLink;
		Node.NodeFlags   	 = NodeFlags;
		Node.iCollisionBound = INDEX_NONE;
		Node.ZoneMask		 = Parent ? Parent->ZoneMask : FZoneSet::AllZones();
		Node.Plane           = plane3df( EdPoly->Vertex[0].Position, EdPoly->Normal );
		Node.iVertPool       = Model->Verts.size();
		for(int i=0;i<EdPoly->NumVertices;i++)
		{
			FVert temp;
			Model->Verts.push_back(temp);//缺省类copy,因为不含指针.
		}
		Node.iFront		     = INDEX_NONE;
		Node.iBack		     = INDEX_NONE;
		Node.iPlane		     = INDEX_NONE;
		if( NodePlace==NODE_Root )
		{
			Node.iLeaf[0]	 = INDEX_NONE;
			Node.iLeaf[1] 	 = INDEX_NONE;
			Node.iZone[0]	 = 0;
			Node.iZone[1]	 = 0;
		}
		else if( NodePlace==NODE_Front || NodePlace==NODE_Back )
		{
			s32 ZoneFront=NodePlace==NODE_Front;
			Node.iLeaf[0]	 = Parent->iLeaf[ZoneFront];
			Node.iLeaf[1] 	 = Parent->iLeaf[ZoneFront];
			Node.iZone[0]	 = Parent->iZone[ZoneFront];
			Node.iZone[1]	 = Parent->iZone[ZoneFront];
		}
		else//共面情况
		{
			s32 IsFlipped    = (Node.Plane.dotProduct(Parent->Plane.Normal))<0.0;//共面也分法线是否一致，两种情况。
			Node.iLeaf[0]    = Parent->iLeaf[IsFlipped  ];
			Node.iLeaf[1]    = Parent->iLeaf[1-IsFlipped];
			Node.iZone[0]    = Parent->iZone[IsFlipped  ];
			Node.iZone[1]    = Parent->iZone[1-IsFlipped];
		}

		// Link parent to this node.
		if     ( NodePlace==NODE_Front ) Parent->iFront = iNode;
		else if( NodePlace==NODE_Back  ) Parent->iBack  = iNode;
		else if( NodePlace==NODE_Plane ) Parent->iPlane = iNode;

		// Add all points to point table, merging nearly-overlapping polygon points
		// with other points in the poly to prevent criscrossing vertices from
		// being generated.

		// Must maintain Node->NumVertices on the fly so that bspAddPoint is always
		// called with the Bsp in a clean state.
		vector3df 	Points[FBspNode::MAX_NODE_VERTICES];
		BYTE	n           = EdPoly->NumVertices;
		Node.NumVertices = 0;
		FVert* VertPool	 = &Model->Verts[Node.iVertPool]  ;
		for( BYTE i=0; i<n; i++ )
		{
			int pVertex = GEditor->bspAddPoint(Model,&EdPoly->Vertex[i],0);//bspAddPoint find near exist point,if find return exist point.no add new
			if( Node.NumVertices==0 || VertPool[Node.NumVertices-1].pVertex!=pVertex )//this point is not same as last point . 
			{
				Points[Node.NumVertices] = EdPoly->Vertex[i].Position ;
				VertPool[Node.NumVertices].iSide   = INDEX_NONE;
				VertPool[Node.NumVertices].pVertex = pVertex;
				Node.NumVertices++;
			}
		}
		if(Node.NumVertices<n)
		{
			Model->Verts.resize(Model->Verts.size()-(n-Node.NumVertices));
		}
		if( Node.NumVertices>=2 && VertPool[0].pVertex==VertPool[Node.NumVertices-1].pVertex )
		{
			Node.NumVertices--;
			 Model->Verts.resize(Model->Verts.size()-1);
		}
		if( Node.NumVertices < 3 )
		{
			//GErrors++;
			//debugf( NAME_Warning, TEXT("bspAddNode: Infinitesimal polygon %i (%i)"), Node.NumVertices, n );
			Node.NumVertices = 0;
			Model->Verts.resize(Model->Verts.size()-n);//can't make ploy
		}

		return iNode;
	}
}

int CBspEditor::bspNodeToFPoly
(
 CBspModel	*Model,
 INT	    iNode,
 CBspPoly	*EdPoly
 )
{
	CBspPoly		MasterEdPoly;
	BYTE		i,j,n,prev;

	FBspNode  *Node     	= Model->Nodes[iNode];
	FBspSurf *Poly     	= Model->Surfs[Node->iSurf];
	FVert	 *VertPool	= &(Model->Verts[Node->iVertPool]);

	//EdPoly->Base		= Model->Points [Poly->pBase].Position;
	EdPoly->Base		= Model->Points [Poly->pBase];
	EdPoly->Normal      = Model->Vectors[Poly->vNormal];

	EdPoly->PolyFlags 	= Poly->PolyFlags & ~(PF_EdCut | PF_EdProcessed | PF_Selected | PF_Memorized);
	EdPoly->iLink     	= Node->iSurf;

	//EdPoly->Material    = Poly->Material;
	//EdPoly->Actor    	= Poly.Actor;
	//if( polyFindMaster(Model,Node.iSurf,MasterEdPoly) )
	//	EdPoly->ItemName  = MasterEdPoly.ItemName;
	//else
	//	EdPoly->ItemName  = NAME_None;

	EdPoly->iBrushPoly  = Poly->iBrushPoly;
//	EdPoly->TextureU = Model->Vectors[Poly->vTextureU];
//	EdPoly->TextureV = Model->Vectors[Poly->vTextureV];

	EdPoly->LightMapScale = Poly->LightMapScale;

	n = Node->NumVertices;
	i=0; j=0; prev=n-1;

	for( i=0; i<n; i++ )
	{
		EdPoly->Vertex[j].Position  = Model->Points[VertPool[i].pVertex].Position;
		prev = i;
		j++;
	}
	if (j>=3) EdPoly->NumVertices=j;
	else      EdPoly->NumVertices=0;

	// Remove colinear points and identical points (which will appear
	// if T-joints were eliminated).
	EdPoly->RemoveColinears();

	return EdPoly->NumVertices;
}
//
// Repartition the bsp.
//
void CBspEditor::bspRepartition( CBspModel *Model, INT iNode, INT Simple )
{
	bspBuildFPolys( Model, 1, iNode );
	bspMergeCoplanars( Model, 0, 0 );
	bspBuild( Model, BSP_Good, 12, 70, Simple, iNode );
	bspRefresh( Model, 1 );
}
//
// Rebuild the level's Bsp from the level's CSG brushes.
//
void CBspEditor::csgRebuild_makePortal( CBspModel *Model )
{
	//bspRepartition( Model, 0, 0 );
	bspRepartition( Model, 0, 1);
	TestVisibility( Model, 0, 0 );
	bspBuildBounds( Model );
}
//
// Merge all coplanar EdPolys in a model.  Not transactional.
// Preserves (though reorders) iLinks.
//
void CBspEditor::bspMergeCoplanars( CBspModel* Model, bool RemapLinks, bool MergeDisparateTextures )
{
	INT OriginalNum = Model->Polys->Element.size();

	// Mark all polys as unprocessed.
	for( INT i=0; i<Model->Polys->Element.size(); i++ )
		Model->Polys->Element[i]->PolyFlags &= ~PF_EdProcessed;
	MergeDisparateTextures=false;
	// Find matching coplanars and merge them.
	//FMemMark Mark(GMem);
	//INT* PolyList = new(GMem,Model->Polys->Element.Num())INT;
	INT* PolyList = new INT [Model->Polys->Element.size()];
	INT n=0;
	for( INT i=0; i<Model->Polys->Element.size(); i++ )
	{
		CBspPoly* EdPoly = Model->Polys->Element[i];
		if( EdPoly->NumVertices>0 && !(EdPoly->PolyFlags & PF_EdProcessed) )
		{
			INT PolyCount         =  0;
			PolyList[PolyCount++] =  i;
			EdPoly->PolyFlags    |= PF_EdProcessed;
			for( INT j=i+1; j<Model->Polys->Element.size(); j++ )
			{
				CBspPoly* OtherPoly = Model->Polys->Element[j];
				//if( OtherPoly->iLink == EdPoly->iLink )
				{
					FLOAT Dist = (OtherPoly->Vertex[0].Position  - EdPoly->Vertex[0].Position ) .dotProduct(EdPoly->Normal);
					if
						(	Dist>-0.001
						&&	Dist<0.001
						&&	(OtherPoly->Normal.dotProduct(EdPoly->Normal))>0.9999
						&&	(MergeDisparateTextures
						||	(	FPointsAreNear(OtherPoly->TextureU,EdPoly->TextureU,THRESH_VECTORS_ARE_NEAR)
						&&	FPointsAreNear(OtherPoly->TextureV,EdPoly->TextureV,THRESH_VECTORS_ARE_NEAR) ) ) )
					{
						OtherPoly->PolyFlags |= PF_EdProcessed;
						PolyList[PolyCount++] = j;
					}
				}
			}
			if( PolyCount > 1 )
			{
				MergeCoplanars( Model, PolyList, PolyCount );
				n++;
			}
		}
	}
	//debugf( NAME_Log, TEXT("Found %i coplanar sets in %i"), n, Model->Polys->Element.Num() );
	//Mark.Pop();
	//SAFE_DELETE_ARR(PolyList);
	SAFE_DELETE(PolyList);
	// Get rid of empty EdPolys while remapping iLinks.
	INT j=0;
	//INT* Remap = new(GMem,Model->Polys->Element.Num())INT;
	INT* Remap = new INT[Model->Polys->Element.size()];

	for( INT i=0; i<Model->Polys->Element.size(); i++ )
	{
		if( Model->Polys->Element[i]->NumVertices==0 )
		{
			SAFE_DELETE(Model->Polys->Element[i]);
		}	

	}

	for( INT i=0; i<Model->Polys->Element.size(); i++ )
	{
		if( Model->Polys->Element[i]!=NULL&&Model->Polys->Element[i]->NumVertices )
		{
			Remap[i] = j;
			Model->Polys->Element[j] = Model->Polys->Element[i];
			j++;
		}
	}
	//Model->Polys->Element.Remove( j, Model->Polys->Element.Num()-j );
	//for(  i=Model->Polys->Element.size();i>j;i--)
	//{
	//	SAFE_DELETE(Model->Polys->Element[i-1]);
	//}
	Model->Polys->Element.resize(j);

	if( RemapLinks )
		for( INT i=0; i<Model->Polys->Element.size(); i++ )
			if( Model->Polys->Element[i]->iLink != INDEX_NONE )
				Model->Polys->Element[i]->iLink = Remap[Model->Polys->Element[i]->iLink];
	SAFE_DELETE(Remap);
}
//
// Build Bsp from the editor polygon set (EdPolys) of a model.
//
// Opt     = Bsp optimization, BSP_Lame (fast), BSP_Good (medium), BSP_Optimal (slow)
// Balance = 0-100, 0=only worry about minimizing splits, 100=only balance tree.
//
void CBspEditor::bspBuild
(
 CBspModel*				Model, 
 EBspOptimization	Opt, 
 INT					Balance, 
 INT					PortalBias,
 INT					RebuildSimplePolys,
 INT					iNode
 )
{
	INT OriginalPolys = Model->Polys->Element.size();

	// Empty the model's tables.
	if( RebuildSimplePolys==1 )
	{
		// Empty everything but polys.
		Model->EmptyModel( 1, 0 );
	}
	else if( RebuildSimplePolys==0 )
	{
		// Empty node vertices.
		for( INT i=0; i<Model->Nodes.size(); i++ )
			Model->Nodes[i]->NumVertices = 0;

		// Refresh the Bsp.
		bspRefresh(Model,1);

		// Empty nodes.
		Model->EmptyModel( 0, 0 );
	}
	if( Model->Polys->Element.size() )
	{
		// Allocate polygon pool.
		//FMemMark Mark(GMem);
		//CBspPoly** PolyList = new( GMem, Model->Polys->Element.Num() )CBspPoly*;
		CBspPoly** PolyList = new CBspPoly*[Model->Polys->Element.size()];

		// Add all FPolys to active list.
		for( int i=0; i<Model->Polys->Element.size(); i++ )
			if( Model->Polys->Element[i]->NumVertices )
				PolyList[i] = Model->Polys->Element[i];

		// Now split the entire Bsp by splitting the list of all polygons.
		if(Model->PolysTemp==NULL)
				Model->PolysTemp=new CBspPolys;
		SplitPolyList
			(
			Model,
			INDEX_NONE,
			NODE_Root,
			Model->Polys->Element.size(),
			PolyList,
			Opt,
			Balance,
			PortalBias,
			RebuildSimplePolys
			);
        SAFE_DELETE(Model->PolysTemp);
		// Now build the bounding boxes for all nodes.
		if( RebuildSimplePolys==0 )
		{
			// Remove unreferenced things.
			bspRefresh( Model, 1 );

			// Rebuild all bounding boxes.
			//bspBuildBounds( Model );
		}
		SAFE_DELETE_ARR(PolyList);//<-----------------------------------------------------------只释放PolyList
 		//Mark.Pop();
	}

	//debugf( NAME_Log, TEXT("bspBuild built %i convex polys into %i nodes"), OriginalPolys, Model->Nodes.Num() );
}


//
// Clean up all nodes after a CSG operation.  Resets temporary bit flags and unlinks
// empty leaves.  Removes zero-vertex nodes which have nonzero-vertex coplanars.
//
void CBspEditor::bspCleanup( CBspModel *Model )
{
	if( Model->Nodes.size() > 0 ) 
		CleanupNodes( Model, 0, INDEX_NONE );
}

//
// If the Bsp's point and vector tables are nearly full, reorder them and delete
// unused ones:
//
void CBspEditor::bspRefresh( CBspModel *Model, int NoRemapSurfs )
{
	//FMemMark Mark(GMem);
	INT *VectorRef, *PointRef, *NodeRef, *PolyRef,i;
	//std::vector<INT*>	VertexRef;
	BYTE  B;

	// Remove unreferenced Bsp surfs.
	//NodeRef		= new(GMem,MEM_Oned,Model->Nodes.Num())INT;
	//PolyRef		= new(GMem,MEM_Oned,Model->Surfs.Num())INT;
	NodeRef	= new INT[Model->Nodes.size()];
	memset(NodeRef,0xff,Model->Nodes.size() * sizeof (INT));

	PolyRef		= new INT[Model->Surfs.size()];
	memset(PolyRef,0xff,Model->Surfs.size() * sizeof (INT));

	if( Model->Nodes.size() > 0 )
		TagReferencedNodes( Model, NodeRef, PolyRef, 0 );

	for(INT PortalIndex = 0;PortalIndex < Model->PortalNodes.size();PortalIndex++)
		TagReferencedNodes(Model,NodeRef,PolyRef,Model->PortalNodes[PortalIndex]);

	if( NoRemapSurfs )
		//appMemzero(PolyRef,Model->Surfs.Num() * sizeof (INT));
		memset(PolyRef,0x00,Model->Surfs.size() * sizeof (INT));

	// Remap Bsp nodes and surfs.
	int n=0;
	for(  i=0; i<Model->Surfs.size(); i++ )
	{
		if( PolyRef[i]!=INDEX_NONE )
		{
			Model->Surfs[n] = Model->Surfs[i];
			PolyRef[i]=n++;
		}
	}
	//debugf( NAME_Log, TEXT("Polys: %i -> %i"), Model->Surfs.Num(), n );
	//Model->Surfs.Remove( n, Model->Surfs.Num()-n );
	for(  i=Model->Surfs.size();i>n;i--)
	{
		//Model->Surfs.pop_back();
		SAFE_DELETE(Model->Surfs[i-1]);
	}
	Model->Surfs.resize(n);

	n=0;
	for( i=0; i<Model->Nodes.size(); i++ ) if( NodeRef[i]!=INDEX_NONE )
	{
		Model->Nodes[n] = Model->Nodes[i];
		NodeRef[i]=n++;
	}
	//debugf( NAME_Log, TEXT("Nodes: %i -> %i"), Model->Nodes.Num(), n );
	//Model->Nodes.Remove( n, Model->Nodes.Num()-n  );
	for( i=Model->Nodes.size();i>n;i--)
	{
		//Model->Surfs.pop_back();
		SAFE_DELETE(Model->Nodes[i-1]);
	}
	Model->Nodes.resize(n);

	// Update Bsp nodes.
	for( i=0; i<Model->Nodes.size(); i++ )
	{
		FBspNode *Node = Model->Nodes[i];
		Node->iSurf = PolyRef[Node->iSurf];
		if (Node->iFront != INDEX_NONE) Node->iFront = NodeRef[Node->iFront];
		if (Node->iBack  != INDEX_NONE) Node->iBack  = NodeRef[Node->iBack];
		if (Node->iPlane != INDEX_NONE) Node->iPlane = NodeRef[Node->iPlane];
	}

	// Update portal list.
	for(INT PortalIndex = 0;PortalIndex < Model->PortalNodes.size();PortalIndex++)
		Model->PortalNodes[PortalIndex] = NodeRef[Model->PortalNodes[PortalIndex]];

	// Remove unreferenced points and vectors.
	//VectorRef = new(GMem,MEM_Oned,Model->Vectors.Num())INT;
	//PointRef  = new(GMem,MEM_Oned,Model->Points .Num ())INT;

	VectorRef = new INT[Model->Vectors.size()];

	PointRef  = new INT[Model->Points.size()];

	// Check Bsp surfs.
	for( i=0; i<Model->Surfs.size(); i++ )
	{
		FBspSurf *Surf = Model->Surfs[i];
		VectorRef [Surf->vNormal   ] = 0;
		//assert(Surf->vNormal<Model->Vectors.size());
		if(Surf->vNormal==(Model->Vectors.size()-1))
		{
			int temp=1;
		}
		//VectorRef [Surf->vTextureU ] = 0;
		//VectorRef [Surf->vTextureV ] = 0;
		PointRef  [Surf->pBase     ] = 0;
	}
	//char *testc=new char[3];
	//testc[3]='5';
	//delete testc;
		
	// Check Bsp nodes.
	for( i=0; i<Model->Nodes.size(); i++ )
	{
		// Tag all points used by nodes.
		FBspNode*	Node		= Model->Nodes[i];
		FVert*		VertPool	= &Model->Verts[Node->iVertPool];
		for( B=0; B<Node->NumVertices;  B++ )
		{
			PointRef[VertPool->pVertex] = 0;
			VertPool++;
		}
		Node++;
	}

	// Remap points.
	n=0; 
	for( i=0; i<Model->Points.size(); i++ ) if( PointRef[i]!=INDEX_NONE )
	{
		Model->Points[n] = Model->Points[i];
		PointRef[i] = n++;
	}
	//debugf( NAME_Log, TEXT("Points: %i -> %i"), Model->Points.Num(), n );
	//Model->Points.Remove( n, Model->Points.Num()-n );
	//check(Model->Points.Num()==n);
	Model->Points.resize(n);

	// Remap vectors.
	n=0; 
	for (i=0; i<Model->Vectors.size(); i++) if (VectorRef[i]!=INDEX_NONE)
	{
		Model->Vectors[n] = Model->Vectors[i];
		VectorRef[i] = n++;
	}
	//debugf( NAME_Log, TEXT("Vectors: %i -> %i"), Model->Vectors.Num(), n );
	//Model->Vectors.Remove( n, Model->Vectors.Num()-n );
	Model->Vectors.resize(n);

	// Update Bsp surfs.
	for( i=0; i<Model->Surfs.size(); i++ )
	{
		FBspSurf *Surf	= Model->Surfs[i];
		Surf->vNormal   = VectorRef [Surf->vNormal  ];
//		assert(Surf->vNormal<Model->Vectors.size());
		//Surf->vTextureU = VectorRef [Surf->vTextureU];
		//Surf->vTextureV = VectorRef [Surf->vTextureV];
		Surf->pBase     = PointRef  [Surf->pBase    ];
	}

	// Update Bsp nodes.
	for( i=0; i<Model->Nodes.size(); i++ )
	{
		FBspNode*	Node		= Model->Nodes[i];
		FVert*		VertPool	= &Model->Verts[Node->iVertPool];
		for( B=0; B<Node->NumVertices;  B++ )
		{			
			VertPool->pVertex = PointRef [VertPool->pVertex];
			VertPool++;
		}

		Node++;
	}

	// Shrink the objects.
	//Model->ShrinkModel();<------------------------------------------------------vector 自动调整.

	SAFE_DELETE_ARR(VectorRef);
	SAFE_DELETE_ARR(PointRef);
	SAFE_DELETE_ARR(NodeRef);
	SAFE_DELETE_ARR(PolyRef);

	
	//SAFE_DELETE(PointRef);
	//SAFE_DELETE(NodeRef);
	//SAFE_DELETE(PolyRef);
	//SAFE_DELETE(VectorRef);
	//Mark.Pop();
}


//
// Build bounding volumes for all Bsp nodes.  The bounding volume of the node
// completely encloses the "outside" space occupied by the nodes.  Note that 
// this is not the same as representing the bounding volume of all of the 
// polygons within the node.
//
// We start with a practically-infinite cube and filter it down the Bsp,
// whittling it away until all of its convex volume fragments land in leaves.
//
void CBspEditor::bspBuildBounds( CBspModel* Model )
{
	if( Model->Nodes.size()==0 )
		return;

	BuildZoneMasks( Model, 0 );

	CBspPoly Polys[6], *PolyList[6];
	for( int i=0; i<6; i++ )
	{
		PolyList[i] = &Polys[i];
		PolyList[i]->Init();
		PolyList[i]->NumVertices=4;
		PolyList[i]->iBrushPoly = INDEX_NONE;
	}

	Polys[0].Vertex[0].Position =vector3df(-HALF_WORLD_MAX,-HALF_WORLD_MAX,HALF_WORLD_MAX);
	Polys[0].Vertex[1].Position =vector3df( HALF_WORLD_MAX,-HALF_WORLD_MAX,HALF_WORLD_MAX);
	Polys[0].Vertex[2].Position =vector3df( HALF_WORLD_MAX, HALF_WORLD_MAX,HALF_WORLD_MAX);
	Polys[0].Vertex[3].Position =vector3df(-HALF_WORLD_MAX, HALF_WORLD_MAX,HALF_WORLD_MAX);
	Polys[0].Normal   =vector3df( 0.000000,  0.000000,  1.000000 );
	//Polys[0].Base     =Polys[0].Vertex[0].Position;
	Polys[0].Base     =Polys[0].Vertex[0];

	Polys[1].Vertex[0].Position =vector3df(-HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[1].Vertex[1].Position =vector3df( HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[1].Vertex[2].Position =vector3df( HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[1].Vertex[3].Position =vector3df(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[1].Normal   =vector3df( 0.000000,  0.000000, -1.000000 );
	Polys[1].Base     =Polys[1].Vertex[0];

	Polys[2].Vertex[0].Position =vector3df(-HALF_WORLD_MAX,HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[2].Vertex[1].Position =vector3df(-HALF_WORLD_MAX,HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[2].Vertex[2].Position =vector3df( HALF_WORLD_MAX,HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[2].Vertex[3].Position =vector3df( HALF_WORLD_MAX,HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[2].Normal   =vector3df( 0.000000,  1.000000,  0.000000 );
	Polys[2].Base     =Polys[2].Vertex[0];

	Polys[3].Vertex[0].Position =vector3df( HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[3].Vertex[1].Position =vector3df( HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[3].Vertex[2].Position =vector3df(-HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[3].Vertex[3].Position =vector3df(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[3].Normal   =vector3df( 0.000000, -1.000000,  0.000000 );
	Polys[3].Base     =Polys[3].Vertex[0];

	Polys[4].Vertex[0].Position =vector3df(HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[4].Vertex[1].Position =vector3df(HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[4].Vertex[2].Position =vector3df(HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[4].Vertex[3].Position =vector3df(HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[4].Normal   =vector3df( 1.000000,  0.000000,  0.000000 );
	Polys[4].Base     =Polys[4].Vertex[0];

	Polys[5].Vertex[0].Position =vector3df(-HALF_WORLD_MAX,-HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[5].Vertex[1].Position =vector3df(-HALF_WORLD_MAX,-HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[5].Vertex[2].Position =vector3df(-HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	Polys[5].Vertex[3].Position =vector3df(-HALF_WORLD_MAX, HALF_WORLD_MAX,-HALF_WORLD_MAX);
	Polys[5].Normal   =vector3df(-1.000000,  0.000000,  0.000000 );
	Polys[5].Base     =Polys[5].Vertex[0];
	// Empty hulls.
	//Model->LeafHulls.Empty();
	Model->LeafHulls.clear();
	for( int i=0; i<Model->Nodes.size(); i++ )
		Model->Nodes[i]->iCollisionBound  = INDEX_NONE;

	if(Model->PolysTemp==NULL)
		Model->PolysTemp=new CBspPolys; 
	FilterBound( Model, NULL, 0, PolyList, 6, Model->RootOutside );
	SAFE_DELETE(Model->PolysTemp);
	//debugf( NAME_Log, TEXT("bspBuildBounds: Generated %i hulls"), Model->LeafHulls.Num() );
}

void CBspEditor::BspFilterFPoly( BSP_FILTER_FUNC FilterFunc, CBspModel *Model, CBspPoly *EdPoly )
{
	FCoplanarInfo StartingCoplanarInfo;
	StartingCoplanarInfo.iOriginalNode = INDEX_NONE;
	if( Model->Nodes.size()== 0 )
	{
		// If Bsp is empty, process at root.
		FilterFunc( Model, 0, EdPoly, Model->RootOutside ? F_OUTSIDE : F_INSIDE, NODE_Root );
	}
	else
	{
		// Filter through Bsp.
		FilterEdPoly( FilterFunc, Model, 0, EdPoly, StartingCoplanarInfo, Model->RootOutside );
	}
}







void CBspEditor:: BulidBspRenderSurf()
{
	//a:有Portal标志不放入VB.
	//b:MaterialID,SurflID->生成Render VB. 相同的MaterialID,SurflID->生成一个Render VB
	//c:若同一Render VB中有不同，iZone时，再切分。每个VB带一个zoneID.
	//d:该组面生成的阴影图〉2046*2046时，切分Render VB.
	if(m_pBspModel)
	{
		//m_pBspModel->
	}
}
//
// Assign contiguous unique numbers to all front and back leaves in the BSP.
// Stores the leaf numbers in FBspNode::iLeaf[2].
//对于实体CSG,前节点或后节点的child 任一为空时(包含同为空的情况）根据本身属性为前后叶子之一赋值。
//对于portal ,包含同为空时，同时为前后叶子赋值。

void FEditorVisibility::AssignLeaves( s32 iNode, s32 Outside )
{
	FBspNode *Node = Model->Nodes[iNode];
	for( int IsFront=0; IsFront<2; IsFront++ )
	{
		if( Node->iChild[IsFront] != INDEX_NONE )
		{
			AssignLeaves( Node->iChild[IsFront], Node->ChildOutside( IsFront, Outside, NF_NotVisBlocking ) );
		}
		else if( Node->ChildOutside( IsFront, Outside, NF_NotVisBlocking ) )
		{
			Model->Leaves.push_back(   new FLeaf(Model->Leaves.size())  );//<------------注意释放，在此设置Leaves.iZone
			Node->iLeaf[IsFront] = Model->Leaves.size()-1;//<----------------------------------------确认是否减1
		}
	}
}
//
// Mark a portal as blocked.
//
void FEditorVisibility::BlockPortal
(
 CBspPoly&	Poly,
 s32		iFrontLeaf,
 s32		iBackLeaf,
 s32		iGeneratingNode,
 s32		iGeneratingBase
 )
{
	if( iFrontLeaf!=INDEX_NONE && iBackLeaf!=INDEX_NONE )
	{
		for( FPortal* Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
		{
			if
				(	(Portal->iFrontLeaf==iFrontLeaf && Portal->iBackLeaf==iBackLeaf )
				||	(Portal->iFrontLeaf==iBackLeaf  && Portal->iBackLeaf==iFrontLeaf) )
			{
				Portal->iZonePortalSurf = iZonePortalSurf;
				NumZoneFragments++;
			}
		}
	}
}

//Add a portal to the portal list.

void FEditorVisibility::AddPortal
(
 CBspPoly&	Poly,
 s32		iFrontLeaf,
 s32		iBackLeaf,
 s32		iGeneratingNode,
 s32		iGeneratingBase
 )
{
	if( iFrontLeaf!=INDEX_NONE && iBackLeaf!=INDEX_NONE )
	{
		// Add to linked list of all portals.
		FirstPortal						= 
			LeafPortals[iFrontLeaf]			= 
			LeafPortals[iBackLeaf]			= 
			NodePortals[iGeneratingNode]	= 
			new FPortal                                 //<--------------------------------------注意释放
			(
			Poly,
			iFrontLeaf,
			iBackLeaf,
			iGeneratingNode,
			FirstPortal,
			NodePortals[iGeneratingNode],
			LeafPortals[iFrontLeaf],
			LeafPortals[iBackLeaf]
			);
			NumPortals++;

	}
}
//
// Filter a portal through a front or back subtree.
//
void FEditorVisibility::FilterThroughSubtree
(
 s32			Pass,
 s32			iGeneratingNode,
 s32			iGeneratingBase,
 s32			iParentLeaf,
 s32			iNode,
 CBspPoly		Poly,
 PORTAL_FUNC Func,
 s32			iBackLeaf
 )
{
	while( iNode != INDEX_NONE )
	{
		// If overflow.
		if( Poly.NumVertices > CBspPoly::VERTEX_THRESHOLD )
		{
			CBspPoly Half;
			Poly.SplitInHalf( &Half );
			FilterThroughSubtree( Pass, iGeneratingNode, iGeneratingBase, iParentLeaf, iNode, Half, Func, iBackLeaf );
		}
		// Test split.
		CBspPoly Front,Back;
		int Split = Poly.SplitWithNode( Model, iNode, &Front, &Back, 1 );

		// Recurse with front.
		if( Split==SP_Front || Split==SP_Split )
			FilterThroughSubtree
			(
			Pass,
			iGeneratingNode,
			iGeneratingBase,
			Model->Nodes[iNode]->iLeaf[1],
			Model->Nodes[iNode]->iFront,
			Split==SP_Front ? Poly : Front,
			Func,
			iBackLeaf
			);

		// Consider back.
		if( Split!=SP_Back && Split!=SP_Split )
			return;

		// Loop with back.
		if( Split == SP_Split )
			Poly = Back;
		iParentLeaf = Model->Nodes[iNode]->iLeaf[0];
		iNode       = Model->Nodes[iNode]->iBack;
	}

	// We reached a leaf in this subtree.
	if( Pass == 0 ) FilterThroughSubtree
		(
		1,
		iGeneratingNode,
		iGeneratingBase,
		Model->Nodes[iGeneratingBase]->iLeaf[1],
		Model->Nodes[iGeneratingBase]->iFront,
		Poly,
		Func,
		iParentLeaf
		);
	else (this->*Func)( Poly, iParentLeaf, iBackLeaf, iGeneratingNode, iGeneratingBase );
}
//
// Clip a portal by all parent nodes above it.
//
void FEditorVisibility::MakePortalsClip
(
 s32			iNode,
 CBspPoly		Poly,
 s32			Clip,
 PORTAL_FUNC Func
 )
{
	// Clip by all parents.
	while( Clip < NumClips )
	{
		s32 iClipNode = Clips[Clip] & ~CLIP_BACK_FLAG;

		// Subdivide if poly vertices overflow.
		if( Poly.NumVertices >= CBspPoly::VERTEX_THRESHOLD )
		{
			CBspPoly TempPoly;
			Poly.SplitInHalf( &TempPoly );
			MakePortalsClip( iNode, TempPoly, Clip, Func );
		}

		// Split by parent.
		CBspPoly Front,Back;
		int Split = Poly.SplitWithNode(Model,iClipNode,&Front,&Back,1);

		// Make sure we generated a useful fragment.
		if(	(Split==SP_Front &&  (Clips[Clip] & CLIP_BACK_FLAG) )
			||	(Split==SP_Back  && !(Clips[Clip] & CLIP_BACK_FLAG) )
			||	(Split==SP_Coplanar))
		{
			// Clipped to oblivion, or useless coplanar.
			return;
		}

		if( Split==SP_Split )
		{
			// Keep the appropriate piece.
			Poly = (Clips[Clip] & CLIP_BACK_FLAG) ? Back : Front;
		}

		// Clip by next parent.
		Clip++;
	}

	// Filter poly down the back subtree.
	FilterThroughSubtree
		(
		0,
		iNode,
		iNode,
		Model->Nodes[iNode]->iLeaf[0],
		Model->Nodes[iNode]->iBack,
		Poly,
		Func,
		INDEX_NONE
		);
}
//
// Make all portals.
//
void FEditorVisibility::MakePortals( s32 iNode )
{
	s32 iOriginalNode = iNode;

	// Make an infinite edpoly for this node.
	CBspPoly Poly = BuildInfiniteFPoly( Model, iNode );

	// Filter the portal through this subtree.
	MakePortalsClip( iNode, Poly, 0, AddPortal );

	//// Make portals for front.
	if( Model->Nodes[iNode]->iFront != INDEX_NONE )
	{
		Clips[NumClips++] = iNode;
		MakePortals( Model->Nodes[iNode]->iFront );
		NumClips--;
	}

	//// Make portals for back.
	if( Model->Nodes[iNode]->iBack != INDEX_NONE )
	{
		Clips[NumClips++] = iNode | CLIP_BACK_FLAG;
		MakePortals( Model->Nodes[iNode]->iBack );
		NumClips--;
	}

	//// For all zone portals at this node, mark the matching FPortals as blocked.
	while( iNode != INDEX_NONE )
	{
		FBspNode*Node = Model->Nodes[iNode];
		FBspSurf* Surf = Model->Surfs[ Node->iSurf ];

		if( (Surf->PolyFlags & PF_Portal) && GEditor->bspNodeToFPoly( Model, iNode, 	&Poly ) )
		{
			Model->PortalNodes.push_back(iNode);
			NumZonePortals++;
			iZonePortalSurf = Node->iSurf;
			FilterThroughSubtree
				(
				0,
				iNode,
				iOriginalNode,
				Model->Nodes[iOriginalNode]->iLeaf[0],
				Model->Nodes[iOriginalNode]->iBack,
				Poly,
				BlockPortal,
				INDEX_NONE
				);
		}

		iNode = Node->iPlane;
	}
}
/*-----------------------------------------------------------------------------
Zoning.
-----------------------------------------------------------------------------*/

//
// Form zones from the leaves.
//
void FEditorVisibility::FormZonesFromLeaves()
{
	//FMemMark Mark(GMem);

	// Go through all portals and merge the adjoining zones.
	for( FPortal* Portal=FirstPortal; Portal; Portal=Portal->GlobalNext )
	{
		if( Portal->iZonePortalSurf==INDEX_NONE )//!!&& Abs(Portal->Area())>10.0 )
		{
			s32 Original = Model->Leaves[Portal->iFrontLeaf]->iZone;
			s32 New      = Model->Leaves[Portal->iBackLeaf ]->iZone;
			for( s32 i=0; i<Model->Leaves.size(); i++ )
			{
				if( Model->Leaves[i]->iZone == Original )
					Model->Leaves[i]->iZone = New;
			}
		}
	}
	// Renumber the leaves.
	s32 NumZones=0;
	for( s32 i=0; i<Model->Leaves.size(); i++ )
	{
		if( Model->Leaves[i]->iZone >= NumZones )
		{
			for( int j=i+1; j<Model->Leaves.size(); j++ )
				if( Model->Leaves[j]->iZone == Model->Leaves[i]->iZone )
					Model->Leaves[j]->iZone = NumZones;

			Model->Leaves[i]->iZone  = NumZones++;
		}
	}
	//debugf( NAME_Log, TEXT("Found %i zones"), NumZones );

	// Confine the zones to 1-63.
	for( s32 i=0; i<Model->Leaves.size(); i++ )
		Model->Leaves[i]->iZone = (Model->Leaves[i]->iZone % 63) + 1;

	// Set official zone count.
	Model->NumZones = Clamp(NumZones+1,1,64);

	//Mark.Pop();
}

//
// Build 64x64 zone connectivity matrix.  Entry(i,j) is set if node i is connected
// to node j.  Entry(i,i) is always set by definition.  This structure is built by
// analyzing all portals in the world and tagging the two zones they connect.
//
// Called by: TestVisibility.
//
void FEditorVisibility::BuildConnectivity()
{
	for( int i=0; i<64; i++ )
	{
		// Init to identity.
		Model->Zones[i].Connectivity = FZoneSet::IndividualZone(i);
	}
	for( int i=0; i<Model->Nodes.size(); i++ )
	{
		// Process zones connected by portals.
		FBspNode *Node = Model->Nodes[i];
		FBspSurf *Surf = Model->Surfs[Node->iSurf];

		if( Surf->PolyFlags & PF_Portal )
		{
			Model->Zones[Node->iZone[1]].Connectivity |= FZoneSet::IndividualZone(Node->iZone[0]);
			Model->Zones[Node->iZone[0]].Connectivity |= FZoneSet::IndividualZone(Node->iZone[1]);
		}
	}
}
//
// Tag a zone portal fragment.
//
void FEditorVisibility::TagZonePortalFragment
(
 CBspPoly&	Poly,
 s32	    iFrontLeaf,
 s32		iBackLeaf,
 s32		iGeneratingNode,
 s32		iGeneratingBase
 )
{
	// Add this node to the bsp as a coplanar to its generator.
	s32 iNewNode = GEditor->bspAddNode( Model, iGeneratingNode, NODE_Plane, Model->Nodes[iGeneratingNode]->NodeFlags|NF_IsNew, &Poly );

	// Set the node's zones.
	int Backward = (Poly.Normal.dotProduct(Model->Nodes[iGeneratingBase]->Plane.Normal) )< 0.0;
	Model->Nodes[iNewNode]->iZone[Backward^0] = iBackLeaf ==INDEX_NONE ? 0 : Model->Leaves[iBackLeaf]->iZone;
	Model->Nodes[iNewNode]->iZone[Backward^1] = iFrontLeaf==INDEX_NONE ? 0 : Model->Leaves	[iFrontLeaf]->iZone;
	//“异或” ①0-∨-0=0 ②0-∨-1=1 ③1-∨-0=1 ④1-∨-1=0
}
/*-----------------------------------------------------------------------------
Assigning zone numbers.
-----------------------------------------------------------------------------*/

//
// Go through the Bsp and assign zone numbers to all nodes.  Prior to this
// function call, only leaves have zone numbers.  The zone numbers for the entire
// Bsp can be determined from leaf zone numbers.
//
void FEditorVisibility::AssignAllZones( s32 iNode, int Outside )
{
	s32 iOriginalNode = iNode;

	// Recursively assign zone numbers to children.
	if( Model->Nodes[iOriginalNode]->iFront != INDEX_NONE )
		AssignAllZones( Model->Nodes[iOriginalNode]->iFront, Outside || Model->Nodes[iOriginalNode]->IsCsg(NF_NotVisBlocking) );

	if( Model->Nodes[iOriginalNode]->iBack != INDEX_NONE )
		AssignAllZones( Model->Nodes[iOriginalNode]->iBack, Outside && !Model->Nodes[iOriginalNode]->IsCsg(NF_NotVisBlocking) );

	// Make sure this node's polygon resides in a single zone.  In other words,
	// find all of the zones belonging to outside Bsp leaves and make sure their
	// zone number is the same, and assign that zone number to this node.
	while( iNode != INDEX_NONE )
	{
		CBspPoly Poly;
		if( !(Model->Nodes[iNode]->NodeFlags & NF_IsNew) && GEditor->bspNodeToFPoly( Model, iNode, &Poly ) )
		{
			// Make sure this node is added to the BSP properly.
			int OriginalNumNodes = Model->Nodes.size();
			FilterThroughSubtree
				(
				0,
				iNode,
				iOriginalNode,
				Model->Nodes[iOriginalNode]->iLeaf [0],
				Model->Nodes[iOriginalNode]->iChild[0],
				Poly,
				TagZonePortalFragment,
				INDEX_NONE
				);

			// See if all of all non-interior added fragments are in the same zone.
			if( Model->Nodes.size() > OriginalNumNodes )
			{
				int CanMerge=1, iZone[2]={0,0};

				for( int i=OriginalNumNodes; i<Model->Nodes.size(); i++ )
					for( int j=0; j<2; j++ )
						if( Model->Nodes[i]->iZone[j] != 0 )
							iZone[j] = Model->Nodes[i]->iZone[j];

				for( int i=OriginalNumNodes; i<Model->Nodes.size(); i++ )
					for( int j=0; j<2; j++ )
						if( Model->Nodes[i]->iZone[j]!=0 && Model->Nodes[i]->iZone[j]!=iZone[j] )
							CanMerge=0;

				if(CanMerge)
				{
					// All fragments were in the same zone, so keep the 	original and discard the new fragments.
					for( int i=OriginalNumNodes; i<Model->Nodes.size(); i++ )
						Model->Nodes[i]->NumVertices = 0;
					for( int i=0; i<2; i++ )
						Model->Nodes[iNode]->iZone[i] = iZone[i];
				}
				else
				{
					// Keep the multi-zone fragments and remove the 	original plus any interior unnecessary polys.
					Model->Nodes[iNode]->NumVertices = 0;
					for( int i=OriginalNumNodes; i<Model->Nodes.size(); i++ )
						if( Model->Nodes[i]->iZone[0]==0 && Model->Nodes[i]->iZone[1]==0 )
							Model->Nodes[i]->NumVertices = 0;
				}
			}
		}
		iNode = Model->Nodes[iNode]->iPlane;
	}
}
void FEditorVisibility:: TestVisibility()
{
	for( int i=0; i<Model->Nodes.size(); i++ )
	{
		for( int j=0; j<2; j++ )
		{
			Model->Nodes[i]->iLeaf [j] = INDEX_NONE;
			Model->Nodes[i]->iZone [j] = 0;
		}
	}
	Model->Leaves.clear();
	AssignLeaves( 0, Model->RootOutside );
	int tempSize=Model->Leaves.size() ;
	LeafPortals  = new FPortal* [tempSize];	
	memset(LeafPortals,0x00,tempSize*4 );
	NodePortals  = new FPortal* [Model->Nodes.size() *2+256];
	memset(NodePortals,0x00,(Model->Nodes.size() *2+256)*4 );
	MakePortals( 0 );
	FormZonesFromLeaves();
	AssignAllZones( 0, Model->RootOutside );
	GEditor->bspCleanup( Model );
	GEditor->bspRefresh( Model, 1 );
	BuildZoneMasks( Model, 0 );
	BuildConnectivity();
}





