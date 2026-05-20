#pragma once

#include "iTypes.h"
#include "vector3d.h"
#include "vector4d.h"
#include "plane3d.h"
#include "aabbox3d.h"

namespace i_math
{
	template <class T>
		class matrix43;
	//! 4x4 matrix. Mostly used as transformation matrix for 3d calculations.
	//The matrix is a D3D style matrix, row major with translations in the 4th row.
	template<class T>
	class matrix44
	{
		public:

			//! Constructor
			matrix44()
			{
				makeIdentity();
			}

			//! Simple operator for directly accessing every element of the matrix.
			T& operator()(s32 row, s32 col) { return M[col * 4 + row]; }

			//! Simple operator for directly accessing every element of the matrix.
			const T& operator()(s32 row, s32 col) const {  return M[col * 4 + row]; }

			inline void set(
				T v00, T v01, T v02,T v03,
				T v10, T v11, T v12,T v13,
				T v20, T v21, T v22,T v23,
				T v30, T v31, T v32,T v33)
			{
				m00 = v00; m01 = v01; m02 = v02;m03=v03;
				m10 = v10; m11 = v11; m12 = v12;m13=v13;
				m20 = v20; m21 = v21; m22 = v22;m23=v23;
				m30 = v30; m31 = v31; m32 = v32;m33=v33;
			}



			//! Sets this matrix equal to the other matrix.
			matrix44<T>& operator=(const matrix44<T> &other)
			{
				for (s32 i = 0; i < 16; ++i)
					M[i] = other.M[i];

				return *this;
			}

			//! Returns true if other matrix is equal to this matrix.
			bool operator==(const matrix44<T> &other) const
			{
				for (s32 i = 0; i < 16; ++i)
					if (M[i] != other.M[i])
						return false;

				return true;
			}

			//! Returns true if other matrix is not equal to this matrix.
			bool operator!=(const matrix44<T> &other) const
			{
				return !(*this == other);
			}

			//! Multiply by another matrix.
			matrix44<T>& operator*=(const matrix44<T>& other)
			{
				T newMatrix[16];
				const T *m2 = M, *m1 = other.M;

				newMatrix[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
				newMatrix[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
				newMatrix[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
				newMatrix[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];

				newMatrix[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
				newMatrix[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
				newMatrix[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
				newMatrix[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];

				newMatrix[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
				newMatrix[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
				newMatrix[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
				newMatrix[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];

				newMatrix[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
				newMatrix[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
				newMatrix[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
				newMatrix[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];

				for (s32 i=0; i<16; ++i)
					M[i] = newMatrix[i];

				return *this;
			}

			//! Multiply by another matrix.
			matrix44<T> operator*(const matrix44<T>& other) const
			{
				matrix44<T> tmtrx;
				const T *m2 = M, *m1 = other.M;
				T *m3 = tmtrx.M;

				m3[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
				m3[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
				m3[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
				m3[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];

				m3[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
				m3[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
				m3[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
				m3[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];

				m3[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
				m3[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
				m3[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
				m3[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];

				m3[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
				m3[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
				m3[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
				m3[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];

				return tmtrx;
			}

			//! Set matrix to identity. 
			void makeIdentity()
			{
				for (s32 i=0; i<16; ++i)
					M[i] = 0;
				M[0] = M[5] = M[10] = M[15] = 1;
			}

			//! Returns true if the matrix is the identity matrix
			bool isIdentity()
			{
				for (s32 i=0; i<4; ++i)
				for (s32 j=0; j<4; ++j)
					if (j != i)
					{
						if ((*this)(i,j) < -0.0001f ||
							(*this)(i,j) >  0.0001f)
							return false;
					}
					else
					{
						if ((*this)(i,j) < 0.9999f ||
							(*this)(i,j) > 1.0001f)
							return false;
					}

				return true;
			}

			//! Set the translation of the current matrix. Will erase any previous values.
			void setTranslation( const vector3d<T>& translation )
			{
				M[12] = translation.X;
				M[13] = translation.Y;
				M[14] = translation.Z;
			}
			//! Gets the current translation
			vector3d<T> getTranslation() const
			{
				return vector3d<T>(M[12], M[13], M[14]);
			}

			//! Set the inverse translation of the current matrix. Will erase any previous values.
			void setInverseTranslation( const vector3d<T>& translation )
			{
				M[12] = -translation.X;
				M[13] = -translation.Y;
				M[14] = -translation.Z;
			}

			//! Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
			void setRotationRadians( const vector3d<T>& rotation )
			{
				f64 cr = (f64)cosf( rotation.X );
				f64 sr = (f64)sinf( rotation.X );
				f64 cp = (f64)cosf( rotation.Y );
				f64 sp = (f64)sinf( rotation.Y );
				f64 cy = (f64)cosf( rotation.Z );
				f64 sy = (f64)sinf( rotation.Z );

				M[0] = (T)( cp*cy );
				M[1] = (T)( cp*sy );
				M[2] = (T)( -sp );

				f64 srsp = sr*sp;
				f64 crsp = cr*sp;

				M[4] = (T)( srsp*cy-cr*sy );
				M[5] = (T)( srsp*sy+cr*cy );
				M[6] = (T)( sr*cp );

				M[8] = (T)( crsp*cy+sr*sy );
				M[9] = (T)( crsp*sy-sr*cy );
				M[10] = (T)( cr*cp );
			}

			//! Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
			void setRotationDegrees( const vector3d<T>& rotation )
			{
				setRotationRadians( rotation * (T)3.1415926535897932384626433832795 / 180.0 );
			}

			//! Returns the rotation, as set by setRotation(). This code was orginally written by by Chev.
			vector3d<T> getRotationDegrees() const
			{
				const matrix44<T> &mat = *this; 

				f64 Y = (f64)-asin(mat(2,0)); 
				f64 D = (f64)Y; 
				f64 C = (f64)cosf(Y); 
				Y *= GRAD_PI; 

				f64 rotx, roty, X, Z; 

				if (fabsf(C)>0.0005f)  
				{ 
					rotx = mat(2,2) / C; 
					roty = mat(2,1)  / C; 
					X = atan2f( roty, rotx ) * (T)GRAD_PI; 
					rotx = mat(0,0) / C; 
					roty = mat(1,0) / C; 
					Z = atan2f( roty, rotx ) * (T)GRAD_PI; 
				} 
				else 
				{ 
					X  = 0.0f; 
					rotx = mat(1,1); 
					roty = -mat(0,1); 
					Z  = atan2f( roty, rotx ) * (T)GRAD_PI; 
				} 

				// fix values that get below zero 
				// before it would set (!) values to 360 
				// that where above 360: 
				if (X < 0.00) X += 360.00; 
				if (Y < 0.00) Y += 360.00; 
				if (Z < 0.00) Z += 360.00; 

				return vector3d<T>((T)X,(T)Y,(T)Z);

			}

			//! Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
			void setInverseRotationRadians( const vector3d<T>& rotation )
			{
				f64 cr = cosf( rotation.X );
				f64 sr = sinf( rotation.X );
				f64 cp = cosf( rotation.Y );
				f64 sp = sinf( rotation.Y );
				f64 cy = cosf( rotation.Z );
				f64 sy = sinf( rotation.Z );

				M[0] = (T)( cp*cy );
				M[4] = (T)( cp*sy );
				M[8] = (T)( -sp );

				f64 srsp = sr*sp;
				f64 crsp = cr*sp;

				M[1] = (T)( srsp*cy-cr*sy );
				M[5] = (T)( srsp*sy+cr*cy );
				M[9] = (T)( sr*cp );

				M[2] = (T)( crsp*cy+sr*sy );
				M[6] = (T)( crsp*sy-sr*cy );
				M[10] = (T)( cr*cp );

			}

			//! Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
			void setInverseRotationDegrees( const vector3d<T>& rotation )
			{
				setInverseRotationRadians( rotation * (T)3.1415926535897932384626433832795 / 180.0 );
			}

			//! Set Scale
			void setScale( const vector3d<T>& scale )
			{
				M[0] = scale.X;
				M[5] = scale.Y;
				M[10] = scale.Z;
			}
			
			//! Translate a vector by the inverse of the translation part of this matrix.
			void inverseTranslateVect( vector3d<T>& vect ) const
			{
				vect.X = vect.X-M[12];
				vect.Y = vect.Y-M[13];
				vect.Z = vect.Z-M[14];
			}

			//! Rotate a vector by the inverse of the rotation part of this matrix.
			void inverseRotateVect( vector3d<T>& vect ) const
			{
				vector3d<T> tmp = vect;
				vect.X = tmp.X*M[0] + tmp.Y*M[1] + tmp.Z*M[2];
				vect.Y = tmp.X*M[4] + tmp.Y*M[5] + tmp.Z*M[6];
				vect.Z = tmp.X*M[8] + tmp.Y*M[9] + tmp.Z*M[10];
			}

			//! Rotate a vector by the rotation part of this matrix.
			void rotateVect( vector3d<T>& vect ) const
			{
				vector3d<T> tmp = vect;
				vect.X = tmp.X*M[0] + tmp.Y*M[4] + tmp.Z*M[8];
				vect.Y = tmp.X*M[1] + tmp.Y*M[5] + tmp.Z*M[9];
				vect.Z = tmp.X*M[2] + tmp.Y*M[6] + tmp.Z*M[10]; 
			}

			//! Transforms the vector by this matrix
			void transformVect( vector3d<T>& vect) const
			{
				vector3d<T>vec;
				T w;

				vec.X	= vect.X*M[0] + vect.Y*M[4] + vect.Z*M[8] + M[12];
				vec.Y	= vect.X*M[1] + vect.Y*M[5] + vect.Z*M[9] + M[13];
				vec.Z	= vect.X*M[2] + vect.Y*M[6] + vect.Z*M[10] + M[14];
				w			= vect.X*M[3] + vect.Y*M[7] + vect.Z*M[11] + M[15];

				vect=vec;
				if (!i_math::equals(w,0))
					vect/=w;
			}

			//! Transforms input vector by this matrix and stores result in output vector 
			void transformVect( const vector3d<T>& in, vector3d<T>& out) const
			{
				T w;
				i_math::vector3df o;
				o.X = in.X*M[0] + in.Y*M[4] + in.Z*M[8] + M[12];
				o.Y = in.X*M[1] + in.Y*M[5] + in.Z*M[9] + M[13];
				o.Z = in.X*M[2] + in.Y*M[6] + in.Z*M[10] + M[14];
				w       = in.X*M[3] + in.Y*M[7] + in.Z*M[11] + M[15];
				out=o;
				if (!i_math::equals(w,0))
					out/=w;
			}

			void transformVect(const vector4d<T>& in,vector4d<T>& out)const
			{
				i_math::vector4df o;
				o.x	= in.x*M[0] + in.y*M[4] + in.z*M[8] + in.w*M[12];
				o.y	= in.x*M[1] + in.y*M[5] + in.z*M[9] + in.w*M[13];
				o.z	= in.x*M[2] + in.y*M[6] + in.z*M[10] +in.w*M[14];
				o.w	= in.x*M[3] + in.y*M[7] + in.z*M[11] + in.w*M[15];
				out=o;
			}

			void transformVect(vector4d<T>& vec)const
			{
				i_math::vector4d<T> t;
				transformVect(vec,t);
				vec=t;
			}

			//! Translate a vector by the translation part of this matrix.
			void translateVect( vector3d<T>& vect ) const
			{
				vect.X = vect.X+M[12];
				vect.Y = vect.Y+M[13];
				vect.Z = vect.Z+M[14];
			}

			//! Transforms a plane by this matrix
			void transformPlane( plane3d<T> &plane) const
			{
				vector3d<T> member;
				transformVect(plane.getMemberPoint(), member);

				vector3d<T> origin(0,0,0);
				transformVect(plane.Normal);
				transformVect(origin);

				plane.Normal -= origin;
				plane.Normal.normalize();
				plane.D = member.dotProduct(plane.Normal);

			}

			//! Transforms a plane by this matrix
			void transformPlane( const plane3d<T> &in, plane3d<T> &out) const
			{
				out = in;
				transformPlane( out );
			}

			void transformPlaneEquation(const vector4d<T>& in,vector4d<T>& out)const
			{
				matrix44<T> t,t2;
				getInverse(t);
				t.getTransposed(t2);
				transformVect(in,out);
			}

			void transformPlaneEquation(vector4d<T>& vec)const
			{
				matrix44<T> t,t2;
				getInverse(t);
				t.getTransposed(t2);
				transformVect(vec);
			}


			//! Transforms a axis aligned bounding box
			/** The result box of this operation may not be very accurate. For
			accurate results, use transformBoxEx() */
			void transformBox(aabbox3d<T>& box) const
			{
				transformVect(box.MinEdge);
				transformVect(box.MaxEdge);
				box.repair();
			}

			//! Transforms a axis aligned bounding box more accurately than transformBox()
			/** The result box of this operation should by quite accurate, but this operation
			is slower than transformBox(). */
			void transformBoxEx(aabbox3d<T>& box) const
			{
				vector3d<T> corners[8];
				box.getCorners(corners);

				int i;
				for (i=0; i<8; ++i)
					transformVect(corners[i]);

				box.reset(corners[0]);

				for (i=1; i<8; ++i)
					box.addInternalPoint(corners[i]);
			}

			//! Multiplies this matrix by a 1x4 matrix
			void multiplyWith1x4Matrix(T* matrix) const
			{
				/*
				0  1  2  3
				4  5  6  7
				8  9  10 11
				12 13 14 15
				*/

				T mat[4];
				mat[0] = matrix[0];
				mat[1] = matrix[1];
				mat[2] = matrix[2];
				mat[3] = matrix[3];

				matrix[0] = M[0]*mat[0] + M[4]*mat[1] + M[8]*mat[2] + M[12]*mat[3];
				matrix[1] = M[1]*mat[0] + M[5]*mat[1] + M[9]*mat[2] + M[13]*mat[3];
				matrix[2] = M[2]*mat[0] + M[6]*mat[1] + M[10]*mat[2] + M[14]*mat[3];
				matrix[3] = M[3]*mat[0] + M[7]*mat[1] + M[11]*mat[2] + M[15]*mat[3];
			}

			//! Calculates inverse of matrix. Slow.
			//! \return Returns false if there is no inverse matrix.
			bool makeInverse()
			{
				matrix44<T> temp;

				if (getInverse(temp))
				{
					*this = temp;
					return true;
				}

				return false;
			}

			//! returns the inversed matrix of this one
			//! \param Target, where result matrix is written to.
			//! \return Returns false if there is no inverse matrix.
			bool getInverse(matrix44<T>& out)
			{
				/// Calculates the inverse of this Matrix 
				/// The inverse is calculated using Cramers rule.
				/// If no inverse exists then 'false' is returned.

				const matrix44<T> &m = *this;

				T d = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3))	- (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3))
					+ (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3))	+ (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3))
					- (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3))	+ (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3));

				if (d == 0.f)
					return false;

				d = 1.f / d;

				out(0, 0) = d * (m(1, 1) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3)) + m(2, 1) * (m(3, 2) * m(1, 3) - m(1, 2) * m(3, 3)) + m(3, 1) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3)));
				out(1, 0) = d * (m(1, 2) * (m(2, 0) * m(3, 3) - m(3, 0) * m(2, 3)) + m(2, 2) * (m(3, 0) * m(1, 3) - m(1, 0) * m(3, 3)) + m(3, 2) * (m(1, 0) * m(2, 3) - m(2, 0) * m(1, 3)));
				out(2, 0) = d * (m(1, 3) * (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) + m(2, 3) * (m(3, 0) * m(1, 1) - m(1, 0) * m(3, 1)) + m(3, 3) * (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)));
				out(3, 0) = d * (m(1, 0) * (m(3, 1) * m(2, 2) - m(2, 1) * m(3, 2)) + m(2, 0) * (m(1, 1) * m(3, 2) - m(3, 1) * m(1, 2)) + m(3, 0) * (m(2, 1) * m(1, 2) - m(1, 1) * m(2, 2)));
				out(0, 1) = d * (m(2, 1) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3)) + m(3, 1) * (m(2, 2) * m(0, 3) - m(0, 2) * m(2, 3)) + m(0, 1) * (m(3, 2) * m(2, 3) - m(2, 2) * m(3, 3)));
				out(1, 1) = d * (m(2, 2) * (m(0, 0) * m(3, 3) - m(3, 0) * m(0, 3)) + m(3, 2) * (m(2, 0) * m(0, 3) - m(0, 0) * m(2, 3)) + m(0, 2) * (m(3, 0) * m(2, 3) - m(2, 0) * m(3, 3)));
				out(2, 1) = d * (m(2, 3) * (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) + m(3, 3) * (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) + m(0, 3) * (m(3, 0) * m(2, 1) - m(2, 0) * m(3, 1)));
				out(3, 1) = d * (m(2, 0) * (m(3, 1) * m(0, 2) - m(0, 1) * m(3, 2)) + m(3, 0) * (m(0, 1) * m(2, 2) - m(2, 1) * m(0, 2)) + m(0, 0) * (m(2, 1) * m(3, 2) - m(3, 1) * m(2, 2)));
				out(0, 2) = d * (m(3, 1) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3)) + m(0, 1) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3)) + m(1, 1) * (m(3, 2) * m(0, 3) - m(0, 2) * m(3, 3)));
				out(1, 2) = d * (m(3, 2) * (m(0, 0) * m(1, 3) - m(1, 0) * m(0, 3)) + m(0, 2) * (m(1, 0) * m(3, 3) - m(3, 0) * m(1, 3)) + m(1, 2) * (m(3, 0) * m(0, 3) - m(0, 0) * m(3, 3)));
				out(2, 2) = d * (m(3, 3) * (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) + m(0, 3) * (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) + m(1, 3) * (m(3, 0) * m(0, 1) - m(0, 0) * m(3, 1)));
				out(3, 2) = d * (m(3, 0) * (m(1, 1) * m(0, 2) - m(0, 1) * m(1, 2)) + m(0, 0) * (m(3, 1) * m(1, 2) - m(1, 1) * m(3, 2)) + m(1, 0) * (m(0, 1) * m(3, 2) - m(3, 1) * m(0, 2)));
				out(0, 3) = d * (m(0, 1) * (m(2, 2) * m(1, 3) - m(1, 2) * m(2, 3)) + m(1, 1) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3)) + m(2, 1) * (m(1, 2) * m(0, 3) - m(0, 2) * m(1, 3)));
				out(1, 3) = d * (m(0, 2) * (m(2, 0) * m(1, 3) - m(1, 0) * m(2, 3)) + m(1, 2) * (m(0, 0) * m(2, 3) - m(2, 0) * m(0, 3)) + m(2, 2) * (m(1, 0) * m(0, 3) - m(0, 0) * m(1, 3)));
				out(2, 3) = d * (m(0, 3) * (m(2, 0) * m(1, 1) - m(1, 0) * m(2, 1)) + m(1, 3) * (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) + m(2, 3) * (m(1, 0) * m(0, 1) - m(0, 0) * m(1, 1)));
				out(3, 3) = d * (m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) + m(1, 0) * (m(2, 1) * m(0, 2) - m(0, 1) * m(2, 2)) + m(2, 0) * (m(0, 1) * m(1, 2) - m(1, 1) * m(0, 2)));

				return true;
			}

			//! Builds a right-handed perspective projection matrix based on a field of view
			void buildProjPerspectiveFovRH(T fieldOfViewRadians, T aspectRatio, T zNear, T zFar)
			{
				T h = (T)(cosf(fieldOfViewRadians/2) / sinf(fieldOfViewRadians/2));
				T w = h / aspectRatio;

				(*this)(0,0) = 2*zNear/w;
				(*this)(1,0) = 0;
				(*this)(2,0) = 0;
				(*this)(3,0) = 0;

				(*this)(0,1) = 0;
				(*this)(1,1) = 2*zNear/h;
				(*this)(2,1) = 0;
				(*this)(3,1) = 0;

				(*this)(0,2) = 0;
				(*this)(1,2) = 0;
				(*this)(2,2) = zFar/(zFar-zNear);
				(*this)(3,2) = -1;

				(*this)(0,3) = 0;
				(*this)(1,3) = 0;
				(*this)(2,3) = zNear*zFar/(zNear-zFar);
				(*this)(3,3) = 0;
			}

			//! Builds a left-handed perspective projection matrix based on a field of view
			void buildProjPerspectiveFovLH(T fieldOfViewRadians, T aspectRatio, T zNear, T zFar)
			{
				T h = (T)(cosf(fieldOfViewRadians/2) / sinf(fieldOfViewRadians/2));
				T w = h / aspectRatio;

				(*this)(0,0) = w;
				(*this)(1,0) = 0;
				(*this)(2,0) = 0;
				(*this)(3,0) = 0;

				(*this)(0,1) = 0;
				(*this)(1,1) = h;
				(*this)(2,1) = 0;
				(*this)(3,1) = 0;

				(*this)(0,2) = 0;
				(*this)(1,2) = 0;
				(*this)(2,2) = zFar/(zFar-zNear);
				(*this)(3,2) = 1;

				(*this)(0,3) = 0;
				(*this)(1,3) = 0;
				(*this)(2,3) = zNear*zFar/(zNear-zFar);
				(*this)(3,3) = 0;
			}

			//! Builds a right-handed perspective projection matrix.
			void buildProjPerspectiveRH(T widthOfViewVolume, T heightOfViewVolume, T zNear, T zFar)
			{
				(*this)(0,0) = 2*zNear/widthOfViewVolume;
				(*this)(1,0) = 0;
				(*this)(2,0) = 0;
				(*this)(3,0) = 0;

				(*this)(0,1) = 0;
				(*this)(1,1) = 2*zNear/heightOfViewVolume;
				(*this)(2,1) = 0;
				(*this)(3,1) = 0;

				(*this)(0,2) = 0;
				(*this)(1,2) = 0;
				(*this)(2,2) = zFar/(zNear-zFar);
				(*this)(3,2) = -1;

				(*this)(0,3) = 0;
				(*this)(1,3) = 0;
				(*this)(2,3) = zNear*zFar/(zNear-zFar);
				(*this)(3,3) = 0;
			}

			//! Builds a left-handed perspective projection matrix.
			void buildProjPerspectiveLH(T widthOfViewVolume, T heightOfViewVolume, T zNear, T zFar)
			{
				(*this)(0,0) = 2*zNear/widthOfViewVolume;
				(*this)(1,0) = 0;
				(*this)(2,0) = 0;
				(*this)(3,0) = 0;

				(*this)(0,1) = 0;
				(*this)(1,1) = 2*zNear/heightOfViewVolume;
				(*this)(2,1) = 0;
				(*this)(3,1) = 0;

				(*this)(0,2) = 0;
				(*this)(1,2) = 0;
				(*this)(2,2) = zFar/(zFar-zNear);
				(*this)(3,2) = 1;

				(*this)(0,3) = 0;
				(*this)(1,3) = 0;
				(*this)(2,3) = zNear*zFar/(zNear-zFar);
				(*this)(3,3) = 0;
			}

			//! Builds a left-handed orthogonal projection matrix.
			void buildProjOrthoLH(T widthOfViewVolume, T heightOfViewVolume, T zNear, T zFar)
			{
				(*this)(0,0) = 2/widthOfViewVolume;
				(*this)(1,0) = 0;
				(*this)(2,0) = 0;
				(*this)(3,0) = 0;

				(*this)(0,1) = 0;
				(*this)(1,1) = 2/heightOfViewVolume;
				(*this)(2,1) = 0;
				(*this)(3,1) = 0;

				(*this)(0,2) = 0;
				(*this)(1,2) = 0;
				(*this)(2,2) = 1/(zFar-zNear);
				(*this)(3,2) = 0;

				(*this)(0,3) = 0;
				(*this)(1,3) = 0;
				(*this)(2,3) = zNear/(zNear-zFar);
				(*this)(3,3) = 1;
			}

			void buildProjOrthoOffCenterLH(T l, T t,T r,T b, T zn, T zf)
			{
				(*this)(0,0) = 2/(r-l);
				(*this)(1,0) = 0;
				(*this)(2,0) = 0;
				(*this)(3,0) = 0;

				(*this)(0,1) = 0;
				(*this)(1,1) = 2/(b-t);
				(*this)(2,1) = 0;
				(*this)(3,1) = 0;

				(*this)(0,2) = 0;
				(*this)(1,2) = 0;
				(*this)(2,2) =1/(zf-zn);
				(*this)(3,2) = 0;

				(*this)(0,3) = (l+r)/(l-r);//(l+r+1)/(l-r);
				(*this)(1,3) = (t+b)/(t-b);//(t+b+1)/(b-t);
				(*this)(2,3) = zn/(zn-zf);
				(*this)(3,3) = 1;
			}



			//! Builds a left-handed look-at matrix.
			void buildCameraLookAtMatrixLH(const vector3d<T>& position, const vector3d<T>& target, const vector3d<T>& upVector)
			{
				vector3d<T> zaxis = target - position;
				zaxis.normalize();

				vector3d<T> xaxis = upVector.crossProduct(zaxis);
				xaxis.normalize();

				vector3d<T> yaxis = zaxis.crossProduct(xaxis);

				buildCameraLookAtMatrixLH(xaxis,yaxis,zaxis,position);
			}

			void buildCameraLookAtMatrixLH(const i_math::vector3d<T> &xaxis,
						const i_math::vector3d<T> &yaxis,const i_math::vector3d<T> &zaxis,
						const i_math::vector3d<T> &o)
			{
				(*this)(0,0) = xaxis.X;
				(*this)(1,0) = yaxis.X;
				(*this)(2,0) = zaxis.X;
				(*this)(3,0) = 0;

				(*this)(0,1) = xaxis.Y;
				(*this)(1,1) = yaxis.Y;
				(*this)(2,1) = zaxis.Y;
				(*this)(3,1) = 0;

				(*this)(0,2) = xaxis.Z;
				(*this)(1,2) = yaxis.Z;
				(*this)(2,2) = zaxis.Z;
				(*this)(3,2) = 0;

				(*this)(0,3) = -xaxis.dotProduct(o);
				(*this)(1,3) = -yaxis.dotProduct(o);
				(*this)(2,3) = -zaxis.dotProduct(o);
				(*this)(3,3) = 1.0f;
			}



			//! Builds a right-handed look-at matrix.
			void buildCameraLookAtMatrixRH(const vector3d<T>& position, const vector3d<T>& target, const vector3d<T>& upVector)
			{
				vector3d<T> zaxis = position - target;
				zaxis.normalize();

				vector3d<T> xaxis = upVector.crossProduct(zaxis);
				xaxis.normalize();

				vector3d<T> yaxis = zaxis.crossProduct(xaxis);

				(*this)(0,0) = xaxis.X;
				(*this)(1,0) = yaxis.X;
				(*this)(2,0) = zaxis.X;
				(*this)(3,0) = 0;

				(*this)(0,1) = xaxis.Y;
				(*this)(1,1) = yaxis.Y;
				(*this)(2,1) = zaxis.Y;
				(*this)(3,1) = 0;

				(*this)(0,2) = xaxis.Z;
				(*this)(1,2) = yaxis.Z;
				(*this)(2,2) = zaxis.Z;
				(*this)(3,2) = 0;

				(*this)(0,3) = -xaxis.dotProduct(position);
				(*this)(1,3) = -yaxis.dotProduct(position);
				(*this)(2,3) = -zaxis.dotProduct(position);
				(*this)(3,3) = 1.0f;
			}

			//! Builds a matrix that flattens geometry into a plane.
			//! \param light: light source
			//! \param plane: plane into which the geometry if flattened into
			//! \param point: value between 0 and 1, describing the light source. 
			//! If this is 1, it is a point light, if it is 0, it is a directional light.
			void buildShadowMatrix(vector3d<T> light, plane3d<T> plane, T point=1)
			{
				plane.Normal.normalize();
				T d = plane.Normal.dotProduct(light);

				(*this)(0,0) = plane.Normal.X * light.X + d;
				(*this)(1,0) = plane.Normal.X * light.Y;
				(*this)(2,0) = plane.Normal.X * light.Z;
				(*this)(3,0) = plane.Normal.X * point;

				(*this)(0,1) = plane.Normal.Y * light.X;
				(*this)(1,1) = plane.Normal.Y * light.Y + d;
				(*this)(2,1) = plane.Normal.Y * light.Z;
				(*this)(3,1) = plane.Normal.Y * point;

				(*this)(0,2) = plane.Normal.Z * light.X;
				(*this)(1,2) = plane.Normal.Z * light.Y;
				(*this)(2,2) = plane.Normal.Z * light.Z + d;
				(*this)(3,2) = plane.Normal.Z * point;

				(*this)(0,3) = plane.D * light.X + d;
				(*this)(1,3) = plane.D * light.Y;
				(*this)(2,3) = plane.D * light.Z;
				(*this)(3,3) = plane.D * point;
			}
			
			//! creates a new matrix as interpolated matrix from to other ones.
			//! \param b: other matrix to interpolate with
			//! \param time: Must be a value between 0 and 1.
			matrix44<T> interpolate(matrix44<T>& b, f32 time)
			{
				f32 ntime = 1.0f - time;
				matrix44<T> mat;
				for (s32 i=0; i<16; ++i)
					mat.M[i] = M[i]*ntime + b.M[i]*time;

				return mat;
			}

			//! returns transposed matrix
			void getTransposed(matrix44<T>&t)
			{
				for (s32 r=0; r<4; ++r)
				for (s32 c=0; c<4; ++c)
					t(r,c) = (*this)(c,r);
			}

			//! Matrix data, stored in column-major order
			union
			{
				struct 
				{
					T m00;T m01;T m02;T m03;
					T m10;T m11;T m12;T m13;
					T m20;T m21;T m22;T m23;
					T m30;T m31;T m32;T m33;
				};
				T M[16];
			};
	};

	typedef matrix44<f32> matrix44f;
	typedef matrix44<s32> matrix44i;



} // end namespace i_math

#define mat44from43(_mat44,_matSrc)\
{\
	(_mat44).set(\
		(_matSrc).m00, (_matSrc).m10, (_matSrc).m20,0,\
		(_matSrc).m01, (_matSrc).m11, (_matSrc).m21,0,\
		(_matSrc).m02, (_matSrc).m12, (_matSrc).m22,0,\
		(_matSrc).m03, (_matSrc).m13, (_matSrc).m23,1);\
}


