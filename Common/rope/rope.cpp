
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Random/Random.h"

#include "rope.h"

//////////////////////////////////////////////////////////////////////////
//Mass 
void Mass::Simulate(float dt)
{
	if (!bStatic)
	{
		vel*=prop->dampingVel;

		// 限制最大速度
		const double maxSpeed = 20.0; // 根据需求调整
		double currentSpeed = vel.getLength();
		if (currentSpeed > maxSpeed) 
		{
			vel = vel * (maxSpeed / currentSpeed);
		}
		if (prop->noise>0.0f)
		{
			double l=force.getLength()*(double)prop->noise;

			// 限制噪声强度
			const double maxNoise = 5.0;
			if (l > maxNoise) 
				l = maxNoise;

			i_math::vector3dd noise;
			noise.x=CSysRandom::RandRange(-l,l);
			noise.y=CSysRandom::RandRange(-l,l);
			noise.z=CSysRandom::RandRange(-l,l);
			force+=noise;
		}
		vel += (force / m) * dt;

		if (TRUE)
		{
			currentSpeed = vel.getLength();
			if (currentSpeed > maxSpeed)
			{
				vel = vel * (maxSpeed / currentSpeed);
			}
		}

		pos += vel * (double)dt;
		if (pos.y-(double)prop->radius<((double)htGround)-(double)prop->bury)
			pos.y=((double)htGround)-((double)prop->bury)+(double)prop->radius;
	}
}


//////////////////////////////////////////////////////////////////////////
//Spring

void Spring::Init(Mass* mass1, Mass* mass2,	RopeProp *prop)
{
	this->prop= prop;               // Set The stiffness
	this->mass1 = mass1;                     // Set mass1
	this->mass2 = mass2;                     // Set mass2
}

void Spring::Solve(float dt)
{
	if (bBroken)
		return;

	i_math::vector3dd springVector = mass1->pos - mass2->pos;      // Vector Between The Two Masses

	double r = springVector.getLength();                // Distance Between The Two Masses
	i_math::vector3dd force;                         // Force Initially Has A Zero Value
	if (r != 0)                         // To Avoid A Division By Zero... Check If r Is Zero
		force += -(springVector / r) * (r - (double)prop->lengthSpring) *(double)prop->stiffnessSpring;// The Spring Force Is Added To The Force
	force += -(mass1->vel - mass2->vel) * (double)prop->frictionSpring;       // The Friction Force Is Added To The force

	// 限制弹簧力最大值
	if (FALSE)
	{
		const double maxSpringForce = mass1->m * 500.0f; // 根据需求调整
		double currentForce = force.getLength();
		if (currentForce > maxSpringForce)
		{
			force = force * (maxSpringForce / currentForce);
		}
	}

	// With This Addition We Obtain The Net Force Of The Spring
	mass1->ApplyForce(force);                    // Force Is Applied To mass1
	mass2->ApplyForce(-force);                   // The Opposite Of Force Is Applied To mass2
}

//////////////////////////////////////////////////////////////////////////
//Impulse
BOOL Impulse::IsFinished()
{
	return t>=dur;
}

void Impulse::Solve(float dt)
{
	if (IsFinished())
		return;

	mass->ApplyForce(force);
	t+=dt;
}


//////////////////////////////////////////////////////////////////////////
//CRope
void CRope::Init(int numOfMasses,RopeProp &prop,i_math::vector3df &g)
{
	_prop=&prop;
	_g.set(g.x,g.y,g.x);

	_frictionAir = (double)prop.frictionAir;
	_friction = (double)prop.frictionGround;
	_repulsion= (double)prop.repulsionGround;
	_absorption= (double)prop.absorptionGround;

	_masses.resize(numOfMasses);
	for (int a = 0; a < numOfMasses; ++a)
	{
		_masses[a] = Class_New2(Mass);
		_masses[a]->m=(double)prop.m;
		_masses[a]->prop=_prop;
	}

	_springs.resize(numOfMasses - 1);
	for (int a = 0; a < numOfMasses - 1; ++a)                // To Create Everyone Of Each Start A Loop
	{
		_springs[a] = Class_New2(Spring);
		_springs[a]->Init(_masses[a], _masses[a + 1],&prop);
	}

}

void CRope::BeginInit(int numOfMasses,int numOfSpring,RopeProp &prop,i_math::vector3dd &g)
{
	_prop=&prop;
	_g= g;

	_frictionAir = (double)prop.frictionAir;
	_friction = (double)prop.frictionGround;
	_repulsion= (double)prop.repulsionGround;
	_absorption= (double)prop.absorptionGround;

	_masses.resize(numOfMasses);
	for (int a = 0; a < numOfMasses; ++a)
	{
		_masses[a] = Class_New2(Mass);
	}

	_springs.resize(numOfSpring);
	for (int a = 0; a < numOfSpring; ++a) 
	{
		_springs[a] = Class_New2(Spring);
	}
}

void CRope::EndInit()
{

}

void CRope::ResetProp(RopeProp &prop)
{
	_prop=&prop;

	_frictionAir = (double)prop.frictionAir;
	_friction = (double)prop.frictionGround;
	_repulsion= (double)prop.repulsionGround;
	_absorption= (double)prop.absorptionGround;

}



void CRope::Clear()
{
	for (int a = 0; a < _masses.size(); ++a)
	{
		Safe_Class_Delete(_masses[a]);
	}
	_masses.clear();

	for (int a = 0; a < _springs.size(); ++a)
	{
		Safe_Class_Delete(_springs[a]);
	}
	_springs.clear();

	for (int a = 0; a < _impulses.size(); ++a)
	{
		Safe_Class_Delete(_impulses[a]);
	}
	_impulses.clear();
}

void CRope::_CollectMassGroundHeights()
{
}


int CRope::FindFirstBrokenSpring()
{
	for (int i=0;i<_springs.size();i++)
	{
		if (_springs[i]->bBroken)
			return i;
	}
	return -1;
}

void CRope::GetSimulateRange(float &s,float &e)
{
	s=e=1.0f;
	for (int i = 0; i< _masses.size(); i++)
	{
		if (!_masses[i]->bStatic)
		{
			s=(((float)i)-0.5f)/(1.0f+(float)_masses.size());
			s=i_math::clamp_f(s,0.0f,1.0f);
			break;
		}
	}

	for (int i = _masses.size()-1; i>=0 ; i--)
	{
		if (!_masses[i]->bStatic)
		{
			e=(((float)i)+0.5f)/(1.0f+(float)_masses.size());
			e=i_math::clamp_f(e,0.0f,1.0f);
			break;
		}
	}

	if (e<s)
		e=s;

}

void CRope::CalcAabb(i_math::aabbox3df &aabb)
{
	aabb.resetInvalid();
	if (_masses.size()>0)
	{
		aabb.addInternalPoint((float)_masses[0]->pos.x,(float)_masses[0]->pos.y,(float)_masses[0]->pos.z);
		for (int a = 1; a < _masses.size(); ++a)
		{
			aabb.addInternalPoint_nocheck((float)_masses[a]->pos.x,(float)_masses[a]->pos.y,(float)_masses[a]->pos.z);
		}
	}

}


void CRope::Solve(float dt)
{
	for (int a = 0; a < _masses.size(); ++a)
	{
		_masses[a]->force.setZero();
	}

	for (int a = 0; a < _springs.size(); ++a)
		_springs[a]->Solve(dt);

	if (_prop->angleBend<179.9f)
	{
		double radBend=((float)_prop->angleBend)*(double)i_math::GRAD_PI2;
		for (int a = 1; a < _masses.size()-1; ++a)
		{
			Mass *massCur=_masses[a];
			Mass *massPrev=_masses[a-1];
			Mass *massNext=_masses[a+1];

			if (_springs[a]->bBroken)
				continue;
			if (_springs[a-1]->bBroken)
				continue;

			i_math::vector3dd dir1=massPrev->pos-massCur->pos;
			i_math::vector3dd dir2=massNext->pos-massCur->pos;

			if ((dir1.getLengthSQ()>0.0001)&&(dir2.getLengthSQ()>0.0001))
			{
				dir1.normalize();
				dir2.normalize();

				double rad=acos(dir1.dotProduct(dir2));
				rad=fabs(i_math::Pi-rad);

				if (rad>radBend)
				{
					i_math::vector3dd force=dir1+dir2;
					if (force.getLengthSQ()>0.0001f)
					{
						force.normalize();
						force*=(rad-radBend)*(double)_prop->stiffnessBend;
						massCur->ApplyForce(force);
					}
				}

			}
		}
	}

	if (TRUE)
	{
		int sz=0;
		for (int a = 0; a < _impulses.size(); ++a)
		{
			_impulses[a]->Solve(dt);
			if(!_impulses[a]->IsFinished())
			{
				_impulses[sz]=_impulses[a];
				sz++;
			}
			else
			{
				Safe_Class_Delete(_impulses[a]);
			}
		}
		_impulses.resize(sz);
	}

	for (int a = 0; a < _masses.size(); ++a)                // Start A Loop To Apply Forces Which Are Common For All Masses
	{
		_masses[a]->ApplyForce(_g* _masses[a]->m);    // The Gravitational Force
		// The air friction
		_masses[a]->ApplyForce(-_masses[a]->vel * _frictionAir);

		double height=_masses[a]->htGround;
		double bottom=_masses[a]->pos.y-_prop->radius;
		if (bottom <= height+0.001)          // Forces From The Ground Are Applied If A Mass Collides With The Ground
		{
			i_math::vector3dd v;                 // A Temporary i_math::vector3df

			v = _masses[a]->vel;              // Get The Velocity
			v.y = 0;                    // Omit The Velocity Component In Y-Direction

			// The Velocity In Y-Direction Is Omited Because We Will Apply A Friction Force To Create
			// A Sliding Effect. Sliding Is Parallel To The Ground. Velocity In Y-Direction Will Be Used
			// In The Absorption Effect.

			// Ground Friction Force Is Applied            
			_masses[a]->ApplyForce(-v * _friction);
//			_masses[a]->ApplyFriction(v,_friction);

			v = _masses[a]->vel;              // Get The Velocity
			v.x = 0;                    // Omit The x And z Components Of The Velocity
			v.z = 0;                    // We Will Use v In The Absorption Effect

			// Above, We Obtained A Velocity Which Is Vertical To The Ground And It Will Be Used In
			// The Absorption Force

			if (v.y < 0)                 // Let's Absorb Energy Only When A Mass Collides Towards The Ground
			{
//				_masses[a]->ApplyFriction(v,_absorption);
				_masses[a]->ApplyForce(-v * _absorption);
			}

			// The Ground Shall Repel A Mass Like A Spring.
			// By "i_math::vector3df(0, groundRepulsionConstant, 0)" We Create A Vector In The Plane Normal Direction
			// With A Magnitude Of groundRepulsionConstant.
			// By (groundHeight - masses[a]->pos.y) We Repel A Mass As Much As It Crashes Into The Ground.
			i_math::vector3dd force = i_math::vector3dd(0, _repulsion, 0) *
				(height- bottom);

			_masses[a]->ApplyForce(force);            // The Ground Repulsion Force Is Applied
		}
	}
}

void CRope::Simulate(float dt)
{
	for (int a = 0; a < _masses.size(); ++a)
		_masses[a]->Simulate(dt); 

// 	ropeConnectionPos += ropeConnectionVel * dt;            // Iterate The Positon Of ropeConnectionPos
// 
// 	if (ropeConnectionPos.y < groundHeight)              // ropeConnectionPos Shall Not Go Under The Ground
// 	{
// 		ropeConnectionPos.y = groundHeight;
// 		ropeConnectionVel.y = 0;
// 	}
// 
// 	masses[0]->pos = ropeConnectionPos;              // Mass With Index "0" Shall Position At ropeConnectionPos
// 	masses[0]->vel = ropeConnectionVel;              // The Mass's Velocity Is Set To Be Equal To ropeConnectionVel
}


void CRope::Operate(float dt)
{
// 	_CollectMassGroundHeights();
	Solve(dt);
	Simulate(dt);
}

void CRope::SetDragging()
{
	for (int i=_masses.size()-1;i>0;i--)
	{
		if (_masses[i]->bStatic)
			_masses[i]->bStatic=0;
		else
			break;
	}
}

void CRope::Break(int iConstraint)
{
	if ((iConstraint>=0)&&(iConstraint<_springs.size()))
	{
		_springs[iConstraint]->bBroken=TRUE;

		if (_springs[iConstraint]->mass1)
		{
			if (!_springs[iConstraint]->mass1->bStatic)
				AddImpulse(_springs[iConstraint]->mass1,i_math::vector3dd(0.0,10.0,0.0),0.2f);
			if (!_springs[iConstraint]->mass2->bStatic)
				AddImpulse(_springs[iConstraint]->mass2,i_math::vector3dd(0.0,10.0,0.0),0.2f);
		}
	}
}


void CRope::Break(float r)
{
	int iStart,iEnd;
	for (int i=0;i<_masses.size();i++)
	{
		if (!_masses[i]->bStatic)
		{
			iStart=i;
			break;
		}
	}
	for (int i=_masses.size()-1;i>=0;i--)
	{
		if (!_masses[i]->bStatic)
		{
			iEnd=i;
			break;
		}
	}

	int iStartConstraint=iStart-1;
	int iEndConstraint=iEnd;

	int iConstraint=FloatToNearestInt((float)iStartConstraint+((float)(iEndConstraint-iStartConstraint))*r);
	if (iConstraint<iStartConstraint)
		iConstraint=iStartConstraint;
	if (iConstraint>iEndConstraint)
		iConstraint=iEndConstraint;

	Break(iConstraint);
}

void CRope::AddImpulse(Mass *mass,i_math::vector3dd &force,float dur)
{
	Impulse *impulse=Class_New2(Impulse);
	impulse->mass=mass;
	impulse->dur=dur;
	impulse->force=force;
	_impulses.push_back(impulse);
}

