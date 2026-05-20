
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "CubicSpline.h"

#include "assert.h"

#include "datapacket/BitPacket.h"


//本文件的技术来自于以下文章:
//Game Programming Gems 4 2.4节 非均匀样条
//Game Programming Gems 2 2.6节 平滑的基于四元数的C2飞行路径

#define PHANTOM_NODE_COUNT 4

// cubic curve defined by 2 positions and 2 velocities
i_math::vector3df GetPositionOnCubic(const i_math::vector3df &startPos, const i_math::vector3df &startVel, const i_math::vector3df &endPos, const i_math::vector3df &endVel, float time)
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
	m.set(startPos.x,startPos.y,startPos.z,1,
		endPos.x,endPos.y,endPos.z,1,
		startVel.x,startVel.y,startVel.z,0,
		endVel.x,endVel.y,endVel.z,0);

	m=hermite*m;

	i_math::vector4df timeVector;
	timeVector.set(time*time*time, time*time, time, 1.0f);
	m.transformVect(timeVector);
	timeVector.x/=timeVector.w;
	timeVector.y/=timeVector.w;
	timeVector.z/=timeVector.w;
	return FORCE_TYPE(i_math::vector3df,timeVector);
}

i_math::vector3df GetVelocityOnCubic(const i_math::vector3df &startPos, const i_math::vector3df &startVel, const i_math::vector3df &endPos, const i_math::vector3df &endVel, float time)
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

	i_math::vector4df timeVector;
	timeVector.set(time*time*time, time*time, time, 1.0f);
	m.transformVect(timeVector);
// 	timeVector.x/=timeVector.w;
// 	timeVector.y/=timeVector.w;
// 	timeVector.z/=timeVector.w;
	return FORCE_TYPE(i_math::vector3df,timeVector);
}


void CCubicSpline::Reset(BOOL bCircular)
{
	_nodes.clear();
	_rotR4.clear();
	_rotR4D2.clear();
	_state=NotBuilt;
	_maxdist=0.0f;
	_bCircular = bCircular;
}

void CCubicSpline::Write(CBitPacket& bp)
{
	// 写入节点数量
	DWORD nodeCount = _nodes.size();
	bp.Data_WriteSimple(nodeCount);

	// 写入每个节点的数据
	for (DWORD i = 0; i < nodeCount; ++i)
	{
		const Node& node = _nodes[i];
		bp.Data_WriteSimple(node.position);
		bp.Data_WriteSimple(node.velocity);
		bp.Data_WriteSimple(node.dist0);
		bp.Data_WriteSimple(node.dist);
		bp.Data_WriteSimple(node.distTotal);
		bp.Data_WriteSimple(node.rot);
	}

	// 写入其他成员变量
	bp.Data_WriteSimple(_maxdist);
	bp.Data_WriteSimple(_bCircular);
	bp.Data_WriteSimple(_state);

	// 写入旋转数据
	DWORD rotCount = _rotR4.size();
	bp.Data_WriteSimple(rotCount);
	for (DWORD i = 0; i < rotCount; ++i)
	{
		bp.Data_WriteSimple(_rotR4[i]);
		bp.Data_WriteSimple(_rotR4D2[i]);
	}
}

void CCubicSpline::Read(CBitPacket& bp)
{
	// 读取节点数量
	DWORD nodeCount;
	bp.Data_ReadSimple(nodeCount);
	_nodes.resize(nodeCount);

	// 读取每个节点的数据
	for (DWORD i = 0; i < nodeCount; ++i)
	{
		Node& node = _nodes[i];
		bp.Data_ReadSimple(node.position);
		bp.Data_ReadSimple(node.velocity);
		bp.Data_ReadSimple(node.dist0);
		bp.Data_ReadSimple(node.dist);
		bp.Data_ReadSimple(node.distTotal);
		bp.Data_ReadSimple(node.rot);
	}

	// 读取其他成员变量
	bp.Data_ReadSimple(_maxdist);
	bp.Data_ReadSimple(_bCircular);
	bp.Data_ReadSimple(_state);

	// 读取旋转数据
	DWORD rotCount;
	bp.Data_ReadSimple(rotCount);
	_rotR4.resize(rotCount);
	_rotR4D2.resize(rotCount);
	for (DWORD i = 0; i < rotCount; ++i)
	{
		bp.Data_ReadSimple(_rotR4[i]);
		bp.Data_ReadSimple(_rotR4D2[i]);
	}
}

void CCubicSpline::AddNode(const i_math::vector2df &pos,const i_math::quatf & rot)
{
	i_math::vector3df pos3D;
	pos3D.setXZ(pos);
	pos3D.y=0.0f;
	AddNode(pos3D,rot);
}

// adds node and updates segment length
void CCubicSpline::AddNode(const i_math::vector3df &pos,const i_math::quatf &rot)
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
	t.rot = rot;
	_nodes.push_back(t);
	_state = NotBuilt;
}

// Use timePeriod in place of actual node spacing
// ie time period is time from last node to this node
void CCubicSpline::AddNode(const i_math::vector3df &pos,const i_math::quatf & rot, float timePeriod)
{
	Node t;
	t.position=pos;
	t.dist0=timePeriod;
	t.dist=-1.0f;
	t.rot = rot;
	_nodes.push_back(t);
	_state = NotBuilt; 
}

void CCubicSpline::_CalcDistance(BOOL bAuto)
{
	int nodeCount=_nodes.size();
	if (nodeCount>1)
	{
		for(int i=0;i<nodeCount-1;i++)
		{
			if ((_nodes[i].dist0<0.0f)||bAuto)
				_nodes[i].dist=(float)(_nodes[i+1].position-_nodes[i].position).getLength();
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

//添加节点使变成一个circle,如果_bCircular为TRUE的话
void CCubicSpline::_MakeCircular(BOOL bAuto)
{
	if ((_state!=NotBuilt)||(!_bCircular))
		return;
	if (_nodes.size()<=1)
	{
		_bCircular=FALSE;
		return;//太少的控制点,不能circular
	}

	Node s=_nodes[0];

	_nodes.push_back(s);


}





// called after all _nodes added. This function calculates the node velocities
void CCubicSpline::_BuildSpline()
{
	int nodeCount=_nodes.size();
	if (nodeCount<=1)
		return;
	for (int i = 1; i<nodeCount-1; i++)
	{
		// split the angle (figure 4)
		_nodes[i].velocity = (_nodes[i+1].position - _nodes[i].position).normalize() - 
			(_nodes[i-1].position - _nodes[i].position).normalize();
		_nodes[i].velocity.normalize();
	}
	// calculate start and end velocities
	_nodes[0].velocity = _GetStartVelocity(0);
	_nodes[nodeCount-1].velocity = _GetEndVelocity(nodeCount-1);
}

void CCubicSpline::_BuildQD()
{
	//没有节点
	if(_nodes.size()<2)
		return;

	//清除旋转所需要的数据
	if(TRUE){
		_rotR4.clear();
		_rotR4D2.clear();
	}

	//检查旋转的有效性
	for(int i = 0;i<_nodes.size();i++){
		i_math::quatf & q = _nodes[i].rot;
		float s = 1.0f - q.W;
		if(s<0.02f)
			q.fromAngleAxis(0.1f,i_math::vector3df(0,1.0f,0));	
	}

	//选择性求负操作
	std::vector<i_math::quatf> rots;
	rots.resize(_nodes.size());
	rots[0] = _nodes[0].rot;

	for(int i = 1;i<_nodes.size();i++){
		rots[i] = _nodes[i].rot;

		float dot = rots[i].getDotProduct(rots[i-1]);
		if(dot<0){
			rots[i].X = - rots[i].X;
			rots[i].Y = - rots[i].Y;
			rots[i].Z = - rots[i].Z;
			rots[i].W = - rots[i].W;
		}
	}
	

	_rotR4.resize(_nodes.size());
	_rotR4D2.resize(_nodes.size());

	//将S3 转化为 R4
	for (int i=0;i<_nodes.size();i++)
	{
		float tmpf = 1.0f /(float)(sqrtf(2.0f*(1.0f - rots[i].W)));
		_rotR4[i].x = rots[i].X * tmpf;
		_rotR4[i].y = rots[i].Y * tmpf;
		_rotR4[i].z = rots[i].Z * tmpf;
		_rotR4[i].w = (1.0f - rots[i].W) * tmpf;
	}

	std::vector<R4> rotR4IntD2(_nodes.size());
	_rotR4D2[0].set(0,0,0,0); //[0]
	rotR4IntD2[0].set(0,0,0,0);

	//初始化x 坐标
	std::vector<float> xa(_nodes.size());
	float t = 0;
	for(int i = 0;i<_nodes.size();i++){
		xa[i] = t;
		t += _nodes[i].dist;//1.0f;//
	}


	//中值定理求出二阶导【1,n-1】
	R4 * y2 = &(_rotR4D2[0]);
	R4 * u = &(rotR4IntD2[0]);
	R4 * y = &(_rotR4[0]);

	if(TRUE){
		//第一个元素
		y2[0].set(0,0,0,0);
		u[0].set(0,0,0,0);

		for (int i=1; i<_nodes.size()-1; i++){
			float sig, p;
			sig = (xa[i]-xa[i-1]) / (xa[i+1] - xa[i-1]);
			for(int k = 0;k<4;k++){
				p = sig*y2[i-1].v[k] + 2.0f;
				y2[i].v[k] = (sig-1.0f)/p;
				
				float d2 = xa[i+1]-xa[i-1];
				float d1 = xa[i+1] - xa[i];
				float d0 = xa[i] - xa[i-1];
				float v1 = y[i+1].v[k]-y[i].v[k];
				float v0 = y[i].v[k]-y[i-1].v[k];
				float r = v1/d1 - v0/d0;

				u[i].v[k] = 6.0f*r/d2 - sig*(u[i-1].v[k]);
				u[i].v[k] /= p;
			}
		}

		//最后一个元素
		int n = _nodes.size()-1; 
		y2[n].set(0,0,0,0);
		u[n].set(0,0,0,0);
	}
	

	//计算得到二阶倒数
	for (int i = _nodes.size()-2;i>=1;i--){
		for(int k = 0;k<4;k++)
			y2[i].v[k] = y2[i].v[k]*y2[i+1].v[k] + u[i].v[k];
	}

}

i_math::quatf CCubicSpline::R4Lerp(R4 &r40,R4 &r41,R4 &r4d0,R4 &r4d1,float h,float t)
{
	float a = 1.0f - t;
	float b = t;
	R4 tmpR4;
	i_math::quatf ret;
	
	float c,d;
	c = (a*a*a-a)*(h*h)/6.0f;
	d = (b*b*b-b)*(h*h)/6.0f;
	for(int k = 0;k<4;k++){
		//Y = A*y0 + B*y1 + C*y20 + D*y21;
		tmpR4.v[k] = r40.v[k]*a + r41.v[k]*b + r4d0.v[k]*c + r4d1.v[k]*d;
	}
	
	//R4 space -> S3 space
//	if(FALSE){
//		float tmpf = 1.0f /(tmpR4.v[0]*tmpR4.v[0]+tmpR4.v[1]*tmpR4.v[1]+tmpR4.v[2]*tmpR4.v[2]+tmpR4.v[3]*tmpR4.v[3]);
//		ret.X = (2.0f*tmpR4.v[0]*tmpR4.v[3]) * tmpf;
//		ret.Y = (2.0f*tmpR4.v[1]*tmpR4.v[3]) * tmpf;
//		ret.Z = (2.0f*tmpR4.v[2]*tmpR4.v[3]) * tmpf;
//		ret.W = (tmpR4.v[0]*tmpR4.v[0]+tmpR4.v[1]*tmpR4.v[1]+tmpR4.v[2]*tmpR4.v[2]-tmpR4.v[3]*tmpR4.v[3])*tmpf;
//	}

	//R4 space -> S3 space
	if(TRUE){
		float w = (tmpR4.w>0)?tmpR4.w:-tmpR4.w;
		ret.X = 2.0f*w*tmpR4.x;
		ret.Y = 2.0f*w*tmpR4.y;
		ret.Z = 2.0f*w*tmpR4.z;
		ret.W = 1.0f - 2.0f*w*w;
	}

	ret.normalize();

	return ret;
}

DWORD CCubicSpline::GetNodeCount()
{
	return _nodes.size();
}
CCubicSpline::Node * CCubicSpline::GetNode(DWORD idx)
{
	return &_nodes[idx];
}

DWORD CCubicSpline::GetSamples(float distGap,Sample*samples,float distStart,float distEnd)
{
	//没有生成插值数据返回
	if(_state==NotBuilt)
		return 0;

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
		samples[0].vel.set(0,0,0);
		samples[0].iNode=0;
		samples[0].tLerp=0.0f;
		samples[0].rot = _nodes[0].rot;
		return 1;
	}

	float distCur=distStart;
	float distNodeCur=0.0f;
	int iCurNode=0;
	int iEnd = nodeCount - 1;

	BOOL bHGDirty=TRUE;
	i_math::vector4df timeVector;
	i_math::quatf ctrpA,ctrpB,q0,q1;

	R4 r40,r41,r4d0,r4d1;
	float h;
	
	BOOL bReachEnd=FALSE;
	while(distCur<_maxdist)
	{
		//找到节点的位置
		while(distCur>distNodeCur+_nodes[iCurNode].dist)
		{
			distNodeCur+=_nodes[iCurNode].dist;
			iCurNode++;
//			distCur=distNodeCur;
			bHGDirty=TRUE;
			if (iCurNode>=iEnd)
				break;
		}
		
		//得到调和函数
		if (bHGDirty)
		{
			i_math::vector3df &startPos=_nodes[iCurNode].position;
			i_math::vector3df &endPos=_nodes[iCurNode+1].position;
			i_math::vector3df startVel = _nodes[iCurNode].velocity * _nodes[iCurNode].dist;
			i_math::vector3df endVel = _nodes[iCurNode+1].velocity * _nodes[iCurNode].dist;   
			hg.set(startPos.x,startPos.y,startPos.z,1,
						endPos.x,endPos.y,endPos.z,1,
						startVel.x,startVel.y,startVel.z,0,
						endVel.x,endVel.y,endVel.z,0);
			hvg=hg;
			hg=hermite*hg;
			hvg=hermiteVelocity*hvg;
			
			//计算旋转控制点
			if (_state==RotBuilt)
			{
				r40 = _rotR4[iCurNode];
				r41 = _rotR4[iCurNode+1];
				r4d0 = _rotR4D2[iCurNode];
				r4d1 = _rotR4D2[iCurNode+1];
				h = _nodes[iCurNode].dist;
			}

			bHGDirty=FALSE;
		}
		
		//得到局部函数的时间
		float t=0.0f;
		if (!i_math::equals(_nodes[iCurNode].dist,0.0f))
			t=(distCur-distNodeCur)/_nodes[iCurNode].dist;
		
		//poistion
		timeVector.set(t*t*t, t*t, t, 1.0f);
		hg.transformVect(timeVector);
		timeVector.x/=timeVector.w;
		timeVector.y/=timeVector.w;
		timeVector.z/=timeVector.w;
		samples[ret].pos=FORCE_TYPE(i_math::vector3df,timeVector);

		//rot
		if (_state==RotBuilt)
			samples[ret].rot =  R4Lerp(r40,r41,r4d0,r4d1,h,t);//squad(q0,q1,ctrpA,ctrpB,t);//

		//velocity
		timeVector.set(t*t*t, t*t, t, 1.0f);
		hvg.transformVect(timeVector);
		samples[ret].vel=FORCE_TYPE(i_math::vector3df,timeVector);

		//lerp info
		samples[ret].tLerp=t;
		samples[ret].iNode=iCurNode;

		ret++;

		if (distEnd>=0.0f)
		{
			if (distCur+distGap>distEnd)
			{
				distGap=distEnd-distCur;
				if (distGap<=0.01f)
				{
					bReachEnd=TRUE;
					break;
				}
			}
		}

		distCur+=distGap;

	}
	
	if (!bReachEnd)
	{
		samples[ret].pos = _nodes[iEnd].position;
		samples[ret].vel = _nodes[iEnd].velocity;
		samples[ret].tLerp = 0.0f;
		samples[ret].iNode = iEnd;
		samples[ret].rot = _nodes[iEnd].rot;

		ret++;
	}

	return ret;

}

DWORD CCubicSpline::GetSamplesByTime(float timeGap,Sample*samples)
{
	return GetSamples(timeGap*_maxdist,samples);
}

BOOL CCubicSpline::GetSampleByDist(Sample &sample,float dist)
{
	if (GetSamples(_maxdist,&sample,dist,dist)>0)
		return TRUE;
	return FALSE;
}


DWORD CCubicSpline::GetCurNode(float time)
{
	int i = 0;
	if (TRUE)
	{
		float dist = time * _maxdist;
		float currentDistance = 0.f;
		while (currentDistance + _nodes[i].dist < dist
			&& i < _nodes.size()-2)
		{
			currentDistance += _nodes[i].dist;
			i++;
		}
	}
	return i;
}

i_math::vector3df CCubicSpline::GetPositionByDist(float dist)
{
	if(_maxdist>0.0f)
		return GetPosition(dist/_maxdist);
	return GetPosition(0.0f);
}

i_math::vector3df CCubicSpline::GetVelocityByDist(float dist)
{
	if(_maxdist>0.0f)
		return GetVelocity(dist/_maxdist);
	return GetVelocity(0.0f);
}


// spline access function. time is 0 -> 1
i_math::vector3df CCubicSpline::GetPosition(float time)
{
	time=i_math::clamp_f(time,0.0f,1.0f);
	int i = 0;
	float t;
	if (TRUE)
	{
		float dist = time * _maxdist;
		float currentDistance = 0.f;
		while (currentDistance + _nodes[i].dist < dist
			&& i < _nodes.size()-2)
		{
			currentDistance += _nodes[i].dist;
			i++;
		}
		t= dist - currentDistance;
		t /= _nodes[i].dist; // scale t in range 0 - 1
		if (t>1.0f)
			t=1.0f;
	}
	i_math::vector3df startVel = _nodes[i].velocity * _nodes[i].dist;
	i_math::vector3df endVel = _nodes[i+1].velocity * _nodes[i].dist;   

	return GetPositionOnCubic(_nodes[i].position, startVel,
		_nodes[i+1].position, endVel, t);
}


// spline access function. time is 0 -> 1
i_math::vector3df CCubicSpline::GetVelocity(float time)
{
	int i = 0;
	float t;
	if (TRUE)
	{
		float dist = time * _maxdist;
		float currentDistance = 0.f;
		while (currentDistance + _nodes[i].dist < dist
			&& i < _nodes.size()-2)
		{
			currentDistance += _nodes[i].dist;
			i++;
		}
		t= dist - currentDistance;
		t /= _nodes[i].dist; // scale t in range 0 - 1
		if (t>1.0f)
			t=1.0f;
	}
// 	if (i>=_nodes.size()-1)
// 		return _nodes[i].velocity;
	i_math::vector3df startVel = _nodes[i].velocity * _nodes[i].dist;
	i_math::vector3df endVel = _nodes[i+1].velocity * _nodes[i].dist;   
	i_math::vector3df vel=GetVelocityOnCubic(_nodes[i].position, startVel,
											_nodes[i+1].position, endVel, t);
	return vel;
}


// internal. Based on Equation 14 
i_math::vector3df CCubicSpline::_GetStartVelocity(int index)
{
  i_math::vector3df temp = 3.f*(_nodes[index+1].position - _nodes[index].position)/_nodes[index].dist;
  return (temp - _nodes[index+1].velocity)*0.5f;
}

// internal. Based on Equation 15 
i_math::vector3df CCubicSpline::_GetEndVelocity(int index)
{
  i_math::vector3df temp = 3.f*(_nodes[index].position - _nodes[index-1].position)/_nodes[index-1].dist;
  return (temp - _nodes[index-1].velocity)*0.5f;
}

int CCubicSpline::_GetModIdx(int idx)
{
	if (idx>0)
		idx%=(_nodes.size()-1);
	else
		idx=(_nodes.size()-1)-(-idx)%_nodes.size();
	return idx;
}



float CCubicSpline::_AddEnds()
{
	if (!_bCircular)
		return _maxdist;

	Node s=_nodes[0];
	Node s2=_nodes[1];
	Node e2=_nodes[_nodes.size()-2];
	Node e3=_nodes[_nodes.size()-3];


	float ret=_maxdist;
	_maxdist+=s.dist+e2.dist+e3.dist;

	//最后一个node在_CalcDistance()后,它的dist为0
	_nodes[_nodes.size()-1].dist=s.dist;

	//在末尾添加
	std::vector<Node>tail,head;
	int idx=1;
	for (int i=0;i<PHANTOM_NODE_COUNT;i++)
	{
		_maxdist+=_nodes[_GetModIdx(idx-1)].dist;
		tail.push_back(_nodes[_GetModIdx(idx)]);
		idx++;
	}

	//在头上插入
	idx=-1;
	for (int i=0;i<PHANTOM_NODE_COUNT;i++)
	{
		_maxdist+=_nodes[_GetModIdx(idx)].dist;
		head.insert(head.begin(),_nodes[_GetModIdx(idx)]);
		idx--;
	}

	VEC_APPEND(head,_nodes);
	VEC_APPEND(head,tail);
	_nodes.swap(head);


	return ret;
}

void CCubicSpline::_RemoveEnds(float dist)
{
	if (!_bCircular)
		return;

	for(int i=0;i<4;i++)
		_nodes.pop_back();

	for (int i=0;i<4;i++)
		_nodes.erase(_nodes.begin());
	_maxdist=dist;
}

BOOL CCubicSpline::BuildRNS()
{
	if (_state!=NotBuilt)
		return FALSE;

	_MakeCircular(TRUE);

	_CalcDistance(TRUE);

	float dist=_AddEnds();

	_BuildSpline();

	_RemoveEnds(dist);

	_state=XlateBuilt;

	return TRUE;
}



BOOL CCubicSpline::BuildSNS()
{ 
	//已经创建了数据
	if (_state==XlateBuilt)
		return FALSE;

	_MakeCircular(TRUE);
	
	_CalcDistance(TRUE);
	
	float dist=_AddEnds();

	CCubicSpline::_BuildSpline(); 

	_SmoothSNS(); 
	_SmoothSNS(); 
	_SmoothSNS();
	
	_RemoveEnds(dist);

	_state = XlateBuilt;

	return TRUE;
}

// smoothing filter.
void CCubicSpline::_SmoothSNS()
{
	int nodeCount=_nodes.size();
	if (nodeCount<=1)
		return;
	i_math::vector3df newVel;
	i_math::vector3df oldVel = _GetStartVelocity(0);
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

BOOL CCubicSpline::BuildTNS()
{
	if (_state!=NotBuilt)
		return FALSE;
	_MakeCircular(FALSE);


	_CalcDistance(FALSE);
	
	float dist=_AddEnds();

	CCubicSpline::_BuildSpline(); 
	_SmoothTNS(); 
	_SmoothTNS(); 
	_SmoothTNS();

	_RemoveEnds(dist);

	_state=XlateBuilt;

	return TRUE;
}

// stabilised version of TNS
void CCubicSpline::_ConstrainTNS()
{
	int nodeCount=_nodes.size();
	for (int i = 1; i<nodeCount-1; i++){
		// Equation 13
		float r0 = (float)(_nodes[i].position-_nodes[i-1].position).getLength()/_nodes[i-1].dist;
		float r1 = (float)(_nodes[i+1].position-_nodes[i].position).getLength()/_nodes[i].dist;
		_nodes[i].velocity *= 4.f*r0*r1/((r0+r1)*(r0+r1));
	}
}

BOOL CCubicSpline::BuildRot()
{
	//在没有创建位置数据之前 或 已经创建了旋转数据时返回
	if (_state!=XlateBuilt)
		return FALSE;

	float dist=_AddEnds();

	_BuildQD();

	_RemoveEnds(dist);
	
	if(_bCircular)
	{
		for (int i=0;i<PHANTOM_NODE_COUNT;i++)
		{
			_rotR4.pop_back();
			_rotR4D2.pop_back();
		}

		for (int i=0;i<PHANTOM_NODE_COUNT;i++)
		{
			_rotR4.erase(_rotR4.begin());
			_rotR4D2.erase(_rotR4D2.begin());
		}
	}

	_state=RotBuilt;
	
	return TRUE;
}




