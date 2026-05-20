//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#include "stdh.h"
#include "DetourNavMesh.h"
#include "DetourCommon.h"


//48-64|32-47|0-31
// rx   ry    ip

// inline unsigned int allocLink(dtMeshTile* tile)
// {
// 	if (tile->linksFreeList == DT_NULL_LINK)
// 		return DT_NULL_LINK;
// 
// 	unsigned int link = tile->linksFreeList;
// 
// 	tile->linksFreeList = tile->links[link].next;
// 
// 	return link;
// }
// 
// inline void freeLink(dtMeshTile* tile, unsigned int link)
// {
// 	tile->links[link].next = tile->linksFreeList;
// 	tile->linksFreeList = link;
// }

inline bool overlapSlabs(const float* amin, const float* amax,
						 const float* bmin, const float* bmax,
						 const float px, const float py)
{
	// Check for horizontal overlap.
	// The segment is shrunken a little so that slabs which touch
	// at end points are not connected.
	const float minx = dtMax(amin[0]+px,bmin[0]+px);
	const float maxx = dtMin(amax[0]-px,bmax[0]-px);
	if (minx > maxx)
		return false;

	// Check vertical overlap.
	const float ad = (amax[1]-amin[1]) / (amax[0]-amin[0]);
	const float ak = amin[1] - ad*amin[0];
	const float bd = (bmax[1]-bmin[1]) / (bmax[0]-bmin[0]);
	const float bk = bmin[1] - bd*bmin[0];
	const float aminy = ad*minx + ak;
	const float amaxy = ad*maxx + ak;
	const float bminy = bd*minx + bk;
	const float bmaxy = bd*maxx + bk;
	const float dmin = bminy - aminy;
	const float dmax = bmaxy - amaxy;

	// Crossing segments always overlap.
	if (dmin*dmax < 0)
		return true;

	// Check for overlap at endpoints.
	const float thr = dtSqr(py*2);
	if (dmin*dmin <= thr || dmax*dmax <= thr)
		return true;

	return false;
}

static float getSlabCoord(const float* va, const int side)
{
	if (side == 0 || side == 4)
		return va[0];
	else if (side == 2 || side == 6)
		return va[2];
	return 0;
}

static void calcSlabEndPoints(const float* va, const float* vb, float* bmin, float* bmax, const int side)
{
	if (side == 0 || side == 4)
	{
		if (va[2] < vb[2])
		{
			bmin[0] = va[2];
			bmin[1] = va[1];
			bmax[0] = vb[2];
			bmax[1] = vb[1];
		}
		else
		{
			bmin[0] = vb[2];
			bmin[1] = vb[1];
			bmax[0] = va[2];
			bmax[1] = va[1];
		}
	}
	else if (side == 2 || side == 6)
	{
		if (va[0] < vb[0])
		{
			bmin[0] = va[0];
			bmin[1] = va[1];
			bmax[0] = vb[0];
			bmax[1] = vb[1];
		}
		else
		{
			bmin[0] = vb[0];
			bmin[1] = vb[1];
			bmax[0] = va[0];
			bmax[1] = va[1];
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void dtMeshBase::addTileLink(dtMeshTile *tile)
{
	assert(tile);

	dtMeshHeader * header = tile->header;

	connectIntLinks(tile);
	// Create connections with neighbour tiles.
	static const int MAX_NEIS = 32;
	dtMeshTile* neis[MAX_NEIS];
	int nneis;

	//*************************************//
	// Connect with layers in current tile.//
	//*************************************//

	// Connect with neighbour tiles.
	for (int i = 0; i < 8; ++i)
	{
		nneis = getNeighbourTilesAt(header->x, header->y, i, neis, MAX_NEIS);
		for (int j = 0; j < nneis; ++j)
		{
			connectExtLinks(tile, neis[j], i);
			connectExtLinks(neis[j], tile, dtOppositeTile(i));
		}
	}
}

void dtMeshBase::removeTileLink(dtMeshTile *tile)
{
	assert(tile);

	dtMeshHeader * header = tile->header;

	// Create connections with neighbour tiles.
	static const int MAX_NEIS = 32; 
	dtMeshTile* neis[MAX_NEIS];
	int nneis;

	// Connect with neighbour tiles.
	for (int i = 0; i < 8; ++i)
	{
		nneis = getNeighbourTilesAt(tile->header->x, tile->header->y, i, neis, MAX_NEIS);
		for (int j = 0; j < nneis; ++j)
		{
			unconnectExtLinks(neis[j], tile);
		}
	}
	
	tile->unconnectSelfLinks();
}

int dtMeshBase::getNeighbourTilesAt(const int x, const int y, const int side,dtMeshTile** tiles, const int maxTiles) const
{
	int nx = x, ny = y;

	switch (side)
	{
	case 0: nx++; break;
	case 1: nx++; ny++; break;
	case 2: ny++; break;
	case 3: nx--; ny++; break;
	case 4: nx--; break;
	case 5: nx--; ny--; break;
	case 6: ny--; break;
	case 7: nx++; ny--; break;
	};

	return getTilesAt(nx, ny, tiles, maxTiles);
}

void dtMeshBase::closestPointOnPolyInTile(const dtMeshTile* tile, unsigned int ip,
							  const float* pos, float* closest) const
{

	const dtPoly* poly = &tile->polys[ip];

	float closestDistSqr = FLT_MAX;
	const dtPolyDetail* pd = &tile->detailMeshes[ip];

	for (int j = 0; j < pd->triCount; ++j)
	{
		const unsigned char* t = &tile->detailTris[(pd->triBase+j)*4];
		const float* v[3];
		for (int k = 0; k < 3; ++k)
		{
			if (t[k] < poly->vertCount)
				v[k] = &tile->verts[poly->verts[t[k]]*3];
			else
				v[k] = &tile->detailVerts[(pd->vertBase+(t[k]-poly->vertCount))*3];
		}
		float pt[3];
		dtClosestPtPointTriangle(pt, pos, v[0], v[1], v[2]);
		float d = dtVdistSqr(pos, pt);
		if (d < closestDistSqr)
		{
			dtVcopy(closest, pt);
			closestDistSqr = d;
		}
	}
}

int dtMeshBase::queryPolygonsInTile(const dtMeshTile* tile, const float* qmin, const float* qmax,
						dtPolyRef* polys, const int maxPolys) const
{
	if (tile->bvTree)
	{
		const dtBVNode* node = &tile->bvTree[0];
		const dtBVNode* end = &tile->bvTree[tile->header->bvNodeCount];
		const float* tbmin = tile->header->bmin;
		const float* tbmax = tile->header->bmax;
		const float qfac = tile->header->bvQuantFactor;

		// Calculate quantized box
		unsigned short bmin[3], bmax[3];
		// dtClamp query box to world box.
		float minx = dtClamp(qmin[0], tbmin[0], tbmax[0]) - tbmin[0];
		float miny = dtClamp(qmin[1], tbmin[1], tbmax[1]) - tbmin[1];
		float minz = dtClamp(qmin[2], tbmin[2], tbmax[2]) - tbmin[2];
		float maxx = dtClamp(qmax[0], tbmin[0], tbmax[0]) - tbmin[0];
		float maxy = dtClamp(qmax[1], tbmin[1], tbmax[1]) - tbmin[1];
		float maxz = dtClamp(qmax[2], tbmin[2], tbmax[2]) - tbmin[2];
		// Quantize
		bmin[0] = (unsigned short)(qfac * minx) & 0xfffe;
		bmin[1] = (unsigned short)(qfac * miny) & 0xfffe;
		bmin[2] = (unsigned short)(qfac * minz) & 0xfffe;
		bmax[0] = (unsigned short)(qfac * maxx + 1) | 1;
		bmax[1] = (unsigned short)(qfac * maxy + 1) | 1;
		bmax[2] = (unsigned short)(qfac * maxz + 1) | 1;

		// Traverse tree
		dtPolyRef base = getPolyRefBase(tile);
		int n = 0;
		while (node < end)
		{
			const bool overlap = dtOverlapQuantBounds(bmin, bmax, node->bmin, node->bmax);
			const bool isLeafNode = node->i >= 0;

			if (isLeafNode && overlap)
			{
				if (n < maxPolys)
					polys[n++] = base | (dtPolyRef)node->i;
			}

			if (overlap || isLeafNode)
				node++;
			else
			{
				const int escapeIndex = -node->i;
				node += escapeIndex;
			}
		}
		
		return n;
	}
	else
	{

		float bmin[3], bmax[3];
		int n = 0;
		dtPolyRef base = getPolyRefBase(tile);
		for (int i = 0; i < tile->header->polyCount; ++i)
		{
			// Calc polygon bounds.
			dtPoly* p = &tile->polys[i];
			const float* v = &tile->verts[p->verts[0]*3];
			dtVcopy(bmin, v);
			dtVcopy(bmax, v);
			for (int j = 1; j < p->vertCount; ++j)
			{
				v = &tile->verts[p->verts[j]*3];
				dtVmin(bmin, v);
				dtVmax(bmax, v);
			}
			if (dtOverlapBounds(qmin,qmax, bmin,bmax))
			{
				if (n < maxPolys)
					polys[n++] = base | (dtPolyRef)i;
			}
		}

		return n;
	}

}

dtPolyRef dtMeshBase::findNearestPolyInTile(const dtMeshTile* tile, const float* center,const float* extents, float* nearestPt) const
{
	float bmin[3], bmax[3];
	dtVsub(bmin, center, extents);
	dtVadd(bmax, center, extents);

	// Get nearby polygons from proximity grid.
	dtPolyRef polys[128];
	int polyCount = queryPolygonsInTile(tile, bmin, bmax, polys, 128);

	// Find nearest polygon amongst the nearby polygons.
	dtPolyRef nearest = 0;
	float nearestDistanceSqr = FLT_MAX;
	for (int i = 0; i < polyCount; ++i)
	{
		dtPolyRef ref = polys[i];
		float closestPtPoly[3];
		closestPointOnPolyInTile(tile, decodePolyIdPoly(ref), center, closestPtPoly);
		float d = dtVdistSqr(center, closestPtPoly);
		if (d < nearestDistanceSqr)
		{
			if (nearestPt)
				dtVcopy(nearestPt, closestPtPoly);
			nearestDistanceSqr = d;
			nearest = ref;
		}
	}

	return nearest;
}

int dtMeshBase::findConnectingPolys(const float* va, const float* vb,
						const dtMeshTile* tile, int side,
						dtPolyRef* con, float* conarea, int maxcon) const
{
	if (!tile) return 0;

	float amin[2], amax[2];
	calcSlabEndPoints(va,vb, amin,amax, side);
	const float apos = getSlabCoord(va, side);

	// Remove links pointing to 'side' and compact the links array. 
	float bmin[2], bmax[2];
	unsigned short m = DT_EXT_LINK | (unsigned short)side;
	int n = 0;

	dtPolyRef base = getPolyRefBase(tile);

	for (int i = 0; i < tile->header->polyCount; ++i)
	{
		dtPoly* poly = &tile->polys[i];
		const int nv = poly->vertCount;
		for (int j = 0; j < nv; ++j)
		{
			// Skip edges which do not point to the right side.
			if (poly->neis[j] != m) continue;

			const float* vc = &tile->verts[poly->verts[j]*3];
			const float* vd = &tile->verts[poly->verts[(j+1) % nv]*3];
			const float bpos = getSlabCoord(vc, side);

			// Segments are not close enough.
			if (dtAbs(apos-bpos) > 0.25f/*0.01f*/)
				continue;

			// Check if the segments touch.
			calcSlabEndPoints(vc,vd, bmin,bmax, side);

			if (!overlapSlabs(amin,amax, bmin,bmax, 0.01f, tile->header->walkableClimb)) continue;

			// Add return value.
			if (n < maxcon)
			{
				conarea[n*2+0] = dtMax(amin[0], bmin[0]);
				conarea[n*2+1] = dtMin(amax[0], bmax[0]);
				con[n] = base | (dtPolyRef)i;
				n++;
			}
			break;
		}
	}

	return n;
}

void dtMeshBase::connectIntLinks(dtMeshTile* tile)
{
	if (!tile) return;
	
	assert(tile->m_linkAlloc);

	dtPolyRef base = getPolyRefBase(tile);

	for (int i = 0; i < tile->header->polyCount; ++i)
	{
		dtPoly* poly = &tile->polys[i];
		poly->firstLink = DT_NULL_LINK;

		if (poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)
			continue;

		// Build edge links backwards so that the links will be
		// in the linked list from lowest index to highest.
		for (int j = poly->vertCount-1; j >= 0; --j)
		{
			// Skip hard and non-internal edges.
			if (poly->neis[j] == 0 || (poly->neis[j] & DT_EXT_LINK)) continue;

			dtLink* link = 0;
			unsigned int idx = tile->m_linkAlloc->allocLink(&link);

			if (link)
			{
				link->ref = base | (dtPolyRef)(poly->neis[j]-1);
				link->edge = (unsigned char)j;
				link->side = 0xff;
				link->bmin = link->bmax = 0;
				// Add to linked list.
				assert(poly->firstLink!=idx);
				link->next = poly->firstLink;
				poly->firstLink = idx;
			}
		}
	}
}

void dtMeshBase::connectIntOffMeshLinks(dtMeshTile* tile)
{
	if (!tile) return;
	
	assert(tile->m_linkAlloc);

	dtPolyRef base = getPolyRefBase(tile);

	// Find Off-mesh connection end points.
	for (int i = 0; i < tile->header->offMeshConCount; ++i)
	{
		dtOffMeshConnection* con = &tile->offMeshCons[i];
		dtPoly* poly = &tile->polys[con->poly];

		const float ext[3] = { con->rad, tile->header->walkableClimb, con->rad };

		for (int j = 0; j < 2; ++j)
		{
			unsigned char side = j == 0 ? 0xff : con->side;

			if (side == 0xff)
			{
				// Find polygon to connect to.
				const float* p = &con->pos[j*3];
				float nearestPt[3];
				dtPolyRef ref = findNearestPolyInTile(tile, p, ext, nearestPt);
				if (!ref) continue;
				// findNearestPoly may return too optimistic results, further check to make sure. 
				if (dtSqr(nearestPt[0]-p[0])+dtSqr(nearestPt[2]-p[2]) > dtSqr(con->rad))
					continue;
				// Make sure the location is on current mesh.
				float* v = &tile->verts[poly->verts[j]*3];
				dtVcopy(v, nearestPt);

				// Link off-mesh connection to target poly.
				dtLink* link = 0;
				unsigned int idx =	tile->m_linkAlloc->allocLink(&link);		//allocLink(tile);
				if (link)
				{
					link->ref = ref;
					link->edge = (unsigned char)j;
					link->side = 0xff;
					link->bmin = link->bmax = 0;
					// Add to linked list.
					assert(poly->firstLink!=idx);
					link->next = poly->firstLink;
					poly->firstLink = idx;
				}

				// Start end-point is always connect back to off-mesh connection,
				// Destination end-point only if it is bidirectional link. 
				if (j == 0 || (j == 1 && (con->flags & DT_OFFMESH_CON_BIDIR)))
				{
					// Link target poly to off-mesh connection.
					dtLink *link = 0;
					unsigned int idx =	tile->m_linkAlloc->allocLink(&link);	//allocLink(tile);
					if (link)
					{
						const unsigned short landPolyIdx = (unsigned short)decodePolyIdPoly(ref);
						dtPoly* landPoly = &tile->polys[landPolyIdx];
						link->ref = base | (dtPolyRef)(con->poly);
						link->edge = 0xff;
						link->side = 0xff;
						link->bmin = link->bmax = 0;
						// Add to linked list.
						assert(poly->firstLink!=idx);
						link->next = landPoly->firstLink;
						landPoly->firstLink = idx;
					}
				}

			}
		}
	}
}

void dtMeshBase::connectExtLinks(dtMeshTile* tile, dtMeshTile* target, int side)
{
	if (!tile) return;
	
	assert(tile->m_linkAlloc);

	// Connect border links.
	for (int i = 0; i < tile->header->polyCount; ++i)
	{
		dtPoly* poly = &tile->polys[i];

		const int nv = poly->vertCount;
		for (int j = 0; j < nv; ++j)
		{
			// Skip non-portal edges.
			if ((poly->neis[j] & DT_EXT_LINK) == 0)
				continue;

			const int dir = (int)(poly->neis[j] & 0xff);
			if (side != -1 && dir != side)
				continue;

			// Create new links
			const float* va = &tile->verts[poly->verts[j]*3];
			const float* vb = &tile->verts[poly->verts[(j+1) % nv]*3];
			dtPolyRef nei[4];
			float neia[4*2];
			int nnei = findConnectingPolys(va,vb, target, dtOppositeTile(dir), nei,neia,4);
			for (int k = 0; k < nnei; ++k)
			{
				dtLink *link = 0;
				unsigned int idx = tile->m_linkAlloc->allocLink(&link);			//allocLink(tile);
				if (link)
				{
					link->ref = nei[k];
					link->edge = (unsigned char)j;
					link->side = (unsigned char)dir;
					
					assert(poly->firstLink!=idx);
					link->next = poly->firstLink;
					poly->firstLink = idx;

					// Compress portal limits to a byte value.
					if (dir == 0 || dir == 4)
					{
						float tmin = (neia[k*2+0]-va[2]) / (vb[2]-va[2]);
						float tmax = (neia[k*2+1]-va[2]) / (vb[2]-va[2]);
						if (tmin > tmax)
							dtSwap(tmin,tmax);
						link->bmin = (unsigned char)(dtClamp(tmin, 0.0f, 1.0f)*255.0f);
						link->bmax = (unsigned char)(dtClamp(tmax, 0.0f, 1.0f)*255.0f);
					}
					else if (dir == 2 || dir == 6)
					{
						float tmin = (neia[k*2+0]-va[0]) / (vb[0]-va[0]);
						float tmax = (neia[k*2+1]-va[0]) / (vb[0]-va[0]);
						if (tmin > tmax)
							dtSwap(tmin,tmax);
						link->bmin = (unsigned char)(dtClamp(tmin, 0.0f, 1.0f)*255.0f);
						link->bmax = (unsigned char)(dtClamp(tmax, 0.0f, 1.0f)*255.0f);
					}
				}
			}
		}
	}
}

void dtMeshBase::connectExtOffMeshLinks(dtMeshTile* tile, dtMeshTile* target, int side)
{
	if (!tile) return;
	
	assert(target->m_linkAlloc);
	
	// Connect off-mesh links.
	// We are interested on links which land from target tile to this tile.
	const unsigned char oppositeSide = (unsigned char)dtOppositeTile(side);

	for (int i = 0; i < target->header->offMeshConCount; ++i)
	{
		dtOffMeshConnection* targetCon = &target->offMeshCons[i];
		if (targetCon->side != oppositeSide)
			continue;

		dtPoly* targetPoly = &target->polys[targetCon->poly];

		const float ext[3] = { targetCon->rad, target->header->walkableClimb, targetCon->rad };

		// Find polygon to connect to.
		const float* p = &targetCon->pos[3];
		float nearestPt[3];
		dtPolyRef ref = findNearestPolyInTile(tile, p, ext, nearestPt);
		if (!ref) continue;
		// findNearestPoly may return too optimistic results, further check to make sure. 
		if (dtSqr(nearestPt[0]-p[0])+dtSqr(nearestPt[2]-p[2]) > dtSqr(targetCon->rad))
			continue;
		// Make sure the location is on current mesh.
		float* v = &target->verts[targetPoly->verts[1]*3];
		dtVcopy(v, nearestPt);

		// Link off-mesh connection to target poly.
		dtLink * link = 0;
		unsigned int idx = target->m_linkAlloc->allocLink(&link);			//allocLink(target);
		if (link)
		{
			link->ref = ref;
			link->edge = (unsigned char)1;
			link->side = oppositeSide;
			link->bmin = link->bmax = 0;

			// Add to linked list.
			assert(targetPoly->firstLink!=idx);
			link->next = targetPoly->firstLink;
			targetPoly->firstLink = idx;
		}

		// Link target poly to off-mesh connection.
		if (targetCon->flags & DT_OFFMESH_CON_BIDIR)
		{
			link = 0;
			unsigned int idx = tile->m_linkAlloc->allocLink(&link);				 //allocLink(tile);
			if (link)
			{
				const unsigned short landPolyIdx = (unsigned short)decodePolyIdPoly(ref);
				dtPoly* landPoly = &tile->polys[landPolyIdx];
				link->ref = getPolyRefBase(target) | (dtPolyRef)(targetCon->poly);
				link->edge = 0xff;
				link->side = (unsigned char)side;
				link->bmin = link->bmax = 0;
				// Add to linked list.
				assert(landPoly->firstLink!=idx);
				link->next = landPoly->firstLink;
				landPoly->firstLink = idx;
			}
		}
	}
}

void dtMeshBase::unconnectExtLinks(dtMeshTile* tile, dtMeshTile* target)
{
	if (!tile || !target) return;

	assert(tile->m_linkAlloc);
	assert(target->m_linkAlloc);
	
	const unsigned int targetNum = decodePolyIdTile(getTileRef(target));

	for (int i = 0; i < tile->header->polyCount; ++i)
	{
		dtPoly* poly = &tile->polys[i];
		unsigned int j = poly->firstLink;
		unsigned int pj = DT_NULL_LINK;
		
		while (j != DT_NULL_LINK)
		{

			dtLink * link = tile->m_linkAlloc->getLink(j);

			if (link->side != 0xff &&
				decodePolyIdTile(link->ref) == targetNum)
			{
				// Revove link.
				unsigned int nj = link->next;//links[j].next;

				if (pj == DT_NULL_LINK)
					poly->firstLink = nj;
				else
					tile->m_linkAlloc->getLink(pj)->next = nj;

				tile->m_linkAlloc->freeLink(j);			// freeLink(tile, j);
				j = nj;
			}
			else
			{
				// Advance
				pj = j;
				j = link->next;						//links[j].next;
			}
		}
	}
}

void dtMeshBase::decodePolyLoc(const dtPolyRef &ref,short &x,short &y) const
{
	int t = int(ref>>32);
	x = short(t>>16);
	y = short(t&0xffff);
}

unsigned int dtMeshBase::decodePolyIdPoly(const dtPolyRef& ref) const
{
	return int(ref&0xffffffff);
}

int dtMeshBase::decodePolyIdTile(const dtPolyRef & ref) const
{
	return int(ref>>32);
}

dtPolyRef dtMeshBase::getPolyRefBase(const dtMeshTile* tile) const
{
	int rx = tile->header->x;
	int ry = tile->header->y;

	dtPolyRef ref = ((__int64(rx&0xffff)<<48)|(__int64(ry&0xffff)<<32))&0xffffffff00000000;

	return ref;
}




