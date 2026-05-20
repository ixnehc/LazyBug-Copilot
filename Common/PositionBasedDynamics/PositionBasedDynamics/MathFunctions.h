#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

#include "../Common/Common.h"

// ------------------------------------------------------------------------------------
namespace PBD
{
	class MathFunctions
	{
	private:
// 		static void jacobiRotate(Matrix3r &A,
// 			Matrix3r &R,
// 			int p,
// 			int q);

	public:
// 		static Real infNorm(const Matrix3r &A);
// 		static Real oneNorm(const Matrix3r &A);
// 
// 		static void eigenDecomposition(const Matrix3r &A,
// 			Matrix3r &eigenVecs,
// 			i_math::vector3df &eigenVals);
// 
// 		static void polarDecomposition(const Matrix3r &A,
// 			Matrix3r &R,
// 			Matrix3r &U,
// 			Matrix3r &D);
// 
// 		static void polarDecompositionStable(const Matrix3r &M,
// 			const Real tolerance,
// 			Matrix3r &R);
// 
// 		static void svdWithInversionHandling(const Matrix3r &A,
// 			i_math::vector3df &sigma,
// 			Matrix3r &U,
// 			Matrix3r &VT);

		static Real cotTheta(const i_math::vector3df &v, const i_math::vector3df &w);

		/** Computes the cross product matrix of a vector.
		 * @param  v		input vector
		 * @param  v_hat	resulting cross product matrix
		 */	
// 		static void crossProductMatrix(const i_math::vector3df &v, Matrix3r &v_hat);

		/** Implementation of the paper: \n
		 * Matthias Müller, Jan Bender, Nuttapong Chentanez and Miles Macklin, 
		 * "A Robust Method to Extract the Rotational Part of Deformations", 
		 * ACM SIGGRAPH Motion in Games, 2016
		 */
// 		static void extractRotation(const Matrix3r &A, i_math::quatf &q, const unsigned int maxIter);
	};
}

#endif