// Random.h: interface for the CRandom class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include <assert.h>
#include <deque>
#include <functional>
#include "math/sphere.h"

#include "../stackcontainer/stackvector.h"


class CSysRandom  
{
public:
	CSysRandom(){}
	virtual ~CSysRandom()	{}
	
	static void	Srand();
	static void	Srand( DWORD dwSeed);	// 指定seed和生成的最大值
	static DWORD	Rand();											// 返回 [0,RAND_MAX)

	template <class T>
	static T RandRange( T low, T high )							// 在一个范围内随机
	{
		float fPercentage = (float)Rand() / RAND_MAX;
		return (T)( low + fPercentage * (float)( high - low ) );
	}

	template <class T>
	static T RandVary( T v, T vary)							// 根据浮动值随机
	{
		T low,hi;
		if (v>vary)
			low=v-vary;
		else
			low=0;
		hi=v+vary;
		return RandRange(low,hi);
	}


	//[low,high)
	template <class T>
	static T RandRangeInt(T low,T high)
	{
		float fPercentage = (float)rand() / RAND_MAX;
		T ret=(T)( low + fPercentage * (float)( high - low ) );
		if (ret>=high)
			ret=high-1;
		if (ret<low)
			ret=low;
		return ret;
	}

	template <class T>
	static T RandVaryUInt(T v,T vary)
	{
		if (vary<=0)
			return v;
		T s,e;
		if (vary>v)
			s=0;
		else
			s=v-vary;
		e=v+vary+1;
		return RandRangeInt<T>(s,e);
	}

	static BOOL Roll(float rate)
	{
		if (rate>1.0f)
			return TRUE;

		if ((((float)rand()) / (float)(RAND_MAX))>rate)
			return FALSE;
		return TRUE;

	}

	template<typename T>
	static 
	void GenRandomIndices(std::vector<T> &indices,int n)
	{
		indices.resize(n);
		for (int i=0;i<indices.size();i++)
			indices[i]=i;

		for (int i=0;i<indices.size()*2;i++)
		{
			int idx=CSysRandom::RandRangeInt<T>(0,indices.size());
			Swap(indices[0],indices[idx]);
		}

	}

	template<typename T>
	static 
	void GenRandomIndices(std::deque<T> &indices,int n)
	{
		indices.resize(n);
		for (int i=0;i<indices.size();i++)
			indices[i]=i;

		for (int i=0;i<indices.size()*2;i++)
		{
			int idx=CSysRandom::RandRangeInt<T>(0,indices.size());
			Swap(indices[0],indices[idx]);
		}

	}


	template<typename T>
	static
	T* RollWeighted(std::vector<T*> entries)
	{
		float total=0.0f;
		for (int i=0;i<entries.size();i++)
		{
			if (entries[i])
				total+=entries[i]->wt;
		}
		if (total>0.0f)
		{
			float v=RandRange(0.0f,total);

			total=0.0f;
			for (int i=0;i<entries.size();i++)
			{
				if (entries[i])
				{
					total+=entries[i]->wt;
					if (total>=v)
						return entries[i];
				}
			}
		}
		for (int i=entries.size()-1;i>=0;i++)
		{
			if (entries[i])
				return entries[i];
		}
		return NULL;
	}

	template<typename T>
	static
	T* RollWeighted(std::vector<T> &entries)
	{
		float total=0.0f;
		for (int i=0;i<entries.size();i++)
		{
			total+=entries[i].wt;
		}
		if (total>0.0f)
		{
			float v=RandRange(0.0f,total);

			total=0.0f;
			for (int i=0;i<entries.size();i++)
			{
				total+=entries[i].wt;
				if (total>=v)
					return &entries[i];
			}
		}
		for (int i=entries.size()-1;i>=0;i++)
		{
			return &entries[i];
		}
		return NULL;
	}

	static
	i_math::vector2df GenRandomPos2DIn(i_math::spheref &sph)
	{
		float theta = RandRange<float>(0.0f,i_math::Pi*2.0f);
		float r = sph.radius * sqrtf(RandRange<float>(0.0f,1.0f));        // 半径需使用 sqrtf() 来均匀分布

		// 转换为直角坐标
		return i_math::vector2df(sph.center.x+r * cosf(theta),sph.center.z+r * sinf(theta));
	}


public:
};


class CRandom_Gauss
{
public:
	CRandom_Gauss()
	{
		m_bReady=FALSE;
	}
	CSysRandom m_rand;
	void srand(DWORD seed)
	{
		m_rand.Srand(seed);
	}

	double rand(double mean=0.0,double deviation=1.0);
private:
	double m_vReady;
	BOOL m_bReady;
};



#define RANDOM_MAX 0x7FFFFFFF
class CRandom
{
public:
	CRandom()
	{
		_next=1;
	}
	CRandom(DWORD seed)
	{
		srand(seed);
	}
	DWORD rand(void)
	{
		return _do_rand(&_next);
	}
	void srand(DWORD seed)
	{
		_next = seed;
	}
	template <class T>
	T RandRange( T low, T high )							// 在一个范围内随机
	{
		float fPercentage = (float)this->rand() / RAND_MAX;
		return (T)( low + fPercentage * (float)( high - low ) );
	}

	template <class T>
	T RandVary( T v, T vary)							// 根据浮动值随机
	{
		T low,hi;
		if (v>vary)
			low=v-vary;
		else
			low=0;
		hi=v+vary;
		return RandRange(low,hi);
	}


	//[low,high)
	template <class T>
	T RandRangeInt(T low,T high)
	{
		float fPercentage = (float)this->rand() / RAND_MAX;
		T ret=(T)( low + fPercentage * (float)( high - low ) );
		if (ret>=high)
			ret=high-1;
		if (ret<low)
			ret=low;
		return ret;
	}

	template <class T>
	T RandVaryUInt(T v,T vary)
	{
		if (vary<=0)
			return v;
		T s,e;
		if (vary>v)
			s=0;
		else
			s=v-vary;
		e=v+vary+1;
		return RandRangeInt<T>(s,e);
	}

	BOOL Roll(float rate)
	{
		if (rate>1.0f)
			return TRUE;

		if ((((float)this->rand()) / (float)(RAND_MAX))>rate)
			return FALSE;
		return TRUE;

	}

protected:
	DWORD _do_rand(DWORD*value)
	{
		long quotient, remainder, t;

		quotient = *value / 127773L;
		remainder = *value % 127773L;
		t = 16807L * remainder - 2836L * quotient;

		if (t <= 0)
		  t += 0x7FFFFFFFL;
		return ((*value = t) % ((unsigned long)RAND_MAX + 1));
	}
	unsigned long _next;

};



template <int PositionContainerSize=32>
class CRandomPositionsGenerator
{
protected:
	std::vector<i_math::spheref> _area;
	float _minDist;
	bool _init;

	// 预计算的区域面积
	std::vector<float> _areaValues;
	float _totalArea;

public:

	typedef StackVector<i_math::vector2df, PositionContainerSize> PositionContainer;

	CRandomPositionsGenerator()
	{
		Zero();
	}

	void Zero()
	{
		_minDist = 0.0f;
		_init = false;
		_totalArea = 0.0f;
	}

	void Clear()
	{
		_areaValues.clear();
		_area.clear();
	}

	// 初始化函数，预计算区域面积
	void Init(const std::vector<i_math::spheref>& areas, float minDistance)
	{
		_area = areas;
		_minDist = minDistance;
		_init = false;

		if (areas.empty())
		{
			return;
		}

		// 预计算每个区域的面积
		_areaValues.resize(areas.size());
		_totalArea = 0.0f;

		for (size_t i = 0; i < areas.size(); ++i)
		{
			float area = i_math::Pi * areas[i].radius * areas[i].radius;
			_areaValues[i] = area;
			_totalArea += area;
		}

		_init = true;
	}

	// 检查是否已初始化
	bool IsInitialized() const
	{
		return _init;
	}

	const std::vector<i_math::spheref>& GetAreas() const
	{
		return _area;
	}

	float GetMinDistance() const
	{
		return _minDist;
	}

	float GetTotalArea() const
	{
		return _totalArea;
	}

	// 生成单个随机点的函数
	i_math::vector2df Gen() const
	{
		if (!_init || _area.empty())
		{
			return i_math::vector2df(0, 0); // 未初始化时返回原点
		}

		// 随机选择一个球体，概率与其面积成正比
		float randomVal = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * _totalArea;
		float areaSum = 0.0f;
		const i_math::spheref* selectedSphere = NULL;
		size_t selectedIndex = 0;

		for (size_t i = 0; i < _area.size(); ++i)
		{
			areaSum += _areaValues[i];
			if (randomVal <= areaSum)
			{
				selectedSphere = &_area[i];
				selectedIndex = i;
				break;
			}
		}

		// 如果没有选中任何球体（由于浮点误差可能发生），选择最后一个
		if (selectedSphere == NULL)
		{
			selectedSphere = &_area.back();
			selectedIndex = _area.size() - 1;
		}

		// 在选中的球体内均匀随机生成一个点
		float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * i_math::Pi;

		// 使用平方根分布来确保均匀性
		float sqrtR = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		float r = sqrtR * selectedSphere->radius;

		// 计算在XZ平面上的坐标
		float x = selectedSphere->center.x + r * cosf(angle);
		float z = selectedSphere->center.z + r * sinf(angle);

		return i_math::vector2df(x, z);
	}

	// 检查点之间是否满足最小距离要求
	bool CheckMinDistanceConstraint(const i_math::vector2df& newPoint,
		const PositionContainer& existingPoints) const
	{
		float minDistSq = _minDist * _minDist;

		for (size_t i = 0; i < existingPoints.size(); ++i)
		{
			const i_math::vector2df& pos = existingPoints[i];
			float dx = newPoint.x - pos.x;
			float dy = newPoint.y - pos.y;
			float distSq = dx * dx + dy * dy;

			if (distSq < minDistSq)
			{
				return false;
			}
		}

		return true;
	}


	// 生成多个满足最小距离要求的随机位置
	// 使用默认的全部有效的验证函数
	void GenRandomPositions(int count, const PositionContainer& existingPositions, PositionContainer& positions)
	{
		// 默认的验证函数总是返回 true
		return GenRandomPositions(count, existingPositions,positions,[](const i_math::vector2df&) { return true; });
	}

	// 生成多个满足最小距离要求的随机位置
	void Gen(int count, const PositionContainer& existingPositions, PositionContainer& positions,
		const std::function<bool(const i_math::vector2df&)>& isPositionValid)
	{
		positions.clear();

		// 确保已初始化且有区域
		if (!_init || _area.empty())
			return;

		// 最大尝试次数，防止无限循环
		const int MAX_ATTEMPTS = count * 10;

		// 对于每个需要生成的位置
		for (int i = 0; i < count; ++i)
		{
			bool validPosition = false;
			int attempts = 0;

			// 尝试生成一个符合条件的新位置
			while (!validPosition && attempts < MAX_ATTEMPTS)
			{
				// 生成一个候选位置
				i_math::vector2df candidate = Gen();

				// 检查额外的有效性条件（由子类实现）
				if (isPositionValid(candidate))
				{
					// 检查与已有位置的最小距离约束
					if (CheckMinDistanceConstraint(candidate, positions))
					{
						if (CheckMinDistanceConstraint(candidate, existingPositions))
						{
							validPosition = true;
							positions.push_back(candidate);
						}
					}
				}

				++attempts;
			}

			// 如果尝试了足够多次仍找不到合适位置，说明空间可能不足
			if (attempts >= MAX_ATTEMPTS)
				break;
		}
	}
};
