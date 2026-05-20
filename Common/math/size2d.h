#pragma once
#include "iTypes.h"

namespace i_math
{

	//! Specifies a 2 dimensional size.
	template <class T>
	class size2d
	{
		public:

			size2d()
				: w(0), h(0) {};

			size2d(T width, T height)
				: w(width), h(height) {};

			size2d(const size2d<T>& other)
				: w(other.w), h(other.h) {};

			void set(T width,T height)
			{
				w=width;
				h=height;
			}


			bool operator == (const size2d<T>& other) const
			{
				return w == other.w && h == other.h;
			}


			bool operator != (const size2d<T>& other) const
			{
				return w != other.w || h != other.h;
			}

			size2d<T>& operator=(const size2d<T>& other) 
			{
				w = other.w;
				h = other.h;
				return *this;
			}

			size2d<T> operator+(const size2d<T>&other) const
			{
				size2d<T> ret(*this);
				ret+=other;
				return ret;
			}

			size2d<T>& operator+=(const size2d<T>&other)
			{
				w+=other.w;
				h+=other.h;
				return *this;
			}

			size2d<T> operator-(const size2d<T>&other) const
			{
				size2d<T> ret(*this);
				ret-=other;
				return ret;
			}

			size2d<T>& operator-=(const size2d<T>&other)
			{
				w-=other.w;
				h-=other.h;
				return *this;
			}

			size2d<T> operator*(T s) const
			{
				size2d<T> ret(*this);
				ret*=s;
				return ret;
			}

			size2d<T> operator/(T s) const
			{
				size2d<T> ret(*this);
				ret/=s;
				return ret;
			}
			size2d<T>& operator*=(T r)
			{
				w*=r;
				h*=r;
				return *this;
			}

			size2d<T>& operator/=(T r)
			{
				w/=r;
				h/=r;
				return *this;
			}

			size2d<T> operator*(const size2d<T>&other) const
			{
				size2d<T> ret(*this);
				ret*=other;
				return ret;
			}

			size2d<T> operator/(const size2d<T>&other) const
			{
				size2d<T> ret(*this);
				ret/=other;
				return ret;
			}
			size2d<T>& operator*=(const size2d<T>&other)
			{
				w*=other.w;
				h*=other.h;
				return *this;
			}

			size2d<T>& operator/=(const size2d<T>&other)
			{
				w/=other.w;
				h/=other.h;
				return *this;
			}



			bool isEmpty()const
			{
				return (w<=0)&&(h<=0);
			}

			T getArea()const
			{
				return w*h;
			}

			void inflate(T dw,T dh)
			{
				w+=dw;
				h+=dh;
			}

			T w, h;
	};

	typedef size2d<f32> size2df;
	typedef size2d<s32> size2di;
	typedef size2d<s16> size2d_sh;
	typedef size2d<u8> size2db;

}

