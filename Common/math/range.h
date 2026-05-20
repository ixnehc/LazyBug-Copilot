#pragma once
#include "iTypes.h"
#include "aabbox3d.h"


namespace i_math
{

	template <class T>
	class range
	{
	public:

		range()
		{
			zero();
		}
		range(T low_,T hi_)
		{
			set(low_,hi_);
		}
		void zero()
		{
			low=(T)1;
			hi=(T)-1;
		}
		bool isEmpty()const
		{
			return low>hi;
		}
		bool isIn(T v)
		{
			return v>=low&&v<=hi;
		}
		void set(T low_,T hi_)
		{
			low=low_;
			hi=hi_;
		}
		bool limit(T &v)
		{
			bool bLimit=false;
			if (v<=low)
			{
				v=low;
				bLimit=true;
			}
			if (v>=hi)
			{
				v=hi;
				bLimit=true;
			}
			return bLimit;
		}

		void inflate(T low_,T hi_)
		{
			low-=low_;
			hi+=hi_;
		}

		T length()
		{
			return hi-low;
		}

		T getMiddle()
		{
			return (low+hi)/(T)2;
		}

		bool merge(T v)
		{
			range<T> t;
			t.set(v,v);
			return merge(t);
		}

		//-1,this is lower than r
		//0, this is intersecting r
		//1, this is higher than r
		int compare(const range<T>&r)
		{
			if (r.low>hi)
				return -1;
			if (low>r.hi)
				return 1;
			return 0;
		}

		bool merge(const range<T>&r)//return whether any modification occurs
		{
			if (r.isEmpty())
				return false;
			if (isEmpty())
			{
				(*this)=r;
				return true;
			}
			bool bMod=false;
			if (low>r.low)
			{
				low=r.low;
				bMod=TRUE;
			}
			if (hi<r.hi)
			{
				hi=r.hi;
				bMod=TRUE;
			}
			return bMod;
		}
		bool mergeY(const i_math::aabbox3d<T> &aabb)//return whether any modification occurs
		{
			range<T> r;
			r.low=aabb.MinEdge.y;
			r.hi=aabb.MaxEdge.y;
			return merge(r);
		}

		T low;
		T hi;
	};

	//! Typedef for an integer rect.
	typedef range<s16> rangesh;
	typedef range<s32> rangei;
	typedef range<f32> rangef;

} // end namespace i_math



