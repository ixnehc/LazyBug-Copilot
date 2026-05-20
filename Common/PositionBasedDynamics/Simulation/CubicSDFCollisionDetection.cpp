#include "CubicSDFCollisionDetection.h"
#include "../Simulation/IDFactory.h"

using namespace PBD;

// int CubicSDFCollisionDetection::CubicSDFCollisionObject::TYPE_ID = IDFactory::getId();
// 
// CubicSDFCollisionDetection::CubicSDFCollisionDetection() :
// 	DistanceFieldCollisionDetection()
// {	
// }
// 
// CubicSDFCollisionDetection::~CubicSDFCollisionDetection()
// {
// }
// 
// bool CubicSDFCollisionDetection::isDistanceFieldCollisionObject(CollisionObject *co) const
// {
//  	return DistanceFieldCollisionDetection::isDistanceFieldCollisionObject(co) ||
// 		(co->getTypeId() == CubicSDFCollisionDetection::CubicSDFCollisionObject::TYPE_ID);
// }
// 
// void CubicSDFCollisionDetection::addCubicSDFCollisionObject(const unsigned int bodyIndex, const unsigned int bodyType, const i_math::vector3df *vertices, const unsigned int numVertices, const std::string &sdfFile, const i_math::vector3df &scale, const bool testMesh, const bool invertSDF)
// {
// 	CubicSDFCollisionDetection::CubicSDFCollisionObject *co = new CubicSDFCollisionDetection::CubicSDFCollisionObject();
// 	co->m_bodyIndex = bodyIndex;
// 	co->m_bodyType = bodyType;
// 	co->m_sdfFile = sdfFile;
// 	co->m_scale = scale;
// 	co->m_sdf = std::make_shared<Grid>(co->m_sdfFile);
//  	co->m_bvh.init(vertices, numVertices);
//  	co->m_bvh.construct();
// 	co->m_testMesh = testMesh;
// 	co->m_invertSDF = 1.0;
// 	if (invertSDF)
// 		co->m_invertSDF = -1.0;
// 	m_collisionObjects.push_back(co);
// }
// 
// void PBD::CubicSDFCollisionDetection::addCubicSDFCollisionObject(const unsigned int bodyIndex, const unsigned int bodyType, const i_math::vector3df *vertices, const unsigned int numVertices, GridPtr sdf, const i_math::vector3df &scale, const bool testMesh /*= true*/, const bool invertSDF /*= false*/)
// {
// 	CubicSDFCollisionDetection::CubicSDFCollisionObject *co = new CubicSDFCollisionDetection::CubicSDFCollisionObject();
// 	co->m_bodyIndex = bodyIndex;
// 	co->m_bodyType = bodyType;
// 	co->m_sdfFile = "";
// 	co->m_scale = scale;
// 	co->m_sdf = sdf;
// 	co->m_bvh.init(vertices, numVertices);
// 	co->m_bvh.construct();
// 	co->m_testMesh = testMesh;
// 	co->m_invertSDF = 1.0;
// 	if (invertSDF)
// 		co->m_invertSDF = -1.0;
// 	m_collisionObjects.push_back(co);
// }
// 
// CubicSDFCollisionDetection::CubicSDFCollisionObject::CubicSDFCollisionObject()
// {
// }
// 
// CubicSDFCollisionDetection::CubicSDFCollisionObject::~CubicSDFCollisionObject()
// {
// }
// 
// double CubicSDFCollisionDetection::CubicSDFCollisionObject::distance(const Eigen::Vector3d &x, const Real tolerance)
// {
// 	const Eigen::Vector3d scaled_x = x.cwiseProduct(m_scale.template cast<double>().cwiseInverse());
// 	const double dist = m_sdf->interpolate(0, scaled_x);
// 	if (dist == std::numeric_limits<double>::max())
// 		return dist;
// 	return m_invertSDF * m_scale[0]*dist - tolerance;
// }
// 
// bool CubicSDFCollisionDetection::CubicSDFCollisionObject::collisionTest(const i_math::vector3df &x, const Real tolerance, i_math::vector3df &cp, i_math::vector3df &n, Real &dist, const Real maxDist)
// {
// 	const i_math::vector3df scaled_x = x.cwiseProduct(m_scale.cwiseInverse());
// 
// 	Eigen::Vector3d normal;	
// 	double d = m_sdf->interpolate(0, scaled_x.template cast<double>(), &normal);
// 	if (d == std::numeric_limits<Real>::max())
// 		return false;
// 	dist = static_cast<Real>(m_invertSDF * d - tolerance);
// 
// 	normal = m_invertSDF * normal;
// 	if (dist < maxDist)
// 	{
// 		normal.normalize();
// 		n = normal.template cast<Real>();
// 
// 		cp = (scaled_x - dist * n);
// 		cp = cp.cwiseProduct(m_scale);
// 
// 		return true;
// 	}
// 	return false;
// }

