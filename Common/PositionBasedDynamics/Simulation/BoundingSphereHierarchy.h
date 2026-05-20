#ifndef __BOUNDINGSPHEREHIERARCHY_H__
#define __BOUNDINGSPHEREHIERARCHY_H__

#include "../Common/Common.h"
#include "BoundingSphere.h"
#include "kdTree.h"

namespace PBD
{
	class PointCloudBSH : public KDTree<BoundingSphere>
	{

	public:

		using super = KDTree<BoundingSphere>;

		PointCloudBSH();

		void init(const i_math::vector3df *vertices, const unsigned int numVertices);
		i_math::vector3df const& entity_position(unsigned int i) const final;
		void compute_hull(unsigned int b, unsigned int n, BoundingSphere& hull)
			const final;
		void compute_hull_approx(unsigned int b, unsigned int n, BoundingSphere& hull)
			const final;

	private:
		const i_math::vector3df *m_vertices;
		unsigned int m_numVertices;
	};


	class TetMeshBSH : public KDTree<BoundingSphere>
	{

	public:

		using super = KDTree<BoundingSphere>;

		TetMeshBSH();

		void init(const i_math::vector3df *vertices, const unsigned int numVertices, const unsigned int *indices, const unsigned int numTets, const Real tolerance);
		i_math::vector3df const& entity_position(unsigned int i) const final;
		void compute_hull(unsigned int b, unsigned int n, BoundingSphere& hull)
			const final;
		void compute_hull_approx(unsigned int b, unsigned int n, BoundingSphere& hull)
			const final;

	private:
		const i_math::vector3df *m_vertices;
		unsigned int m_numVertices;
		const unsigned int *m_indices;
		unsigned int m_numTets;
		Real m_tolerance;
		std::vector<i_math::vector3df> m_com;
	};

	class BVHTest
	{
	public:
		using TraversalCallback = std::function <void(unsigned int node_index1, unsigned int node_index2)>;

		static void traverse(PointCloudBSH const& b1, TetMeshBSH const& b2, TraversalCallback func);
		static void traverse(PointCloudBSH const& b1, const unsigned int node_index1, TetMeshBSH const& b2, const unsigned int node_index2, TraversalCallback func);
	};
}

#endif
