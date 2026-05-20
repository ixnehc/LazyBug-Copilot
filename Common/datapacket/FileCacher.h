#pragma once

#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include "../fastdelegate/FastDelegate.h"


//TKey should override operator== 
template <class TKey,DWORD TMaxEntry>
class CFileCacher
{
public:
	typedef fastdelegate::FastDelegate2<TKey &,std::vector<BYTE>&,BOOL> BuilderType;
	//return a temply pointer
	void *GetCache(const char *path,TKey &key,BuilderType &dlgtBuilder)
	{
		BOOL bLoad=FALSE;
		if (_Cache(path,key,dlgtBuilder))
		{
			FILE *file=fopen(path,"rb");
			if (file)
			{
				Header header;
				fread(&header,sizeof(header),1,file);

				for (int i=0;i<TMaxEntry;i++)
				{
					if (header.sz[i]!=0)
					{//an available entry
						if (header.keys[i]==key)
						{
							_buf.resize(header.sz[i]);
							fseek(file,header.offs[i],SEEK_SET);
							fread(_buf.data(),header.sz[i],1,file);
							bLoad=TRUE;
							break;
						}
					}
				}
				fclose(file);
			}
		}

		if (bLoad)
			return _buf.data();

		//directly build one
		if (!dlgtBuilder(key,_buf))
			return NULL;
		return _buf.data();
	}
protected:

	struct Header
	{
		Header()
		{
			memset(sz,0,sizeof(sz));
		}
		TKey keys[TMaxEntry];
		int offs[TMaxEntry];
		DWORD sz[TMaxEntry];//if 0,the entry is empty
	};

	BOOL _Cache(const char *path,TKey &key,BuilderType &dlgtBuilder)
	{
		BOOL bRet=FALSE;
		BOOL bNeedRecreate=FALSE;
		Header header;
		if (TRUE)//first read all the cache data
		{
			FILE *file=fopen(path,"rb");
			if (file)
			{
				if (fread(&header,sizeof(Header),1,file)==1)
				{
					int i;
					for (i=0;i<TMaxEntry;i++)
					{
						if (header.sz[i]!=0)
						{//an available entry
							if (header.keys[i]==key)
								break;
						}
					}
					if (i<TMaxEntry)
					{//Already in cache
						fclose(file);
						return TRUE;
					}
				}
				fclose(file);
			}
			else
				bNeedRecreate=TRUE;//文件不存在
		}

		DWORD iEntry=0;
		if (!bNeedRecreate)//Find an empty entry
		{
			for (iEntry=0;iEntry<TMaxEntry;iEntry++)
			{
				if (header.sz[iEntry]==0)
					break;
			}
			if (iEntry>=TMaxEntry)
				bNeedRecreate=TRUE;//没有空间了,我们清空所有的cache,重新开始
		}
		if (bNeedRecreate)
		{
			memset(header.sz,0,sizeof(header.sz));
			iEntry=0;
			FILE *file=fopen(path,"wb");
			if (!file)
				goto _final;
			fwrite(&header,sizeof(header),1,file);
			
			fclose(file);
		}

		if (TRUE)//Build a cache and fill it into the entry
		{
			if (FALSE==dlgtBuilder(key,_buf))
				goto _final;//Fail
			FILE *file=fopen(path,"rb+");
			if (!file)
				goto _final;

			fseek(file,0,SEEK_END);
			header.offs[iEntry]=ftell(file);
			header.keys[iEntry]=key;
			header.sz[iEntry]=_buf.size();

			fwrite((const void*)_buf.data(),_buf.size(),1,file);

			fseek(file,0,SEEK_SET);
			fwrite(&header,sizeof(header),1,file);

			fclose(file);
		}

		bRet=TRUE;

	_final:

		return bRet;
	}

	std::vector<BYTE>_buf;
};
