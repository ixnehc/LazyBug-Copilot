#ifndef __PARTICLEDATA_H__
#define __PARTICLEDATA_H__

#include <vector>
#include <deque>
#include "../Common/Common.h"


namespace PBD
{
	/** This class encapsulates the state of all vertices.
	* All parameters are stored in individual arrays.
	*/
	class VertexData
	{
	private:
		std::vector<i_math::vector3df> m_x;

	public:
		FORCE_INLINE VertexData(void) :
			m_x()
		{
		}

		FORCE_INLINE ~VertexData(void)
		{
			m_x.clear();
		}

		FORCE_INLINE void addVertex(const i_math::vector3df &vertex)
		{
			m_x.push_back(vertex);
		}

		FORCE_INLINE i_math::vector3df &getPosition(const unsigned int i)
		{
			return m_x[i];
		}

		FORCE_INLINE const i_math::vector3df &getPosition(const unsigned int i) const
		{
			return m_x[i];
		}

		FORCE_INLINE void setPosition(const unsigned int i, const i_math::vector3df &pos)
		{
			m_x[i] = pos;
		}

		/** Resize the array containing the particle data.
		*/
		FORCE_INLINE void resize(const unsigned int newSize)
		{
			m_x.resize(newSize);
		}

		/** Reserve the array containing the particle data.
		*/
		FORCE_INLINE void reserve(const unsigned int newSize)
		{
			m_x.reserve(newSize);
		}

		/** Release the array containing the particle data.
		*/
		FORCE_INLINE void release()
		{
			m_x.clear();
		}

		/** Release the array containing the particle data.
		*/
		FORCE_INLINE unsigned int size() const
		{
			return (unsigned int)m_x.size();
		}

		FORCE_INLINE const std::vector<i_math::vector3df>* getVertices()
		{
			return &m_x;
		}
	};

	/** This class encapsulates the state of all particles of a particle model.
	 * All parameters are stored in individual arrays.
	 */
	class ParticleData
	{
		private:
			std::deque<DWORD> m_freelist;

			// Mass
			// If the mass is zero, the particle is static
			std::vector<Real> m_masses;
			std::vector<Real> m_invMasses;

			// Dynamic state
			std::vector<i_math::vector3df> m_x0;
			std::vector<i_math::vector3df> m_x;
			std::vector<i_math::vector3df> m_v;
			std::vector<i_math::vector3df> m_a;
			std::vector<i_math::vector3df> m_oldX;
			std::vector<i_math::vector3df> m_lastX;

		public:
			FORCE_INLINE ParticleData(void)	:
				  m_masses(),
				  m_invMasses(),
				  m_x0(),
				  m_x(),
				  m_v(),
				  m_a(),
				  m_oldX(),
				  m_lastX()
			{
			}

			FORCE_INLINE ~ParticleData(void) 
			{
				m_masses.clear();
				m_invMasses.clear();
				m_x0.clear();
				m_x.clear();
				m_v.clear();
				m_a.clear();
				m_oldX.clear();
				m_lastX.clear();
			}

			void removeVertex(int particle)
			{
				if (((DWORD)particle)<size())
				{
					m_freelist.push_back(particle);
				}
			}

			FORCE_INLINE int addVertex(const i_math::vector3df &vertex)
			{
				if (m_freelist.size()>0)
				{
					int iParticle=m_freelist[0];
					m_freelist.pop_front();

					m_x0[iParticle]=vertex;
					m_x[iParticle]=vertex;
					m_oldX[iParticle]=vertex;
					m_lastX[iParticle]=vertex;
					m_masses[iParticle]=1.0;
					m_invMasses[iParticle]=1.0;
					m_v[iParticle]=i_math::vector3df(0.0, 0.0, 0.0);
					m_a[iParticle]=i_math::vector3df(0.0, 0.0, 0.0);

					return iParticle;
				}

				int iParticle=m_x0.size();

				m_x0.push_back(vertex);
				m_x.push_back(vertex);
				m_oldX.push_back(vertex);
				m_lastX.push_back(vertex);
				m_masses.push_back(1.0);
				m_invMasses.push_back(1.0);
				m_v.push_back(i_math::vector3df(0.0, 0.0, 0.0));
				m_a.push_back(i_math::vector3df(0.0, 0.0, 0.0));

				return iParticle;
			}

			FORCE_INLINE i_math::vector3df &getPosition(const unsigned int i)
			{
				return m_x[i];
			}

			FORCE_INLINE const i_math::vector3df &getPosition(const unsigned int i) const 
			{
				return m_x[i];
			}

			FORCE_INLINE void setPosition(const unsigned int i, const i_math::vector3df &pos)
			{
				m_x[i] = pos;
			}

			FORCE_INLINE i_math::vector3df &getPosition0(const unsigned int i)
			{
				return m_x0[i];
			}

			FORCE_INLINE const i_math::vector3df &getPosition0(const unsigned int i) const
			{
				return m_x0[i];
			}

			FORCE_INLINE void setPosition0(const unsigned int i, const i_math::vector3df &pos)
			{
				m_x0[i] = pos;
			}

			FORCE_INLINE i_math::vector3df &getLastPosition(const unsigned int i)
			{
				return m_lastX[i];
			}

			FORCE_INLINE const i_math::vector3df &getLastPosition(const unsigned int i) const
			{
				return m_lastX[i];
			}

			FORCE_INLINE void setLastPosition(const unsigned int i, const i_math::vector3df &pos)
			{
				m_lastX[i] = pos;
			}

			FORCE_INLINE i_math::vector3df &getOldPosition(const unsigned int i)
			{
				return m_oldX[i];
			}

			FORCE_INLINE const i_math::vector3df &getOldPosition(const unsigned int i) const
			{
				return m_oldX[i];
			}

			FORCE_INLINE void setOldPosition(const unsigned int i, const i_math::vector3df &pos)
			{
				m_oldX[i] = pos;
			}
			
			FORCE_INLINE i_math::vector3df &getVelocity(const unsigned int i)
			{
				return m_v[i];
			}

			FORCE_INLINE const i_math::vector3df &getVelocity(const unsigned int i) const 
			{
				return m_v[i];
			}

			FORCE_INLINE void setVelocity(const unsigned int i, const i_math::vector3df &vel)
			{
				m_v[i] = vel;
			}

			FORCE_INLINE i_math::vector3df &getAcceleration(const unsigned int i)
			{
				return m_a[i];
			}

			FORCE_INLINE const i_math::vector3df &getAcceleration(const unsigned int i) const 
			{
				return m_a[i];
			}

			FORCE_INLINE void setAcceleration(const unsigned int i, const i_math::vector3df &accel)
			{
				m_a[i] = accel;
			}

			FORCE_INLINE const Real getMass(const unsigned int i) const
			{
				return m_masses[i];
			}

			FORCE_INLINE Real& getMass(const unsigned int i)
			{
				return m_masses[i];
			}

			FORCE_INLINE void setMass(const unsigned int i, const Real mass)
			{
				m_masses[i] = mass;
				if (mass != 0.0)
					m_invMasses[i] = static_cast<Real>(1.0) / mass;
				else
					m_invMasses[i] = 0.0;
			}

			FORCE_INLINE const Real getInvMass(const unsigned int i) const
			{
				return m_invMasses[i];
			}

			FORCE_INLINE const unsigned int getNumberOfParticles() const
			{
				return (unsigned int) m_x.size();
			}

			/** Resize the array containing the particle data.
			 */
			FORCE_INLINE void resize(const unsigned int newSize)
			{
				m_masses.resize(newSize);
				m_invMasses.resize(newSize);
				m_x0.resize(newSize);
				m_x.resize(newSize);
				m_v.resize(newSize);
				m_a.resize(newSize);
				m_oldX.resize(newSize);
				m_lastX.resize(newSize);
			}

			/** Reserve the array containing the particle data.
			 */
			FORCE_INLINE void reserve(const unsigned int newSize)
			{
				m_masses.reserve(newSize);
				m_invMasses.reserve(newSize);
				m_x0.reserve(newSize);
				m_x.reserve(newSize);
				m_v.reserve(newSize);
				m_a.reserve(newSize);
				m_oldX.reserve(newSize);
				m_lastX.reserve(newSize);
			}

			/** Release the array containing the particle data.
			 */
			FORCE_INLINE void release()
			{
				m_masses.clear();
				m_invMasses.clear();
				m_x0.clear();
				m_x.clear();
				m_v.clear();
				m_a.clear();
				m_oldX.clear();
				m_lastX.clear();
			}

			/** Release the array containing the particle data.
			 */
			FORCE_INLINE unsigned int size() const 
			{
				return (unsigned int) m_x.size();
			}
	};

	/** This class encapsulates the state of all orientations of a quaternion model.
	* All parameters are stored in individual arrays.
	*/
	class OrientationData
	{
	private:
		// Mass
		// If the mass is zero, the particle is static
		std::vector<Real> m_masses;
		std::vector<Real> m_invMasses;

		// Dynamic state
		std::vector<i_math::quatf> m_q0;
		std::vector<i_math::quatf> m_q;
		std::vector<i_math::vector3df> m_omega;
		std::vector<i_math::vector3df> m_alpha;
		std::vector<i_math::quatf> m_oldQ;
		std::vector<i_math::quatf> m_lastQ;

	public:
		FORCE_INLINE OrientationData(void) :
			m_masses(),
			m_invMasses(),
			m_q0(),
			m_q(),
			m_omega(),
			m_alpha(),
			m_oldQ(),
			m_lastQ()
		{
		}

		FORCE_INLINE ~OrientationData(void)
		{
			m_masses.clear();
			m_invMasses.clear();
			m_q0.clear();
			m_q.clear();
			m_omega.clear();
			m_alpha.clear();
			m_oldQ.clear();
			m_lastQ.clear();
		}

		FORCE_INLINE void addQuaternion(const i_math::quatf &vertex)
		{
			m_q0.push_back(vertex);
			m_q.push_back(vertex);
			m_oldQ.push_back(vertex);
			m_lastQ.push_back(vertex);
			m_masses.push_back(1.0);
			m_invMasses.push_back(1.0);
			m_omega.push_back(i_math::vector3df(0.0, 0.0, 0.0));
			m_alpha.push_back(i_math::vector3df(0.0, 0.0, 0.0));
		}

		FORCE_INLINE i_math::quatf &getQuaternion(const unsigned int i)
		{
			return m_q[i];
		}

		FORCE_INLINE const i_math::quatf &getQuaternion(const unsigned int i) const
		{
			return m_q[i];
		}

		FORCE_INLINE void setQuaternion(const unsigned int i, const i_math::quatf &pos)
		{
			m_q[i] = pos;
		}

		FORCE_INLINE i_math::quatf &getQuaternion0(const unsigned int i)
		{
			return m_q0[i];
		}

		FORCE_INLINE const i_math::quatf &getQuaternion0(const unsigned int i) const
		{
			return m_q0[i];
		}

		FORCE_INLINE void setQuaternion0(const unsigned int i, const i_math::quatf &pos)
		{
			m_q0[i] = pos;
		}

		FORCE_INLINE i_math::quatf &getLastQuaternion(const unsigned int i)
		{
			return m_lastQ[i];
		}

		FORCE_INLINE const i_math::quatf &getLastQuaternion(const unsigned int i) const
		{
			return m_lastQ[i];
		}

		FORCE_INLINE void setLastQuaternion(const unsigned int i, const i_math::quatf &pos)
		{
			m_lastQ[i] = pos;
		}

		FORCE_INLINE i_math::quatf &getOldQuaternion(const unsigned int i)
		{
			return m_oldQ[i];
		}

		FORCE_INLINE const i_math::quatf &getOldQuaternion(const unsigned int i) const
		{
			return m_oldQ[i];
		}

		FORCE_INLINE void setOldQuaternion(const unsigned int i, const i_math::quatf &pos)
		{
			m_oldQ[i] = pos;
		}

		FORCE_INLINE i_math::vector3df &getVelocity(const unsigned int i)
		{
			return m_omega[i];
		}

		FORCE_INLINE const i_math::vector3df &getVelocity(const unsigned int i) const
		{
			return m_omega[i];
		}

		FORCE_INLINE void setVelocity(const unsigned int i, const i_math::vector3df &vel)
		{
			m_omega[i] = vel;
		}

		FORCE_INLINE i_math::vector3df &getAcceleration(const unsigned int i)
		{
			return m_alpha[i];
		}

		FORCE_INLINE const i_math::vector3df &getAcceleration(const unsigned int i) const
		{
			return m_alpha[i];
		}

		FORCE_INLINE void setAcceleration(const unsigned int i, const i_math::vector3df &accel)
		{
			m_alpha[i] = accel;
		}

		FORCE_INLINE const Real getMass(const unsigned int i) const
		{
			return m_masses[i];
		}

		FORCE_INLINE Real& getMass(const unsigned int i)
		{
			return m_masses[i];
		}

		FORCE_INLINE void setMass(const unsigned int i, const Real mass)
		{
			m_masses[i] = mass;
			if (mass != 0.0)
				m_invMasses[i] = static_cast<Real>(1.0) / mass;
			else
				m_invMasses[i] = 0.0;
		}

		FORCE_INLINE const Real getInvMass(const unsigned int i) const
		{
			return m_invMasses[i];
		}

		FORCE_INLINE const unsigned int getNumberOfQuaternions() const
		{
			return (unsigned int)m_q.size();
		}

		/** Resize the array containing the particle data.
		*/
		FORCE_INLINE void resize(const unsigned int newSize)
		{
			m_masses.resize(newSize);
			m_invMasses.resize(newSize);
			m_q0.resize(newSize);
			m_q.resize(newSize);
			m_omega.resize(newSize);
			m_alpha.resize(newSize);
			m_oldQ.resize(newSize);
			m_lastQ.resize(newSize);
		}

		/** Reserve the array containing the particle data.
		*/
		FORCE_INLINE void reserve(const unsigned int newSize)
		{
			m_masses.reserve(newSize);
			m_invMasses.reserve(newSize);
			m_q0.reserve(newSize);
			m_q.reserve(newSize);
			m_omega.reserve(newSize);
			m_alpha.reserve(newSize);
			m_oldQ.reserve(newSize);
			m_lastQ.reserve(newSize);
		}

		/** Release the array containing the particle data.
		*/
		FORCE_INLINE void release()
		{
			m_masses.clear();
			m_invMasses.clear();
			m_q0.clear();
			m_q.clear();
			m_omega.clear();
			m_alpha.clear();
			m_oldQ.clear();
			m_lastQ.clear();
		}

		/** Release the array containing the particle data.
		*/
		FORCE_INLINE unsigned int size() const
		{
			return (unsigned int)m_q.size();
		}
	};
}

#endif
