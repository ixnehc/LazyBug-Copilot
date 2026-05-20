#ifndef SPHKERNELS_H
#define SPHKERNELS_H

#define _USE_MATH_DEFINES
#include <math.h>
#include "../Common/Common.h"
#include <algorithm>

#define NO_DISTANCE_TEST

namespace PBD
{
	class CubicKernel
	{
	protected:
		static Real m_radius;
		static Real m_k;
		static Real m_l;
		static Real m_W_zero;
	public:
		static Real getRadius() { return m_radius; }
		static void setRadius(Real val)
		{
			m_radius = val;
			static const Real pi = static_cast<Real>(M_PI);

			const Real h3 = m_radius*m_radius*m_radius;
			m_k = static_cast<Real>(8.0) / (pi*h3);
			m_l = static_cast<Real>(48.0) / (pi*h3);
			m_W_zero = W(i_math::vector3df(0.0, 0.0, 0.0));
		}

	public:
		//static unsigned int counter;
		static Real W(const i_math::vector3df &r)
		{
			//counter++;
			Real res = 0.0;
			const Real rl = r.getLength();
			const Real q = rl/m_radius;
#ifndef NO_DISTANCE_TEST
			if (q <= 1.0)
#endif
			{
				if (q <= 0.5)
				{
					const Real q2 = q*q;
					const Real q3 = q2*q;
					res = m_k * (static_cast<Real>(6.0f)*q3- static_cast<Real>(6.0f)*q2+ static_cast<Real>(1.0f));
				}
				else
				{
					res = m_k * (static_cast<Real>(2.0f)*pow(static_cast<Real>(1.0f)-q,3.0f));
				}
			}
			return res;
		}

		static i_math::vector3df gradW(const i_math::vector3df &r)
		{
			i_math::vector3df res;
			const Real rl = r.getLength();
			const Real q = rl / m_radius;
#ifndef NO_DISTANCE_TEST
			if (q <= 1.0f)
#endif
			{
				if (rl > 1.0e-6)
				{
					const i_math::vector3df gradq = r * ((Real) 1.0 / (rl*m_radius));
					if (q <= 0.5)
					{
						res = m_l*q*((Real) 3.0*q - (Real) 2.0)*gradq;
					}
					else
					{
						const Real factor = static_cast<Real>(1.0) - q;
						res = m_l*(-factor*factor)*gradq;
					}
				}
			}
#ifndef NO_DISTANCE_TEST
 			else
 				res.zero();
#endif

			return res;
		}

		static Real W_zero()
		{
			return m_W_zero;
		}
	};
}

#endif
