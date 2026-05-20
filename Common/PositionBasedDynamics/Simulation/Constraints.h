#ifndef _CONSTRAINTS_H
#define _CONSTRAINTS_H

#include "../Common/Common.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include <vector>
#include <list>
#include <memory>
//#include "../PositionBasedDynamics/DirectPositionBasedSolverForStiffRodsInterface.h"

namespace PBD
{
	class SimulationModel;

	class Constraint
	{
	public: 
		unsigned int m_numberOfBodies;
		/** indices of the linked bodies */
		unsigned int *m_bodies;

		BOOL m_enabled;

		Constraint(const unsigned int numberOfBodies) 
		{
			m_numberOfBodies = numberOfBodies; 
			m_bodies = new unsigned int[numberOfBodies]; 
			m_enabled=TRUE;
		}

		virtual ~Constraint() { delete[] m_bodies; };
		virtual int &getTypeId() const = 0;

		bool isEnabled()		{			return m_enabled!=0;		}
		void setEnabled(bool enabled)		{			m_enabled=enabled;		}

		virtual bool initConstraintBeforeProjection(SimulationModel &model) { return true; };
		virtual bool updateConstraint(SimulationModel &model) { return true; };
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter) { return true; };
		virtual bool solveVelocityConstraint(SimulationModel &model, const unsigned int iter) { return true; };
	};

// 	class BallJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_jointInfo;
// 
// 		BallJoint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};	
// 
// 	class BallOnLineJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 3, 10, Eigen::DontAlign> m_jointInfo;
// 
// 		BallOnLineJoint() : Constraint(2) {} 
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &dir);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
//  
// 	class HingeJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 4, 7, Eigen::DontAlign> m_jointInfo;
// 
// 		HingeJoint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
//  
// 	class UniversalJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 3, 8, Eigen::DontAlign> m_jointInfo;
// 
// 		UniversalJoint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis1, const i_math::vector3df &axis2);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class SliderJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;
// 
// 		SliderJoint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class MotorJoint: public Constraint
// 	{
// 	public:
// 		Real m_target;
// 		std::vector<Real> m_targetSequence;
// 		MotorJoint() : Constraint(2) { m_target = 0.0; }
// 
// 		virtual Real getTarget() const { return m_target; }
// 		virtual void setTarget(const Real val) { m_target = val; }
// 
// 		virtual std::vector<Real> &getTargetSequence() { return m_targetSequence; }
// 		virtual void setTargetSequence(const std::vector<Real> &val) { m_targetSequence = val; }
// 
// 		bool getRepeatSequence() const { return m_repeatSequence; }
// 		void setRepeatSequence(bool val) { m_repeatSequence = val; }
// 
// 	private:
// 		bool m_repeatSequence;
// 	};
// 
// 	class TargetPositionMotorSliderJoint : public MotorJoint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;
// 
// 		TargetPositionMotorSliderJoint() : MotorJoint() {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class TargetVelocityMotorSliderJoint : public MotorJoint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;
// 
// 		TargetVelocityMotorSliderJoint() : MotorJoint() {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 		virtual bool solveVelocityConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class TargetAngleMotorHingeJoint : public MotorJoint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 4, 8, Eigen::DontAlign> m_jointInfo;
// 		TargetAngleMotorHingeJoint() : MotorJoint() {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual void setTarget(const Real val) 
// 		{ 
// 			const Real pi = (Real)M_PI;
// 			m_target = std::max(val, -pi);
// 			m_target = std::min(m_target, pi);
// 		}
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	private:
// 		std::vector<Real> m_targetSequence;
// 	};
// 
// 	class TargetVelocityMotorHingeJoint : public MotorJoint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 4, 8, Eigen::DontAlign> m_jointInfo;
// 		TargetVelocityMotorHingeJoint() : MotorJoint() {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos, const i_math::vector3df &axis);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 		virtual bool solveVelocityConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class DamperJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Real m_stiffness;
// 		Eigen::Matrix<Real, 4, 6, Eigen::DontAlign> m_jointInfo;
// 		Real m_lambda;
// 
// 		DamperJoint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &axis, const Real stiffness);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
//  
// 	class RigidBodyParticleBallJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 3, 2, Eigen::DontAlign> m_jointInfo;
// 
// 		RigidBodyParticleBallJoint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex, const unsigned int particleIndex);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class RigidBodySpring : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_jointInfo;
// 		Real m_restLength;
// 		Real m_stiffness;
// 		Real m_lambda;
// 
// 		RigidBodySpring() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos1, const i_math::vector3df &pos2, const Real stiffness);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class DistanceJoint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_jointInfo;
// 		Real m_restLength;
// 
// 		DistanceJoint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, const i_math::vector3df &pos1, const i_math::vector3df &pos2);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};
 
	class DistanceConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_restLength;
		Real m_stiffness;

		DistanceConstraint() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,Real length,Real stiffness);
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
	};

	class SimpleContactConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_minLength;

		SimpleContactConstraint() : Constraint(2) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,float minLength);
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
	};

	class PullConstraint:public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_length;
		Real m_minStiffness;
		Real m_maxStiffness;

		i_math::vector3df m_dir;

		PullConstraint() : Constraint(2) 		{		}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
			float length,float minStiffness,float maxStiffness,i_math::vector3df &dir);
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
		virtual bool solveVelocityConstraint(SimulationModel &model, const unsigned int iter);

	};

	class BendConstraint: public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_toleranceAngle;
		Real m_maxLength;
		i_math::vector3df m_dirDef;

		BendConstraint() : Constraint(3) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2, const unsigned int particle3,
			i_math::vector3df &dirDef,float toleranceAngle,float maxLength);
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);

		void SetToleranceAngle(Real angle)		{			m_toleranceAngle=angle*i_math::GRAD_PI2;		}

	};

	class DihedralConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_restAngle;

		DihedralConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4);
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
	};
	
	class IsometricBendingConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		i_math::matrix44f m_Q;

		IsometricBendingConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
									const unsigned int particle3, const unsigned int particle4);
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
	};

// 	class FEMTriangleConstraint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Real m_area;
// 		Matrix2r m_invRestMat;
// 
// 		FEMTriangleConstraint() : Constraint(3) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 			const unsigned int particle3);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

// 	class StrainTriangleConstraint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Matrix2r m_invRestMat;
// 
// 		StrainTriangleConstraint() : Constraint(3) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 			const unsigned int particle3);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

	class VolumeConstraint : public Constraint
	{
	public:
		static int TYPE_ID;
		Real m_restVolume;

		VolumeConstraint() : Constraint(4) {}
		virtual int &getTypeId() const { return TYPE_ID; }

		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
								const unsigned int particle3, const unsigned int particle4);
		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
	};

// 	class FEMTetConstraint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Real m_volume;
// 		Matrix3r m_invRestMat;
// 
// 		FEMTetConstraint() : Constraint(4) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 									const unsigned int particle3, const unsigned int particle4);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

// 	class StrainTetConstraint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Matrix3r m_invRestMat;
// 
// 		StrainTetConstraint() : Constraint(4) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2,
// 			const unsigned int particle3, const unsigned int particle4);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

// 	class ShapeMatchingConstraint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		i_math::vector3df m_restCm;
// 		Matrix3r m_invRestMat;
// 		Real *m_w;
// 		i_math::vector3df *m_x0;
// 		i_math::vector3df *m_x;
// 		i_math::vector3df *m_corr;
// 		unsigned int *m_numClusters;
// 
// 		ShapeMatchingConstraint(const unsigned int numberOfParticles) : Constraint(numberOfParticles)
// 		{
// 			m_x = new i_math::vector3df[numberOfParticles];
// 			m_x0 = new i_math::vector3df[numberOfParticles];
// 			m_corr = new i_math::vector3df[numberOfParticles];
// 			m_w = new Real[numberOfParticles];
// 			m_numClusters = new unsigned int[numberOfParticles];
// 		}
// 		virtual ~ShapeMatchingConstraint() 
// 		{ 
// 			delete[] m_x; 
// 			delete[] m_x0;
// 			delete[] m_corr;
// 			delete[] m_w;
// 			delete[] m_numClusters;
// 		}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual bool initConstraint(SimulationModel &model, const unsigned int particleIndices[], const unsigned int numClusters[]);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

// 	class RigidBodyContactConstraint 
// 	{
// 	public:
// 		static int TYPE_ID;
// 		/** indices of the linked bodies */
// 		unsigned int m_bodies[2];
// 		Real m_stiffness; 
// 		Real m_frictionCoeff;
// 		Real m_sum_impulses;
// 		Eigen::Matrix<Real, 3, 5, Eigen::DontAlign> m_constraintInfo;
// 
// 		RigidBodyContactConstraint() {}
// 		~RigidBodyContactConstraint() {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int rbIndex1, const unsigned int rbIndex2, 
// 			const i_math::vector3df &cp1, const i_math::vector3df &cp2, 
// 			const i_math::vector3df &normal, const Real dist, 
// 			const Real restitutionCoeff, const Real stiffness, const Real frictionCoeff);
// 		virtual bool solveVelocityConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class ParticleRigidBodyContactConstraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		/** indices of the linked bodies */
// 		unsigned int m_bodies[2];
// 		Real m_stiffness;
// 		Real m_frictionCoeff;
// 		Real m_sum_impulses;
// 		Eigen::Matrix<Real, 3, 5, Eigen::DontAlign> m_constraintInfo;
// 
// 		ParticleRigidBodyContactConstraint() {}
// 		~ParticleRigidBodyContactConstraint() {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int particleIndex, const unsigned int rbIndex,
// 			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
// 			const i_math::vector3df &normal, const Real dist,
// 			const Real restitutionCoeff, const Real stiffness, const Real frictionCoeff);
// 		virtual bool solveVelocityConstraint(SimulationModel &model, const unsigned int iter);
// 	};
// 
// 	class ParticleTetContactConstraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		/** indices of the linked bodies */
// 		unsigned int m_bodies[2];
// 		unsigned int m_solidIndex;
// 		unsigned int m_tetIndex; 
// 		i_math::vector3df m_bary;
// 		Real m_lambda;
// 		Real m_frictionCoeff;
// 		Eigen::Matrix<Real, 3, 3, Eigen::DontAlign> m_constraintInfo;
// 		Real m_invMasses[4];
// 		i_math::vector3df m_x[4];
// 		i_math::vector3df m_v[4];
// 
// 		ParticleTetContactConstraint() { }
// 		~ParticleTetContactConstraint() {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int particleIndex, const unsigned int solidIndex,
// 			const unsigned int tetindex, const i_math::vector3df &bary,
// 			const i_math::vector3df &cp1, const i_math::vector3df &cp2,
// 			const i_math::vector3df &normal, const Real dist,
// 			const Real frictionCoeff);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 		virtual bool solveVelocityConstraint(SimulationModel &model, const unsigned int iter);
// 	};

// 	class StretchShearConstraint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		Real m_restLength;
// 
// 		StretchShearConstraint() : Constraint(3) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual bool initConstraint(SimulationModel &model, const unsigned int particle1, const unsigned int particle2, const unsigned int quaternion1);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

// 	class BendTwistConstraint : public Constraint
// 	{
// 	public:
// 		static int TYPE_ID;
// 		i_math::quatf m_restDarbouxVector;
// 
// 		BendTwistConstraint() : Constraint(2) {}
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		virtual bool initConstraint(SimulationModel &model, const unsigned int quaternion1, const unsigned int quaternion2);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

// 	class StretchBendingTwistingConstraint : public Constraint
// 	{
// 		using Matrix6r = Eigen::Matrix<Real, 6, 6, Eigen::DontAlign>;
// 		using Vector6r = Eigen::Matrix<Real, 6, 1, Eigen::DontAlign>;
// 	public:
// 		static int TYPE_ID;
// 		Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_constraintInfo;
// 
// 		Real m_averageRadius;
// 		Real m_averageSegmentLength;
// 		i_math::vector3df m_restDarbouxVector;
// 		i_math::vector3df m_stiffnessCoefficientK;
// 		i_math::vector3df m_stretchCompliance;
// 		i_math::vector3df m_bendingAndTorsionCompliance;
// 		Vector6r m_lambdaSum;		
// 
// 		StretchBendingTwistingConstraint() : Constraint(2){}
// 
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model, const unsigned int segmentIndex1, const unsigned int segmentIndex2, const i_math::vector3df &pos,
// 			const Real averageRadius, const Real averageSegmentLength, Real youngsModulus, Real torsionModulus);
// 		virtual bool initConstraintBeforeProjection(SimulationModel &model);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 	};

	struct Node;
	struct Interval;
	class SimulationModel;
// 	using Vector6r = Eigen::Matrix<Real, 6, 1, Eigen::DontAlign>;

// 	class DirectPositionBasedSolverForStiffRodsConstraint : public Constraint
// 	{
// 		class RodSegmentImpl : public RodSegment
// 		{			
// 		public:
// 			RodSegmentImpl(SimulationModel &model, unsigned int idx) :
// 				m_model(model), m_segmentIdx(idx) {};
// 
// 			virtual bool isDynamic();
// 			virtual Real Mass();
// 			virtual const i_math::vector3df & InertiaTensor();
// 			virtual const i_math::vector3df & Position();
// 			virtual const i_math::quatf & Rotation();
// 
// 			SimulationModel &m_model;
// 			unsigned int m_segmentIdx;
// 		};
// 
// 		class RodConstraintImpl : public RodConstraint
// 		{
// 		public:
// 			std::vector<unsigned int> m_segments;
// 			Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> m_constraintInfo;
// 
// 			Real m_averageRadius;
// 			Real m_averageSegmentLength;
// 			i_math::vector3df m_restDarbouxVector;
// 			i_math::vector3df m_stiffnessCoefficientK;
// 			i_math::vector3df m_stretchCompliance;
// 			i_math::vector3df m_bendingAndTorsionCompliance;
// 			
// 			virtual unsigned int segmentIndex(unsigned int i){
// 				if (i < static_cast<unsigned int>(m_segments.size()))
// 					return m_segments[i];
// 				return 0u;
// 			};
// 
// 			virtual Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> & getConstraintInfo(){ return m_constraintInfo; }
// 			virtual Real getAverageSegmentLength(){ return m_averageSegmentLength; }
// 			virtual i_math::vector3df &getRestDarbouxVector(){ return m_restDarbouxVector; }
// 			virtual i_math::vector3df &getStiffnessCoefficientK() { return m_stiffnessCoefficientK; };
// 			virtual i_math::vector3df & getStretchCompliance(){ return m_stretchCompliance; }
// 			virtual i_math::vector3df & getBendingAndTorsionCompliance(){ return m_bendingAndTorsionCompliance; }
// 		};
// 
// 	public:
// 		static int TYPE_ID;
// 
// 		DirectPositionBasedSolverForStiffRodsConstraint() :  Constraint(2),
// 			root(NULL), numberOfIntervals(0), intervals(NULL), forward(NULL), backward(NULL){}
// 		~DirectPositionBasedSolverForStiffRodsConstraint();
// 
// 		virtual int &getTypeId() const { return TYPE_ID; }
// 
// 		bool initConstraint(SimulationModel &model,
// 			const std::vector<std::pair<unsigned int, unsigned int>> & constraintSegmentIndices,
// 			const std::vector<i_math::vector3df> &constraintPositions,
// 			const std::vector<Real> &averageRadii,
// 			const std::vector<Real> &averageSegmentLengths,
// 			const std::vector<Real> &youngsModuli,
// 			const std::vector<Real> &torsionModuli);
// 
// 		virtual bool initConstraintBeforeProjection(SimulationModel &model);
// 		virtual bool updateConstraint(SimulationModel &model);
// 		virtual bool solvePositionConstraint(SimulationModel &model, const unsigned int iter);
// 
// 	protected:
// 		
// 		/** root node */
// 		Node *root;
// 		/** intervals of constraints */
// 		Interval *intervals;
// 		/** number of intervals */
// 		int numberOfIntervals;		
// 		/** list to process nodes with increasing row index in the system matrix H (from the leaves to the root) */
// 		std::list <Node*> *forward;
// 		/** list to process nodes starting with the highest row index to row index zero in the matrix H (from the root to the leaves) */
// 		std::list <Node*> *backward;
// 
// 		std::vector<RodConstraintImpl> m_Constraints;
// 		std::vector<RodConstraint*> m_rodConstraints;
// 
// 		std::vector<RodSegmentImpl> m_Segments;
// 		std::vector<RodSegment*> m_rodSegments;
// 
// 		std::vector<Vector6r> m_rightHandSide;
// 		std::vector<Vector6r> m_lambdaSums;
// 		std::vector<std::vector<Matrix3r>> m_bendingAndTorsionJacobians;
// 		std::vector<i_math::vector3df> m_corr_x;
// 		std::vector<i_math::quatf> m_corr_q;
// 
// 		void deleteNodes();
// 	};
}

#endif
