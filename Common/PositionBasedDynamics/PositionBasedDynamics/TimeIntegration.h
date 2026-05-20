#ifndef TIMEINTEGRATION_H
#define TIMEINTEGRATION_H

#include "../Common/Common.h"

// ------------------------------------------------------------------------------------
namespace PBD
{
	class TimeIntegration
	{
	public:	
		/** Perform an integration step for a particle using the semi-implicit Euler 
		 * (symplectic Euler) method:
		 * \f{align*}{
		 * \mathbf{v}(t+h) &= \mathbf{v}(t) + \mathbf{a}(t) h\\
		 * \mathbf{x}(t+h) &= \mathbf{x}(t) + \mathbf{v}(t+h) h
		 * \f}
		 *
		 * @param  h time step size
		 * @param  mass mass of the particle
		 * @param  position position of the particle
		 * @param  velocity velocity of the particle
		 * @param  acceleration acceleration of the particle
		 */		
		static void semiImplicitEuler(
			const Real h,
			const Real mass,
			i_math::vector3df &position,
			i_math::vector3df &velocity,
			const i_math::vector3df &acceleration);

		// -------------- semi-implicit Euler (symplectic Euler) for rotational part of a rigid body -----------------
// 		static void semiImplicitEulerRotation(
// 			const Real h,
// 			const Real mass,
// 			const Matrix3r &invInertiaW,
// 			i_math::quatf &rotation,
// 			i_math::vector3df &angularVelocity,
// 			const i_math::vector3df &torque);


		// -------------- velocity update (first order) -----------------------------------------------------
		/** Perform a velocity update (first order) for the linear velocity:
		 * \f{equation*}{
		 * \mathbf{v}(t+h) = \frac{1}{h} (\mathbf{p}(t+h) - \mathbf{p}(t)
		 * \f}
		 *
		 * @param  h time step size
		 * @param  mass mass of the particle
		 * @param  position new position \f$\mathbf{p}(t+h)\f$ of the particle
		 * @param  oldPosition position \f$\mathbf{p}(t)\f$ of the particle before the time step
		 * @param  velocity resulting velocity of the particle
		 */		
		static void velocityUpdateFirstOrder(
			const Real h,
			const Real mass,
			const i_math::vector3df &position,				// position after constraint projection	at time t+h
			const i_math::vector3df &oldPosition,				// position before constraint projection at time t
			i_math::vector3df &velocity);

		// -------------- angular velocity update (first order)  ------------------------------------------------
		static void angularVelocityUpdateFirstOrder(
			const Real h,
			const Real mass,
			const i_math::quatf &rotation,				// rotation after constraint projection	at time t+h
			const i_math::quatf &oldRotation,			// rotation before constraint projection at time t
			i_math::vector3df &angularVelocity);


		// -------------- velocity update (second order) -----------------------------------------------------
		static void velocityUpdateSecondOrder(
			const Real h,
			const Real mass,
			const i_math::vector3df &position,				// position after constraint projection	at time t+h
			const i_math::vector3df &oldPosition,				// position before constraint projection at time t
			const i_math::vector3df &positionOfLastStep,		// position of last simulation step at time t-h
			i_math::vector3df &velocity);

		// -------------- angular velocity update (second order)  ------------------------------------------------
		static void angularVelocityUpdateSecondOrder(
			const Real h,
			const Real mass,
			const i_math::quatf &rotation,				// rotation after constraint projection	at time t+h
			const i_math::quatf &oldRotation,			// rotation before constraint projection at time t
			const i_math::quatf &rotationOfLastStep,	// rotation of last simulation step at time t-h
			i_math::vector3df &angularVelocity);

	};
}

#endif