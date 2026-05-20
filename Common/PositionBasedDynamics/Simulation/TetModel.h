#ifndef __TETMODEL_H__
#define __TETMODEL_H__

#include "../Common/Common.h"
#include "../Utils/IndexedFaceMesh.h"
#include "../Utils/IndexedTetMesh.h"
#include "../Simulation/ParticleData.h"
#include <vector>

namespace PBD 
{	
	class TetModel 
	{
		public:
			TetModel();
			virtual ~TetModel();

			typedef Utilities::IndexedFaceMesh SurfaceMesh;
			typedef Utilities::IndexedTetMesh ParticleMesh;

			struct Attachment
			{
				unsigned int m_index;
				unsigned int m_triIndex;
				Real m_bary[3];
				Real m_dist;
				Real m_minError;
			};

			i_math::vector3df& getInitialX() { return m_initialX; }
			void setInitialX(const i_math::vector3df &val) { m_initialX = val; }
			Matrix3r& getInitialR() { return m_initialR; }
			void setInitialR(const Matrix3r &val) { m_initialR = val; }
			i_math::vector3df& getInitialScale() { return m_initialScale; }
			void setInitialScale(const i_math::vector3df &val) { m_initialScale = val; }

	protected:
			/** offset which must be added to get the correct index in the particles array */
			unsigned int m_indexOffset;
			/** Tet mesh of particles which represents the simulation model */
			ParticleMesh m_particleMesh;			
			SurfaceMesh m_surfaceMesh;
			VertexData m_visVertices;
			SurfaceMesh m_visMesh;
			Real m_restitutionCoeff;
			Real m_frictionCoeff;
			std::vector<Attachment> m_attachments;
			i_math::vector3df m_initialX;
			Matrix3r m_initialR;
			i_math::vector3df m_initialScale;
			
			void createSurfaceMesh();
 			void solveQuadraticForZero(const i_math::vector3df& F, const i_math::vector3df& Fu, 
 				const i_math::vector3df& Fv, const i_math::vector3df& Fuu,
 				const i_math::vector3df&Fuv, const i_math::vector3df& Fvv, 
 				Real& u, Real& v);
 			bool pointInTriangle(const i_math::vector3df& p0, const i_math::vector3df& p1, const i_math::vector3df& p2, 
 				const i_math::vector3df& p, i_math::vector3df& inter, i_math::vector3df &bary);


		public:
			void updateConstraints();

			SurfaceMesh &getSurfaceMesh();
			VertexData &getVisVertices();
			SurfaceMesh &getVisMesh();
			ParticleMesh &getParticleMesh();
			void cleanupModel();

			unsigned int getIndexOffset() const;

			void initMesh(const unsigned int nPoints, const unsigned int nTets, const unsigned int indexOffset, unsigned int* indices);
			void updateMeshNormals(const ParticleData &pd);

			/** Attach a visualization mesh to the surface of the body.
			 * Important: The vertex normals have to be updated before 
			 * calling this function by calling updateMeshNormals(). 
			 */
 			void attachVisMesh(const ParticleData &pd);

			/** Update the visualization mesh of the body.
			* Important: The vertex normals have to be updated before
			* calling this function by calling updateMeshNormals().
			*/
			void updateVisMesh(const ParticleData &pd);

			FORCE_INLINE Real getRestitutionCoeff() const
			{
				return m_restitutionCoeff;
			}

			FORCE_INLINE void setRestitutionCoeff(Real val)
			{
				m_restitutionCoeff = val;
			}

			FORCE_INLINE Real getFrictionCoeff() const
			{
				return m_frictionCoeff;
			}

			FORCE_INLINE void setFrictionCoeff(Real val)
			{
				m_frictionCoeff = val;
			}
			
	};
}

#endif