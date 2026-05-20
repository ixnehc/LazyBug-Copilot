#pragma once

#include "FileSystem/IFileSystem.h"
#include "FileSystem/IMapFile.h"

#include "mempool/mempool.h"

#include "gds/GObj.h"

#include "supermapchannel.h"


#include <assert.h>

//NOTE:
//Each map field contains MAPFIELD_CHUNKWIDTH*MAPFIELD_CHUNKWIDTH entries
//Each entry contains CHUNK_BLOCKWIDTH*CHUNK_BLOCKWIDTH blocks
#define MAPFIELD2_LENGTH 256
#define MAPBLOCK_LENGTH 4
#define MAPCHUNK_LENGTH 16
#define MAPFIELD_CHUNKWIDTH (MAPFIELD2_LENGTH/MAPCHUNK_LENGTH)
#define MAPFIELD_BLOCKWIDTH (MAPFIELD2_LENGTH/MAPBLOCK_LENGTH)
#define CHUNK_BLOCKWIDTH (MAPFIELD_BLOCKWIDTH/MAPFIELD_CHUNKWIDTH)
#define BLOCK_PER_CHUNK (CHUNK_BLOCKWIDTH*CHUNK_BLOCKWIDTH)


#define MAPCLUSTER_SIZE 4096
#define MAPCLUSTER_ACTUALSIZE (MAPCLUSTER_SIZE-sizeof(DWORD))

#define MAX_FREE_CLUSTER (1024*16)

struct RawPool
{
	RawPool()
	{
		memset(this,0,sizeof(*this));
	}
	void Reset(DWORD sz0)
	{
		SAFE_DELETE(data);
		sz=sz0;
		cur=0;
	}
	DWORD GetSize()	{		return sz;	}
	BYTE *Alloc(DWORD sz0);
	BYTE *data;
	DWORD sz;
	DWORD cur;
};


class CMapField2
{
public:
	CMapField2()
	{
		_fl=NULL;
		_bModified=FALSE;
		_bReadOnly=FALSE;
		_reftag=0;
	}
	~CMapField2()
	{
		Close();
	}
	BOOL New(IFileSystem *pFS,const char *pathRoot,i_math::pos2di &ptFld);
	BOOL Open(IFileSystem *pFS,BOOL bReadOnly,const char *pathRoot,i_math::pos2di &ptFld);
	void Close();

	void Flush();//save all the modified to disk

	BOOL IsOpened()	{		return _fl!=NULL;	}

	//ptBlk is relative to this field's left-up block
	//IMPORTANT:the returned ptr(data) is temp ptr,and should NOT be kept for later use
	//bModify whether this func should mark the block's channel as modified after loading
	BOOL Load(BOOL bModify,i_math::pos2di &ptBlk,MapChannel channel,BYTE *&data,DWORD &szData);

	//ptBlk is relative to this field's left-up block
	//NOTE: this func will NOT save the data to harddisk immediately,it just modify the 
	//data in memory. Call Flush() to save all these modified data to disk
	BOOL Save(i_math::pos2di &ptBlk,MapChannel channel,BYTE *data,DWORD szData);

	BOOL Compress(CProgress *prg);

	void SetRefTag(DWORD tag)	{		_reftag=tag;	}
	DWORD GetRefTag()	{		return _reftag;	}

	i_math::pos2di& GetPos()	{		return _pt;	}


protected:
	struct _ChunkEntry
	{
		DWORD szCluster;//the actual data size saved to clusters,in byte
		DWORD idx:31;//index to the first cluster in file,in byte
		DWORD szData[BLOCK_PER_CHUNK][SuperMapChannels::SuperMapChannel_Max];//data size for each channel
	};
	struct _Chunk
	{
		BYTE *data[BLOCK_PER_CHUNK][SuperMapChannels::SuperMapChannel_Max];
		short bModified;
		short bLoaded;
	};

	BOOL _SaveChunk(_ChunkEntry *entry,_Chunk *entry2);
	DWORD _SaveCluster(DWORD idx,BYTE *data,DWORD szData);

	BOOL _LoadChunk(_ChunkEntry *entry,_Chunk *entry2);
	void _LoadCluster(DWORD idx,BYTE *&data,DWORD &szData);

	void _ClearChunk(_ChunkEntry *entry,_Chunk *entry2);

	void _SaveInfo(IFile *fl);
	void _LoadInfo(IFile *fl);

	DWORD _AllocCluster();
	void _FreeCluster(DWORD idx);

	void _DiscardClusters(DWORD idx);

	DWORD _CalcInfoSize();

	//Info
	_ChunkEntry _entries[MAPFIELD_CHUNKWIDTH][MAPFIELD_CHUNKWIDTH];
	_Chunk _chunk[MAPFIELD_CHUNKWIDTH][MAPFIELD_CHUNKWIDTH];
	std::vector<DWORD>_free;//the indices(in byte) to free clusters available in this field
	DWORD _szData;//the total size of all the channels data in this field
	DWORD _szCluster;//all the cluster data size(in byte)

	//working data
	IFile *_fl;
	RawPool _poolR;//all the read-only data
	CMemPoolEx _poolW;//all the modified data

	BOOL _bReadOnly;
	BOOL _bModified;

	i_math::pos2di _pt;//the left-up block's world coord
	DWORD _reftag;


	static SuperMapChannels _schs;
	static std::vector<BYTE>_bufTemp;//temp buffer for save/load an entry
	static std::vector<BYTE>_bufTemp2;//another temp buffer for save/load an entry

};


class CMapFile2:public IMapFile
{
public:
	IMPLEMENT_REFCOUNT;

	struct _Info
	{
		i_math::recti rc;//in block
		i_math::recti rcField;//in field

		// GObj Defination --------------------------------------------------
		BEGIN_GOBJ_PURE(_Info,1);
			GELEM_VAR_INIT(i_math::recti,rc,i_math::recti(0,0,0,0));
			GELEM_VAR_INIT(i_math::recti,rcField,i_math::recti(0,0,0,0));
		END_GOBJ();
	};

	CMapFile2();
	~CMapFile2()
	{
		Clear();
	}
	void Zero()
	{
		_pFS=NULL;
		_path="";
		_reftag=0;
	}

	void Clear();


	//interfaces
	virtual IMapFile *CreateInstance();

	virtual BOOL New(const char *path,i_math::recti &rcBound);//rcBound is in block
	virtual BOOL Open(const char *path,BOOL bReadOnly,DWORD cachearea=20000);
	virtual BOOL IsOpened()	{		return _fields.size()>0;	}
	virtual void Close();
	virtual BOOL IsReadOnly()	{		return _bReadOnly;	}

	virtual const char *GetPath()	{		return _path.c_str();	}
	virtual i_math::recti &GetRect()	{		return _info.rc;	}//in block
	virtual i_math::recti &GetFieldRect()	{		return _info.rcField;	}
	virtual DWORD GetFieldWidth()	{		return MAPFIELD_BLOCKWIDTH;	}//得到field的边长,以block为单位,这个值的平方代表一个field有多少个block

	//ptBlk is world coord
	//IMPORTANT:the returned ptr(data) is temp ptr,and should NOT be kept for later use
	//the different between Load()&Query(): the latter will mark this block's channel as modified
	virtual BOOL Load(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData);
	virtual BOOL Query(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData);

	virtual BOOL Save(i_math::pos2di &ptBlock,MapChannel ch,BYTE *data,DWORD szData);

	virtual void Flush();//save all the modified to disk
	virtual BOOL Compress(CProgress *prg,CProgress *prgSub);


	virtual const char *GetInfoSscPath();//得到info文件对应的文件名
	virtual const char **PreSscOp(i_math::recti &rcFld,DWORD &nPathes);
	virtual void PostSscOp(i_math::recti &rcFd);
	virtual BOOL FieldCheckedOut(i_math::pos2di &ptFld);//test whether the field is checked out
	virtual BOOL BlockCheckedOut(i_math::pos2di &ptBlk);//test whether the block is checked out
	virtual const char *GetUniqueSscPath(const char *name);//得到某个unique数据对应的文件名

	virtual BOOL SaveUnique(const char *name,BYTE *data,DWORD szData);
	virtual BOOL LoadUnique(const char *name,BYTE *&data,DWORD &szData);
	virtual DWORD EnumUniques(const char **&names);

	//not supported interfaces
	virtual BOOL NewByCompare(const char *path,IMapFile *mfOld,IMapFile *mfNew)	{		return FALSE;	}
	virtual BOOL Update(IMapFile *mfSrc)	{		return FALSE;	}
	virtual BOOL CleanUp(CProgress *prg)	{		return FALSE;	}
	virtual HMapFileCache CreateCache(MapChannel *channels,DWORD nChannel)	{		return HMapFileCache_Null;	}
	virtual void DestroyCache(HMapFileCache hCache)	{	}
	virtual BOOL UpdateCache(HMapFileCache hCache,i_math::recti &rcBlks)	{		return FALSE;	}


protected:

	BOOL _Load(BOOL bModify,i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData);

	BOOL _SaveInfo();
	BOOL _LoadInfo();

	CMapField2 *&_FieldFromBlock(i_math::pos2di &ptBlk);
	i_math::pos2di _FieldPosFromBlock(i_math::pos2di &ptBlk);
	CMapField2 *_ObtainField(i_math::pos2di &ptBlk);

	void _RefreshFieldAttr(i_math::recti &rcFld);

	void _SetPath(const char *path);

	IFileSystem *_pFS;

	std::string _path;
	BOOL _bReadOnly;

	_Info _info;

	std::vector<CMapField2*> _fields;//field map,
	std::vector<FileAttr>_fieldattr;

	CMemPool_fv<CMapField2> _pool;

	DWORD _reftag;

	std::vector<BYTE> _temp;
	std::vector<std::string> _temp2;//unique names
	std::vector<const char *>_temp3;//unique names


};
