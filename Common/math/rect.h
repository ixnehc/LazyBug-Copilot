#pragma once

#include "iTypes.h"
#include "size2d.h"
#include "pos2d.h"
#include "line2d.h"
#include "vector2d.h"

#define rectiToRECT(ri,rc) (rc).left=(ri).Left();(rc).right=(ri).Right();(rc).top=(ri).Top();(rc).bottom=(ri).Bottom();

#define SIDE_LEFT 0
#define SIDE_TOP 1
#define SIDE_RIGHT 2
#define SIDE_BOTTOM 3


namespace i_math
{

	//!	Rectangle template.
	/** Mostly used by 2D GUI elements and for 2D drawing methods.
	    It has 2 positions instead of position and dimension
		and a fast method for collision detection with other rectangles and points.
	*/
	template <class T>
	class rect
	{
	public:

		rect()
			: UpperLeftCorner(0,0), LowerRightCorner(0,0) {};


		rect(T x, T y, T x2, T y2)
			: UpperLeftCorner(x,y), LowerRightCorner(x2,y2) {};


		rect(const pos2d<T>& upperLeft, const pos2d<T>& lowerRight)
			: UpperLeftCorner(upperLeft), LowerRightCorner(lowerRight) {};

		rect(const rect<T>& other)
			: UpperLeftCorner(other.UpperLeftCorner), LowerRightCorner(other.LowerRightCorner) {};

		rect(const pos2d<T>& pos, const size2d<T>& size)
			: UpperLeftCorner(pos), LowerRightCorner(pos.X + size.w, pos.Y + size.h) {};

		void set(const pos2d<T>& pos, const size2d<T>& size)
		{
			UpperLeftCorner=pos;
			LowerRightCorner.set(pos.X + size.w, pos.Y + size.h);
		}

		void set(T x, T y, T x2, T y2)
		{	
			UpperLeftCorner.X=x;
			UpperLeftCorner.Y=y;
			LowerRightCorner.X=x2;
			LowerRightCorner.Y=y2;
		}

		void inflate(T x,T y,T x2,T y2)
		{
			UpperLeftCorner.X-=x;
			UpperLeftCorner.Y-=y;
			LowerRightCorner.X+=x2;
			LowerRightCorner.Y+=y2;
		}
		void inflate(T r)
		{
			inflate(r,r,r,r);
		}


		rect<T> operator+(const pos2d<T>& pos) const
		{
			rect<T> ret(*this);
			ret.UpperLeftCorner += pos;
			ret.LowerRightCorner += pos;
			return ret;
		}

		const rect<T>& operator+=(const pos2d<T>& pos)
		{
			UpperLeftCorner += pos;
			LowerRightCorner += pos;
			return *this;
		}

		rect<T> operator-(const pos2d<T>& pos) const
		{
			rect<T> ret(*this);
			ret.UpperLeftCorner -= pos;
			ret.LowerRightCorner -= pos;
			return ret;
		}

		const rect<T>& operator-=(const pos2d<T>& pos)
		{
			UpperLeftCorner -= pos;
			LowerRightCorner -= pos;
			return *this;
		}

		rect<T> operator*(T s) const
		{
			rect<T> ret(*this);
			ret.UpperLeftCorner *= s;
			ret.LowerRightCorner *= s;
			return ret;
		}

		rect<T> operator/(T s) const
		{
			rect<T> ret(*this);
			ret.UpperLeftCorner /= s;
			ret.LowerRightCorner /= s;
			return ret;
		}



		const rect<T>& operator*=(T r)
		{
			UpperLeftCorner *= r;
			LowerRightCorner *= r;
			return *this;
		}

		const rect<T>& operator/=(T r)
		{
			UpperLeftCorner /= r;
			LowerRightCorner /= r;
			return *this;
		}

        bool operator == (const rect<T>& other) const
		{
			return UpperLeftCorner == other.UpperLeftCorner && LowerRightCorner == other.LowerRightCorner;
		}

		bool operator != (const rect<T>& other) const
		{
			return UpperLeftCorner != other.UpperLeftCorner || LowerRightCorner != other.LowerRightCorner;
		}

		const rect<T>& operator = (const rect<T>& other)
		{
			UpperLeftCorner = other.UpperLeftCorner;
			LowerRightCorner = other.LowerRightCorner;
			return *this;
		}

		//set the rect's left-top to (0,0) while keeping the size
		void zeroBase()
		{
			LowerRightCorner-=UpperLeftCorner;
			UpperLeftCorner.X=UpperLeftCorner.Y=0;
		}

		void merge(rect<T> &rcOther)
		{
			if (!isValid())
				*this=rcOther;
			else
			{
				if (Left()>rcOther.Left())
					Left()=rcOther.Left();
				if (Right()<rcOther.Right())
					Right()=rcOther.Right();
				if (Top()>rcOther.Top())
					Top()=rcOther.Top();
				if (Bottom()<rcOther.Bottom())
					Bottom()=rcOther.Bottom();
			}
		}

		void merge(pos2d<T> &ptOther)
		{
			rect<T> rcOther;
			rcOther.UpperLeftCorner=ptOther;
			rcOther.LowerRightCorner=ptOther;
			rcOther.LowerRightCorner.x++;
			rcOther.LowerRightCorner.y++;
			merge(rcOther);
		}

		void merge(T x,T y)
		{
			rect<T> rcOther;
			rcOther.UpperLeftCorner.set(x,y);
			rcOther.LowerRightCorner.set(x+1,y+1);
			merge(rcOther);
		}

		//! Returns if a 2d point is within this rectangle.
		//! \param pos: Position to test if it lies within this rectangle.
		//! \return Returns true if the position is within the rectangle, false if not.
		bool isPointInside(const pos2d<T>& pos) const
		{
			return UpperLeftCorner.X <= pos.X && UpperLeftCorner.Y <= pos.Y &&
				LowerRightCorner.X > pos.X && LowerRightCorner.Y > pos.Y;
		}

		//! Returns if a 2d point is within this rectangle.
		//! \param pos: Position to test if it lies within this rectangle.
		//! \return Returns true if the position is within the rectangle, false if not.
		bool isPointInside(const vector2d<T>& pos) const
		{
			return UpperLeftCorner.X <= pos.X && UpperLeftCorner.Y <= pos.Y &&
				LowerRightCorner.X > pos.X && LowerRightCorner.Y > pos.Y;
		}


		//! Returns if a 2d point is within this rectangle.
		//! \param pos: Position to test if it lies within this rectangle.
		//! \return Returns true if the position is within the rectangle, false if not.
		bool isPointInside(const T x,const T y) const
		{
			return UpperLeftCorner.X <= x && UpperLeftCorner.Y <= y &&
				LowerRightCorner.X > x && LowerRightCorner.Y > y;
		}

		//Check whether other is totally inside me
		bool isRectInside(const rect<T>& other)
		{
			rect<T> rc=other;
			rc.clipAgainst(*this);
			return (rc==other);
		}


		//! Returns if the rectangle collides with an other rectangle.
		bool isRectCollided(const rect<T>& other) const
		{
			return (LowerRightCorner.Y > other.UpperLeftCorner.Y && UpperLeftCorner.Y < other.LowerRightCorner.Y &&
					LowerRightCorner.X > other.UpperLeftCorner.X && UpperLeftCorner.X < other.LowerRightCorner.X);
		}

		//! Clips this rectangle with another one.
		void clipAgainst(const rect<T>& other) 
		{
			if (other.LowerRightCorner.X < LowerRightCorner.X)
				LowerRightCorner.X = other.LowerRightCorner.X;
			if (other.LowerRightCorner.Y < LowerRightCorner.Y)
				LowerRightCorner.Y = other.LowerRightCorner.Y;

			if (other.UpperLeftCorner.X > UpperLeftCorner.X)
				UpperLeftCorner.X = other.UpperLeftCorner.X;
			if (other.UpperLeftCorner.Y > UpperLeftCorner.Y)
				UpperLeftCorner.Y = other.UpperLeftCorner.Y;

			// correct possible invalid rect resulting from clipping
            if (UpperLeftCorner.Y > LowerRightCorner.Y)
                UpperLeftCorner.Y = LowerRightCorner.Y;
            if (UpperLeftCorner.X > LowerRightCorner.X)
                UpperLeftCorner.X = LowerRightCorner.X;
		}

		void clamp(vector2d<T> &pt)
		{
			if (pt.x<UpperLeftCorner.X)
				pt.x=UpperLeftCorner.X;
			if (pt.y<UpperLeftCorner.Y)
				pt.y=UpperLeftCorner.Y;
			if (pt.x>=LowerRightCorner.X)
				pt.x=LowerRightCorner.X-1;
			if (pt.y>=LowerRightCorner.Y)
				pt.y=LowerRightCorner.Y-1;
		}

		void clamp_f(vector2d<T> &pt)
		{
			if (pt.x<UpperLeftCorner.X)
				pt.x=UpperLeftCorner.X;
			if (pt.y<UpperLeftCorner.Y)
				pt.y=UpperLeftCorner.Y;
			if (pt.x>=LowerRightCorner.X)
				pt.x=LowerRightCorner.X;
			if (pt.y>=LowerRightCorner.Y)
				pt.y=LowerRightCorner.Y;
		}


		void clamp(pos2d<T> &pt)
		{
			if (pt.x<UpperLeftCorner.X)
				pt.x=UpperLeftCorner.X;
			if (pt.y<UpperLeftCorner.Y)
				pt.y=UpperLeftCorner.Y;
			if (pt.x>=LowerRightCorner.X)
				pt.x=LowerRightCorner.X-1;
			if (pt.y>=LowerRightCorner.Y)
				pt.y=LowerRightCorner.Y-1;
		}

		void clamp_f(T &x, T &y)
		{
			if (x<UpperLeftCorner.X)
				x=UpperLeftCorner.X;
			if (y<UpperLeftCorner.Y)
				y=UpperLeftCorner.Y;
			if (x>=LowerRightCorner.X)
				x=LowerRightCorner.X;
			if (y>=LowerRightCorner.Y)
				y=LowerRightCorner.Y;
		}

		//! Returns width of rectangle.
		T getWidth() const
		{
			return LowerRightCorner.X - UpperLeftCorner.X;
		}

		//! Returns height of rectangle.
		T getHeight() const
		{
			return LowerRightCorner.Y - UpperLeftCorner.Y;
		}

		//! If the lower right corner of the rect is smaller then the upper left,
		//! the points are swapped.
		void repair()
		{
			if (LowerRightCorner.X < UpperLeftCorner.X)
			{
				T t = LowerRightCorner.X;
				LowerRightCorner.X = UpperLeftCorner.X;
				UpperLeftCorner.X = t;
			}

			if (LowerRightCorner.Y < UpperLeftCorner.Y)
			{
				T t = LowerRightCorner.Y;
				LowerRightCorner.Y = UpperLeftCorner.Y;
				UpperLeftCorner.Y = t;
			}
		}

		//! Returns if the rect is valid to draw. It could be invalid, if
		//! The UpperLeftCorner is lower or more right than the LowerRightCorner,
		//! or if the area described by the rect is 0.
		bool isValid() const
		{
			// thx to jox for a correction to this method

			T xd = LowerRightCorner.X - UpperLeftCorner.X;
			T yd = LowerRightCorner.Y - UpperLeftCorner.Y;

//			return !(xd < 0 || yd < 0 || (xd == 0 && yd == 0));
			return !(xd <= 0 || yd <= 0);
		}

		//! Returns the center of the rectangle
		pos2d<T> getCenter() const
		{
			return pos2d<T>((UpperLeftCorner.X + LowerRightCorner.X) / 2,
				(UpperLeftCorner.Y + LowerRightCorner.Y) / 2);
		}

		size2d<T> getSize() const
		{
			return size2d<T>(LowerRightCorner.X-UpperLeftCorner.X,LowerRightCorner.Y-UpperLeftCorner.Y);
		}

		T getArea()
		{
			return getWidth()*getHeight();
		}

		rect<T> arrangeCenter(T w,T h)
		{
			if (((float)w/(float)h)>(float)getWidth()/(float)getHeight())
			{
				if (w>getWidth())
				{
					h=h*getWidth()/w;
					w=getWidth();
				}
			}
			else
			{
				if (h>getHeight())
				{
					w=w*getHeight()/h;
					h=getHeight();
				}
			}
			rect<T> rc;
			rc.Left()=Left()+(getWidth()-w)/2;
			rc.Top()=Top()+(getHeight()-h)/2;
			rc.Right()=rc.Left()+w;
			rc.Bottom()=rc.Top()+h;
			rc.clipAgainst(*this);
			return rc;
		}

		void scale_signed(T v)
		{
			UpperLeftCorner.X=(T)floor((double)UpperLeftCorner.X/(double)v);
			UpperLeftCorner.Y=(T)floor((double)UpperLeftCorner.Y/(double)v);
			LowerRightCorner.X=(T)floor(((double)LowerRightCorner.X+(double)v-0.0001)/(double)v);
			LowerRightCorner.Y=(T)floor(((double)LowerRightCorner.Y+(double)v-0.0001)/(double)v);
		}

		template<typename TTarget>
		rect<TTarget> convert()
		{
			rect<TTarget> t;
			t.UpperLeftCorner.set((TTarget)UpperLeftCorner.x,(TTarget)UpperLeftCorner.y);
			t.LowerRightCorner.set((TTarget)LowerRightCorner.x,(TTarget)LowerRightCorner.y);
			return t;
		}

		//cut out a rect of me from 4 sides
		//the cut out rect is stored in rc
		//side:0: left,1:top,2:right,3:bottom
		bool cutout(int side,T len,rect<T>&rc)
		{
			if (len<0)
				len=0;
			rc=*this;
			T middle;
			switch(side)
			{
				case 0:
					middle=Left()+len;
					if (middle>Right())
						middle=Right();
					Left()=middle;
					rc.Right()=middle;
					break;
				case 1:
					middle=Top()+len;
					if (middle>Bottom())
						middle=Bottom();
					Top()=middle;
					rc.Bottom()=middle;
					break;
				case 2:
					middle=Right()-len;
					if (middle<Left())
						middle=Left();
					Right()=middle;
					rc.Left()=middle;
					break;
				case 3:
					middle=Bottom()-len;
					if (middle<Top())
						middle=Top();
					Bottom()=middle;
					rc.Top()=middle;
					break;
				default:
					return false;
			}
			return true;
		}

		//得到这个rect切去一个角后剩下的部分(最多会有两个rect组成剩下的部分)
		//rcCorner必须贴在这个rect的一个角上
		int cutcorner(i_math::rect<T> &rcCorner,i_math::rect<T> *rcs)
		{
			i_math::rect<T> rc=*this;
			if (!rcCorner.isValid())
			{
				rcs[0]=rc;
				return 1;
			}
			int c=0;
			if (rcCorner.Left()==rc.Left())
			{
				if (rcCorner.Right()!=rc.Right())
					rc.cutout(2,rc.Right()-rcCorner.Right(),rcs[c++]);//切下右面一块放在rcs里
			}
			else
				rc.cutout(0,rcCorner.Left()-rc.Left(),rcs[c++]);//切下左面一块放在rcs里

			if (rcCorner.Top()==rc.Top())
			{
				if (rcCorner.Bottom()!=rc.Bottom())
					rc.cutout(3,rc.Bottom()-rcCorner.Bottom(),rcs[c++]);//切下下面一块放在rcs里
			}
			else
				rc.cutout(1,rcCorner.Top()-rc.Top(),rcs[c++]);//切下上面一块放在rcs里

			return c;
		}

		//! Tests if the box intersects with a line
		//! \param line: Line to test intersection with.
		//! \return Returns true if there is an intersection and false if not.
		bool intersectsWithLine(const line2d<T>& line) const
		{
			return intersectsWithLine(line.getMiddle(), line.getVector().normalize(), 
				(T)(line.getLength() * 0.5));
		}

		//! Tests if the box intersects with a line
		//! \return Returns true if there is an intersection and false if not.
		bool intersectsWithLine(const line2d<T>& linemiddle, 
			const vector2d<T>& linevect,
			T halflength) const
		{
			const vector2d<T> e = (LowerRightCorner - UpperLeftCorner) * (T)0.5;
			const vector2d<T> t = (UpperLeftCorner+ e) - linemiddle;
			float r;

			if ((fabsf(t.x) > e.x + halflength * fabsf(linevect.x)) || 
				(fabsf(t.y) > e.y + halflength * fabsf(linevect.y)) )
				return false;

			r = e.x * (T)fabsf(linevect.y) + e.y * (T)fabsf(linevect.x);
			if (fabsf(t.x*linevect.y - t.y*linevect.x) > r)
				return false;

			return true;
		}


		T& Left()
		{
			return UpperLeftCorner.X;
		}

		T& Right()
		{
			return LowerRightCorner.X;
		}

		T& Top()
		{
			return UpperLeftCorner.Y;
		}

		T& Bottom()
		{
			return LowerRightCorner.Y;
		}


		pos2d<T> UpperLeftCorner;
		pos2d<T> LowerRightCorner;
	};

	//! Typedef for an integer rect.
	typedef rect<s8> rect_c;
	typedef rect<u8> rect_u8;
	typedef rect<s16> rect_sh;
	typedef rect<s32> recti;
	typedef rect<f32> rectf;
	typedef rect<f64> rectd;

} // end namespace i_math




