#include "stdh.h"

#include "GuiData_OverallMap.h"

#include "FileSystem/IMapFile.h"

#include "WorldSystem/IMiniMapBaker.h"
#include "WorldSystem/IOutlineMapBaker.h"

#include "commondefines/general_stl.h"

#include "bitset/bitset.h"


void GuiData_OverallMap::Clear()
{
	fldSels.clear();
	SAFE_DELETE((Gdiplus::Image *&)pImage);

	if (TRUE)
	{
		std::map<i_math::pos2di,MiniMapField*>::iterator it;
		for (it=fldsRawMiniMap.begin();it!=fldsRawMiniMap.end();it++)
		{
			MiniMapField*p=(*it).second;
			Safe_Class_Delete(p);
		}

		fldsRawMiniMap.clear();
	}

	if (TRUE)
	{
		std::map<i_math::pos2di,OutlineMapField*>::iterator it;
		for (it=fldsOutlineMap.begin();it!=fldsOutlineMap.end();it++)
		{
			OutlineMapField*p=(*it).second;
			Safe_Class_Delete(p);
		}

		fldsOutlineMap.clear();
	}

	Zero();
}


DWORD *GuiData_OverallMap::GetFieldRawMiniMap(i_math::pos2di &ptFld,DWORD &w,DWORD &h)
{
	MiniMapField *fld=NULL;
	std::map<i_math::pos2di,MiniMapField*>::iterator it;
	it=fldsRawMiniMap.find(ptFld);
	if (it!=fldsRawMiniMap.end())
		fld=(*it).second;
	else
	{
		DWORD wFld=mf->GetFieldWidth()*BLOCK_LENGTH;
		i_math::recti rc;
		rc.set(ptFld.x*wFld,ptFld.y*wFld,(ptFld.x+1)*wFld,(ptFld.y+1)*wFld);

		rc.scale_signed(MINIMAPBLOCK_LEN);

		MiniMapBlock blk;

		for (int i=rc.Left();i<rc.Right();i++)
		for (int j=rc.Top();j<rc.Bottom();j++)
		{
			i_math::pos2di ptBlk(i,j);
			ptBlk*=(MINIMAPBLOCK_LEN/BLOCK_LENGTH);

			BYTE *data;
			DWORD szData;
			if (FALSE==mf->Load(ptBlk,MapChannel_RawMiniMap,data,szData))
				continue;
			if (!data)
				continue;

			CDataPacket dp;
			dp.SetDataBufferPointer(data);
			blk.Load(dp);

			if (!fld)
			{
				fld=Class_New2(MiniMapField);
				fld->len=blk.len*rc.getWidth();
				fld->data.resize(fld->len*fld->len);
			}

			int x,y;
			x=i-rc.Left();
			y=j-rc.Top();

			DWORD *p0=&fld->data[(rc.getHeight()-y-1)*blk.len*fld->len+x*blk.len];
			DWORD *q=&blk.data[0];

			for (int jj=0;jj<blk.len;jj++)
			{
				DWORD *p=p0+jj*fld->len;
				memcpy(p,q,sizeof(DWORD)*blk.len);
				q+=blk.len;
			}
		}

		fldsRawMiniMap[ptFld]=fld;
	}
	if (!fld)
		return NULL;

	w=h=fld->len;
	return &fld->data[0];
}

void GuiData_OverallMap::DiscardFieldRawMiniMapCache(i_math::pos2di &ptFld)
{
	std::map<i_math::pos2di,MiniMapField*>::iterator it;
	it=fldsRawMiniMap.find(ptFld);
	if (it!=fldsRawMiniMap.end())
	{
		MiniMapField*p=(*it).second;
		Safe_Class_Delete(p);
		fldsRawMiniMap.erase(it);
	}
	
}


DWORD*GuiData_OverallMap::GetFieldOutlineMap(i_math::pos2di &ptFld,DWORD &w,DWORD &h)
{
	OutlineMapField *fld=NULL;
	std::map<i_math::pos2di,OutlineMapField*>::iterator it;
	it=fldsOutlineMap.find(ptFld);
	if (it!=fldsOutlineMap.end())
		fld=(*it).second;
	else
	{
		DWORD wFld=mf->GetFieldWidth()*BLOCK_LENGTH;
		i_math::recti rc;
		rc.set(ptFld.x*wFld,ptFld.y*wFld,(ptFld.x+1)*wFld,(ptFld.y+1)*wFld);

		rc.scale_signed(OUTLINEMAPBLOCK_LEN);

		OutlineMapBlock blk;

		for (int i=rc.Left();i<rc.Right();i++)
		for (int j=rc.Top();j<rc.Bottom();j++)
		{
			i_math::pos2di ptBlk(i,j);
			ptBlk*=(OUTLINEMAPBLOCK_LEN/BLOCK_LENGTH);

			BYTE *data;
			DWORD szData;
			if (FALSE==mf->Load(ptBlk,MapChannel_OutlineMap,data,szData))
				continue;
			if (!data)
				continue;

			CDataPacket dp;
			dp.SetDataBufferPointer(data);
			blk.Load(dp);

			DWORD lenBlk=OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO;

			if (!fld)
			{
				fld=Class_New2(OutlineMapField);
				fld->len=lenBlk*rc.getWidth();
				fld->data.resize(fld->len*fld->len);
			}

			int x,y;
			x=i-rc.Left();
			y=j-rc.Top();

			DWORD *p0=&fld->data[(rc.getHeight()-y-1)*lenBlk*fld->len+x*lenBlk];

			Bitset<OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO*OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO/32> bs;
			Bitset<OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO*OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO/32> bs2;

			bs.resize(OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO*OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO);
			bs2.resize(OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO*OUTLINEMAPBLOCK_LEN*OUTLINEMAP_RESO);

			assert(bs.getdatasize()==blk.dataRgn.size());
			memcpy(bs.getdata(),&blk.dataRgn[0],blk.dataRgn.size());
			memcpy(bs2.getdata(),&blk.dataOutline[0],blk.dataOutline.size());

			DWORD idxBit=0;
			for (int jj=0;jj<lenBlk;jj++)
			{
				DWORD *p=p0+jj*fld->len;
				for (int ii=0;ii<lenBlk;ii++)
				{
					BOOL b,b2;
					b=bs.test(idxBit);
					b2=bs2.test(idxBit);
					idxBit++;

					if (b2)
						p[ii]=0xffffffff;
					else
					{
						if (b)
							p[ii]=0x3f7f7f7f;
						else
							p[ii]=0x0;
					}
				}
			}
		}

		fldsOutlineMap[ptFld]=fld;
	}
	if (!fld)
		return NULL;

	w=h=fld->len;
	return &fld->data[0];
}

void GuiData_OverallMap::DiscardFieldOutlineMap(i_math::pos2di &ptFld)
{
	std::map<i_math::pos2di,OutlineMapField*>::iterator it;
	it=fldsOutlineMap.find(ptFld);
	if (it!=fldsOutlineMap.end())
	{
		OutlineMapField*p=(*it).second;
		Safe_Class_Delete(p);
		fldsOutlineMap.erase(it);
	}

}
