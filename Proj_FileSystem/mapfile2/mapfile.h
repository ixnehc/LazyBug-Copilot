#pragma once

#include "FileSystem/IFileSystem.h"
#include "FileSystem/IMapFile.h"

#include "gds/GObj.h"

#include "supermapchannel.h"


#include <assert.h>

#define FIELD_WIDTH 64 //以block为单位,一个field有64x64个blocks

#define MAX_UNIQUE_NAME 32

#define MAX_UNIQUE 64

#define FIELD_BYTE_SIZE (FIELD_WIDTH*FIELD_WIDTH*sizeof(BlockEntry))


struct BlockEntry
{
	unsigned __int64 off:38;//在文件里的偏移
	unsigned __int64 sz:26;//最大可以为64M
	DWORD actual;//这个值表示这个block里实际有用的数据的大小
};


struct FieldEntry
{
	BlockEntry *GetBlock(i_math::pos2di &pt)
	{
		return &blks[pt.y*FIELD_WIDTH+pt.x];
	}
	BlockEntry blks[FIELD_WIDTH*FIELD_WIDTH];
	DWORD timestamp;//最近一次访问的时刻
};

struct ChannelEntry
{
	std::vector<FieldEntry *>fields;
	i_math::size2di sz;//多少个fields

};

struct UniqueEntry:public BlockEntry
{
	char name[MAX_UNIQUE_NAME];
};

struct MapFileHeader
{
	MapFileHeader()
	{
		nUnique=0;
	}
	i_math::recti rc;
	DWORD nUnique;

	DWORD channeloff[MapChannel_Max];//各个channel entries在文件中的位置
	DWORD uniqueoff;//unique entries在文件中的位置
};


class CMapFile:public IMapFile
{
public:
	IMPLEMENT_REFCOUNT;

	CMapFile();
	~CMapFile()
	{
		Clear();
	}

	void Zero()
	{
		_hFile=NULL;
		_bReadOnly=FALSE;
		_timeCur=0;
		memset(&_header,0,sizeof(_header));
	}
	void Clear();

	virtual IMapFile *CreateInstance();

	//note:when a map file is opened,internal cache will be built to accelerate 
	//data loading/saving. cachearea indicates the cache size,in block
	//path should be full path
	virtual BOOL Open(const char *path,BOOL bReadOnly,DWORD cachearea=20000);
	virtual BOOL IsOpened()	{		return _hFile!=NULL;	}
	virtual void Close();
	virtual BOOL IsReadOnly()	{		return _bReadOnly;	}


	virtual const char *GetPath()	{		return _path.c_str();	}
	virtual i_math::recti &GetRect()	{		return _header.rc;	}
	virtual i_math::recti &GetFieldRect()	{		return _header.rc;	}
	virtual DWORD GetFieldWidth()	{		return 1;	}


	//note:after calling this function,the map file will be left closed
	//path could be either full path or a relative path under WSPath_Map
	virtual BOOL New(const char *path,i_math::recti &rcBound);
	virtual BOOL NewByCompare(const char *path,IMapFile *mfOld,IMapFile *mfNew);

	//call this function after this map file is opened
	virtual BOOL Update(IMapFile *mfSrc);

	//clean up the map file if needed,
	virtual BOOL CleanUp(CProgress *prg)	{		return FALSE;	}



	virtual BOOL Load(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData);
	virtual BOOL Query(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData)	{		return FALSE;	}

	virtual BOOL Save(i_math::pos2di &ptBlk,MapChannel ch,BYTE *data,DWORD szData)	{		return FALSE;	}

	virtual void Flush()	{	}
	virtual BOOL Compress(CProgress *prg,CProgress *prgSub)	{		return FALSE;	}

	//for field version control
	virtual const char *GetInfoSscPath()	{		return "";	}
	virtual const char **PreSscOp(i_math::recti &rcFld,DWORD &nPathes)	{		nPathes=0;		return NULL;	}
	virtual void PostSscOp(i_math::recti &rcFd)	{	}
	virtual BOOL FieldCheckedOut(i_math::pos2di &ptFld)	{		return FALSE;	}
	virtual BOOL BlockCheckedOut(i_math::pos2di &ptBlk)	{		return FALSE;	}
																									//if out of the map range,return FALSE
	virtual const char *GetUniqueSscPath(const char *name)	{		return "";	}

	virtual BOOL SaveUnique(const char *name,BYTE *data,DWORD szData)	{		return FALSE;	}
	virtual BOOL LoadUnique(const char *name,BYTE *&data,DWORD &szData);
	virtual DWORD EnumUniques(const char **&names);

	virtual HMapFileCache CreateCache(MapChannel *channels,DWORD nChannel)	{		return 0;	}
	virtual void DestroyCache(HMapFileCache hCache)	{	}
	virtual BOOL UpdateCache(HMapFileCache hCache,i_math::recti &rcBlks)	{		return FALSE;	}

protected:

	void _InitEntries();

	void _SaveEntries();

	void _SaveBlock(BlockEntry *blk,BYTE *data,DWORD szData);
	void _LoadBlock(BlockEntry *blk,BYTE *&data,DWORD &szData);

	BOOL _Save(i_math::pos2di &ptBlk,MapChannel ch,BYTE *data,DWORD szData);
	BOOL _SaveUnique(const char *name,BYTE *data,DWORD szData);

	FieldEntry *_ObtainField(MapChannel ch,i_math::pos2di &ptFld);

	BOOL _ResolveBlock(i_math::pos2di &ptBlk,MapChannel ch,i_math::pos2di &ptFld,i_math::pos2di &ptOff)
	{
		if (!_header.rc.isPointInside(ptBlk))
			return FALSE;
		DWORD len=_schs.GetLen(ch);

		i_math::pos2di pt=ptBlk;
		pt-=_header.rc.UpperLeftCorner;
		pt/=len;
		ptFld=pt;
		ptFld/=FIELD_WIDTH;
		pt-=ptFld*FIELD_WIDTH;
		ptOff=pt;
		return TRUE;
	}

	HANDLE _hFile;

	MapFileHeader _header;

	ChannelEntry _entries[MapChannel_Max];

	UniqueEntry _uniques[MAX_UNIQUE];

	SuperMapChannels _schs;

	BOOL _bReadOnly;

	std::string _path;

	DWORD _temp;

	std::vector<BYTE>_bufTemp;

	std::vector<const char*>_bufTemp2;

	DWORD _timeCur;


};
