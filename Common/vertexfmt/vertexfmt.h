#pragma once

#include "../fvfex/fvfex_type.h"

#define FVFEx_Basic (FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_FLAG_TEX0)
struct VtxBasic
{
	i_math::vector3df pos;
	i_math::vector3df normal;
	i_math::vector2df uv0;
};

#define FVFEx_PosColor (FVFEX_XYZ0|FVFEX_DIFFUSE)
struct VtxPosColor
{
	i_math::vector3df pos;
	DWORD color;
};

#define FVFEX_DECAL (FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_BINORMAL|FVFEX_TANGENT|FVFEX_FLAG_TEX0|FVFEX_FLAG_TEX1)
struct VtxDecal
{
	i_math::vector3df pos;
	i_math::vector3df normal;
	i_math::vector3df binormal;
	i_math::vector3df tangent;
	i_math::vector2df uv0;
	i_math::vector2df uv1;
};



//the vertex format used in 2D screen space quad
#define FVFEx_Quad (FVFEX_XYZ0|FVFEX_DIFFUSE|FVFEX_FLAG_TEX0)
struct VtxQuad
{
	i_math::vector3df pos;
	DWORD color;
	i_math::vector2df uv;
};

template<typename T_Vtx,typename T_Rect,typename T_Size>
void MakeQuad(T_Vtx *vertices,WORD *indices,WORD vbase,
							T_Rect &rc,float scale,
							T_Rect &rcOnTex,T_Size&szTex,
							DWORD color)
{
	if (indices)
	{
		indices[0]=vbase;
		indices[1]=vbase+1;
		indices[2]=vbase+2;
		indices[3]=vbase+1;
		indices[4]=vbase+3;
		indices[5]=vbase+2;
	}

	//The dest
	i_math::rectf rcF;
	rcF.Left()=((float)(int)(scale*(float)rc.Left()+0.5f))-0.5f;
	rcF.Top()=((float)(int)(scale*(float)rc.Top()+0.5f))-0.5f;
	rcF.Right()=((float)(int)(scale*(float)rc.Right()+0.5f))-0.5f;
	rcF.Bottom()=((float)(int)(scale*(float)rc.Bottom()+0.5f))-0.5f;

	//至少得画出来
	if (rcF.Right()<rcF.Left()+1.0f)
		rcF.Right()=rcF.Left()+1.0f;
	if (rcF.Bottom()<rcF.Top()+1.0f)
		rcF.Bottom()=rcF.Top()+1.0f;

	vertices[0].pos.set((float)rcF.Left(),(float)rcF.Top(),0.5f);
	vertices[1].pos.set((float)rcF.Right(),(float)rcF.Top(),0.5f);
	vertices[2].pos.set((float)rcF.Left(),(float)rcF.Bottom(),0.5f);
	vertices[3].pos.set((float)rcF.Right(),(float)rcF.Bottom(),0.5f);

	//The src
	i_math::rectf rcTexF;
	if (!rcOnTex.isValid())
		rcTexF.set(0,0,1,1);
	else
	{
		rcTexF.Left()=((float)rcOnTex.Left())/(float)szTex.w;
		rcTexF.Top()=((float)rcOnTex.Top())/(float)szTex.h;
		rcTexF.Right()=((float)rcOnTex.Right())/(float)szTex.w;
		rcTexF.Bottom()=((float)rcOnTex.Bottom())/(float)szTex.h;
	}

	vertices[0].uv.set(rcTexF.Left(),rcTexF.Top());
	vertices[1].uv.set(rcTexF.Right(),rcTexF.Top());
	vertices[2].uv.set(rcTexF.Left(),rcTexF.Bottom());
	vertices[3].uv.set(rcTexF.Right(),rcTexF.Bottom());

	vertices[0].color=vertices[1].color=vertices[2].color=vertices[3].color=color;
}


template<typename T_Vtx,typename T_Rect,typename T_Size>
void MakeQuad_NoDestSnap(T_Vtx *vertices,WORD *indices,WORD vbase,
			  T_Rect &rc,float scale,
			  T_Rect &rcOnTex,T_Size&szTex,
			  DWORD color)
{
	if (indices)
	{
		indices[0]=vbase;
		indices[1]=vbase+1;
		indices[2]=vbase+2;
		indices[3]=vbase+1;
		indices[4]=vbase+3;
		indices[5]=vbase+2;
	}

	//The dest
	i_math::rectf rcF;
	rcF.Left()=((scale*(float)rc.Left()+0.5f))-0.5f;
	rcF.Top()=((scale*(float)rc.Top()+0.5f))-0.5f;
	rcF.Right()=((scale*(float)rc.Right()+0.5f))-0.5f;
	rcF.Bottom()=((scale*(float)rc.Bottom()+0.5f))-0.5f;

	//至少得画出来
	if (rcF.Right()<rcF.Left()+1.0f)
		rcF.Right()=rcF.Left()+1.0f;
	if (rcF.Bottom()<rcF.Top()+1.0f)
		rcF.Bottom()=rcF.Top()+1.0f;

	vertices[0].pos.set((float)rcF.Left(),(float)rcF.Top(),0.5f);
	vertices[1].pos.set((float)rcF.Right(),(float)rcF.Top(),0.5f);
	vertices[2].pos.set((float)rcF.Left(),(float)rcF.Bottom(),0.5f);
	vertices[3].pos.set((float)rcF.Right(),(float)rcF.Bottom(),0.5f);

	//The src
	i_math::rectf rcTexF;
	if (!rcOnTex.isValid())
		rcTexF.set(0,0,1,1);
	else
	{
		rcTexF.Left()=((float)rcOnTex.Left())/(float)szTex.w;
		rcTexF.Top()=((float)rcOnTex.Top())/(float)szTex.h;
		rcTexF.Right()=((float)rcOnTex.Right())/(float)szTex.w;
		rcTexF.Bottom()=((float)rcOnTex.Bottom())/(float)szTex.h;
	}

	vertices[0].uv.set(rcTexF.Left(),rcTexF.Top());
	vertices[1].uv.set(rcTexF.Right(),rcTexF.Top());
	vertices[2].uv.set(rcTexF.Left(),rcTexF.Bottom());
	vertices[3].uv.set(rcTexF.Right(),rcTexF.Bottom());

	vertices[0].color=vertices[1].color=vertices[2].color=vertices[3].color=color;
}


//注意,vertices和indices必须为足够大的buffer
template<typename T_Vtx,typename T_Rect>
void MakeHourglass(T_Vtx *vertices,DWORD &nVertices,WORD *indices,DWORD &nIndices,float ratio,T_Rect &rc,DWORD color,BOOL bInvert)
{
	nVertices=0;
	nIndices=0;
	const float pi = 3.14159265358979323846f;

	float pre_ratio = ratio;
	if (bInvert)
		pre_ratio=1.0f-pre_ratio;

	float radian = 2.0f*pi*pre_ratio;

	T_Rect rcScreen  = rc;

	float w = 0.5f*(float)rcScreen.getWidth();
	float h = 0.5f*(float)rcScreen.getHeight();
	float r = sqrtf(w*w+h*h);

	T_Vtx center;
	center.pos.x = rcScreen.UpperLeftCorner.x + w;
	center.pos.y = rcScreen.UpperLeftCorner.y + h;
	center.pos.z = 0.5f;
	center.color = color;
	center.uv.set(0.5f,0.5f);

	BOOL bX = (radian>pi);
	BOOL bY = (radian>=0.5f*pi&&radian<1.5f*pi);

	DWORD quadrant = 1;

	float rad = radian;
	if(radian>0.5f*pi&&radian<=pi)
	{
		rad = pi - radian;
		quadrant = 2;
	}

	if (radian>pi&&radian<=1.5f*pi)
	{
		rad = radian - pi;
		quadrant = 3;
	}

	if(radian>1.5f*pi)
	{
		rad = 2.0f*pi - radian;
		quadrant = 4;
	}


	float c0 = h/r;
	float c = cosf(rad);
	BOOL bSec = FALSE;

	T_Vtx v;
	if(c>c0)
	{
		v.pos.x = tan(rad)*h;
		v.pos.y = -h;	
	}
	else
	{
		v.pos.x = r*sinf(rad);
		v.pos.y = -r*cosf(rad);
		float d = v.pos.x - w;
		v.pos = w*v.pos/(d + w);
		bSec = TRUE;
	}

	if(bX) v.pos.x = - v.pos.x;
	if(bY) v.pos.y = - v.pos.y;

	v.uv.set(v.pos.x/w,v.pos.y/h);
	v.pos += center.pos;
	v.uv.x = 0.5f*v.uv.x + 0.5f;
	v.uv.y = 0.5f*v.uv.y + 0.5f;
	v.color = center.color;

	T_Vtx cy;
	cy.pos = center.pos;
	cy.color = center.color;
	cy.uv.set(0.5f,0.0f);
	cy.pos.y -= h;

#define OnStepUp_FillIndex(i0,i1,i2)				\
	{																	\
	indices[nIndices++]=(i0);							\
	indices[nIndices++]=(i1);							\
	indices[nIndices++]=(i2);							\
	}

	T_Vtx cor[4];
	cor[0].pos.set(w, h,0);
	cor[0].uv.set(1.0,1.0f);
	cor[1].pos.set(w,-h,0);
	cor[1].uv.set(1.0f,0.0f);
	cor[2].pos.set(-w,-h,0);
	cor[2].uv.set(0,0.0f);
	cor[3].pos.set(-w,h,0);
	cor[3].uv.set(0,1);
	for(int i = 0;i<4;i++)
	{
		cor[i].pos += center.pos;
		cor[i].color = center.color;
	}

	switch(quadrant)
	{
		case 1:
		{
			if(bSec)
			{
				vertices[nVertices++]=(v);		//0
				vertices[nVertices++]=(cor[0]); //1
				vertices[nVertices++]=(center); //2
				vertices[nVertices++]=(cor[3]); //3
				vertices[nVertices++]=(cor[2]); //4
				vertices[nVertices++]=(cy);     //5

				OnStepUp_FillIndex(0,1,2)
				OnStepUp_FillIndex(1,3,2)
				OnStepUp_FillIndex(3,4,2)
				OnStepUp_FillIndex(4,5,2)
			}
			else
			{
				vertices[nVertices++]=(v);		//0
				vertices[nVertices++]=(cor[1]); //1
				vertices[nVertices++]=(center); //2
				vertices[nVertices++]=(cor[0]); //3
				vertices[nVertices++]=(cor[3]); //4
				vertices[nVertices++]=(cor[2]); //5
				vertices[nVertices++]=(cy);		//6	
				OnStepUp_FillIndex(0,1,2)
				OnStepUp_FillIndex(1,3,2)
				OnStepUp_FillIndex(3,4,2)
				OnStepUp_FillIndex(4,5,2)
				OnStepUp_FillIndex(5,6,2)
			}
			break;
		}
		case 2:
		{
			if(bSec)// == 1 && bsec
			{
				vertices[nVertices++]=(v);		//0
				vertices[nVertices++]=(cor[0]); //1
				vertices[nVertices++]=(center); //2
				vertices[nVertices++]=(cor[3]); //3
				vertices[nVertices++]=(cor[2]); //4
				vertices[nVertices++]=(cy);     //5

				OnStepUp_FillIndex(0,1,2)
				OnStepUp_FillIndex(1,3,2)
				OnStepUp_FillIndex(3,4,2)
				OnStepUp_FillIndex(4,5,2)

			}
			else
			{
				vertices[nVertices++]=(v);     //0
				vertices[nVertices++]=(cor[3]); //1
				vertices[nVertices++]=(center); //2
				vertices[nVertices++]=(cor[2]); //3
				vertices[nVertices++]=(cy);      //4

				OnStepUp_FillIndex(0,1,2)
				OnStepUp_FillIndex(1,3,2)
				OnStepUp_FillIndex(3,4,2)
			}
			break;
		}
		case 3:
		{
			if(bSec) 
			{
				vertices[nVertices++]=(v);      //0
				vertices[nVertices++]=(cor[2]); //1
				vertices[nVertices++]=(center); //2
				vertices[nVertices++]=(cy);     //3

				OnStepUp_FillIndex(0,1,2)
				OnStepUp_FillIndex(1,3,2)
			}
			else
			{
				vertices[nVertices++]=(v);      //0
				vertices[nVertices++]=(cor[3]); //1
				vertices[nVertices++]=(center); //2
				vertices[nVertices++]=(cor[2]); //3
				vertices[nVertices++]=(cy);     //4

				OnStepUp_FillIndex(0,1,2)
				OnStepUp_FillIndex(1,3,2)
				OnStepUp_FillIndex(3,4,2)
			}
			break;
		}
		case 4:
		{	
			if(bSec)	// == 3&& !bSec
			{
				vertices[nVertices++]=(v);      //0
				vertices[nVertices++]=(cor[2]); //1
				vertices[nVertices++]=(center); //2
				vertices[nVertices++]=(cy);     //3

				OnStepUp_FillIndex(0,1,2)
				OnStepUp_FillIndex(1,3,2)
			}
			else
			{	
				vertices[nVertices++]=(v);		//0
				vertices[nVertices++]=(cy);		//1
				vertices[nVertices++]=(center); //2

				OnStepUp_FillIndex(0,1,2)
			}
			break;
		}
		default :break;
	}

	if (bInvert)
	{
		for (int i=0;i<nVertices;i++)
			vertices[i].pos.x=center.pos.x-(vertices[i].pos.x-center.pos.x);
	}

}



// 顶点格式 ：(位置+索引)	 颜色  粒子寿命 大小 (旋转轴+旋转角度)
#define FVFEx_Particle (FVFEX_XYZW0|FVFEX_DIFFUSE|FVFEX_WEIGHT01|FVFEX_FLAG_TEX0|FVFEX_FLAG_QUX1)

struct ParticleVertex
{
	i_math::vector4df pos;
	DWORD color;
	float age;//0..1
	i_math::vector2df sz;
	//dir的意义根据粒子的渲染方式不同而不同,
	//在Billboard和Facing模式下,dir的前3个值没有用,最后一个值表示旋转角度
	//在SpeedAlign模式下,前三个值表示速度的方向,最后一个值没有用,
	//在Free模式下,前三个值表示旋转轴 最后一个值是旋转角度
	i_math::vector4df dir;
};

#define FVFEx_MeshParticle (FVFEX_XYZW0|FVFEX_DIFFUSE|FVFEX_WEIGHT01|FVFEX_FLAG_TEX0|FVFEX_FLAG_QUX1|FVFEX_FLAG_QUX2)
struct MeshParticleVtx
{
	i_math::vector4df pos;//w为scale
	DWORD color;
	float age;
	i_math::vector2df sz;
	i_math::vector4df rot;//前三个值表示旋转轴 最后一个值是旋转角度
	i_math::vector4df localpos;//xyz为local坐标
};


#define FVFEx_Cloth (FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_FLAG_TEX0)
struct VtxCloth
{
	i_math::vector3df pos;
	i_math::vector3df normal;
// 	i_math::vector3df binormal;
// 	i_math::vector3df tangent;
	i_math::vector2df uv0;
};
