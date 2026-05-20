/********************************************************************
	created:	2007/1/26   9:39
	filename: 	e:\IxEngine\Proj_TerrainTool\mapfile.cpp
	author:		cxi
	
	purpose:	map file save/load
*********************************************************************/

#include "stdh.h"

#include "commondefines/general_stl.h"
#include "interface/interface.h"

#include "mapfile.h"

#include <assert.h>

#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"


#include "Log/LogFile.h"

#include "progress/progress.h"
#include "timer/profiler.h"

#pragma warning(disable:4312)
#pragma warning(disable:4311)


EXPOSE_INTERFACE(CMapFile,IMapFile,"MapFile01")

#define WRITE_FILE(hFile,data,szData) ::WriteFile(hFile,data,szData,&_temp,NULL)
#define READ_FILE(hFile,data,szData) ::ReadFile(hFile,data,szData,&_temp,NULL)

#define SEEK_FILE(hFile,off)											\
_temp=(LONG)(((unsigned __int64)(off))>>32);			\
::SetFilePointer(hFile,(LONG)off,(PLONG)&_temp,FILE_BEGIN)


CMapFile::CMapFile()
{
	Zero();

	AddRef();

}


void CMapFile::Clear()
{
	Close();
	Zero();
}

IMapFile *CMapFile::CreateInstance()
{
	return new CMapFile;
}

void CMapFile::Close()
{
	if (_hFile)
		CloseHandle(_hFile);
	_hFile=NULL;

	for (int i=0;i<MapChannel_Max;i++)
	{
		ChannelEntry *e=&_entries[i];

		for (int ii=0;ii<e->fields.size();ii++)
			SAFE_DELETE(e->fields[ii]);

		e->sz.set(0,0);
		e->fields.clear();
	}

	memset(&_header,0,sizeof(_header));

	_path="";
	_timeCur=0;
}


void CMapFile::_InitEntries()
{

	for (int i=0;i<MapChannel_Max;i++)
	{
		int len=_schs.GetLen((MapChannel)i);
		DWORD w,h;
		w=_header.rc.getWidth();
		h=_header.rc.getHeight();

		w/=len;
		h/=len;

		w=(w+FIELD_WIDTH-1)/FIELD_WIDTH;
		h=(h+FIELD_WIDTH-1)/FIELD_WIDTH;

		ChannelEntry *e=&_entries[i];
		e->sz.set(w,h);
		e->fields.resize(w*h);
		VEC_SET(e->fields,0);
	}
}


#define MAPFILE_VER 1
//note:after calling this function,the map file will be left closed
//path could be either full path or a relative path under WSPath_Map
BOOL CMapFile::New(const char *path,i_math::recti &rcBound)
{
	Close();

	std::string s=path;
	MakeFileSuffix(s,"mp");

	DeleteFile(path);

	HANDLE hFile=::CreateFile(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;

	_header.rc=rcBound;
	_InitEntries();

	DWORD ver=MAPFILE_VER;

	DWORD off=sizeof(DWORD)+sizeof(_header);
	DWORD nFlds=0;
	for (int i=0;i<MapChannel_Max;i++)
	{
		nFlds+=_entries[i].sz.getArea();
		_header.channeloff[i]=off;
		off+=_entries[i].sz.getArea()*FIELD_BYTE_SIZE;
	}
	_header.uniqueoff=off;

	WRITE_FILE(hFile,&ver,sizeof(ver));
	WRITE_FILE(hFile,&_header,sizeof(_header));


	std::vector<BYTE>temp;
	temp.resize(FIELD_BYTE_SIZE);
	VEC_SET(temp,0);

	for (int i=0;i<nFlds;i++)
		WRITE_FILE(hFile,temp.data(),temp.size());

	temp.resize(MAX_UNIQUE*sizeof(UniqueEntry));
	VEC_SET(temp,0);
	WRITE_FILE(hFile,temp.data(),temp.size());

	CloseHandle(hFile);

	Close();

	return TRUE;
}

BOOL CMapFile::NewByCompare(const char *path,IMapFile *mfOld,IMapFile *mfNew)
{
	return FALSE;
}


BOOL CMapFile::Open(const char *path,BOOL bReadOnly,DWORD cachearea)
{
	Close();
	_path=path;
	MakeFileSuffix(_path,"mp");
	path=_path.c_str();

	HANDLE hFile;
	if (bReadOnly)
		hFile=::CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	else
		hFile=::CreateFile(path,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD ver;
	READ_FILE(hFile,&ver,sizeof(ver));
	READ_FILE(hFile,&_header,sizeof(_header));

	_InitEntries();

	_path=path;
	_hFile=hFile;
	_bReadOnly=bReadOnly;

	//把unique全部读进来
	if (_header.nUnique>0)
	{
		SEEK_FILE(_hFile,_header.uniqueoff);
		READ_FILE(_hFile,_uniques,_header.nUnique*sizeof(UniqueEntry));
	}

	return TRUE;
}

FieldEntry *CMapFile::_ObtainField(MapChannel ch,i_math::pos2di &ptFld)
{
	DWORD idx=ptFld.y*_entries[ch].sz.w+ptFld.x;

	if (!_entries[ch].fields[idx])
	{
		FieldEntry *fld=new FieldEntry;

		DWORD off=_header.channeloff[ch];
		off+=FIELD_BYTE_SIZE*idx;
		SEEK_FILE(_hFile,off);
		READ_FILE(_hFile,fld->blks,FIELD_BYTE_SIZE);

		_entries[ch].fields[idx]=fld;
	}
	_entries[ch].fields[idx]->timestamp=_timeCur;//更新时间标签
	return _entries[ch].fields[idx];
}

void CMapFile::_SaveBlock(BlockEntry *blk,BYTE *data,DWORD szData)
{
	if (szData<=0)
	{//如果为空的数据,我们只需要简单更新一下entry的actual就行了
		blk->actual=0;
		return;
	}

	if (blk->off>0)
	{
		if (blk->sz<szData)
			blk->off=0;//超出原来的大小了,我们要开辟一块新的地方存储
		else
			blk->actual=szData;//没有超出原来的大小,我们更新actual
	}

	if (blk->off==0)
	{//在文件末尾开辟新的地方存储
		DWORD vLo,vHi;
		vLo=::GetFileSize(_hFile,&vHi);
		blk->off=(((unsigned __int64)vHi)<<32)|((unsigned __int64)vLo);
		blk->sz=szData;
		blk->actual=szData;
	}

	SEEK_FILE(_hFile,blk->off);
	WRITE_FILE(_hFile,data,szData);

}


BOOL CMapFile::_Save(i_math::pos2di &ptBlk,MapChannel ch,BYTE *data,DWORD szData)
{
	i_math::pos2di ptFld,ptOff;
	if (FALSE==_ResolveBlock(ptBlk,ch,ptFld,ptOff))
		return FALSE;

	FieldEntry *fld=_ObtainField(ch,ptFld);
	BlockEntry *blk=fld->GetBlock(ptOff);

	_SaveBlock(blk,data,szData);
	return TRUE;
}

void CMapFile::_LoadBlock(BlockEntry *blk,BYTE *&data,DWORD &szData)
{
	if (blk->actual<=0)
		return;

	assert(blk->off>0);

	_bufTemp.resize(blk->actual);
	SEEK_FILE(_hFile,blk->off);
	READ_FILE(_hFile,_bufTemp.data(),blk->actual);

	szData=blk->actual;
	data=_bufTemp.data();
}


BOOL CMapFile::Load(i_math::pos2di &ptBlk,MapChannel ch,BYTE *&data,DWORD &szData)
{
	data=NULL;
	szData=0;

	i_math::pos2di ptFld,ptOff;
	if (FALSE==_ResolveBlock(ptBlk,ch,ptFld,ptOff))
		return FALSE;

	FieldEntry *fld=_ObtainField(ch,ptFld);
	BlockEntry *blk=fld->GetBlock(ptOff);

	_LoadBlock(blk,data,szData);
	return TRUE;
}

DWORD CMapFile::EnumUniques(const char **&names)
{
	names=NULL;
	if (_header.nUnique<=0)
		return 0;

	_bufTemp2.resize(_header.nUnique);
	for (int i=0;i<_header.nUnique;i++)
		_bufTemp2[i]=_uniques[i].name;

	names=_bufTemp2.data();
	return _header.nUnique;
}


BOOL CMapFile::_SaveUnique(const char *name,BYTE *data,DWORD szData)
{
	int i;
	for (i=0;i<_header.nUnique;i++)
	{
		if (strcmp(_uniques[i].name,name)==0)
			break;
	}
	if (i>=_header.nUnique)
	{//没找到,加一个
		if (_header.nUnique>=MAX_UNIQUE)
			return FALSE;//放不下了
		memset(&_uniques[_header.nUnique],0,sizeof(UniqueEntry));
		strcpy(_uniques[_header.nUnique].name,name);
		_header.nUnique++;
	}

	_SaveBlock(&_uniques[i],data,szData);

	return TRUE;

}

BOOL CMapFile::LoadUnique(const char *name,BYTE *&data,DWORD &szData)
{
	data=NULL;
	szData=0;

	int i;
	for (i=0;i<_header.nUnique;i++)
	{
		if (strcmp(_uniques[i].name,name)==0)
			break;
	}
	if (i>=_header.nUnique)
		return FALSE;//没找到

	_LoadBlock(&_uniques[i],data,szData);

	return TRUE;
}

#define FIELD_BLOCK_LEN 64
struct BlockData
{
	std::vector<BYTE>bufs[MapChannel_Max];
};
struct FieldData
{
	BlockData blks[FIELD_BLOCK_LEN*FIELD_BLOCK_LEN];
};


//这个函数专门为MapFile2作优化
BOOL CMapFile::Update(IMapFile *mfSrc)
{
	if (!IsOpened() || IsReadOnly())
		return FALSE;

	FieldData *fd=new FieldData;

	BYTE *data;
	DWORD szData;
	i_math::recti& rc = mfSrc->GetRect();

	i_math::recti rcFld=rc;
	rcFld.scale_signed(FIELD_BLOCK_LEN);

	BlockData *blk;
	for (int i=rcFld.Left();i<rcFld.Right();i++)
	for (int j=rcFld.Top();j<rcFld.Bottom();j++)
	{
		i_math::pos2di ptStart(i*FIELD_BLOCK_LEN,j*FIELD_BLOCK_LEN);
		for (int k=0;k<MapChannel_Max;k++)
		{
			DWORD len=_schs.GetLen((MapChannel)k);
			for (int jj=0;jj<FIELD_BLOCK_LEN/len;jj++)
			for (int ii=0;ii<FIELD_BLOCK_LEN/len;ii++)
			{
				blk=&fd->blks[0]+jj*len*FIELD_BLOCK_LEN+ii*len;
				i_math::pos2di pt(ptStart.x+ii*len,ptStart.y+jj*len);
				if (mfSrc->Load(pt, MapChannel(k), data, szData))
				{
					VEC_SET_BUFFER(blk->bufs[k],data,szData);
				}
				else
					blk->bufs[k].clear();
			}
		}

		for (int k=0;k<MapChannel_Max;k++)
		{
			blk=&fd->blks[0];
			for (int jj=0;jj<FIELD_BLOCK_LEN;jj++)
			for (int ii=0;ii<FIELD_BLOCK_LEN;ii++)
			{
				i_math::pos2di pt(ptStart.x+ii,ptStart.y+jj);
				if (blk->bufs[k].size()>0)
					_Save(pt, MapChannel(k), &(blk->bufs[k][0]), blk->bufs[k].size());
				blk++;
			}
		}
	}

	//now the uniques
	if (TRUE)
	{
		const char **uniques;
		DWORD c=mfSrc->EnumUniques(uniques);

		for (int i=0;i<c;i++)
		{
			BYTE *data;
			DWORD szData;
			if (mfSrc->LoadUnique(uniques[i],data,szData))
				_SaveUnique(uniques[i],data,szData);
		}
	}

	_SaveEntries();

	return TRUE;

}



void CMapFile::_SaveEntries()
{
	for (int i=0;i<MapChannel_Max;i++)
	{
		DWORD off;
		ChannelEntry *chnl=&_entries[i];
		for (int j=0;j<chnl->fields.size();j++)
		{
			FieldEntry *fld=chnl->fields[j];
			if (fld)
			{
				off=_header.channeloff[i]+j*FIELD_BYTE_SIZE;
				SEEK_FILE(_hFile,off);
				WRITE_FILE(_hFile,fld->blks,FIELD_BYTE_SIZE);
			}
		}
	}

	SEEK_FILE(_hFile,_header.uniqueoff);
	WRITE_FILE(_hFile,_uniques,_header.nUnique*sizeof(UniqueEntry));

	SEEK_FILE(_hFile,sizeof(DWORD));
	WRITE_FILE(_hFile,&_header,sizeof(_header));
}
