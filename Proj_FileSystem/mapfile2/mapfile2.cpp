/********************************************************************
	created:	2007/1/26   9:39
	filename: 	e:\IxEngine\Proj_TerrainTool\mapfile.cpp
	author:		cxi
	
	purpose:	map file save/load
*********************************************************************/

#include "stdh.h"

#include "commondefines/general_stl.h"
#include "FileSystem/IFileSystem.h"
#include "FileSystem/ISscSystem.h"
#include "interface/interface.h"

#include "mapfile2.h"

#include <assert.h>

#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"

#include "LzmaApi.h"
//#include "ijlApi/ijlApi.h"

#include "Log/LogFile.h"
#include "log/LogDump.h"

#include "progress/progress.h"
#include "timer/profiler.h"

#pragma warning(disable:4312)
#pragma warning(disable:4311)

#define FIELDFILE_TEMPLATE "%s\\%+04d,%+04d.d"
#define UNIQUE_SUFFIX ".u"


//////////////////////////////////////////////////////////////////////////
//RawPool
BYTE *RawPool::Alloc(DWORD sz0)
{
	assert(sz0>0);
	assert(sz0+cur<=sz);

	if (!data)
		data=new BYTE[sz];
	BYTE *p=data+cur;
	cur+=sz0;
	return p;
}


//////////////////////////////////////////////////////////////////////////
//CMapField2

#define INVALID_CLUSTER 0x7fffffff

std::vector<BYTE>CMapField2::_bufTemp;//temp buffer for save/load an entry
std::vector<BYTE>CMapField2::_bufTemp2;//another temp buffer for save/load an entry

SuperMapChannels CMapField2::_schs;


DWORD CMapField2::_CalcInfoSize()
{
	return sizeof(_szData)+sizeof(_szCluster)+sizeof(_entries)+MAX_FREE_CLUSTER*sizeof(DWORD)+sizeof(DWORD);
}


void CMapField2::_SaveInfo(IFile *fl)
{
	if (!fl)
		return;
	fl->Reset();
	fl->Write(&_szData,sizeof(_szData));
	fl->Write(&_szCluster,sizeof(_szCluster));
	fl->Write(_entries,sizeof(_entries));
	
	DWORD nFree=_free.size();
	if (nFree>MAX_FREE_CLUSTER)
		nFree=MAX_FREE_CLUSTER;
	(*fl)<<nFree;
	if (nFree>0)
		fl->Write(_free.data(),nFree*sizeof(DWORD));
}

void CMapField2::_LoadInfo(IFile *fl)
{
	if (!fl)
		return;

	fl->Reset();
	fl->Read(&_szData,sizeof(_szData));
	fl->Read(&_szCluster,sizeof(_szCluster));
	fl->Read(_entries,sizeof(_entries));
	DWORD nFree;
	(*fl)>>nFree;
	_free.resize(nFree);
	if (nFree>0)
		fl->Read(_free.data(),nFree*sizeof(DWORD));
}

//note:after calling this function,the field is left closed
BOOL CMapField2::New(IFileSystem *pFS,const char *pathRoot,i_math::pos2di &ptFld)
{
	Close();
	std::string path,pathInfo;

	FormatString(path,FIELDFILE_TEMPLATE,pathRoot,ptFld.x,ptFld.y);

	IFile *fl=pFS->OpenFileAbs(path.c_str(),FileAccessMode_Write);
	if (!fl)
		return FALSE;

	//init the info
	for (int i=0;i<MAPFIELD_CHUNKWIDTH;i++)
	for (int j=0;j<MAPFIELD_CHUNKWIDTH;j++)
	{
		memset(_chunk[i][j].data,0,sizeof(_chunk[i][j].data));
		memset(_entries[i][j].szData,0,sizeof(_entries[i][j].szData));
		_entries[i][j].idx=INVALID_CLUSTER;
		_entries[i][j].szCluster=0;
		_chunk[i][j].bModified=FALSE;
		_chunk[i][j].bLoaded=FALSE;
	}
	_szCluster=0;
	_szData=0;
	_free.clear();

	_SaveInfo(fl);

	//保存padding
	_bufTemp.resize(MAX_FREE_CLUSTER*sizeof(DWORD));
	VEC_SET(_bufTemp,0);
	fl->Write(_bufTemp.data(),_bufTemp.size());

	fl->Close();

	return TRUE;
}


BOOL CMapField2::Open(IFileSystem *pFS,BOOL bReadOnly,const char *pathRoot,i_math::pos2di &ptFld)
{
	std::string path,pathInfo;

	FormatString(path,FIELDFILE_TEMPLATE,pathRoot,ptFld.x,ptFld.y);

	_pt.x=ptFld.x*MAPFIELD_BLOCKWIDTH;
	_pt.y=ptFld.y*MAPFIELD_BLOCKWIDTH;

	FileAccessMode mode;
	if (bReadOnly)
		mode=FileAccessMode_Read;
	else
		mode=FileAccessMode_Modify;

	if (TRUE)//first read the info
	{
		_fl=pFS->OpenFileAbs(path.c_str(),mode);
		if (!_fl)
			goto _fail;

		_LoadInfo(_fl);

		//init the entries
		for (int i=0;i<MAPFIELD_CHUNKWIDTH;i++)
		for (int j=0;j<MAPFIELD_CHUNKWIDTH;j++)
		{
			memset(_chunk[i][j].data,0,sizeof(_chunk[i][j].data));
			_chunk[i][j].bModified=FALSE;
			_chunk[i][j].bLoaded=FALSE;
		}

	}

	//the mempool
	if (!bReadOnly)
		_poolW.Reset(256,15);//NOTE: the max data size for a single channel is 4M
	else
		_poolR.Reset(_szData);


	_bReadOnly=bReadOnly;
	_bModified=FALSE;

	_reftag=0;

	return TRUE;

_fail:
	if (_fl)
		_fl->Close();
	_fl=NULL;

	return FALSE;
}


DWORD CMapField2::_AllocCluster()
{
	DWORD ret;
	if (_free.size()>0)
	{
		ret=_free[_free.size()-1];
		_free.resize(_free.size()-1);
		return ret;
	}
	ret=_szCluster;
	_szCluster+=MAPCLUSTER_SIZE;
	return ret;
}

void CMapField2::_FreeCluster(DWORD idx)
{
	_free.push_back(idx);
}


void CMapField2::_DiscardClusters(DWORD idx)
{
	while(idx!=INVALID_CLUSTER)
	{
		_fl->Seek(idx+_CalcInfoSize());
		DWORD idxNext;
		(*_fl)>>idxNext;
		_fl->Seek(idx+_CalcInfoSize());
		(*_fl)<<(DWORD)INVALID_CLUSTER;
		_FreeCluster(idx);

		idx=idxNext;
	}
}


DWORD CMapField2::_SaveCluster(DWORD idx,BYTE *data,DWORD szData)
{
	DWORD idxEntry=INVALID_CLUSTER;
	if ((idx!=INVALID_CLUSTER)&&(szData>0))
		idxEntry=idx;

	DWORD idxLast=INVALID_CLUSTER;
	while(szData>0)
	{
		if (idx==INVALID_CLUSTER)
		{
			idx=_AllocCluster();
			if (idxEntry==INVALID_CLUSTER)
				idxEntry=idx;
			if (idxLast!=INVALID_CLUSTER)
			{
				_fl->Seek(idxLast+_CalcInfoSize());
				(*_fl)<<idx;//link to me
			}
			_fl->Seek(idx+_CalcInfoSize());
			idxLast=idx;
			idx=INVALID_CLUSTER;
			(*_fl)<<(DWORD)INVALID_CLUSTER;
		}
		else
		{
			_fl->Seek(idx+_CalcInfoSize());
			idxLast=idx;
			(*_fl)>>idx;
		}

		DWORD c;
		if (szData>MAPCLUSTER_ACTUALSIZE)
		{
			szData-=MAPCLUSTER_ACTUALSIZE;
			c=MAPCLUSTER_ACTUALSIZE;
		}
		else
		{
			c=szData;
			szData=0;
		}
		_fl->Write(data,c);
		data+=c;

		//writing the padding data if needed
		if ((idxLast+MAPCLUSTER_SIZE>=_szCluster)&&(c<MAPCLUSTER_ACTUALSIZE))
		{//the current cluster is at the end of the file,we need to write the padding data
			BYTE padding[MAPCLUSTER_ACTUALSIZE];
			_fl->Write(padding,MAPCLUSTER_ACTUALSIZE-c);
		}
	}

	if (idx!=INVALID_CLUSTER)
	{//terminate it
		if (idxLast!=INVALID_CLUSTER)
		{
			_fl->Seek(idxLast+_CalcInfoSize());
			(*_fl)<<(DWORD)INVALID_CLUSTER;
		}
	}

	_DiscardClusters(idx);

	return idxEntry;
}

void CMapField2::_ClearChunk(_ChunkEntry *entry,_Chunk *entry2)
{
	for (int i=0;i<SuperMapChannels::SuperMapChannel_Max;i++)
	for (int j=0;j<BLOCK_PER_CHUNK;j++)
	{
		entry->szData[j][i]=0;
		if ((entry2->data[j][i])&&(!_bReadOnly))
			_poolW.Free((MemHandle)entry2->data[j][i]);
	}
	entry->idx=INVALID_CLUSTER;
	entry->szCluster=0;

	entry2->bLoaded=TRUE;
	entry2->bModified=TRUE;
}


BOOL CMapField2::_SaveChunk(_ChunkEntry *entry,_Chunk *entry2)
{
	if (!entry2->bModified)
		return TRUE;

	_bufTemp.clear();

	CDataPacket dp;

	for (int i=0;i<SuperMapChannels::SuperMapChannel_Max;i++)
	for (int j=0;j<BLOCK_PER_CHUNK;j++)
		dp.Data_MarchData(entry->szData[j][i]);

	_bufTemp.resize(dp.GetDataSize());
	dp.SetDataBufferPointer(_bufTemp.data());

	for (int i=0;i<SuperMapChannels::SuperMapChannel_Max;i++)
	for (int j=0;j<BLOCK_PER_CHUNK;j++)
	{
		BYTE *ptr=(BYTE*)_poolW.ObtainPtr((MemHandle)entry2->data[j][i]);
		assert(ptr);
		dp.Data_WriteData(ptr,entry->szData[j][i]);
	}


	entry->idx=_SaveCluster(entry->idx,_bufTemp.data(),_bufTemp.size());
	entry->szCluster=_bufTemp.size();

	entry2->bModified=FALSE;
	return TRUE;
}

BOOL CMapField2::_LoadChunk(_ChunkEntry *entry,_Chunk *entry2)
{
	if (entry2->bLoaded)
		return TRUE;
	DWORD szData; 
	BYTE *data;
 
	szData=entry->szCluster;
	_LoadCluster(entry->idx,data,szData);

	for (int i=0;i<SuperMapChannels::SuperMapChannel_Max;i++)
	for (int j=0;j<BLOCK_PER_CHUNK;j++)
	{
		DWORD sz=entry->szData[j][i];
		if (sz>0)
		{
			BYTE *p;
			if (_bReadOnly)
			{
				p=_poolR.Alloc(sz);
				entry2->data[j][i]=p;
			}
			else
			{
				MemHandle h=_poolW.Alloc(sz);
				assert(h!=MemHandle_Null);
				entry2->data[j][i]=(BYTE*)h;
				p=(BYTE*)_poolW.ObtainPtr(h);
			}

			memcpy(p,data,sz);
			data+=sz;
		}
	}

	entry2->bLoaded=TRUE;
	return TRUE;
}

//szData indicates the data size to read in this cluster
void CMapField2::_LoadCluster(DWORD idx,BYTE *&data,DWORD &szData)
{

	DWORD szDataToRead=szData;
	_bufTemp.clear();
	while(idx!=INVALID_CLUSTER)
	{
		_fl->Seek(idx+_CalcInfoSize());
		(*_fl)>>idx;//the next cluster
		DWORD sz=_bufTemp.size();
		DWORD c;
		if (szDataToRead>MAPCLUSTER_ACTUALSIZE)
		{
			c=MAPCLUSTER_ACTUALSIZE;
			szDataToRead-=MAPCLUSTER_ACTUALSIZE;
		}
		else
		{
			c=szDataToRead;
			szDataToRead=0;
		}
		_bufTemp.resize(sz+c);
		_fl->Read(&_bufTemp[sz],c);
	}
	assert(szData==_bufTemp.size());
	if (szData<=0)
		data=NULL;
	else
		data=_bufTemp.data();

}



void CMapField2::Flush()
{
	if (!IsOpened())
		return;
	if (_bReadOnly)
		return;

	for (int i=0;i<MAPFIELD_CHUNKWIDTH;i++)
	for (int j=0;j<MAPFIELD_CHUNKWIDTH;j++)
	{
		if (FALSE==_SaveChunk(&_entries[i][j],&_chunk[i][j]))
		{
			LogFile::Prompt("MapFile writing error:fail to save chunk of blocks[(%d,%d)~(%d,%d)]!",
				_pt.x+i*CHUNK_BLOCKWIDTH,_pt.y+j*CHUNK_BLOCKWIDTH,
				_pt.x+(i+1)*CHUNK_BLOCKWIDTH,_pt.y+(j+1)*CHUNK_BLOCKWIDTH);

			_ClearChunk(&_entries[i][j],&_chunk[i][j]);
		}
	}

	if (_bModified)
		_SaveInfo(_fl);

	_bModified=FALSE;
}


void CMapField2::Close()
{
	if (!IsOpened())
		return;
	Flush();
	if (_fl)
		_fl->Close();
	_fl=NULL;

	_free.clear();
	_poolR.Reset(0);
	_poolW.Clear();
}


BOOL CMapField2::Load(BOOL bModify,i_math::pos2di &ptBlk0,MapChannel ch0,
																			BYTE *&data,DWORD &szData)
{
	if (!IsOpened())
		return FALSE;
	if (bModify)
	{
		if (_bReadOnly)			
			return FALSE;
	}

	i_math::pos2di ptBlk=ptBlk0;
	_schs.Resolve(ch0,ptBlk);
	int sch=_schs.GetSuper(ch0);


	data=NULL;
	szData=0;

	i_math::pos2di ptEntry=(ptBlk-_pt)/CHUNK_BLOCKWIDTH;
	DWORD iBlock=((ptBlk.y-_pt.y)%CHUNK_BLOCKWIDTH)*CHUNK_BLOCKWIDTH+((ptBlk.x-_pt.x)%CHUNK_BLOCKWIDTH);
	_ChunkEntry *p=&_entries[ptEntry.x][ptEntry.y];
	_Chunk *p2=&_chunk[ptEntry.x][ptEntry.y];

	if (FALSE==_LoadChunk(p,p2))//ensure loaded
		return FALSE;

	if (p->szData[iBlock][sch]<=0)
		return TRUE;

	szData=p->szData[iBlock][sch];
	if (_bReadOnly)
		data=p2->data[iBlock][sch];
	else
		data=(BYTE*)_poolW.ObtainPtr((MemHandle)p2->data[iBlock][sch]);

	if (bModify)
	{
		p2->bModified=TRUE;
		_bModified=TRUE;
	}

	return TRUE;
}

//ptBlk is world coord
BOOL CMapField2::Save(i_math::pos2di &ptBlk0,MapChannel channel0,BYTE *data,DWORD szData)
{
	if (!IsOpened())
		return FALSE;

	if (_bReadOnly)
		return FALSE;

	i_math::pos2di ptBlk=ptBlk0;
	_schs.Resolve(channel0,ptBlk);
	int sch=_schs.GetSuper(channel0);

	i_math::pos2di ptEntry=(ptBlk-_pt)/CHUNK_BLOCKWIDTH;
	DWORD iBlock=((ptBlk.y-_pt.y)%CHUNK_BLOCKWIDTH)*CHUNK_BLOCKWIDTH+((ptBlk.x-_pt.x)%CHUNK_BLOCKWIDTH);
	_ChunkEntry *p=&_entries[ptEntry.x][ptEntry.y];
	_Chunk *p2=&_chunk[ptEntry.x][ptEntry.y];
	_LoadChunk(p,p2);//ensure loaded

	MemHandle h=MemHandle_Null;
	if (szData>0)
	{
		if (p->szData[iBlock][sch]<=0)
			h=_poolW.Alloc(szData);
		else
		{
			h=(MemHandle)p2->data[iBlock][sch];
			if (FALSE==_poolW.ReAlloc(h,szData))
				h=MemHandle_Null;
		}
		if (h==MemHandle_Null)
		{
			LOG_DUMP_2P("MapFile",Log_Error,"保存地图文件时,出现了数据过大的错误(block坐标:%d,%d),目前,单个地图block的数据容量是4M!",
				ptBlk0.x,ptBlk0.y);
			return FALSE;
		}
	}

	//copy the channel data
	if (szData>0)
	{
		BYTE *p=(BYTE *)_poolW.ObtainPtr(h);
		memcpy(p,data,szData);
	}

	//update info
	p2->data[iBlock][sch]=(BYTE*)h;
	_szData-=p->szData[iBlock][sch];
	p->szData[iBlock][sch]=szData;
	_szData+=p->szData[iBlock][sch];

	p2->bModified=TRUE;
	_bModified=TRUE;

	return TRUE;
}

BOOL CMapField2::Compress(CProgress *prg)
{
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//CMapFile2
EXPOSE_INTERFACE(CMapFile2,IMapFile,"MapFile2_01")


extern "C" void* CreateInterface( const char *pName);
CMapFile2::CMapFile2()
{
	Zero();
	_pFS=(IFileSystem*)CreateInterface("FileSystem01");

	AddRef();

}

IMapFile *CMapFile2::CreateInstance()
{
	return new CMapFile2;
}



void CMapFile2::Clear()
{
	Close();
	Zero();
}

BOOL CMapFile2::_SaveInfo()
{
	std::string path;
	path=_path+"\\mapinfo.dat";

	IFile *fl=_pFS->OpenFileAbs(path.c_str(),FileAccessMode_Write);
	if (!fl)
		return FALSE;

	CDataPacket dp;
	_info.GSave(dp);
	std::vector<BYTE>buf;
	buf.resize(dp.GetDataSize());
	dp.SetDataBufferPointer(buf.data());
	_info.GSave(dp);

	IFile_WriteVector(fl,buf);

	fl->Close();

	return TRUE;
}

BOOL CMapFile2::_LoadInfo()
{
	std::string path;
	path=_path+"\\mapinfo.dat";

	IFile *fl=_pFS->OpenFileAbs(path.c_str(),FileAccessMode_Read);
	if (!fl)
		return FALSE;

	std::vector<BYTE>buf;
	IFile_ReadVector(fl,buf);
	fl->Close();

	CDataPacket dp;
	dp.SetDataBufferPointer(buf.data());
	_info.GClear();
	_info.GLoad(dp);


	return TRUE;
}

void CMapFile2::_RefreshFieldAttr(i_math::recti &rcFld)
{
	FileAttr*p=_fieldattr.data() +(rcFld.Top()-_info.rcField.Top())*_info.rcField.getWidth()+
												(rcFld.Left()-_info.rcField.Left());
	std::string path;
	for (int j=rcFld.Top();j<rcFld.Bottom();j++)
	{
		for (int i=rcFld.Left();i<rcFld.Right();i++)
		{
			//check whether the field files are read-only
			FormatString(path,FIELDFILE_TEMPLATE,_path.c_str(),i,j);

			FileAttr attr;
			attr=_pFS->GetFileAttr(path.c_str());
			p[i-rcFld.Left()]=File_Default;
			if (attr&File_Miss)
				p[i-rcFld.Left()]=File_Miss;
			else
			{
				if (attr&File_ReadOnly)
					p[i-rcFld.Left()]=File_ReadOnly;
			}
		}
		p+=_info.rcField.getWidth();
	}

}


void CMapFile2::_SetPath(const char *path)
{
	if (path[0]==0)
		_path=path;
	else
	{
		if (IsFullPath(path))
			_path=path;
		else
			assert(FALSE);
	}
}


BOOL CMapFile2::New(const char *path,i_math::recti &rcBound)
{
	Close();

	_SetPath(path);

	_pFS->RemoveFolder(_path.c_str());

	CMapField2 field;

	//the info
	_info.rc=rcBound;
	_info.rcField.Left()=(int)floor((float)rcBound.Left()/(float)MAPFIELD_BLOCKWIDTH);
	_info.rcField.Right()=(int)floor((float)(rcBound.Right()-1)/(float)MAPFIELD_BLOCKWIDTH)+1;
	_info.rcField.Top()=(int)floor((float)rcBound.Top()/(float)MAPFIELD_BLOCKWIDTH);
	_info.rcField.Bottom()=(int)floor((float)(rcBound.Bottom()-1)/(float)MAPFIELD_BLOCKWIDTH)+1;
	if (FALSE==_SaveInfo())
		goto _fail;

	//the data
	if (TRUE)
	{
		for (int i=_info.rcField.Left();i<_info.rcField.Right();i++)
		for (int j=_info.rcField.Top();j<_info.rcField.Bottom();j++)
		{
			if (FALSE==field.New(_pFS,_path.c_str(),i_math::pos2di(i,j)))
				goto _fail;
		}
	}

	return TRUE;

_fail:

	_pFS->RemoveFolder(_path.c_str());
	return FALSE;
}

BOOL CMapFile2::Open(const char *path,BOOL bReadOnly,DWORD cachearea)
{
	_SetPath(path);
	_bReadOnly=bReadOnly;

	if (FALSE==_LoadInfo())
	{
		_SetPath("");
		return FALSE;
	}

	_fields.resize(_info.rcField.getArea());
	VEC_SET(_fields,0);


	//init field attr
	_fieldattr.resize(_info.rcField.getArea());
	_RefreshFieldAttr(_info.rcField);


	_reftag=0;

	//calculate the cache fields needed
	if (TRUE)
	{
		DWORD c=cachearea/(MAPFIELD_BLOCKWIDTH*MAPFIELD_BLOCKWIDTH);
		if (c<1)
			c=1;//至少要有1
		_pool.Reset(c);
	}

	return TRUE;
}

void CMapFile2::Close()
{
	_fields.clear();
	_pool.Reset(1);//the fields will be closed in their CMapField2::~CMapField2()

	_SetPath("");
}


i_math::pos2di CMapFile2::_FieldPosFromBlock(i_math::pos2di &ptBlk)
{
	i_math::pos2di ptFld;
	ptFld.x=(int)floor((float)ptBlk.x/(float)MAPFIELD_BLOCKWIDTH);
	ptFld.y=(int)floor((float)ptBlk.y/(float)MAPFIELD_BLOCKWIDTH);
	return ptFld;
}


CMapField2 *&CMapFile2::_FieldFromBlock(i_math::pos2di &ptBlk)
{
	i_math::pos2di ptFld=_FieldPosFromBlock(ptBlk);

	assert(_info.rcField.isPointInside(ptFld));

	return _fields[(ptFld.y-_info.rcField.Top())*_info.rcField.getWidth()+(ptFld.x-_info.rcField.Left())];
}


CMapField2 *CMapFile2::_ObtainField(i_math::pos2di &ptBlk)
{
	CMapField2 *&field=_FieldFromBlock(ptBlk);
	if (field)
		return field;

	field=_pool.Alloc();
	if (!field)
	{
		//pool is full,try to find a least-refered field to discard
		DWORD c=_pool.GetCount();
		CMapField2 *pMin=NULL;
		for (int i=0;i<c;i++)
		{
			CMapField2 *p=_pool.Get(i);
			assert(p);
			if (!pMin)
				pMin=p;
			else
			{
				if (p->GetRefTag()<pMin->GetRefTag())
					pMin=p;
			}
		}

		//discard
		CMapField2 *&field2=_FieldFromBlock(pMin->GetPos());
		_pool.Free(field2);
		field2=NULL;


		field=_pool.Alloc();
		assert(field);
	}


	i_math::pos2di ptFld=_FieldPosFromBlock(ptBlk);
	if (!FieldCheckedOut(ptFld))
		field->Open(_pFS,TRUE,_path.c_str(),ptFld);//use read-only flag
	else
	{
		if (FALSE==field->Open(_pFS,_bReadOnly,_path.c_str(),ptFld))
			field->Open(_pFS,TRUE,_path.c_str(),ptFld);//use read-only flag to give another try
	}


	return field;
}

BOOL CMapFile2::Load(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData)
{		
//	ProfilerStart_Recent(MapFile2_Load);
	BOOL bRet;
	bRet=_Load(FALSE,ptBlk,ch,data,szData);	
//	ProfilerEnd();
	return bRet;
}

BOOL CMapFile2::Query(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData)
{		
	return _Load(TRUE,ptBlk,ch,data,szData);	
}



BOOL CMapFile2::_Load(BOOL bModify,i_math::pos2di &ptBlock,MapChannel ch,BYTE *&data,DWORD &szData)
{
	data=NULL;
	szData=0;

	if (!_info.rc.isPointInside(ptBlock))
		return FALSE;

	CMapField2 *field=_ObtainField(ptBlock);
	if (!field)
		return FALSE;

	_reftag++;
	field->SetRefTag(_reftag);//update ref tag for this field

	return field->Load(bModify,ptBlock,ch,data,szData);
}


BOOL CMapFile2::Save(i_math::pos2di &ptBlock,MapChannel ch,BYTE *data,DWORD szData)
{
	if (!_info.rc.isPointInside(ptBlock))
		return FALSE;

	CMapField2 *field=_ObtainField(ptBlock);
	if (!field)
		return FALSE;

	_reftag++;
	field->SetRefTag(_reftag);//update ref tag for this field

	return field->Save(ptBlock,ch,data,szData);
}

void CMapFile2::Flush()
{
	DWORD c=_pool.GetCount();
	for (int i=0;i<c;i++)
	{
		CMapField2 *p=_pool.Get(i);
		if (p)
			p->Flush();
	}
}

BOOL CMapFile2::Compress(CProgress *prg,CProgress *prgSub)
{
	if (_bReadOnly)
		return FALSE;

	if (prg)
	{
		prg->SetBegin();
		prg->SetTitle("Compressing MapFile");
	}

	int total=_info.rcField.getArea();
	int cur=0;
	for (int i=_info.rcField.Left();i<_info.rcField.Right();i++)
	for (int j=_info.rcField.Top();j<_info.rcField.Bottom();j++)
	{
		cur++;
		if (prg)
			prg->SetProgress("",cur,total);

		i_math::pos2di ptBlk(i*MAPFIELD_BLOCKWIDTH,j*MAPFIELD_BLOCKWIDTH);
		CMapField2 *field=_ObtainField(ptBlk);
		if (!field)
			continue;
		field->Compress(prgSub);
	}

	if (prg)
		prg->SetEnd();

	return TRUE;
}

const char **CMapFile2::PreSscOp(i_math::recti &rcFld,DWORD &nPathes)
{
	_temp2.clear();
	_temp3.clear();
	std::string path;
	for (int j=rcFld.Top();j<rcFld.Bottom();j++)
	for (int i=rcFld.Left();i<rcFld.Right();i++)
	{
		FormatString(path,FIELDFILE_TEMPLATE,_path.c_str(),i,j);
		//确保这个field 没有被打开
		if (TRUE)
		{
			CMapField2 *&field=_fields[(j-_info.rcField.Top())*_info.rcField.getWidth()+(i-_info.rcField.Left())];
			if (field)
			{
				_pool.Free(field);
				field=NULL;
			}
		}

		_temp2.push_back(path);
	}

	_temp3.resize(_temp2.size());
	for (int i=0;i<_temp2.size();i++)
		_temp3[i]=_temp2[i].c_str();
	
	nPathes=_temp3.size();
	return _temp3.data();
}

void CMapFile2::PostSscOp(i_math::recti &rcFld)
{
	_RefreshFieldAttr(rcFld);
}



//test whether the field is checked out
BOOL CMapFile2::FieldCheckedOut(i_math::pos2di &ptFld)
{
	FileAttr attr=_fieldattr[(ptFld.y-_info.rcField.Top())*_info.rcField.getWidth()+ptFld.x-_info.rcField.Left()];
	if (attr&(File_Miss|File_ReadOnly))
		return FALSE;
	return TRUE;//not miss and not readonly
}

//test whether the block is checked out
BOOL CMapFile2::BlockCheckedOut(i_math::pos2di &ptBlk)
{
	if (!_info.rc.isPointInside(ptBlk))
		return FALSE;
	return FieldCheckedOut(_FieldPosFromBlock(ptBlk));
}


const char *CMapFile2::GetInfoSscPath()
{
	_temp2.resize(1);
	_temp2[0]=_path+"\\mapinfo.dat";;
	return _temp2[0].c_str();
}


const char *CMapFile2::GetUniqueSscPath(const char *name)
{
	_temp2.resize(1);
	_temp2[0]=_path+"\\"+name+UNIQUE_SUFFIX;
	return _temp2[0].c_str();
}


BOOL CMapFile2::SaveUnique(const char *name,BYTE *data,DWORD szData)
{
	std::string path;
	path=_path+"\\"+name+UNIQUE_SUFFIX;
	IFile *fl=_pFS->OpenFileAbs(path.c_str(),FileAccessMode_Write);
	if (!fl)
		return FALSE;

	fl->Write(data,szData);
	fl->Close();

	return TRUE;
}

BOOL CMapFile2::LoadUnique(const char *name,BYTE *&data,DWORD &szData)
{
	std::string path;
	path=_path+"\\"+name+UNIQUE_SUFFIX;
	IFile *fl=_pFS->OpenFileAbs(path.c_str(),FileAccessMode_Read);
	if (!fl)
		return FALSE;

	szData=fl->GetSize();
	_temp.resize(szData);
	if (szData==_temp.size())
		fl->Read(_temp.data(),szData);
	fl->Close();

	data=_temp.data();

	return TRUE;
}

DWORD CMapFile2::EnumUniques(const char **&names)
{
	IFileSystem_EnumFiles(_pFS,_path.c_str(),_temp2);

	std::string s;
	_temp3.clear();
	//过滤掉mapfile的地图数据文件
	for (int i=0;i<_temp2.size();i++)
	{
		if (!CheckFileSuffix(_temp2[i],"u"))
			continue;
		RemoveFileSuffix(_temp2[i]);

		_temp3.push_back(_temp2[i].c_str());
	}

	names=_temp3.data();
	return _temp3.size();
}
