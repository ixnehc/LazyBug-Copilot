#pragma once
#include "iTypes.h"

namespace i_math
{

	//! Simple class for holding 2d coordinates.
	/** Not supposed for doing geometric calculations.
	use vector2d instead for things like that. 
	*/
	template <class T>
	class pos2d
	{
		public:
			pos2d(T x, T y)
				: X(x), Y(y) {};


			pos2d()
				: X(0), Y(0) {};


			pos2d(const pos2d<T>& other)
				: X(other.X), Y(other.Y) {};

			void set(T x0,T y0)
			{
				X=x0;
				Y=y0;
			}


			bool operator == (const pos2d<T>& other) const
			{
				return X == other.X && Y == other.Y;
			}


			bool operator != (const pos2d<T>& other) const
			{
				return X != other.X || Y != other.Y;
			}

			const pos2d<T>& operator+=(const pos2d<T>& other)
			{
				X += other.X;
				Y += other.Y;
				return *this;
			}

			const pos2d<T>& operator-=(const pos2d<T>& other)
			{
				X -= other.X;
				Y -= other.Y;
				return *this;
			}

			const pos2d<T>& operator*=(T r)
			{
				X *=r;
				Y *=r;
				return *this;
			}

			const pos2d<T>& operator/=(T r)
			{
				X /=r;
				Y /=r;
				return *this;
			}


			pos2d<T> operator-(const pos2d<T>& other) const
			{
				return pos2d<T>(X-other.X, Y-other.Y);
			}

			pos2d<T> operator+(const pos2d<T>& other) const
			{
				return pos2d<T>(X+other.X, Y+other.Y);
			}

			pos2d<T> operator*(const pos2d<T>& other) const
			{
				return pos2d<T>(X*other.X, Y*other.Y);
			}

			pos2d<T> operator/(const pos2d<T>& other) const
			{
				return pos2d<T>(X/other.X, Y/other.Y);
			}

			pos2d<T> operator*(T v) const
			{
				return pos2d<T>(X*v, Y*v);
			}

			pos2d<T> operator/(T v) const
			{
				return pos2d<T>(X/v, Y/v);
			}

			bool operator <(const pos2d<T>& other) const
			{
				return (X<other.X||(X==other.X&&Y<other.Y));
			}
			const pos2d<T>& operator=(const pos2d<T>& other) 
			{
				X = other.X;
				Y = other.Y;
				return *this;
			}

			void scale_signed(T v)
			{
				X=(T)floor((double)X/(double)v);
				Y=(T)floor((double)Y/(double)v);
			}

			union 
			{
				T X;
				T x;
			};
			union
			{
				T Y;
				T y;
			};

	};

	typedef pos2d<f32> pos2df;
	typedef pos2d<s32> pos2di;
	typedef pos2d<s16> pos2d_sh;
	typedef pos2d<u8> pos2db;


} 


