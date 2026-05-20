
#pragma once
#include "IRenderSystem.h"

#include "shaderlib/SLEffectParam.h"
#include "resdata/ResDataDefines.h"

#include "IShader.h"

struct SurfHandle;
class IRenderSystem;
class IRenderer;
class ICamera;
struct DrawTextureArg;
struct DrawFontArg;
class ITexture;
class ITextPiece;
struct FeatureCode;
class ILight;

struct DrawTextureArg
{
	DrawTextureArg()
	{
		color=0xffffffff;
		rcDest.set(0,0,0,0);
		rcSrc.set(0,0,0,0);
		bDestRect=FALSE;
		bSrcRect=FALSE;
		bForceOpaque=FALSE;
	}
	void SetColor(DWORD c)	{		color=c;	}
	void SetSrc(int left,int top,int right,int bottom)	{		rcSrc.set(left,top,right,bottom);		bSrcRect=TRUE;	}
	void SetSrc_Total()	{		bSrcRect=FALSE;	}
	void SetDest(int left,int top,int right,int bottom)	{		rcDest.set(left,top,right,bottom);		bDestRect=TRUE;	}
	void SetDest(int left,int top)	{		rcDest.set(left,top,0x7fffffff,0x7fffffff);		bDestRect=FALSE;	}
	void SetForceOpaque(BOOL bOpaque=TRUE)	{		bForceOpaque=bOpaque;	}
	DWORD color;
	i_math::recti rcDest;
	i_math::recti rcSrc;
	BOOL bDestRect;//if bDestRect is FALSE,only rcDest.Left(),rcDest.Top() is valid,indicating the position
	BOOL bSrcRect;//if bSrcRect is FALSE,rcSrc is totally not valid
	BOOL bForceOpaque;//whether ignore the alpha channel effect
};


class ICamera;
struct PostProcessArg
{
	PostProcessArg()
	{
		cam=NULL;
		memset(maps,0,sizeof(maps));
		bDestRect=FALSE;
		rcDest.set(0,0,0,0);
		rcSrc.set(0,0,1,1);
	}
	void SetDest(int l,int t,int r,int b)
	{
		bDestRect=TRUE;
		rcDest.set(l,t,r,b);
	}
	void SetDest_Total()//set to total port size
	{
		bDestRect=FALSE;
		rcDest.set(0,0,0,0);
	}
	void SetSrc(float l,float t,float r,float b)	{		rcSrc.set(l,t,r,b);	}
	void SetSrc_Total()	{		rcSrc.set(0,0,1,1);	}//set to full texture size

	void SetCamera(ICamera *cam_)	{		cam=cam_;	}

	void SetMap(DWORD iMap,ITexture *tex)
	{
		if (iMap>=ARRAY_SIZE(maps))
			return;
		maps[iMap]=tex;
	}

	void SetState(ShaderState state_)	{		state=state_;	}

	ICamera *cam;

	ITexture *maps[6];

	i_math::recti rcDest;
	BOOL bDestRect;//if bDestRect is FALSE,use full rect of target port,rcDest is totaly invalid
	i_math::rectf rcSrc;

	EffectParamPacket<1024> epk;

	ShaderState state;

	//XXXXX more post process routine
};


class IRenderPort
{
public:
	INTERFACE_REFCOUNT;

	virtual IRenderSystem *GetRS()=0;

	//NOTE:the returned IRenderer should NOT be kept for later usage,it will be invalid when 
	//you use another port to draw something.So you should use the returned pointer BEFORE
	//using any other render port.
	//Another NOTE:the following functions should not be called on the returned IRenderer pointer:
	//			virtual BOOL SetViewport(ViewportInfo &v);
	//because the viewport are managed by the render port internally.
	//Another NOTE:the returned pointer will also be invalid when you change the rect ,the
	//camera of this render port,or the render target.If they are changed,you should call ObtainRenderer() again
	//to get a valid IRenderer
	virtual IRenderer *ObtainRenderer()=0;

	//[State]--state includes rendertarget,camera,viewport,clip planes
	//State stack
	virtual BOOL PushState()=0;//the pushed state will inherit the camera and rect info of the original state
	virtual BOOL PopState()=0;

	//render target management
	virtual BOOL SetRenderTarget(SurfHandle *rts,DWORD count=1)=0;//pass (NULL,0) to clear RenderTarget
	virtual BOOL SetDSBuffer(SurfHandle &ds)=0;//pass an empty SurfHandle to clear DS buffer


	//Viewport management
	virtual void SetRect(int left,int top,int right,int bottom)=0;
	virtual void SetRect(i_math::recti &rc)=0;
	virtual void SetRect_Total()=0;//Set to full size
	virtual void GetRect(i_math::recti &rc)=0;

	//camera
	virtual ICamera* QueryCamera()=0;//query to modify
	virtual ICamera* GetCamera()=0;//get to read info
	virtual BOOL SetCamera(ICamera *cam)=0;
	virtual ICamera* GetOrthoCamera()=0;//get the 2D drawing camera
	virtual BOOL AdjustCameraRatio(ICamera *cam)=0;//调整camera的aspect ratio以适应这个render port

	//indicate whether automatically adjust the camera's aspect ratio 
	//based on the viewport size
	virtual void SetAutoRatio(BOOL bAutoRatio)=0;


	//[Operations]
	//Buffer 
	virtual BOOL FillColor(DWORD col)=0;
	virtual BOOL FillColor(i_math::vector4df &col)=0;
	virtual BOOL FillColor(i_math::recti &rc,i_math::vector4df &col)=0;
	virtual BOOL ClearBuffer(ClearBufferFlag flag,DWORD col=0,float z=1,DWORD s=0)=0;  

	//2D draw
	virtual BOOL Line(int x1,int y1,int x2,int y2,DWORD col)=0;
	virtual BOOL Lines(i_math::pos2di *lines,DWORD count,DWORD col)=0;//count为线段的个数
	virtual BOOL FrameRect(i_math::recti &rc,DWORD col)=0;
	virtual BOOL FillRect(i_math::recti &rc,DWORD col)=0;
	virtual BOOL DrawTexture(ITexture *pTex,const DrawTextureArg &arg)=0;
	virtual BOOL DrawText(const char *str,const DrawFontArg &arg)=0;
	virtual BOOL CalcDrawText(const char *str,const DrawFontArg &arg,i_math::size2di &sz)=0;
	virtual BOOL DrawText(ITextPiece *piece,const DrawFontArg &arg)=0;
	virtual BOOL CalcDrawText(ITextPiece *piece,const DrawFontArg &arg,i_math::size2di &sz)=0;

	//PostProcess
	virtual BOOL PostProcess(const char *nameLib,const char *nameUF,const PostProcessArg &arg)=0;
	virtual BOOL PostProcess(const char *nameLib,FeatureCode &fc,const PostProcessArg &arg)=0;
	virtual BOOL PostProcess(IShader *shader,const PostProcessArg &arg)=0;


	//3D draw
	//NOTE: for the following drawing functions,if state is NULL,will used the default state
	//(using alpha blending,z-enabled)
	virtual BOOL Points(i_math::vector3df *points,DWORD count,DWORD col,ShaderState *state=NULL)=0;
	virtual BOOL Points(i_math::vector3df *points,DWORD count,DWORD *cols,ShaderState *state=NULL)=0;
	virtual BOOL Line(i_math::vector3df &v1,i_math::vector3df &v2,DWORD col,ShaderState *state=NULL)=0;
	virtual BOOL Lines(i_math::vector3df *lines,DWORD count,DWORD col,ShaderState *state=NULL)=0;//2*count points stored in lines
	virtual BOOL LinesStrip(i_math::vector3df *lines,DWORD count,DWORD col,ShaderState *state=NULL) = 0;//count: vtx count 
	virtual BOOL Lines(i_math::vector3df *lines,DWORD count,DWORD *cols,ShaderState *state=NULL)=0;//2*count points stored in lines,cols contains 2*count color
	virtual BOOL Triangles(i_math::vector3df *tris,DWORD count,DWORD col,ShaderState *state=NULL)=0;//3*count points stored in tris
	virtual BOOL Triangles(i_math::vector3df *tris,DWORD count,DWORD *cols,ShaderState *state=NULL)=0;//3*count points stored in tris,cols contains 3*count color
	virtual BOOL DrawFrame(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col=0xffffffff)=0;//如果indices为NULL,不使用index
	virtual BOOL DrawFace(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col=0xffffffff)=0;//如果indices为NULL,不使用index

	virtual BOOL SimpleDrawMesh(IMesh *mesh,i_math::matrix43f &mat,DWORD col=0xffffffff,BOOL bWireframe=FALSE,IMtrl *mtrl=NULL,ILight *lgt=NULL,ShaderState *state=NULL)=0;
	virtual BOOL SimpleDrawMesh(IMesh *mesh,i_math::matrix43f *mats,DWORD nMats,DWORD col=0xffffffff,BOOL bWireframe=FALSE,IMtrl *mtrl=NULL,ILight *lgt=NULL,ShaderState *state=NULL)=0;

	//some port based calculation
	virtual BOOL TransPos(i_math::vector3df &v,int &x,int &y)=0;//convert a 3D pos to coordinate in this port
	virtual BOOL CalcHitProbe(int x,int y,HitProbe &probe,i_math::f32 length=HITPROBE_DefaultLength)=0;
	virtual BOOL CalcHitVolume(i_math::recti &rc,i_math::volumeCvxf &vol,i_math::f32 length=HITPROBE_DefaultLength)=0;
	virtual BOOL TransAabb(i_math::aabbox3df&aabb,i_math::recti &rc)=0;//convert a 3D aabb to rect in this port

};

struct ViewportInfo;
struct FeatureParam;
struct DrawMeshArg;
struct VBHandles;
class IRenderer
{
public:
	virtual BOOL Begin()=0;
	virtual void End()=0;

	virtual BOOL Render()=0;

	//USAGE of BeginRaw()/EndRaw():
	//当调用完BindXXXX的函数后,渲染的environment就被绑定到renderer中,这时你可以
	//调用Render()来直接画出这些内容,但有时候,我们需要在同一个environment下绘制
	//多次,只修改少数参数(比如vb,某张贴图,matrice),在这种情况下,通过反复调用BindXXX()
	//以及Render()来画的方式显得比较没有效率.所以你可以选择BeginRaw()/EndRaw()
	//来直接得到底层的IShader接口进行操作.
	virtual IShader *BeginRaw(IMtrl *mtrl,int iMtrlLayor=0,IMesh *mesh=NULL,ILight *lgt=NULL,FeatureCode *fc=NULL)=0;
	virtual IShader *BeginRaw(ShaderCode&sc)=0;
	virtual void EndRaw(IShader *shader)=0;

	virtual BOOL ResetContent()=0;//clear all the binding,light,and additional features/feature params

	//Set what/how to render(the rendering environment)
	virtual BOOL BindMesh(IMesh *mesh,DrawMeshArg &dmg)=0;
	virtual BOOL BindVB(VBHandles &vbh,VBBindArg&arg)=0;//NOTE:only prim range and fill mode and vbase in dmg are valid
	virtual BOOL BindMtrl(IMtrl *mtrl,int iLod)=0;
	virtual BOOL BindMats(i_math::matrix43f *mat,DWORD c)=0;
	virtual BOOL BindLight(ILight *l)=0;
	virtual void ClearFeature()=0;//clear all the additional features and effect params
	virtual BOOL AddFeature(FeatureCode &fc)=0;//add a single additional feature
	virtual void AddEP(EffectParam ep,int v)=0;
	virtual void AddEP(EffectParam ep,float v)=0;
	virtual void AddEP(EffectParam ep,i_math::vector2df&v)=0;
	virtual void AddEP(EffectParam ep,i_math::vector3df&v)=0;
	virtual void AddEP(EffectParam ep,i_math::vector4df&v)=0;
	virtual void AddEP(EffectParam ep,i_math::matrix43f&v)=0;
	virtual void AddEP(EffectParam ep,i_math::matrix44f&v)=0;
	virtual void AddEP(EffectParam ep,ITexture *v)=0;
	virtual BOOL RemoveFeature(FeatureCode &fc)=0;

	//Device State
	virtual BOOL SetRenderState(D3DRENDERSTATETYPE state, DWORD value)=0;
};
