#pragma once

#include "vector3d.h"
#include "line3d.h"
#include "plane3d.h"
#include "aabbox3d.h"

namespace i_math
{
	
	//! 3d triangle template class for doing collision detection and other things.
	template <class T>
	class triangle3d  
	{
	public:

		//! Determinates if the triangle is totally inside a bounding box.
		//! \param box: Box to check.
		//! \return Returns true if the triangle is withing the box,
		//! and false if it is not.
		bool isTotalInsideBox(const aabbox3d<f32>& box) const
		{
			return (box.isPointInside(pointA) && 
				    box.isPointInside(pointB) &&
					box.isPointInside(pointC));
		}

		bool operator==(const triangle3d<T>& other) const { return other.pointA==pointA && other.pointB==pointB && other.pointC==pointC; }
		bool operator!=(const triangle3d<T>& other) const { return other.pointA!=pointA || other.pointB!=pointB || other.pointC!=pointC; }

		//! Returns the closest point on a triangle to a point on the same plane.
		//! \param p: Point which must be on the same plane as the triangle.
		vector3df closestPointOnTriangle(const vector3df& p) const
		{
			vector3df rab = line3d<f32>(pointA, pointB).getClosestPoint(p);
			vector3df rbc = line3d<f32>(pointB, pointC).getClosestPoint(p);
			vector3df rca = line3d<f32>(pointC, pointA).getClosestPoint(p);

			T d1 = (T)rab.getDistanceFrom(p);
			T d2 = (T)rbc.getDistanceFrom(p);
			T d3 = (T)rca.getDistanceFrom(p);

			if (d1 < d2)
				return d1 < d3 ? rab : rca;
            
			return d2 < d3 ? rbc : rca;
		}

		//Calculate the closed distance from this triangle to the given point
		//if p is in this triangle,return 0.0;
		//! \param p: Point which must be on the same plane as the triangle.
		f64 distanceToPoint(const vector3d<T> &p) const
		{
			if (isPointInside(p))
				return 0.0;
			vector3d<T> rab = line3d<T>(pointA, pointB).getClosestPoint(p);
			vector3d<T> rbc = line3d<T>(pointB, pointC).getClosestPoint(p);
			vector3d<T> rca = line3d<T>(pointC, pointA).getClosestPoint(p);

			f64 d1=rab.getDistanceFrom(p);
			f64 d2=rbc.getDistanceFrom(p);
			f64 d3=rca.getDistanceFrom(p);

			if (d1>d2)
				d1=d2;
			if (d1>d3)
				d1=d3;
			return d1;
		}

		//! Returns if a point is inside the triangle
		//! \param p: Point to test. Assumes that this point is already on the plane
		//! of the triangle.
		//! \return Returns true if the point is inside the triangle, otherwise false.
		bool isPointInside(const vector3d<T>& p) const
		{
			return (isOnSameSide(p, pointA, pointB, pointC) &&
				isOnSameSide(p, pointB, pointA, pointC) &&
				isOnSameSide(p, pointC, pointA, pointB));
		}

		//! Returns if a point is inside the triangle. This method is an implementation
		//! of the example used in a paper by Kasper Fauerby original written
		//! by Keidy from Mr-Gamemaker.
		//! \param p: Point to test. Assumes that this point is already on the plane
		//! of the triangle.
		//! \return Returns true if the point is inside the triangle, otherwise false.
		bool isPointInsideFast(const vector3d<T>& p) const
		{
			vector3d<T> f = pointB - pointA;
			vector3d<T> g = pointC - pointA;

			f32 a = f.dotProduct(f);
			f32 b = f.dotProduct(g);
			f32 c = g.dotProduct(g);

            f32 ac_bb = (a*c)-(b*b);
			vector3d<T> vp = p - pointA;

			f32 d = vp.dotProduct(f);
			f32 e = vp.dotProduct(g);
			f32 x = (d*c)-(e*b);
			f32 y = (e*a)-(d*b);
			f32 z = x+y-ac_bb;

			return (( ((u32&)z)& ~(((u32&)x)|((u32&)y))) & 0x80000000)!=0;
		}


		bool isOnSameSide(const vector3d<T>& p1, const vector3d<T>& p2, 
			const vector3d<T>& a, const vector3d<T>& b) const
		{
			vector3d<T> bminusa = b - a;
			vector3d<T> cp1 = bminusa.crossProduct(p1 - a);
			vector3d<T> cp2 = bminusa.crossProduct(p2 - a);
			return (cp1.dotProduct(cp2) >=0.0f);
		}


		//! Returns an intersection with a 3d line.
		//! \param lineVect: Vector of the line to intersect with.
		//! \param linePoint: Point of the line to intersect with.
		//! \param outIntersection: Place to store the intersection point, if there is one.
		//! \return Returns true if there was an intersection, false if there was not.
		bool getIntersectionWithLimitedLine(const line3d<T>& line,
			vector3d<T>& outIntersection) const
		{
			return getIntersectionWithLine(line.start,
				line.getVector(), outIntersection) &&
				outIntersection.isBetweenPoints(line.start, line.end);
		}


		//! Returns an intersection with a 3d line.
		//! Please note that also points are returned as intersection, which
		//! are on the line, but not between the start and end point of the line.
		//! If you want the returned point be between start and end, please
		//! use getIntersectionWithLimitedLine().
		//! \param lineVect: Vector of the line to intersect with.
		//! \param linePoint: Point of the line to intersect with.
		//! \param outIntersection: Place to store the intersection point, if there is one.
		//! \return Returns true if there was an intersection, false if there was not.
		bool getIntersectionWithLine(const vector3d<T>& linePoint,
			const vector3d<T>& lineVect, vector3d<T>& outIntersection) const
		{
			if (getIntersectionOfPlaneWithLine(linePoint, lineVect, outIntersection))
				return isPointInside(outIntersection);

			return false;			
		}

		//meaning of "safe":if the intersection is very close to the triangle,though a littile outer,we think it's in the triangle
		//by now,the safe range is 0.001
		bool getSafeIntersectionWithLine(const vector3d<T>& linePoint,
			const vector3d<T>& lineVect, vector3d<T>& outIntersection) const
		{
			if (getIntersectionOfPlaneWithLine(linePoint, lineVect, outIntersection))
			{
				if (distanceToPoint(outIntersection)<=0.001)
					return true;
				return false;
			}
			return false;			
		}

		//meaning of "safe":if the intersection is very close to the triangle,though a littile outer,we think it's in the triangle
		//by now,the safe range is 0.001
		bool getSafeIntersectionWithLimitedLine(const line3d<T>& line,
			vector3d<T>& outIntersection) const
		{
			return getSafeIntersectionWithLine(line.start,
				line.getVector(), outIntersection) &&
				outIntersection.isBetweenPoints(line.start, line.end);
		}



		//! Calculates the intersection between a 3d line and 
		//! the plane the triangle is on.
		//! \param lineVect: Vector of the line to intersect with.
		//! \param linePoint: Point of the line to intersect with.
		//! \param outIntersection: Place to store the intersection point, if there is one.
		//! \return Returns true if there was an intersection, false if there was not.
		bool getIntersectionOfPlaneWithLine(const vector3d<T>& linePoint,
			const vector3d<T>& lineVect, vector3d<T>& outIntersection) const
		{
			vector3d<T> normal = getNormal();
			T t2 = normal.dotProduct(lineVect);
            
			if (t2 == 0.0f)
				return false;

            T d = pointA.dotProduct(normal);
			T t =- (normal.dotProduct(linePoint) - d) / t2;
			outIntersection = linePoint + (lineVect * t);
			return true;
		}

		//check whether the aabb of this triangle intersects with the given aabb
		bool intersectsWithAABBbyAABB(const aabbox3d<T>& box)
		{
			aabbox3d<T> boxMe;
			boxMe.reset(pointA);
			boxMe.addInternalBox(pointB);
			boxMe.addInternalBox(pointC);
			return boxMe.intersectsWithBox(box);
		}

		bool intersectsWithAABB(const aabbox3d<T>& box) const
		{
			if (box.isPointInside(pointA)||box.isPointInside(pointB)||box.isPointInside(pointC))
				return true;
			aabbox3d<T> boxTriangle;
			boxTriangle.reset(pointA);
			boxTriangle.addInternalPoint(pointB);
			boxTriangle.addInternalPoint(pointC);

			if (!box.intersectsWithBox(boxTriangle))
				return false;

			vector3d<T> v;
			if (true)//second pass
			{
				line3d<T> lines[3];
				lines[0].setLine(pointA,pointB);
				lines[1].setLine(pointB,pointC);
				lines[2].setLine(pointC,pointA);

				for (int i=0;i<3;i++)
				{
					if (box.intersectsWithLine(lines[i]))
						return true;
				}
// 				vector3d<T> corners[8];
// 				box.getCorners(corners);
// 				triangle3d<T> triCube[12];
// 				triCube[0].set(corners[0],corners[1],corners[5]);
// 				triCube[1].set(corners[0],corners[5],corners[4]);
// 				triCube[2].set(corners[1],corners[3],corners[5]);
// 				triCube[3].set(corners[3],corners[7],corners[5]);
// 				triCube[4].set(corners[0],corners[1],corners[3]);
// 				triCube[5].set(corners[3],corners[2],corners[0]);
// 				triCube[6].set(corners[5],corners[7],corners[4]);
// 				triCube[7].set(corners[7],corners[6],corners[4]);
// 				triCube[8].set(corners[0],corners[2],corners[4]);
// 				triCube[9].set(corners[2],corners[6],corners[4]);
// 				triCube[10].set(corners[3],corners[2],corners[7]);
// 				triCube[11].set(corners[2],corners[6],corners[7]);
// 
// 				s32 i,j;
// 				for (i=0;i<3;i++)
// 				for (j=0;j<12;j++)
// 				{
// 					if (triCube[j].getSafeIntersectionWithLimitedLine(lines[i],v))
// 						return true;
// 				}
			}

			line3d<T> lines[4];
			box.getDiagonals(lines);
			s32 i;
			for (i=0;i<4;i++)
			{
				if (getSafeIntersectionWithLimitedLine(lines[i],v))
					return true;
			}
			return false;
		}

		
		//! Returns the normal of the triangle.
		//! Please note: The normal is not normalized.
		vector3d<T> getNormal() const
		{
			return (pointB - pointA).crossProduct(pointC - pointA);
		}

		//! Returns if the triangle is front of backfacing.
		//! \param lookDirection: Look direction.
		//! \return Returns true if the plane is front facing, which mean it would
		//! be visible, and false if it is backfacing.
		bool isFrontFacing(const vector3d<T>& lookDirection) const
		{
			vector3d<T> n = getNormal();
			n.normalize();
			return n.dotProduct(lookDirection) <= 0.0f;
		}

		bool isZeroArea()const
		{
			vector3d<T> d1,d2;
			d1=pointA-pointB;
			d2=pointC-pointB;
			d1.normalize();
			d2.normalize();

			if (fabsf(d1.dotProduct(d2))>=(1.0-ROUNDING_ERROR))
				return true;

			return false;
		}

		//! Returns the plane of this triangle.
		plane3d<T> getPlane() const
		{
			return plane3d<T>(pointA, pointB, pointC);
		}

		void set(const vector3d<T>& a, const vector3d<T>& b, const vector3d<T>& c)
		{
			pointA = a;
			pointB = b;
			pointC = c;
		}

		//º£Â×¹«Ê½
		float getArea()
		{
			double a,b,c,p;
			a=(pointB-pointA).getLength();
			b=(pointB-pointC).getLength();
			c=(pointA-pointC).getLength();
			p=(a+b+c)/2.0f;
			double v=p*(p-a)*(p-b)*(p-c);
			if (v>0.0)
				return (float)sqrt(v);
			return 0.0f;
		}

		//! the three points of the triangle
		vector3d<T> pointA; 
		vector3d<T> pointB; 
		vector3d<T> pointC; 
	};


	//! Typedef for a f32 3d triangle.
	typedef triangle3d<f32> triangle3df;
	//! Typedef for a f64 3d triangle.
	typedef triangle3d<f64> triangle3dd;

	//! Typedef for an integer 3d triangle.
	typedef triangle3d<s32> triangle3di;

} // end namespac i_math

