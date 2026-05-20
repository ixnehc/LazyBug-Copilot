#include "stdh.h"
#include "BspModel.h"


std::vector <std::string >   CBspModel::strBspMtrls;
/*---------------------------------------------------------------------------------------
Point searching.
---------------------------------------------------------------------------------------*/

//
// Find closest vertex to a point at or below a node in the Bsp.  If no vertices
// are closer than MinRadius, returns -1.
//
static f32 FindNearestVertex
(
 const CBspModel	&Model, 
 const vector3df	&SourcePoint,
 vector3df			&DestPoint, 
 f32			MinRadius, 
 s32				iNode, 
 s32				&pVertex
 )
{
	f32 ResultRadius = -1.f;
	while( iNode != INDEX_NONE )
	{
		const FBspNode	*Node	= Model.Nodes[iNode];
		s32			    iBack   = Node->iBack;
		//f32 PlaneDist = Node->Plane.PlaneDot( SourcePoint );  getDistanceTo
		f32 PlaneDist = Node->Plane.getDistanceTo( SourcePoint ); 
		if( PlaneDist>=-MinRadius && Node->iFront!=INDEX_NONE )
		{
			// Check front.
			f32 TempRadius = FindNearestVertex (Model,SourcePoint,DestPoint,MinRadius,Node->iFront,pVertex);
			if (TempRadius >= 0.f) {ResultRadius = TempRadius; MinRadius = TempRadius;};
		}
		if( PlaneDist>-MinRadius && PlaneDist<=MinRadius )
		{
			// Check this node's poly's vertices.
			while( iNode != INDEX_NONE )
			{
				// Loop through all coplanars.
				Node                    = Model.Nodes	[iNode];
				const FBspSurf* Surf    = Model.Surfs	[Node->iSurf];
				const vector3df  Base	    = Model.Points	[Surf->pBase].Position;
				f32 TempRadiusSquared	= FDistSquared( SourcePoint, Base );

				if( TempRadiusSquared < Square(MinRadius) )
				{
					pVertex = Surf->pBase;
					ResultRadius = MinRadius = appSqrt(TempRadiusSquared);
					DestPoint = Base;
				}

				const FVert* VertPool = &Model.Verts[Node->iVertPool];
				for (u8 B=0; B<Node->NumVertices; B++)
				{
					const vector3df  Vertex   = Model.Points[VertPool->pVertex].Position;
					f32 TempRadiusSquared = FDistSquared( SourcePoint, Vertex );
					if( TempRadiusSquared < Square(MinRadius) )
					{
						pVertex      = VertPool->pVertex;
						ResultRadius = MinRadius = appSqrt(TempRadiusSquared);
						DestPoint    = Vertex;
					}
					VertPool++;
				}
				iNode = Node->iPlane;
			}
		}
		if( PlaneDist > MinRadius )
			break;
		iNode = iBack;
	}
	return ResultRadius;
}
BOOL CBspPolys::INIPolysFromMesh( IRenderSystem *pRS,IMeshSnapshot *snapshot, IMtrl *mtrl,s32	PolyFlags)
{	
	if (!snapshot)
		return FALSE;
    CleanPloys();

	MeshSnapshotArg dmg;
	DWORD nSurf=snapshot->GetSurfCount();
	IMatrice43 *mat=pRS->GetMatrice43Mgr()->Create();
    //Vertex->IMatrice43 ID->Surf ID
	for (int i=0;i<nSurf;i++)
	{
		//设置要访问的层.
		dmg.iSurf=i;

        //激活要访问的层.
		if(!snapshot->TakeSnapshot(mat,dmg))
			 return FALSE;

         // 得到顶点格式
		 FVFEx  tempVF=snapshot->GetFVF();

         // 得到顶点数量与缓冲
		 DWORD tempVBCount=snapshot->GetVBCount();
		 DWORD tempStridelong,tempStrideshort;
		 tempVF&=~(FVFEx)FVFEX_XYZ0;//将顶点位置值排除.
		 void *tempVerticesBuf=snapshot->GetVertices(tempVF,&tempStridelong);
		 vector3df  *VerticesBufPosition=( vector3df  *)snapshot->GetVertices((FVFEx)FVFEX_XYZ0,&tempStrideshort);
	
		 //得到索引数量与缓冲
		 DWORD tempIBCount=snapshot->GetIBCount();
		 WORD   *tempIndices=snapshot->GetIndices();

		//得到变换矩阵
		 i_math::matrix43f * tempMat=mat->GetPtr();
		 
		 WORD tirNum=tempIBCount/3;
		  
		 for(int j=0;j<tirNum;j++)
		 {
			CBspPoly* tempBspPoly=new CBspPoly;
            vector3df  tempVertice;
			for(int m=0;m<3;m++)
			{
				tempMat->transformVect(VerticesBufPosition[tempIndices[j*3+m]],tempVertice);
				tempBspPoly->Vertex[m].Position=tempVertice;

				tempBspPoly->Vertex[m].buffer=new char[tempStridelong];
				tempBspPoly->Vertex[m].bufferSize=tempStridelong;
				tempBspPoly->Vertex[m].modelVF=tempVF;
				fvfCopy(1,
					tempBspPoly->Vertex[m].buffer,
					tempVF,
					&((BYTE *)tempVerticesBuf)[tempIndices[j*3+m]*tempStridelong],
					tempVF);
		 }
			// tempMat->transformVect(VerticesBufPosition[tempIndices[(j*3+1)]],tempVertice);
			//tempBspPoly->Vertex[1].Position=tempVertice;
			// tempMat->transformVect(VerticesBufPosition[tempIndices[(j*3+2)]],tempVertice);
			//tempBspPoly->Vertex[2].Position=tempVertice;
		    tempBspPoly->Normal=getNormalFrom3Points(tempBspPoly->Vertex[0].Position ,tempBspPoly->Vertex[1].Position ,tempBspPoly->Vertex[2].Position );
			tempBspPoly->NumVertices=3;
			tempBspPoly->PolyFlags=PolyFlags;

			Element.push_back(tempBspPoly);
			//((BYTE*&)q)+=stride;
		 }
	}

	SAFE_RELEASE(mat);
	//if (!snapshot)
	//	return FALSE;

	//BOOL bOk=TRUE;
	//DrawMeshArg dmg;
	//DWORD nSurf;
	//DWORD TirCount=0;

	//int idx;
	//std::string v=mtrl->GetPath();
	//VEC_FIND(CBspModel::strBspMtrls,v,idx);
	//if(idx==-1)
	//{
	//	CBspModel::strBspMtrls.push_back(v);
	//	idx=CBspModel::strBspMtrls.size()-1;
	//}


	//nSurf=snapshot->GetSurfCount();

	//for (int i=0;i<nSurf;i++)
	//{
	//	dmg.SetSurf(i);
	//	DWORD IndexCountInSurf=0;
	//	snapshot->GetIndices(dmg,IndexCountInSurf);
	//	TirCount+=IndexCountInSurf/3;
	//}

	//CleanPloys();
	//Element.resize(TirCount);

	//DWORD totGetCount=0;

	//for (int i=0;i<nSurf;i++)
	//{
	//	DWORD PosCountInSurf,IndexCountInSurf,NormalCountInSurf;
	//	vector3df *pos;
	//	vector3df *normal;
	//	WORD * index;
 //       
	//	dmg.SetSurf(i);
	//	pos=(vector3df *)snapshot->GetPos(NULL,dmg,PosCountInSurf);
	//	normal=(vector3df *)snapshot->GetNormal(NULL,dmg,NormalCountInSurf);
	//	index=(WORD *)snapshot->GetIndices(dmg,IndexCountInSurf);
	//	for(int j=0,k=0;j<IndexCountInSurf;j+=3,totGetCount++,k++)
	//	{
	//		(CBspPoly*)(Element[totGetCount])=new  CBspPoly;
	//		//((CBspPoly*)(Element[totGetCount]))->Vertex[0]=((CBspPoly*)(Element[totGetCount]))->Base=pos[index[j]];
	//		//((CBspPoly*)(Element[totGetCount]))->Vertex[1]=pos[index[(j+1)]];
	//		//((CBspPoly*)(Element[totGetCount]))->Vertex[2]=pos[index[(j+2)]];
	//		//((CBspPoly*)(Element[totGetCount]))->Normal=normal[i]; //for normal map 
	//		((CBspPoly*)(Element[totGetCount]))->Vertex[0].Position=pos[index[j]];
	//		((CBspPoly*)(Element[totGetCount]))->Vertex[1].Position=pos[index[(j+1)]];
	//		((CBspPoly*)(Element[totGetCount]))->Vertex[2].Position=pos[index[(j+2)]];
	//		((CBspPoly*)(Element[totGetCount]))->Vertex[0].Normal=normal[j]; //for normal map
	//		((CBspPoly*)(Element[totGetCount]))->Vertex[1].Normal=normal[j+1];
	//		((CBspPoly*)(Element[totGetCount]))->Vertex[2].Normal=normal[j+2];	
	//		//读入TexNum ,读入TexCoordx,TexCoordy
	//		/*for(int k=0;k<TexNum;k++)
	//			((CBspPoly*)(Element[totGetCount]))->Vertex[0].TexCoordx[k]=
	//			((CBspPoly*)(Element[totGetCount]))->Vertex[0].TexCoordy[k]=*/
	//		Element[totGetCount]->Base=Element[totGetCount]->Vertex[0];

	//		((CBspPoly*)(Element[totGetCount]))->Normal=getNormalFrom3Points(Element[totGetCount]->Vertex[0].Position ,Element[totGetCount]->Vertex[1].Position ,Element[totGetCount]->Vertex[2].Position );
	//		((CBspPoly*)(Element[totGetCount]))->NumVertices=3;
	//		((CBspPoly*)(Element[totGetCount]))->PolyFlags=PolyFlags;	

	//		Element[totGetCount]->SurflID=i;
	//		Element[totGetCount]->MaterialID=idx;
	//	}
	//}
	//return bOk;
}
void CBspPolys::AddOnePoly()
{
	CBspPoly* pBspPoly=new  CBspPoly;
	Element.push_back(pBspPoly);
}





f32 CBspModel::FindNearestVertex

(
 const vector3df	&SourcePoint,
 vector3df			&DestPoint,
 f32			MinRadius,
 s32				&pVertex
 ) 
 const
{
	return Nodes.size() ? ::FindNearestVertex( *this,SourcePoint,DestPoint,MinRadius,0,pVertex ) : -1.f;
};
//
// Empty the contents of a model.
//
void CBspModel::EmptyModel( INT EmptySurfInfo, INT EmptyPolys )
{
	for (int i=0;i<Nodes.size();i++)
	{
		SAFE_DELETE(Nodes[i]);
	}
	Nodes.resize(0);

	for (int i=0;i<Leaves.size();i++)
	{
		SAFE_DELETE(Leaves[i]);
	}
	Leaves.resize(0);

	Verts.resize(0);
	PortalNodes	.resize(0);

	if( EmptySurfInfo )
	{
		Vectors.resize(0);
		Points.resize(0);
		for (int i=0;i<Surfs.size();i++)
		{
			SAFE_DELETE(Surfs[i]);
		}
		Surfs.resize(0);
	}
	if( EmptyPolys )
	{
		SAFE_DELETE(Polys);
		
		Polys = new CBspPolys;
	}

	// Init variables.
	NumSharedSides	= 4;
	NumZones = 0;
	for( INT i=0; i<FBspNode::MAX_ZONES; i++ )
	{
		//Zones[i].ZoneActor    = NULL;
		Zones[i].Connectivity = FZoneSet::IndividualZone(i);
		Zones[i].Visibility   = FZoneSet::AllZones();
	}
}
bool CBspModel::BuildRenderSurf(IRenderSystem *pRS )
{
	//a:有Portal标志不放入VB.
	//d:MaterialID,SurflID->生成Render VB. 相同的MaterialID,SurflID->生成一个Render VB
	//c:若同一Render VB中有不同，iZone时，再切分。每个VB带一个zoneID.
	//d:该组面生成的阴影图〉2046*2046时,切分Render VB.
	//需要成员:
	//std::vector< CRenderSurf >  RenderSurfs
	//生成Mtrl
	for( INT i=0; i<Mtrls.size(); i++ )
		SAFE_RELEASE(Mtrls[i]);
	Mtrls .resize(CBspModel::strBspMtrls.size());
	for( INT i=0; i<CBspModel::strBspMtrls.size(); i++ )
		Mtrls[i]=(IMtrl*)pRS->GetMtrlMgr()->ObtainRes(CBspModel::strBspMtrls[i].c_str());

	//如果该过程过于耗时,则renderSurfPointIndices  renderSurfPoints读写硬盘.
	
	
	RenderSurfs.clear();//<----------------------------------------- SAFE_RELEASE(i_pVB);
	//遍历树. MaterialID;  SurflID; ZoneID ;只要一个不同,就作为新VB.
	//相同的MaterialID;  SurflID; ZoneID ->生成Points index序列 renderSurfPointIndices;（拆分多边形）
	//生成Points index序列renderSurfPointIndices的同时放置 vector<Points index>  renderSurfPoints  
	//遍历Point ndex序列renderSurfPointIndices，根据renderSurfPoints   整合renderSurfPointIndices
	//Point index序列  &  vector<Points index> ->生成VB.

	//遍历树. --->无需前后中递归序列遍历
	std::vector<FBspNode*>		Nodes;
	for (int i=0;i<Nodes.size();i++)
	{
		bool isFind=false;
		FBspNode  *Node     	= Nodes[i];
		FBspSurf *Poly     	= Surfs[Node->iSurf];
		FVert	 *VertPool	= &(Verts[Node->iVertPool]);

		int idx_0,idx_k;
	

        for (int j=0;j<RenderSurfs.size();j++)
		{
			VEC_FIND(RenderSurfs[j].renderSurfPoints,VertPool[0].pVertex,idx_0);
			if(idx_0==-1)
			{
				RenderSurfs[j].renderSurfPoints.push_back(VertPool[0].pVertex);
				idx_0=RenderSurfs[j].renderSurfPoints.size()-1;
			}

             if(Nodes[i]->MaterialID==RenderSurfs[j].MaterialID
				 &&Nodes[i]->SurflID==RenderSurfs[j].SurflID
				 &&Nodes[i]->iZone[1]==RenderSurfs[j].ZoneID)//<-------------------iZone[1]需确认
			 {
				//
				isFind=true;
				//判断是否三角面,是三角面直接加入,多边形则拆分后加入

				//if(Node->NumVertices>3)
				{
				//--------------------------------------------------------------------------------------------
					for (int k=1;k<Node->NumVertices-1;k++)
					{						
						RenderSurfs[j].renderSurfPointIndices.push_back(idx_0);

						if(k==1)
						{
							VEC_FIND(RenderSurfs[j].renderSurfPoints,VertPool[k].pVertex,idx_k);
							if(idx_k!=-1)
							{
								RenderSurfs[j].renderSurfPointIndices.push_back(idx_k);
							}						
							else
							{
								RenderSurfs[j].renderSurfPoints.push_back(VertPool[k+1].pVertex);
								RenderSurfs[j].renderSurfPointIndices.push_back(RenderSurfs[j].renderSurfPoints.size()-1);
							}
						}						
						else
						{
							RenderSurfs[j].renderSurfPointIndices.push_back(idx_k);					
						}

						VEC_FIND(RenderSurfs[j].renderSurfPoints,VertPool[k+1].pVertex,idx_k);
						if(idx_k!=-1)
						{
							RenderSurfs[j].renderSurfPointIndices.push_back(idx_k);
						}						
						else
						{
							RenderSurfs[j].renderSurfPoints.push_back(VertPool[k+1].pVertex);
							RenderSurfs[j].renderSurfPointIndices.push_back(RenderSurfs[j].renderSurfPoints.size()-1);
							idx_k=RenderSurfs[j].renderSurfPoints.size()-1;
						}
					
					}
				//--------------------------------------------------------------------------------------------
				}

			 }
		}
		if(!isFind)
		{
            //make new  CRenderSurf push
			CRenderSurf tempRenderSurf;
			tempRenderSurf.MaterialID=Nodes[i]->MaterialID;
			tempRenderSurf.SurflID=Nodes[i]->SurflID;
			tempRenderSurf.ZoneID=Nodes[i]->iZone[1];
			RenderSurfs.push_back(tempRenderSurf);
			int m=RenderSurfs.size()-1;

			RenderSurfs[m].renderSurfPoints.push_back(VertPool[0].pVertex);
			RenderSurfs[m].renderSurfPoints.push_back(VertPool[1].pVertex);
			//------------------------------------------------------------------------------------------------------
			for (int k=1;k<Node->NumVertices-1;k++)
			{
				RenderSurfs[m].renderSurfPointIndices.push_back(0);
				//------------------------------------------------------------------------------------
				if(k==1)
				{
					RenderSurfs[m].renderSurfPointIndices.push_back(1);				
				}						
				else
				{
					RenderSurfs[m].renderSurfPointIndices.push_back(idx_k);				
				}
				//------------------------------------------------------------------------------------
				VEC_FIND(RenderSurfs[m].renderSurfPoints,VertPool[k+1].pVertex,idx_k);
				if(idx_k!=-1)
				{
					RenderSurfs[m].renderSurfPointIndices.push_back(idx_k);
				}						
				else
				{
					RenderSurfs[m].renderSurfPoints.push_back(VertPool[k+1].pVertex);
					RenderSurfs[m].renderSurfPointIndices.push_back(RenderSurfs[m].renderSurfPoints.size()-1);
					idx_k=RenderSurfs[m].renderSurfPoints.size()-1;
				}
			}
			//------------------------------------------------------------------------------------------------------
		}
	}
	//-----------------------------------------------------------------------------------------
	//整理 Point_Old_New(,new); 应无需
	//-----------------------------------------------------------------------------------------
	//renderSurfPoints ,renderSurfPointIndices  ,Points;------>VB

	 for (int j=0;j<RenderSurfs.size();j++)
	 {
		 RenderSurfs[i].clearVB();

		 int PointTexNum =Points[RenderSurfs[j].renderSurfPoints[0]].TexNum;

		// int MaterialNum=Mtrls[RenderSurfs[i].MaterialID]->GetLayorCount(RenderSurfs[i].MaterialSurfID);

		 /*FVFEx   FVFEXTexFlag=FVFEX_FLAG_TEX0;
		 for(int m=1;m<PointTexNum;m++)
		 {
			 FVFEXTexFlag|=((FVFEx)0x1)<<(32+m);           
		 }*/

		 IVertexBuffer * pVB=pRS->GetVertexMgr()->AllocVB(RenderSurfs[j].renderSurfPoints.size()
			 ,FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_TAGENT|FVFEX_DIFFUSE
			 ,1);
		 if (!pVB)
			 return NULL;
          //(FVFEx & FVFEX_XYZ0)!=0 
		 if (!pVB->AttachIB(RenderSurfs[i].renderSurfPointIndices.size()))
		 {
			 SAFE_RELEASE(pVB);
			 return NULL;
		 }
		 if (TRUE)
		 {

			 DWORD stride;

			 float *qXYZ			=(float *)pVB->QueryVBPtr(FVFEX_XYZ0,stride,0);
			 float *qNORMAL	=(float *)pVB->QueryVBPtr(FVFEX_NORMAL0,stride,0);
			 float *qTEX0		=(float *)pVB->QueryVBPtr(FVFEX_FLAG_TEX0,stride,0);

			for (int i=0;i<RenderSurfs[i].renderSurfPoints.size();i++)
			{
					 //q[0]=i*xGap+rc.Left();//x
					 //q[1]=0;						//z
					 //q[2]=j*yGap+rc.Top();//y
					 //q[3]=0;                       //normal x
					 //q[4]=1;						//normal z
					 //q[5]=0;						//normal y

             
					 //q[4+6]=(float)i/(float)w;
					 //q[4+7]=(float)j/(float)h;

					 //((BYTE*&)qXYZ)+=stride;
					 //((BYTE*&)qNORMAL)+=stride;
					 //((BYTE*&)qTEX0)+=stride;
			}

			pVB->UnlockVB();
		 }
		// RenderSurfs[i].setVB( pVB);
	 }
	//
    //make aabb
	//-----------------------------------------------------------------------------------------
	//已知zoneID
	//在编辑器里观察，对每个renderSurf,求包裹，已知摄像机，求相交renderSurf.
}
//
// Figure out which zone a point is in, and return it.  A value of
// zero indicates that the point doesn't fall into any zone.
//

FPointRegion CBspModel::PointRegion( vector3df Location ) const
{
	//check(Zone!=NULL);

	FPointRegion Result( INDEX_NONE, 0 );
	if( Nodes.size() ) 
	{
		bool Outside=RootOutside, IsFront=0;
		INT iNode=0, iParent=0;
		while( iNode != INDEX_NONE )
		{
			const FBspNode& Node = *(Nodes[iNode]);
			IsFront = Node.Plane.dotProduct(Location) >= 0.f;
			Outside = Node.ChildOutside(IsFront,Outside);
			iParent = iNode;
			iNode   = Node.iChild[IsFront];
		}
		Result.iLeaf      = Nodes[iParent]->iLeaf[IsFront];
		Result.ZoneNumber = NumZones ? Nodes[iParent]->iZone[IsFront] : 0;
		//Result.Zone       = Zones[Result.ZoneNumber].ZoneActor ? Zones[Result.ZoneNumber].ZoneActor : Zone;
	}
	return Result;
}
bool FConvexVolume::ClipPolygon(CBspPoly& Polygon) const
{
	for(UINT PlaneIndex = 0;PlaneIndex < (u32)Planes.size();PlaneIndex++)
	{
		const plane3df&	Plane = Planes[PlaneIndex];
		//(Plane.Normal )* (Plane.D)  Point on Plane ,Turn  Normal,反转视锥面
		//Split :只要有面在,反转的视锥面后面.立即返回0.故只要与视锥交,都返回1
		//-Plane.Normal ->Normal ,(Plane.Normal )* (Plane.D)   ->base,plane(base,normal),         D=    base 点积 normal,
		if(!Polygon.Split(-Plane.Normal,Plane.Normal * Plane.D,1))
			return 0;
	}
	return 1;
}
void CBspVisibilitySet::AddVisibilityVolume(INT ZoneIndex,const FConvexVolume& Volume)
{
	static std::vector	<INT>	PortalStack;
	static std::vector<INT>	ZoneStack;
    int idx=INDEX_NONE;
	if(IgnoreZones.ContainsZone(ZoneIndex))
		return;

//	assert(ZoneIndex >= 0);
//	assert(ZoneIndex == 0 || ZoneIndex < pBspModel->NumZones);

	//GVisibilityStats.VisibilityVolumes.Value++;
	VisibleZones.AddZone(ZoneIndex);

	
	VisibilityVolumes.insert ( Int_Pair ( ZoneIndex,Volume ) );
	//VisibilityVolumes.p(ZoneIndex,Volume);

	if(UsePortals)
	{
		ZoneStack.push_back(ZoneIndex);

		// Extend visibility through the zone's portals.
		for(INT PortalIndex = 0;PortalIndex < pBspModel->PortalNodes.size();PortalIndex++)
		{
			FBspNode&	PortalNode = *(pBspModel->Nodes[pBspModel->PortalNodes[PortalIndex]]);

			// Skip the portal if it's already on the portal stack.
			// This case is equivalent to seeing the portal through itself.
			//if(PortalStack.FindItemIndex(Level->Model->PortalNodes(PortalIndex)) != INDEX_NONE)
			//	continue;
			VEC_FIND(PortalStack,pBspModel->PortalNodes[PortalIndex],idx);
			if(idx!= INDEX_NONE)
			{
				idx=INDEX_NONE;
				continue;
			}
					
			
			// Determine which side of the portal is being viewed.
			INT	ThisSide = ((PortalNode.Plane.Normal.dotProduct(ViewOrigin.Normal)  ) - PortalNode.Plane.D * ViewOrigin.D) > 0.0 ? 1 : 0,
				OtherSide = 1 - ThisSide,
				OtherZoneIndex = PortalNode.iZone[OtherSide];

			if(PortalNode.iZone[ThisSide] == ZoneIndex && OtherZoneIndex != ZoneIndex )
			{
				VEC_FIND(ZoneStack,OtherZoneIndex,idx);
				if(idx!= INDEX_NONE)
				{
					idx= INDEX_NONE;
					// Clip the portal node against the frustum 
					CBspPoly PortalPolygon;
					PortalPolygon.Init();
					for(INT VertexIndex = 0;VertexIndex < PortalNode.NumVertices;VertexIndex++)
						PortalPolygon.Vertex[VertexIndex].Position  = pBspModel->Points[pBspModel->Verts[PortalNode.iVertPool + VertexIndex].pVertex].Position;
					PortalPolygon.NumVertices = PortalNode.NumVertices;

					// Don't clip the portal polygon if the viewer is too close to get good precision.
					bool SkipPolygonClip = ViewOrigin.D >= 1.0f && Abs(PortalNode.Plane.dotProduct(ViewOrigin.Normal)) < 1.0f;

					if(SkipPolygonClip || Volume.ClipPolygon(PortalPolygon))
					{
						// Build a portal volume by extruding the portal polygon away from the view origin.
						FConvexVolume	PortalVolume;
						for(INT VertexIndex = 0;VertexIndex < PortalPolygon.NumVertices;VertexIndex++)
						{
							vector3df 	EdgeVertices[2] =
							{
								PortalPolygon.Vertex[VertexIndex].Position ,
								PortalPolygon.Vertex[(VertexIndex + 1) % PortalPolygon.NumVertices].Position 
							};

							/*new(PortalVolume.Planes) plane3df(
								EdgeVertices[0] + ((vector3df)ViewOrigin - EdgeVertices[0] * ViewOrigin.W),
								ThisSide ? EdgeVertices[0] : EdgeVertices[1],
								ThisSide ? EdgeVertices[1] : EdgeVertices[0]
								);*/
							PortalVolume.Planes.push_back(
									plane3df(
									EdgeVertices[0] + (ViewOrigin.Normal - EdgeVertices[0] * ViewOrigin.D),
									ThisSide ? EdgeVertices[0] : EdgeVertices[1],
									ThisSide ? EdgeVertices[1] : EdgeVertices[0]
									)
								);
						}
						/*new(PortalVolume.Planes) plane3df(
							ThisSide ? PortalNode.Plane : PortalNode.Plane.Flip()
							);*/
						PortalVolume.Planes.push_back(plane3df(
							ThisSide ? PortalNode.Plane : PortalNode.Plane.Flip())
							);

						//GVisibilityStats.VisiblePortals.Value++;

						// Add the volume to the visibility volumes for the zone on the other side of the portal.
						PortalStack.push_back(pBspModel->PortalNodes[PortalIndex]);
						AddVisibilityVolume(PortalNode.iZone[OtherSide],PortalVolume);
						PortalStack.pop_back();
					}
				}
			}
		}

		ZoneStack.pop_back();
	}
}