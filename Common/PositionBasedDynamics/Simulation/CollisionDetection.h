#ifndef _COLLISIONDETECTION_H
#define _COLLISIONDETECTION_H

#include "../Common/Common.h"
#include "SimulationModel.h"
// #include "AABB.h"

namespace PBD
{
	class CollisionDetection
	{
	public:
		static const unsigned int RigidBodyContactType = 0;
		static const unsigned int ParticleContactType = 1;
		static const unsigned int ParticleRigidBodyContactType = 2;		
		static const unsigned int ParticleSolidContactType = 3;

		typedef void(*ContactCallbackFunction)(const unsigned int contactType, const unsigned int bodyIndex1, const unsigned int bodyIndex2,
			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
			const i_math::vector3df &normal, const Real dist, 
			const Real restitutionCoeff, const Real frictionCoeff, void *userData);

		typedef void(*SolidContactCallbackFunction)(const unsigned int contactType, const unsigned int bodyIndex1, const unsigned int bodyIndex2,
			const unsigned int tetIndex, const i_math::vector3df &bary,
			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
			const i_math::vector3df &normal, const Real dist,
			const Real restitutionCoeff, const Real frictionCoeff, void *userData);

		struct CollisionObject
		{
			static const unsigned int RigidBodyCollisionObjectType = 0;
			static const unsigned int TriangleModelCollisionObjectType = 1;
			static const unsigned int TetModelCollisionObjectType = 2;

			i_math::aabbox3df m_aabb;
			unsigned int m_bodyIndex;
			unsigned int m_bodyType;

			virtual ~CollisionObject() {}
			virtual int &getTypeId() const = 0;
		};

		struct CollisionObjectWithoutGeometry : public CollisionObject
		{
			static int TYPE_ID;
			virtual int &getTypeId() const { return TYPE_ID; }
			virtual ~CollisionObjectWithoutGeometry() {}
		};

	protected:
		Real m_tolerance;
		ContactCallbackFunction m_contactCB;
		SolidContactCallbackFunction m_solidContactCB;
		void *m_contactCBUserData;
		void *m_solidContactCBUserData;
		std::vector<CollisionObject*> m_collisionObjects;

		void updateAABB(const i_math::vector3df &p, i_math::aabbox3df &aabb);

	public:
		CollisionDetection();
		virtual ~CollisionDetection();

		void cleanup();

		Real getTolerance() const { return m_tolerance; }
		void setTolerance(Real val) { m_tolerance = val; }

		void addRigidBodyContact(const unsigned int rbIndex1, const unsigned int rbIndex2,
			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
			const i_math::vector3df &normal, const Real dist,
			const Real restitutionCoeff, const Real frictionCoeff);

		void addParticleRigidBodyContact(const unsigned int particleIndex, const unsigned int rbIndex,
			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
			const i_math::vector3df &normal, const Real dist, 
			const Real restitutionCoeff, const Real frictionCoeff);

		void addParticleSolidContact(const unsigned int particleIndex, const unsigned int solidIndex,
			const unsigned int tetIndex, const i_math::vector3df &bary,
			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
			const i_math::vector3df &normal, const Real dist,
			const Real restitutionCoeff, const Real frictionCoeff);

		virtual void addCollisionObject(const unsigned int bodyIndex, const unsigned int bodyType);

		std::vector<CollisionObject*> &getCollisionObjects() { return m_collisionObjects; }

		virtual void collisionDetection(SimulationModel &model) = 0;

		void setContactCallback(CollisionDetection::ContactCallbackFunction val, void *userData);
		void setSolidContactCallback(CollisionDetection::SolidContactCallbackFunction val, void *userData);
		void updateAABBs(SimulationModel &model);
		void updateAABB(SimulationModel &model, CollisionDetection::CollisionObject *co);
	};
}

#endif
