
#include "stdh.h"

#include "NavService.h"

bool navPath::getPos(float dist,dtVec3 &pos)
{	
	if(!m_navService)
		return false;

	return m_navService->getPos(m_idx,dist,pos);
}

void navPath::reset(dtVec3 &start,dtVec3 &target)	
{
	if(m_navService)
		m_navService->reset(m_idx,start,target);	
}

bool navPath::isCompleted()
{
	if(!m_navService)
		return false;

	return m_navService->isCompleted(m_idx);	
}

bool navPath::getVaildDist(float &dist)	
{
	if(!m_navService)
		return false;

	return m_navService->getVaildDist(m_idx,dist);	
}

bool navPath::ensureValid(float dist)
{
	if(!m_navService)
		return false;

	return m_navService->ensureValid(m_idx,dist);	
}

void navPath::discard(float dist)	
{
	if(m_navService)
		m_navService->discard(m_idx,dist);	
}

void navPath::append(dtVec3 *pos,int num)
{
	if(num>0)
	{
		float d = 0;
		if(!m_footprints.empty())
			d = m_footprints.back().m_dist;
		
		for(int i = 0;i<num;++i)
		{	
			if(!m_footprints.empty())
			{
				float dCur = (pos[i] - m_footprints.back().m_pos).getLength();
				if(dCur<0.001f)
					continue;
				d = d + dCur;
			}
			m_footprints.push_back(FootPrint(pos[i],d));
		}
	}
}

void navPath::addRef()
{
	++m_refcount;
}

void navPath::release()
{
	--m_refcount;
	if(m_refcount<=0)
	{
		if(m_navService)
			m_navService->destroyPath(m_idx);
		else
			delete this;
	}
}

void navPath::setParams(navPathParams &params)
{
	if(params.m_maxSearchNodes>m_navService->m_maxSearchNodes)
		params.m_maxSearchNodes = m_navService->m_maxSearchNodes;

	if(params.m_maxSearchStep>m_navService->m_maxSearchStep)
		params.m_maxSearchStep = m_navService->m_maxSearchStep;

	m_params = params;
}

//////////////////////////////////////////////////////////////////////////

navService::navService(void)
{
	m_navMesh = 0;
	m_world = 0;
	m_pathRef			= 0;
	m_pathStraigthRef	= 0;
	m_pathPoint			= 0;
	m_pathFlags			= 0;

}

navService::~navService(void)
{

}

void navService::init(dtMeshBase * mesh,rtWorld * world,
					  int maxSearchNodes/* = default_maxSearchNodes*/,
					  int maxSearchStep /*= default_maxSearchStep*/)
{
	m_maxSearchStep = maxSearchStep;
	m_maxSearchNodes = maxSearchNodes;

	m_navMesh = mesh;
	m_world = world;
	m_rtQuery.init(world);
	m_navQuery.init(m_navMesh,m_maxSearchNodes);
	m_navTileQuery.init(m_maxSearchNodes);

	m_pathRef			= new dtPolyRef[m_maxSearchNodes];
	m_pathStraigthRef	= new dtPolyRef[m_maxSearchNodes];
	m_pathPoint			= new dtVec3[m_maxSearchNodes];
	m_pathFlags			= new unsigned char[m_maxSearchNodes];
}

void navService::unInit(void)
{
	for(int i = 0;i<m_Paths.size();++i)
	{
		navPath * path = m_Paths[i];
		if(path->m_refcount==0)
			delete path;
		else
			path->m_navService = NULL;
	}

	m_Paths.clear();
	m_idles.clear();
//	m_tempSearchpath.clear();

	m_navTileQuery.unInit();
	
	delete [] m_pathRef;
	delete [] m_pathStraigthRef;
	delete [] m_pathPoint;
	delete [] m_pathFlags;
	
	m_pathRef = 0;
	m_pathStraigthRef = 0;
	m_pathPoint = 0;
	m_pathFlags = 0;

	m_navMesh = 0;
	m_world = 0;
}

navPath * navService::createPath()
{
	if(!m_navMesh||!m_world)
		return NULL;

	int idx = -1;
	
	if(m_idles.empty())
	{
		idx = m_Paths.size();
		m_Paths.resize(idx + 1);
		m_Paths[idx] = new navPath;
	}
	else
	{
		idx = m_idles.back();
		m_idles.pop_back();
	}
	
	assert(idx<m_Paths.size());

	navPath * path = m_Paths[idx];
	path->m_navService = this;
	path->m_idx = idx;
	assert(path->m_refcount==0);
	path->addRef();
	
	path->m_params.m_maxSearchNodes = m_maxSearchNodes;
	path->m_params.m_maxSearchStep = m_maxSearchStep;

	return path;
}

void navService::destroyPath(int ia)
{
	assert(ia<m_Paths.size());
	navPath * path = m_Paths[ia];
	assert(path->m_refcount==0);
	path->m_state = navPath::Disable;
	m_idles.push_back(ia);
}

bool navService::getPos(int ia,float dist,dtVec3 &pos)
{
	if(ia<m_Paths.size())
	{
		navPath * path = m_Paths[ia];
		
		if(path->m_state==navPath::OnNav||
			path->m_state==navPath::EndNav)
		{
			int k0 =0 ,k1 = 0;
			float r = 0;

			if(!findKey(k0,k1,r,path,dist))
				return FALSE;
			else
			{
				if(k0==k1)
					pos = path->m_footprints[k0].m_pos;
				else
					pos = r*path->m_footprints[k0].m_pos + (1.0f - r)*path->m_footprints[k1].m_pos;
			}
			return true;
		}
	}

	return false;
}

bool navService::isCompleted(int ia)
{
	if(ia<m_Paths.size())
	{
		navPath * path = m_Paths[ia];
		return (path->m_state==navPath::EndNav);
	}
	return false;
}

bool navService::getVaildDist(int ia,float &dist)
{
	if(ia<m_Paths.size())
	{
		navPath * path = m_Paths[ia];

		dist = 0;
		if(!path->m_footprints.empty())
			dist = path->m_footprints.back().m_dist;

		return true;
	}

	return false;
}

bool navService::ensureValid(int ia,float dist)
{
	if(ia<m_Paths.size())
	{
		navPath *path = m_Paths[ia];
		
		if(!path->m_footprints.empty()&&
			dist<path->m_footprints[0].m_dist)
			return false;

		updatePos(ia,dist);
		
		if(!path->m_footprints.empty()&&path->m_footprints.back().m_dist>=dist)
			return true;
	}

	return false;
}

void navService::discard(int ia,float dist)
{
	const int max_tolerance = 100;

	if(ia<m_Paths.size())
	{
		navPath *path = m_Paths[ia];
		
		int k0 = 0,k1 = 0;
		float r = 0;
		if(findKey(k0,k1,r,path,dist)&&k0>max_tolerance)
		{
			std::vector<navPath::FootPrint> temp;

			for(int i = k0;i<path->m_footprints.size();++i)
				temp.push_back(path->m_footprints[i]);
			
			path->m_footprints.swap(temp);
		}
	}
}

void navService::reset(int ia,dtVec3 &start,dtVec3 &target)
{
	if(ia<m_Paths.size())
	{
		navPath *path = m_Paths[ia];
		path->clear();
		path->m_start = start;
		path->m_target = target;
		path->m_state = navPath::BeginNav;
	}
}

#include "timer/profiler.h"

void navService::updatePos(int ia,float dist)
{
	assert(ia<m_Paths.size());

	navPath *path = m_Paths[ia];	
	if(path->m_state==navPath::Disable)
		return;
	
	if(path->m_state==navPath::EndNav)
		return;

	if(path->m_state==navPath::BeginNav)
	{
		ProfilerStart_Recent(FindGate);
		
		if(!path->m_params.m_diableRouting)
			findGatePath(ia);
		
		ProfilerEnd();

		path->m_state = navPath::OnNav;
	}
	
	calcPolyPath2Dist(ia,dist);
}

bool navService::findKey(int &k0,int &k1,float &r,navPath *path,float dist)
{
	if(path->m_footprints.empty())
		return false;

	int s0 = 0,s1 = path->m_footprints.size() - 1,mid = 0;
	
	navPath::FootPrint * p = &(path->m_footprints[0]);
	navPath::FootPrint *cur = NULL;
	
	if(dist<path->m_footprints[0].m_dist||dist>path->m_footprints.back().m_dist)
		return false;

	while(s1-s0>1){
		
		mid = s0 + (s1-s0)/2;		
		cur = p + mid;

		if(cur->m_dist==dist)
		{
			k0 = mid;
			k1 = mid;
			break;
		}	
		else if(cur->m_dist>dist)
			s1 = mid;
		else if(cur->m_dist<dist)
			s0 = mid;
		else
			s0 = s1 = mid;
	}
	
	k0 = s0;
	k1 = s1;
	
	if(s1>s0)
		r = (p[k1].m_dist - dist)/(p[k1].m_dist - p[k0].m_dist);
	else
		r = 1.0f;
		
	return true;
}

void navService::findGatePath(int ia)
{
	assert(ia<m_Paths.size());

	navPath *path = m_Paths[ia];

	path->m_gates.clear();
	path->m_iGateCur = 0;

	int rx0,ry0,x0,y0;
	int rx1,ry1,x1,y1;

	m_world->calcRegionLoc(path->m_start,rx0,ry0,x0,y0);
	m_world->calcRegionLoc(path->m_target,rx1,ry1,x1,y1);

	rtRegion * rgSrc = m_world->getRegion(rx0,ry0);
	rtRegion * rgDst = m_world->getRegion(rx1,ry1);

	if(!rgSrc||!rgDst)
		return;

	const  int maxGates = 255;
	static rtGateRef gatesPath[maxGates];
	static rtGateRef gatesSrc[maxGates];
	static rtGateRef gatesDst[maxGates];
	static int mincostSrc[maxGates];
	static int mincostDst[maxGates];

	int numGateSrc = 0;
	int numGateDst = 0;
	int numPathGates = 0;

	if(!m_world->getBestRegionGates(rgSrc,path->m_start,gatesSrc,mincostSrc,numGateSrc,maxGates))
		return;

	if(!m_world->getBestRegionGates(rgDst,path->m_target,gatesDst,mincostDst,numGateDst,maxGates))
		return;

	dtVec3 gateMid;
	bool bFound = false;
	rtStatus status = RT_FAILURE;
	int minD2End = 0x7fffffff,dcur = 0x7fffffff;
	m_tempSearchpath.clear();

	for(int i = 0;i<numGateSrc;++i){

		if(!m_world->getGateMid(gatesSrc[i],gateMid))
			continue;

		if(!isConInRegion(rgSrc,path->m_start,gateMid))
			continue;

		for(int j = 0; j<numGateDst; ++j){

			if(!m_world->getGateMid(gatesDst[j],gateMid))
				continue;

			if(!isConInRegion(rgDst,path->m_target,gateMid))
				continue;
			
			status = m_rtQuery.findRegPath(gatesSrc[i],gatesDst[j],gatesPath,numPathGates,maxGates,dcur);
			
			if(status==RT_SUCCESS)
			{
				m_tempSearchpath.resize(numPathGates);
				memcpy(m_tempSearchpath.data(),gatesPath,numPathGates*sizeof(rtGateRef));
				break;
			}
			else
			{
				if(status!=RT_FAILURE&&dcur<minD2End)
				{
					m_tempSearchpath.resize(numPathGates);
					memcpy(m_tempSearchpath.data(),gatesPath,numPathGates*sizeof(rtGateRef));
					minD2End = dcur;
				}
			}
		}
		if(status==RT_SUCCESS)
			break;
	}
	
	path->m_gates.swap(m_tempSearchpath);
	
	if(!path->m_gates.empty())
	{
		dtVec3 midPre;
		rtGateRef &ref = path->m_gates[0];
		m_world->getGateMid(ref,midPre);
		midPre.y += 1.0f;

		path->m_gatePath.resize(2*(numPathGates-1));					
		
		for(int k = 1; k<numPathGates; ++k)
		{
			ref = gatesPath[k];
			m_world->getGateMid(ref,gateMid);
			gateMid.y += 1.0f;

			path->m_gatePath.push_back(midPre);
			path->m_gatePath.push_back(gateMid);
			midPre = gateMid;
		}
	}

	if(!bFound)
		path->m_msg.append(" [ can't find gates path ] ");
}

dtMeshTile * navService::getOffMeshTile(int rx,int ry)
{
	dtMeshTile *tile = NULL;

	if(m_navMesh->getTilesAt(rx,ry,&tile,1))
	{
		return (dtMeshTile *)tile;
	}
	return NULL;
}

bool navService::isConInRegion(rtRegion * rg,const dtVec3 &startPos,const dtVec3 &endPos)
{
	dtMeshTile *tile = getOffMeshTile(rg->rx,rg->ry);
	if(tile)
	{
		m_navTileQuery.setMeshTile(tile);
		return m_navTileQuery.isCon(startPos,endPos);
	}
	return false;
}

void navService::calcPolyPath2Dist(int ia,float dist)
{
	assert(ia<m_Paths.size());

	navPath *path = m_Paths[ia];
	
	assert(path->m_state!=navPath::EndNav);	//已经完成计算

	if(path->m_params.m_diableRouting)
	{
		dtVec3 * posPath = 0;
		int nPos = 0;
		
		if(findPolyPath(path,path->m_start,path->m_target,path->m_params.m_maxSearchNodes,posPath,nPos))
			path->append(posPath,nPos);

		path->m_state = navPath::EndNav;
	}
	else
	{
		float curDist = -1.0f;

		if(!path->m_footprints.empty())
		{
			navPath::FootPrint & fp = path->m_footprints.back();
			curDist = fp.m_dist;
		}

		while(curDist<dist)
		{	
			dtVec3 start;
			dtVec3 target;
			dtVec3 *posPath = NULL;
			int nPos = 0;
			
			int i = 0;
			if(!path->m_gates.empty())
			{
				rtGateRef gateSrc = path->m_gates[path->m_iGateCur];

				i = path->m_iGateCur + 1;
				float dist = 0;
				for(;i<path->m_gates.size();++i)
				{
					rtGateRef &gateDst = path->m_gates[i];

					dist += m_world->getDistGateToGate(gateSrc,gateDst);
					if(dist>path->m_params.m_maxSearchStep)
						break;

					gateSrc = gateDst;
				}
			}
			
			if(i<path->m_gates.size())
			{
				rtGateRef &gateDst = path->m_gates[i];
				m_world->getGateMid(gateDst,target);
			}
			else
			{	
				i = path->m_gates.size();
				target = path->m_target;
				path->m_state = navPath::EndNav;
			}

			path->m_iGateCur = i;

			if(path->m_footprints.empty())
				start = path->m_start;
			else
				start = path->m_footprints.back().m_pos;

			if(findPolyPath(path,start,target,path->m_params.m_maxSearchNodes,posPath,nPos))
			{
				path->append(posPath,nPos);
				assert(!path->m_footprints.empty());
				curDist = path->m_footprints.back().m_dist;
			}
			else
			{
				//失败原因有：1.需要的信息未加载 2.路径过于复杂，超过尝试的次数
				break;
			}

			if(path->m_state ==  navPath::EndNav)
				break;
		}
	}
}

bool navService::findPolyPath(navPath *path,const dtVec3 &startPos,const dtVec3 &endPos,int maxPath,dtVec3 *&posPath,int &nPos)
{
	int nPoly = 0;
	const float ext[3] = {1.0f,1.5f,1.0f};

	ProfilerStart_Recent(queryPolygon);

	m_navQuery.findNearestPoly((const float *)&startPos,ext,&m_filter,&m_startRef,0);
	m_navQuery.findNearestPoly((const float *)&endPos,ext,&m_filter,&m_endRef,0);
	
	ProfilerEnd();

	nPos = 0;
	posPath = 0;
	
	ProfilerStart_Recent(findPathPolygon);
	dtStatus status = m_navQuery.findPath(m_startRef,m_endRef,
										  (float *)&startPos,(float *)&endPos,
										  &m_filter,
										  m_pathRef,&nPoly,maxPath,false);
	ProfilerEnd();
	
	if(status&DT_OUT_OF_NODES)
		path->m_msg.append(" [searchNodes is not enough !] ");
	
	if(status&DT_PARTIAL_RESULT)
		path->m_msg.append(" [can't attach to end !] ");

	if(status&DT_BUFFER_TOO_SMALL)
		path->m_msg.append(" [ path buffer is too small !] ");

	ProfilerStart_Recent(findStraightpath);
	if(nPoly>0)
	{
		dtVec3 epos = endPos;

		if (m_pathRef[nPoly-1] != m_endRef)
			m_navQuery.closestPointOnPoly(m_pathRef[nPoly-1], (const float *)&endPos,(float*)&epos);
		
		m_navQuery.findStraightPath( (const float *)&startPos,(const float *)&epos, m_pathRef, nPoly,
									(float *)(m_pathPoint), m_pathFlags,
									m_pathStraigthRef, &nPos,maxPath);
		
		const dtMeshTile * tile = 0;
		const dtPoly * poly = 0;
		for(int i = 0;i<nPoly;++i)
		{
			dtPolyRef &ref = m_pathRef[i];
			m_navMesh->getTileAndPolyByRef(ref,&tile,&poly);
			
			if(!poly->vertCount)
				continue;

			dtVec3 polyCenter;
			for(int k = 0;k<poly->vertCount;++k)
			{
				int idx0 = poly->verts[k];
				int idx1 = poly->verts[(k+1)%poly->vertCount];
				
				dtVec3 *v0 = (dtVec3 *)(tile->verts + idx0*3);
				dtVec3 *v1 = (dtVec3 *)(tile->verts + idx1*3);
				
				path->m_polygonPath.push_back(*v0);
				path->m_polygonPath.push_back(*v1);
				
				polyCenter += *v0;
			}

			polyCenter /= float(poly->vertCount);
			path->m_polygonCenter.push_back(polyCenter);
		}
		
		posPath = m_pathPoint;
	}
	ProfilerEnd();

	return (nPos>0);
}



