#ifndef _CubicSDFCollisionDetection_H
#define _CubicSDFCollisionDetection_H

#include "../Common/Common.h"
#include "../Simulation/DistanceFieldCollisionDetection.h"
#include <memory>

// #include "Discregrid/All"

namespace PBD
{
	/** Collision detection based on cubic signed distance fields. 
	*/
// 	class CubicSDFCollisionDetection : public DistanceFieldCollisionDetection
// 	{
// 	public:
// 		using Grid = Discregrid::CubicLagrangeDiscreteGrid;
// 		using GridPtr = std::shared_ptr<Discregrid::CubicLagrangeDiscreteGrid>;
// 
// 		struct CubicSDFCollisionObject : public DistanceFieldCollisionDetection::DistanceFieldCollisionObject
// 		{
// 			std::string m_sdfFile;
// 			i_math::vector3df m_scale;
// 			GridPtr m_sdf;
// 			static int TYPE_ID;
// 
// 			CubicSDFCollisionObject();
// 			virtual ~CubicSDFCollisionObject();
// 			virtual int &getTypeId() const { return TYPE_ID; }
// 			virtual bool collisionTest(const i_math::vector3df &x, const Real tolerance, i_math::vector3df &cp, i_math::vector3df &n, Real &dist, const Real maxDist = 0.0);
// 			virtual double distance(const Eigen::Vector3d &x, const Real tolerance);
// 		};
// 
// 	public:
// 		CubicSDFCollisionDetection();
// 		virtual ~CubicSDFCollisionDetection();
// 
// 		virtual bool isDistanceFieldCollisionObject(CollisionObject *co) const;
// 
// 		void addCubicSDFCollisionObject(const unsigned int bodyIndex, const unsigned int bodyType, const i_math::vector3df *vertices, const unsigned int numVertices, const std::string &sdfFile, const i_math::vector3df &scale, const bool testMesh = true, const bool invertSDF = false);
// 		void addCubicSDFCollisionObject(const unsigned int bodyIndex, const unsigned int bodyType, const i_math::vector3df *vertices, const unsigned int numVertices, GridPtr sdf, const i_math::vector3df &scale, const bool testMesh = true, const bool invertSDF = false);
// 	};
}

#endif
