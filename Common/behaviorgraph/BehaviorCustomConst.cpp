/********************************************************************
	created:	2014/10/01 
	author:		cxi
	
	purpose:	 Behavior Custom Const
*********************************************************************/
#include "stdh.h"
#include "BehaviorCustomConst.h"

REGISTER_BCC_CLASS(BccArea,"ÇøÓò")


BccClasses *BccClass_GetSingleton()
{
	static BccClasses clsses;
	return &clsses;
}

void BccClasses::Add(CClass *clss,const char *showname)
{
	Entry *e=&clsses[clss->_classname];
	e->clss=clss;
	e->showname=showname;
}

CClass *BccClasses::Find(const char *nmClss)
{
	std::unordered_map<std::string,Entry>::iterator it=clsses.find(std::string(nmClss));
	if (it==clsses.end())
		return NULL;
	return (*it).second.clss;
}



BccClassRegister::BccClassRegister(CClass *clss,const char *showname)
{
	BccClass_GetSingleton()->Add(clss,showname);
}




//////////////////////////////////////////////////////////////////////////
//BccRoute
void BccRoute::UpdateDistsToGo()
{
	if (distsToGo.size()==sphereset.size())
		return;//already cached

	distsToGo.resize(sphereset.size());
	for (int i=sphereset.size()-1;i>=0;i--)
	{
		int iNext=i+1;
		if (iNext>=sphereset.size())
			iNext=sphereset.size()-1;

		distsToGo[i]=sphereset[i].center.getDistanceXZFrom(sphereset[iNext].center);
		if (iNext>i)
			distsToGo[i]+=distsToGo[iNext];
	}
}
