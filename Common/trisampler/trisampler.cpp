/********************************************************************
	created:	12/16/2009
	filename: 	trisampler
	author:		chenxi
	
	purpose:	triangle sampler,根据uv得到三角形上的采样点
*********************************************************************/
#include "stdh.h"

#include "../commondefines/general_stl.h"

#include "../fvfex/fvfex.h"

#include "trisampler.h"

#include "log/LogDump.h"

#include <assert.h>

void CTriSampler::_Normalize(BYTE *p,DWORD count,DWORD stride)
{
	if (p)
	{
		for(int i=0;i<count;i++)
		{
			((i_math::vector3df *)p)->normalize();
			p+=stride;
		}
	}
}



TriSample *CTriSampler::Build(void *vertices0,DWORD nVertices,FVFEx fvf,FVFEx fvfUV,
													WORD *indices,DWORD nIndices,
													DWORD w,DWORD h,DWORD &nSamples,const char *nmDebug)
{
	nSamples=0;
	if (!(fvf&fvfUV))
		return NULL;

	DWORD stride=fvfSize(fvf);
	void *vertices;

	BYTE *uvs0;
	BYTE *pos0=NULL,*normal0=NULL,*binormal0=NULL,*tangent0=NULL;

	//先把顶点数据拷一份,并作一些预处理
	if (TRUE)
	{
		VEC_SET_BUFFER(_vertices,((BYTE*&)vertices0),stride*nVertices);
		vertices=_vertices.data();

		uvs0=((BYTE*)vertices)+fvfOffset(fvf,fvfUV);

		int off;
		off=fvfOffset(fvf,FVFEX_XYZ0);
		if (off>=0)
			pos0=((BYTE*)vertices)+off;
		off=fvfOffset(fvf,FVFEX_NORMAL0);
		if (off>=0)
			normal0=((BYTE*)vertices)+off;
		off=fvfOffset(fvf,FVFEX_BINORMAL);
		if (off>=0)
			binormal0=((BYTE*)vertices)+off;
		off=fvfOffset(fvf,FVFEX_TANGENT);
		if (off>=0)
			tangent0=((BYTE*)vertices)+off;

		//保证它们都是归一化的
		_Normalize(normal0,nVertices,stride);
		_Normalize(binormal0,nVertices,stride);
		_Normalize(tangent0,nVertices,stride);

		//调整uv比例
		if (TRUE)
		{
			BOOL bOutOfRange=FALSE;
			float ratio=((float)h)/(float)w;
			BYTE *p=uvs0;
			for(int i=0;i<nVertices;i++)
			{
				i_math::texcoordf *t=((i_math::texcoordf *)p);
//				assert((t->x>=-0.0005f)&&(t->x<=1.0005f)&&(t->y>=-0.0005f)&&(t->y<=1.0005f));
				if (!((t->x>=-0.0005f)&&(t->x<=1.0005f)&&(t->y>=-0.0005f)&&(t->y<=1.0005f)))
					bOutOfRange=TRUE;
				t->x=i_math::clamp_f(t->x,-0.0005f,1.0005f);
				t->y=i_math::clamp_f(t->y,-0.0005f,1.0005f);
				t->y*=ratio;
				p+=stride;
			}

			if (bOutOfRange)
			{
				LOG_DUMP_1P("CTriSampler",Log_Error,"uv超出范围!(%s)",nmDebug);
			}
		}

	}

	float tilelen=1.0f/(float)w;
	i_math::recti rcMap(0,0,w,h);

	_samples.clear();

	_host.resize(w*h);
	for (int i=0;i<w*h;i++)
		_host[i].idxTri=-1;

	_trimat.resize(nIndices/3);

	_triarea.resize(nIndices/3);
	
	i_math::triangle3df tri2;
	//for each triangle
	for (int i=0;i<nIndices;i+=3)
	{
		_queue.clear();

		i_math::texcoordf tri[3];
		for (int j=0;j<3;j++)
			tri[j]=*(i_math::texcoordf*)&uvs0[indices[i+j]*stride];

		int idxTri=i/3;
		//build a convert matrix
		_trimat[idxTri].Build(tri);

		if (TRUE)
		{
			tri2.pointA=*(i_math::vector3df*)&pos0[indices[i+0]*stride];
			tri2.pointB=*(i_math::vector3df*)&pos0[indices[i+1]*stride];
			tri2.pointC=*(i_math::vector3df*)&pos0[indices[i+2]*stride];
			_triarea[idxTri].area=tri2.getArea();

			_triarea[idxTri].nSamples=0;
		}


		//occupy the pixels 

		extern void TileByTriangle(float *xys,float tilelength,std::vector<i_math::pos2di>&queueTiles);
		TileByTriangle((float*)tri,tilelen,_queue);

		if (_queue.size()<=0)
			continue;

		i_math::triangle3df tri3D;
		tri3D.pointA.set(tri[0].x,tri[0].y,0);
		tri3D.pointB.set(tri[1].x,tri[1].y,0);
		tri3D.pointC.set(tri[2].x,tri[2].y,0);

		i_math::pos2di *pt0=_queue.data();
		for (int j=0;j<_queue.size();j++)
		{
			i_math::pos2di *pt=&pt0[j];
			if (!rcMap.isPointInside(*pt))
				continue;

			TriangleHost *h=&_host[pt->y*w+pt->x];
			if ((h->idxTri>=0)&&(h->dist<0.0f))
				continue;//already occupied by other triangle

			//now compete it
			i_math::vector3df v3D;
			v3D.x=tilelen*(0.5f+(float)(pt->x));
			v3D.y=tilelen*(0.5f+(float)(pt->y));
			v3D.z=0;

			float dist=(float)tri3D.distanceToPoint(v3D);
			if (dist<=0.0f)
			{
				h->idxTri=idxTri;
				h->dist=-1.0f;
				continue;
			}

			if ((dist<h->dist)||(h->idxTri<0))
			{//nearer than before,occupy it
				h->dist=dist;
				h->idxTri=idxTri;
			}
		}
	}

	TriangleHost *hst=_host.data();

	//累加各个triangle内的采样点
	if (TRUE)
	{
		for (int i=0;i<w*h;i++)
		{
			if (hst[i].idxTri>=0)
				_triarea[hst[i].idxTri].nSamples++;
		}
	}


	hst=_host.data();
	for (int j=0;j<h;j++)
	for (int i=0;i<w;i++)
	{
		if (hst->idxTri<0)
		{
			hst++;
			continue;//not occupied by any triangle
		}

		i_math::vector2df v;
		v.x=tilelen*(0.5f+(float)i);
		v.y=tilelen*(0.5f+(float)j);

		TriangleMatrix *tm=&_trimat[hst->idxTri];
		i_math::vector3df pos[3],normal[3],tangent[3],binormal[3];

		int istart=hst->idxTri*3;

		for (int k=0;k<3;k++)
		{
			if (pos0)
				pos[k]=*(i_math::vector3df *)&pos0[indices[istart+k]*stride];
			if (normal0)
				normal[k]=*(i_math::vector3df *)&normal0[indices[istart+k]*stride];
			if (tangent0)
				tangent[k]=*(i_math::vector3df *)&tangent0[indices[istart+k]*stride];
			if (binormal0)
				binormal[k]=*(i_math::vector3df *)&binormal0[indices[istart+k]*stride];
		}

		TriSample sm;

		if (hst->dist>0.0f)//not totally within the triangle,clamp into it
			tm->Clamp(v);
		if (pos0)
			sm.pos=tm->ConvertPos(v,pos);
		if (normal0)
			sm.normal=tm->ConvertNormal(v,normal);
		if (binormal0)
			sm.binormal=tm->ConvertNormal(v,binormal);
		if (tangent0)
			sm.tangent=tm->ConvertNormal(v,tangent);
		sm.pt.set(i,j);

		if (TRUE)
			sm.area=_triarea[hst->idxTri].GetSampleArea();
		else
			sm.area=0.0f;

		_samples.push_back(sm);

		hst++;
	}

	nSamples=_samples.size();
	return _samples.data();
}


