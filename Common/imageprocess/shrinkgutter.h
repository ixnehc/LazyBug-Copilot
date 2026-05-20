#pragma once

#include <vector>

#include "../math/rect.h"

//T要有一个bValid的成员变量
//T要支持operator+(T &src)和operator*(float scale)
//T在构造函数里要把bValid设为0,并将原始值设为0
template<typename T>
void ShrinkGutter(T *mp,int w,int h)
{
	T*p=mp;
	static i_math::pos2di off[4]=
	{
		i_math::pos2di(-1,0),i_math::pos2di(1,0),i_math::pos2di(0,1),i_math::pos2di(0,-1),
	};

	i_math::recti rc(0,0,w,h);
	for (int j=0;j<h;j++)
	for (int i=0;i<w;i++)
	{
		if (p->bValid==0)
		{//a gutter pixel
			T t;
			DWORD count=0;
			for (int k=0;k<ARRAY_SIZE(off);k++)
			{
				i_math::pos2di pt(i+off[k].x,j+off[k].y);
				if (!rc.isPointInside(pt))
					continue;

				T *q=p+off[k].x+off[k].y*w;
				if (q->bValid==1)
				{//A pixel
					t=t+(*q);
					count++;
				}
			}

			if (count>0)
			{
				*p=t*(1/(float)count);//this gutter is near to at least a pixel
				p->bValid=2;//mark as gutter edge
			}
		}
		p++;
	}

	p=mp;
	//shrink(convert gutter edge to pixel)
	for (int i=0;i<w*h;i++)
	{
		if (p->bValid==2)
			p->bValid=1;
		p++;
	}

}
