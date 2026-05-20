/********************************************************************
	created:	2006/8/3   16:14
	filename: 	e:\IxEngine\Common\resdata\ResDataGroup.cpp
	author:		cxi
	
	purpose:	resource data group( map of resdata) management
*********************************************************************/
#include "stdh.h"
#include "ResData.h"
#include "ResDataGroup.h"

#include "stringparser/stringparser.h"

#include <assert.h>

#include <algorithm>
#include <functional>      // For greater<int>( )


void ResDataGroup::PushBack(const char *path,ResData *resdata)
{
	RDGroupItem t;
	t.path=path;
	t.resdata=resdata;
	push_back(t);
}

int ResDataGroup::Find(const char *pathToFind)
{
	for (int i=0;i<size();i++)
	{
		if ( StringEqualNoCase((*this)[i].path.c_str(),pathToFind))
			return i;
	}
	return -1;
}


BOOL ResDataGroup::Copy(ResDataGroup&src)
{
	Clean();
	resize(src.size());
	//Make a copy to my own data
	for (int i=0;i<size();i++)
	{
		(*this)[i].path=src[i].path;
		(*this)[i].resdata=ResData_Clone(src[i].resdata);
	}
	return TRUE;
}


void ResDataGroup::Clean()
{
	for (int i=0;i<size();i++)
		ResData_Delete((*this)[i].resdata);
	clear();
}

void ResDataGroup::Erase(int i)
{
	ResData_Delete((*this)[i].resdata);
	erase(begin()+i);
}


//move another map into me(src will be cleared),if duplication key found,auto-generate a unique name
BOOL ResDataGroup::Move(ResDataGroup&src)
{
	for (int i=0;i<src.size();i++)
	{
		ResData *p;
		p=src[i].resdata;
		std::string s,ss;
		ss=s=src[i].path;

		int count=0;
		while (Find(ss.c_str())!=-1)//duplication found
			FormatString(ss,"%s_%02d",s.c_str(),++count);
		PushBack(ss.c_str(),p);
	}
	src.clear();//NOTE:not Clean()

	return TRUE;
}


BOOL ResDataGroup::Merge(ResDataGroup&src)
{
	for (int i=0;i<src.size();i++)
	{
		ResData *p;
		p=src[i].resdata;
		std::string s,ss;
		ss=s=src[i].path;

		int count=0;
		while (Find(ss.c_str())!=-1)//duplication found
			FormatString(ss,"%s_%02d",s.c_str(),++count);
		ResData *q;
		q=ResData_Clone(p);
		assert(q);
		PushBack(ss.c_str(),q);
	}
	return TRUE;
}

bool RDGroupItemGT(const RDGroupItem&Left,const RDGroupItem&Right)
{
	BOOL bNode1,bNode2;
	bNode1=(Left.resdata==NULL)||(-1!=Left.path.find(C_RES_LINKER));
	bNode2=(Right.resdata==NULL)||(-1!=Right.path.find(C_RES_LINKER));
	if (bNode1==bNode2)
		return (StringCmpNoCase(Left.path.c_str(),Right.path.c_str())<0);
	if (bNode1&&(!bNode2))
		return true;
	return false;
}

void ResDataGroup::Sort()
{
	std::sort(begin(),end(),RDGroupItemGT);
}

//////////////////////////////////////////////////////////////////////////
//ResDataList
void ResDataList::FromRdg(ResDataGroup &rdg)
{
	resize(rdg.size());
	for (int i=0;i<rdg.size();i++)
	{
		(*this)[i].path=rdg[i].path;
		(*this)[i].type=rdg[i].resdata?rdg[i].resdata->GetType():Res_Node;
	}
}
