
#pragma  once

#include "math/imath_all.h"
#include "fastdelegate/FastDelegate.h"

#include "RenderSystem/IRenderSystemDefines.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITools.h"

#include "resdata/ResDataDefines.h"

class HitProbe;

enum Matrix_EditMode
{
	EditMode_Move  = 0x1,
	EditMode_Scale = 0x2,
	EditMode_Select = 0x4,
	EditMode_Rot = 0x8,
	EditMode_All = 0xffffffff
};
enum Matrix_EditSpace
{
	EditSpace_View,
	EditSpace_World,
	EditSpace_Parent,
	EditSpace_Local,
	EditSpace_Screen,
	EditSpace_Unknown,
};
enum
{
	Active_X = 0x1,
	Active_Y = 0x2,
	Active_Z = 0x4,
	Active_XY = 0x8,
	Active_YZ = 0x10,
	Active_ZX = 0x20,
	Active_OZ = 0x40,
	Active_XYZ = 0x80,
};

struct MatrixEditData
{
	MatrixEditData()
	{
		matrix = NULL;
		modespace = EditSpace_Unknown;
		modeedit = EditMode_Select;
	}
	i_math::matrix43f * matrix;
	i_math::matrix43f matParent;
	Matrix_EditSpace modespace;
	Matrix_EditMode  modeedit; 
};

#define PreEdit(mode)\
	if(_data.modeedit!=mode||_funPerEditListener.empty()||!_funPerEditListener(this)||!IsBind()||!_bWorkable)  \
	return TRUE;	

class CMatrixEditBase
{
	typedef fastdelegate::FastDelegate1<CMatrixEditBase *,BOOL>   PreEditListener;
	typedef fastdelegate::FastDelegate1<CMatrixEditBase *,BOOL>   OnEditListener;
	typedef fastdelegate::FastDelegate1<CMatrixEditBase *,BOOL>   EndEditListener;
public:
	virtual const char * getClassName()=0;
	CMatrixEditBase()
	{
		_bBind = FALSE;
		_bSel = FALSE;
		_bWorkable = TRUE;
		_shaderState.modeDepth = Depth_NoCmp;
	}
	virtual BOOL IsSelected()
	{
		return _bSel;
	}
	virtual void SetPreEditListener(PreEditListener funPerEditListener)
	{
		_funPerEditListener = funPerEditListener;
	}
	virtual void SetOnEditListener(OnEditListener funOnEditListener)
	{
		_funOnEditListener = funOnEditListener;
	}
	virtual void SetEndEditListener(EndEditListener funEndEditListener)
	{
		_funEndEditListener = funEndEditListener;
	}
	virtual BOOL IsBind() 
	{
		return _bBind;
	}
	virtual void SetWorkable(BOOL bWorkable){ _bWorkable = bWorkable;}

	virtual BOOL IsWorkable() { return _bWorkable;}
	virtual BOOL Bind(MatrixEditData &data)
	{	
		if(!data.matrix)
		{	
			_data.matrix = NULL;
			_bBind = FALSE;
			return FALSE;
		}

		if(_data.matrix!=data.matrix)
			_matValue = *data.matrix;
		
		if(data.modespace==EditSpace_Unknown)
			data.modespace = _data.modespace;

		if(data.modespace==EditSpace_Unknown)
			data.modespace = _data.modespace;

		_data = data;
		_bBind = TRUE;
		return TRUE;
	}

	struct SpaceData
	{
		SpaceData()
		{
			rp = NULL;
			matParent = NULL;
			matOffset = NULL;
			mode = EditSpace_World;
		}
		IRenderPort * rp;
		i_math::matrix43f *matParent;
		i_math::matrix43f *matOffset;
		DWORD mode;
	};

	void GetSpace(SpaceData& data,i_math::vector3df &axisX,i_math::vector3df &axisY,i_math::vector3df &axisZ,i_math::matrix43f &mat,i_math::matrix43f *matfromWld=NULL)
	{
		i_math ::matrix43f matP,matOf,matfrmWld;
		if(data.matParent) 
			matP = *(data.matParent);
		if (data.matOffset) 
			matOf = *(data.matOffset);
		
		i_math::vector3df v0;
		i_math::matrix43f matTrans;
		
		switch(data.mode)
		{
		case EditSpace_View:
			{
				i_math::matrix44f matViewO;
				i_math::matrix43f matView;
				ICamera * camer = data.rp->GetCamera();
				camer->GetView(matViewO);
				mat43from44(matView,matViewO);
				matView.getInverse(&matTrans);
				mat = matOf*matP*matView;
				matfrmWld = matView;
				break;
			}
		case EditSpace_World:
			{
				matTrans.makeIdentity();
				mat = matOf*matP;
				matfrmWld.makeIdentity();
				break;
			}
		case EditSpace_Parent:
			{
				matTrans = matP;
				mat = matOf;
				matfrmWld = matP;
				break;
			}
		case EditSpace_Local:
			{
				matTrans = matOf*matP;
				mat.makeIdentity();
				matTrans.getInverse(&matfrmWld);
				break;
			}
		case EditSpace_Screen:
			{
				ICamera * camer = data.rp->GetCamera();
			
				v0.set(0.0f,0.0f,0.0f);
				axisX.set(1.0f,0.0f,0.0f);
				axisY.set(0.0f,1.0f,0.0f);
			
				camer->TransPosInverse(v0);
				camer->TransPosInverse(axisX);
				camer->TransPosInverse(axisY);

				axisX -= v0;
				axisY -= v0;
				axisZ = axisX.crossProduct(axisY);
				
				break;
			}
		}
		
		if(data.mode!=EditSpace_Screen)
		{	
			axisX.set(1.0f,0.0f,0.0f);
			axisY.set(0.0f,1.0f,0.0f);
			axisZ.set(0.0f,0.0f,1.0f);

			matTrans.transformVect(v0,v0);
			matTrans.transformVect(axisX,axisX);
			matTrans.transformVect(axisY,axisY);
			matTrans.transformVect(axisZ,axisZ);

			axisX = axisX - v0;
			axisY = axisY - v0;
			axisZ = axisZ - v0;
		}

		axisX.normalize();
		axisY.normalize();
		axisZ.normalize();

		if(matfromWld)
			*matfromWld = matfrmWld;
	}
protected:
	float _getDistDir(HitProbe &testVec,i_math::vector3df &start,i_math::vector3df &axis,float len,i_math::plane3df &pane)
	{
		i_math::vector3df intersec,inl,closePoint,dir;
		float dist;
		dir = testVec.start - testVec.end;
		bool ret = pane.getIntersectionWithLine(testVec.start,dir,intersec);
		if(!ret)
			return -1;

		inl = axis;
		inl.setLength(len);
		inl = start + inl;

		i_math::line3df line;
		line.setLine(start,inl);
		closePoint = line.getClosestPoint(intersec);

		dist = (float)closePoint.getDistanceFrom(intersec);

		return dist;
	}

	BOOL _HitTestQuad(i_math::line3df &line,i_math::vector3df * quad,float &z)
	{
		i_math::plane3df plane;
		i_math::vector3df inter;

		i_math::triangle3df tri0,tri1;
		tri0.set(quad[0],quad[1],quad[2]);
		tri1.set(quad[0],quad[2],quad[3]);
		
		BOOL bIntersec = FALSE;
		bool ret = false;
		ret = tri0.getSafeIntersectionWithLine(line.start,line.start - line.end,inter);
		if(ret)
		{
			bIntersec = TRUE;
			z = (float)inter.getDistanceFrom(line.start);
		}
		ret = tri1.getSafeIntersectionWithLine(line.start,line.start - line.end,inter);
		if(ret)
		{
			bIntersec = TRUE;
			z = min((float)inter.getDistanceFrom(line.start),z);
		}

		return bIntersec;
	}
	//listener 
	PreEditListener  _funPerEditListener;
	OnEditListener  _funOnEditListener;
	EndEditListener  _funEndEditListener;
	ShaderState _shaderState;
	BOOL _bBind;
	BOOL _bSel;
	BOOL _bWorkable;//表示这个agent是否可以工作
	MatrixEditData _data;
	i_math::matrix43f _matValue;
};

#define M_PI_F		3.14159265358979323846f










