#pragma once

#include <vector>

#include "../math/rect.h"


//NOTE: functions T should implement include:
//operator *
//operator /
//operator +
template <class T,typename T_RectElem=int>
class CGaussianBlur
{
public:
	CGaussianBlur(T zero)
	{
		_zero=zero;
		_w=0;
		_h=0;
	}
	void SetData(T *data,T_RectElem  w,T_RectElem h,DWORD pitch=0)
	{
		_w=w;
		_h=h;
		_buf.resize(w*h);
		if (pitch==0)
			pitch=w;
		T *p=_buf.data();
		T *q=data;

		for (int j=0;j<h;j++)
		{
			memcpy(p,q,sizeof(T)*w);
			p+=w;
			q+=pitch;
		}
	}
	void GetData(T *data,T_RectElem w,T_RectElem h,DWORD pitch=0)
	{
		_w=w;
		_h=h;
		_buf.resize(w*h);
		if (pitch==0)
			pitch=w;
		T *p=_buf.data();
		T *q=data;

		for (int j=0;j<h;j++)
		{
			memcpy(q,p,sizeof(T)*w);
			p+=w;
			q+=pitch;
		}
	}
	T* GetDataPtr()
	{
		return _buf.data();
	}

	// algorithm from http://www.gamedev.net/reference/programming/features/imageproc/page2.asp
	void Blur(i_math::rect<T_RectElem> &rc0,DWORD order)
	{
		i_math::rect<T_RectElem> rc=rc0;
		if (!rc.isValid())
			rc.set(0,0,_w,_h);


		std::vector<float>gauss_fact;
		float gauss_sum;
		int gauss_width;

		//calculate gauss fact
		if (TRUE)
		{
			std::vector<float>t;
			gauss_fact.push_back(1.0f);
			for (int i=1;i<=order;i++)
			{
				t.resize(i+1);
				for (int j=0;j<i+1;j++)
				{
					if (j==0)
						t[j]=gauss_fact[0];
					else
					{
						if (j==i)
							t[j]=gauss_fact[j-1];
						else
							t[j]=gauss_fact[j-1]+gauss_fact[j];
					}
				}
				gauss_fact=t;
			}
		}

		//gauss sum
		gauss_sum=(float)(1<<order);

		gauss_width=gauss_fact.size();

		i_math::rect<T_RectElem> rcMap;
		rcMap.set(0,0,_w,_h);
		i_math::rect<T_RectElem> rc1;
		if (TRUE)
		{
			rc1=rc;
			int b=gauss_width/2+1;
			rc1.inflate(b,b,b,b);
			rc1.clipAgainst(rcMap);
		}

		std::vector<T> buf1;
		buf1.resize(_buf.size());

		int x,y;

		float sum_f;
		T sum;

		for (int j=0;j<rc1.getHeight();j++)
		for (int i=0;i<rc1.getWidth();i++)
		{
			sum=_zero;
			sum_f=0.0f;

			x=i+rc1.Left();
			y=j+rc1.Top();

			for (int k=0;k<gauss_width;k++)
			{
				int xx;
				xx=x-((gauss_width-1)>>1)+k;
				if (rcMap.isPointInside(xx,y))
				{
					sum=sum+_buf[y*_w+xx]*gauss_fact[k];
					sum_f+=gauss_fact[k];
				}
			}

			buf1[y*_w+x]=sum/sum_f;
		}

		for (int j=0;j<rc.getHeight();j++)
		for (int i=0;i<rc.getWidth();i++)
		{
			sum=_zero;
			sum_f=0.0f;

			x=i+rc.Left();
			y=j+rc.Top();

			for (int k=0;k<gauss_width;k++)
			{
				int yy;
				yy=y-((gauss_width-1)>>1)+k;
				if (rcMap.isPointInside(x,yy))
				{
					sum=sum+buf1[yy*_w+x]*gauss_fact[k];
					sum_f+=gauss_fact[k];
				}
			}

			_buf[y*_w+x]=sum/sum_f;
		}
	}

protected:
	T _zero;

	std::vector<T> _buf;
	T_RectElem  _w,_h;
};