#pragma once


class CRuler
{
public:
	typedef int RulerSpaceUnit;
	typedef float WorldSpaceUnit;
	CRuler()
	{
		_length=100;
		_off=0.0f;
		_scale=1.0f;
	}

	//标尺的长度
	void SetLength(RulerSpaceUnit length)	{		_length=length;	}
	void SetOff(WorldSpaceUnit off)	{		_off=off;	}
	void SetScale(float scale)	{		_scale=scale;	}

	RulerSpaceUnit ToRS(WorldSpaceUnit v)
	{
		return (RulerSpaceUnit)(v*_scale+_off);
	}
	WorldSpaceUnit ToWS(RulerSpaceUnit v)
	{
		return (WorldSpaceUnit)((v-_off)/_scale);
	}

	//steps里的数值必须从小到大排列,以0结尾
	int BuildMarks(WorldSpaceUnit*marks,float *steps,RulerSpaceUnit gap)
	{
		float step=1.0f;
		//首先我们决定使用哪个step
		if (steps)
		{
			int i=0;
			while(steps[i]>0.0f)
			{
				step=steps[i];
				RulerSpaceUnit v1,v2;
				v1=ToRS(0.0f);
				v2=ToRS(step);
				if (abs(v2-v1)>gap)
					break;
				i++;
			}
		}

		float start,end;
		start=ToWS(0);
		start=floor(start/step)*step;
		end=ToWS(_length);
		end=(floor(end/step)+1.0f)*step;
		BOOL bNeedSwap=FALSE;
		if (start>end)
		{
			Swap<float>(start,end);
			bNeedSwap=TRUE;
		}

		start-=2*step;
		end+=2*step;

		DWORD c=0;
		for (float t=start;t<=end;t+=step)
		{
			if (marks)
			{
				marks[c]=t;
				//防止在靠近0的时候,出现浮点误差
				if ((marks[c]<step/4.0f)&&(marks[c]>(-step/4.0f)))
					marks[c]=0.0f;
			}
			c++;
		}

		if (bNeedSwap)
		{
			for (int i=0;i<c/2;i++)
				Swap(marks[i],marks[c-1-i]);
		}
		return c;
	}

protected:
	RulerSpaceUnit _length;
	WorldSpaceUnit _off;
	float _scale;

};

