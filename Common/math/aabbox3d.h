#pragma once

#include "iMath.h"
#include "plane3d.h"
#include "line3d.h"

namespace i_math
{

//! Axis aligned bounding box in 3d dimensional space.
/** Has some useful methods used with occlusion culling or clipping.
*/
template <class T>
class aabbox3d
{
	public:

		// Constructors

		aabbox3d()		{			resetInvalid();		};
		aabbox3d(const vector3d<T>& min, const vector3d<T>& max): MinEdge(min), MaxEdge(max) {};
		aabbox3d(const vector3d<T>& init): MinEdge(init), MaxEdge(init) {};
		aabbox3d(T minx, T miny, T minz, T maxx, T maxy, T maxz): MinEdge(minx, miny, minz), MaxEdge(maxx, maxy, maxz) {};

		// operators

		inline bool operator==(const aabbox3d<T>& other) const { return (MinEdge == other.MinEdge && other.MaxEdge == MaxEdge);};
		inline bool operator!=(const aabbox3d<T>& other) const { return !(MinEdge == other.MinEdge && other.MaxEdge == MaxEdge);};
		inline aabbox3d &operator&=(const aabbox3d<T>& other) 
		{ 
			if  (MinEdge.x)
			return !(MinEdge == other.MinEdge && other.MaxEdge == MaxEdge);
		};

		// functions


		//! Adds an other bounding box to the bounding box, causing it to grow bigger,
		//! if the box is outside of the box
		//! \param b: Other bounding box to add into this box.
		void addInternalBox(const aabbox3d<T>& b)
		{
			if (isInvalid())
			{
				*this=b;
				return;
			}
			if (b.MinEdge.x<MinEdge.x)
				MinEdge.x=b.MinEdge.x;
			if (b.MinEdge.y<MinEdge.y)
				MinEdge.y=b.MinEdge.y;
			if (b.MinEdge.z<MinEdge.z)
				MinEdge.z=b.MinEdge.z;

			if (b.MaxEdge.x>MaxEdge.x)
				MaxEdge.x=b.MaxEdge.x;
			if (b.MaxEdge.y>MaxEdge.y)
				MaxEdge.y=b.MaxEdge.y;
			if (b.MaxEdge.z>MaxEdge.z)
				MaxEdge.z=b.MaxEdge.z;
		}

		//! Resets the bounding box.
		void reset(T x, T y, T z)
		{
			MaxEdge.set(x,y,z);
			MinEdge = MaxEdge;
		}

		//! Resets the bounding box.
		void reset(const aabbox3d<T>& initValue)
		{
			*this = initValue;
		}

		//! Resets the bounding box.
		void reset(const vector3d<T>& initValue)
		{
			MaxEdge = initValue;
			MinEdge = initValue;
		}

		void resetInvalid()
		{
			MinEdge.set(1,1,1);
			MaxEdge.set(-1,-1,-1);
		}

		//! Adds a point to the bounding box, causing it to grow bigger, 
		//! if point is outside of the box.
		//! \param x: X Coordinate of the point to add to this box.
		//! \param y: Y Coordinate of the point to add to this box.
		//! \param z: Z Coordinate of the point to add to this box.
		void addInternalPoint(T x, T y, T z)
		{
			if (isInvalid())
			{
				reset(x,y,z);
				return;
			}
			if (x>MaxEdge.X) MaxEdge.X = x;
			if (y>MaxEdge.Y) MaxEdge.Y = y;
			if (z>MaxEdge.Z) MaxEdge.Z = z;

			if (x<MinEdge.X) MinEdge.X = x;
			if (y<MinEdge.Y) MinEdge.Y = y;
			if (z<MinEdge.Z) MinEdge.Z = z;
		}

		//! Adds a point to the bounding box, causing it to grow bigger, 
		//! if point is outside of the box
		//! \param p: Point to add into the box.
		void addInternalPoint(const vector3d<T>& p)
		{
			addInternalPoint(p.X, p.Y, p.Z);
		}

		void addInternalPoint_nocheck(T x, T y, T z)
		{
			if (x>MaxEdge.X) MaxEdge.X = x;
			if (y>MaxEdge.Y) MaxEdge.Y = y;
			if (z>MaxEdge.Z) MaxEdge.Z = z;

			if (x<MinEdge.X) MinEdge.X = x;
			if (y<MinEdge.Y) MinEdge.Y = y;
			if (z<MinEdge.Z) MinEdge.Z = z;
		}

		void addInternalPoint_nocheck(const vector3d<T>& p)
		{
			addInternalPoint_nocheck(p.x,p.y,p.z);
		}


		void inflate(T x,T y,T z)
		{
			MinEdge.X-=x;
			MinEdge.Y-=y;
			MinEdge.Z-=z;

			MaxEdge.X+=x;
			MaxEdge.Y+=y;
			MaxEdge.Z+=z;
		}

		//! Determinates if a point is within this box.
		//! \param p: Point to check.
		//! \return Returns true if the point is withing the box, and false if it is not.
		bool isPointInside(const vector3d<T>& p) const
		{
			return (	p.X >= MinEdge.X && p.X <= MaxEdge.X &&
						p.Y >= MinEdge.Y && p.Y <= MaxEdge.Y &&
						p.Z >= MinEdge.Z && p.Z <= MaxEdge.Z);
		};

		//! Determinates if a point is within this box and its borders.
		//! \param p: Point to check.
		//! \return Returns true if the point is withing the box, and false if it is not.
		bool isPointTotalInside(const vector3d<T>& p) const
		{
			return (	p.X > MinEdge.X && p.X < MaxEdge.X &&
						p.Y > MinEdge.Y && p.Y < MaxEdge.Y &&
						p.Z > MinEdge.Z && p.Z < MaxEdge.Z);
		};

		//! Determinates if the box intersects with another box.
		//! \param other: Other box to check a intersection with.
		//! \return Returns true if there is a intersection with the other box, 
		//! otherwise false.
		bool intersectsWithBox(const aabbox3d<T>& other) const
		{
			return (MinEdge <= other.MaxEdge && MaxEdge >= other.MinEdge);
		}

		//! Determinates if the box totally contains another box.
		bool containsBox(const aabbox3d<T>& other) const
		{
			return (MinEdge<=other.MinEdge&&MaxEdge>=other.MaxEdge);
		}

		//! Tests if the box intersects with a line
		//! \param line: Line to test intersection with.
		//! \return Returns true if there is an intersection and false if not.
		bool intersectsWithLine(const line3d<T>& line) const
		{
			return intersectsWithLine(line.getMiddle(), line.getVector().normalize(), 
					(T)(line.getLength() * 0.5));
		}

		//! Tests if the box intersects with a line
		//! \return Returns true if there is an intersection and false if not.
		bool intersectsWithLine(const vector3d<T>& linemiddle, 
								const vector3d<T>& linevect,
								T halflength) const
		{
			const vector3d<T> e = (MaxEdge - MinEdge) * (T)0.5;
			const vector3d<T> t = (MinEdge + e) - linemiddle;
			float r;

			if ((fabsf(t.X) > e.X + halflength * fabsf(linevect.X)) || 
				(fabsf(t.Y) > e.Y + halflength * fabsf(linevect.Y)) ||
				(fabsf(t.Z) > e.Z + halflength * fabsf(linevect.Z)) )
				return false;

			r = e.Y * (T)fabsf(linevect.Z) + e.Z * (T)fabsf(linevect.Y);
			if (fabsf(t.Y*linevect.Z - t.Z*linevect.Y) > r )
				return false;

			r = e.X * (T)fabsf(linevect.Z) + e.Z * (T)fabsf(linevect.X);
			if (fabsf(t.Z*linevect.X - t.X*linevect.Z) > r )
				return false;

			r = e.X * (T)fabsf(linevect.Y) + e.Y * (T)fabsf(linevect.X);
			if (fabsf(t.X*linevect.Y - t.Y*linevect.X) > r)
				return false;

			return true;
		}


		//! Classifies a relation with a plane.
		//! \param plane: Plane to classify relation to.
		//! \return Returns ISREL3D_FRONT if the box is in front of the plane,
		//! ISREL3D_BACK if the box is back of the plane, and
		//! ISREL3D_CLIPPED if is on both sides of the plane.
		EIntersectionRelation3D classifyPlaneRelation(const plane3d<f32>& plane) const
		{
			vector3d<T> nearPoint(MaxEdge);
			vector3d<T> farPoint(MinEdge);

			if (plane.Normal.X > (T)0)
			{
				nearPoint.X = MinEdge.X;
				farPoint.X = MaxEdge.X;
			}

			if (plane.Normal.Y > (T)0)
			{
				nearPoint.Y = MinEdge.Y;
				farPoint.Y = MaxEdge.Y;
			}

			if (plane.Normal.Z > (T)0)
			{
				nearPoint.Z = MinEdge.Z;
				farPoint.Z = MaxEdge.Z;
			}

			if (plane.Normal.dotProduct(nearPoint) - plane.D > (T)0)
				return ISREL3D_FRONT;

			if (plane.Normal.dotProduct(farPoint) - plane.D > (T)0)
				return ISREL3D_CLIPPED;

			return ISREL3D_BACK;
		}


		//! returns center of the bounding box
		vector3d<T> getCenter() const
		{
			return (MinEdge + MaxEdge) / 2;
		}

		//! returns extend of the box
		vector3d<T> getExtent() const
		{
			return MaxEdge - MinEdge;
		}
		vector3d<T> getHalfExtent()
		{
			return (MaxEdge - MinEdge)/2;
		}

		//the 6 planes are in the order:left,right,front,back,up,down
		void getPlanes(plane3d<T> *planes) const
		{
			vector3df middle = (MinEdge + MaxEdge) / 2;
			vector3df diag = middle - MaxEdge;

			vector3d<T> corners[8];

			corners[0].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[1].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[2].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[3].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z - diag.Z);
			corners[4].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[5].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[6].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[7].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z - diag.Z);

			//All these points are clockwise when looking from OUTSIDE the aabb
			planes[0].setPlane(corners[4],corners[6],corners[5]);
			planes[1].setPlane(corners[2],corners[0],corners[1]);
			planes[2].setPlane(corners[6],corners[2],corners[7]);
			planes[3].setPlane(corners[0],corners[4],corners[5]);
			planes[4].setPlane(corners[0],corners[6],corners[4]);
			planes[5].setPlane(corners[3],corners[1],corners[7]);
		}


		//! stores all 8 corners of the box into a array
		//! \param corners: Pointer to array of 8 corners
        void getCorners(vector3d<T> *corners) const
		{
			vector3df middle = (MinEdge + MaxEdge) / 2;
			vector3df diag = middle - MaxEdge;

			/*
			Corners are stored in this way:
			Hey, am I an ascii artist, or what? :) niko.
//                  /4--------/0
//                 /  |      / |
//                /   |     /  |
//                6---------2  |
//                |   5- - -| -1
//                |  /      |  /
//                |/        | /
//                7---------3/ 

//			MaMaHuHu, cxi
			*/

			corners[0].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[1].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[2].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[3].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z - diag.Z);
			corners[4].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[5].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[6].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[7].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z - diag.Z);
		}


		//! stores all 12 edges of the box into a array
		//! \param edges: Pointer to array of 12 edges
		void getEdges(line3d<T> *edges) const
		{
			vector3df middle = (MinEdge + MaxEdge) / 2;
			vector3df diag = middle - MaxEdge;

			vector3d<T> corners[8];
			corners[0].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[1].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[2].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[3].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z - diag.Z);
			corners[4].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[5].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[6].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[7].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z - diag.Z);

			edges[0].setLine(corners[0],corners[1]);
			edges[1].setLine(corners[2],corners[3]);
			edges[2].setLine(corners[4],corners[5]);
			edges[3].setLine(corners[6],corners[7]);
			edges[4].setLine(corners[1],corners[3]);
			edges[5].setLine(corners[5],corners[7]);
			edges[6].setLine(corners[0],corners[2]);
			edges[7].setLine(corners[4],corners[6]);
			edges[8].setLine(corners[1],corners[5]);
			edges[9].setLine(corners[3],corners[7]);
			edges[10].setLine(corners[0],corners[4]);
			edges[11].setLine(corners[2],corners[6]);
		}

		void getDiagonals(line3d<T> *diagonals) const
		{
			vector3df middle = (MinEdge + MaxEdge) / 2;
			vector3df diag = middle - MaxEdge;

			vector3d<T> corners[8];
			corners[0].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[1].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[2].set(middle.X + diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[3].set(middle.X + diag.X, middle.Y - diag.Y, middle.Z - diag.Z);
			corners[4].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z + diag.Z);
			corners[5].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z + diag.Z);
			corners[6].set(middle.X - diag.X, middle.Y + diag.Y, middle.Z - diag.Z);
			corners[7].set(middle.X - diag.X, middle.Y - diag.Y, middle.Z - diag.Z);

			diagonals[0].setLine(corners[6],corners[1]);
			diagonals[1].setLine(corners[0],corners[7]);
			diagonals[2].setLine(corners[4],corners[3]);
			diagonals[3].setLine(corners[2],corners[5]);
		}

		//calculate the coord where the line enter & leave the aabb
		//lineVect不需要(但最好是)normalized
		//return whether there is any intersection
		//if linePoint is in the aabb,outIntersectionEnter will be filled with it
		bool calcIntersectionWithLine(const vector3d<T>& linePoint, const vector3d<T>& lineVect,vector3d<T>& outIntersectionEnter,vector3d<T>& outIntersectionLeave)
		{
			plane3d<T> planes[6];
			getPlanes(planes);

			vector3d<T> v;
			vector3d<T> buffer[6];
			int n;
			n=0;

			//The left/right plane
			planes[0].getIntersectionWithLine(linePoint,lineVect,v);
			if ((v.Y>=MinEdge.Y)&&(v.Y<=MaxEdge.Y)&&(v.Z>=MinEdge.Z)&&(v.Z<=MaxEdge.Z)&&((v-linePoint).dotProduct(lineVect)>=0.0f))
				buffer[n++]=v;
			planes[1].getIntersectionWithLine(linePoint,lineVect,v);
			if ((v.Y>=MinEdge.Y)&&(v.Y<=MaxEdge.Y)&&(v.Z>=MinEdge.Z)&&(v.Z<=MaxEdge.Z)&&((v-linePoint).dotProduct(lineVect)>=0.0f))
				buffer[n++]=v;

			//The Front/Back plane
			planes[2].getIntersectionWithLine(linePoint,lineVect,v);
			if ((v.Y>=MinEdge.Y)&&(v.Y<=MaxEdge.Y)&&(v.X>=MinEdge.X)&&(v.X<=MaxEdge.X)&&((v-linePoint).dotProduct(lineVect)>=0.0f))
				buffer[n++]=v;
			planes[3].getIntersectionWithLine(linePoint,lineVect,v);
			if ((v.Y>=MinEdge.Y)&&(v.Y<=MaxEdge.Y)&&(v.X>=MinEdge.X)&&(v.X<=MaxEdge.X)&&((v-linePoint).dotProduct(lineVect)>=0.0f))
				buffer[n++]=v;

			//The Up/Down plane
			planes[4].getIntersectionWithLine(linePoint,lineVect,v);
			if ((v.Z>=MinEdge.Z)&&(v.Z<=MaxEdge.Z)&&(v.X>=MinEdge.X)&&(v.X<=MaxEdge.X)&&((v-linePoint).dotProduct(lineVect)>=0.0f))
				buffer[n++]=v;
			planes[5].getIntersectionWithLine(linePoint,lineVect,v);
			if ((v.Z>=MinEdge.Z)&&(v.Z<=MaxEdge.Z)&&(v.X>=MinEdge.X)&&(v.X<=MaxEdge.X)&&((v-linePoint).dotProduct(lineVect)>=0.0f))
				buffer[n++]=v;

			if (n<=0)
				return false;

			int i;
			f64 maxdistSQ,mindistSQ;
			maxdistSQ=-1.0f;
			mindistSQ=1e8;
			for (i=0;i<n;i++)
			{
				f64 distSQ;
				distSQ=(buffer[i]-linePoint).getLengthSQ();
				if (distSQ>maxdistSQ)
				{
					maxdistSQ=distSQ;
					outIntersectionLeave=buffer[i];
				}
				if (distSQ<mindistSQ)
				{
					mindistSQ=distSQ;
					outIntersectionEnter=buffer[i];
				}
			}

			if (isPointInside(linePoint))
				outIntersectionEnter=linePoint;

			return true;
		}

		bool isInvalid()
		{
			return 
				(MinEdge.x>MaxEdge.x+ROUNDING_ERROR)||
				(MinEdge.y>MaxEdge.y+ROUNDING_ERROR)||
				(MinEdge.z>MaxEdge.z+ROUNDING_ERROR);
		}

		//! returns if the box is empty, which means that there is
		//! no space within the min and the max edge.
		bool isEmpty() const
		{
			return 
				(MinEdge.x>=MaxEdge.x)&&
				(MinEdge.y>=MaxEdge.y)&&
				(MinEdge.z>=MaxEdge.z);
		}

		bool isPartialEmpty() const
		{
			return 
				(MinEdge.x>=MaxEdge.x)||
				(MinEdge.y>=MaxEdge.y)||
				(MinEdge.z>=MaxEdge.z);
		}


		//! repairs the box, if for example MinEdge and MaxEdge are swapped.
		void repair()
		{
			T t;

			if (MinEdge.X > MaxEdge.X)
				{ t=MinEdge.X; MinEdge.X = MaxEdge.X; MaxEdge.X=t; }
			if (MinEdge.Y > MaxEdge.Y)
				{ t=MinEdge.Y; MinEdge.Y = MaxEdge.Y; MaxEdge.Y=t; }
			if (MinEdge.Z > MaxEdge.Z)
				{ t=MinEdge.Z; MinEdge.Z = MaxEdge.Z; MaxEdge.Z=t; }
		}

		//! Calculates a new interpolated bounding box.
		//! \param other: other box to interpolate between
		//! \param d: value between 0.0f and 1.0f.
		aabbox3d<T> getInterpolated(const aabbox3d<T>& other, f32 d) const
		{
			f32 inv = 1.0f - d;
			return aabbox3d<T>((other.MinEdge*inv) + (MinEdge*d),
				(other.MaxEdge*inv) + (MaxEdge*d));
		}

		//split the aabb along the longest axis
		void split(i_math::aabbox3d<T>&aabb1,i_math::aabbox3d<T>&aabb2)
		{
			int longaxis=0;
			i_math::vector3df extend=getExtent();
			i_math::vector3df center=getCenter();
			if (extend.y>extend.x)
			{
				if (extend.z>extend.y)
					longaxis=2;
				else
					longaxis=1;
			}
			else
			{
				if (extend.z>extend.x)
					longaxis=2;
			}

			aabb1=aabb2=(*this);

			((float*)&aabb1.MaxEdge)[longaxis]=((float*)&center)[longaxis];
			((float*)&aabb2.MinEdge)[longaxis]=((float*)&center)[longaxis];
		}


		// member variables
		
		vector3d<T> MinEdge;
		vector3d<T> MaxEdge;
};

//! Typedef for a f64 3d bounding box.
	typedef aabbox3d<f64> aabbox3dd;
	//! Typedef for a f32 3d bounding box.
	typedef aabbox3d<f32> aabbox3df;
	//! Typedef for an integer 3d bounding box.
	typedef aabbox3d<s32> aabbox3di;


} 

