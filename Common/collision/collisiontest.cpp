#include "stdh.h"
#include "collisiontest.h"

#include "../math/line2d.h"

//2D 版本

f32 intersectSphere2D(i_math::vector2df& vOrigin,i_math::vector2df& vRay, i_math::vector2df& vCenter, f32 sR)
{		
	i_math::vector2df vOriginToCenter = vCenter - vOrigin;
	f32 c =  (f32)vOriginToCenter.getLength();
	if (c<sR)
		return -1.0;//The vOrigin is within the sphere
	f32 v =  vOriginToCenter.dotProduct( vRay.normalize() );  // Q 在rV上的投影
	if (v<0)
		return -1.0;//the sphere is on the back side of the ray
	double d = sR*sR - (c*c - v*v);

	if (d < 0.0) return -1.0;		// If there was no intersection, return -1
	return v - (f32)sqrt(d); 
} 

//由一个顶点(corner)和若干个经过这个顶点的直线(的正面区域)可以构成一个空间区域,这个函数为vPos在这个空间区域里寻找离它最近的
//点
bool FindNearestPointAgainstCornerSegs(i_math::vector2df &vPos,i_math::vector2df &vNearest,i_math::vector2df &vCorner,i_math::line2df *segs,int nSegs)
{
	int i,j;

	for (i=0;i<nSegs;i++)
	{
		if (segs[i].classifyPoint(vPos)<0)
			break;
	}

	if (i>=nSegs)
	{
		vNearest=vPos;
		return true;
	}

	bool bFound=false;
	f32 distSQNearest=1e7;
	i_math::vector2df vProj;
	for (i=0;i<nSegs;i++)
	{
		//得到vPos在某个seg上的投影
		segs[i].getProjectionPoint(vPos,vProj);

		//检查这个投影点是否在其它所有seg的正面
		for (j=0;j<nSegs;j++)
		{
			if (i==j)
				continue;
			if (segs[j].classifyPoint(vProj)<0)
				break;
		}

		if (j>=nSegs)
		{
			f32 distSQ=(f32)(vProj-vPos).getLengthSQ();
			if (distSQ<distSQNearest)
			{
				vNearest=vProj;
				distSQNearest=distSQ;
				bFound=true;
			}
		}
	}

	if (bFound)
		return true;

	
	vNearest=vCorner;
	return true;
}




bool collisionDetection2D(f32 radius,i_math::vector2df& vSrc,i_math::vector2df& vDir,i_math::line2df*segs,int nSegs,int depth)
{ 
	static int ignores[256];
	static int nIgnores=0;
	static int aEmbedded[128];//记录segs内的索引

	depth--;
	if (depth<0)
		return false;
	double fDist = vDir.getLength();
	if ( fDist < 0.000001f) 		return false;

	i_math::vector2df vTarget = vSrc + vDir ;
	if (nSegs<= 0)
	{
		vSrc += vDir ;
		return false ;
	}

	f32 radiusOrg=radius;//Back up it
	bool   firstTimeThrough = true;
	float  nearestDist= -1.0;
	int iNearestSeg = -1;
	i_math::vector2df  nearesetIntersect ;
	i_math::vector2df  nearestSegIntersect;

	int nEmbedded=0;

	for ( int i = 0 ; i< nSegs; i++ )
	{	
		if (true)//Whether this seg is ignored
		{
			int j;
			for (j=0;j<nIgnores;j++)
			{
				if (ignores[j]==i)
					break;
			}
			if (j<nIgnores)//in ignore list
				continue;
		}

		//判断运动的方向是否朝向这个seg(从正面向反面运动),如果是的话,略过这个seg
		if (true)
		{
			i_math::vector2df t=segs[i].start+vDir;
			if (segs[i].classifyPoint(t)>0)
				continue;//移向正面,忽略这个seg
		}

		radius=radiusOrg;


		float rate;
		segs[i].getProjection(vSrc,rate);
		i_math::vector2df vProj;
		vProj=segs[i].start+(segs[i].end-segs[i].start)*rate;
		f32 dist=(f32)vProj.getDistanceFrom(vSrc);// vSrc到这个seg的距离

		i_math::vector2df  sphIntersect;//圆上的碰撞点
		i_math::vector2df  segIntersect;//线段上的碰撞点

		bool bEmbedded=false;


		if (dist <= radius+0.005f)// 圆心到seg的距离<小于半径
		{
			bEmbedded=true;
			if (radius<dist+0.005f)
				radius=dist+0.005f;//Ensure it's embedded
		}
		else
		{
			sphIntersect=vSrc+(vProj-vSrc)/dist*radius;//圆上离线段最近的一点

			
			if (false==segs[i].getIntersection(i_math::line2df(sphIntersect,sphIntersect+vDir),
																rate))
				continue;//最近的一点移动vDir后,仍然没有碰到seg所在的直线
		} 

		rate=i_math::clamp_f(rate,0.0f,1.0f);
		segIntersect =segs[i].start+(segs[i].end-segs[i].start)*rate;

		//原来圆与seg所在直线相交,所以判断为embed,但有可能圆与seg并没有相交,这里进一步检查一下
		if (bEmbedded)
		if (!((segIntersect-vSrc).getLength()<radiusOrg+0.0001))
			bEmbedded=false;

		if (bEmbedded)
		{
			//把嵌入的seg记录下来
			if (nEmbedded<ARRAY_SIZE(aEmbedded))
			{
				aEmbedded[nEmbedded]=i;
				nEmbedded++;
				firstTimeThrough = false;
			}
		}

		
		if (nEmbedded>0)
			continue;//如果已经发现有嵌入了,我们就不要考虑碰撞后的滑动了

		// Invert the velocity vector
		i_math::vector2df vDirNeg = -vDir;

		//从碰撞点出发,反向移动vDir,看是否与圆相交
		f32 t = intersectSphere2D(segIntersect,vDirNeg,vSrc, radius);

		//相交的话,我们确认找到了一个确实会碰撞的点,我们要把所有碰撞的点中离圆最近的那个记录下来
		if (t >= 0.0 && t <= fDist) 
		{
			i_math::vector2df V =vDirNeg;
			V.normalize();
			V*=t; 
			i_math::vector2df intersectionPoint = segIntersect + V;  // Where did we intersect the sphere?
			if (firstTimeThrough || t < nearestDist)    // Closest intersection thus far?
			{
				nearestDist= t;
				iNearestSeg =i;
				nearesetIntersect = intersectionPoint;
				nearestSegIntersect = segIntersect;
				firstTimeThrough = false;
			}
		}
	}	


	// If we never found a collision, we can safely move to the destination and bail
	if (firstTimeThrough)
	{
		vSrc+= vDir;
		return false;
	}

	i_math::vector2df newVelocityVector;
	if (nEmbedded>0)
	{
		//如果有嵌入,我们尝试着尽量移到新的位置,但是保证不会嵌得更深
		static i_math::line2df aEmbeddedSeg[128];
		i_math::vector2df vTargetNew;
		for (int i=0;i<nEmbedded;i++)
		{
			i_math::line2df &seg=segs[aEmbedded[i]];
			aEmbeddedSeg[i].start=vSrc;
			aEmbeddedSeg[i].end=vSrc+(seg.end-seg.start);
		}
		FindNearestPointAgainstCornerSegs(vTarget,vTargetNew,vSrc,aEmbeddedSeg,nEmbedded);
		newVelocityVector=vTargetNew-vSrc;

		memcpy(ignores,aEmbedded,nEmbedded*sizeof(ignores[0]));
		nIgnores=nEmbedded;
	}
	else
	{

		// Move to the nearest collision
		i_math::vector2df V = vDir;
		V.normalize();
		V*=nearestDist;
		vSrc += V; // 先移到碰撞点

		//判断向那个方向上滑动(沿着vSrc和碰撞点连线的垂直方向滑动)
		i_math::line2df segSliding;
		segSliding.start=nearestSegIntersect;
		segSliding.end.x=nearestSegIntersect.x-(vSrc.y-nearestSegIntersect.y);
		segSliding.end.y=nearestSegIntersect.y+(vSrc.x-nearestSegIntersect.x);

		i_math::vector2df IntersectionPointTarget;//the target point for the nearesetIntersect
		IntersectionPointTarget=nearesetIntersect+vDir;
		//the IntersectionPointTarget's projected point on segSliding 
		i_math::vector2df newDestinationPoint;
		segSliding.getProjectionPoint(IntersectionPointTarget,newDestinationPoint);

		//计算出滑动的速度
		newVelocityVector = newDestinationPoint-nearestSegIntersect;	
//		ignores[0]=iNearestSeg;
//		nIgnores=1;
		nIgnores=0;
	}

	//向着滑动方向再进行一次test
	collisionDetection2D(radiusOrg,vSrc, newVelocityVector,segs,nSegs,depth);
	nIgnores=0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//3D版本


inline f32 intersectSphere(i_math::vector3df& vOrigin,i_math::vector3df& vRay, i_math::vector3df& vCenter, f32 sR)
{		
	i_math::vector3df vOriginToCenter = vCenter - vOrigin;
	f32 c =  (f32)vOriginToCenter.getLength();
	if (c<sR)
		return -1.0;//The vOrigin is within the sphere
	f32 v =  vOriginToCenter.dotProduct( vRay.normalize() );  // Q 在rV上的投影
	if (v<0)
		return -1.0;//the sphere is on the back side of the ray
	double d = sR*sR - (c*c - v*v);

	if (d < 0.0) return -1.0;		// If there was no intersection, return -1
	return v - (f32)sqrt(d); 

} 

//NOTE:the order of the planes in pPlanes will be changed after calling this function
bool FindNearestPointAgainstCornerPlanes(i_math::vector3df &vPos,i_math::vector3df &vNearest,i_math::vector3df &vCorner,i_math::plane3df *pPlanes,int nPlanes)
{
	int i,j,k;

	//combine the nearly co-planar planes
	if (TRUE)
	{
		i_math::vector3df linePoint,lineVect;
		i_math::plane3df temp;
		for (i=0;i<nPlanes;i++)
		for (j=i+1;j<nPlanes;j++)
		{
			if (true==pPlanes[i].getIntersectionWithPlane(pPlanes[j],linePoint,lineVect))
			{
				if (lineVect.getLengthSQ()>=0.000001)
					continue;
			}
			//j-plane is nearly the same as i-plane,remove it from the plane list
			temp=pPlanes[nPlanes-1];
			pPlanes[nPlanes-1]=pPlanes[j];
			pPlanes[j]=temp;
			j--;
			nPlanes--;
		}
	}


	for (i=0;i<nPlanes;i++)
	{
		if (pPlanes[i].classifyPointRelation(vPos)==ISREL3D_BACK)
			break;
	}

	if (i>=nPlanes)
	{
		vNearest=vPos;
		return true;
	}

	bool bFound;
	bFound=false;
	f32 distSQNearest;
	distSQNearest=1e7;
	i_math::vector3df vProj;
	for (i=0;i<nPlanes;i++)
	{
		pPlanes[i].getProjectionOf(vPos,vProj);
		for (j=0;j<nPlanes;j++)
		{
			if (i==j)
				continue;
			if (pPlanes[j].classifyPointRelation(vProj)==ISREL3D_BACK)
				break;
		}

		if (j>=nPlanes)
		{
			f32 distSQ;
			distSQ=(f32)(vProj-vPos).getLengthSQ();
			if (distSQ<distSQNearest)
			{
				vNearest=vProj;
				distSQNearest=distSQ;
				bFound=true;
			}
		}
	}

	if (bFound)
		return true;


	i_math::vector3df linePoint,lineVect;
	line3d<f32> line;
	for (i=0;i<nPlanes;i++)
	for (j=i+1;j<nPlanes;j++)
	{
		if (false==pPlanes[i].getIntersectionWithPlane(pPlanes[j],linePoint,lineVect))
			continue;
		if (lineVect.getLengthSQ()<0.000001)
			continue;
		line.setLine(linePoint,linePoint+lineVect);

		line.getProjectionPoint(vPos,vProj);

		for (k=0;k<nPlanes;k++)
		{
			if ((k==i)||(k==j))
				continue;
			if (pPlanes[k].classifyPointRelation(vProj)==ISREL3D_BACK)
				break;
		}

		if (k>=nPlanes)
		{
			f32 distSQ;
			distSQ=(f32)(vProj-vPos).getLengthSQ();
			if (distSQ<distSQNearest)
			{
				vNearest=vProj;
				distSQNearest=distSQ;
				bFound=true;
			}
		}
	}

	if (bFound)
		return true;

	vNearest=vCorner;
	return true;
}



bool collisionDetection(f32 radius,i_math::vector3df& vSrc,i_math::vector3df& vDir,i_math::triangle3df *pTris,int nTris,int iRecursiveDepth)
{ 
	static int s_aIgnoreTris[256];
	static int s_nIgnoreTris=0;
	iRecursiveDepth--;
	if (iRecursiveDepth<0)
		return false;
	double fDistance = vDir.getLength();
	if ( fDistance < 0.000001f) 		return false;

	i_math::vector3df vTarget = vSrc + vDir ;
	if (nTris<= 0)
	{
		vSrc += vDir ;
		return false ;
	}

	f32 radiusOrg;
	radiusOrg=radius;//Back up it
	bool   firstTimeThrough = true;
	float  nearestDistance = -1.0;
	int idxNearestTriangle = -1;
	i_math::vector3df  nearestIntersectionPoint ;
	i_math::vector3df  nearestPolygonIntersectionPoint;


	int aEmbeddedTris[128];
	i_math::plane3df aEmbeddedPlanes[128];//Big enough
	int nEmbedded;
	nEmbedded=0;

	for ( int i = 0 ; i< nTris; i++ )
	{	
		if (true)//Whether this tri is ignored
		{
			int j;
			for (j=0;j<s_nIgnoreTris;j++)
			{
				if (s_aIgnoreTris[j]==i)
					break;
			}
			if (j<s_nIgnoreTris)//in ignore list
				continue;
		}
		radius=radiusOrg;
		i_math::plane3df planeTri(pTris[i].pointA,pTris[i].pointB,pTris[i].pointC);

		f32 dist;
		dist=planeTri.getDistanceTo(vSrc);// Determine the distance from the plane to the source

		i_math::vector3df  sphereIntersectionPoint;//plane on the sphere
		i_math::vector3df  planeIntersectionPoint;//point on the plane

		bool bEmbedded;
		bEmbedded=false;

		if (fabsf(dist) <= radius+0.005f)// Is the plane embedded?
		{
			// Calculate the plane intersection point       
			planeIntersectionPoint = vSrc + (-planeTri.Normal* dist);
			bEmbedded=true;
			if (radius<fabsf(dist)+0.005f)
				radius=fabsf(dist)+0.005f;//Ensure it's embedded
		}
		else
		{
			// Calculate the sphere intersection point
			if (dist>=0.0)
				sphereIntersectionPoint = vSrc + (-planeTri.Normal* radius);
			else
				sphereIntersectionPoint = vSrc + (planeTri.Normal* radius);

			if (false==planeTri.getIntersectionWithLimitedLine(sphereIntersectionPoint,
						sphereIntersectionPoint+vDir,planeIntersectionPoint))
				continue;
		} 

		// Unless otherwise stated, our polygonIntersectionPoint is the
		// same point as planeIntersectionPoint
		i_math::vector3df polygonIntersectionPoint = planeIntersectionPoint;

		if ( !pTris[i].isPointInside( planeIntersectionPoint ) )
		{
			polygonIntersectionPoint = pTris[i].closestPointOnTriangle(planeIntersectionPoint);

			if ((polygonIntersectionPoint-vSrc).getLength()<radiusOrg+0.0001)
				bEmbedded=true;
			else
				bEmbedded=false;
		}

		if (bEmbedded)
		{
			if (nEmbedded<sizeof(aEmbeddedPlanes)/sizeof(aEmbeddedPlanes[0]))
			{
				aEmbeddedPlanes[nEmbedded].setPlane(vSrc,vSrc-polygonIntersectionPoint);
				aEmbeddedTris[nEmbedded]=i;
				nEmbedded++;
				firstTimeThrough = false;
			}
		}

		//if some plane is embedded, we should no longer process those not embedded
		if (nEmbedded>0)
			continue;

		// Invert the velocity vector
		i_math::vector3df vDirNeg = -vDir;

		// Using the polygonIntersectionPoint, we need to reverse-intersect with the sphere				
		f32 t = intersectSphere(polygonIntersectionPoint,vDirNeg,vSrc, radiusOrg);

		// Was there an intersection with the sphere?
		if (t >= 0.0 && t <= fDistance) 
		{
			i_math::vector3df V =vDirNeg;
			V.normalize();
			V*=t; 
			i_math::vector3df intersectionPoint = polygonIntersectionPoint + V;  // Where did we intersect the sphere?
			if (firstTimeThrough || t < nearestDistance)    // Closest intersection thus far?
			{
				nearestDistance = t;
				idxNearestTriangle =i;
				nearestIntersectionPoint = intersectionPoint;
				nearestPolygonIntersectionPoint = polygonIntersectionPoint;
				firstTimeThrough = false;
			}
		}
	}	


	// If we never found a collision, we can safely move to the destination and bail
	if (firstTimeThrough)
	{
		i_math::vector3df vSrc2;
		vSrc2=vSrc;
		vSrc2 += vDir;

		//for testing ,check whether the new position will make the sphere collide into some plane
		if (false)
		{
			int i;
			for (i=0;i<nTris;i++)
			{
				i_math::plane3df plane(pTris[i].pointA,pTris[i].pointB,pTris[i].pointC);

				f32 dist;
				dist=plane.getDistanceTo(vSrc2);// Determine the distance from the plane to the source

				if (fabsf(dist)<radiusOrg-0.01)
				{
					i_math::vector3df v;
					v= vSrc2 + (-plane.Normal* dist);
					if ( pTris[i].isPointInside( v ) )
					{
						int dd;
						dd=0;
					}
				}
			}
		}

		vSrc=vSrc2;

		return false;
	}

	i_math::vector3df newVelocityVector;
	if (nEmbedded>0)
	{
		i_math::vector3df vTargetNew;
		FindNearestPointAgainstCornerPlanes(vTarget,vTargetNew,vSrc,aEmbeddedPlanes,nEmbedded);
		newVelocityVector=vTargetNew-vSrc;

		memcpy(s_aIgnoreTris,aEmbeddedTris,nEmbedded*sizeof(s_aIgnoreTris[0]));
		s_nIgnoreTris=nEmbedded;
	}
	else
	{

		// Move to the nearest collision
		i_math::vector3df V = vDir;
		V.normalize();
		V*=nearestDistance ;
		vSrc += V; // 切换到新的位置(最近的碰撞点)


		if (false)
		{
			int i;
			for (i=0;i<nTris;i++)
			{
				i_math::plane3df plane(pTris[i].pointA,pTris[i].pointB,pTris[i].pointC);

				f32 dist;
				dist=plane.getDistanceTo(vSrc);// Determine the distance from the plane to the source

				if (fabsf(dist)<radiusOrg-0.01)
				{
					i_math::vector3df v;
					v= vSrc + (-plane.Normal* dist);
					if ( pTris[i].isPointInside( v ) )
					{
						int dd;
						dd=0;
					}
				}
			}
		}
		// Determine the sliding plane (we do this now, because we're about to change sourcePoint)
		i_math::plane3df planeSliding(nearestPolygonIntersectionPoint,(nearestPolygonIntersectionPoint - vSrc).normalize());

		i_math::vector3df IntersectionPointTarget;//the target point for the nearestIntersectionPoint
		IntersectionPointTarget=nearestIntersectionPoint+vDir;
		//the IntersectionPointTarget's projected point on planeSliding 
		i_math::vector3df newDestinationPoint=IntersectionPointTarget+
			(-planeSliding.Normal * planeSliding.getDistanceTo(IntersectionPointTarget));

		//// Generate the slide vector, which will become our new velocity vector for the next iteration
		newVelocityVector = newDestinationPoint-nearestPolygonIntersectionPoint;	
//		s_aIgnoreTris[0]=idxNearestTriangle;
//		s_nIgnoreTris=1;
		s_nIgnoreTris=0;
	}
	collisionDetection(radiusOrg,vSrc, newVelocityVector,pTris,nTris,iRecursiveDepth);
	s_nIgnoreTris=0;
	return true;
}


static bool CheckStandableByNextMove(const i_math::vector3df &vNextMove)
{
	const f64 Standable_Threshold=1.0f;

	f64 lengthXZ;
	lengthXZ=vNextMove.getLengthXZ();
	f64 lengthY;
	lengthY=fabsf(vNextMove.Y);
	if (lengthXZ<=0.0001)
	{
		if (lengthY<=0.0001)
			return true;
		return false;
	}

	if (lengthY/lengthXZ<Standable_Threshold)
		return true;

	return false;
}

//Collide and stop at the first met standable triangle( or plane)
//return whether meet a standable triangle( or plane)
bool collisionDetection_Land(f32 radius,i_math::vector3df& vSrc,i_math::vector3df& vDir,i_math::triangle3df *pTris,BYTE *pFlags,int nTris,int iRecursiveDepth)
{ 
	static int s_aIgnoreTris[256];
	static int s_nIgnoreTris=0;
	iRecursiveDepth--;
	if (iRecursiveDepth<0)
		return false;
	double fDistance = vDir.getLength();
	if ( fDistance < 0.000001f) 		return false;

	i_math::vector3df vTarget = vSrc + vDir ;
	if (nTris<= 0)
	{
		vSrc += vDir ;
		return false ;
	}

	f32 radiusOrg;
	radiusOrg=radius;//Back up it
	bool   firstTimeThrough = true;
	float  nearestDistance = -1.0;
	int idxNearestTriangle = -1;
	i_math::vector3df  nearestIntersectionPoint ;
	i_math::vector3df  nearestPolygonIntersectionPoint;


	int aEmbeddedTris[128];
	i_math::plane3df aEmbeddedPlanes[128];//Big enough
	int nEmbedded;
	nEmbedded=0;

	for ( int i = 0 ; i< nTris; i++ )
	{	
		if (true)//Whether this tri is ignored
		{
			int j;
			for (j=0;j<s_nIgnoreTris;j++)
			{
				if (s_aIgnoreTris[j]==i)
					break;
			}
			if (j<s_nIgnoreTris)//in ignore list
				continue;
		}
		radius=radiusOrg;
		i_math::plane3df planeTri(pTris[i].pointA,pTris[i].pointB,pTris[i].pointC);

		f32 dist;
		dist=planeTri.getDistanceTo(vSrc);// Determine the distance from the plane to the source

		i_math::vector3df  sphereIntersectionPoint;//plane on the sphere
		i_math::vector3df  planeIntersectionPoint;//point on the plane

		bool bEmbedded;
		bEmbedded=false;

		if (fabsf(dist) <= radius+0.005f)// Is the plane embedded?
		{
			// Calculate the plane intersection point       
			planeIntersectionPoint = vSrc + (-planeTri.Normal* dist);
			bEmbedded=true;
			if (radius<fabsf(dist)+0.005f)
				radius=fabsf(dist)+0.005f;//Ensure it's embedded
		}
		else
		{
			// Calculate the sphere intersection point
			if (dist>=0.0)
				sphereIntersectionPoint = vSrc + (-planeTri.Normal* radius);
			else
				sphereIntersectionPoint = vSrc + (planeTri.Normal* radius);

			if (false==planeTri.getIntersectionWithLimitedLine(sphereIntersectionPoint,
				sphereIntersectionPoint+vDir,planeIntersectionPoint))
				continue;
		} 

		// Unless otherwise stated, our polygonIntersectionPoint is the
		// same point as planeIntersectionPoint
		i_math::vector3df polygonIntersectionPoint = planeIntersectionPoint;

		if ( !pTris[i].isPointInside( planeIntersectionPoint ) )
		{
			polygonIntersectionPoint = pTris[i].closestPointOnTriangle(planeIntersectionPoint);

			if ((polygonIntersectionPoint-vSrc).getLength()<radiusOrg+0.001)
				bEmbedded=true;
			else
				bEmbedded=false;
		}

		if (bEmbedded&&(pFlags[i]&1))
			return true;

		if (bEmbedded)
		{
			if (nEmbedded<sizeof(aEmbeddedPlanes)/sizeof(aEmbeddedPlanes[0]))
			{
				aEmbeddedPlanes[nEmbedded].setPlane(vSrc,vSrc-polygonIntersectionPoint);
				aEmbeddedTris[nEmbedded]=i;
				nEmbedded++;
				firstTimeThrough = false;
			}
		}

		//if some plane is embedded, we should no longer process those not embedded
		if (nEmbedded>0)
			continue;

		// Invert the velocity vector
		i_math::vector3df vDirNeg = -vDir;

		// Using the polygonIntersectionPoint, we need to reverse-intersect with the sphere				
		f32 t = intersectSphere(polygonIntersectionPoint,vDirNeg,vSrc, radiusOrg);

		// Was there an intersection with the sphere?
		if (t >= 0.0 && t <= fDistance) 
		{
			i_math::vector3df V =vDirNeg;
			V.normalize();
			V*=t; 
			i_math::vector3df intersectionPoint = polygonIntersectionPoint + V;  // Where did we intersect the sphere?
			if (firstTimeThrough || t < nearestDistance)    // Closest intersection thus far?
			{
				nearestDistance = t;
				idxNearestTriangle =i;
				nearestIntersectionPoint = intersectionPoint;
				nearestPolygonIntersectionPoint = polygonIntersectionPoint;
				firstTimeThrough = false;
			}
		}
	}	


	// If we never found a collision, we can safely move to the destination and bail
	if (firstTimeThrough)
	{
		i_math::vector3df vSrc2;
		vSrc2=vSrc;
		vSrc2 += vDir;
		vSrc=vSrc2;

		return false;
	}

	i_math::vector3df newVelocityVector;
	if (nEmbedded>0)
	{
		i_math::vector3df vTargetNew;
		FindNearestPointAgainstCornerPlanes(vTarget,vTargetNew,vSrc,aEmbeddedPlanes,nEmbedded);
		newVelocityVector=vTargetNew-vSrc;

		if (CheckStandableByNextMove(newVelocityVector))
			return true;

		memcpy(s_aIgnoreTris,aEmbeddedTris,nEmbedded*sizeof(s_aIgnoreTris[0]));
		s_nIgnoreTris=nEmbedded;
	}
	else
	{

		// Move to the nearest collision
		i_math::vector3df V = vDir;
		V.normalize();
		V*=nearestDistance ;
		vSrc += V; // 切换到新的位置(最近的碰撞点)

		if (pFlags[idxNearestTriangle]&1)
			return true;

		// Determine the sliding plane (we do this now, because we're about to change sourcePoint)
		i_math::plane3df planeSliding(nearestPolygonIntersectionPoint,(nearestPolygonIntersectionPoint - vSrc).normalize());

		i_math::vector3df IntersectionPointTarget;//the target point for the nearestIntersectionPoint
		IntersectionPointTarget=nearestIntersectionPoint+vDir;
		//the IntersectionPointTarget's projected point on planeSliding 
		i_math::vector3df newDestinationPoint=IntersectionPointTarget+
			(-planeSliding.Normal * planeSliding.getDistanceTo(IntersectionPointTarget));

		//// Generate the slide vector, which will become our new velocity vector for the next iteration
		newVelocityVector = newDestinationPoint-nearestPolygonIntersectionPoint;	

		if (CheckStandableByNextMove(newVelocityVector))
			return true;

		s_nIgnoreTris=0;
	}
	if (collisionDetection_Land(radiusOrg,vSrc, newVelocityVector,pTris,pFlags,nTris,iRecursiveDepth))
	{
		s_nIgnoreTris=0;
		return true;
	}
	s_nIgnoreTris=0;
	return false;
}

