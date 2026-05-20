#pragma once

#include "iMath.h"
#include "vector2d.h"
#include "aabbox3d.h"
#include "line3d.h"
namespace i_math
{

template <class T>
class circle
{
	public:

		// Constructors

		circle()
		{
			center.set(0,0);
			radius=0;
		}

		bool operator==(const circle<T>& other) const 
		{
			return (center==other.center&& radius==other.radius);
		}


		// functions
		void set(const vector2d<T> &c,T r)
		{
			center=c;
			radius=r;
		}
		void setZero()
		{
			center.set(0,0);
			radius=0;
		}
		void setRadius(T r)
		{
			radius=r;
		}
		void setCenter(T x,T y)
		{
			center.set(x,y);
		}
		void setCenter(const vector2d<T> &c)
		{
			center=c;
		}


		void fromPoints(vector2d<T> *pts,int nPts)
		{
			setZero();
			if (nPts<=0)
				return;

			setCenter(pts[0]);
			for (int i=1;i<nPts;i++)
			{
				vector2d<T> off=(pts[i]-center);
				T d=T(off.getLengthSQ());
				if(d>radius*radius)
				{
					d=sqrtf(d);
					T r = ((T)0.5) * (d+radius);
					T scale = (r-radius) / d;
					center= center+off*scale;
					radius = r;
				}
			}
		}

// 		void fromAABB(const i_math::aabbox3d<T> &aabb)
// 		{
// 			set(aabb.getCenter(),((T)aabb.getExtent().getLength())/2.0f);
// 		}
// 
// 		void toAABB(i_math::aabbox3d<T> &aabb)
// 		{
// 			aabb.MinEdge.set(center.x-radius,center.y-radius,center.z-radius);
// 			aabb.MaxEdge.set(center.x+radius,center.y+radius,center.z+radius);
// 		}

		//update the circle's radius with the point(while keeping the center fixed)
		//FC for :Fixed Center
		void addPointFC(const vector2d<T> &v)
		{
			T length=(T)((v-center).getLength());
			if (length>radius)
				radius=length;
		}

// 		//update the circle's radius with the aabb(while keeping the center fixed)
// 		//FC for :Fixed Center
// 		void addBoxFC(const i_math::aabbox3d<T> &aabb)
// 		{
// 			vector2d<T> buf[8];
// 			aabb.getCorners(buf);
// 			for (int i=0;i<8;i++)
// 				addPointFC(buf[i]);
// 		}
		
		//! Returns if the line intersects with a shpere
		//! \param sorigin: Origin of the shpere.
		//! \param sradius: Radius if the circle.
		//! \return Returns true if there is an intersection.
		//! If there is one, the distance to the first intersection point
		//! is stored in outdistance.
		//注意:如果线段两个端点都在球体内部,返回false
		bool getIntersectionWithLine(line2d<T> &line,f64 & outdistance)
		{
			vector2d<T> q = center - line.start;
			f64 c = q.getLength();

			if (((center-line.end).getLengthSQ()<radius*radius)&&(c<=radius))
			{
				//起始点和结束点都在球内部,没有交点
				return false;
			}

			i_math::vector2d<T> dirLine=line.getVector();
			f64 distLine=dirLine.getLength();
			dirLine/=(T)distLine;

			f64 v = q.dotProduct(dirLine);
			if ((v<0.0f)&&(c>radius))
				return false;//起始点在球体外部时,并且向着远离圆心的方向射去时,没有交点
			f64 d = radius *radius - (c*c - v*v);

			if (d < 0.0) 
				return false;

			if (c>radius)
				outdistance = v - sqrt(d);
			else
				outdistance = v + sqrt(d);

			if (outdistance>distLine)
				return false;
			return true;
		}

		bool getIntersectionWithLine(line2d<T> &line,vector2d<T>&outIntersection)
		{

			f64 dist;
			if (false==getIntersectionWithLine(line,dist))
				return false;

			vector2d<T> linevect=line.getVector().normalize();
			outIntersection=line.start+linevect*(T)dist;
			return true;
		}

		bool isIntersectingWithLine(line2d<T> &line)
		{
			f64 dist;
			if (false==getIntersectionWithLine(line,dist))
				return false;
			return true;
		}

		bool isPointIn(i_math::vector2d<T> &v)
		{
			return (v-center).getLengthSQ()<radius*radius;
		}

		void snapToBoundary(i_math::vector2d<T> &v)
		{
			vector2d<T> dir=v-center;
			dir.safe_normalize();
			v=center+dir*radius;
		}

		vector2d<T> center;
		T radius;
	};

	//! Typedef for a f32 circle.
	typedef circle<f32> circlef;
} 

