
#pragma once

struct TexAtlasArg
{
	TexAtlasArg()
	{
		w=h=0;
		rcSrc.set(0,0,0,0);
		filter=2;//D3DTEXF_LINEAR
	}
	DWORD w;
	DWORD h;
	i_math::recti rcSrc;
	int filter;
};


class ITexture;
class ITexAtlas
{
public:
	INTERFACE_REFCOUNT;
	
	virtual BOOL CanBatchWith(ITexAtlas *atlas)=0;//check whether this atlas could be used in a same batch with another one

	virtual BOOL IsValid()=0;

	//Location on the owner texture
	virtual i_math::vector2df GetSize()=0;//both width and height are within(0,1)
	virtual i_math::vector2df GetOff()=0;//within (0,1)
	virtual i_math::rectf GetRect()=0;

	//the owner texture (The texture this atlas belongs to)
	virtual ITexture *GetTex()=0;//the returned tex should not be kept for using unless you add a reference count on it
};

//ITexAtlasPool is used to stitch a lot of small tex atlas into a big texture
//the atlas will be put to more than 1 texture if they are too many.
//each texture could only contains atlas of same size,atlas in different size will be
//put into different textures
class ITexAtlasPool
{
public:
	INTERFACE_REFCOUNT;

	virtual ITexAtlas *Alloc(ITexture *tex)=0;//alloc an atlas with the same size of tex
	//alloc an atlas of size (w x h),scale if needed(use filter)
	virtual ITexAtlas *Alloc(DWORD w,DWORD h,ITexture *tex,int filter=D3DTEXF_LINEAR)=0;
	virtual void FreeAll()=0;//free all the atlas,NOTE: use ITexAtlas::Release() to free a single atlas

	virtual BOOL CheckLeak()=0;//Check whether there is any atlas left not released in the mgr

};

//ITexAtlasMap is also used to stitch a lot of small tex atlas into a big texture
//different from ITexAtlasPool:each ITexAtlasMap have only ONE texture
//all the atlas are put into this texture ,no matter what size they are
//the atlases will be automatically resize if they are too big to be put into the texture,
//with their size ratio to each other kept.
//another difference:the atlas could not be free/alloc,once they are put into the texture,
//they could not be changed.
//NOTE:each atlas will be put into a minimal 2-power-size quad to avoid mipmap artifact
//for example: if a 367*367 atlas is added,it will be put into a 512x512 quad,or 256x256
//if resized to 1/4,or 128x128 if resized to 1/16,...
//NOTE:by now,the tex to add should be equal in width and height
class ITexAtlasMap
{
public:
	INTERFACE_REFCOUNT;
	virtual void BeginStitch()=0;

	//the ITexAtlas returned will contain right content after a successful EndStitch() calling
	//Will return NULL if this is a not-allow-resize ITexAtlasMap and there is no room for the tex
	//Will add ref on tex internally
	virtual ITexAtlas *Add(ITexture *tex)=0;//use default TexAtlasArg value
	virtual ITexAtlas *Add(ITexture *tex,TexAtlasArg &tag)=0;
	virtual BOOL EndStitch()=0;

	virtual ITexture *GetTex()=0;
};

