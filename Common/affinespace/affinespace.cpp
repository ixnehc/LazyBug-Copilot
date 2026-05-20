/********************************************************************
	created:	2007/5/30   16:48
	filename: 	e:\IxEngine\Common\affinespace\affinespace.cpp
	author:		cxi
	
	purpose:	affine space
*********************************************************************/
#include "stdh.h"

#include "affinespace.h"


float CalcDeterminant_3Order(i_math::vector3df &col1,i_math::vector3df &col2,i_math::vector3df &col3)
{
	return col1.x*col2.y*col3.z+col2.x*col3.y*col1.z+col3.x*col1.y*col2.z-
		col3.x*col2.y*col1.z-col3.y*col2.z*col1.x-col3.z*col2.x*col1.y;
}

float CalcDeterminant_2Order(i_math::vector2df &col1,i_math::vector2df &col2)
{
	return col1.x*col2.y-col2.x*col1.y;
}

//////////////////////////////////////////////////////////////////////////
//AffineSpace3

AffineSpace3::AffineSpace3()
{
	m_vOrg.x=0.0;
	m_vOrg.y=0.0;
	m_vOrg.z=0.0;

	m_xAxis=m_yAxis=m_zAxis=m_vOrg;
	m_xAxis.x=1.0;
	m_yAxis.y=1.0;
	m_zAxis.z=1.0;

	Construct(m_vOrg,m_xAxis,m_yAxis,m_zAxis);
}


//xAxis,yAxis,zAxis need not to be normalized vector.
//and their length will be taken as a unit-value.
//for example,if xAxis has a length of 2.0,and we calc a affine coordinate 
//with x=0.3,that means this point has a real x value of 0.6(2.0x0.3)
void AffineSpace3::Construct(i_math::vector3df &vOrg,i_math::vector3df &xAxis,i_math::vector3df &yAxis,i_math::vector3df &zAxis)
{
	m_vOrg=vOrg;
	m_xAxis=xAxis;
	m_yAxis=yAxis;
	m_zAxis=zAxis;

	m_determinant=CalcDeterminant_3Order(m_xAxis,m_yAxis,m_zAxis);
}

BOOL AffineSpace3::CalcCoord(i_math::vector3df &vAffineCoord,i_math::vector3df &vSrc)
{
	if (m_determinant==0.0)
		return FALSE;

	i_math::vector3df v;
	v.x=vSrc.x-m_vOrg.x;
	v.y=vSrc.y-m_vOrg.y;
	v.z=vSrc.z-m_vOrg.z;

	vAffineCoord.x=CalcDeterminant_3Order(v,m_yAxis,m_zAxis)/m_determinant;
	vAffineCoord.y=CalcDeterminant_3Order(m_xAxis,v,m_zAxis)/m_determinant;
	vAffineCoord.z=CalcDeterminant_3Order(m_xAxis,m_yAxis,v)/m_determinant;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//AffineSpace2

AffineSpace2::AffineSpace2()
{
	m_vOrg.x=0.0;
	m_vOrg.y=0.0;

	m_xAxis=m_yAxis=m_vOrg;
	m_xAxis.x=1.0;
	m_yAxis.y=1.0;

	Construct(m_vOrg,m_xAxis,m_yAxis);
}


//xAxis,yAxisneed not to be normalized vector.
//and their length will be taken as a unit-value.
//for example,if xAxis has a length of 2.0,and we calc a affine coordinate 
//with x=0.3,that means this point has a real x value of 0.6(2.0x0.3)
void AffineSpace2::Construct(i_math::vector2df &vOrg,
								i_math::vector2df &xAxis,i_math::vector2df &yAxis)
{
	m_vOrg=vOrg;
	m_xAxis=xAxis;
	m_yAxis=yAxis;

	m_determinant=CalcDeterminant_2Order(m_xAxis,m_yAxis);
}

BOOL AffineSpace2::CalcCoord(i_math::vector2df &coord,i_math::vector2df &vSrc)
{
	if (m_determinant==0.0)
		return FALSE;

	i_math::vector2df v;
	v.x=vSrc.x-m_vOrg.x;
	v.y=vSrc.y-m_vOrg.y;

	coord.x=CalcDeterminant_2Order(v,m_yAxis)/m_determinant;
	coord.y=CalcDeterminant_2Order(m_xAxis,v)/m_determinant;

	return TRUE;
}
