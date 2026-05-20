
#pragma once
#include "IResource.h"
#include "fvfex/fvfex_type.h"



#define VBFlag_Immutable 0
#define VBFlag_Dynamic 1

#define VBHANDLE_NULL (NULL)

class IVertexBuffer;
class IIndexBuffer;
struct VBHandles
{
	VBHandles()	{		vb=NULL;		ib=NULL;	}
	VBHandles(IVertexBuffer *vb0,IIndexBuffer *ib0)	{		vb=vb0;		ib=ib0;	}
	BOOL IsEmpty()	{		return vb==NULL;	}
	BOOL IsValid()	{		return vb!=NULL;	}

	IVertexBuffer *vb;
	IIndexBuffer *ib;
};





//fvfDraw is used to indicate how many elements in vb should be sent to the vertex process
//pipeline,default value indicates all the element should be sent.
//primstart/primcount is used to draw a range of vertex in this vb,the default value indicates
//draw all range of the vb. they are all in primitive(triangle,line or point). 
//for example,if start is 3,count is 6,that means the 4th to 9th primitive(triangle,line or point) 
//will be drawn
struct VBBindArg
{
	VBBindArg()
	{
		Zero();
	}
	void Zero()
	{
		iFrame=0;
		fvfDraw=0;
		primstart=-1;
		primcount=-1;
		vcount=0;
		vbase=0;
		fillmode=3;//solid
		dpt=4;//D3DPT_TRIANGLELIST
	}
	void SetFrame(DWORD f)	{		iFrame=f;	}
	void SetFVFDraw(FVFEx fvf)	{		fvfDraw=fvf;	}
	//pass -1,-1 to indicate all the primtive should be drawn
	void SetPrimRange(int s,int c)
	{
		primstart=s;
		primcount=c;
	}
	void SetVertexBase(DWORD base)	{		vbase=base;	}
	void SetFillMode(DWORD mode)	{		fillmode=mode;	}
	void SetDPT(DWORD v)	{		dpt=v;	}

	DWORD iFrame;
	FVFEx fvfDraw;
	int primstart;
	int primcount;
	DWORD fillmode;
	DWORD dpt;//DrawPrimitiveType
	DWORD vbase;
	DWORD vcount;
};

struct VBPatch
{
	VBPatch()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL IsEmpty()
	{
		return (nVtx<=0)||(nIdx<=0);
	}
	void *vtx;
	DWORD nVtx;
	WORD *idx;
	DWORD nIdx;
	FVFEx fvf;
};

class ITexture;
struct TxtPatch:public VBPatch
{
	TxtPatch()
	{
		tex=NULL;
	}
	ITexture *tex;
};

//////////////////////////////////////////////////////////////////////////
//Macro for easier accessing VertexBuffer
typedef i_math::vector3df _VB_XYZ;
typedef i_math::vector3df _VB_NORMAL;
typedef i_math::vector2df _VB_UV;
struct _VB_Color
{
	BYTE r,g,b,a;
};

#define VB_GET_BEGIN(vb,fvf,iFrame)\
	BYTE *___p0,*___p;DWORD ___stride;\
	___p0=___p=(BYTE *)(vb)->GetPtr(fvf,___stride,iFrame);

#define VB_QUERY_BEGIN(vb,fvf,iFrame)\
	BYTE *___p0,*___p;DWORD ___stride;\
	___p0=___p=(BYTE *)(vb)->QueryPtr(fvf,___stride,iFrame);

#define VB_LOCK(vb,fvf,iFrame)\
	BYTE *___p0,*___p;DWORD ___stride;\
	___p0=___p=(BYTE *)(vb)->Lock(___stride,TRUE,fvf,iFrame);

#define VB_UNLOCK(vb)	(vb)->Unlock();


#define VB_GET_OK()\
	(___p!=NULL)

#define VB_QUERY_OK()\
	(___p!=NULL)

#define VB_LOCK_OK()\
	(___p!=NULL)


#define VB_BaseAt(idx)\
	___p=___p0+___stride*(idx);

#define VB_ELEM(type,idx)\
	(*(type*)(___p+___stride*(idx)))

#define VB_XYZ(idx) VB_ELEM(_VB_XYZ,idx)
#define VB_NORMAL(idx) VB_ELEM(_VB_NORMAL,idx)
#define VB_UV(idx) VB_ELEM(_VB_UV,idx)
#define VB_Color(idx) VB_ELEM(_VB_Color,idx)
#define VB_ColorValue(idx) VB_ELEM(DWORD,idx)



class IIndexBuffer:public IResource
{
public:

	//access 
	virtual int GetCount()=0;//return -1 on failure
	virtual int GetSize()=0;//index buffer size in byte,return -1 on failure 
	virtual DWORD GetStride()=0;//size of each index

	virtual void *Lock(BOOL bDiscard)=0;
	virtual void Unlock()=0;
	virtual void UnlockAll()=0;//清除所有的lock的引用计数
};


class IVertexBuffer:public IResource
{
public:
	//access 
	virtual int GetCount()=0;//return -1 on failure
	virtual int GetFrameCount()=0;//return -1 on failure
	virtual int GetSize()=0;//vertex buffer size in byte,return -1 on failure 
	virtual FVFEx GetFVF()=0;//fvf of this vb
	virtual BOOL SetFVF(FVFEx fvf)=0;//修改vb的fvf,注意修改fvf会导致vb的count/framecount/size都发生变化,慎用
									//目前不支持frame count大于1的vb
	virtual DWORD GetStride()=0;//fvf size of this vb

	virtual void *Lock(BOOL bDiscard,FVFEx fvf=0,DWORD iFrame=0)=0;
	virtual void AddDirty(DWORD from,DWORD to)=0;//标记哪些顶点被修改了,from/to都以顶点为单位
	virtual void Unlock()=0;
	virtual void UnlockAll()=0;//清除所有的lock的引用计数

};



class IVertexMgr:public IResourceMgr
{
public:
	virtual IVertexBuffer* CreateVB(DWORD nVertice,FVFEx fvf,DWORD nFrames,DWORD flags=0)=0;
	virtual IIndexBuffer* CreateIB(DWORD nIndice,DWORD flags=0)=0;//flags: VBFlag_XXXX
};

