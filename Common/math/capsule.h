#pragma  once

#include "vector3d.h"
#include "line3d.h"

namespace i_math
{
	template<class T>
	struct capsule
	{
		void set(const vector3d<T> &s,const vector3d<T>& e,T r)
		{
			start = s;
			end = e;
			radius = r;
		}
		void setRadius(T r)
		{
			radius = r;
		}
		void setHeight(T h)
		{
			vector3d<T> dir = end - start;
			dir.normalize();
			end = start + h*dir;
		}
		void setDirection(const vector3d<T> & dir)
		{
			f64 len = start.getDistanceFrom(end);
			end = start + len*dir;
		}
		void setCenter(const vector3d<T>& c)
		{
			center = c;
		}
		bool getIntersectionWithLine(const line3d<T>& line)
		{
			bool bintersect = false;

			vector3d<T> dirLine = line.end - line.start;
			dirLine.normalize();

			vector3d<T> r0 ,r1;
			r0 = line.getClosestPoint(start);
			r1 = line.getClosestPoint(end);

			f64 d0,d1;
			d0 = r0.getDistanceFrom(start);
			d1 = r1.getDistanceFrom(end);

			if(d0 < radius||d1 < radius)
				return TRUE;

			vector3d<T> capDir = end - start;
			capDir.normalize();

			f32 f = dirLine.dotProduct(capDir);

			if(abs(f)<0.999f)
			{
				vector3d<T> normal = dirLine.crossProduct(capDir);
				plane3d<T> plane;
				plane.setPlane(line.start,normal);

				f32 dist = plane.getDistanceTo(start); //异面直线最短距离

				vector3d<T> projV0,projV1;

				projV0 = start + dist*normal;
				projV1 = end + dist*normal;

				f32 k = (line.end.y - line.start.y)/(line.end.x - line.start.x);
				f32 b = k*line.end.x - line.end.y;

				f32 f0 = projV0.y - k*projV0.x + b;
				f32 f1 = projV1.y - k*projV1.x + b;

				bool bInside = (f0>0&&f1>0)||(f0<0&&f1<0);

				if(bInside)		// 是否在直线的两侧
					return FALSE;

				if(abs(dist)<radius)
					bintersect = TRUE;
			}

			return bintersect;
		}

	protected:
		bool _calcCylinderLineIntersection(i_math::line3d<T> &line,T&dist)
		{

			//from link: http://www.gamedev.net/community/forums/topic.asp?topic_id=467789

			//--------------------------------------------------------------------------
			// Ray : P(t) = O + V * t
			// Cylinder [O, D, r].
			// point Q on cylinder if ((Q - O) x D)^2 = r^2
			//
			// Cylinder [A, B, r].
			// Point P on infinite cylinder if ((P - A) x (B - A))^2 = r^2 * (B - A)^2
			// expand : ((O - A) x (B - A) + t * (V x (B - A)))^2 = r^2 * (B - A)^2
			// equation in the form (X + t * Y)^2 = d
			// where : 
			//  X = (O - A) x (B - A)
			//  Y = V x (B - A)
			//  d = r^2 * (B - A)^2
			// expand the equation :
			// t^2 * (Y . Y) + t * (2 * (X . Y)) + (X . X) - d = 0
			// => second order equation in the form : a*t^2 + b*t + c = 0 where
			// a = (Y . Y)
			// b = 2 * (X . Y)
			// c = (X . X) - d
			//--------------------------------------------------------------------------

			//pseudo code
			// 			Vector AB = (B - A);
			// 			Vector AO = (O - A);
			// 			Vector AOxAB = (AO ^ AB); // cross product
			// 			Vector VxAB  = (V ^ AB); // cross product
			// 			float  ab2   = (AB * AB); // dot product
			// 			float a      = (VxAB * VxAB); // dot product
			// 			float b      = 2 * (VxAB * AOxAB); // dot product
			// 			float c      = (AOxAB * AOxAB) - (r*r * ab2);

			T lengthLine;

			i_math::vector3d<T> O,V;
			i_math::vector3d<T> A,B;
			T r;

			O=line.start;
			V=line.getVector();
			lengthLine=V.getLength();
			V/=lengthLine;;

			A=start;
			B=end;
			r=radius;

			i_math::vector3d<T> AB=B-A;
			i_math::vector3d<T> AO=O-A;
			i_math::vector3d<T> AOxAB=AO.crossProduct(AB);
			i_math::vector3d<T> VxAB=V.crossProduct(AB);

			T ab2=AB.dotProduct(AB);
			T a=VxAB.dotProduct(VxAB);
			T b=2*VxAB.dotProduct(AOxAB);
			T c=AOxAB.dotProduct(AOxAB)-r*r*ab2;

			T v=b*b-4*a*c;
			if (v<0)
				return false;

			v=sqrtf(v);

			T t,root1,root2;
			root1=(-b+v)/(2*a);
			root2=(-b-v)/(2*a);

			dist=(T)1000000;

			if (root1>0)
				dist=root1;
			if (root2>0)
			{
				if (root2<dist)
					dist=root2;
			}
			if (dist>=lengthLine)
				return false;

			//交点
			vector3d<T> pos=line.start+V*dist;

			//cylinder的高
			T h=(end-start).getLength();
			vector3d<T> dir=AB;
			dir/=h;

			T hPos=pos.dotProduct(dir);//投影到cylinder的轴上

			if ((hPos>h+(T)0.001)||(hPos<(T)-0.001))
				return false;//交点在cylinder两端以外

			return true;
		}

	public:
		T  radius;						// radius.
		vector3d<T> end;				// end  sphere center
		vector3d<T> start;				// start sphere center
	};
	
	typedef capsule<float> capsulef;
	typedef capsule<int>  capsulei;
};




