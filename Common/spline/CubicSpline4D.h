#pragma once

typedef i_math::vector4df Vector4d;


class CCubicSpline4D
{
public:
	CCubicSpline4D()
	{
		_maxdist=0.0f;
	}
	struct Node
	{
		Vector4d position;
		Vector4d velocity;
		float dist0;
		float dist;
		float distTotal;//表示这个node到起始node的距离
	};

	struct Sample
	{
		float t;//绝对时间
		Vector4d pos;
		Vector4d vel;//速度
		int iNode;
		float tLerp;//iNode与下一个node之间的可用来插值的时间
	};
	void Reset();

	void AddNode(const Vector4d &pos);
	void AddNode(const Vector4d &pos, float timePeriod);

	void BuildRNS(BOOL bCircular);
	void BuildSNS(BOOL bCircular);
	void BuildTNS(BOOL bCircular);

	float GetDistance()	{		return _maxdist;	}
	DWORD GetNodeCount()	{		return _nodes.size();	}
	Node *GetNode(DWORD idx)	{		return &_nodes[idx];	}

	Vector4d GetPosition(float time);
	Vector4d GetVelocity(float time);

	DWORD GetSamples(float distGap,Sample*samples);
	DWORD GetSamplesByTime(float timeGap,Sample*samples);


protected:
	std::vector<Node>_nodes;
	float _maxdist;

	void _CalcDistance(BOOL bAuto);
	void _BuildSpline();
	Vector4d _GetStartVelocity(int index);
	Vector4d _GetEndVelocity(int index);

	float _AddEnds(BOOL bAuto);
	void _RemoveEnds(float dist);

	void _SmoothSNS();
	void _SmoothTNS(){ _SmoothSNS(); _ConstrainTNS(); _ConstrainTNS();_ConstrainTNS();}
	void _ConstrainTNS(); 

};
