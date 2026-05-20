#pragma once

#include "../math/vector2d.h"
#include "../math/vector3d.h"

struct AffineSpace3
{
	AffineSpace3();

	void Construct(i_math::vector3df&vOrg,
				i_math::vector3df&xAxis,i_math::vector3df&yAxis,i_math::vector3df&zAxis);

	BOOL CalcCoord(i_math::vector3df&coord,i_math::vector3df&vSrc);

	i_math::vector3df m_vOrg;
	i_math::vector3df m_xAxis,m_yAxis,m_zAxis;
	float m_determinant;
};

struct AffineSpace2
{
	AffineSpace2();

	void Construct(i_math::vector2df&vOrg,
					i_math::vector2df&xAxis,i_math::vector2df&yAxis);

	BOOL CalcCoord(i_math::vector2df&coord,i_math::vector2df&vSrc);

	i_math::vector2df m_vOrg;
	i_math::vector2df m_xAxis,m_yAxis;
	float m_determinant;
};