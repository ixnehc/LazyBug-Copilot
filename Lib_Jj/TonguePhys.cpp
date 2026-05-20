/********************************************************************
	created:	2020/12/27
	author:		cxi
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include "unitmgr/UnitMgr.h"
#include "timer/profiler.h"
#include "TonguePhys.h"


void CTonguePhysEnv::Init(PBD::SimulationModel * model)
{
	_model=model;

	_bAllowInterParticleCollision=FALSE;
}

void CTonguePhysEnv::Clear()
{
	_cells.Reset();
	_entries.clear();
	_obstacles.clear();

	Zero();
}

TongueParticleHandle CTonguePhysEnv::AddParticle(i_math::vector3df &pos,float radius,DWORD collision)
{
	if (!_model)
		return TongueParticleHandle_Invalid;

	PBD::ParticleData &pd=_model->getParticles();

	i_math::pos2di ptCell=_ToCellCoord(pos.x,pos.z);

	Entry e;
	e.particle=pd.addVertex(pos);
	e.radius=radius;
	e.collision=collision;
	e.ptCell=ptCell;

	Entry *pe=&_entries[e.particle];
	*pe=e;

	Cell *cell=_cells.Obtain(ptCell.x,ptCell.y);
	if (cell)
		cell->Add(pe);

	return e.particle;
}

TongueParticleHandle CTonguePhysEnv::AddObstacle(i_math::vector3df &pos,float radius)
{
	TongueParticleHandle particle=AddParticle(pos,radius,TongueParticleCollisionGroup_Obstacle);
	if (particle!=TongueParticleHandle_Invalid)
	{
		std::unordered_map<TongueParticleHandle,Entry>::iterator it=_entries.find(particle);
		if (it!=_entries.end())
			_obstacles.push_back(&(*it).second);

		PBD::ParticleData &pd=_model->getParticles();

		pd.setMass(particle,0.0f);
	}

	return particle;
}

void CTonguePhysEnv::RemoveParticle(TongueParticleHandle particle)
{
	if (!_model)
		return;

	PBD::ParticleData &pd=_model->getParticles();

	std::unordered_map<TongueParticleHandle,Entry>::iterator it=_entries.find(particle);
	if (it!=_entries.end())
	{
		Entry &e=(*it).second;

		Cell *cell=_cells.Get(e.ptCell.x,e.ptCell.y);
		if (cell)
			cell->Remove(&e);

		_entries.erase(it);

		pd.removeVertex(particle);
	}

}

void CTonguePhysEnv::RemoveObstacle(TongueParticleHandle particle)
{
	std::unordered_map<TongueParticleHandle,Entry>::iterator it=_entries.find(particle);
	if (it!=_entries.end())
	{
		Entry *e=&(*it).second;
		VEC_REMOVE_SWAP(_obstacles,e);
	}

	RemoveParticle(particle);

}

void CTonguePhysEnv::SetParticleCollision(TongueParticleHandle particle,TongueParticleCollisionGroup collision)
{
	std::unordered_map<TongueParticleHandle,Entry>::iterator it=_entries.find(particle);
	if (it!=_entries.end())
	{
		Entry *e=&(*it).second;
		e->collision=collision;
	}

}


void CTonguePhysEnv::_GenContacts()
{

	_model->resetContacts();

	if (_bAllowInterParticleCollision)
	{
		std::unordered_map<TongueParticleHandle,Entry>::iterator it;
		for (it=_entries.begin();it!=_entries.end();it++)
		{
			Entry &e=(*it).second;

			if (e.collision==TongueParticleCollisionGroup_None)
				continue;

			i_math::pos2di offs[]={ 
				i_math::pos2di(-1,-1),i_math::pos2di(0,-1),i_math::pos2di(1,-1),
				i_math::pos2di(-1,0),i_math::pos2di(0,0),i_math::pos2di(1,0),
				i_math::pos2di(-1,1),i_math::pos2di(0,1),i_math::pos2di(1,1)
			};

			for (int i=0;i<ARRAY_SIZE(offs);i++)
			{
				i_math::pos2di pt=e.ptCell+offs[i];

				Cell *cell=_cells.Get(pt.x,pt.y);
				if (!cell)
					continue;

				std::unordered_set<void *>::iterator it;
				for (it=cell->entries.begin();it!=cell->entries.end();it++)
				{
					Entry *e2=(Entry*)(*it);

					if (e2==&e)
						continue;

					if (e2->collision==e.collision)
						continue;

					_model->addSimpleContactConstraint(e.particle,e2->particle,e.radius+e2->radius);
				}
			}
		}
	}
	else
	{
		PBD::ParticleData &pd=_model->getParticles();

		std::unordered_map<TongueParticleHandle,Entry>::iterator it;
		for (it=_entries.begin();it!=_entries.end();it++)
		{
			Entry &e=(*it).second;

			if (e.collision==TongueParticleCollisionGroup_None)
				continue;
			if (e.collision==TongueParticleCollisionGroup_Obstacle)
				continue;

			i_math::vector3df &pos=pd.getPosition(e.particle);

			for (int i=0;i<_obstacles.size();i++)
			{
				Entry *e2=_obstacles[i];

				i_math::vector3df &pos2=pd.getPosition(e2->particle);

				float r=e.radius+e2->radius+2.0f;

				if (pos.getDistanceFromSQ(pos2)<r*r)
					_model->addSimpleContactConstraint(e.particle,e2->particle,e.radius+e2->radius);
			}
		}

	}
}


void CTonguePhysEnv::Update()
{
	if (!_model)
		return;

	if (_bAllowInterParticleCollision)
	{
		PBD::ParticleData &pd=_model->getParticles();

		const i_math::pos2di ptInvalid=i_math::pos2di(-10000,-10000);

		std::unordered_map<TongueParticleHandle,Entry>::iterator it;
		for (it=_entries.begin();it!=_entries.end();it++)
		{
			Entry &e=(*it).second;

			i_math::vector3df const& pos=pd.getPosition(e.particle);

			i_math::pos2di ptCellNew=ptInvalid;

			if (e.collision!=TongueParticleCollisionGroup_None)
				ptCellNew=_ToCellCoord(pos.x,pos.z);

			if(ptCellNew!=e.ptCell)
			{
				if (e.ptCell!=ptInvalid)
				{
					Cell *cell=_cells.Get(e.ptCell.x,e.ptCell.y);
					if (cell)
						cell->Remove(&e);
				}

				if (ptCellNew!=ptInvalid)
				{
					Cell *cell=_cells.Obtain(ptCellNew.x,ptCellNew.y);
					if (cell)
						cell->Add(&e);
				}

				e.ptCell=ptCellNew;
			}
		}
	}

	_GenContacts();
}



//////////////////////////////////////////////////////////////////////////
//CTongueBranchPhys
float g_stepSize=0.002f;
float g_stiffnessDistance=0.8f;
float g_stiffnessPull=0.0001f;
int g_nNodes=21;//这个数字与tongue的骨架的最大序号Bone的数值一样(目前为Bone21)
float g_dampVel=0.5f;
float g_bendToleranceMin=10.0f;
float g_bendToleranceMax=30.0f;
float g_speed=5.0f;
float g_radiusNode=0.5f;//要与CGameBuff_TongueFly::SyncBigData(..)中的radiusNode保持一致
float g_radiusObstacle=2.5f;

void CTongueBranchPhys::Init(CTonguePhysEnv &env,DWORD nNodes,float radiusNode,i_math::vector3df &posRoot,i_math::vector3df &dirRoot)
{
	_env=&env;
	PBD::SimulationModel *model=_env->GetSimModel();
	PBD::ParticleData &pd=model->getParticles();

	_posRoot=posRoot;
	_dirRoot=dirRoot;

	_radiusNode=radiusNode;

	_nodes.resize(nNodes);

	_collision=_env->NewCollisionGroup();

	for (int i=0;i<_nodes.size();i++)
	{
		Node &node=_nodes[i];
		node.particle=_env->AddParticle(posRoot,_radiusNode,_collision);
	}

	for (int i=1;i<_nodes.size();i++)
	{
		Node &node=_nodes[i];
		Node &nodePrev=_nodes[i-1];

		if (i==1)
			node.constraintPulled=(TongueConstraintHandle)model->addPullConstraint(nodePrev.particle,node.particle,GetNodeDist(),g_stiffnessPull,1.0f,-dirRoot);
		else
			node.constraintPulled=(TongueConstraintHandle)model->addPullConstraint(nodePrev.particle,node.particle,GetNodeDist()*2.0f,g_stiffnessPull,1.0f,-dirRoot);

		node.stage=Node::Stage_PulledIn;
	}

	for (int i=1;i<_nodes.size()-1;i++)
	{
		Node &node=_nodes[i];
		Node &nodePrev=_nodes[i-1];
		Node &nodeNext=_nodes[i+1];

		float bendTol=g_bendToleranceMin+(g_bendToleranceMax-g_bendToleranceMin)*(float)i/(float)(_nodes.size()-1);
		node.constraintBend=(TongueConstraintHandle)model->addBendConstraint(nodePrev.particle,node.particle,nodeNext.particle,dirRoot,bendTol,GetNodeDist());
	}

	pd.setMass(_nodes[0].particle,0.0f);
	_nodes[0].constraintBend=(TongueConstraintHandle)model->addBendConstraint(_nodes[0].particle,_nodes[0].particle,_nodes[1].particle,dirRoot,g_bendToleranceMin,GetNodeDist());

}

void CTongueBranchPhys::Clear()
{

	_nodes.clear();
}

int CTongueBranchPhys::_GetLastPulledIn()
{
	int iStart=1;
	for (int i=_nodes.size()-1;i>0;i--)
	{
		if (_nodes[i].stage==CTongueBranchPhys::Node::Stage_PulledIn)
		{
			iStart=i;
			break;
		}
	}
	return iStart;
}

void CTongueBranchPhys::_CalcEnd(PBD::SimulationModel &model,i_math::vector3df &velEnd,float &dEnd)
{
	PBD::ParticleData &pd=model.getParticles();

	dEnd=0.0f;
	velEnd.setZero();

	if (_nodes.size()>=2)
	{
		Node &node=_nodes[_nodes.size()-1];
		Node &nodePrev=_nodes[_nodes.size()-2];

		i_math::vector3df pos=pd.getPosition(node.particle);
		i_math::vector3df posPrev=pd.getPosition(nodePrev.particle);

		i_math::vector3df vel=_posTarget-pos;
		vel.setLength(g_speed);

		i_math::vector3df dir=pos-posPrev;
		dir.normalize();

		dEnd=dir.dotProduct(vel);
		velEnd=vel;
	}
}

void CTongueBranchPhys::_AverageVelocity(PBD::SimulationModel &model,int iStart)
{
	PBD::ParticleData &pd=model.getParticles();

	for (int i=iStart+1;i<_nodes.size()-1;i++)
	{
		Node &node=_nodes[i];
		Node &nodePrev=_nodes[i-1];
		Node &nodeNext=_nodes[i+1];

		node.velTemp=(pd.getVelocity(nodePrev.particle)+pd.getVelocity(nodeNext.particle))/2.0f;
		i_math::vector3df vel=pd.getVelocity(node.particle);
		node.velTemp.setLength(vel.getLength());
	}

	for (int i=iStart+1;i<_nodes.size()-1;i++)
	{
		Node &node=_nodes[i];

		pd.setVelocity(node.particle,node.velTemp);
	}

}

void CTongueBranchPhys::_DumpRecentVel(PBD::SimulationModel &model)
{
	PBD::ParticleData &pd=model.getParticles();

	for (int i=0;i<_nodes.size();i++)
	{
		Node &node=_nodes[i];

		node.velRecent=pd.getVelocity(node.particle);
	}
}



void CTongueBranchPhys::_UpdateBendTolerance(PBD::SimulationModel &model)
{
	PBD::SimulationModel::ConstraintVector &constraints=model.getConstraints();

	int iStart=_GetLastPulledIn();

	int c=_nodes.size()-iStart-1;
	for (int i=0;i<_nodes.size();i++)
	{
		Node &node=_nodes[i];

		float bendTol=g_bendToleranceMin;
		if (i>=iStart)
		{
			if (c>0)
				bendTol=g_bendToleranceMin+(g_bendToleranceMax-g_bendToleranceMin)*(float)(i-iStart)/(float)c;
		}

		if (node.constraintBend>=0)
		{
			PBD::BendConstraint *contraint=(PBD::BendConstraint *)constraints[node.constraintBend];
			if (contraint)
				contraint->SetToleranceAngle(bendTol);
		}
	}

}

BOOL CTongueBranchPhys::_CheckTargetPosObstacle(PBD::SimulationModel &model,int &nNodeObstacled,int &idxObstacle)
{
	nNodeObstacled=0;
	idxObstacle=-1;

	return FALSE;

	if (!_bTargetPos)
		return FALSE;


	PBD::ParticleData &pd=model.getParticles();

	for (int i=_nodes.size()-1;i>1;i--)
	{
		Node &node=_nodes[i];

		float distToObstacleMin=100000000.0f;

		for (int j=0;j<_env->_obstacles.size();j++)
		{
			CTonguePhysEnv::Entry *e=_env->_obstacles[j];

			i_math::vector2df dir;
			i_math::vector2df posNode=pd.getPosition(node.particle).getXZ();
			i_math::vector2df posObstacle=pd.getPosition(e->particle).getXZ();

			dir=_posTarget.getXZ()-posNode;
			float distToTarget=dir.getLength();
			distToTarget-=_radiusNode;
			dir.normalize();

			float distToObstacle=intersectSphereBySphere(posNode,_radiusNode,dir,posObstacle,e->radius);
			if (distToObstacle>=0.0f)
			{
				if(distToObstacle<distToTarget)
				{
					const float distTolerance=2.0f;
					if (distToObstacle<distTolerance)
					{
						nNodeObstacled=i+1;
						if (i<_nodes.size()-1)
						{
							//不是末尾的Node,只需要找一个足够近的就行了
							idxObstacle=j;
							return TRUE;
						}
						else
						{
							//末尾的Node,要找一个最近的
							if (distToObstacle<distToObstacleMin)
							{
								idxObstacle=j;
								distToObstacleMin=distToObstacle;
							}
						}
					}
				}
			}
		}

		if (idxObstacle>=0)
			return TRUE;
	}

	return FALSE;
}


void CTongueBranchPhys::_UpdateVelocityToTargetPos(PBD::SimulationModel &model)
{
	PBD::ParticleData &pd=model.getParticles();

	int iLastPulledIn=_GetLastPulledIn();

	int nObstacled=0;
	int idxObstacle;
	_CheckTargetPosObstacle(model,nObstacled,idxObstacle);

	float dEnd=-100000.0f;

	for (int i=_nodes.size()-1;i>0;i--)
	{
		Node &node=_nodes[i];
		Node &nodePrev=_nodes[i-1];

		if ((node.stage!=Node::Stage_PulledOut)&&(i!=_nodes.size()-1))
			break;

		i_math::vector3df pos=pd.getPosition(node.particle);
		i_math::vector3df posPrev=pd.getPosition(nodePrev.particle);

		//		float scaleSpeed=(float)i/(float)(_nodes.size()-1);
		float scaleSpeed=1.0f;
		if (_nodes.size()-1-iLastPulledIn>0)
			scaleSpeed=(float)(i-iLastPulledIn)/(float)(_nodes.size()-1-iLastPulledIn);
		i_math::vector3df vel;
		float speed=g_speed*scaleSpeed;

		BOOL bObstacled=FALSE;
		if (i<=nObstacled)
			bObstacled=TRUE;

		if (bObstacled)
		{
			if (i+1<_nodes.size())
			{
				Node &nodeNext=_nodes[i+1];

				i_math::vector3df posNext=pd.getPosition(nodeNext.particle);

				vel=posNext-pos;
				vel.setLength(speed);
			}
			else
			{
				vel=pos-posPrev;
				vel.setLength(speed);
			}
		}
		else
		{
			vel=_posTarget-pos;
			vel.setLength(speed);
		}

		if (i>=_nodes.size()-1)
		{
			i_math::vector3df dir=pos-posPrev;
			dir.normalize();

			dEnd=vel.dotProduct(dir);
		}
		else
		{
			Node &nodeNext=_nodes[i+1];

			i_math::vector3df posNext=pd.getPosition(nodeNext.particle);
			i_math::vector3df velNext=pd.getVelocity(nodeNext.particle);

			i_math::vector3df dirNext=posNext-pos;
			dirNext.normalize();

			i_math::vector3df dir=pos-posPrev;
			dir.normalize();

			float dNext=velNext.dotProduct(dirNext);
			float d=vel.dotProduct(dir);

			i_math::vector3df velSide=vel-dir*d;

			if (!bObstacled)
			{
				if (d>dNext)
					d=dNext;
			}
			else
				d=dEnd*scaleSpeed;

			vel=velSide+dir*d;
		}

		pd.setVelocity(node.particle,vel);
	}

	// 	_AverageVelocity(model,iLastPulledIn+1);
	// 	_AverageVelocity(model,iLastPulledIn+1);
	// 	_AverageVelocity(model,iLastPulledIn+1);
	// 	_AverageVelocity(model,iLastPulledIn+1);
	_DumpRecentVel(model);
}


void CTongueBranchPhys::_UpdateVelocityToTargetPos2(PBD::SimulationModel &model)
{
	PBD::ParticleData &pd=model.getParticles();

	int iLastPulledIn=_GetLastPulledIn();

	int nObstacled=0;
	int idxObstacle;
	_CheckTargetPosObstacle(model,nObstacled,idxObstacle);

	i_math::vector3df velEnd;
	float dEnd;
	_CalcEnd(model,velEnd,dEnd);

	for (int i=_nodes.size()-1;i>0;i--)
	{
		BOOL bEnd=(i==_nodes.size()-1);
		BOOL bObstacled=FALSE;
		if (i<=nObstacled)
			bObstacled=TRUE;

		Node &node=_nodes[i];
		Node &nodePrev=_nodes[i-1];
		Node &nodeNext=bEnd?node:_nodes[i+1];

		i_math::vector3df const& pos=pd.getPosition(node.particle);
		i_math::vector3df const& posPrev=pd.getPosition(nodePrev.particle);
		i_math::vector3df const& posNext=pd.getPosition(nodeNext.particle);

		if ((node.stage!=Node::Stage_PulledOut)&&(!bEnd))
			break;

		float ratioPulledOut=1.0f;
		if (_nodes.size()-1-iLastPulledIn>0)
			ratioPulledOut=(float)(i-iLastPulledIn)/(float)(_nodes.size()-1-iLastPulledIn);

		i_math::vector3df vel;

		if (bEnd)
			vel=velEnd;
		else
		{
			if (dEnd>0.0f)
			{
				vel=posNext-pos;
				vel.setLength(g_speed*ratioPulledOut);
			}
			else
			{
				vel=posPrev-pos;
				vel.setLength(g_speed*(1.0f-ratioPulledOut));
			}

		}

		// 		if (bObstacled)
		// 		{
		// 			if (i+1<_nodes.size())
		// 			{
		// 				Node &nodeNext=_nodes[i+1];
		// 
		// 				i_math::vector3df posNext=pd.getPosition(nodeNext.particle);
		// 
		// 				vel=posNext-pos;
		// 				vel.setLength(speed);
		// 			}
		// 			else
		// 			{
		// 				vel=pos-posPrev;
		// 				vel.setLength(speed);
		// 			}
		// 		}
		// 		else
		// 		{
		// 			vel=_posTarget-pos;
		// 			vel.setLength(speed);
		// 		}
		// 
		// 		if (!bEnd)
		// 		{
		// 			Node &nodeNext=_nodes[i+1];
		// 
		// 			i_math::vector3df posNext=pd.getPosition(nodeNext.particle);
		// 			i_math::vector3df velNext=pd.getVelocity(nodeNext.particle);
		// 
		// 			i_math::vector3df dirNext=posNext-pos;
		// 			dirNext.normalize();
		// 
		// 			i_math::vector3df dir=pos-posPrev;
		// 			dir.normalize();
		// 
		// 			float dNext=velNext.dotProduct(dirNext);
		// 			float d=vel.dotProduct(dir);
		// 
		// 			i_math::vector3df velSide=vel-dir*d;
		// 
		// 			if (!bObstacled)
		// 			{
		// 				if (d>dNext)
		// 					d=dNext;
		// 			}
		// 			else
		// 				d=dEnd*scaleSpeed;
		// 
		// 			vel=velSide+dir*d;
		// 		}

		pd.setVelocity(node.particle,vel);
		node.velRecent=vel;
	}
}

void CTongueBranchPhys::_UpdateVelocityToTargetPos3(PBD::SimulationModel &model)
{
	PBD::ParticleData &pd=model.getParticles();

	int iLastPulledIn=_GetLastPulledIn();

	float distTargetToRoot=_posRoot.getDistanceFrom(_posTarget);
	i_math::vector3df dirTargetToRoot=_posRoot-_posTarget;
	dirTargetToRoot.normalize();

	for (int i=_nodes.size()-1;i>0;i--)
	{
		Node &node=_nodes[i];
		Node &nodePrev=_nodes[i-1];

		if ((node.stage!=Node::Stage_PulledOut)&&(i!=_nodes.size()-1))
			break;

		float dist=_radiusNode*2.0f*(float)(_nodes.size()-1-i);

		BOOL bWithdraw=FALSE;
		if (dist>distTargetToRoot)
		{
			dist=distTargetToRoot;
			bWithdraw=TRUE;
		}

		i_math::vector3df posTarget=_posTarget+dist*dirTargetToRoot;

		i_math::vector3df pos=pd.getPosition(node.particle);
		i_math::vector3df posPrev=pd.getPosition(nodePrev.particle);

		i_math::vector3df vel;

		float speed;
		if (TRUE)
		{
			float scaleSpeed=1.0f;
// 			if (_nodes.size()-1-iLastPulledIn>0)
// 				scaleSpeed=(float)(i-iLastPulledIn)/(float)(_nodes.size()-1-iLastPulledIn);
			speed=g_speed*scaleSpeed;
		}

		if (!bWithdraw)
			vel=posTarget-pos;
		else
			vel=posPrev-pos;
		vel.setLength(speed);

		pd.setVelocity(node.particle,vel);
	}

	_AverageVelocity(model,iLastPulledIn+1);
	_AverageVelocity(model,iLastPulledIn+1);
	_AverageVelocity(model,iLastPulledIn+1);
	_AverageVelocity(model,iLastPulledIn+1);
// 	_DumpRecentVel(model);
}

float CTongueBranchPhys::GetFullLength()
{
	if (!_env)
		return 0.0f;

	PBD::SimulationModel *model=_env->GetSimModel();
	PBD::ParticleData &pd=model->getParticles();

	float dist=0.0f;
	for (int i=_nodes.size()-1;i>0;i--)
	{
		Node &node=_nodes[i];
		Node &nodePrev=_nodes[i-1];

		i_math::vector3df pos=pd.getPosition(node.particle);
		i_math::vector3df posPrev=pd.getPosition(nodePrev.particle);
		dist+=pos.getDistanceFrom(posPrev);
	}

	return dist;
}


void CTongueBranchPhys::Update(PBD::SimulationModel &model)
{
	PBD::ParticleData &pd=model.getParticles();

	ProfilerStart_Recent(UpdatePullIn)
	for (int i=1;i<_nodes.size()-1;i++)
	{
		Node &node=_nodes[i];
		Node &nodeNext=_nodes[i+1];

		if (node.stage==Node::Stage_PulledOut)
			continue;

		i_math::vector3df pos=pd.getPosition(node.particle);
		i_math::vector3df posNext=pd.getPosition(nodeNext.particle);
		if (nodeNext.stage==Node::Stage_PulledIn)
		{
			if(_posRoot.getDistanceFrom(posNext)>GetNodeDist()+0.2f)
			{
				model.addDistanceConstraint(node.particle,nodeNext.particle,g_stiffnessDistance,GetNodeDist(),nodeNext.constraintPulled);
				nodeNext.constraintDist=nodeNext.constraintPulled;
				nodeNext.constraintPulled=-1;

				nodeNext.stage=Node::Stage_PulledOut;
			}
		}
		else
		{
			if (nodeNext.stage==Node::Stage_PulledOut)
			{
				if(_posRoot.getDistanceFrom(posNext)<GetNodeDist())
				{
					model.addPullConstraint(node.particle,nodeNext.particle,GetNodeDist(),g_stiffnessPull,1.0f,-_dirRoot,nodeNext.constraintDist);
					nodeNext.constraintPulled=nodeNext.constraintDist;
					nodeNext.constraintDist=-1;

					nodeNext.stage=Node::Stage_PulledIn;
				}
			}
		}
	}
	ProfilerEnd();

	for (int i=1;i<_nodes.size()-1;i++)
	{
		Node &node=_nodes[i];
		if (node.stage==Node::Stage_PulledOut)
			_env->SetParticleCollision(node.particle,_collision);
		else
			_env->SetParticleCollision(node.particle,TongueParticleCollisionGroup_None);
	}


	_UpdateBendTolerance(model);

	for (int i=0;i<_nodes.size();i++)
	{
		Node &node=_nodes[i];
		pd.setAcceleration(node.particle,i_math::vector3df(0.0f,0.0f,0.0f));
		i_math::vector3df vel=pd.getVelocity(node.particle);
		pd.setVelocity(node.particle,vel*g_dampVel);
	}

	if (_bTargetPos)
	{
		ProfilerStart_Recent(_UpdateVelocityToTargetPos);
		_UpdateVelocityToTargetPos(model);
		ProfilerEnd();
	}

}

void CTongueBranchPhys::SetTargetPos(i_math::vector3df &pos)
{
	_bTargetPos=TRUE;
	_posTarget=pos;
}

void CTongueBranchPhys::ClearTargetPos()
{
	_bTargetPos=FALSE;
}

BOOL CTongueBranchPhys::GetTargetPos(i_math::vector3df &pos)
{
	if (!_bTargetPos)
		return FALSE;

	pos=_posTarget;
	return TRUE;
}

