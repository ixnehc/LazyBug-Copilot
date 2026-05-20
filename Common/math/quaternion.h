
#pragma once
#include "iTypes.h"
#include "matrix44.h"

namespace i_math
{

//! quat class. 
template <class T>
class quat
{
	public:

		//! Default Constructor
		quat(): X(0), Y(0), Z(0), W(1)
		{
		}

		//! Constructor
		quat(T X, T Y, T Z, T W)
			: X(X), Y(Y), Z(Z), W(W)
		{
		}

		//! Constructor which converts euler angles to a quat
		quat(T x, T y, T z)
		{
			set(x,y,z);
		}

		//! Constructor which converts a matrix to a quat
		quat(const matrix44<T>& mat)
		{
			(*this) = mat;
		}

		//! equal operator
		bool operator==(const quat<T>& other) const
		{
			if(X != other.X)
				return false;
			if(Y != other.Y)
				return false;
			if(Z != other.Z)
				return false;
			if(W != other.W)
				return false;

			return true;
		}

		//! assignment operator
		quat<T>& operator=(const quat<T>& other)
		{
			X = other.X;
			Y = other.Y;
			Z = other.Z;
			W = other.W;
			return *this;
		}

		//! matrix assignment operator
		quat<T>& operator=(const matrix44<T>& m)
		{
			T diag = m(0,0) + m(1,1) + m(2,2) + 1;
			T scale = 0.0f;

			if( diag > 0.0f )
			{
				scale = sqrtf(diag) * (T)2; // get scale from diagonal

				// TODO: speed this up
				X = ( m(1,2) - m(2,1)) / scale;
				Y = ( m(2,0) - m(0,2)) / scale;
				Z = ( m(0,1) - m(1,0)) / scale;
				W = ((T)0.25f) * scale;
			}
			else
			{
				if ( m(0,0) > m(1,1) && m(0,0) > m(2,2))  
				{	
					// 1st element of diag is greatest value
					// find scale according to 1st element, and f64 it
					scale = sqrtf( ((T)1.0f) + m(0,0) - m(1,1) - m(2,2)) * (T)2.0f;

					// TODO: speed this up
					X = ((T)0.25f) * scale;
					Y = (m(0,1) + m(1,0)) / scale;
					Z = (m(2,0) + m(0,2)) / scale;
					W = (m(1,2) - m(2,1)) / scale;	
				} 
				else if ( m(1,1) > m(2,2)) 
				{
					// 2nd element of diag is greatest value
					// find scale according to 2nd element, and f64 it
					scale = sqrtf( ((T)1.0f) + m(1,1) - m(0,0) - m(2,2)) * (T)2.0f;

					// TODO: speed this up
					X = (m(0,1) + m(1,0) ) / scale;
					Y = ((T)0.25f) * scale;
					Z = (m(1,2) + m(2,1) ) / scale;
					W = (m(2,0) - m(0,2) ) / scale;
				} 
				else 
				{	
					// 3rd element of diag is greatest value
					// find scale according to 3rd element, and f64 it
					scale  = sqrtf( ((T)1.0f) + m(2,2) - m(0,0) - m(1,1)) * (T)2.0f;

					// TODO: speed this up
					X = (m(2,0) + m(0,2)) / scale;
					Y = (m(1,2) + m(2,1)) / scale;
					Z = ((T)0.25f) * scale;
					W = (m(0,1) - m(1,0)) / scale;
				}
			}

			normalize();
			return *this;
		}

		//! add operator
		quat<T> operator+(const quat<T>& other) const
		{
			quat tmp;
			tmp.X=X+other.X;
			tmp.Y=Y+other.Y;
			tmp.Z=Z+other.Z;
			tmp.W=W+other.W;
			return tmp;
		}

		//! multiplication operator
		quat<T> operator*(const quat<T>& other) const
		{
			quat tmp;

			tmp.W = (other.W * W) - (other.X * X) - (other.Y * Y) - (other.Z * Z);
			tmp.X = (other.W * X) + (other.X * W) + (other.Y * Z) - (other.Z * Y);
			tmp.Y = (other.W * Y) + (other.Y * W) + (other.Z * X) - (other.X * Z);
			tmp.Z = (other.W * Z) + (other.Z * W) + (other.X * Y) - (other.Y * X);

			return tmp;
		}

		//! multiplication operator
		quat<T> operator*(T s) const
		{
			return quat(s*X, s*Y, s*Z, s*W);
		}

		//! multiplication operator
		quat<T>& operator*=(T s)
		{
			X *= s; Y*=s; Z*=s; W*=s;
			return *this;
		}

		//! multiplication operator
		vector3d<T> operator* (const vector3d<T>& v) const
		{
			// nVidia SDK implementation 

			vector3d<T> uv, uuv; 
			vector3d<T> qvec(X, Y, Z); 
			uv = qvec.crossProduct(v); 
			uuv = qvec.crossProduct(uv); 
			uv *= (((T)2.0f) * W); 
			uuv *= (T)2.0f; 

			return v + uv + uuv; 
		}

		//! multiplication operator
		quat<T>& operator*=(const quat<T>& other)
		{
			*this = other * (*this);
			return *this;
		}

		bool equals(const quat<T>& other)const
		{
			return i_math::equals(X, other.X) &&
				i_math::equals(Y, other.Y) &&
				i_math::equals(Z, other.Z)&&
				i_math::equals(W, other.W);
		}


		//! calculates the dot product
		T getDotProduct(const quat<T>& q2) const
		{
			return (X * q2.X) + (Y * q2.Y) + (Z * q2.Z) + (W * q2.W);
		}

		//! sets new quat 
		void set(T x, T y, T z, T w)
		{
			X = x;
			Y = y;
			Z = z;
			W = w;
		}

		//! sets new quat based on euler angles
		void set(T x, T y, T z)
		{
			T angle;

			angle = x * 0.5f;
			T sr = (T)sinf(angle);
			T cr = (T)cosf(angle);

			angle = y * 0.5f;
			T sp = (T)sinf(angle);
			T cp = (T)cosf(angle);

			angle = z * 0.5f;
			T sy = (T)sinf(angle);
			T cy = (T)cosf(angle);

			T cpcy = cp * cy;
			T spcy = sp * cy;
			T cpsy = cp * sy;
			T spsy = sp * sy; 

			X = sr * cpcy - cr * spsy;
			Y = cr * spcy + sr * cpsy;
			Z = cr * cpsy - sr * spcy;
			W = cr * cpcy + sr * spsy;

			normalize();
		}

		//! normalizes the quat
		quat<T>& normalize()
		{
			T n = X*X + Y*Y + Z*Z + W*W;

			if (n == 1)
				return *this; 

			n = 1.0f / sqrtf(n);
			X *= n;
			Y *= n;
			Z *= n;
			W *= n;

			return *this;

		}

		//! Creates a matrix from this quat
		matrix44<T> getMatrix() const
		{
			matrix44<T> m;

			m(0,0) = 1.0f - 2.0f*Y*Y - 2.0f*Z*Z; 
			m(0,1) = 2.0f*X*Y + 2.0f*Z*W; 
			m(0,2) = 2.0f*X*Z - 2.0f*Y*W; 
			m(0,3) = 0.0f;

			m(1,0) = 2.0f*X*Y - 2.0f*Z*W; 
			m(1,1) = 1.0f - 2.0f*X*X - 2.0f*Z*Z; 
			m(1,2) = 2.0f*Z*Y + 2.0f*X*W; 
			m(1,3) = 0.0f;

			m(2,0) = 2.0f*X*Z + 2.0f*Y*W; 
			m(2,1) = 2.0f*Z*Y - 2.0f*X*W; 
			m(2,2) = 1.0f - 2.0f*X*X - 2.0f*Y*Y; 
			m(2,3) = 0.0f;

			m(3,0) = 0.0f; 
			m(3,1) = 0.0f; 
			m(3,2) = 0.0f; 
			m(3,3) = 1.0f;

			return m;
		}

		//! Inverts this quat
        void makeInverse()
		{
			X = -X; Y = -Y; Z = -Z;
		}

		quat<T> getInvert() const
		{
			quat<T> t=*this;
			t.makeInverse();
			return t;
		}

		//! Interpolates the quat between to quats based on time
		//time is 0,full q1; time is 1,full q2
		quat<T> slerp(quat<T> q1, quat<T> q2, T time)
		{
			T angle = q1.getDotProduct(q2);

			if (angle < 0.0f) 
			{
				q1 *= -1.0f;
				angle *= -1.0f;
			}

			T scale;
			T invscale;

			if ((angle + 1.0f) > 0.05f) 
			{
				if ((1.0f - angle) >= 0.05f)  // spherical interpolation
				{
					T theta = (T)acos(angle);
					T invsintheta = 1.0f / (T)sinf(theta);
					scale = (T)sinf(theta * (((T)1.0f)-((T)time))) * invsintheta;
					invscale = (T)sinf(theta * ((T)time)) * invsintheta;
				}
				else // linear interploation
				{
					scale = 1.0f - time;
					invscale = time;
				}
			}
			else 
			{
				q2 = quat(-q1.Y, q1.X, -q1.W, q1.Z);
				scale = (T)sinf(Pi * (((T)0.5f) - ((T)time)));
				invscale = (T)sinf(((T)Pi) * ((T)time));
			}

			*this = (q1*scale) + (q2*invscale);
			return *this;
		}

		//! axis must be unit length 
		//angle是一个顺时针的角度(当axis指向观察者的时候)
		//! The quat representing the rotation is 
		//!  q = cosf(A/2)+sinf(A/2)*(x*i+y*j+z*k) 
		void fromAngleAxis (T angle, const vector3d<T>& axis)
		{
			T fHalfAngle = 0.5f*angle; 
			T fSin = (T)sinf(fHalfAngle); 
			W = (T)cosf(fHalfAngle); 
			X = fSin*axis.X; 
			Y = fSin*axis.Y; 
			Z = fSin*axis.Z; 
		}

		//quaternion必须是normalize的
		void toAngleAxis(T &angle,vector3d<T>&axis )
		{
			angle = 2 * acos(W);
			T s = sqrtf(1-W*W); // assuming quaternion normalised then w is less than 1, so term always positive.
			if (s < 0.001) 
			{
				axis.x=X;
				axis.y=Y;
				axis.z=Z;
			}
			 else 
			 {
				axis.x = X/ s; // normalise axis
				axis.y = Y/ s;
				axis.z = Z/ s;
			}
		}

		//欧拉角的一些说明:
		//head:正值表示沿着y轴的顺时针方向旋转,当y轴指向观察者的时候
		//pitch:正值表示向上抬
		//roll:正值表示沿着z轴的顺时针方向旋转,当z轴指向观察者的时候
		void toEuler(vector3d<T>& euler)
		{
			f64 sqw0 = W*W;    
			f64 sqz0 = Z*Z;    
			f64 sqy0 = Y*Y;    
			f64 sqx0 = (-X)*(-X); 

			//head
			euler.x= (float) atan2((f64)(2*Y*W-2*Z*(-X)),(f64)(sqw0+sqz0-sqy0-sqx0));
			//pitch
			euler.y= -(float)asin(clamp_dbl(-(f64)(2*Z*Y + 2*(-X)*W),-1,1));
			euler.y= normalize_radian(euler.y);

			//roll
			euler.z= (float)atan2((f64)(2*Z*W-2*Y*(-X)),(f64)(sqw0 -sqz0-sqx0+sqy0));
		}

		void fromEuler(i_math::vector3df &a)
		{
			float c1 = cosf(a.x/2.0f);
			float c2 = cosf(-a.y/2.0f);
			float c3 = cosf(a.z/2.0f);
			float s1 = sinf(a.x/2.0f);
			float s2 = sinf(-a.y/2.0f);
			float s3 = sinf(a.z/2.0f);

			W= c3*c2*c1+s3*s2*s1;
			Z = s3*c2*c1-c3*s2*s1;
			X = c3*s2*c1+s3*c2*s1;
			Y = c3*c2*s1-s3*s2*c1;
		}


		//build a rotate quat which could rotate start to end
		//both the 2 vector should be normalized
		//from Game Programming Gems I ,chapter 2,section 10
		void from2Vector(vector3d<T>&start,vector3d<T>&end)
		{
			vector3d<T> t;
			t=start.crossProduct(end);
			T d;
			d=start.dotProduct(end);
			d=(1+d)*2;
			if (d<0.0001f)
				d=0.0001f;
			T s;
			s=(T)sqrtf(d);
			X=t.x/s;
			Y=t.Y/s;
			Z=t.z/s;
			W=s/(T)2.0;
		}

		static quat<T> exp(quat<T> & q) //[0,a*V]-->[cosa,sina*V]
		{
			vector3d<T> v(q.X,q.Y,q.Z);
			float a = float(v.getLength());
			float cosa  = cosf(a);
			float sina = sinf(a);
			quat<T> ret;
			ret.W = cosa;
			if(a>0){
				ret.X = sina*q.X/a;
				ret.Y = sina*q.Y/a;
				ret.Z = sina*q.Z/a;
			}
			else{
				ret.X = ret.Y = ret.Z = 0;
			}

			return ret;
		}
		static quat<T> squad(quat<T> &q0,quat<T> &q1,quat<T> &a,quat<T> &b,float t)// a,b:ctrl point
		{
			quat<T> c,d,ret;
			c.slerp(q0,q1,t);
			d.slerp(a,b,t);
			ret.slerp(c,d,2*t*(1-t));
			return ret;
		}
		static quat<T> spline(quat<T> &qn1,quat<T>&q,quat<T>&qp1)//ctrl point
		{
			quat<T> ret;
			quat<T> q1 = q;
			q1.makeInverse();
			quat<T> a = log(q1*qn1) + log(q1*qp1);
			a *= -0.25;
			ret = q*exp(a);
			return ret;
		}

		static quat<T> log(quat<T> &q) // [cosa,sina*V] --> [0,a*V]
		{
			quat<T> ret;
			float a = acos(q.W);
			float sina = sinf(a);
			ret.W = 0;
			if(sina>0){
				ret.X = T(float(q.X)/sina);
				ret.Y = T(float(q.Y)/sina);
				ret.Z = T(float(q.Z)/sina);
			}
			else{
				ret.X = ret.Y = ret.Z = 0;
			}
			return ret;
		}

		void align(i_math::vector3df &dir,i_math::vector3df &vAxis)
		{
			i_math::vector3df a=vAxis;
			a=(*this)*a;

			//构建一个quaternion把za转到vel
			i_math::quatf qOff;
			qOff.from2Vector(a,dir);

			(*this)=(*this)*qOff;
			normalize();
		}

		//把这个旋转的z轴对齐到一个方向上,dir必须是normalize过的
		void alignZ(i_math::vector3df &dir)
		{
			i_math::vector3df a(0,0,1);
			align(dir,a);
		}

		void alignY(i_math::vector3df &dir)
		{
			i_math::vector3df a(0,1,0);
			align(dir,a);
		}

		void alignX(i_math::vector3df &dir)
		{
			i_math::vector3df a(1,0,0);
			align(dir,a);
		}

		i_math::vector3d<T> getVec() const
		{
			return i_math::vector3d<T>(X,Y,Z);
		}


		union
		{
			struct 
			{T X, Y, Z, W;};
			T array[4];
		};
};

typedef quat<f32> quatf;
typedef quat<s32> quati;
typedef quat<u16> quatu16;

#define slerpf slerp<i_math::f32>


//v1,v2 should be normalized
//if r is 0,return v1,if r is 1,return v2
template<class T>
vector3d<T> slerp(vector3d<T>&v1,vector3d<T>&v2,T r)
{
	i_math::quat<T> qt,qtZero;
	qt.from2Vector(v1,v2);
	qt.slerp(qtZero,qt,r);

	i_math::vector3d<T> ret=qt*v1;//could make it no longer normalized
	ret.normalize();
	return ret;
}


} // end namespace i_math


