#pragma once
#include <math.h>
#include "iMath.h"
#include "iTypes.h"

#include "vector3d.h"

namespace i_math
{
	
	//! 3d vector template class with lots of operators and methods.
	template <class T>
	class vector4d  
	{
	public:

		vector4d(): x(0), y(0), z(0),w(0) {};
		vector4d(T nx, T ny, T nz, T nw) : x(nx), y(ny), z(nz),w(nw) {};
		vector4d(const vector4d<T>& other)	:x(other.x), y(other.y), z(other.z),w(other.w) {};

		// operators

		vector4d<T> operator-() const { return vector4d<T>(-x, -y, -z,-w);   }

		vector4d<T>& operator=(const vector4d<T>& other)	{ x = other.x; y = other.y; z = other.z; w=other.w;return *this; }

		vector4d<T> operator+(const vector4d<T>& other) const { return vector4d<T>(x + other.x, y + other.y, z + other.z,w+other.w);	}
		vector4d<T>& operator+=(const vector4d<T>& other)	{ x+=other.x; y+=other.y; z+=other.z; w+=other.w;return *this; }

		vector4d<T> operator-(const vector4d<T>& other) const { return vector4d<T>(x - other.x, y - other.y, z - other.z,w-other.w);	}
		vector4d<T>& operator-=(const vector4d<T>& other)	{ x-=other.x; y-=other.y; z-=other.z; w-=other.w;return *this; }

		vector4d<T> operator*(const vector4d<T>& other) const { return vector4d<T>(x * other.x, y * other.y, z * other.z,w*other.w);	}
		vector4d<T>& operator*=(const vector4d<T>& other)	{ x*=other.x; y*=other.y; z*=other.z;w*=other.w; return *this; }
		vector4d<T> operator*(const T v) const { return vector4d<T>(x * v, y * v, z * v,w*v);	}
		vector4d<T>& operator*=(const T v) { x*=v; y*=v; z*=v; w*=v;return *this; }

		vector4d<T> operator/(const vector4d<T>& other) const { return vector4d<T>(x / other.x, y / other.y, z / other.z,w/other.w);	}
		vector4d<T>& operator/=(const vector4d<T>& other)	{ x/=other.x; y/=other.y; z/=other.z; w/=other.w;return *this; }
		vector4d<T> operator/(const T v) const {return vector4d<T>(x/v,y/v, z/v,w/v);	}
		vector4d<T>& operator/=(const T v) {x/=v; y/=v; z/=v;w/=v; return *this; }

		bool operator<=(const vector4d<T>&other) const { return x<=other.x && y<=other.y && z<=other.z&& w<=other.w;};
		bool operator>=(const vector4d<T>&other) const { return x>=other.x && y>=other.y && z>=other.z&& w>=other.w;};

		bool operator==(const vector4d<T>& other) const { return other.x==x && other.y==y && other.z==z&&other.w==w; }
		bool operator!=(const vector4d<T>& other) const { return other.x!=x || other.y!=y || other.z!=z||other.w!=w; }

		// functions

		//! returns if this vector equals the other one, taking floating point rounding errors into account
		bool equals(const vector4d<T>& other)const
		{
			return i_math::equals(x, other.x) &&
				   i_math::equals(y, other.y) &&
				   i_math::equals(z, other.z)&&
					i_math::equals(w, other.w);
		}

		//! returns if this vector equals (0,0,0), taking floating point rounding errors into account
		bool equalsZero()const
		{
			return i_math::equals(x, 0) &&
				i_math::equals(y, 0) &&
				i_math::equals(z, 0)&&
				i_math::equals(w, 0);
		}

		bool equalsOne()const
		{
			return i_math::equals(x, 1) &&
				i_math::equals(y, 1) &&
				i_math::equals(z, 1)&&
				i_math::equals(w, 1);
		}



		void set(const T nx, const T ny, const T nz,const T nw) {x=nx; y=ny; z=nz;w=nw; }
		void set(const vector4d<T>& p) { x=p.x; y=p.y; z=p.z;w=p.w;}
		void setZero() {x=0;y=0;z=0;w=0;}

		//! Returns the dot product with another vector.
		T dotProduct(const vector4d<T>& other) const
		{
			return x*other.x + y*other.y + z*other.z+w*other.w;
		}

		void fromDwordColor(u32 c)
		{
			unsigned char *p=(unsigned char *)&c;
			b=((T)p[0])/((T)255);
			g=((T)p[1])/((T)255);
			r=((T)p[2])/((T)255);
			a=((T)p[3])/((T)255);
		}

		void toDwordColor(u32 &c)
		{
			unsigned char *p=(unsigned char *)&c;
			p[0]= i_math::clamp_u((u32)(b*255.0+0.5),0,255);
			p[1]= i_math::clamp_u((u32)(g*255.0+0.5),0,255);
			p[2]= i_math::clamp_u((u32)(r*255.0+0.5),0,255);
			p[3]= i_math::clamp_u((u32)(a*255.0+0.5),0,255);
		}


		void fromColor3d(vector3d<T> &c)
		{
			r=c.r;
			g=c.g;
			b=c.b;
			a=1;
		}


		// member variables

		union
		{
			T x; T r;
		};
		union
		{
			T y; T g;
		};
		union
		{
			T z; T b;
		};
		union
		{
			T w; T a;
		};
	};


	//! Typedef for a f32 4d vector.
	typedef vector4d<f32> vector4df;
	typedef vector4d<f32> color4df;
	//! Typedef for an integer 4d vector.
	typedef vector4d<s32> vector4di;
	typedef vector4d<u32> vector4du;
	typedef vector4d<s16> vector4ds;
	typedef vector4d<u8> vector4db;

} // end namespac i_math

