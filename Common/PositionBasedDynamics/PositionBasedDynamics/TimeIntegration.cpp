#include "stdh.h"
#include "TimeIntegration.h"

using namespace PBD;


// ----------------------------------------------------------------------------------------------
void TimeIntegration::semiImplicitEuler(
	const Real h, 
	const Real mass, 
	i_math::vector3df &position,
	i_math::vector3df &velocity,
	const i_math::vector3df &acceleration)
{				
	if (mass != 0.0)
	{
		velocity += acceleration * h;
		position += velocity * h;
	}
}

// ----------------------------------------------------------------------------------------------
// void TimeIntegration::semiImplicitEulerRotation(
// 	const Real h,
// 	const Real mass,
// 	const Matrix3r &invInertiaW,
// 	i_math::quatf &rotation,
// 	i_math::vector3df &angularVelocity,	
// 	const i_math::vector3df &torque)
// {
// 	if (mass != 0.0)
// 	{
// 		// simple form without nutation effect
// 		angularVelocity += h * invInertiaW * torque;
// 
// 		i_math::quatf angVelQ(0.0, angularVelocity[0], angularVelocity[1], angularVelocity[2]);
// 		rotation.coeffs() += h * 0.5 * (angVelQ * rotation).coeffs();
// 		rotation.normalize();
// 	}
// }

// ----------------------------------------------------------------------------------------------
void TimeIntegration::velocityUpdateFirstOrder(
	const Real h,
	const Real mass,
	const i_math::vector3df &position,
	const i_math::vector3df &oldPosition,
	i_math::vector3df &velocity)
{
	if (mass != 0.0f)
		velocity = (1.0f / h) * (position - oldPosition);
}

// ----------------------------------------------------------------------------------------------
void TimeIntegration::angularVelocityUpdateFirstOrder(
	const Real h,
	const Real mass,
	const i_math::quatf &rotation,
	const i_math::quatf &oldRotation,
	i_math::vector3df &angularVelocity)
{
	if (mass != 0.0f)
	{
		const i_math::quatf relRot = (rotation * oldRotation.getInvert());
		angularVelocity = relRot.getVec() *(2.0f / h);
	}
}

// ----------------------------------------------------------------------------------------------
void TimeIntegration::velocityUpdateSecondOrder(
	const Real h,
	const Real mass,
	const i_math::vector3df &position,
	const i_math::vector3df &oldPosition,
	const i_math::vector3df &positionOfLastStep,
	i_math::vector3df &velocity)
{
	if (mass != 0.0f)
		velocity = (1.0f / h) * (1.5f*position - 2.0f*oldPosition + 0.5f*positionOfLastStep);
}

// ----------------------------------------------------------------------------------------------
void TimeIntegration::angularVelocityUpdateSecondOrder(
	const Real h,
	const Real mass,
	const i_math::quatf &rotation,				
	const i_math::quatf &oldRotation,			
	const i_math::quatf &rotationOfLastStep,	
	i_math::vector3df &angularVelocity)
{
	// ToDo: is still first order
	if (mass != 0.0f)
	{
		const i_math::quatf relRot = (rotation * oldRotation.getInvert());
		angularVelocity = relRot.getVec() *(2.0f / h);
	}
}