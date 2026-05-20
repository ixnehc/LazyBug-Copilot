/********************************************************************
	created:	2006/8/3   16:15
	filename: 	e:\IxEngine\Common\resdata\SkeletonInfo.cpp
	author:		cxi
	
	purpose:	skeleton info
*********************************************************************/
#include "stdh.h"


#include "stringparser/stringparser.h"

#include "SkeletonInfo.h"

#include <assert.h>

#pragma warning(disable:4018)



//////////////////////////////////////////////////////////////////////////
//SkeletonInfo
DWORD SkeletonInfo::GetBranchCount()
{
	DWORD c=0;
	int i;
	for (i=0;i<size();i++)
	{
		if ((*this)[i].iParent==-1)
			c++;
	}
	return c;
}

BOOL SkeletonInfo::IsMatch(SkeletonInfo &src,BOOL bCase)
{
	if (size()!=src.size())
		return FALSE;
	int i;
	for (i=0;i<size();i++)
	{
		if (!bCase)
		{
			if (!StringEqualNoCase((*this)[i].name,src[i].name))
				return FALSE;
		}
		else
		{
			if (strcmp((*this)[i].name,src[i].name)!=0)
				return FALSE;
		}
		if((*this)[i].iParent!=src[i].iParent)
			return FALSE;
	}	
	return TRUE;
}

//not case sensitive,return -1 if not found
int SkeletonInfo::FindBone(const char *name)
{
	int i;
	for (i=0;i<size();i++)
	{
		if (StringEqualNoCase((*this)[i].name,name))
			return i;
	}
	return -1;
}

