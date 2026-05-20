#pragma once

#include "../fvfex/fvfex_type.h"

#include "../affinespace/affinespace.h"



struct TriangleArea
{
	float area;//三角形的面积
	int nSamples;//三角形里的采样点
	float GetSampleArea()
	{
		return nSamples>0? (area/(float)nSamples): area;
	}
};




struct TriangleHost
{
	int idxTri;//if -1,not classified to any triangle yet
	float dist;//distance to that triangle,if -1,totally belongs to that triangle
};

struct TriangleMatrix
{
	void Build(i_math::vector2df *uv)
	{
		memcpy(tri,uv,sizeof(tri));

		//build an affine space based on this triangle
		i_math::vector2df edge[3];
		float edgedist[3];
		float angle[3];
		for (int j=0;j<3;j++)
		{
			edgedist[j]=(float)(tri[(j+1)%3]-tri[j]).getLength();
			edge[j]=(tri[(j+1)%3]-tri[j]).normalize();
		}
		for (int j=0;j<3;j++)
			angle[j]=edge[j].dotProduct(-edge[(j+2)%3]);

		//check the angle on each corner
		int j;
		for (j=0;j<3;j++)
		{
			if (angle[j]>=0.9999f)
				break;
		}

		if (j<3)
		{//this corner angle is too small,build a line with this corner and the furthest corner to it 
			O=j;
			if (edgedist[j]>edgedist[(j+2)%3])
				X=(j+1)%3;
			else
				X=(j+2)%3;

			linedir=tri[X]-tri[O];
			linelen=(float)linedir.getLength();
			linedir/=linelen;
			bUsingLine=TRUE;
		}
		else
		{
			O=0;

			float min=2.0f;
			for (int k=0;k<3;k++)
			{
				if (fabsf(angle[k])<min)
				{
					min=fabsf(angle[k]);
					O=k;
				}
			}
			X=(O+1)%3;
			Y=(O+2)%3;

			aspace.Construct(tri[O],tri[X]-tri[O],tri[Y]-tri[O]);
			bUsingLine=FALSE;
		}
	}
	void Clamp(i_math::vector2df &v)
	{
		i_math::triangle3df tri3D;
		tri3D.pointA.set(tri[0].x,tri[0].y,0);
		tri3D.pointB.set(tri[1].x,tri[1].y,0);
		tri3D.pointC.set(tri[2].x,tri[2].y,0);

		i_math::vector3df v3D;
		v3D.set(v.x,v.y,0);
		v3D=tri3D.closestPointOnTriangle(v3D);
		v.set(v3D.x,v3D.y);
	}
	i_math::vector3df ConvertPos(i_math::vector2df &v,i_math::vector3df *triPos)
	{
		i_math::vector3df pos;
		if (bUsingLine)
		{
			float r=i_math::clamp_f(linedir.dotProduct(v-tri[O])/linelen,0,1);
			pos=triPos[X].getInterpolated(triPos[O],r);
		}
		else
		{
			i_math::vector2df coord;
			aspace.CalcCoord(coord,v);
			coord.x=i_math::clamp_f(coord.x,0,1);
			coord.y=i_math::clamp_f(coord.y,0,1);
			pos=triPos[O];
			pos+=(triPos[X]-triPos[O])*coord.x;
			pos+=(triPos[Y]-triPos[O])*coord.y;
		}
		return pos;
	}
	i_math::vector3df ConvertNormal(i_math::vector2df &v,i_math::vector3df *triNormal)
	{
		i_math::vector3df normal;
		if (bUsingLine)
		{
			float r=i_math::clamp_f(linedir.dotProduct(v-tri[O])/linelen,0,1);
			normal=slerpf(triNormal[O],triNormal[X],r);
			normal.normalize();
		}
		else
		{
			i_math::vector2df coord;
			aspace.CalcCoord(coord,v);
			coord.x=i_math::clamp_f(coord.x,0,1);
			coord.y=i_math::clamp_f(coord.y,0,1);

			normal=triNormal[O];
			i_math::quatf qtX,qtY,qt,qtZero;
			qtX.from2Vector(triNormal[O],triNormal[X]);
			qtY.from2Vector(triNormal[O],triNormal[Y]);

			qt.slerp(qtZero,qtX,coord.x);
			normal=qt*normal;
			normal.normalize();

			qt.slerp(qtZero,qtY,coord.y);
			normal=qt*normal;
			normal.normalize();
		}
		return normal;
	}

	i_math::vector2df tri[3];

	AffineSpace2 aspace;
	i_math::vector2df linedir;
	float linelen;

	char O,X,Y;
	bool bUsingLine;//line is used for too slim triangle
};

struct TriSample
{
	i_math::pos2di pt;//像素坐标
	vector3df pos;
	vector3df normal;
	vector3df tangent;
	vector3df binormal;
	float area;
};

class CTriSampler
{
public:

	//nIndices必须是3的倍数
	//返回的TriSample里的方向向量全部是normalize过的
	TriSample *Build(void *vertices,DWORD nVertices,FVFEx fvf,FVFEx fvfUV,WORD *indices,DWORD nIndices,DWORD w,DWORD h,DWORD &nSamples,const char *nmOwner);//nmOwner用来输出调试信息


protected:

	void _Normalize(BYTE *p,DWORD count,DWORD stride);

	std::vector<TriSample> _samples;
	std::vector<BYTE> _vertices;
	std::vector<TriangleHost>_host;
	std::vector<TriangleMatrix>_trimat;
	std::vector<TriangleArea>_triarea;
	std::vector<i_math::pos2di>_queue;

};