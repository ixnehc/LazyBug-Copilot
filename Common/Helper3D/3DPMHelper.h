#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.h>
#include <dxerr9.h>

#include <vector>
#include <set>

#include "..\D3DBase\D3DBaseType.h"

/** The way to derive the quota of vertices which are reduced at each LOD. */
enum VertexReductionQuota
{
	/// A set number of vertices are removed at each reduction
	VRQ_CONSTANT,
	/// A proportion of the remaining number of vertices are removed at each reduction
	VRQ_PROPORTIONAL
};

typedef std::vector<std::vector<WORD> > LODFaceList;




//Progressive Mesh Helper,copied mainly from ogre
class C3DPMHelper
{
public:
	/** This class reduces the complexity of the geometry it is given.
	This class is dedicated to reducing the number of triangles in a given mesh
	taking into account seams in both geometry and texture co-ordinates and meshes 
	which have multiple frames.
	@par
	The primary use for this is generating LOD versions of Mesh objects, but it can be
	used by any geometry provider. The only limitation at the moment is that the 
	provider uses a common vertex buffer for all LODs and one index buffer per LOD.
	Therefore at the moment this class can only handle indexed geometry.
	@par
	NB the interface of this class will certainly change when compiled vertex buffers are
	supported.
	*/


		/** Constructor, takes the geometry data and index buffer. 
		*/
	C3DPMHelper();

	void SetVertexData(void *pVertexData,FVFEx fvf,DWORD nVertice,WORD *pVerticeIndice,DWORD nIndice);

	/** Adds an extra vertex position buffer. 
	@remarks
	As well as the main vertex buffer, the client of this class may add extra versions
	of the vertex buffer which will also be taken into account when the cost of 
	simplifying the mesh is taken into account. This is because the cost of
	simplifying an animated mesh cannot be calculated from just the reference position,
	multiple positions needs to be assessed in order to find the best simplification option.
	*/
	void AddAdditionalVertexData(void *pVertexData,FVFEx fvf,DWORD nVertice);

	/** Builds the progressive mesh with the specified number of levels.
	@param numLevels The number of levels to include in the output excluding the full detail version.
	@param outList Pointer to a list of LOD geometry data which will be completed by the application.
	Each entry is a reduced form of the mesh, in decreasing order of detail.
	@param quota The way to derive the number of vertices removed at each LOD
	@param reductionValue Either the proportion of vertices to remove at each level, or a fixed
	number of vertices to remove at each level, depending on the value of quota
	*/
	virtual void Build(WORD numLevels, LODFaceList* outList, 
		VertexReductionQuota quota = VRQ_PROPORTIONAL, float reductionValue = 0.5f );

protected:
	DWORD m_nVertex;
	std::vector<WORD>m_vecIndexData;

	size_t m_CurrNumIndexes;
	size_t m_NumCommonVertices;

	// Internal classes
	class PMTriangle;
	class PMVertex;

public: // VC6 hack

	/** A vertex as used by a face. This records the index of the actual vertex which is used
	by the face, and a pointer to the common vertex used for surface evaluation. */
	class PMFaceVertex {
	public:
		size_t realIndex;
		PMVertex* commonVertex;
	};

protected:

	/** A triangle in the progressive mesh, holds extra info like face normal. */
	class PMTriangle {
	public:
		PMTriangle();
		void setDetails(size_t index, PMFaceVertex *v0, PMFaceVertex *v1, PMFaceVertex *v2);
		void computeNormal(void);
		void replaceVertex(PMFaceVertex *vold, PMFaceVertex *vnew);
		bool hasCommonVertex(PMVertex *v) const;
		bool hasFaceVertex(PMFaceVertex *v) const;
		PMFaceVertex* getFaceVertexFromCommon(PMVertex* commonVert);
		void notifyRemoved(void);

		PMFaceVertex* vertex[3]; // the 3 points that make this tri
		D3DXVECTOR3   normal;    // unit vector othogonal to this face
		bool      removed;   // true if this tri is now removed
		size_t index;
	};

	/** A vertex in the progressive mesh, holds info like collapse cost etc. 
	This vertex can actually represent several vertices in the final model, because
	vertices along texture seams etc will have been duplicated. In order to properly
	evaluate the surface properties, a single common vertex is used for these duplicates,
	and the faces hold the detail of the duplicated vertices.
	*/
	class PMVertex {
	public:
		PMVertex();
		void setDetails(const D3DXVECTOR3& v, size_t index);
		void removeIfNonNeighbor(PMVertex *n);
		bool isBorder(void);/// true if this vertex is on the edge of an open geometry patch
		bool isManifoldEdgeWith(PMVertex* v); // is edge this->src a manifold edge?
		void notifyRemoved(void);

		D3DXVECTOR3  position;  // location of point in euclidean space
		size_t index;       // place of vertex in original list
		typedef std::set<PMVertex *> NeighborList;
		typedef std::set<PMVertex *> DuplicateList;
		NeighborList neighbor; // adjacent vertices
		typedef std::set<PMTriangle *> FaceList;
		FaceList face;     // adjacent triangles

		float collapseCost;  // cached cost of collapsing edge
		PMVertex * collapseTo; // candidate vertex for collapse
		bool      removed;   // true if this vert is now removed
		bool	  toBeRemoved; // denug

		bool seam;	/// true if this vertex is on a model seam where vertices are duplicated

	};

	typedef std::vector<PMTriangle> TriangleList;
	typedef std::vector<PMFaceVertex> FaceVertexList;
	typedef std::vector<PMVertex> CommonVertexList;
	typedef std::vector<float> WorstCostList;

	/// Data used to calculate the collapse costs
	struct PMWorkingData
	{
		TriangleList m_TriList; /// List of faces
		FaceVertexList m_FaceVertList; // The vertex details referenced by the triangles
		CommonVertexList m_VertList; // The master list of common vertices
	};

	typedef std::vector<PMWorkingData> WorkingDataList;
	/// Multiple copies, 1 per vertex buffer
	WorkingDataList m_WorkingData;

	/// The worst collapse cost from all vertex buffers for each vertex
	WorstCostList m_WorstCosts;

	/// Internal method for building PMWorkingData from geometry data
	void AddWorkingData(void *pVertexData,FVFEx fvf,DWORD nVertice,WORD* pIndiceData,DWORD nIndice);

	/// Internal method for initialising the edge collapse costs
	void initialiseEdgeCollapseCosts(void);
	/// Internal calculation method for deriving a collapse cost  from u to v
	float computeEdgeCollapseCost(PMVertex *src, PMVertex *dest);
	/// Internal method evaluates all collapse costs from this vertex and picks the lowest for a single buffer
	float computeEdgeCostAtVertexForBuffer(WorkingDataList::iterator idata, size_t vertIndex);
	/// Internal method evaluates all collapse costs from this vertex for every buffer and returns the worst
	void computeEdgeCostAtVertex(size_t vertIndex);
	/// Internal method to compute edge collapse costs for all buffers /
	void computeAllCosts(void);
	/// Internal method for getting the index of next best vertex to collapse
	size_t getNextCollapser(void);
	/// Internal method builds an new LOD based on the current state
	void bakeNewLOD(std::vector<WORD>&vecIndice);

	/** Internal method, collapses vertex onto it's saved collapse target. 
	@remarks
	This updates the working triangle list to drop a triangle and recalculates
	the edge collapse costs around the collapse target. 
	This also updates all the working vertex lists for the relevant buffer. 
	*/
	void collapse(PMVertex *collapser);

	/** Internal debugging method */
//	void dumpContents(const String& log);


};


