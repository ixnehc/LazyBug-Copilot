#ifndef __TimeStep_h__
#define __TimeStep_h__

#include "../Common/Common.h"
#include "SimulationModel.h"
#include "CollisionDetection.h"

namespace PBD
{
	/** \brief Base class for the simulation methods. 
	*/
	class TimeStep 
	{
	protected:
// 		CollisionDetection *m_collisionDetection;

		/** Clear accelerations and add gravitation.
		*/
		void clearAccelerations(SimulationModel &model);

		virtual void initParameters();

// 		static void contactCallbackFunction(const unsigned int contactType,
// 			const unsigned int bodyIndex1, const unsigned int bodyIndex2,
// 			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
// 			const i_math::vector3df &normal, const Real dist,
// 			const Real restitutionCoeff, const Real frictionCoeff, void *userData);
// 
// 		static void solidContactCallbackFunction(const unsigned int contactType,
// 			const unsigned int bodyIndex1, const unsigned int bodyIndex2,
// 			const unsigned int tetIndex, const i_math::vector3df &bary,
// 			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
// 			const i_math::vector3df &normal, const Real dist,
// 			const Real restitutionCoeff, const Real frictionCoeff, void *userData);

	public:
		TimeStep();
		virtual ~TimeStep(void);

		virtual void step(SimulationModel &model) = 0;
		virtual void reset();

		virtual void init();

// 		void setCollisionDetection(SimulationModel &model, CollisionDetection *cd);
// 		CollisionDetection *getCollisionDetection();
	};
}

#endif
