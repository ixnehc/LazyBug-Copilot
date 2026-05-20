#include "stdh.h"
#include "CubicSpline4D.h"

float Vector4d_getLength(Vector4d v)
{
	return sqrt(v.x*v.x+v.y*v.y+v.z*v.z+v.w*v.w);
}

Vector4d Vector4d_normalize(Vector4d v)
{
	float dist=Vector4d_getLength(v);
	v.x/=dist;
	v.y/=dist;
	v.z/=dist;
	v.w/=dist;
	return v;
}


// cubic curve defined by 2 positions and 2 velocities
Vector4d GetPositionOnCubic(const Vector4d &startPos, const Vector4d &startVel, const Vector4d &endPos, const Vector4d &endVel, float time)
{
	static i_math::matrix44f hermite;
	static BOOL bInit=FALSE;
	if (!bInit)
	{ 
		hermite.set( 2.f,-2.f, 1.f, 1.f,
			-3.f, 3.f,-2.f,-1.f,
			0.f, 0.f, 1.f, 0.f,
			1.f, 0.f, 0.f, 0.f);

		bInit=TRUE;
	};

	i_math::matrix44f m;
	m.set(startPos.x,startPos.y,startPos.z,startPos.w,
		endPos.x,endPos.y,endPos.z,endPos.w,
		startVel.x,startVel.y,startVel.z,startVel.w,
		endVel.x,endVel.y,endVel.z,endVel.w);

	m=hermite*m;

	Vector4d timeVector;
	timeVector.set(time*time*time, time*time, time, 1.0f);
	m.transformVect(timeVector);
	return FORCE_TYPE(Vector4d,timeVector);
}

Vector4d GetVelocityOnCubic(const Vector4d &startPos, const Vector4d &startVel, const Vector4d &endPos, const Vector4d &endVel, float time)
{
	static i_math::matrix44f hermiteVelocity;
	static BOOL bInit=FALSE;
	if (!bInit)
	{ 
		hermiteVelocity.set( 0.0f,0.0f,0.0f,0.0f,
										6.0f,-6.0f,3.0f,3.0f,
										-6.0f,6.0f,-4.0f,-2.0f,
										0.0f,0.0f,1.0f,0.0f);

		bInit=TRUE;
	};

	i_math::matrix44f m;
	m.set(startPos.x,startPos.y,startPos.z,1,
		endPos.x,endPos.y,endPos.z,1,
		startVel.x,startVel.y,startVel.z,0,
		endVel.x,endVel.y,endVel.z,0);

	m=hermiteVelocity*m;

	Vector4d timeVector;
	timeVector.set(time*time*time, time*time, time, 1.0f);
	m.transformVect(timeVector);
// 	timeVector.x/=timeVector.w;
// 	timeVector.y/=timeVector.w;
// 	timeVector.z/=timeVector.w;
	return FORCE_TYPE(Vector4d,timeVector);
}


void CCubicSpline4D::Reset()
{
	_nodes.clear();
	_maxdist=0.0f;
}


// adds node and updates segment length
void CCubicSpline4D::AddNode(const Vector4d &pos)
{
	if (_nodes.size()>0)
	{
		if (_nodes[_nodes.size()-1].position.equals(pos))
			return;
	}
	Node t;
	t.position=pos;
	t.dist0=-1.0f;//负数表示需要自动计算
	t.dist=-1.0f;
	_nodes.push_back(t);
}

// Use timePeriod in place of actual node spacing
// ie time period is time from last node to this node
void CCubicSpline4D::AddNode(const Vector4d &pos, float timePeriod)
{
	Node t;
	t.position=pos;
	t.dist0=timePeriod;
	t.dist=-1.0f;
	_nodes.push_back(t);
}

void CCubicSpline4D::_CalcDistance(BOOL bAuto)
{
	int nodeCount=_nodes.size();
	if (nodeCount>1)
	{
		for(int i=0;i<nodeCount-1;i++)
		{
			if ((_nodes[i].dist0<0.0f)||bAuto)
				_nodes[i].dist=Vector4d_getLength((_nodes[i+1].position-_nodes[i].position));
			else
				_nodes[i].dist=_nodes[i].dist0;
		}
	}

	//最后一个点
	if (nodeCount>=1)
	{
		if ((_nodes[nodeCount-1].dist0<0.0f)||bAuto)
			_nodes[nodeCount-1].dist=0.0f;
		else
			_nodes[nodeCount-1].dist=_nodes[nodeCount-1].dist0;
	}

	_maxdist=0.0f;
	for(int i=0;i<nodeCount-1;i++)
	{
		_nodes[i].distTotal=_maxdist;
		_maxdist+=_nodes[i].dist;
	}
}


void CCubicSpline4D::BuildRNS(BOOL bCircular)
{
	_CalcDistance(TRUE);
	_BuildSpline();
	if (bCircular)
	{
		int nodeCount=_nodes.size();
		_nodes.push_back(_nodes[0]);

		_nodes[nodeCount-1].dist=Vector4d_getLength(_nodes[nodeCount].position-_nodes[nodeCount-1].position);
		_maxdist+=_nodes[nodeCount-1].dist;
	}
}


// called after all _nodes added. This function calculates the node velocities
void CCubicSpline4D::_BuildSpline()
{
	int nodeCount=_nodes.size();
	if (nodeCount<=1)
		return;
	for (int i = 1; i<nodeCount-1; i++)
	{
		// split the angle (figure 4)
		_nodes[i].velocity = Vector4d_normalize(_nodes[i+1].position - _nodes[i].position) - 
			Vector4d_normalize(_nodes[i-1].position - _nodes[i].position);
		_nodes[i].velocity=Vector4d_normalize(_nodes[i].velocity);
	}
	// calculate start and end velocities
	_nodes[0].velocity = _GetStartVelocity(0);
	_nodes[nodeCount-1].velocity = _GetEndVelocity(nodeCount-1);

}


// spline access function. time is 0 -> 1
Vector4d CCubicSpline4D::GetPosition(float time)
{
	int i = 0;
	float t;
	if (TRUE)
	{
		float dist = time * _maxdist;
		float currentDistance = 0.f;
		while (currentDistance + _nodes[i].dist < dist
			&& i < _nodes.size())
		{
			currentDistance += _nodes[i].dist;
			i++;
		}
		t= dist - currentDistance;
		t /= _nodes[i].dist; // scale t in range 0 - 1
	}
	Vector4d startVel = _nodes[i].velocity * _nodes[i].dist;
	Vector4d endVel = _nodes[i+1].velocity * _nodes[i].dist;   
	return GetPositionOnCubic(_nodes[i].position, startVel,
		_nodes[i+1].position, endVel, t);
}

DWORD CCubicSpline4D::GetSamples(float distGap,Sample*samples)
{
	i_math::matrix44f hermite;
	hermite.set( 2.f,-2.f, 1.f, 1.f,
						-3.f, 3.f,-2.f,-1.f,
						0.f, 0.f, 1.f, 0.f,
						1.f, 0.f, 0.f, 0.f);

	i_math::matrix44f hermiteVelocity;
	hermiteVelocity.set( 0.0f,0.0f,0.0f,0.0f,
										6.0f,-6.0f,3.0f,3.0f,
										-6.0f,6.0f,-4.0f,-2.0f,
										0.0f,0.0f,1.0f,0.0f);
	i_math::matrix44f hg;
	i_math::matrix44f hvg;

	DWORD ret=0;
	if (distGap>_maxdist)
		distGap=_maxdist;

	float timeGap=distGap/_maxdist;

	int nodeCount=_nodes.size();

	if (nodeCount<=0)
		return ret;
	if (nodeCount==1)
	{
		samples[0].pos=_nodes[0].position;
		samples[0].vel.set(0,0,0,0);
		samples[0].t=0.0f;
		samples[0].iNode=0;
		samples[0].tLerp=0.0f;
		return 1;
	}

	float distCur=0.0f;
	float distNodeCur=0.0f;
	int iCurNode=0;
	BOOL bHGDirty=TRUE;
	Vector4d timeVector;
	while(distCur<_maxdist)
	{
		while(distCur>distNodeCur+_nodes[iCurNode].dist)
		{
			distNodeCur+=_nodes[iCurNode].dist;
			iCurNode++;
			bHGDirty=TRUE;
			if (iCurNode>=nodeCount-1)
				goto _out;
		}

		if (bHGDirty)
		{
			Vector4d &startPos=_nodes[iCurNode].position;
			Vector4d &endPos=_nodes[iCurNode+1].position;
			Vector4d startVel = _nodes[iCurNode].velocity * _nodes[iCurNode].dist;
			Vector4d endVel = _nodes[iCurNode+1].velocity * _nodes[iCurNode].dist;   
			hg.set(startPos.x,startPos.y,startPos.z,startPos.w,
						endPos.x,endPos.y,endPos.z,endPos.w,
						startVel.x,startVel.y,startVel.z,startVel.w,
						endVel.x,endVel.y,endVel.z,endVel.w);
			hvg=hg;
			hg=hermite*hg;
			hvg=hermiteVelocity*hvg;
			bHGDirty=FALSE;
		}

		float t=(distCur-distNodeCur)/_nodes[iCurNode].dist;

		//poistion
		timeVector.set(t*t*t, t*t, t, 1.0f);
		hg.transformVect(timeVector);
		samples[ret].pos=FORCE_TYPE(Vector4d,timeVector);

		//velocity
		timeVector.set(t*t*t, t*t, t, 1.0f);
		hvg.transformVect(timeVector);
		samples[ret].vel=FORCE_TYPE(Vector4d,timeVector);

		//lerp info
		samples[ret].t=timeGap*(float)ret;
		samples[ret].tLerp=t;
		samples[ret].iNode=iCurNode;

		ret++;

		distCur+=distGap;
	}

_out:

	samples[ret].pos=_nodes[nodeCount-1].position;
	samples[ret].vel=_nodes[nodeCount-1].velocity;
	samples[ret].t=1.0f;
	samples[ret].tLerp=0.0f;
	samples[ret].iNode=nodeCount-1;

	ret++;

	return ret;

}

DWORD CCubicSpline4D::GetSamplesByTime(float timeGap,Sample*samples)
{
	return GetSamples(timeGap*_maxdist,samples);
}

// spline access function. time is 0 -> 1
Vector4d CCubicSpline4D::GetVelocity(float time)
{
	int i = 0;
	float t;
	if (TRUE)
	{
		float dist = time * _maxdist;
		float currentDistance = 0.f;
		while (currentDistance + _nodes[i].dist < dist
			&& i < _nodes.size())
		{
			currentDistance += _nodes[i].dist;
			i++;
		}
		t= dist - currentDistance;
		t /= _nodes[i].dist; // scale t in range 0 - 1
	}
	Vector4d startVel = _nodes[i].velocity * _nodes[i].dist;
	Vector4d endVel = _nodes[i+1].velocity * _nodes[i].dist;   
	Vector4d vel=GetVelocityOnCubic(_nodes[i].position, startVel,
											_nodes[i+1].position, endVel, t);
	return vel;
}


// internal. Based on Equation 14 
Vector4d CCubicSpline4D::_GetStartVelocity(int index)
{
  Vector4d temp = (_nodes[index+1].position - _nodes[index].position)*3.0f/_nodes[index].dist;
  return (temp - _nodes[index+1].velocity)*0.5f;
}

// internal. Based on Equation 15 
Vector4d CCubicSpline4D::_GetEndVelocity(int index)
{
  Vector4d temp = (_nodes[index].position - _nodes[index-1].position)*3.0f/_nodes[index-1].dist;
  return (temp - _nodes[index-1].velocity)*0.5f;
}

float CCubicSpline4D::_AddEnds(BOOL bAuto)
{
	float dist;
	Node s,s2,e,e2;
	s=_nodes[0];
	s2=_nodes[1];
	e=_nodes[_nodes.size()-1];
	e2=_nodes[_nodes.size()-2];
	if ((e.dist0<0.0f)||bAuto)
		dist=Vector4d_getLength(s.position-e.position);
	else
		dist=e.dist0;

	_nodes[_nodes.size()-1].dist=dist;
	e.dist=dist;

	_nodes.insert(_nodes.begin(),e);
	_nodes.insert(_nodes.begin(),e2);
	_nodes.push_back(s);
	_nodes.push_back(s2);

	float ret=_maxdist+dist;
	_maxdist+=dist*2.0f+s.dist+e2.dist;

	return ret;
}

void CCubicSpline4D::_RemoveEnds(float dist)
{
	_nodes.pop_back();
	_nodes.erase(_nodes.begin());
	_nodes.erase(_nodes.begin());
	_maxdist=dist;
}


void CCubicSpline4D::BuildSNS(BOOL bCircular)
{ 
	_CalcDistance(TRUE);
	if(_nodes.size()<=1)
		bCircular=FALSE;
	float dist;
	if (bCircular)
		dist=_AddEnds(TRUE);

	CCubicSpline4D::_BuildSpline(); 

	_SmoothSNS(); 
	_SmoothSNS(); 
	_SmoothSNS(); 

	if (bCircular)
		_RemoveEnds(dist);
}

// smoothing filter.
void CCubicSpline4D::_SmoothSNS()
{
	int nodeCount=_nodes.size();
	if (nodeCount<=1)
		return;
	Vector4d newVel;
	Vector4d oldVel = _GetStartVelocity(0);
	for (int i = 1; i<nodeCount-1; i++)
	{
		// Equation 12
		newVel = _GetEndVelocity(i)*_nodes[i].dist +
			_GetStartVelocity(i)*_nodes[i-1].dist;
		newVel /= (_nodes[i-1].dist + _nodes[i].dist);
		_nodes[i-1].velocity = oldVel;
		oldVel = newVel;
	}
	_nodes[nodeCount-1].velocity = _GetEndVelocity(nodeCount-1);
	_nodes[nodeCount-2].velocity = oldVel;
}

void CCubicSpline4D::BuildTNS(BOOL bCircular)
{
	_CalcDistance(FALSE);
	if(_nodes.size()<=1)
		bCircular=FALSE;
	float dist;
	if (bCircular)
		dist=_AddEnds(FALSE);

	CCubicSpline4D::_BuildSpline(); 
	_SmoothTNS(); 
	_SmoothTNS(); 
	_SmoothTNS();

	if (bCircular)
		_RemoveEnds(dist);

}


// stabilised version of TNS
void CCubicSpline4D::_ConstrainTNS()
{
	int nodeCount=_nodes.size();
	for (int i = 1; i<nodeCount-1; i++)
	{
		// Equation 13
		float r0 = Vector4d_getLength(_nodes[i].position-_nodes[i-1].position)/_nodes[i-1].dist;
		float r1 = Vector4d_getLength(_nodes[i+1].position-_nodes[i].position)/_nodes[i].dist;
		_nodes[i].velocity *= 4.f*r0*r1/((r0+r1)*(r0+r1));
	}
}

