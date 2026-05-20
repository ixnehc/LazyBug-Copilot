#pragma once
#include <math.h>
#include <cmath>
#include "iMath.h"
#include "iTypes.h"

#include "vector2d.h"

namespace i_math
{

	//! 3d vector template class with lots of operators and methods.
	template <class T>
	class vector3d  
	{
	public:

		vector3d(): X(0), Y(0), Z(0) {};
		vector3d(T nx, T ny, T nz) : X(nx), Y(ny), Z(nz) {};
		vector3d(const vector3d<T>& other)	:X(other.X), Y(other.Y), Z(other.Z) {};

		// operators

		vector3d<T> operator-() const { return vector3d<T>(-X, -Y, -Z);   }

		vector3d<T>& operator=(const vector3d<T>& other)	{ X = other.X; Y = other.Y; Z = other.Z; return *this; }

		vector3d<T> operator+(const vector3d<T>& other) const { return vector3d<T>(X + other.X, Y + other.Y, Z + other.Z);	}
		vector3d<T>& operator+=(const vector3d<T>& other)	{ X+=other.X; Y+=other.Y; Z+=other.Z; return *this; }

		vector3d<T> operator-(const vector3d<T>& other) const { return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z);	}
		vector3d<T>& operator-=(const vector3d<T>& other)	{ X-=other.X; Y-=other.Y; Z-=other.Z; return *this; }
        
		vector3d<T> operator*(const vector3d<T>& other) const { return vector3d<T>(X * other.X, Y * other.Y, Z * other.Z);	}
		vector3d<T>& operator*=(const vector3d<T>& other)	{ X*=other.X; Y*=other.Y; Z*=other.Z; return *this; }
		vector3d<T> operator*(const T v) const { return vector3d<T>(X * v, Y * v, Z * v);	}
		vector3d<T>& operator*=(const T v) { X*=v; Y*=v; Z*=v; return *this; }

		vector3d<T> operator/(const vector3d<T>& other) const { return vector3d<T>(X / other.X, Y / other.Y, Z / other.Z);	}
		vector3d<T>& operator/=(const vector3d<T>& other)	{ X/=other.X; Y/=other.Y; Z/=other.Z; return *this; }
		vector3d<T> operator/(const T v) const { return vector3d<T>(X /v, Y /v, Z /v);	}
		vector3d<T>& operator/=(const T v) { T i=(T)1.0/v; X*=i; Y*=i; Z*=i; return *this; }

		bool operator<=(const vector3d<T>&other) const { return X<=other.X && Y<=other.Y && Z<=other.Z;};
		bool operator>=(const vector3d<T>&other) const { return X>=other.X && Y>=other.Y && Z>=other.Z;};

		bool operator==(const vector3d<T>& other) const { return other.X==X && other.Y==Y && other.Z==Z; }
		bool operator!=(const vector3d<T>& other) const { return other.X!=X || other.Y!=Y || other.Z!=Z; }

		// functions

		bool isAnyNan()
		{
			return i_math::isnan(x)||i_math::isnan(y)||i_math::isnan(z);
		}

		//! returns if this vector equals the other one, taking floating point rounding errors into account
		bool equals(const vector3d<T>& other)const
		{
			return i_math::equals(X, other.X) &&
				   i_math::equals(Y, other.Y) &&
				   i_math::equals(Z, other.Z);
		}

		bool equals(const vector3d<T>& other,T epsilon)const
		{
			return i_math::equals(X, other.X,epsilon) &&
				   i_math::equals(Y, other.Y,epsilon) &&
				   i_math::equals(Z, other.Z,epsilon);
		}

		//! returns if this vector equals (0,0,0), taking floating point rounding errors into account
		bool equalsZero()const
		{
			return i_math::equals(X, 0) &&
				i_math::equals(Y, 0) &&
				i_math::equals(Z, 0);
		}

		bool equalsOne()const
		{
			return i_math::equals(X, 1) &&
				i_math::equals(Y, 1) &&
				i_math::equals(Z, 1);
		}

		bool isUniform()const//三个值都相同称为uniform
		{
			return i_math::equals(X, Y) &&
				i_math::equals(Y, Z) &&
				i_math::equals(Z, X);
		}

		bool divUniform(vector3d<T> &target,T&result,T epsilon=0.0001f)
		{
			if (target.x!=(T)0)
				result=x/target.x;
			else
			{
				if (target.y!=(T)0)
					result=y/target.y;
				else
				{
					if (target.z!=(T)0)
						result=z/target.z;
					else
						return false;
				}
			}

			vector3d<T> t;
			t=target*result;

			if (equals(t,epsilon))
				return true;

			return false;
		}



		void set(const T nx, const T ny, const T nz) {X=nx; Y=ny; Z=nz; }
		void set(const vector3d<T>& p) { X=p.X; Y=p.Y; Z=p.Z;}
		void setZero() {X=0;Y=0;Z=0;}
		void setXZ(i_math::vector2d<T> const &v)		{			X=v.x; Z=v.y;		}

		//! Returns length of the vector.
		T getLength() const { return std::sqrt(X * X + Y * Y + Z * Z); }
		T getLengthXZ() const { return std::sqrt(X*X + Z*Z); }
		T getLengthXY() const { return std::sqrt(X*X + Y*Y); }
		T getLengthYZ() const { return std::sqrt(Z*Z + Y*Y); }

		//! Returns squared length of the vector.
		/** This is useful because it is much faster then
		getLength(). */
		T getLengthSQ() const { return X*X + Y*Y + Z*Z; }
		T getLengthSQ_XZ() const { return (X*X + Z*Z); }
		T getLengthSQ_XY() const { return (X*X + Y*Y); }
		T getLengthSQ_YZ() const { return (Z*Z + Y*Y); }

		i_math::vector2d<T> getXZ()
		{
			return i_math::vector2d<T>(X,Z);
		}

		void setXZ(i_math::vector2d<T> &other)
		{
			x=other.x;
			z=other.y;
		}

		//! Returns the dot product with another vector.
		T dotProduct(const vector3d<T>& other) const
		{
			return X*other.X + Y*other.Y + Z*other.Z;
		}

		//! Returns XZ distance from an other point.
		T getDistanceXZFrom(const vector3d<T>& other) const
		{
			T vx = X - other.X; T vz = Z - other.Z;
			return sqrtf(vx*vx + vz*vz);
		}

		//! Returns squared XZ distance from an other point.
		T getDistanceXZFromSQ(const vector3d<T>& other) const
		{
			T vx = X - other.X; T vz = Z - other.Z;
			return vx*vx + vz*vz;
		}

		//! Returns squared distance from an other point. 
		/** Here, the vector is interpreted as point in 3 dimensional space. */
		T getDistanceFromSQ(const vector3d<T>& other) const
		{
			T vx = X - other.X; T vy = Y - other.Y; T vz = Z - other.Z;
			return (vx*vx + vy*vy + vz*vz);
		}

		//! Returns distance from an other point.
		/** Here, the vector is interpreted as point in 3 dimensional space. */
		T getDistanceFrom(const vector3d<T>& other) const
		{
			T vx = X - other.X; T vy = Y - other.Y; T vz = Z - other.Z;
			return std::sqrt(vx*vx + vy*vy + vz*vz);
		}


		//! Calculates the cross product with another vector
		vector3d<T> crossProduct(const vector3d<T>& p) const
		{
			return vector3d<T>(Y * p.Z - Z * p.Y, Z * p.X - X * p.Z, X * p.Y - Y * p.X);
		}

		//! Returns if this vector interpreted as a point is on a line between two other points.
		/** It is assumed that the point is on the line. */
		bool isBetweenPoints(const vector3d<T>& begin, const vector3d<T>& end) const
		{
			T f = (T)(end - begin).getLengthSQ();
			return (T)getDistanceFromSQ(begin) <= f && 
				(T)getDistanceFromSQ(end) <= f;
		}

		//! Normalizes the vector.
		vector3d<T>& normalize()
		{
			T l = (T)getLength();
			if (l == 0)
				return *this;

			l = (T)1.0 / l;
			X *= l;
			Y *= l;
			Z *= l;
			return *this;
		}

		//! Sets the lenght of the vector to a new value
		void setLength(T newlength)
		{
			normalize();
			*this *= newlength;
		}

		//newdir should be normalized
		void setDir(vector3d<T> &newdir)
		{
			vector3d<T>t;
			t=newdir;
			t*=(T)getLength();
			*this=t;
		}

		//! Inverts the vector.
		void invert()
		{
			X *= -1.0f;
			Y *= -1.0f;
			Z *= -1.0f;
		}

		void rotateXZBy(T degrees, const vector3d<T>& center)
		{
			degrees *=(T)GRAD_PI2;
			T cs = (T)cosf(degrees);
			T sn = (T)sinf(degrees);
			X -= center.X;
			Z -= center.Z;
			set(X*cs - Z*sn, Y, X*sn + Z*cs);
			X += center.X;
			Z += center.Z;
		}

		void rotateXYBy(T degrees, const vector3d<T>& center)
		{
			degrees *=(T)GRAD_PI2;
			T cs = (T)cosf(degrees);
			T sn = (T)sinf(degrees);
			X -= center.X;
			Y -= center.Y;
			set(X*cs - Y*sn, X*sn + Y*cs, Z);
			X += center.X;
			Y += center.Y;
		}

		void rotateYZBy(T degrees, const vector3d<T>& center)
		{
			degrees *=(T)GRAD_PI2;
			T cs = (T)cosf(degrees);
			T sn = (T)sinf(degrees);
			Z -= center.Z;
			Y -= center.Y;
			set(X, Y*cs - Z*sn, Y*sn + Z*cs);
			Z += center.Z;
			Y += center.Y;
		}

		//! Returns interpolated vector.
		/** \param other: other vector to interpolate between
		\param d: value between 0.0f and 1.0f. */
		//if d is 1, fully me,if d is 0,fully other
		vector3d<T> getInterpolated(const vector3d<T>& other, T d) const
		{
			T inv = 1.0f - d;
			return vector3d<T>(other.X*inv + X*d,
								other.Y*inv + Y*d,
								other.Z*inv + Z*d);
		}

		//! Gets the Y and Z rotations of a vector.
		/** Thanks to Arras on the Irrlicht forums to add this method.
		 \return A vector representing the rotation in degrees of
		this vector. The Z component of the vector will always be 0. */
		vector3d<T> getHorizontalAngle()
		{
			vector3d<T> angle;

			angle.Y = (T)atan2f(X, Z); 
			angle.Y *= (T)GRAD_PI;
			    
			if (angle.Y < 0.0f) angle.Y += 360.0f; 
			if (angle.Y >= 360.0f) angle.Y -= 360.0f; 
			    
			T z1; 
			z1 = (T)sqrtf(X*X + Z*Z); 
			    
			angle.X = (T)atan2f(z1, Y); 
			angle.X *= (T)GRAD_PI;
			angle.X -= 90.0f; 
			    
			if (angle.X < 0.0f) angle.X += 360.0f; 
			if (angle.X >= 360) angle.X -= 360.0f; 

			return angle;
		}

		//convert Direction representation to Head/Pitch/Banking representation
		//should be a normalized direction vector
		//Direction转Angle
		void toAngle()
		{
			// now calculate the angles
			T h,p,b;

			// banking is always irrelevant
			b = 0;
			// calculate pitch
			p = (T)(asin(Y)*GRAD_PI);

			// if y is near +1 or -1
			if (Y>0.9999 || Y<-0.9999) 
				h = 0;
			else 
				h = (T)(atan2f(-X, -Z)*GRAD_PI);
			X=h;
			Y=p;
			Z=b;
		}

		//convert Head/Pitch/Banking representation to Direction representation 
		//Angle转Direction
		void toDirection()
		{
			T fSinH = (T)sinf(X*GRAD_PI2);  // heading
			T fCosH = (T)cosf(X*GRAD_PI2);
			T fSinP = (T)sinf(Y*GRAD_PI2);  // pitch
			T fCosP = (T)cosf(Y*GRAD_PI2);

			X = (T)(-fCosP*fSinH);
			Y = (T)+fSinP;
			Z = (T)(-fCosP*fCosH);
		}

		//Direction转换为欧拉角
		void toEuler()
		{
			toAngle();
			x=x*(T)i_math::GRAD_PI2+i_math::Pi;
			y=y*(T)i_math::GRAD_PI2;
			z=z*(T)i_math::GRAD_PI2;
		}

		//欧拉角转Direction
		void eulerToDir()
		{
			x=(x-i_math::Pi)*(T)i_math::GRAD_PI;
			y=y*(T)i_math::GRAD_PI;
			z=z*(T)i_math::GRAD_PI;
			toDirection();
		}

		//! Fills an array of 4 values with the vector data (usually floats).
		/** Useful for setting in shader constants for example. The fourth value
		 will always be 0. */
		void getAs4Values(T* array)
		{
			array[0] = X;
			array[1] = Y;
			array[2] = Z;
			array[3] = 0;
		}

		void fromDwordColor(u32 c)
		{
			unsigned char *p=(unsigned char *)&c;
			b=((T)p[0])/((T)255);
			g=((T)p[1])/((T)255);
			r=((T)p[2])/((T)255);
		}

		void toDwordColor(u32 &c)
		{
			unsigned char *p=(unsigned char *)&c;
			p[0]= i_math::clamp_u((u32)(b*255.0+0.5),0,255);
			p[1]= i_math::clamp_u((u32)(g*255.0+0.5),0,255);
			p[2]= i_math::clamp_u((u32)(r*255.0+0.5),0,255);
			p[3]=255;
		}

		u32 toDwordColor()
		{
			u32 t;
			toDwordColor(t);
			return t;
		}

		T toLightValue()
		{
			return (T)(((T)x)*0.3+((T)y)*0.59+((T)z)*0.11);
		}

		//floor the x,y,z onto the bound of gran
		void floor(T gran)
		{
			x=::floor(x/gran)*gran;
			y=::floor(y/gran)*gran;
			z=::floor(z/gran)*gran;
		}

		friend vector3d<T> getNormalFrom3Points(const vector3d<T>& point1, const vector3d<T>& point2, const vector3d<T>& point3)
		{
			vector3d<T>  Normal;
			// creates the plane from 3 memberpoints
			Normal = (point2 - point1).crossProduct(point3 - point1);
			Normal.normalize();
			return Normal;
		}
		friend vector3d<T> operator*( T Scale, const vector3d<T>& V )
		{
			return vector3d<T>( V.X * Scale, V.Y * Scale, V.Z * Scale );
		}


		//寻找与本vector垂直的另两根轴,并且这两根轴也互相垂直
		//本vector要求为normalized的
		void findBestAxis( vector3d<T>& Axis1, vector3d<T>& Axis2 ) const
		{
			T NX = abs(X);
			T NY = abs(Z);
			T NZ = abs(Y);

			if( NZ>NX && NZ>NY )	Axis1 = vector3d<T>(1,0,0);
			else					Axis1 = vector3d<T>(0,0,1);


			Axis2=crossProduct(Axis1);
			Axis2.normalize();

			Axis1=Axis2.crossProduct(*this);
			Axis1.normalize();

// 			vector3d<T> tempV=*this;
// 			T temp=tempV.Z;
// 			tempV.Z=tempV.Y;
// 			tempV.Y=temp;
// 
// 			Axis1 = (Axis1 - tempV * (Axis1.dotProduct(tempV)));
// 			Axis1.normalize();
// 			Axis2 = Axis1 .crossProduct(tempV) ;
// 
// 			temp=Axis1.Z;
// 			Axis1.Z=Axis1.Y;
// 			Axis1.Y=temp;
// 
// 			temp=Axis2.Z;
// 			Axis2.Z=Axis2.Y;
// 			Axis2.Y=temp;
		}

		//返回x,y,z中最大的那个值
		T getMaxComponent()
		{
			T t;
			if (x>y)
				t=x;
			else
				t=y;
			if (z>t)
				return z;
			return t;
		}


		static vector3d<T> *zero()
		{
			static vector3d<T> t;
			return &t;
		}

		template<typename TTarget>
		vector3d<TTarget> convert()
		{
			vector3d<TTarget> t;
			t.x=(TTarget)x;
			t.y=(TTarget)y;
			t.z=(TTarget)z;
			return t;
		}


		// member variables

		union
		{
			T X;T x;T w0;T r;
		};
		union
		{
			T Y;T y;T w1;T g;
		};
		union
		{
			T Z;T z;T w2;T b;
		};
	};


	//! Typedef for a f32 3d vector.
	typedef vector3d<f32> vector3df;
	typedef vector3d<f32> weight3f;
	typedef vector3d<f32> color3df;
	//! Typedef for a f64 3d vector.
	typedef vector3d<f64> vector3dd;
	//! Typedef for an integer 3d vector.
	typedef vector3d<s32> vector3di;
	typedef vector3d<s16> vector3ds;

} // end namespac i_math



// 
// 
// //------------------------------------------------------------------------------------------------------add by wy begin
// bool  IsZero() const
// {
// 	return (f32)X==0.f && (f32)Y==0.f && (f32)Z==0.f;
// }
// //------------------------------------------------------------------------------------------------------
// vector3d<T> SafeNormal() const
// {
// 	f32 SquareSum = X*X + Y*Y + Z*Z;
// 	if( SquareSum < SMALL_NUMBER )
// 		//return FVector( 0.f, 0.f, 0.f );
// 		return vector3d( );
// 
// 	f32 Scale = appInvSqrt(SquareSum);
// 	return vector3d( X*(T)Scale, Y*(T)Scale, Z*(T)Scale );
// }
// 
// //------------------------------------------------------------------------------------------------------
// bool Normalize()
// {
// 	f32 SquareSum = X*X+Y*Y+Z*Z;
// 	if( SquareSum >= SMALL_NUMBER )
// 	{
// 		f32 Scale = appInvSqrt(SquareSum);
// 		X *= Scale; Y *= Scale; Z *= Scale;
// 		return 1;
// 	}
// 	else return 0;
// }
// //------------------------------------------------------------------------------------------------------
// T Size() const
// {
// 	return appSqrt( X*X + Y*Y + Z*Z );
// }
// //------------------------------------------------------------------------------------------------------
// T SizeSquared() const
// {
// 	return X*X + Y*Y + Z*Z;
// }
// 
