#pragma once

#include "iMath.h"
#include "vector3d.h"
#include "aabbox3d.h"
#include "line3d.h"
#include "line2d.h"
#include "capsule.h"

namespace i_math
{

template <class T>
class sphere
{
	public:

		// Constructors

		sphere()
		{
			center.set(0,0,0);
			radius=0;
		}

		bool operator==(const sphere<T>& other) const 
		{
			return (center==other.center&& radius==other.radius);
		}


		// functions
		void set(const vector3d<T> &c,T r)
		{
			center=c;
			radius=r;
		}
		void setZero()
		{
			center.set(0,0,0);
			radius=0;
		}
		void setRadius(T r)
		{
			radius=r;
		}
		void setCenter(T x,T y,T z)
		{
			center.set(x,y,z);
		}
		void setCenter(const vector3d<T> &c)
		{
			center=c;
		}


		void fromPoints(vector3d<T> *pts,int nPts)
		{
			setZero();
			if (nPts<=0)
				return;

			setCenter(pts[0]);
			for (int i=1;i<nPts;i++)
			{
				vector3d<T> off=(pts[i]-center);
				T d=T(off.getLengthSQ());
				if(d>radius*radius)
				{
					d=sqrtf(d);
					T r = ((T)0.5) * (d+radius);
					T scale = (r-radius) / d;
					center= center+scale*off;
					radius = r;
				}
			}
		}

		void fromAABB(const i_math::aabbox3d<T> &aabb)
		{
			set(aabb.getCenter(),((T)aabb.getExtent().getLength())/2.0f);
		}

		void toAABB(i_math::aabbox3d<T> &aabb)
		{
			aabb.MinEdge.set(center.x-radius,center.y-radius,center.z-radius);
			aabb.MaxEdge.set(center.x+radius,center.y+radius,center.z+radius);
		}

		//update the sphere's radius with the point(while keeping the center fixed)
		//FC for :Fixed Center
		void addPointFC(const vector3d<T> &v)
		{
			T length=(T)((v-center).getLength());
			if (length>radius)
				radius=length;
		}

		//update the sphere's radius with the aabb(while keeping the center fixed)
		//FC for :Fixed Center
		void addBoxFC(const i_math::aabbox3d<T> &aabb)
		{
			vector3d<T> buf[8];
			aabb.getCorners(buf);
			for (int i=0;i<8;i++)
				addPointFC(buf[i]);
		}
		
		//! Returns if the line intersects with a shpere
		//! \param sorigin: Origin of the shpere.
		//! \param sradius: Radius if the sphere.
		//! \return Returns true if there is an intersection.
		//! If there is one, the distance to the first intersection point
		//! is stored in outdistance.
		//注意:如果线段两个端点都在球体内部,返回false
		bool getIntersectionWithLine(line3d<T> &line,f64 & outdistance)
		{
			vector3d<T> q = center - line.start;
			f64 c = q.getLength();

			if (((center-line.end).getLengthSQ()<radius*radius)&&(c<=radius))
			{
				//起始点和结束点都在球内部,没有交点
				return false;
			}

			i_math::vector3d<T> dirLine=line.getVector();
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

		bool getIntersectionWithLine(line3d<T> &line,vector3d<T>&outIntersection)
		{

			f64 dist;
			if (false==getIntersectionWithLine(line,dist))
				return false;

			vector3d<T> linevect=line.getVector().normalize();
			outIntersection=line.start+linevect*(T)dist;
			return true;
		}

		bool testIntersectionWithLine2D(line2d<T> &line)
		{
			if (line.getDistTo(center.getXZ())<=radius)
				return true;
			return false;
		}

		bool isPointIn(i_math::vector3d<T> &v)
		{
			return (v-center).getLengthSQ()<radius*radius;
		}

		bool isPointIn(i_math::vector2d<T> &v)
		{
			return (v.x-center.x)*(v.x-center.x)+(v.y-center.z)*(v.y-center.z)<radius*radius;
		}

		T getDistanceTo(i_math::vector2d<T> &v)
		{
			T dist=sqrtf((v.x-center.x)*(v.x-center.x)+(v.y-center.z)*(v.y-center.z))-radius;
			if (dist<0.0f)
				return 0.0f;
			return dist;
		}

		i_math::vector2d<T> clip(i_math::vector2d<T> v)
		{
			if (isPointIn(v))
				return v;

			T dist=sqrtf((v.x-center.x)*(v.x-center.x)+(v.y-center.z)*(v.y-center.z));

			i_math::vector2d<T> v2;
			v2.x=center.x+(v.x-center.x)*radius/dist;
			v2.y=center.z+(v.y-center.z)*radius/dist;

			return v2;
		}

		bool intersectWithSphere(sphere<T> &other)
		{
			return center.getDistanceFromSQ(other.center)<(radius+other.radius)*(radius+other.radius);
		}

		bool intersectWithStandingCapsule(i_math::capsule<T> &other)
		{
			i_math::sphere<T> sphOther;
			sphOther.set(other.start,other.radius);
			if (intersectWithSphere(sphOther))
				return true;
			sphOther.set(other.end,other.radius);
			if (intersectWithSphere(sphOther))
				return true;
			if (center.getDistanceXZFromSQ(other.start)<(radius+other.start.radius)*(radius+other.start.radius))
			{
				if (other.end.y>other.start.y)
				{
					if (center.y<other.end.y)&&(center.y>other.start.y)
						return true;
				}
				else
				{
					if (center.y>other.end.y)&&(center.y<other.start.y)
						return true;
				}
			}
			return false;
		}

		vector3d<T> center;
		T radius;
	};

	//! Typedef for a f32 sphere.
	typedef sphere<f32> spheref;
} 

