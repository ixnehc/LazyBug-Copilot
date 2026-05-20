#include "stdafx.h"
#include "collisiontest.h"




namespace irr_base
{		
		const float fRadius = 0.50f;		
	  std::vector<triangle3df> listTriangles;			 

		double intersect(vector3df &pOrigin, vector3df& pNormal,vector3df& vSource ,vector3df &vDir)
		{
			double d = - pNormal.dotProduct(pOrigin.normalize() );
			double numer = pNormal.dotProduct( vSource.normalize() ) + d;
			double denom = pNormal.dotProduct( vDir.normalize() );
			return -(numer / denom);
		}

		inline double intersectSphere(vector3df& vOrigin,vector3df& vRay, vector3df& vCenter, double sR)
		{		
			vector3df vOriginToCenter = vCenter - vOrigin;
			double c =  vOriginToCenter.getLength();
			double v =  vOriginToCenter.dotProduct( vRay.normalize() );  // Q 在rV上的投影
			double d = sR*sR - (c*c - v*v);
	
			if (d < 0.0) return -1.0;		// If there was no intersection, return -1
			return v - sqrt(d); 
			
		} 

		const int    ctTriangles = 32;
	 
		bool collisionDetection(vector3df& vSource,vector3df& vDirection )
		{ 
				double fDistance = vDirection.getLength();
				if ( fDistance < 0.000001f) 		return false;

				vector3df vTarget = vSource + vDirection ;
				if (listTriangles.size()== 0)
				{
						vSource += vDirection ;
						return false ;
				}

				bool   firstTimeThrough = true;
				float  nearestDistance = -1.0;
				triangle3df   *nearestTriangle = NULL;
				vector3df  nearestIntersectionPoint ;
				vector3df  nearestPolygonIntersectionPoint;

				for ( int i = 0 ; i< listTriangles.size(); i++ )//ctTriangles ; i++)	
				{	
						vector3df vOrigin =  listTriangles[i].pointA;
						vector3df pVector1 = listTriangles[i].pointA;
						vector3df pVector2 = listTriangles[i].pointB;
						vector3df pVector3 = listTriangles[i].pointC;

						vector3df vNormal = (pVector2- pVector1)*( pVector3 - pVector1);
						vNormal.normalize();  // Plane Normal

						// Determine the distance from the plane to the source
						double pDist = intersect(vOrigin ,-vNormal ,vSource , vDirection);//ok 

						vector3df  sphereIntersectionPoint;
						vector3df  planeIntersectionPoint;

						vector3df directionalRadius = -vNormal * fRadius;
						double radius = directionalRadius.getLength();

						if (fabs(pDist) <= radius)// Is the plane embedded?
						{
							// Calculate the plane intersection point       
							vector3df temp = -vNormal * pDist;
							planeIntersectionPoint = vSource + temp;
						}
						else
						{
							// Calculate the ellipsoid intersection point
							vector3df temp = -vNormal* radius;
							sphereIntersectionPoint = vSource + temp;

							// Calculate the plane intersection point      
							double t = intersect(sphereIntersectionPoint, vDirection, vOrigin, vNormal);

							// Calculate the plane intersection point
							vector3df V = vDirection *t;
							planeIntersectionPoint = sphereIntersectionPoint + V;
						} 
						// Unless otherwise stated, our polygonIntersectionPoint is the
						// same point as planeIntersectionPoint

						vector3df polygonIntersectionPoint = planeIntersectionPoint;

						if ( listTriangles[i].isPointInside( planeIntersectionPoint ) )
						{
							polygonIntersectionPoint = listTriangles[i].closestPointOnTriangle( planeIntersectionPoint );
								 
						}

						// Invert the velocity vector
						vector3df negativeVelocityVector = -vDirection;

						// Using the polygonIntersectionPoint, we need to reverse-intersect with the ellipsoid				
						double t = intersectSphere( polygonIntersectionPoint, negativeVelocityVector,
																												vSource,   fRadius);

						// Was there an intersection with the ellipsoid?
						if (t >= 0.0 && t <= fDistance) 
						{
								vector3df V =vSource + negativeVelocityVector *t ; 
								vector3df intersectionPoint = polygonIntersectionPoint + V;  // Where did we intersect the ellipsoid?
								if (firstTimeThrough || t < nearestDistance)    // Closest intersection thus far?
								{
										nearestDistance = t;
										nearestTriangle = & listTriangles[i];
										nearestIntersectionPoint = intersectionPoint;
										nearestPolygonIntersectionPoint = polygonIntersectionPoint;
										firstTimeThrough = false;
								}
						}
				}	

		       
			// If we never found a collision, we can safely move to the destination and bail
				if (firstTimeThrough)
				{
					vSource += vDirection;
					return false;
				}

				// Move to the nearest collision
				vector3df V = vDirection * nearestDistance ;
				vSource += V; // 切换到新的位置(最近的碰撞点)

				// Determine the sliding plane (we do this now, because we're about to change sourcePoint)
				vector3df  slidePlaneOrigin = nearestPolygonIntersectionPoint;
				vector3df  slidePlaneNormal = nearestPolygonIntersectionPoint - vSource;

				// We now project the destination point onto the sliding plane
				double time = intersect(vTarget, slidePlaneNormal,slidePlaneOrigin, slidePlaneNormal);

				vector3df destinationProjectionNormal = slidePlaneNormal;
				vector3df newDestinationPoint = vTarget + destinationProjectionNormal;

				//// Generate the slide vector, which will become our new velocity vector for the next iteration
				vector3df newVelocityVector = newDestinationPoint.crossProduct(nearestPolygonIntersectionPoint);	
				collisionDetection(vTarget, newVelocityVector );
				return false;
		}

} // end namespace irr_base
