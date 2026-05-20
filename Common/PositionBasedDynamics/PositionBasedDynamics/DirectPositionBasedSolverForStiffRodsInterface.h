#ifndef DIRECT_POSITION_BASED_SOLVER_FOR_STIFF_RODS_INTERFACE
#define DIRECT_POSITION_BASED_SOLVER_FOR_STIFF_RODS_INTERFACE

#include "../Common/Common.h"

namespace PBD{
	// Implementation of "Direct Position-Based Solver for Stiff Rods" paper
	// (https://animation.rwth-aachen.de/publication/0557/)
	//
	//	Implemented by:
	//
	//	Crispin Deul
	//	Graduate School CE
	//	Technische Universität Darmstadt
	//
	//  deul[at] gsc.tu-darmstadt.de
	//


// 	class RodSegment
// 	{
// 	public:
// 		virtual bool isDynamic() = 0;
// 		virtual Real Mass() = 0;
// 		virtual const i_math::vector3df & InertiaTensor() = 0;
// 		virtual const i_math::vector3df & Position() = 0;
// 		virtual const i_math::quatf & Rotation() = 0;
// 	};
// 
// 	class RodConstraint
// 	{
// 	public:
// 		using Vector6r = Eigen::Matrix<Real, 6, 1, Eigen::DontAlign>;
// 		virtual unsigned int segmentIndex(unsigned int i) = 0;
// 		virtual Eigen::Matrix<Real, 3, 4, Eigen::DontAlign> & getConstraintInfo() = 0;
// 		virtual Real getAverageSegmentLength() = 0;
// 		virtual i_math::vector3df &getRestDarbouxVector() = 0;
// 		virtual i_math::vector3df &getStiffnessCoefficientK() = 0;
// 		virtual i_math::vector3df & getStretchCompliance() = 0;
// 		virtual i_math::vector3df & getBendingAndTorsionCompliance() = 0;
// 	};
}
#endif
