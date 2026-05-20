#pragma once 
#include "vector3d.h"
#include "quaternion.h"
#include "aabbox3d.h"
#include "sphere.h"
#include "capsule.h"
#include "line3d.h"

#include <string>

namespace i_math
{

	template <class T>
	class matrix43
	{
	public:
		union
		{
			struct
			{
				T m00;T m10;T m20;
				T m01;T m11;T m21;
				T m02;T m12;T m22;
				T m03;T m13;T m23;
			};

			T M[4][3];
			T m[12];
		};

		matrix43()
		{
			makeIdentity();
		}

		inline matrix43(
			T i00, T i10, T i20,
			T i01, T i11, T i21,
			T i02, T i12, T i22,
			T i03, T i13, T i23):
		m00(i00), m01(i01), m02(i02), m03(i03),
			m10(i10), m11(i11), m12(i12), m13(i13),
			m20(i20), m21(i21), m22(i22), m23(i23)
		{
		}


		//! Simple operator for directly accessing every element of the matrix.
		T& operator()(s32 row, s32 col) { return M[col * 4 + row]; }

		//! Simple operator for directly accessing every element of the matrix.
		const T& operator()(s32 row, s32 col) const {  return M[col * 4 + row]; }

		//! Sets this matrix equal to the other matrix.
		matrix43<T>& operator=(const matrix43<T> &other)
		{
			for (s32 i = 0; i < 12; ++i)
				m[i] = other.m[i];

			return *this;
		}

		inline void set(
			T v00, T v01, T v02,
			T v10, T v11, T v12,
			T v20, T v21, T v22,
			T v30, T v31, T v32)
		{
			m00 = v00; m10 = v01; m20 = v02;
			m01 = v10; m11 = v11; m21 = v12;
			m02 = v20; m12 = v21; m22 = v22;
			m03 = v30; m13 = v31; m23 = v32;
		}

		inline void set(i_math::vector3df &xaxis,i_math::vector3df &yaxis,i_math::vector3df &zaxis,i_math::vector3df &o)
		{
			memcpy(m,&xaxis,sizeof(xaxis));
			memcpy(m+3,&yaxis,sizeof(yaxis));
			memcpy(m+6,&zaxis,sizeof(zaxis));
			memcpy(m+9,&o,sizeof(o));
		}

		inline void setZero()
		{
			memset(m,0,sizeof(m));
		}

		inline void makeIdentity()
		{
			memset(m,0,sizeof(m));
			m00=m11=m22=1.0f;
		}

		inline void setScale(T scaleX, T scaleY, T scaleZ)
		{
			set(scaleX,    0.f,    0.f,
				0.f, scaleY,    0.f,
				0.f,    0.f, scaleZ,
				0.f,    0.f,    0.f);
		}

		inline void setScale(const vector3d<T>& scale)
		{
			setScale(scale.X, scale.Y, scale.Z);
		}

		inline void addScale(T scaleX, T scaleY, T scaleZ)
		{
			matrix43<T> matrix;
			matrix.setScale(scaleX, scaleY, scaleZ);
			(*this) *= matrix;
		}

		inline void addScale(const vector3d<T>& scale)
		{
			matrix43<T> matrix;
			matrix.setScale(scale.X, scale.Y, scale.Z);
			(*this) *= matrix;
		}

		inline vector3d<T> getScale()
		{
			vector3d<T>t;
			t.x=(T)(((vector3d<T>*)&m00)->getLength());
			t.y=(T)(((vector3d<T>*)&m01)->getLength());
			t.z=(T)(((vector3d<T>*)&m02)->getLength());
			return t;
		}

		inline T getScaleX()
		{
			return (T)(((vector3d<T>*)&m00)->getLength());
		}

		inline void removeScale()
		{
			i_math::matrix43<T> t;
			t.setScale(((T)1)/(T)(((vector3d<T>*)&m00)->getLength()),
							((T)1)/(T)(((vector3d<T>*)&m01)->getLength()),
							((T)1)/(T)(((vector3d<T>*)&m02)->getLength()));
			(*this)=t*(*this);
		}

		inline void setRotationY(T radian)
		{
			T sinv = sinf(radian);
			T cosv = cosf(radian);
			set( cosv,  0.f, -sinv,
				0.f,  1.f,  0.f,
				sinv,  0.f,  cosv,
				0.f,  0.f,  0.f);
		}

		inline void addRotationY(T radian)
		{
			matrix43<T> matrix;
			matrix.setRotationY(radian);
			(*this) *= matrix;
		}

		inline void setRotationZ(T radian)
		{
			T sinv = sinf(radian);
			T cosv = cosf(radian);
			set( cosv,  sinv,  0.f,
				-sinv,  cosv,  0.f,
				0.f,  0.f,  1.f,
				0.f,  0.f,  0.f);
		}

		inline void addRotationZ(T radian)
		{
			matrix43<T> matrix;
			matrix.setRotationZ(radian);
			(*this) *= matrix;
		}

		//axis is required normalized
		inline void setRotationAxis(const vector3d<T>& axis, T radian)
		{
			T sinv = sinf(radian);
			T cosv = cosf(radian);
			T invCos = 1.f - cosv;
			T xyInv = axis.X * axis.Y * invCos;
			T xzInv = axis.X * axis.Z * invCos;
			T yzInv = axis.Y * axis.Z * invCos;
			T xSin = axis.X * sinv;
			T ySin = axis.Y * sinv;
			T zSin = axis.Z * sinv;
			set((axis.X * axis.X * invCos + cosv), (xyInv - zSin), (xzInv + ySin),
				(xyInv + zSin), (axis.Y * axis.Y * invCos + cosv), (yzInv - xSin),
				(xzInv - ySin), (yzInv + xSin), (axis.Z * axis.Z * invCos + cosv),
				0.0f, 0.0f, 0.0f
				);
			/*
			col major
			set((axis.X * axis.X * invCos + cosv), (xyInv + zSin), (xzInv - ySin),
				(xyInv - zSin), (axis.Y * axis.Y * invCos + cosv), (yzInv + xSin),
				(xzInv + ySin), (yzInv - xSin), (axis.Z * axis.Z * invCos + cosv),
				0.f, 0.f, 0.f);
			*/
		}

		inline void addRotationAxis(const vector3d<T>& axis, T radian)
		{
			matrix43<T> matrix;
			matrix.setRotationAxis(axis, radian);
			(*this) *= matrix;
		}

		inline void getRotationAxis(vector3d<T>* axis, T* radian) const
		{
			T radianResult = acos(0.5f * ((m00 + m11 + m22) - 1.f));
			*radian = radianResult;
			if(radianResult > 0.f)
			{
				if(radianResult < 3.141592653589793f)
				{
					axis->set(m21 - m12, m02 - m20, m10 - m01);
					axis->normalize();
				}
				else
				{
					if(m00 >= m11)
					{
						if(m00 >= m22)
						{
							axis->X = 0.5f * sqrtf(m00 - m11 - m22 + 1.f);
							T halfInverse = 0.5f / axis->X;
							axis->Y = halfInverse * m01;
							axis->Z = halfInverse * m02;
						}
						else
						{
							axis->Z = 0.5f * sqrtf(m22 - m00 - m11 + 1.f);
							T halfInverse = 0.5f / axis->Z;
							axis->X = halfInverse * m02;
							axis->Y = halfInverse * m12;
						}
					}
					else
					{
						if(m11 >= m22)
						{
							axis->Y = 0.5f * sqrtf(m11 - m00 - m22 + 1.f);
							T halfInverse = 0.5f / axis->Y;
							axis->X = halfInverse * m01;
							axis->Z = halfInverse * m12;
						}
						else
						{
							axis->Z = 0.5f * sqrtf(m22 - m00 - m11 + 1.f);
							T halfInverse = 0.5f / axis->Z;
							axis->X = halfInverse * m02;
							axis->Y = halfInverse * m12;
						}
					}
				}
			}
			else
			{
				axis->set(1.f, 0.f, 0.f);
			}
		}

		//q should be normalized
		inline void setRotationQuaternion(const quat<T>& q)
		{
			T x2 = q.X + q.X;
			T y2 = q.Y + q.Y;
			T z2 = q.Z + q.Z;
			T xx2 = q.X * x2;
			T xy2 = q.X * y2;
			T xz2 = q.X * z2;
			T yy2 = q.Y * y2;
			T yz2 = q.Y * z2;
			T zz2 = q.Z * z2;
			T wx2 = q.W * x2;
			T wy2 = q.W * y2;
			T wz2 = q.W * z2;
			m00 = 1.f - (yy2 + zz2);
			m10 = xy2 + wz2;
			m20 = xz2 - wy2;

			m01 = xy2 - wz2;
			m11 = 1.f - (xx2 + zz2);
			m21 = yz2 + wx2;

			m02 = xz2 + wy2;
			m12 = yz2 - wx2;
			m22 = 1.f - (xx2 + yy2);

			m03 = m13 = m23 = 0.f;
		}

		inline void addRotationQuaternion(const quat<T>& q)
		{
			matrix43<T> matrix;
			matrix.setRotationQuaternion(q);
			(*this) *= matrix;
		}

		//只在这个matrix的3x3部分是一个纯粹的旋转矩阵(没有scale)时结果正确
		//返回结果不能保证是normalize的
		inline quat<T> getRotationQuaternion() const
		{
			quat<T> q;

			float trace = m00 + m11 + m22 + 1.0f;
			if( trace > 0.1f) 
			{
				float s = 0.5f / sqrtf(trace);
				q.W = 0.25f / s;
				q.X = ( m21 - m12 ) * s;
				q.Y = ( m02 - m20 ) * s;
				q.Z = ( m10 - m01) * s;
			} 
			else 
			{
				if ( m00 > m11 && m00 > m22 ) {
					float s = 2.0f * sqrtf( 1.0f + m00 - m11 - m22);
					q.W = (m21 - m12 ) / s;
					q.X = 0.25f * s;
					q.Y = (m01 + m10 ) / s;
					q.Z = (m02 + m20 ) / s;
				} else if (m11 > m22) {
					float s = 2.0f * sqrtf( 1.0f + m11 - m00 - m22);
					q.W = (m02 - m20 ) / s;
					q.X = (m01 + m10 ) / s;
					q.Y = 0.25f * s;
					q.Z = (m12 + m21 ) / s;
				} else {
					float s = 2.0f * sqrtf( 1.0f + m22 - m00 - m11 );
					q.W = (m10 - m01 ) / s;
					q.X = (m02 + m20 ) / s;
					q.Y = (m12 + m21 ) / s;
					q.Z = 0.25f * s;
				}
			}

			return q;
		}

		inline void setRotationXYZ(const vector3d<T>& radian)
		{
			T sinX = sinf(radian.X);
			T cosX = cosf(radian.X);
			T sinY = sinf(radian.Y);
			T cosY = cosf(radian.Y);
			T sinZ = sinf(radian.Z);
			T cosZ = cosf(radian.Z);
			m00 = cosY * cosZ;
			m10 = cosY * sinZ;
			m20 = -sinY;
			m01 = sinX * sinY * cosZ - cosX * sinZ;
			m11 = sinX * sinY * sinZ + cosX * cosZ;
			m21 = sinX * cosY;
			m02 = cosX * sinY * cosZ + sinX * sinZ;
			m12 = cosX * sinY * sinZ - sinX * cosZ;
			m22 = cosX * cosY;
			m03 = 0.f;
			m13 = 0.f;
			m23 = 0.f;
		}

		inline void addRotationXYZ(const vector3d<T>& radian)
		{
			matrix43<T> matrix;
			matrix.setRotationXYZ(radian);
			(*this) *= matrix; 
		}

		inline bool getRotationXYZ(vector3d<T>* radian) const
		{
			T yRadian = asin(-m20);
			radian->Y = yRadian;
			if(yRadian < 1.57079632679489661923f)
			{
				if(yRadian > -1.57079632679489661923f)
				{
					radian->X = atan2f(m21, m22);
					radian->Z = atan2f(m10, m00);
					return true;
				}
				else
				{
					radian->X = -atan2f(m01, m11);
					radian->Z = 0.f;
					return false;
				}
			}
			else
			{
				radian->X = atan2f(m01, m11);
				radian->Z = 0.f;
				return false;
			}
		}

		inline vector3d<T>*getRow(int iRow)
		{
			return (vector3d<T>*)(&m00+iRow*3);
		}

		inline vector3d<T> getTranslation()
		{
			return vector3d<T>(m03,m13,m23);
		}
		inline vector3d<T>*getTranslationP()
		{
			return (vector3d<T>*)&m03;
		}

		inline void setTranslation(T translationX, T translationY, T translationZ)
		{
			set(1.f, 0.f, 0.f,
				0.f, 1.f, 0.f,
				0.f, 0.f, 1.f,
				translationX, translationY, translationZ);
		}

		inline void setTranslation(const vector3d<T>& translation)
		{
			setTranslation(translation.X, translation.Y, translation.Z);
		}

		inline void addTranslation(T translationX, T translationY, T translationZ)
		{
			m03+=translationX;
			m13+=translationY;
			m23+=translationZ;
		}

		inline void addTranslation(const vector3d<T>& translation)
		{
			m03+=translation.X;
			m13+=translation.Y;
			m23+=translation.Z;
		}

		inline void setTransform(const vector3d<T>& scale,const vector3d<T>& radian, const vector3d<T>& translation)
		{
			T sinX = sinf(radian.X);
			T cosX = cosf(radian.X);
			T sinY = sinf(radian.Y);
			T cosY = cosf(radian.Y);
			T sinZ = sinf(radian.Z);
			T cosZ = cosf(radian.Z);
			m00 = scale.X * (cosY * cosZ);
			m10 = scale.X * (cosY * sinZ);
			m20 = scale.X * (-sinY);
			m01 = scale.Y * (sinX * sinY * cosZ - cosX * sinZ);
			m11 = scale.Y * (sinX * sinY * sinZ + cosX * cosZ);
			m21 = scale.Y * (sinX * cosY);
			m02 = scale.Z * (cosX * sinY * cosZ + sinX * sinZ);
			m12 = scale.Z * (cosX * sinY * sinZ - sinX * cosZ);
			m22 = scale.Z * (cosX * cosY);
			m03 = translation.X;
			m13 = translation.Y;
			m23 = translation.Z;
		}

		inline void addTransform(const vector3d<T>& scale,const vector3d<T>& radian, const vector3d<T>& translation)
		{
			matrix43<T> matrix;
			matrix.setTransformationXYZ(scale, radian, translation);
			(*this) *= matrix; 
		}


		//q should be normilized
		inline void setTransform(const vector3d<T>& scale,const quat<T>& q, const vector3d<T>& translation)
		{
			T x2 = q.X + q.X;
			T y2 = q.Y + q.Y;
			T z2 = q.Z + q.Z;
			T xx2 = q.X * x2;
			T xy2 = q.X * y2;
			T xz2 = q.X * z2;
			T yy2 = q.Y * y2;
			T yz2 = q.Y * z2;
			T zz2 = q.Z * z2;
			T wx2 = q.W * x2;
			T wy2 = q.W * y2;
			T wz2 = q.W * z2;
			m00 = scale.X * (1.f - (yy2 + zz2));
			m10 = scale.X * (xy2 + wz2);
			m20 = scale.X * (xz2 - wy2);
			m01 = scale.Y * (xy2 - wz2);
			m11 = scale.Y * (1.f - (xx2 + zz2));
			m21 = scale.Y * (yz2 + wx2);
			m02 = scale.Z * (xz2 + wy2);
			m12 = scale.Z * (yz2 - wx2);
			m22 = scale.Z * (1.f - (xx2 + yy2));
			m03 = translation.X;
			m13 = translation.Y;
			m23 = translation.Z;
		}

		inline void addTransform(const vector3d<T>& scale,const quat<T>& q, const vector3d<T>& translation)
		{
			matrix43<T> matrix;
			matrix.setTransform(scale, q, translation);
			(*this) *= matrix; 
		}


		void buildCameraLookAtMatrixLH(const i_math::vector3d<T> &xaxis,
			const i_math::vector3d<T> &yaxis,const i_math::vector3d<T> &zaxis,
			const i_math::vector3d<T> &o)
		{
			m00= xaxis.X;
			m10= yaxis.X;
			m20= zaxis.X;

			m01= xaxis.Y;
			m11= yaxis.Y;
			m21= zaxis.Y;

			m02 = xaxis.Z;
			m12 = yaxis.Z;
			m22 = zaxis.Z;

			m03 = -xaxis.dotProduct(o);
			m13 = -yaxis.dotProduct(o);
			m23 = -zaxis.dotProduct(o);
		}

		void buildMatrixLH(const i_math::vector3d<T> &xaxis,
			const i_math::vector3d<T> &yaxis,const i_math::vector3d<T> &zaxis,
			const i_math::vector3d<T> &o)
		{
			buildCameraLookAtMatrixLH(xaxis,yaxis,zaxis,o);
			makeInverse();
		}

		void buildMatrixLH2(const i_math::vector3d<T> &xAxis,
			const i_math::vector3d<T> &yAxis,const i_math::vector3d<T> &zAxis,
			const i_math::vector3d<T> &o)
		{
			m00= zAxis.X;
			m10= zAxis.Y;
			m20= zAxis.Z;

			m01= xAxis.X;
			m11= xAxis.Y;
			m21= xAxis.Z;

			m02 = yAxis.X;
			m12 = yAxis.Y;
			m22 = yAxis.Z;

			m03=o.X;
			m13=o.Y;
			m23=o.Z;
		}

		void buildFromPosDir(i_math::vector3df &pos,i_math::vector3df &dir)
		{
			i_math::vector3df vUp(0,1,0);
			i_math::vector3df axisX=vUp.crossProduct(dir);
			axisX.normalize();
			i_math::vector3df axisZ=dir;

			i_math::vector3df axisY=axisZ.crossProduct(axisX);
			axisY.normalize();

			buildMatrixLH(axisX,axisY,axisZ,pos);
		}


		inline matrix43 operator *(const matrix43& mtx) const
		{
			return matrix43(
				(m00 * mtx.m00) + (m10 * mtx.m01) + (m20 * mtx.m02),
				(m00 * mtx.m10) + (m10 * mtx.m11) + (m20 * mtx.m12),
				(m00 * mtx.m20) + (m10 * mtx.m21) + (m20 * mtx.m22),
				(m01 * mtx.m00) + (m11 * mtx.m01) + (m21 * mtx.m02),
				(m01 * mtx.m10) + (m11 * mtx.m11) + (m21 * mtx.m12),
				(m01 * mtx.m20) + (m11 * mtx.m21) + (m21 * mtx.m22),
				(m02 * mtx.m00) + (m12 * mtx.m01) + (m22 * mtx.m02),
				(m02 * mtx.m10) + (m12 * mtx.m11) + (m22 * mtx.m12),
				(m02 * mtx.m20) + (m12 * mtx.m21) + (m22 * mtx.m22),
				(m03 * mtx.m00) + (m13 * mtx.m01) + (m23 * mtx.m02) + mtx.m03,
				(m03 * mtx.m10) + (m13 * mtx.m11) + (m23 * mtx.m12) + mtx.m13,
				(m03 * mtx.m20) + (m13 * mtx.m21) + (m23 * mtx.m22) + mtx.m23);
		}

		inline matrix43& operator *=(matrix43 mtx)
		{
			T old00 = m00;
			T old10 = m10;
			m00 = (old00 * mtx.m00) + (old10 * mtx.m01) + (m20 * mtx.m02);
			m10 = (old00 * mtx.m10) + (old10 * mtx.m11) + (m20 * mtx.m12);
			m20 = (old00 * mtx.m20) + (old10 * mtx.m21) + (m20 * mtx.m22);
			T old01 = m01;
			T old11 = m11;
			m01 = (old01 * mtx.m00) + (old11 * mtx.m01) + (m21 * mtx.m02);
			m11 = (old01 * mtx.m10) + (old11 * mtx.m11) + (m21 * mtx.m12);
			m21 = (old01 * mtx.m20) + (old11 * mtx.m21) + (m21 * mtx.m22);
			T old02 = m02;
			T old12 = m12;
			m02 = (old02 * mtx.m00) + (old12 * mtx.m01) + (m22 * mtx.m02);
			m12 = (old02 * mtx.m10) + (old12 * mtx.m11) + (m22 * mtx.m12);
			m22 = (old02 * mtx.m20) + (old12 * mtx.m21) + (m22 * mtx.m22);
			T old03 = m03;
			T old13 = m13;
			m03 = (old03 * mtx.m00) + (old13 * mtx.m01) + (m23 * mtx.m02) + mtx.m03;
			m13 = (old03 * mtx.m10) + (old13 * mtx.m11) + (m23 * mtx.m12) + mtx.m13;
			m23 = (old03 * mtx.m20) + (old13 * mtx.m21) + (m23 * mtx.m22) + mtx.m23;
			return *this;
		}

		//in 和 out 可以为同一个vector3d<T>
		inline void transformVect(const vector3d<T>& in,vector3d<T> &out) const
		{
			out.set(
				in.X * m00 + in.Y * m01 + in.Z * m02 + m03,
				in.X * m10 + in.Y * m11 + in.Z * m12 + m13,
				in.X * m20 + in.Y * m21 + in.Z * m22 + m23);
		}

		//using 3x3 part
		inline void rotateVect(const vector3d<T>& in,vector3d<T>& out) const
		{
			return  out.set(
				in.X * m00 + in.Y * m01 + in.Z * m02,
				in.X * m10 + in.Y * m11 + in.Z * m12,
				in.X * m20 + in.Y * m21 + in.Z * m22);
		}

		inline void transformSphere(const sphere<T>&in,sphere<T>&out) const
		{
			transformVect(in.center,out.center);

			float scale=(T)(((vector3d<T>*)&m00)->getLength());
			out.radius=in.radius*scale;
		}

		inline void transformCapsule(const capsule<T>&in,capsule<T>&out) const
		{
			transformVect(in.start,out.start);
			transformVect(in.end,out.end);

			float scale=(T)(((vector3d<T>*)&m00)->getLength());
			out.radius=in.radius*scale;
		}


		//! Transforms a plane by this matrix
		inline void transformPlane( plane3d<T> &plane) const
		{
			vector3d<T> member;
			transformVect(plane.getMemberPoint(), member);

			vector3d<T> nNew;
			rotateVect(plane.Normal,nNew);

			plane.Normal=nNew;
			plane.D = member.dotProduct(nNew);
		}

		//! Transforms a plane by this matrix
		inline void transformPlane( const plane3d<T> &in, plane3d<T> &out) const
		{
			out = in;
			transformPlane( out );
		}

		//! Transforms a axis aligned bounding box
		/** The result box of this operation may not be very accurate. For
		accurate results, use transformBoxEx() */
// 		void transformBox(aabbox3d<T>& box) const
// 		{
// 			transformVect(box.MinEdge,box.MinEdge);
// 			transformVect(box.MaxEdge,box.MaxEdge);
// 			box.repair();
// 		}

		//! Transforms a axis aligned bounding box more accurately than transformBox()
		/** The result box of this operation should by quite accurate, but this operation
		is slower than transformBox(). */
		void transformBoxEx(aabbox3d<T>& box) const
		{
			vector3d<T> corners[8],cornersT[8];
			box.getCorners(corners);

			int i;
			for (i=0; i<8; ++i)
				transformVect(corners[i],cornersT[i]);

			box.reset(cornersT[0]);

			for (i=1; i<8; ++i)
				box.addInternalPoint(cornersT[i]);
		}

		void transformLine(line3d<T>&line)const
		{
			transformVect(line.start);
			transformVect(line.end);
		}

		void transformLine(line3d<T>&in,line3d<T>&out)const
		{
			transformVect(in.start,out.start);
			transformVect(in.end,out.end);
		}



		inline matrix43 operator *(T value) const
		{
			return matrix43(
				m00 * value, m10 * value, m20 * value,
				m01 * value, m11 * value, m21 * value,
				m02 * value, m12 * value, m22 * value,
				m03 * value, m13 * value, m23 * value);
		}

		inline matrix43& operator *=(T value)
		{
			m00 *= value;
			m10 *= value;
			m20 *= value;
			m01 *= value;
			m11 *= value;
			m21 *= value;
			m02 *= value;
			m12 *= value;
			m22 *= value;
			m03 *= value;
			m13 *= value;
			m23 *= value;
			return *this;
		}

		inline void transpose()
		{
			T swap;
			swap = m10; m10 = m01; m01 = swap;
			swap = m20; m20 = m02; m02 = swap;
			swap = m21; m21 = m12; m12 = swap;
			m03 = m13 = m23 = 0.f;
		}

		inline T determinant() const
		{
			return
				m00 * (m11 * m22 - m21 * m12) -
				m10 * (m01 * m22 - m21 * m02) +
				m20 * (m01 * m12 - m11 * m02);
		}

		bool makeInverse()
		{
			matrix43<T> temp;

			if (getInverse(&temp))
			{
				*this = temp;
				return true;
			}

			return false;
		}

		inline bool getInverse(matrix43* invertMatrix) const
		{
			invertMatrix->m00 = m11 * m22 - m21 * m12;
			invertMatrix->m01 = -(m01 * m22 - m21 * m02);
			invertMatrix->m02 = m01 * m12 - m11 * m02;
			T determ =
				m00 * invertMatrix->m00 +
				m10 * invertMatrix->m01 +
				m20 * invertMatrix->m02;
			if (determ==0.0f)
				return false;
			invertMatrix->m10 = -(m10 * m22 - m20 * m12);
			invertMatrix->m20 = m10 * m21 - m20 * m11;
			invertMatrix->m11 = m00 * m22 - m20 * m02;
			invertMatrix->m21 = -(m00 * m21 - m20 * m01);
			invertMatrix->m12 = -(m00 * m12 - m10 * m02);
			invertMatrix->m22 = m00 * m11 - m10 * m01;
			invertMatrix->m03 = -(
				m01 * (m12 * m23 - m13 * m22) -
				m11 * (m02 * m23 - m03 * m22) +
				m21 * (m02 * m13 - m03 * m12));
			invertMatrix->m13 =
				m00 * (m12 * m23 - m13 * m22) -
				m10 * (m02 * m23 - m03 * m22) +
				m20 * (m02 * m13 - m03 * m12);
			invertMatrix->m23 = -(
				m00 * (m11 * m23 - m13 * m21) -
				m10 * (m01 * m23 - m03 * m21) +
				m20 * (m01 * m13 - m03 * m11));
			T invDeterm = 1.f / determ;
			(*invertMatrix) *= invDeterm;
			return true;
		}

		inline bool operator ==(const matrix43& target) const
		{
			return (
				(m00 == target.m00) && (m01 == target.m01) && (m02 == target.m02) &&
				(m10 == target.m10) && (m11 == target.m11) && (m12 == target.m12) &&
				(m20 == target.m20) && (m21 == target.m21) && (m22 == target.m22) &&
				(m03 == target.m03) && (m13 == target.m13) && (m23 == target.m23));
		}

		inline bool equals(const matrix43& target) const
		{
			return 
				(i_math::equals(m00 ,target.m00) &&
				i_math::equals(m10 ,target.m10) &&
				i_math::equals(m20 ,target.m20) &&
				i_math::equals(m03 ,target.m03) &&
				i_math::equals(m01 ,target.m01) &&
				i_math::equals(m11 ,target.m11) &&
				i_math::equals(m21 ,target.m21) &&
				i_math::equals(m13 ,target.m13) &&
				i_math::equals(m02 ,target.m02) &&
				i_math::equals(m12 ,target.m12) &&
				i_math::equals(m22 ,target.m22) &&
				i_math::equals(m23 ,target.m23));
		}

		inline bool equalsIdentity() const
		{
			return 
				(i_math::equals(m00 ,1) &&
				i_math::equals(m10 ,0) &&
				i_math::equals(m20 ,0) &&
				i_math::equals(m01 ,0) &&
				i_math::equals(m11 ,1) &&
				i_math::equals(m21 ,0) &&
				i_math::equals(m02 ,0) &&
				i_math::equals(m12 ,0) &&
				i_math::equals(m22 ,1) &&
				i_math::equals(m03 ,0) &&
				i_math::equals(m13 ,0) &&
				i_math::equals(m23 ,0));
		}


		inline bool operator !=(const matrix43& target) const
		{
			return (
				(m00 != target.m00) || (m01 != target.m01) || (m02 != target.m02) ||
				(m10 != target.m10) || (m11 != target.m11) || (m12 != target.m12) ||
				(m20 != target.m20) || (m21 != target.m21) || (m22 != target.m22) ||
				(m03 != target.m03) || (m13 != target.m13) || (m23 != target.m23));
		}


		inline void toString(std::string &s) const
		{
			char buffer[256];
			sprintf(buffer,
				"{\n  ( %.8f, %.8f, %.8f )\n  ( %.8f, %.8f, %.8f )\n"
				"  ( %.8f, %.8f, %.8f )\n  ( %.8f, %.8f, %.8f )\n}",
				m00, m10, m20, m01, m11, m21, m02, m12, m22, m03, m13, m23);
			s=buffer;
		}

		static matrix43<T> *identity()
		{
			static matrix43<T> t;
			return &t;
		}
	};


	typedef matrix43<f32> matrix43f;
	typedef matrix43<s32> matrix43i;



}

#define mat43from44(_mat43,_matSrc)\
{\
	(_mat43).set(\
	_matSrc.m00, _matSrc.m01, _matSrc.m02,\
	_matSrc.m10, _matSrc.m11, _matSrc.m12,\
	_matSrc.m20, _matSrc.m21, _matSrc.m22,\
	_matSrc.m30, _matSrc.m31, _matSrc.m32);\
}
