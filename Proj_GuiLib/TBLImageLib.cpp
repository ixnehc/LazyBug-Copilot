/********************************************************************
	created:	2007/2/8   12:40
	filename: 	e:\IxEngine\Proj_GuiLib\TBLImageLib.cpp
	author:		cxi
	
	purpose:	image lib for brlib
*********************************************************************/

#include "stdh.h"
#include "TBLImageLib.h"

#include "RenderSystem/IUtilRS.h"
#include "WorldSystem/IWorldSystem.h"
#include "ximage.h"
#include "ImageBase.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CTBLImageLib

void CTBLImageLib::SetBrLib(ITrrnBrushLib *brlib)
{
	SAFE_REPLACE(_brlib,brlib);
}


void CTBLImageLib::TexSet::Clear()
{
	for (int i=0;i<MAX_BRUSH_TEX;i++)
	{
		SAFE_DELETE(images[i]);
		SAFE_DELETE(images_ns[i]);
	}
}

void CTBLImageLib::Clear()
{
	std::map<TTSID,TexSet>::iterator it2;
	for (it2=_texsets.begin();it2!=_texsets.end();it2++)
		(*it2).second.Clear();
	_texsets.clear();
}

void CTBLImageLib::SyncForAll()
{
	Clear();

	if ((!_brlib)||(!_pRS))
		return;

	DWORD c;

	for (int j=0;j<MAX_TRRN_BRUSHLEVEL;j++)
	{
		c=_brlib->GetTexSetCount(j);
		for(int i=0;i<c;i++)
			SyncForTexSet(_brlib->GetTexSetID(j,i));
	}

}



void CTBLImageLib::SyncForTexSet(TTSID idTexSet)
{
	assert(_brlib);
	assert(_pRS);


	std::map<TTSID,TexSet>::iterator itTexSet;

	//first remove the tex set in image lib
	itTexSet=_texsets.find(idTexSet);
	if (itTexSet!=_texsets.end())
	{
		(*itTexSet).second.Clear();
		_texsets.erase(itTexSet);
	}

	//Now build a new tex set from brlib and add to image lib
	std::string name=_brlib->GetTexSetName(idTexSet);
	if (name!="")
	{//this is a valid TTSID
		TexSet ts;
		for (int i=0;i<MAX_BRUSH_TEX;i++)
		{
			TexData *data=_brlib->GetTexData(idTexSet,i,FALSE);
			if (!data)
				break;//No more
			ts.images[i]=ImageFromTexData(_pRS,data);
			data=_brlib->GetTexData(idTexSet,i,TRUE);
			if (data)
				ts.images_ns[i]=ImageFromTexData(_pRS,data);
		}
		_texsets[idTexSet]=ts;
	}
}


DWORD CTBLImageLib::GetTexCount(TTSID idTexSet)
{
	std::map<TTSID,TexSet>::iterator it;
	it=_texsets.find(idTexSet);
	if (it==_texsets.end())
		return NULL;

	int i;
	for (i=0;i<MAX_BRUSH_TEX;i++)
	{
		if (!(*it).second.images[i])
			break;
	}
	return i;
}



CxImage *CTBLImageLib::GetTex(TTSID idTexSet,DWORD iTex,BOOL bNS)
{
	std::map<TTSID,TexSet>::iterator it;
	it=_texsets.find(idTexSet);
	if (it==_texsets.end())
		return NULL;

	if (iTex>=MAX_BRUSH_TEX)
		return NULL;

	if (!bNS)
		return (*it).second.images[iTex];
	return (*it).second.images_ns[iTex];
}