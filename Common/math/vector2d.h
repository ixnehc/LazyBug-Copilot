#pragma once

#include "iMath.h"

namespace i_math
{


//! 2d vector template class with lots of operators and methods.
template <class T>
class vector2d
{
public:

	vector2d(): X(0), Y(0) {};
	vector2d(T nx, T ny) : X(nx), Y(ny) {};
	vector2d(const vector2d<T>& other)	:X(other.X), Y(other.Y) {};

	// operators

	vector2d<T> operator-() const { return vector2d<T>(-X, -Y);   }

	vector2d<T>& operator=(const vector2d<T>& other)	{ X = other.X; Y = other.Y; return *this; }

	vector2d<T> operator+(const vector2d<T>& other) const { return vector2d<T>(X + other.X, Y + other.Y);	}
	vector2d<T>& operator+=(const vector2d<T>& other)	{ X+=other.X; Y+=other.Y; return *this; }

	vector2d<T> operator-(const vector2d<T>& other) const { return vector2d<T>(X - other.X, Y - other.Y);	}
	vector2d<T>& operator-=(const vector2d<T>& other)	{ X-=other.X; Y-=other.Y; return *this; }

	vector2d<T> operator*(const vector2d<T>& other) const { return vector2d<T>(X * other.X, Y * other.Y);	}
	vector2d<T>& operator*=(const vector2d<T>& other)	{ X*=other.X; Y*=other.Y; return *this; }
	vector2d<T> operator*(const T v) const { return vector2d<T>(X * v, Y * v);	}
	vector2d<T>& operator*=(const T v) { X*=v; Y*=v; return *this; }

	vector2d<T> operator/(const vector2d<T>& other) const { return vector2d<T>(X / other.X, Y / other.Y);	}
	vector2d<T>& operator/=(const vector2d<T>& other)	{ X/=other.X; Y/=other.Y; return *this; }
	vector2d<T> operator/(const T v) const { return vector2d<T>(X / v, Y / v);	}
	vector2d<T>& operator/=(const T v) { X/=v; Y/=v; return *this; }

	bool operator==(const vector2d<T>& other) const { return other.X==X && other.Y==Y; }
	bool operator!=(const vector2d<T>& other) const { return other.X!=X || other.Y!=Y; }

	// functions

	//! returns if this vector equals the other one, taking floating point rounding errors into account
	bool equals(const vector2d<T>& other)const
	{
		return i_math::equals(X, other.X) &&
			i_math::equals(Y, other.Y);
	}

	bool equals(const vector2d<T>& other,T epsilon)const
	{
		return i_math::equals(X, other.X,epsilon) &&
			i_math::equals(Y, other.Y,epsilon);
	}


	//! returns if this vector equals (0,0), taking floating point rounding errors into account
	bool equalsZero()const
	{
		return i_math::equals(X, 0) &&
			i_math::equals(Y, 0);
	}


	void set(const T& nx, const T& ny) {X=nx; Y=ny; }
	void set(const vector2d<T>& p) { X=p.X; Y=p.Y;}

	//! Returns the length of the vector
	//! \return Returns the length of the vector.
	f32 getLength() const { return sqrtf((f32)(X*X + Y*Y)); }
	T getLengthSQ() const	{ return ((T)(X*X + Y*Y)); }

	//! Returns the dot product of this vector with an other.
	T dotProduct(const vector2d<T>& other) const
	{
		return X*other.X + Y*other.Y;
	}



	//! Returns distance from an other point. Here, the vector is interpreted as
	//! point in 2 dimensional space.
	float getDistanceFrom(const vector2d<T>& other) const
	{
		f64 vx = X - other.X; f64 vy = Y - other.Y;
		return (float)sqrt(vx*vx + vy*vy);
	}

	float getDistanceSQFrom(const vector2d<T>& other) const
	{
		float vx = X - other.X; float vy = Y - other.Y;
		return (vx*vx + vy*vy);
	}

	//! rotates the point around a center by an amount of degrees.
	void rotateBy(f64 degrees, const vector2d<T>& center)
	{
		degrees *= GRAD_PI2;
		T cs = (T)cos(degrees);
		T sn = (T)sin(degrees);

		X -= center.X;
		Y -= center.Y;

		set(X*cs - Y*sn, X*sn + Y*cs);

		X += center.X;
		Y += center.Y;
	}

	//! normalizes the vector.
	vector2d<T>& normalize()
	{
		T l = (T)getLength();
		if (l == 0)
			return *this;

		l = (T)1.0 / l;
		X *= l;
		Y *= l;
		return *this;
	}

	//如果长度太小,设成缺省值
	vector2d<T>& safe_normalize()
	{
		T l = (T)getLength();
		if (l == 0)
		{
			set(1,0);
			return *this;
		}

		l = (T)1.0 / l;
		X *= l;
		Y *= l;
		return *this;
	}


	//! Calculates the angle of this vector in grad in the trigonometric sense.
	//! This method has been suggested by Pr3t3nd3r.
	//! \return Returns a value between 0 and 360.
	inline f64 getAngleTrig() const
	{
		if (X == 0.0)
			return Y < 0.0 ? 270.0 : 90.0;
		else
		if (Y == 0)
			return X < 0.0 ? 180.0 : 0.0;

		if ( Y > 0.0)
			if (X > 0.0)
				return atan(Y/X) * GRAD_PI;
			else
				return 180.0-atan(Y/-X) * GRAD_PI;
		else
			if (X > 0.0)
				return 360.0-atan(-Y/X) * GRAD_PI;
			else
				return 180.0+atan(-Y/-X) * GRAD_PI;
	} 

	//! Calculates the angle of this vector in grad in the counter trigonometric sense.
	//! \return Returns a value between 0 and 360.
	inline f64 getAngle() const
	{
		if (Y == 0.0)  // corrected thanks to a suggestion by Jox
			return X < 0.0 ? 180.0 : 0.0; 
		else if (X == 0.0) 
			return Y < 0.0 ? 90.0 : 270.0;

		f64 tmp = Y / sqrtf(X*X + Y*Y);
		tmp = atan(sqrtf(1 - tmp*tmp) / tmp) * GRAD_PI;

		if (X>0.0 && Y>0.0)
			return tmp + 270;
		else
		if (X>0.0 && Y<0.0)
			return tmp + 90;
		else
		if (X<0.0 && Y<0.0)
			return 90 - tmp;
		else
		if (X<0.0 && Y>0.0)
			return 270 - tmp;

		return tmp;
	}

	//! Calculates the angle between this vector and another one in grad.
	//! \return Returns a value between 0 and 90.
	inline f64 getAngleWith(const vector2d<T>& b) const
	{
		f64 tmp = X*b.X + Y*b.Y;

		if (tmp == 0.0)
			return 90.0;

		tmp = tmp / sqrtf((X*X + Y*Y) * (b.X*b.X + b.Y*b.Y));
		if (tmp < 0.0) tmp = -tmp;

		return atan(sqrtf(1 - tmp*tmp) / tmp) * GRAD_PI;
	}

	inline f32 getEuler()
	{
		return atan2f(-x, -y)+i_math::Pi;
	}

	inline void setEuler(f32 eulerX)
	{
		x=sinf(eulerX);
		y=cosf(eulerX);
	}


	//! returns interpolated vector
	//! \param other: other vector to interpolate between
	//! \param d: value between 0.0f and 1.0f.
	vector2d<T> getInterpolated(const vector2d<T>& other, f32 d) const
	{
		f32 inv = 1.0f - d;
		return vector2d<T>(other.X*inv + X*d,
						   other.Y*inv + Y*d);
	}

	template<typename TTarget>
	vector2d<TTarget> convert()
	{
		vector2d<TTarget> t;
		t.x=(TTarget)x;
		t.y=(TTarget)y;
		return t;
	}

	// member variables
	union 
	{
		T X;
		T x;
		T u;
	};

	union 
	{
		T Y;
		T y;
		T v;
	};
};

	//! Typedef for f32 2d vector.
	typedef vector2d<f32> vector2df;
	typedef vector2d<f64> vector2dd;
	typedef vector2d<f32> texcoordf;
	//! Typedef for integer 2d vector.
	typedef vector2d<s32> vector2di;
	typedef vector2d<s32> point2di;
	typedef vector2d<u32> vector2du;
	typedef vector2d<u32> point2du;
	typedef vector2d<u8> vector2db;
	typedef vector2d<u8> point2db;

} // end namespace i_math
