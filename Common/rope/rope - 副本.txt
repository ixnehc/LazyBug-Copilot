
#pragma once

#include "class/class.h"
#include "gds/GObj.h"

struct RopeProp
{
	BEGIN_GOBJ_PURE(RopeProp,1);
		GELEM_VAR_INIT(float,m,1.0f);
			GELEM_EDITVAR("质点质量",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"质点质量");
		GELEM_VAR_INIT(float,radius,0.1f);
			GELEM_EDITVAR("质点半径",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"速度衰减");
		GELEM_VAR_INIT(float,stiffnessSpring,1.0f);
			GELEM_EDITVAR("弹簧stiffness",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"弹簧stiffness");
		GELEM_VAR_INIT(float,lengthSpring,0.2f);
			GELEM_EDITVAR("弹簧长度",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"弹簧长度");
		GELEM_VAR_INIT(float,frictionSpring,1.0f);
			GELEM_EDITVAR("弹簧摩擦系数",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"弹簧摩擦系数");
		GELEM_VAR_INIT(float,angleBend,180.0f);
			GELEM_EDITVAR("弯曲角度",GVT_F,GSem(GSem_Float,"0.000,180.0f.0,0.001"),"弯曲角度");
		GELEM_VAR_INIT(float,stiffnessBend,0.2f);
			GELEM_EDITVAR("弯曲stiffness",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"弯曲stiffness");
		GELEM_VAR_INIT(float,frictionAir,2.0f);
			GELEM_EDITVAR("空气摩擦系数",GVT_F,GSem(GSem_Float,"0.000,10000.0,0.001"),"空气摩擦系数");
		GELEM_VAR_INIT(float,repulsionGround,1.0f);
			GELEM_EDITVAR("地面弹性",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"地面弹性");
		GELEM_VAR_INIT(float,frictionGround,2.0f);
			GELEM_EDITVAR("地面摩擦系数",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"地面摩擦系数");
		GELEM_VAR_INIT(float,absorptionGround,2.0f);
			GELEM_EDITVAR("地面冲量吸收",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"地面冲量吸收");
		GELEM_VAR_INIT(float,dampingVel,0.2f);
			GELEM_EDITVAR("速度衰减",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"速度衰减");
		GELEM_VAR_INIT(float,bury,0.0f);
			GELEM_EDITVAR("可以陷入地表多深",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"可以陷入地表多深");
		GELEM_VAR_INIT(float,noise,0.0f);
			GELEM_EDITVAR("随机扰动",GVT_F,GSem(GSem_Float,"0.00,1.0,0.001"),"随机扰动");

	END_GOBJ();

	float m;
	float radius;
	float stiffnessSpring;
	float lengthSpring;
	float frictionSpring;
	float angleBend;
	float stiffnessBend;
	float frictionAir;
	float repulsionGround;
	float frictionGround;
	float absorptionGround;
	float dampingVel;
	float bury;
	float noise;
};


struct Mass
{
	DEFINE_CLASS(Mass);
	Mass()
	{
		m=0.0f;
		bStatic=TRUE;
		bVisible=1;
		htGround=-1000.0f;
	}
	double m;  
	RopeProp *prop;
	i_math::vector3dd pos;
	i_math::vector3dd vel; 
	i_math::vector3dd force;
	i_math::vector3dd friction;
	double htGround;
	DWORD bVisible:1;
	DWORD bStatic:1;

	void ApplyForce(i_math::vector3dd &force_)
	{
		force += force_;
	}

	void Simulate(float dt);
};

struct Spring
{                                       
	DEFINE_CLASS(Spring);

	Spring()
	{
		mass1=mass2;

		prop=NULL;
		bBroken=FALSE;
	}

	Mass* mass1;
	Mass* mass2;

	BOOL bBroken;

	RopeProp *prop;

	void Init(Mass* mass1, Mass* mass2,	RopeProp *prop);

	void Solve(float dt);
};

struct Impulse
{
	DEFINE_CLASS(Impulse);
	Impulse()
	{
		dur=0.0f;
		t=0.0f;
		mass=NULL;
	}


	void Solve(float dt);
	BOOL IsFinished();

	float dur;
	float t;
	Mass *mass;
	i_math::vector3dd force;
};

class CRope
{
public:

	void Init(int numOfMasses,RopeProp &prop,i_math::vector3df &g);
	void BeginInit(int numOfMasses,int numOfSpring,RopeProp &prop,i_math::vector3dd &g);
	void EndInit();

	void ResetProp(RopeProp &prop);

	void Clear();

	RopeProp *GetProp()	{		return _prop;	}

	DWORD GetMassCount()	{		return _masses.size();	}

	Mass* GetMass(int index)
	{
		if (index < 0 || index >= _masses.size())
			return NULL; 

		return _masses[index];
	}
	Spring *GetSpring(int index)
	{
		if (index < 0 || index >= _springs.size())
			return NULL; 

		return _springs[index];
	}

	int FindFirstBrokenSpring();

	void CalcAabb(i_math::aabbox3df &aabb);

	void Solve(float dt);
	void Simulate(float dt);
	void Operate(float dt);

	void Break(float r);
	void Break(int iConstraint);
	void SetDragging();

	void AddImpulse(Mass *mass,i_math::vector3dd &force,float dur);

	void GetSimulateRange(float &s,float &e);

protected:
	virtual void _CollectMassGroundHeights();

	RopeProp *_prop;

	std::vector<Mass*> _masses;
	std::vector<Spring*> _springs;                           // Springs Binding The Masses (There Shall Be [numOfMasses - 1] Of Them)
	std::vector<Impulse*> _impulses;
	i_math::vector3dd _g;                           // Gravitational Acceleration (Gravity Will Be Applied To All Masses)

	double _repulsion;                      // A Constant To Represent How Much The Ground Shall Repel The Masses
	double _friction;                       // A Constant Of Friction Applied To Masses By The Ground(Used For Sliding Of Rope On The Ground)

	double _absorption;                     // A Constant Of Absorption Friction Applied To Masses By The Ground(Used For Vertical Collisions Of The Rope With The Ground)

	double _frictionAir;                      // A Constant Of Air Friction Applied To Masses


};