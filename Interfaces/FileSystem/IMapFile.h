/********************************************************************
	created:	2008/1/16   12:33
	filename: 	e:\IxEngine\Interfaces\WorldSystem\IWorldSystem.h
	author:		cxi
	
	purpose:	exposed world system interfaces for the world system user
*********************************************************************/

#pragma once
#include "IMapFileDefines.h"

class CProgress;

class ISscSystem;
class IMapFile
{
public:
	INTERFACE_REFCOUNT;

	virtual IMapFile *CreateInstance()=0;//返回指针带一个引用计数

	//note:when a map file is opened,internal cache will be built to accelerate 
	//data loading/saving. cachearea indicates the cache size,in block
	//path should be full path
	virtual BOOL Open(const char *path,BOOL bReadOnly,DWORD cachearea=20000)=0;
	virtual BOOL IsOpened()=0;


	//note:after calling this function,the map file will be left closed
	//path could be either full path or a relative path under WSPath_Map
	virtual BOOL New(const char *path,i_math::recti &rcBound)=0;//rcBound is in block
	virtual BOOL NewByCompare(const char *path,IMapFile *mfOld,IMapFile *mfNew)=0;

	//call this function after this map file is opened
	virtual BOOL Update(IMapFile *mfSrc)=0;//add data from another map file

	//clean up the map file if needed,
	virtual BOOL CleanUp(CProgress *prg)=0;

	virtual void Close()=0;
	virtual BOOL IsReadOnly()=0;

	virtual const char *GetPath()=0;//返回完整路径
	virtual i_math::recti &GetRect()=0;//the map rect,in block
	virtual i_math::recti &GetFieldRect()=0;//field rect,in field
	virtual DWORD GetFieldWidth()=0;//得到field的边长,以block为单位,这个值的平方代表一个field有多少个block

	//ptBlk is world coord
	//IMPORTANT:the returned ptr(data) is temp ptr,and should NOT be kept for later use
	//the difference between Load()&Query(): the latter will mark this block's channel as modified
	//NOTE: if no data in the specified channel,return TRUE,and data/szData will be filled with NULL/0
	//NOTE: if return FALSE,data/szData will be filled with NULL/0
	virtual BOOL Load(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData)=0;
	virtual BOOL Query(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData)=0;

	//ptBlk is world coord
	//pass data/szData as NULL/0 to clear that channel
	virtual BOOL Save(i_math::pos2di &ptBlk,MapChannel ch,BYTE *data,DWORD szData)=0;

	virtual void Flush()=0;//save all the modified to disk
	virtual BOOL Compress(CProgress *prg,CProgress *prgSub)=0;

	//for version control
	virtual const char *GetInfoSscPath()=0;//得到info文件对应的文件名
	virtual const char **PreSscOp(i_math::recti &rcFld,DWORD &nPathes)=0;
	virtual void PostSscOp(i_math::recti &rcFd)=0;
	virtual BOOL FieldCheckedOut(i_math::pos2di &ptFld)=0;//test whether the field is checked out
	virtual BOOL BlockCheckedOut(i_math::pos2di &ptBlk)=0;//test whether the block is checked out
																									//if out of the map range,return FALSE
	virtual const char *GetUniqueSscPath(const char *name)=0;//得到某个unique数据对应的文件名

	virtual BOOL SaveUnique(const char *name,BYTE *data,DWORD szData)=0;
	virtual BOOL LoadUnique(const char *name,BYTE *&data,DWORD &szData)=0;
	virtual DWORD EnumUniques(const char **&names)=0;

	virtual HMapFileCache CreateCache(MapChannel *channels,DWORD nChannel)=0;
	virtual void DestroyCache(HMapFileCache hCache)=0;
	virtual BOOL UpdateCache(HMapFileCache hCache,i_math::recti &rcBlks)=0;//rcBlks is in block


};


#define MapFile_SaveUniqueString(mf,name,str)												\
{																																\
	(mf)->SaveUnique(name,(BYTE*)str,strlen(str)+1);											\
}

#define MapFile_LoadUniqueString(mf,name,str)												\
{																																\
	BYTE *data;																											\
	DWORD szData;																									\
	 if ((mf)->LoadUnique(name,data,szData))														\
		 (str)=(char*)(data);																						\
	 else																														\
		 (str)="";																											\
}

#define MapFile_SaveUniqueData(mf,name,vec)												\
{																																\
	(mf)->SaveUnique(name,(BYTE*)vec.data(),vec.size());										\
}

#define MapFile_LoadUniqueData(mf,name,vec)												\
{																																\
	BYTE *data;																											\
	DWORD szData;																									\
	if ((mf)->LoadUnique(name,data,szData))														\
	{																															\
		vec.resize(szData);																							\
		memcpy(vec.data(),data,szData);																		\
	}																															\
	 else																														\
		vec.clear();																										\
}

#define MapFile_SaveUniqueGObj(mf,name,obj)												\
{																																\
	std::vector<BYTE>__buf;																					\
	DP_BeginSave(__dp,__buf);																					\
		obj.GSave(__dp);																								\
	DP_EndSave();																										\
	MapFile_SaveUniqueData(mf,name,__buf);														\
}

#define MapFile_LoadUniqueGObj(mf,name,obj)												\
{																																\
	BYTE *__data;																										\
	DWORD __szData;																								\
	if ((mf)->LoadUnique(name,__data,__szData))													\
	{																															\
		CDataPacket __dp;																							\
		__dp.SetDataBufferPointer(__data);																\
		obj.GLoad(__dp);																								\
	}																															\
}


inline void MapFile_GetBlockData(IMapFile *mf,i_math::pos2di ptBlk,MapBlockData&dataBlk)
{
	dataBlk.Clear();

	BYTE *data;
	DWORD szData;
	for (int i=0;i<MapChannel_Max;i++)
	{
		if ((i==MapChannel_NavMesh)||
			(i==MapChannel_Tris)||
			(i==MapChannel_OutlineMap)||
			(i==MapChannel_MiniMap)||
			(i==MapChannel_RawMiniMap))
			continue;
		mf->Load(ptBlk,(MapChannel)i,data,szData);
		if ((data)&&(szData>0))
		{
			dataBlk.indices[i]=dataBlk.data.size();
			dataBlk.sizes[i]=szData;
			dataBlk.data.resize(szData+dataBlk.data.size());
			memcpy(&dataBlk.data[dataBlk.data.size()-szData],data,szData);
		}
	}

	dataBlk.ptBlk=ptBlk;
}

inline BOOL MapFile_SetBlockData(IMapFile *mf,MapBlockData&dataBlk)
{
	BOOL bRet=TRUE;
	for (int i=0;i<MapChannel_Max;i++)
	{
		if ((i==MapChannel_NavMesh)||
			(i==MapChannel_Tris)||
			(i==MapChannel_OutlineMap)||
			(i==MapChannel_MiniMap)||
			(i==MapChannel_RawMiniMap))
			continue;

		BYTE *data=NULL;
		if (dataBlk.indices[i]>=0)
			data=&dataBlk.data[dataBlk.indices[i]];

		if (FALSE==mf->Save(dataBlk.ptBlk,(MapChannel)i,data,dataBlk.sizes[i]))
			bRet=FALSE;
	}
	return bRet;
}