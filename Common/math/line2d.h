#pragma once
#include "iTypes.h"
#include "vector2d.h"

namespace i_math
{

//! 2D line between two points with intersection methods.
template <class T>
class line2d
{
	public:

		line2d(): start(0,0), end(1,1) {};
		line2d(T xa, T ya, T xb, T yb) : start(xa, ya), end(xb, yb) {};
		line2d(const vector2d<T>& start, const vector2d<T>& end) : start(start), end(end) {};
		line2d(const line2d<T>& other) :start(other.start), end(other.end) {};

		// operators

		line2d<T> operator+(const vector2d<T>& point) const { return line2d<T>(start + point, end + point); };
		line2d<T>& operator+=(const vector2d<T>& point) { start += point; end += point; return *this; };

		line2d<T> operator-(const vector2d<T>& point) const { return line2d<T>(start - point, end - point); };
		line2d<T>& operator-=(const vector2d<T>& point) { start -= point; end -= point; return *this; };

		bool operator==(const line2d<T>& other) const { return (start==other.start && end==other.end) || (end==other.start && start==other.end);};
		bool operator!=(const line2d<T>& other) const { return !(start==other.start && end==other.end) || (end==other.start && start==other.end);};

		// functions

		void setLine(const T& xa, const T& ya, const T& xb, const T& yb){start.set(xa, ya); end.set(xb, yb);}
		void setLine(const vector2d<T>& nstart, const vector2d<T>& nend){start.set(nstart); end.set(nend);}
		void setLine(const line2d<T>& line){start.set(line.start); end.set(line.end);}

		//! Returns length of line
		//! \return Returns length of line.
		f64 getLength() const { return start.getDistanceFrom(end); };

		//! Returns the vector of the line.
		//! \return Returns the vector of the line.
		vector2d<T> getVector() const { return vector2d<T>(start.X - end.X, start.Y - end.Y); };

		vector2d<T> getMiddle() const { return vector2d<T>((start.X + end.X)/2.0f, (start.Y + end.Y)/2.0f); };

		//判断other(这条线段)与自己所在的直线是否相交,如果相交,rate里返回一个start到end的比率值
		bool getIntersection(const line2d<T>& other, T &rate)
		{
			T denom = ((other.end.y - other.start.y)*(end.x - start.x)) -
				((other.end.x - other.start.x)*(end.y - start.y));

			T nume_a = ((other.end.x - other.start.x)*(start.y - other.start.y)) -
				((other.end.y - other.start.y)*(start.x - other.start.x));

			T nume_b = ((end.x - start.x)*(start.y - other.start.y)) -
				((end.y - start.y)*(start.x - other.start.x));

			if(denom == 0.0f)
			{
				return false;
				//				if(nume_a == 0.0f && nume_b == 0.0f)
				//					return COINCIDENT;
				//				return PARALLEL;
			}

			T ua = nume_a / denom;
			T ub = nume_b / denom;

			if((ub >= 0.0f) && (ub <= 1.0f))
			{
				rate=ua;
				return true;
			}
			return false;
		}

		//判断两条线段是否相交,如果相交,out里返回交点
		//目前共线当作不相交
		bool getIntersectionPoint(const line2d<T>& other, vector2d<T>& out)
		{
			T rate;
			if (!getIntersection(other,rate))
				return false;

			if( (rate>= 0.0f) && (rate<= 1.0f) )
			{
				out=start+(end-start)*rate;
				return true;
			}
			return false;
		}


		//! Returns unit vector of the line.
		//! \return Returns unit vector of this line.
		vector2d<T> getUnitVector()
		{
			T len = (T)1.0 / (T)getLength();
			return vector2d<T>((end.X - start.X) * len, (end.Y - start.Y) * len);
		}

		f64 getAngleWith(const line2d<T>& l)
		{
			vector2d<T> vect = getVector();
			vector2d<T> vect2 = l.getVector();
			return vectorAngle(vect.X, vect.Y, vect2.X, vect2.Y);
		}

		//返回投影点在这条线段上的比例,(如果投影在这条线段上的话,返回值为0..1之间)
		void getProjection(const vector2d<T>& point,T &rate) const
		{
			vector2d<T> c = point - start;
			vector2d<T> v = end - start;
			T d = (T)v.getLength();
			v /= d;
			rate= v.dotProduct(c);
			rate/=d;
		}

		//get the projection on this line for the point
		void getProjectionPoint(const vector2d<T>& point,vector2d<T>&pointProj) const
		{
			T rate;
			getProjection(point,rate);
			pointProj=start + (end-start)*rate;
		}

		//! Returns the closest point on this line to a point
		vector2d<T> getClosestPoint(const vector2d<T>& point) const
		{
			vector2d<T> c = point - start;
			vector2d<T> v = end - start;
			T d = (T)v.getLength();
			v /= d;
			T t = v.dotProduct(c);

			if (t < (T)0.0) return start;
			if (t > d) return end;

			v *= t;
			return start + v;
		}

		bool isPointOnMe(const vector2d<T>& point) const
		{
			vector2d<T> c = point - start;
			vector2d<T> v = end - start;
			T d = (T)v.getLength();
			v /= d;
			T t = v.dotProduct(c);

			if (t < (T)0.0) return false;
			if (t > d) return false;

			v*=t;
			v=start+v;
			if ((v-point).getLengthSQ()>ROUNDING_ERROR*ROUNDING_ERROR)
				return false;
			return true;
		}

		T getDistTo(const vector2d<T>& point) const
		{
			vector2d<T> v;
			getProjectionPoint(point,v);
			return point.getDistanceFrom(v);
		}

		T getClosestDistTo(const vector2d<T>& point) const
		{
			vector2d<T> v;
			v=getClosestPoint(point);
			return point.getDistanceFrom(v);
		}


		//返回:1:在正面,0:在线上,-1:在反面,
		//所谓正面是指当你从start向end移动时,正面在你的右侧
		int classifyPoint(const vector2d<T> &point) const
		{
			T d=(point.x-start.x)*(end.y-start.y)-(point.y-start.y)*(end.x-start.x);
			if (d>0.0)
				return 1;
			if (d<0.0)
				return -1;
			return 0;
		}

		
		// member variables
		
		vector2d<T> start;
		vector2d<T> end;
};

typedef line2d<f32> line2df;
typedef line2d<f64> line2dd;
typedef line2d<s32> line2di;


}

