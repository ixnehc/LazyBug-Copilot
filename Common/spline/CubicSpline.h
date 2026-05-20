#pragma once

#include "../math/imath_all.h"


class CBitPacket;
class CCubicSpline
{
public:
	CCubicSpline()
	{
		Reset(FALSE);
	}

	CCubicSpline(BOOL bCircular)
	{
		Reset(bCircular);
	}

	enum State
	{
		NotBuilt,//尚未Build
		XlateBuilt,//位置数据Build了
		RotBuilt,//旋转数据Build了
	};

	struct Node
	{
		i_math::vector3df position;
		i_math::vector3df velocity;
		float dist0;
		float dist;
		float distTotal;	//表示这个node到起始node的距离
		i_math::quatf rot;	//表示该节点的旋转量
	};

	struct Sample
	{
		i_math::vector3df pos;
		i_math::vector3df vel;//速度
		int iNode;
		float tLerp;//iNode与下一个node之间的可用来插值的时间 局部函数段的时间
		i_math::quatf rot;
	};

	void Reset(BOOL bCircular);
	BOOL IsEmpty()	{		return _state==NotBuilt||_nodes.size()<=0;	}

	void Write(CBitPacket& bp);
	void Read(CBitPacket& bp);

	void AddNode(const i_math::vector2df &pos,const i_math::quatf & rot);
	void AddNode(const i_math::vector3df &pos,const i_math::quatf & rot);
	void AddNode(const i_math::vector3df &pos,const i_math::quatf & rot, float timePeriod);
	void SetNodeRot(DWORD idx,i_math::quatf &rot)	{		_nodes[idx].rot=rot;	}

	BOOL BuildRNS();
	BOOL BuildSNS();//
	BOOL BuildTNS();

	BOOL BuildRot();

	float GetDistance()	{		return _maxdist;	}
	DWORD GetNodeCount();
	Node *GetNode(DWORD idx);

	i_math::vector3df GetPosition(float time);
	i_math::vector3df GetPositionByDist(float dist);
	i_math::vector3df GetVelocity(float time);
	i_math::vector3df GetVelocityByDist(float dist);
	DWORD GetCurNode(float time);

	DWORD GetSamples(float distGap,Sample*samples,float distStart=0.0f,float distEnd=-1.0f);
	DWORD GetSamplesByTime(float timeGap,Sample*samples);
	BOOL GetSampleByDist(Sample &sample,float dist);

	void CopyFrom(CCubicSpline &other)
	{
		_state=other._state;
		_nodes=other._nodes;
		_maxdist=other._maxdist;

		_rotR4=other._rotR4;
		_rotR4D2=other._rotR4D2;

		_bCircular=other._bCircular;
	}

	
protected:
	State _state;

	void _CalcDistance(BOOL bAuto);
	void _BuildSpline();
	i_math::vector3df _GetStartVelocity(int index);
	i_math::vector3df _GetEndVelocity(int index);

	void _MakeCircular(BOOL bAuto);//添加节点使变成一个circle,如果_bCircular为TRUE的话

	int _GetModIdx(int idx);
	float _AddEnds();
	void _RemoveEnds(float dist);

	void _SmoothSNS();
	void _SmoothTNS(){ _SmoothSNS(); _ConstrainTNS(); _ConstrainTNS();_ConstrainTNS();}
	void _ConstrainTNS(); 

	//S3->R4 并有中值定理得到d2
	void  _BuildQD();

	std::vector<Node>_nodes;
	float _maxdist;


	struct R4{
		union{
			float v[4];
			struct{
				float x,y,z,w;
			};
		};
		void set(float ax,float ay,float az,float aw){x=ax;y=ay;z=az;w=aw;}
		R4 operator *(float m0)
		{
			R4 ret;
			for(int i = 0;i<4;i++)
				ret.v[i] = v[i]*m0;
			return ret;
		}

		R4  operator +(R4 & oth)
		{
			R4 ret;
			for(int i = 0;i<4;i++)
				ret.v[i] = v[i]*oth.v[i];
			return ret;
		}
		R4  operator /(float m0)
		{
			for(int i = 0;i<4;i++)
				v[i] = v[i]/m0;
			return *this;
		}
	};

	i_math::quatf R4Lerp(R4 &r40,R4 &r41,R4 &r4d0,R4 &r4d1,float h,float t);

	std::vector<R4> _rotR4;				//四元数的R4形式
	std::vector<R4> _rotR4D2;			//四元数R4形式的2阶导
	BOOL _bCircular;						//是否要生成封闭的曲线
};



