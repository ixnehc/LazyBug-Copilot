#pragma once
#include <deque>
#include "../math/imath_all.h"

class CSimpleCurve
{
public:
	CSimpleCurve() : _totalLength(0.0f) {}

	// 1. 从头部添加点并带有时间标签
	void AddPointToHead(const i_math::vector3df& point, float time)
	{
		if (!_points.empty()) // 计算与上一个点的距离，并更新总长度
		{
			float length = _points.front().getDistanceFrom(point);
			if (length < 0.001f)
				return;

			_lengths.push_front(length); // 在 deque 的头部插入新段长度
			_totalLength += length;
		}
		else
		{
			_lengths.push_front(0.0f); // 第一个点长度设为 0
		}

		_points.push_front(point);   // 在 deque 的头部插入新点
		_times.push_front(time);     // 插入时间标签到对应位置
	}

	// 从尾部添加点并带有时间标签，并返回该点离曲线头部的总距离
	float AddPointToTail(const i_math::vector3df& point, float time)
	{
		if (!_points.empty()) // 计算新点与尾部点的距离，并更新总长度
		{
			float length = _points.back().getDistanceFrom(point);
			if (length < 0.001f)
				return _totalLength;

			_lengths.back()=length;
			_lengths.push_back(0.0f); // 在 deque 的尾部插入新段长度
			_totalLength += length;
		}
		else
		{
			_lengths.push_back(0.0f); // 第一个点长度设为 0
		}

		_points.push_back(point);  // 在 deque 的尾部插入新点
		_times.push_back(time);    // 插入时间标签到对应位置
		return _totalLength;       // 返回新添加点的总长度（从头部到该点）
	}

	// 2. 求曲线的总长度
	float GetTotalLength() const
	{
		return _totalLength;
	}

	BOOL GetPositionAndDirectionAtDistance(float distance, i_math::vector3df& position, i_math::vector3df& direction) const
	{
		if (_points.empty() || distance < 0 || distance > _totalLength)
			return FALSE;

		float accumulatedLength = 0.0f;

		// 顺序遍历各段，查找对应距离的段落
		for (size_t i = 0; i < _points.size()-1; ++i)
		{
			accumulatedLength += _lengths[i];

			if (accumulatedLength >= distance) // 找到目标段
			{
				float segmentStartLength = accumulatedLength - _lengths[i];
				float segmentRatio = (distance - segmentStartLength) / _lengths[i];

				// 计算目标位置
				position = _points[i+1].getInterpolated(_points[i], segmentRatio);
				// 计算目标方向
				direction = (_points[i+1] - _points[i]).normalize();
				return TRUE;
			}
		}

		return FALSE;
	}

	// 4. 删除指定距离后尾部的点
	void RemovePointsAfterDistance(float distance)
	{
		if (distance <= 0 || _points.empty())
		{
			_points.clear();
			_lengths.clear();
			_times.clear();
			_totalLength = 0.0f;
			return;
		}

		float currentLength = 0.0f;
		for (size_t i = 0; i < _points.size(); ++i)
		{
			currentLength += _lengths[i];
			if (currentLength > distance)
			{
				_points.resize(i+1); // 调整 deque 大小，删除尾部元素
				_lengths.resize(i+1);
				_times.resize(i+1); // 删除多余的时间标签

				_totalLength = currentLength;
				break;
			}
		}
	}

	void RemovePointsFromHead()
	{
		if(!_points.empty())
		{
			_points.pop_front();
			_times.pop_front();

			if (!_lengths.empty())
			{
				_totalLength -= _lengths.front();
				_lengths.pop_front();
			}
		}
	}

	float GetHeadLength()
	{
		if (_lengths.size()>0)
			return _lengths[0];
		return 0.0f;
	}

// 	//! 定位到曲线上指定距离处的点，并返回最小旋转角度对齐的新的旋转四元数
// 	i_math::quatf AlignRotationToCurve(f32 distance, const i_math::quatf& currentRotation) const
// 	{
// 		// 1. 获取曲线上的目标点方向（假设获取方向的方法已经存在）
// 		vector3df curveDirection = GetCurveDirectionAtDistance(distance);
// 
// 		// 2. 将传入的旋转四元数的 Z 轴方向提取出来
// 		vector3df currentZAxis = currentRotation * vector3df(0, 0, 1);
// 
// 		// 3. 计算对齐四元数：从当前 Z 轴方向旋转到曲线上目标方向
// 		quatf alignmentQuat;
// 		alignmentQuat.from2Vector(currentZAxis, curveDirection);
// 
// 		// 4. 计算新的四元数：用最小角度旋转到目标方向
// 		quatf newRotation = alignmentQuat * currentRotation;
// 
// 		// 5. 返回新的旋转四元数
// 		return newRotation;
// 	}

private:
	std::deque<i_math::vector3df> _points;  // 使用 deque 存储曲线的所有点
	std::deque<float> _lengths;             // 使用 deque 存储各段长度
	std::deque<float> _times;               // 使用 deque 存储各点的时间标签
	float _totalLength;                     // 曲线的总长度
};
