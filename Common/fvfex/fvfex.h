#pragma once

#include "fvfex_type.h"

struct FVFExInfo
{
	FVFExInfo(FVFEx fvf,int type,DWORD size,int usage,int usageindex,const char *sem,DWORD fmt,const char *nmGL,int countGL,DWORD fmtGL,BOOL bNormalizeGL)
	{
		m_fvf=fvf;
		m_type=type;
		m_size=size;
		m_usage=usage;
		m_usageindex=usageindex;
		m_sem=sem;
		m_fmt=fmt;
		m_nmGL=nmGL;
		m_countGL=countGL;
		m_fmtGL=fmtGL;
		m_bNormalizeGL=bNormalizeGL;
	}
	FVFEx m_fvf;
	int m_type;
	DWORD m_size;
	int m_usage;
	int m_usageindex;

	const char *m_sem;
	DWORD m_fmt;

	const char *m_nmGL;
	int m_countGL;
	DWORD m_fmtGL;
	BOOL m_bNormalizeGL;
};



struct D3DVERTEXELEMENT9_x;

extern BOOL fvfCheck(FVFEx fvf);
extern DWORD fvfSize(FVFEx fvf);//得到某个顶点格式的数据大小,in byte
extern int fvfOffset(FVFEx fvf,FVFEx fvfPart);//得到顶点格式中某个element的偏移量,in byte,return -1 if error
extern FVFEx fvfFirst(FVFEx fvf);//找到顶点格式中第一个element
extern FVFEx fvfFirstPos(FVFEx fvf);
extern FVFEx fvfFirstNormal(FVFEx fvf);
extern FVFEx fvfFirstTex(FVFEx fvf);
extern FVFEx fvfFirstVox(FVFEx fvf);
extern int fvfToD3DVERTEXELEMENT9(FVFEx fvfTotal,FVFEx fvf,D3DVERTEXELEMENT9_x *pElements,int iStream);


extern void fvfCopy(DWORD nVertice,void *pDest,FVFEx fvfDest,void *pSrc,FVFEx fvfSrc);//copy all the copyable elements from source buffer to dest buffer
extern void fvfCopyByStride(DWORD nVertice,FVFEx fvfSrc,void *pDest,DWORD nStrideDest,void *pSrc,DWORD nStrideSrc);
extern void fvfInterpolate(DWORD nVertice,FVFEx fvf,void *pDest,void *pSrc1,void *pSrc2,float r);//


