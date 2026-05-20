#include "stdh.h"
#include "Constraints.h"
#include "SimulationModel.h"
#include "../PositionBasedDynamics/PositionBasedDynamics.h"
// #include "../PositionBasedDynamics/PositionBasedRigidBodyDynamics.h"
#include "TimeManager.h"
#include "../Simulation/IDFactory.h"
// #include "../PositionBasedDynamics/PositionBasedElasticRods.h"


#include <set>
#include <map>

using namespace PBD;


// int BallJoint::TYPE_ID = IDFactory::getId();
// int BallOnLineJoint::TYPE_ID = IDFactory::getId();
// int HingeJoint::TYPE_ID = IDFactory::getId();
// int UniversalJoint::TYPE_ID = IDFactory::getId();
// int RigidBodyParticleBallJoint::TYPE_ID = IDFactory::getId();
// int RigidBodySpring::TYPE_ID = IDFactory::getId();
// int DistanceJoint::TYPE_ID = IDFactory::getId();
int DistanceConstraint::TYPE_ID = IDFactory::getId();
int PullConstraint::TYPE_ID = IDFactory::getId();
int SimpleContactConstraint::TYPE_ID = IDFactory::getId();
int DihedralConstraint::TYPE_ID = IDFactory::getId();
int BendConstraint::TYPE_ID = IDFactory::getId();
int IsometricBendingConstraint::TYPE_ID = IDFactory::getId();
// int FEMTriangleConstraint::TYPE_ID = IDFactory::getId();
//int StrainTriangleConstraint::TYPE_ID = IDFactory::getId();
int VolumeConstraint::TYPE_ID = IDFactory::getId();
//int FEMTetConstraint::TYPE_ID = IDFactory::getId();
//int StrainTetConstraint::TYPE_ID = IDFactory::getId();
//int ShapeMatchingConstraint::TYPE_ID = IDFactory::getId();
//int TargetAngleMotorHingeJoint::TYPE_ID = IDFactory::getId();
//int TargetVelocityMotorHingeJoint::TYPE_ID = IDFactory::getId();
//int SliderJoint::TYPE_ID = IDFactory::getId();
// int TargetPositionMotorSliderJoint::TYPE_ID = IDFactory::getId();
// int TargetVelocityMotorSliderJoint::TYPE_ID = IDFactory::getId();
// int DamperJoint::TYPE_ID = IDFactory::getId();
// int RigidBodyContactConstraint::TYPE_ID = IDFactory::getId();
// int ParticleRigidBodyContactConstraint::TYPE_ID = IDFactory::getId();
// int ParticleTetContactConstraint::TYPE_ID = IDFactory::getId();
// int StretchShearConstraint::TYPE_ID = IDFactory::getId();
// int BendTwistConstraint::TYPE_ID = IDFactory::getId();
// int StretchBendingTwistingConstraint::TYPE_ID = IDFactory::getId();
// int DirectPositionBasedSolverForStiffRodsConstraint::TYPE_ID = IDFactory::getId();

//////////////////////////////////////////////////////////////////////////
// BallJoint
//////////////////////////////////////////////////////////////////////////
// bool BallJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_BallJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos,
// 		m_jointInfo);
// }
// 
// bool BallJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_BallJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool BallJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_BallJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// BallOnLineJoint
//////////////////////////////////////////////////////////////////////////
// bool BallOnLineJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &dir)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_BallOnLineJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos, dir,
// 		m_jointInfo);
// }
// 
// bool BallOnLineJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_BallOnLineJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool BallOnLineJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_BallOnLineJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// HingeJoint
//////////////////////////////////////////////////////////////////////////
// bool HingeJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_HingeJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos, axis,
// 		m_jointInfo);
// }
// 
// bool HingeJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_HingeJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool HingeJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_HingeJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// UniversalJoint
//////////////////////////////////////////////////////////////////////////
// bool UniversalJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis1, const i_math::vector3df &axis2)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_UniversalJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos,
// 		axis1,
// 		axis2,
// 		m_jointInfo);
// }
// 
// bool UniversalJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_UniversalJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool UniversalJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_UniversalJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// SliderJoint
//////////////////////////////////////////////////////////////////////////
// bool SliderJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_SliderJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		axis,
// 		m_jointInfo);
// }
// 
// bool SliderJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_SliderJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool SliderJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_SliderJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// TargetPositionMotorSliderJoint
//////////////////////////////////////////////////////////////////////////
// bool TargetPositionMotorSliderJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_TargetPositionMotorSliderJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		axis,
// 		m_jointInfo);
// }
// 
// bool TargetPositionMotorSliderJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_TargetPositionMotorSliderJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool TargetPositionMotorSliderJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_TargetPositionMotorSliderJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_target,
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }



//////////////////////////////////////////////////////////////////////////
// TargetVelocityMotorSliderJoint
//////////////////////////////////////////////////////////////////////////
// bool TargetVelocityMotorSliderJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_TargetVelocityMotorSliderJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		axis,
// 		m_jointInfo);
// }
// 
// bool TargetVelocityMotorSliderJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_TargetVelocityMotorSliderJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool TargetVelocityMotorSliderJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_TargetVelocityMotorSliderJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }
// 
// 
// bool TargetVelocityMotorSliderJoint::solveVelocityConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_v1, corr_v2;
// 	i_math::vector3df corr_omega1, corr_omega2;
// 	const bool res = PositionBasedRigidBodyDynamics::velocitySolve_TargetVelocityMotorSliderJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getVelocity(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb1.getAngularVelocity(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getVelocity(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		rb2.getAngularVelocity(),
// 		m_target,
// 		m_jointInfo,
// 		corr_v1,
// 		corr_omega1,
// 		corr_v2,
// 		corr_omega2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getVelocity() += corr_v1;
// 			rb1.getAngularVelocity() += corr_omega1;
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getVelocity() += corr_v2;
// 			rb2.getAngularVelocity() += corr_omega2;
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// TargetAngleMotorHingeJoint
//////////////////////////////////////////////////////////////////////////
// bool TargetAngleMotorHingeJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_TargetAngleMotorHingeJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos, axis,
// 		m_jointInfo);
// }
// 
// bool TargetAngleMotorHingeJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_TargetAngleMotorHingeJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool TargetAngleMotorHingeJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_TargetAngleMotorHingeJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_target,
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// TargetVelocityMotorHingeJoint
//////////////////////////////////////////////////////////////////////////
// bool TargetVelocityMotorHingeJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis)
// {
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_TargetVelocityMotorHingeJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos, axis,
// 		m_jointInfo);
// }
// 
// bool TargetVelocityMotorHingeJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_TargetVelocityMotorHingeJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool TargetVelocityMotorHingeJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_TargetVelocityMotorHingeJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }
// 
// bool TargetVelocityMotorHingeJoint::solveVelocityConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_v1, corr_v2;
// 	i_math::vector3df corr_omega1, corr_omega2;
// 	const bool res = PositionBasedRigidBodyDynamics::velocitySolve_TargetVelocityMotorHingeJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getVelocity(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getAngularVelocity(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getVelocity(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getAngularVelocity(),
// 		m_target, 
// 		m_jointInfo,
// 		corr_v1,
// 		corr_omega1,
// 		corr_v2,
// 		corr_omega2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getVelocity() += corr_v1;
// 			rb1.getAngularVelocity() += corr_omega1;
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getVelocity() += corr_v2;
// 			rb2.getAngularVelocity() += corr_omega2;
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// DamperJoint
//////////////////////////////////////////////////////////////////////////
// bool DamperJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis, const Real stiffness)
// {
// 	m_stiffness = stiffness;
// 	m_lambda = 0.0;
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_DamperJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		axis,
// 		m_jointInfo);
// }
// 
// bool DamperJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_DamperJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool DamperJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	const Real dt = TimeManager::getCurrent()->getTimeStepSize();
// 
// 	if (iter == 0)
// 		m_lambda = 0.0;
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_DamperJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_stiffness,
// 		dt,
// 		m_jointInfo,
// 		m_lambda,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// RigidBodyParticleBallJoint
//////////////////////////////////////////////////////////////////////////
// bool RigidBodyParticleBallJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex, const unsigned int particleIndex)
// {
// 	m_bodies[0] = rbIndex;
// 	m_bodies[1] = particleIndex;
// 	SimulationModel::RigidBodyVector &rbs = model.getRigidBodies();
// 	ParticleData &pd = model.getParticles();
// 	RigidBody &rb = *rbs[m_bodies[0]];
// 	return PositionBasedRigidBodyDynamics::init_RigidBodyParticleBallJoint(
// 		rb.getPosition(),
// 		rb.getRotation(),
// 		pd.getPosition(particleIndex),
// 		m_jointInfo);
// }
// 
// bool RigidBodyParticleBallJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	ParticleData &pd = model.getParticles();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	return PositionBasedRigidBodyDynamics::update_RigidBodyParticleBallJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		pd.getPosition(m_bodies[1]),
// 		m_jointInfo);
// }
// 
// bool RigidBodyParticleBallJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	ParticleData &pd = model.getParticles();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_RigidBodyParticleBallJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		pd.getInvMass(m_bodies[1]),
// 		pd.getPosition(m_bodies[1]),
// 		m_jointInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (pd.getMass(m_bodies[1]) != 0.0)
// 		{
// 			pd.getPosition(m_bodies[1]) += corr_x2;
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// RigidBodySpring
//////////////////////////////////////////////////////////////////////////
// bool RigidBodySpring::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos1, const i_math::vector3df &pos2, const Real stiffness)
// {
// 	m_stiffness = stiffness;
// 	m_lambda = 0.0;
// 	m_restLength = (pos1 - pos2).norm();
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_DistanceJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos1,
// 		pos2, 
// 		m_jointInfo);
// }
// 
// bool RigidBodySpring::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_DistanceJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool RigidBodySpring::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	const Real dt = TimeManager::getCurrent()->getTimeStepSize();
// 
// 	if (iter == 0)
// 		m_lambda = 0.0;
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_DistanceJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		m_stiffness, 
// 		m_restLength,
// 		dt,
// 		m_jointInfo,
// 		m_lambda,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// DistanceJoint
//////////////////////////////////////////////////////////////////////////
// bool DistanceJoint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos1, const i_math::vector3df &pos2)
// {
// 	m_restLength = (pos1 - pos2).norm();
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::init_DistanceJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		pos1,
// 		pos2,
// 		m_jointInfo);
// }
// 
// bool DistanceJoint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 	return PositionBasedRigidBodyDynamics::update_DistanceJoint(
// 		rb1.getPosition(),
// 		rb1.getRotation(),
// 		rb2.getPosition(),
// 		rb2.getRotation(),
// 		m_jointInfo);
// }
// 
// bool DistanceJoint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	Real lambda = 0.0;
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = PositionBasedRigidBodyDynamics::solve_DistanceJoint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		0.0,
// 		m_restLength,
// 		0.0,
// 		m_jointInfo,
// 		lambda,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += corr_x1;
// 			rb1.getRotation().coeffs() += corr_q1.coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getPosition() += corr_x2;
// 			rb2.getRotation().coeffs() += corr_q2.coeffs();
// 			rb2.getRotation().normalize();
// 			rb2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// FEMTriangleConstraint
//////////////////////////////////////////////////////////////////////////
// bool FEMTriangleConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 	const unsigned int particle3)
// {
// 	m_bodies[0] = particle1;
// 	m_bodies[1] = particle2;
// 	m_bodies[2] = particle3;
// 	
// 	ParticleData &pd = model.getParticles();
// 
// 	i_math::vector3df &x1 = pd.getPosition0(particle1);
// 	i_math::vector3df &x2 = pd.getPosition0(particle2);
// 	i_math::vector3df &x3 = pd.getPosition0(particle3);
// 
// 	return PositionBasedDynamics::init_FEMTriangleConstraint(x1, x2, x3, m_area, m_invRestMat);
// }
// 
// bool FEMTriangleConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 
// 	const unsigned i1 = m_bodies[0];
// 	const unsigned i2 = m_bodies[1];
// 	const unsigned i3 = m_bodies[2];
// 
// 	i_math::vector3df &x1 = pd.getPosition(i1);
// 	i_math::vector3df &x2 = pd.getPosition(i2);
// 	i_math::vector3df &x3 = pd.getPosition(i3);
// 
// 	const Real invMass1 = pd.getInvMass(i1);
// 	const Real invMass2 = pd.getInvMass(i2);
// 	const Real invMass3 = pd.getInvMass(i3);
// 	
// 	i_math::vector3df corr1, corr2, corr3;
// 	const bool res = PositionBasedDynamics::solve_FEMTriangleConstraint(
// 		x1, invMass1,
// 		x2, invMass2,
// 		x3, invMass3,
// 		m_area,
// 		m_invRestMat,
// 		model.getValue<Real>(SimulationModel::CLOTH_STIFFNESS_XX),
// 		model.getValue<Real>(SimulationModel::CLOTH_STIFFNESS_YY),
// 		model.getValue<Real>(SimulationModel::CLOTH_STIFFNESS_XY),
// 		model.getValue<Real>(SimulationModel::CLOTH_POISSON_RATIO_XY),
// 		model.getValue<Real>(SimulationModel::CLOTH_POISSON_RATIO_YX),
// 		corr1, corr2, corr3);
// 
// 	if (res)
// 	{
// 		if (invMass1 != 0.0)
// 			x1 += corr1;
// 		if (invMass2 != 0.0)
// 			x2 += corr2;
// 		if (invMass3 != 0.0)
// 			x3 += corr3;
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// StrainTriangleConstraint
//////////////////////////////////////////////////////////////////////////
// bool StrainTriangleConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 	const unsigned int particle3)
// {
// 	m_bodies[0] = particle1;
// 	m_bodies[1] = particle2;
// 	m_bodies[2] = particle3;
// 
// 	ParticleData &pd = model.getParticles();
// 
// 	i_math::vector3df &x1 = pd.getPosition0(particle1);
// 	i_math::vector3df &x2 = pd.getPosition0(particle2);
// 	i_math::vector3df &x3 = pd.getPosition0(particle3);
// 
// 	// Bring triangles to xy plane
// 	const i_math::vector3df y1(x1[0], x1[2], 0.0);
// 	const i_math::vector3df y2(x2[0], x2[2], 0.0);
// 	const i_math::vector3df y3(x3[0], x3[2], 0.0);
// 
// 	return PositionBasedDynamics::init_StrainTriangleConstraint(y1, y2, y3, m_invRestMat);
// }
// 
// bool StrainTriangleConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 
// 	const unsigned i1 = m_bodies[0];
// 	const unsigned i2 = m_bodies[1];
// 	const unsigned i3 = m_bodies[2];
// 
// 	i_math::vector3df &x1 = pd.getPosition(i1);
// 	i_math::vector3df &x2 = pd.getPosition(i2);
// 	i_math::vector3df &x3 = pd.getPosition(i3);
// 
// 	const Real invMass1 = pd.getInvMass(i1);
// 	const Real invMass2 = pd.getInvMass(i2);
// 	const Real invMass3 = pd.getInvMass(i3);
// 
// 	i_math::vector3df corr1, corr2, corr3;
// 	const bool res = PositionBasedDynamics::solve_StrainTriangleConstraint(
// 		x1, invMass1,
// 		x2, invMass2,
// 		x3, invMass3,
// 		m_invRestMat,
// 		model.getValue<Real>(SimulationModel::CLOTH_STIFFNESS_XX),
// 		model.getValue<Real>(SimulationModel::CLOTH_STIFFNESS_YY),
// 		model.getValue<Real>(SimulationModel::CLOTH_STIFFNESS_XY),
// 		model.getValue<bool>(SimulationModel::CLOTH_NORMALIZE_STRETCH),
// 		model.getValue<bool>(SimulationModel::CLOTH_NORMALIZE_SHEAR),
// 		corr1, corr2, corr3);
// 
// 	if (res)
// 	{
// 		if (invMass1 != 0.0)
// 			x1 += corr1;
// 		if (invMass2 != 0.0)
// 			x2 += corr2;
// 		if (invMass3 != 0.0)
// 			x3 += corr3;
// 	}
// 	return res;
// }



//////////////////////////////////////////////////////////////////////////
// FEMTetConstraint
//////////////////////////////////////////////////////////////////////////
// bool FEMTetConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 									const unsigned int particle3, const unsigned int particle4)
// {
// 	m_bodies[0] = particle1;
// 	m_bodies[1] = particle2;
// 	m_bodies[2] = particle3;
// 	m_bodies[3] = particle4;
// 
// 	ParticleData &pd = model.getParticles();
// 
// 	i_math::vector3df &x1 = pd.getPosition0(particle1);
// 	i_math::vector3df &x2 = pd.getPosition0(particle2);
// 	i_math::vector3df &x3 = pd.getPosition0(particle3);
// 	i_math::vector3df &x4 = pd.getPosition0(particle4);
// 
// 	return PositionBasedDynamics::init_FEMTetraConstraint(x1, x2, x3, x4, m_volume, m_invRestMat);
// }
// 
// bool FEMTetConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 
// 	const unsigned i1 = m_bodies[0];
// 	const unsigned i2 = m_bodies[1];
// 	const unsigned i3 = m_bodies[2];
// 	const unsigned i4 = m_bodies[3];
// 
// 	i_math::vector3df &x1 = pd.getPosition(i1);
// 	i_math::vector3df &x2 = pd.getPosition(i2);
// 	i_math::vector3df &x3 = pd.getPosition(i3);
// 	i_math::vector3df &x4 = pd.getPosition(i4);
// 
// 	const Real invMass1 = pd.getInvMass(i1);
// 	const Real invMass2 = pd.getInvMass(i2);
// 	const Real invMass3 = pd.getInvMass(i3);
// 	const Real invMass4 = pd.getInvMass(i4);
// 
// 	Real currentVolume = -static_cast<Real>(1.0 / 6.0) * (x4 - x1).dot((x3 - x1).cross(x2 - x1));
// 	bool handleInversion = false;
// 	if (currentVolume / m_volume < 0.2)		// Only 20% of initial volume left
// 		handleInversion = true;
// 
// 
// 	i_math::vector3df corr1, corr2, corr3, corr4;
// 	const bool res = PositionBasedDynamics::solve_FEMTetraConstraint(
// 		x1, invMass1,
// 		x2, invMass2,
// 		x3, invMass3,
// 		x4, invMass4,
// 		m_volume,
// 		m_invRestMat,
// 		model.getValue<Real>(SimulationModel::SOLID_STIFFNESS),
// 		model.getValue<Real>(SimulationModel::SOLID_POISSON_RATIO), handleInversion,
// 		corr1, corr2, corr3, corr4);
// 
// 	if (res)
// 	{
// 		if (invMass1 != 0.0)
// 			x1 += corr1;
// 		if (invMass2 != 0.0)
// 			x2 += corr2;
// 		if (invMass3 != 0.0)
// 			x3 += corr3;
// 		if (invMass4 != 0.0)
// 			x4 += corr4;
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// StrainTetConstraint
//////////////////////////////////////////////////////////////////////////
// bool StrainTetConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 	const unsigned int particle3, const unsigned int particle4)
// {
// 	m_bodies[0] = particle1;
// 	m_bodies[1] = particle2;
// 	m_bodies[2] = particle3;
// 	m_bodies[3] = particle4;
// 
// 	ParticleData &pd = model.getParticles();
// 
// 	i_math::vector3df &x1 = pd.getPosition0(particle1);
// 	i_math::vector3df &x2 = pd.getPosition0(particle2);
// 	i_math::vector3df &x3 = pd.getPosition0(particle3);
// 	i_math::vector3df &x4 = pd.getPosition0(particle4);
// 
// 	return PositionBasedDynamics::init_StrainTetraConstraint(x1, x2, x3, x4, m_invRestMat);
// }
// 
// bool StrainTetConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 
// 	const unsigned i1 = m_bodies[0];
// 	const unsigned i2 = m_bodies[1];
// 	const unsigned i3 = m_bodies[2];
// 	const unsigned i4 = m_bodies[3];
// 
// 	i_math::vector3df &x1 = pd.getPosition(i1);
// 	i_math::vector3df &x2 = pd.getPosition(i2);
// 	i_math::vector3df &x3 = pd.getPosition(i3);
// 	i_math::vector3df &x4 = pd.getPosition(i4);
// 
// 	const Real invMass1 = pd.getInvMass(i1);
// 	const Real invMass2 = pd.getInvMass(i2);
// 	const Real invMass3 = pd.getInvMass(i3);
// 	const Real invMass4 = pd.getInvMass(i4);
// 
// 	const Real stiff = model.getValue<Real>(SimulationModel::SOLID_STIFFNESS);
// 	i_math::vector3df stiffness(stiff, stiff, stiff);
// 
// 	i_math::vector3df corr1, corr2, corr3, corr4;
// 	const bool res = PositionBasedDynamics::solve_StrainTetraConstraint(
// 		x1, invMass1,
// 		x2, invMass2,
// 		x3, invMass3,
// 		x4, invMass4,
// 		m_invRestMat,
// 		stiffness,
// 		stiffness,
// 		model.getValue<bool>(SimulationModel::SOLID_NORMALIZE_STRETCH),
// 		model.getValue<bool>(SimulationModel::SOLID_NORMALIZE_SHEAR),
// 		corr1, corr2, corr3, corr4);
// 
// 	if (res)
// 	{
// 		if (invMass1 != 0.0)
// 			x1 += corr1;
// 		if (invMass2 != 0.0)
// 			x2 += corr2;
// 		if (invMass3 != 0.0)
// 			x3 += corr3;
// 		if (invMass4 != 0.0)
// 			x4 += corr4;
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// ShapeMatchingConstraint
//////////////////////////////////////////////////////////////////////////
// bool ShapeMatchingConstraint::initConstraint(SimulationModel &model, 
// 			const unsigned int particleIndices[], const unsigned int numClusters[])
// {
// 	ParticleData &pd = model.getParticles();
// 	for (unsigned int i = 0; i < m_numberOfBodies; i++)
// 	{
// 		m_bodies[i] = particleIndices[i];
// 		m_x0[i] = pd.getPosition0(m_bodies[i]);
// 		m_w[i] = pd.getInvMass(m_bodies[i]);
// 		m_numClusters[i] = numClusters[i];
// 	}
// 
// 	const bool res = PositionBasedDynamics::init_ShapeMatchingConstraint(m_x0, m_w, m_numberOfBodies, m_restCm, m_invRestMat);
// 	return res;
// }
// 
// bool ShapeMatchingConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 	for (unsigned int i = 0; i < m_numberOfBodies; i++)
// 	{
// 		m_x[i] = pd.getPosition(m_bodies[i]);
// 	}
// 
// 	const bool res = PositionBasedDynamics::solve_ShapeMatchingConstraint(
// 		m_x0, m_x, m_w, m_numberOfBodies,
// 		m_restCm, m_invRestMat,
// 		model.getValue<Real>(SimulationModel::SOLID_STIFFNESS), false,
// 		m_corr);
// 
// 	if (res)
// 	{
// 		for (unsigned int i = 0; i < m_numberOfBodies; i++)
// 		{
// 			// Important: Divide position correction by the number of clusters 
// 			// which contain the vertex. 
// 			if (m_w[i] != 0.0)
// 				pd.getPosition(m_bodies[i]) += (1.0 / m_numClusters[i]) * m_corr[i];
// 		}
// 	}
// 	return res;
// }


//////////////////////////////////////////////////////////////////////////
// RigidBodyContactConstraint
//////////////////////////////////////////////////////////////////////////
// bool RigidBodyContactConstraint::initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2,
// 		const i_math::vector3df &cp1, const i_math::vector3df &cp2,
// 		const i_math::vector3df &normal, const Real dist,
// 		const Real restitutionCoeff, const Real stiffness, const Real frictionCoeff)
// {
// 	m_stiffness = stiffness;
// 	m_frictionCoeff = frictionCoeff;
// 
// 	m_bodies[0] = rbIndex1;
// 	m_bodies[1] = rbIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	m_sum_impulses = 0.0;
// 
// 	return PositionBasedRigidBodyDynamics::init_RigidBodyContactConstraint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getVelocity(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getRotation(),
// 		rb1.getAngularVelocity(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getVelocity(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getRotation(),
// 		rb2.getAngularVelocity(),
// 		cp1, cp2, normal, restitutionCoeff, 
// 		m_constraintInfo);
// }
// 
// bool RigidBodyContactConstraint::solveVelocityConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &rb1 = *rb[m_bodies[0]];
// 	RigidBody &rb2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_v1, corr_v2;
// 	i_math::vector3df corr_omega1, corr_omega2;
// 	const bool res = PositionBasedRigidBodyDynamics::velocitySolve_RigidBodyContactConstraint(
// 		rb1.getInvMass(),
// 		rb1.getPosition(),
// 		rb1.getVelocity(),
// 		rb1.getInertiaTensorInverseW(),
// 		rb1.getAngularVelocity(),
// 		rb2.getInvMass(),
// 		rb2.getPosition(),
// 		rb2.getVelocity(),
// 		rb2.getInertiaTensorInverseW(),
// 		rb2.getAngularVelocity(),
// 		m_stiffness,
// 		m_frictionCoeff,
// 		m_sum_impulses,
// 		m_constraintInfo,
// 		corr_v1,
// 		corr_omega1,
// 		corr_v2,
// 		corr_omega2);
// 
// 	if (res)
// 	{
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getVelocity() += corr_v1;
// 			rb1.getAngularVelocity() += corr_omega1;
// 		}
// 		if (rb2.getMass() != 0.0)
// 		{
// 			rb2.getVelocity() += corr_v2;
// 			rb2.getAngularVelocity() += corr_omega2;
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// ParticleRigidBodyContactConstraint
//////////////////////////////////////////////////////////////////////////
// bool ParticleRigidBodyContactConstraint::initConstraint(SimulationModel &model, 
// 	const unsigned int particleIndex, const unsigned int rbIndex,
// 	const i_math::vector3df &cp1, const i_math::vector3df &cp2,
// 	const i_math::vector3df &normal, const Real dist,
// 	const Real restitutionCoeff, const Real stiffness, const Real frictionCoeff)
// {
// 	m_stiffness = stiffness;
// 	m_frictionCoeff = frictionCoeff;
// 
// 	m_bodies[0] = particleIndex;
// 	m_bodies[1] = rbIndex;
// 	SimulationModel::RigidBodyVector &rbs = model.getRigidBodies();
// 	ParticleData &pd = model.getParticles();
// 
// 	RigidBody &rb = *rbs[m_bodies[1]];
// 
// 	m_sum_impulses = 0.0;
// 
// 	return PositionBasedRigidBodyDynamics::init_ParticleRigidBodyContactConstraint(
// 		pd.getInvMass(particleIndex),
// 		pd.getPosition(particleIndex),
// 		pd.getVelocity(particleIndex),
// 		rb.getInvMass(),
// 		rb.getPosition(),
// 		rb.getVelocity(),
// 		rb.getInertiaTensorInverseW(),
// 		rb.getRotation(),
// 		rb.getAngularVelocity(),		
// 		cp1, cp2, normal, restitutionCoeff,
// 		m_constraintInfo);
// }
// 
// bool ParticleRigidBodyContactConstraint::solveVelocityConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rbs = model.getRigidBodies();
// 	ParticleData &pd = model.getParticles();
// 
// 	RigidBody &rb = *rbs[m_bodies[1]];
// 
// 	i_math::vector3df corr_v1, corr_v2;
// 	i_math::vector3df corr_omega2;
// 	const bool res = PositionBasedRigidBodyDynamics::velocitySolve_ParticleRigidBodyContactConstraint(
// 		pd.getInvMass(m_bodies[0]),
// 		pd.getPosition(m_bodies[0]),
// 		pd.getVelocity(m_bodies[0]),
// 		rb.getInvMass(),
// 		rb.getPosition(),
// 		rb.getVelocity(),
// 		rb.getInertiaTensorInverseW(),
// 		rb.getAngularVelocity(),
// 		m_stiffness,
// 		m_frictionCoeff,
// 		m_sum_impulses,
// 		m_constraintInfo,
// 		corr_v1,		
// 		corr_v2, 
// 		corr_omega2);
// 
// 	if (res)
// 	{
// 		if (pd.getMass(m_bodies[0]) != 0.0)
// 		{
// 			pd.getVelocity(m_bodies[0]) += corr_v1;
// 		}
// 		if (rb.getMass() != 0.0)
// 		{
// 			rb.getVelocity() += corr_v2;
// 			rb.getAngularVelocity() += corr_omega2;
// 		}	
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// ParticleSolidContactConstraint
//////////////////////////////////////////////////////////////////////////
// bool ParticleTetContactConstraint::initConstraint(SimulationModel &model,
// 	const unsigned int particleIndex, const unsigned int solidIndex,
// 	const unsigned int tetIndex, const i_math::vector3df &bary,
// 	const i_math::vector3df &cp1, const i_math::vector3df &cp2,
// 	const i_math::vector3df &normal, const Real dist,
// 	const Real frictionCoeff)
// {
// 	m_frictionCoeff = frictionCoeff;
// 
// 	m_bodies[0] = particleIndex;
// 	m_bodies[1] = solidIndex;
// 	m_tetIndex = tetIndex;
// 	m_solidIndex = solidIndex;
// 	m_bary = bary;
// 	ParticleData &pd = model.getParticles();
// 
// 	const SimulationModel::TetModelVector &tetModels = model.getTetModels();
// 	TetModel *tm = tetModels[solidIndex];
// 	const unsigned int offset = tm->getIndexOffset();
// 	const unsigned int *indices = tm->getParticleMesh().getTets().data();
// 	m_x[0] = pd.getPosition(indices[4 * tetIndex] + offset);
// 	m_x[1] = pd.getPosition(indices[4 * tetIndex + 1] + offset);
// 	m_x[2] = pd.getPosition(indices[4 * tetIndex + 2] + offset);
// 	m_x[3] = pd.getPosition(indices[4 * tetIndex + 3] + offset);
// 	m_v[0] = pd.getVelocity(indices[4 * tetIndex] + offset);
// 	m_v[1] = pd.getVelocity(indices[4 * tetIndex + 1] + offset);
// 	m_v[2] = pd.getVelocity(indices[4 * tetIndex + 2] + offset);
// 	m_v[3] = pd.getVelocity(indices[4 * tetIndex + 3] + offset);
// 	m_invMasses[0] = pd.getInvMass(indices[4 * tetIndex] + offset);
// 	m_invMasses[1] = pd.getInvMass(indices[4 * tetIndex + 1] + offset);
// 	m_invMasses[2] = pd.getInvMass(indices[4 * tetIndex + 2] + offset);
// 	m_invMasses[3] = pd.getInvMass(indices[4 * tetIndex + 3] + offset);
// 
// 	return PositionBasedDynamics::init_ParticleTetContactConstraint(
// 		pd.getInvMass(particleIndex),
// 		pd.getPosition(particleIndex),
// 		pd.getVelocity(particleIndex),
// 		m_invMasses,
// 		m_x,
// 		m_v,
// 		bary, normal, 
// 		m_constraintInfo);
// }
// 
// bool ParticleTetContactConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 
// 	const SimulationModel::TetModelVector &tetModels = model.getTetModels();
// 	TetModel *tm = tetModels[m_solidIndex];
// 	const unsigned int offset = tm->getIndexOffset();
// 	const unsigned int *indices = tm->getParticleMesh().getTets().data();
// 	i_math::vector3df &x0 = pd.getPosition(indices[4 * m_tetIndex] + offset);
// 	i_math::vector3df &x1 = pd.getPosition(indices[4 * m_tetIndex + 1] + offset);
// 	i_math::vector3df &x2 = pd.getPosition(indices[4 * m_tetIndex + 2] + offset);
// 	i_math::vector3df &x3 = pd.getPosition(indices[4 * m_tetIndex + 3] + offset);
// 
// 	i_math::vector3df corr0;
// 	i_math::vector3df corr[4];
// 	const bool res = PositionBasedDynamics::solve_ParticleTetContactConstraint(
// 		pd.getInvMass(m_bodies[0]),
// 		pd.getPosition(m_bodies[0]),
// 		m_invMasses,
// 		m_x,
// 		m_bary,
// 		m_constraintInfo,
// 		m_lambda,
// 		corr0,
// 		corr);
// 
// 	if (res)
// 	{
// 		if (pd.getMass(m_bodies[0]) != 0.0)
// 			pd.getPosition(m_bodies[0]) += corr0;
// 		if (m_invMasses[0] != 0.0)
// 			x0 += corr[0];
// 		if (m_invMasses[1] != 0.0)
// 			x1 += corr[1];
// 		if (m_invMasses[2] != 0.0)
// 			x2 += corr[2];
// 		if (m_invMasses[3] != 0.0)
// 			x3 += corr[3];
// 	}
// 	return res;
// }
// 
// bool ParticleTetContactConstraint::solveVelocityConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 
// 	const SimulationModel::TetModelVector &tetModels = model.getTetModels();
// 	TetModel *tm = tetModels[m_solidIndex];
// 	const unsigned int offset = tm->getIndexOffset();
// 	const unsigned int *indices = tm->getParticleMesh().getTets().data();
// 	i_math::vector3df &v0 = pd.getVelocity(indices[4 * m_tetIndex] + offset);
// 	i_math::vector3df &v1 = pd.getVelocity(indices[4 * m_tetIndex + 1] + offset);
// 	i_math::vector3df &v2 = pd.getVelocity(indices[4 * m_tetIndex + 2] + offset);
// 	i_math::vector3df &v3 = pd.getVelocity(indices[4 * m_tetIndex + 3] + offset);
// 	m_v[0] = v0;
// 	m_v[1] = v1;
// 	m_v[2] = v2;
// 	m_v[3] = v3;
// 
// 
// 	i_math::vector3df corr_v0;
// 	i_math::vector3df corr_v[4];
//  	const bool res = PositionBasedDynamics::velocitySolve_ParticleTetContactConstraint(
//  		pd.getInvMass(m_bodies[0]),
//  		pd.getPosition(m_bodies[0]),
//  		pd.getVelocity(m_bodies[0]),
//  		m_invMasses,
//  		m_x,
//  		m_v,
// 		m_bary, 
// 		m_lambda,
//  		m_frictionCoeff,
//  		m_constraintInfo,
//  		corr_v0,
//  		corr_v);
// 
// 	if (res)
// 	{
// 		if (pd.getMass(m_bodies[0]) != 0.0)
// 			pd.getVelocity(m_bodies[0]) += corr_v0;
// 		if (m_invMasses[0] != 0.0)
// 			v0 += corr_v[0];
// 		if (m_invMasses[1] != 0.0)
// 			v1 += corr_v[1];
// 		if (m_invMasses[2] != 0.0)
// 			v2 += corr_v[2];
// 		if (m_invMasses[3] != 0.0)
// 			v3 += corr_v[3];
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// StretchShearConstraint
//////////////////////////////////////////////////////////////////////////
// bool StretchShearConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2, const unsigned int quaternion1)
// {
// 	m_bodies[0] = particle1;
// 	m_bodies[1] = particle2;
// 	m_bodies[2] = quaternion1;
// 	ParticleData &pd = model.getParticles();
// 
// 	const i_math::vector3df &x1_0 = pd.getPosition0(particle1);
// 	const i_math::vector3df &x2_0 = pd.getPosition0(particle2);
// 
// 	m_restLength = (x2_0 - x1_0).getLength();
// 
// 	return true;
// }
// 
// bool StretchShearConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	ParticleData &pd = model.getParticles();
// 	OrientationData &od = model.getOrientations();
// 
// 	const unsigned i1 = m_bodies[0];
// 	const unsigned i2 = m_bodies[1];
// 	const unsigned iq1 = m_bodies[2];
// 
// 	i_math::vector3df &x1 = pd.getPosition(i1);
// 	i_math::vector3df &x2 = pd.getPosition(i2);
// 	i_math::quatf &q1 = od.getQuaternion(iq1);
// 	const Real invMass1 = pd.getInvMass(i1);
// 	const Real invMass2 = pd.getInvMass(i2);
// 	const Real invMassq1 = od.getInvMass(iq1);
// 	i_math::vector3df stiffness(model.getRodShearingStiffness1(), 
// 					   model.getRodShearingStiffness2(), 
// 					   model.getRodStretchingStiffness());
// 
// 	i_math::vector3df corr1, corr2;
// 	i_math::quatf corrq1;
// 	const bool res = PositionBasedCosseratRods::solve_StretchShearConstraint(
// 		x1, invMass1, x2, invMass2, q1, invMassq1, 
// 		stiffness,
// 		m_restLength, corr1, corr2, corrq1);
// 
// 	if (res)
// 	{
// 		if (invMass1 != 0.0)
// 			x1 += corr1;
// 		if (invMass2 != 0.0)
// 			x2 += corr2;
// 		if (invMassq1 != 0.0)
// 		{
// 			q1.coeffs() += corrq1.coeffs();
// 			q1.normalize();
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// BendTwistConstraint
//////////////////////////////////////////////////////////////////////////
// bool BendTwistConstraint::initConstraint(SimulationModel &model, const unsigned int quaternion1, const unsigned int quaternion2)
// {
// 	m_bodies[0] = quaternion1;
// 	m_bodies[1] = quaternion2;
// 	OrientationData &od = model.getOrientations();
// 
// 	const i_math::quatf &q1_0 = od.getQuaternion(quaternion1);
// 	const i_math::quatf &q2_0 = od.getQuaternion(quaternion2);
// 
// 	m_restDarbouxVector = q1_0.conjugate() * q2_0;
// 	i_math::quatf omega_plus, omega_minus;
// 	omega_plus.coeffs() = m_restDarbouxVector.coeffs() + i_math::quatf(1, 0, 0, 0).coeffs();
// 	omega_minus.coeffs() = m_restDarbouxVector.coeffs() - i_math::quatf(1, 0, 0, 0).coeffs();
// 	if (omega_minus.squaredNorm() > omega_plus.squaredNorm())
// 		m_restDarbouxVector.coeffs() *= -1.0;
// 
// 	return true;
// }
// 
// bool BendTwistConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	OrientationData &od = model.getOrientations();
// 
// 	const unsigned i1 = m_bodies[0];
// 	const unsigned i2 = m_bodies[1];
// 
// 	i_math::quatf &q1 = od.getQuaternion(i1);
// 	i_math::quatf &q2 = od.getQuaternion(i2);
// 	const Real invMass1 = od.getInvMass(i1);
// 	const Real invMass2 = od.getInvMass(i2);
// 	i_math::vector3df stiffness(model.getRodBendingStiffness1(), 
// 					   model.getRodBendingStiffness2(), 
// 					   model.getRodTwistingStiffness());
// 
// 	i_math::quatf corr1, corr2;
// 	const bool res = PositionBasedCosseratRods::solve_BendTwistConstraint(
// 		q1, invMass1, q2, invMass2, 
// 		stiffness,
// 		m_restDarbouxVector, corr1, corr2);
// 
// 	if (res)
// 	{
// 		if (invMass1 != 0.0)
// 		{
// 			q1.coeffs() += corr1.coeffs();
// 			q1.normalize();
// 		}
// 			
// 		if (invMass2 != 0.0)
// 		{
// 			q2.coeffs() += corr2.coeffs();
// 			q2.normalize();
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// StretchBendingTwistingConstraint
//////////////////////////////////////////////////////////////////////////

// bool PBD::StretchBendingTwistingConstraint::initConstraint(
// 	SimulationModel &model,
// 	const unsigned int segmentIndex1,
// 	const unsigned int segmentIndex2,
// 	const i_math::vector3df &pos,
// 	const Real averageRadius,
// 	const Real averageSegmentLength,
// 	Real youngsModulus,
// 	Real torsionModulus)
// {
// 	m_bodies[0] = segmentIndex1;
// 	m_bodies[1] = segmentIndex2;
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	const RigidBody &segment1 = *rb[m_bodies[0]];
// 	const RigidBody &segment2 = *rb[m_bodies[1]];
// 
// 	m_lambdaSum.setZero();
// 	m_averageRadius = averageRadius;
// 	m_averageSegmentLength = averageSegmentLength;
// 
// 	return DirectPositionBasedSolverForStiffRods::init_StretchBendingTwistingConstraint(
// 		segment1.getPosition(),
// 		segment1.getRotation(),
// 		segment2.getPosition(),
// 		segment2.getRotation(),
// 		pos,
// 		m_averageRadius,
// 		m_averageSegmentLength,
// 		youngsModulus,
// 		torsionModulus,
// 		m_constraintInfo,
// 		m_stiffnessCoefficientK,
// 		m_restDarbouxVector);
// }
// 
// 
// bool StretchBendingTwistingConstraint::initConstraintBeforeProjection(SimulationModel &model)
// {
// 	DirectPositionBasedSolverForStiffRods::initBeforeProjection_StretchBendingTwistingConstraint(
// 		m_stiffnessCoefficientK,
// 		static_cast<Real>(1.0) / TimeManager::getCurrent()->getTimeStepSize(),
// 		m_averageSegmentLength,
// 		m_stretchCompliance,
// 		m_bendingAndTorsionCompliance,
// 		m_lambdaSum);
// 	return true;
// };
// 
// bool StretchBendingTwistingConstraint::updateConstraint(SimulationModel &model)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 	const RigidBody &segment1 = *rb[m_bodies[0]];
// 	const RigidBody &segment2 = *rb[m_bodies[1]];
// 	return DirectPositionBasedSolverForStiffRods::update_StretchBendingTwistingConstraint(
// 		segment1.getPosition(),
// 		segment1.getRotation(),
// 		segment2.getPosition(),
// 		segment2.getRotation(),		
// 		m_constraintInfo);
// }
// 
// bool StretchBendingTwistingConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	SimulationModel::RigidBodyVector &rb = model.getRigidBodies();
// 
// 	RigidBody &segment1 = *rb[m_bodies[0]];
// 	RigidBody &segment2 = *rb[m_bodies[1]];
// 
// 	i_math::vector3df corr_x1, corr_x2;
// 	i_math::quatf corr_q1, corr_q2;
// 	const bool res = DirectPositionBasedSolverForStiffRods::solve_StretchBendingTwistingConstraint(
// 		segment1.getInvMass(),
// 		segment1.getPosition(),
// 		segment1.getInertiaTensorInverseW(),
// 		segment1.getRotation(),
// 		segment2.getInvMass(),
// 		segment2.getPosition(),
// 		segment2.getInertiaTensorInverseW(),
// 		segment2.getRotation(),
// 		m_restDarbouxVector,
// 		m_averageSegmentLength,
// 		m_stretchCompliance,
// 		m_bendingAndTorsionCompliance,
// 		m_constraintInfo,
// 		corr_x1,
// 		corr_q1,
// 		corr_x2,
// 		corr_q2,
// 		m_lambdaSum);
// 
// 	if (res)
// 	{
// 		if (segment1.getMass() != 0.0)
// 		{
// 			segment1.getPosition() += corr_x1;
// 			segment1.getRotation().coeffs() += corr_q1.coeffs();
// 			segment1.getRotation().normalize();
// 			segment1.rotationUpdated();
// 		}
// 		if (segment2.getMass() != 0.0)
// 		{
// 			segment2.getPosition() += corr_x2;
// 			segment2.getRotation().coeffs() += corr_q2.coeffs();
// 			segment2.getRotation().normalize();
// 			segment2.rotationUpdated();
// 		}
// 	}
// 	return res;
// }

//////////////////////////////////////////////////////////////////////////
// DirectPositionBasedSolverForStiffRodsConstraint
//////////////////////////////////////////////////////////////////////////

// PBD::DirectPositionBasedSolverForStiffRodsConstraint::~DirectPositionBasedSolverForStiffRodsConstraint()
// {
// 	deleteNodes();
// 	if (intervals != NULL)
// 		delete[] intervals;
// 	if (forward != NULL)
// 		delete[] forward;
// 	if (backward != NULL)
// 		delete[] backward;
// 	if (root != NULL)
// 		delete[] root;
// 	root = NULL;
// 	forward = NULL;
// 	backward = NULL;
// 	intervals = NULL;
// 	numberOfIntervals = 0;
// }
// 
// void DirectPositionBasedSolverForStiffRodsConstraint::deleteNodes()
// {
// 	std::list<Node*>::iterator nodeIter;
// 	for (int i = 0; i < numberOfIntervals; i++)
// 	{
// 		for (nodeIter = forward[i].begin(); nodeIter != forward[i].end(); nodeIter++)
// 		{
// 			Node *node = *nodeIter;
// 
// 			// Root node does not have to be deleted
// 			if (node->parent != NULL)
// 				delete node;
// 		}
// 	}
// }
// 
// bool PBD::DirectPositionBasedSolverForStiffRodsConstraint::initConstraint(
// 	SimulationModel &model, 
// 	const std::vector<std::pair<unsigned int, unsigned int>> & constraintSegmentIndices, 
// 	const std::vector<i_math::vector3df> &constraintPositions,
// 	const std::vector<Real> &averageRadii,
// 	const std::vector<Real> &averageSegmentLengths,
// 	const std::vector<Real> &youngsModuli,
// 	const std::vector<Real> &torsionModuli
// 	)
// {
// 	// create unique segment indices from joints
// 
// 	std::set<unsigned int> uniqueSegmentIndices;
// 	for (auto &idxPair :  constraintSegmentIndices)
// 	{
// 		uniqueSegmentIndices.insert(idxPair.first);
// 		uniqueSegmentIndices.insert(idxPair.second);
// 	}
// 
// 	delete[] m_bodies;
// 	m_numberOfBodies = (unsigned int)uniqueSegmentIndices.size();
// 	m_bodies = new unsigned int[m_numberOfBodies];
// 
// 	// initialize m_bodies for constraint colouring algorithm of multi threading implementation
// 
// 	size_t segmentIdx(0);
// 
// 	for (auto idx : uniqueSegmentIndices)
// 	{
// 		m_bodies[segmentIdx] = idx;
// 		++segmentIdx;
// 	}
// 
// 	// create RodSegment instances and map simulation model body indices to RodSegment indices
// 
// 	std::map<unsigned int, unsigned int> idxMap;
// 	unsigned int idx(0);
// 
// 	m_Segments.reserve(uniqueSegmentIndices.size());
// 	m_rodSegments.reserve(uniqueSegmentIndices.size());
// 	for (auto bodyIdx : uniqueSegmentIndices)
// 	{
// 		idx = (unsigned int)m_Segments.size();
// 		idxMap[bodyIdx] = idx;
// 		m_Segments.push_back(RodSegmentImpl(model, bodyIdx));
// 		m_rodSegments.push_back(&m_Segments.back());
// 	}
// 
// 	// create rod constraints
// 
// 	m_Constraints.resize(constraintPositions.size());
// 	m_rodConstraints.resize(constraintPositions.size());
// 
// 	for (size_t idx(0); idx < constraintPositions.size(); ++idx)
// 	{
// 		const std::pair<unsigned int, unsigned int> &bodyIndices( constraintSegmentIndices[idx]);
// 		unsigned int firstSegmentIndex(idxMap.find(bodyIndices.first)->second);
// 		unsigned int secondSegmentIndex(idxMap.find(bodyIndices.second)->second);
// 
// 		m_Constraints[idx].m_segments.push_back(firstSegmentIndex);
// 		m_Constraints[idx].m_segments.push_back(secondSegmentIndex);
// 		m_Constraints[idx].m_averageSegmentLength = averageSegmentLengths[idx];
// 		m_rodConstraints[idx] = &m_Constraints[idx];
// 	}
// 
// 	// initialize data of the sparse direct solver
// 	deleteNodes();
// 	DirectPositionBasedSolverForStiffRods::init_DirectPositionBasedSolverForStiffRodsConstraint(
// 		m_rodConstraints, m_rodSegments, intervals, numberOfIntervals, forward, backward, root,
// 		constraintPositions, averageRadii, youngsModuli, torsionModuli,
// 		m_rightHandSide, m_lambdaSums, m_bendingAndTorsionJacobians, m_corr_x, m_corr_q);
// 
// 	return true;
// }
// 
// bool PBD::DirectPositionBasedSolverForStiffRodsConstraint::initConstraintBeforeProjection(SimulationModel &model)
// {
// 	DirectPositionBasedSolverForStiffRods::initBeforeProjection_DirectPositionBasedSolverForStiffRodsConstraint(
// 		m_rodConstraints, static_cast<Real>(1.0) / TimeManager::getCurrent()->getTimeStepSize(), m_lambdaSums);
// 	return true;
// }
// 
// 
// bool PBD::DirectPositionBasedSolverForStiffRodsConstraint::updateConstraint(SimulationModel &model)
// {
// 	DirectPositionBasedSolverForStiffRods::update_DirectPositionBasedSolverForStiffRodsConstraint(
// 		m_rodConstraints, m_rodSegments);
// 	return true;
// }
// 
// 
// bool PBD::DirectPositionBasedSolverForStiffRodsConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
// {
// 	const bool res = DirectPositionBasedSolverForStiffRods::solve_DirectPositionBasedSolverForStiffRodsConstraint(
// 		m_rodConstraints, m_rodSegments, intervals, numberOfIntervals, forward, backward,
// 		m_rightHandSide, m_lambdaSums, m_bendingAndTorsionJacobians, m_corr_x, m_corr_q
// 		);
// 	
// 	// apply corrections to bodies
// 	SimulationModel::RigidBodyVector &rbs = model.getRigidBodies();
// 
// 	for (size_t i(0); i < m_rodSegments.size(); ++i)
// 	{
// 		RodSegmentImpl & segment = m_Segments[i];
// 		RigidBody &rb1 = *rbs[segment.m_segmentIdx];
// 		if (rb1.getMass() != 0.0)
// 		{
// 			rb1.getPosition() += m_corr_x[i];
// 			rb1.getRotation().coeffs() += m_corr_q[i].coeffs();
// 			rb1.getRotation().normalize();
// 			rb1.rotationUpdated();
// 		}
// 	}
// 
// 	return res;
// }
// 
// bool PBD::DirectPositionBasedSolverForStiffRodsConstraint::RodSegmentImpl::isDynamic()
// {
// 	return 0 != (m_model.getRigidBodies())[m_segmentIdx]->getMass();
// }
// 
// Real PBD::DirectPositionBasedSolverForStiffRodsConstraint::RodSegmentImpl::Mass()
// {
// 	return (m_model.getRigidBodies())[m_segmentIdx]->getMass();
// }
// 
// const i_math::vector3df & PBD::DirectPositionBasedSolverForStiffRodsConstraint::RodSegmentImpl::InertiaTensor()
// {
// 	return (m_model.getRigidBodies())[m_segmentIdx]->getInertiaTensor();
// }
// 
// const i_math::vector3df & PBD::DirectPositionBasedSolverForStiffRodsConstraint::RodSegmentImpl::Position()
// {
// 	return (m_model.getRigidBodies())[m_segmentIdx]->getPosition();
// }
// 
// const i_math::quatf & PBD::DirectPositionBasedSolverForStiffRodsConstraint::RodSegmentImpl::Rotation()
// {
// 	return (m_model.getRigidBodies())[m_segmentIdx]->getRotation();
// }



//////////////////////////////////////////////////////////////////////////
// DistanceConstraint
//////////////////////////////////////////////////////////////////////////
bool DistanceConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,float length,float stiffness)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	ParticleData &pd = model.getParticles();

	if (length<0.0f)
	{
		const i_math::vector3df &x1_0 = pd.getPosition0(particle1);
		const i_math::vector3df &x2_0 = pd.getPosition0(particle2);

		m_restLength = (x2_0 - x1_0).getLength();
	}
	else
		m_restLength=length;

	m_stiffness=stiffness;

	return true;
}

bool DistanceConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];

	i_math::vector3df &x1 = pd.getPosition(i1);
	i_math::vector3df &x2 = pd.getPosition(i2);
	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);

	i_math::vector3df corr1, corr2;
	const bool res = PositionBasedDynamics::solve_DistanceConstraint(
		x1, invMass1, x2, invMass2,
		m_restLength, m_stiffness, m_stiffness, corr1, corr2);

	if (corr2.getLength()>0.2f)
	{
		int v=0;
		v++;
	}


	if (res)
	{
		if (invMass1 != 0.0)
			x1 += corr1;
		if (invMass2 != 0.0)
			x2 += corr2;
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
// PullConstraint
//////////////////////////////////////////////////////////////////////////
bool PullConstraint::initConstraint(SimulationModel &model, 
									const unsigned int particle1, const unsigned int particle2,
									float length,float minStiffness,float maxStiffness,i_math::vector3df &dir)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	ParticleData &pd = model.getParticles();

	if(TRUE)
	{
		const unsigned i1 = m_bodies[0];
		const unsigned i2 = m_bodies[1];

		i_math::vector3df &x1 = pd.getPosition(i1);
		i_math::vector3df &x2 = pd.getPosition(i2);

	}


	m_length = length;
	m_minStiffness=minStiffness;
	m_maxStiffness=maxStiffness;
	m_dir=dir;
	m_dir.normalize();

	return true;
}

bool PullConstraint::solveVelocityConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];

	return true;

}


bool PullConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];

	i_math::vector3df &x1 = pd.getPosition(i1);
	i_math::vector3df &x2 = pd.getPosition(i2);
	i_math::vector3df &x2_old = pd.getOldPosition(i2);
	i_math::vector3df &x2_last = pd.getLastPosition(i2);
	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);

	i_math::vector3df corr1, corr2;
	bool clamp2;
	const bool res = PositionBasedDynamics::solve_PullConstraint(
		x1, invMass1, x2, invMass2,
		m_length, m_minStiffness, m_maxStiffness,m_dir, corr1, corr2,clamp2);
	if (corr1.isAnyNan()||corr2.isAnyNan())
	{
		int v=0;
		v++;
	}

	if (res)
	{
		if (invMass1 != 0.0)
			x1 += corr1;
		if (invMass2 != 0.0)
		{
			x2 += corr2;
			if (clamp2)
			{
				x2_old=x2_last=x2;
			}
		}
	}
	return res;
}


//////////////////////////////////////////////////////////////////////////
// SimpleContactConstraint
//////////////////////////////////////////////////////////////////////////
bool SimpleContactConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,float minLength)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	ParticleData &pd = model.getParticles();

	m_minLength = minLength;

	return true;
}

bool SimpleContactConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];

	i_math::vector3df &x1 = pd.getPosition(i1);
	i_math::vector3df &x2 = pd.getPosition(i2);
	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);

	i_math::vector3df corr1, corr2;
	const bool res = PositionBasedDynamics::solve_SimpleContactConstraint(
		x1, invMass1, x2, invMass2,
		m_minLength, 2.0f, corr1, corr2);

	if (res)
	{
		if (invMass1 != 0.0)
			x1 += corr1;
		if (invMass2 != 0.0)
			x2 += corr2;
	}
	return res;
}


//////////////////////////////////////////////////////////////////////////
// BendConstraint
//////////////////////////////////////////////////////////////////////////

bool BendConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3,i_math::vector3df &dirDef,float toleranceAngle,float maxLength)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	m_bodies[2] = particle3;

	m_toleranceAngle=toleranceAngle*i_math::GRAD_PI2;
	m_maxLength=maxLength;
	m_dirDef=dirDef;
	return true;
}

bool BendConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];
	const unsigned i3 = m_bodies[2];

	i_math::vector3df &x1 = pd.getPosition(i1);
	i_math::vector3df &x2 = pd.getPosition(i2);
	i_math::vector3df &x3 = pd.getPosition(i3);

	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);
	const Real invMass3 = pd.getInvMass(i3);

	float toleranceAngle=m_toleranceAngle*i_math::clamp_f(x3.getDistanceFrom(x2)/m_maxLength,0.0f,1.0f);

	i_math::vector3df corr1, corr2, corr3;
	const bool res = PositionBasedDynamics::solve_BendConstraint(
		x1, invMass1, x2, invMass2, x3, invMass3,m_dirDef,
		toleranceAngle,
		1.f,
		corr1, corr2, corr3);

	if (corr3.getLength()>0.2f)
	{
		int v=0;
		v++;
	}

	if (res)
	{
		if (invMass1 != 0.0)
			x1 += corr1;
		if (invMass2 != 0.0)
			x2 += corr2;
		if (invMass3 != 0.0)
			x3 += corr3;
	}
	return res;
}


//////////////////////////////////////////////////////////////////////////
// DihedralConstraint
//////////////////////////////////////////////////////////////////////////

bool DihedralConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
										const unsigned int particle3, const unsigned int particle4)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	m_bodies[2] = particle3;
	m_bodies[3] = particle4;
	ParticleData &pd = model.getParticles();

	const i_math::vector3df &p0 = pd.getPosition0(particle1);
	const i_math::vector3df &p1 = pd.getPosition0(particle2);
	const i_math::vector3df &p2 = pd.getPosition0(particle3);
	const i_math::vector3df &p3 = pd.getPosition0(particle4);

	i_math::vector3df e = p3 - p2;
	Real  elen = e.getLength();
	if (elen < 1e-6)
		return false;

	Real invElen = static_cast<Real>(1.0) / elen;

	i_math::vector3df n1 = (p2 - p0).crossProduct(p3 - p0); //n1 /= n1.squaredNorm();
	i_math::vector3df n2 = (p3 - p1).crossProduct(p2 - p1); //n2 /= n2.squaredNorm();

	n1.normalize();
	n2.normalize();
	Real dot = n1.dotProduct(n2);

	if (dot < -1.0) dot = -1.0;
	if (dot > 1.0) dot = 1.0;

	m_restAngle = acos(dot);

	return true;
}

bool DihedralConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];
	const unsigned i3 = m_bodies[2];
	const unsigned i4 = m_bodies[3];

	i_math::vector3df &x1 = pd.getPosition(i1);
	i_math::vector3df &x2 = pd.getPosition(i2);
	i_math::vector3df &x3 = pd.getPosition(i3);
	i_math::vector3df &x4 = pd.getPosition(i4);

	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);
	const Real invMass3 = pd.getInvMass(i3);
	const Real invMass4 = pd.getInvMass(i4);

	i_math::vector3df corr1, corr2, corr3, corr4;
	const bool res = PositionBasedDynamics::solve_DihedralConstraint(
		x1, invMass1, x2, invMass2, x3, invMass3, x4, invMass4,
		m_restAngle,
		model.getValue<Real>(SimulationModel::CLOTH_BENDING_STIFFNESS),
		corr1, corr2, corr3, corr4);

	if (res)
	{
		if (invMass1 != 0.0)
			x1 += corr1;
		if (invMass2 != 0.0)
			x2 += corr2;
		if (invMass3 != 0.0)
			x3 += corr3;
		if (invMass4 != 0.0)
			x4 += corr4;
	}
	return res;
}


//////////////////////////////////////////////////////////////////////////
// IsometricBendingConstraint
//////////////////////////////////////////////////////////////////////////
bool IsometricBendingConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
												const unsigned int particle3, const unsigned int particle4)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	m_bodies[2] = particle3;
	m_bodies[3] = particle4;

	ParticleData &pd = model.getParticles();

	const i_math::vector3df &x1 = pd.getPosition0(particle1);
	const i_math::vector3df &x2 = pd.getPosition0(particle2);
	const i_math::vector3df &x3 = pd.getPosition0(particle3);
	const i_math::vector3df &x4 = pd.getPosition0(particle4);

	return PositionBasedDynamics::init_IsometricBendingConstraint(x1, x2, x3, x4, m_Q);
}

bool IsometricBendingConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];
	const unsigned i3 = m_bodies[2];
	const unsigned i4 = m_bodies[3];

	i_math::vector3df &x1 = pd.getPosition(i1);
	i_math::vector3df &x2 = pd.getPosition(i2);
	i_math::vector3df &x3 = pd.getPosition(i3);
	i_math::vector3df &x4 = pd.getPosition(i4);

	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);
	const Real invMass3 = pd.getInvMass(i3);
	const Real invMass4 = pd.getInvMass(i4);

	i_math::vector3df corr1, corr2, corr3, corr4;
	const bool res = PositionBasedDynamics::solve_IsometricBendingConstraint(
		x1, invMass1, x2, invMass2, x3, invMass3, x4, invMass4,
		m_Q,
		model.getValue<Real>(SimulationModel::CLOTH_BENDING_STIFFNESS),
		corr1, corr2, corr3, corr4);

	if (res)
	{
		if (invMass1 != 0.0)
			x1 += corr1;
		if (invMass2 != 0.0)
			x2 += corr2;
		if (invMass3 != 0.0)
			x3 += corr3;
		if (invMass4 != 0.0)
			x4 += corr4;
	}
	return res;
}


//////////////////////////////////////////////////////////////////////////
// VolumeConstraint
//////////////////////////////////////////////////////////////////////////

bool VolumeConstraint::initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
									  const unsigned int particle3, const unsigned int particle4)
{
	m_bodies[0] = particle1;
	m_bodies[1] = particle2;
	m_bodies[2] = particle3;
	m_bodies[3] = particle4;
	ParticleData &pd = model.getParticles();

	const i_math::vector3df &p0 = pd.getPosition0(particle1);
	const i_math::vector3df &p1 = pd.getPosition0(particle2);
	const i_math::vector3df &p2 = pd.getPosition0(particle3);
	const i_math::vector3df &p3 = pd.getPosition0(particle4);

	m_restVolume = fabsf(static_cast<Real>(1.0 / 6.0) * (p3 - p0).dotProduct((p2 - p0).crossProduct(p1 - p0)));

	return true;
}

bool VolumeConstraint::solvePositionConstraint(SimulationModel &model, const unsigned int iter)
{
	ParticleData &pd = model.getParticles();

	const unsigned i1 = m_bodies[0];
	const unsigned i2 = m_bodies[1];
	const unsigned i3 = m_bodies[2];
	const unsigned i4 = m_bodies[3];

	i_math::vector3df &x1 = pd.getPosition(i1);
	i_math::vector3df &x2 = pd.getPosition(i2);
	i_math::vector3df &x3 = pd.getPosition(i3);
	i_math::vector3df &x4 = pd.getPosition(i4);

	const Real invMass1 = pd.getInvMass(i1);
	const Real invMass2 = pd.getInvMass(i2);
	const Real invMass3 = pd.getInvMass(i3);
	const Real invMass4 = pd.getInvMass(i4);

	i_math::vector3df corr1, corr2, corr3, corr4;
	const bool res = PositionBasedDynamics::solve_VolumeConstraint(x1, invMass1,
		x2, invMass2,
		x3, invMass3,
		x4, invMass4,
		m_restVolume,
		model.getValue<Real>(SimulationModel::SOLID_STIFFNESS),
		model.getValue<Real>(SimulationModel::SOLID_STIFFNESS),
		corr1, corr2, corr3, corr4);

	if (res)
	{
		if (invMass1 != 0.0)
			x1 += corr1;
		if (invMass2 != 0.0)
			x2 += corr2;
		if (invMass3 != 0.0)
			x3 += corr3;
		if (invMass4 != 0.0)
			x4 += corr4;
	}
	return res;
}
