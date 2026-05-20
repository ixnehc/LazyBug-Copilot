#pragma once

#pragma warning(disable:4018)
#pragma warning(disable:4267)
#pragma warning(disable:4312)

#pragma warning(disable:4996)
#pragma warning(disable:4819)


#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif
#ifndef SAFE_ADDREF
#define SAFE_ADDREF(p)      { if(p) { (p)->AddRef();} }
#endif

#define SAFE_DESTROY(p)	{ if(p) { (p)->Destroy(); (p)=NULL; } }

#define SAFE_REPLACE(dest,src)									\
{																						\
	SAFE_ADDREF(src);														\
	SAFE_RELEASE(dest);													\
	memcpy(&dest,&src,sizeof(src));								\
}




#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))


#define ColorAlpha(c,a) (((c)&0x00ffffff)|(a)<<24)
#define ColorAlpha_Alpha(v) ((v)>>24)
#define ColorAlpha_Color(v) ((v)&0x00ffffff)

//0111 1110 0000
#define To565(col) ((WORD)((((col)&0xff0000)>>19<<11)|(((col)&0xff00)>>10<<5)|(((col)&0xff)>>3)))
#define From565(col) (DWORD)(((((col)>>11)<<19)|(((col)&0x7e0)<<5)|(((col)&0x1f)<<3))|0xff000000)

inline DWORD COLOR_SWAP_RB(DWORD v)
{
	DWORD t=v;
	BYTE *__p=(BYTE*)&(t);
	BYTE __t;
	__t=__p[0];
	__p[0]=__p[2];
	__p[2]=__t;
	return t;
}


#ifndef INTERFACE_REFCOUNT
#define INTERFACE_REFCOUNT \
virtual int AddRef()=0;\
virtual int Release()=0;\
virtual void ReleaseAll()=0
#endif

#ifndef IMPLEMENT_REFCOUNT
#define IMPLEMENT_REFCOUNT \
struct __refcount\
{\
	__refcount(){refcount=0;}\
	int refcount;\
};\
virtual int AddRef(){return ++__rc.refcount;}\
virtual int Release()\
{\
	__rc.refcount--;\
	if (__rc.refcount<=0)\
	{\
		delete this;\
		return 0;\
	}\
	return __rc.refcount;\
}\
virtual void ReleaseAll()\
{\
	__rc.refcount=0;\
	delete this;\
}\
int GetRef()\
{\
	return __rc.refcount;\
}\
__refcount __rc
#endif

#ifndef IMPLEMENT_REFCOUNT_C
#define IMPLEMENT_REFCOUNT_C \
struct __refcount\
{\
	__refcount(){refcount=0;}\
	int refcount;\
};\
__refcount __rc;\
virtual int AddRef(){return ++__rc.refcount;}\
virtual int Release()\
{\
	__rc.refcount--;\
	if (__rc.refcount<=0)\
	{\
		Class_Delete(this);\
		return 0;\
	}\
	return __rc.refcount;\
}\
virtual void ReleaseAll()\
{\
	__rc.refcount=0;\
	Class_Delete(this);\
}\
int GetRef()\
{\
	return __rc.refcount;\
}\
void SetRef(int v)\
{\
	__rc.refcount=v;\
}
#endif


//Need an additional OnRelease() implement
#ifndef IMPLEMENT_REFCOUNT_OVERRIDE
#define IMPLEMENT_REFCOUNT_OVERRIDE \
struct __refcount\
{\
	__refcount(){refcount=0;}\
	int refcount;\
};\
	virtual int AddRef(){return ++__rc.refcount;}\
	virtual int Release()\
{\
	__rc.refcount--;\
	if (__rc.refcount<=0)\
	{\
		OnRelease();\
		return 0;\
	}\
	return __rc.refcount;\
}\
virtual void ReleaseAll()\
{\
	__rc.refcount=0;\
	OnRelease();\
}\
int GetRef()\
{\
	return __rc.refcount;\
}\
void SetRef(int v)\
{\
	__rc.refcount=v;\
}\
__refcount __rc
#endif



#define PT_IN_RECT(rc,pt) (((rc).left<=(pt).x)&&((rc).right>(pt).x)&&((rc).top<=(pt).y)&&((rc).bottom>(pt).y))
#define RECT_ADD_POINT(rc,pt) {(rc).left+=(pt).x;(rc).right+=(pt).x;(rc).top+=(pt).y;(rc).bottom+=(pt).y;}
#define RECT_WIDTH(rc) ((rc).right-(rc).left)
#define RECT_HEIGHT(rc) ((rc).bottom-(rc).top)
#define RECT_SET(rc,l,t,r,b) {(rc).left=(l);(rc).top=(t);(rc).right=(r);(rc).bottom=(b);}
#define RECT_NORMALIZE(rc) {(rc).right-=(rc).left;(rc).bottom-=(rc).top;(rc).left=0;(rc).top=0;}
#define RECT_ISEMPTY(rc) ((RECT_WIDTH(rc)<=0)||(RECT_HEIGHT(rc)<=0))
#define RECT_INTERSECT(rc,rc2) {::IntersectRect(&(rc),&(rc),&(rc2));}


#define CYCLE_VALUE(v,range) \
if ((v)>=(range))\
	(v)%=(range);\
else\
{\
	if ((v)<0)\
	{\
		(v)=(-v)%(range);\
		if ((v)!=0)\
			(v)=(range)-(v);\
	}\
}

inline int FloatToNearestInt(float f)
{
	if (f>=0.0f)
		return (int)(f+0.5f);
	else
		return (int)(f-0.5f);
}

template<class T>
void Swap(T &v1,T &v2)
{
	T t;
	t=v1;
	v1=v2;
	v2=t;
}

#define FORCE_TYPE(T,v) (*(T*)&(v))

//异步返回结果
enum AResult
{
	A_Fail=0,//失败
	A_Pending=1,//等待
	A_Ok=2,//成功
};

#define EMPTYNESS 

#define _1Base(v) (v)+1
#define _0Base(v) (v)-1
#define to_1Base(v) (v)++;
#define to_0Base(v) (v)--;

//产生序列: 0,step,-step,2*step,-2*step,3*step,-3*step,...
inline float GenScatteringStepValue(int idx, float step)
{
	if (idx==0)
		return 0.0f;
	float v=step*(float)((idx+1)/2);
	if (idx%2==0)
		v=-v;
	return v;
}