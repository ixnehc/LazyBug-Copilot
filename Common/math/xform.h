#pragma once

#include "iMath.h"
#include "vector3d.h"
#include "quaternion.h"
#include "matrix43.h"

namespace i_math
{

template <class T>
class xform
{
	public:

		// Constructors

		xform()
		{
			scale_=1;
		};

		// functions
		void set(vector3d<T> &t,quat<T>&q,T s)
		{
			pos=t;
			rot=q;
			scale_=s;
		}

		void setPos(T x,T y,T z)		{			pos.set(x,y,z);		}
		void setPos(vector3d<T> p)		{			pos=p;		}
		void setRot(quat<T> q)		{			rot=q;		}
		void setScale(T s)		{			scale_=s;		}
		vector3d<T> getPos()		{			return pos;		}
		quat<T> getRot()		{			return rot;		}
		T getScale()		{			return scale_;		}
		void getMatrix(matrix43<T>&mat)
		{
			if (i_math::equals(scale_,1))
				mat.setRotationQuaternion(rot);
			else
			{
				mat.setScale(scale_,scale_,scale_);
				mat.addRotationQuaternion(rot);
			}
			mat.addTranslation(pos);
		}
		matrix43<T> getMatrix()
		{
			matrix43<T>mat;
			getMatrix(mat);
			return mat;
		}


		//mat必须是按照scale_*rotation*translation乘出来的
		void fromMatrix(matrix43<T>&mat)
		{
			scale_=mat.getScaleX();
			pos=mat.getTranslation();

			matrix43<T> matT=mat;
			*((vector3d<T>*)&matT.m00)/=scale_;
			*((vector3d<T>*)&matT.m01)/=scale_;
			*((vector3d<T>*)&matT.m02)/=scale_;

			rot=matT.getRotationQuaternion();
			rot.normalize();
		}

		//目前对于scale不是1的情况尚未测试
		xform<T> operator*(const xform<T>& other) const
		{
			xform<T> ret;
			ret.pos=other.rot*pos*other.scale_+other.pos;
			ret.rot=rot*other.rot;
			ret.rot.normalize();
			ret.scale_=scale_*other.scale_;
			return ret;
		}

		void applyBase(const xform<T>& base)
		{
			*this=(*this)*base;
		}

		void makeInverse()
		{
			rot.makeInverse();
			pos=rot*(-pos);
			pos/=scale_;
			scale_=1.0f/scale_;
		}

		void makeLocal(const i_math::xform<T> &xfmBase)
		{
			i_math::xformf xfmBaseInv=xfmBase;
			xfmBaseInv.makeInverse();
			*this=(*this)*xfmBaseInv;
		}

		void rotateVect(const vector3d<T>& in,vector3d<T>& out) const
		{
			out=rot*in;
		}

		void transformVect(const vector3d<T>& in,vector3d<T> &out) const
		{
			out=(rot*in)*scale_+pos;
		}

		void transformVectInv(const vector3d<T>& in,vector3d<T> &out) const
		{
			i_math::xformf inv=*this;
			inv.makeInverse();
			inv.transformVect(in,out);
		}


		void fromZAxis(i_math::vector3df &pos,i_math::vector3df &axisZ0)
		{
			i_math::vector3df axisZ=axisZ0;
			if (axisZ.getLengthSQ()<0.0001f)
				axisZ.set(0,1,0);
			i_math::vector3df axisY,axisX;

			axisZ.findBestAxis(axisX,axisY);

			i_math::matrix43f mat;
			mat.buildMatrixLH(axisX,axisY,axisZ,pos);

			fromMatrix(mat);
		}


		//! Calculates a new interpolated xform
		//! \param other: other xform to interpolate between
		//! \param d: value between 0.0f and 1.0f. 
		//! 1: full me,0: full other
		xform<T> getInterpolated(const xform<T>& other, f32 d) const
		{
			xform<T>ret;
			f32 inv = 1.0f - d;
			ret.pos=(other.pos*inv) + (pos*d);
			ret.rot.slerp(other.rot,rot,d);
			ret.scale_=(other.scale_*inv) + (scale_*d);
			return ret;
		}

		void flattenY()
		{
			pos.y=0.0f;
			i_math::vector3df euler;
			rot.toEuler(euler);
			euler.y=euler.z=0.0f;
			rot.fromEuler(euler);
		}

		// member variables
		
		vector3d<T> pos;
		T scale_;
		quat<T> rot;
	};

	//! Typedef for a f32 3d bounding box.
	typedef xform<f32> xformf;
	//! Typedef for an integer 3d bounding box.
	typedef xform<s32> xformi;

} 

