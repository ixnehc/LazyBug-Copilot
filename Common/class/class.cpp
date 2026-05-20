/********************************************************************
	created:	2007/7/5   17:43
	filename: 	e:\IxEngine\Common\class\class.cpp
	author:		cxi
	
	purpose:	General Runtime Class Info
*********************************************************************/
#include "stdh.h"

#include "class.h"

#include <assert.h> 

std::vector<CClass *> &CClass::_classes()
{
	static std::vector<CClass *>classes;
	return classes;
}
std::map<std::string,CClass*> *CClass::_classmap()
{
	static std::map<std::string,CClass*> classmap[MAX_CLASS_LIB];
	return (std::map<std::string,CClass*> *)&classmap[0];
}
BOOL *CClass::_libempty()
{
	static BOOL libempty[MAX_CLASS_LIB];
	static BOOL bInit=FALSE;
	if (!bInit)
	{
		for (int i=0;i<ARRAY_SIZE(libempty);i++)
			libempty[i]=TRUE;
		bInit=TRUE;
	}
	
	return libempty;
}
DWORD &CClass::_nlib()
{
	static DWORD nlib=0;
	return nlib;
}



int CClass::AllocLib()
{
	for (int i=0;i<_nlib();i++)
	{
		if (_libempty()[i])
		{
			_libempty()[i]=FALSE;
			return i+1;
		}
	}
	if (_nlib()>=MAX_CLASS_LIB)
		return -1;
	_nlib()++;
	_libempty()[_nlib()-1]=FALSE;
	return _nlib()-1+1;
}

void CClass::FreeLib(int lib)
{
	lib--;
	if (((DWORD)lib)<MAX_CLASS_LIB)
		_libempty()[lib]=TRUE;
}


void CClass::Enum(int lib,DWORD flag,std::vector<std::string>&result)
{
	result.clear();
	for (int i=1;i<_classes().size();i++)
	{
		if (_classes()[i])
		{
			if (((_classes()[i]->_flag&flag)==flag)&&(lib==_classes()[i]->_lib))
				result.push_back(_classes()[i]->_classname);
		}
	}
	
}

void CClass::Enum(int lib,DWORD flag,std::vector<CClass*>&result)
{
	result.clear();
	for (int i=1;i<_classes().size();i++)
	{
		if (_classes()[i])
		{
			if (((_classes()[i]->_flag&flag)==flag)&&(lib==_classes()[i]->_lib))
				result.push_back(_classes()[i]);
		}
	}
}



CClass *CClass::Find(const char *classname,int lib)
{
	std::map<std::string,CClass*>::iterator it;
	it=CClass::_classmap()[lib].find(std::string(classname));
	if (it==CClass::_classmap()[lib].end())
	{
		if (lib!=0)
		{
			it=CClass::_classmap()[0].find(std::string(classname));
			if (it==CClass::_classmap()[0].end())
				return NULL;
		}
		else
			return NULL;
	}
	return (*it).second;
}

CClass *CClass::Find(ClassID classid)
{
	if ((classid>=CClass::_classes().size())||(classid<1))
		return NULL;

	return CClass::_classes()[classid];
}

void *CClass::New()
{
	CClass *cl=this;
	return cl->_new();
}


BOOL CClass::CheckInstance(void *p)
{
	DWORD c;
	void **intstances=GetInstances(c);
	for (int i=0;i<c;i++)
	{
		if (p==intstances[i])
			return TRUE;
	}
	return FALSE;
}



void *CClass::New(const char *classname,int lib)
{
	CClass *cl=Find(classname,lib);
	if (!cl)
		return NULL;
	return cl->New();
}

void *CClass::New(ClassID idClass)
{
	CClass *cl=Find(idClass);
	if (!cl)
		return NULL;
	return cl->New();
}


void CClass::SetName(const char *classname)
{
	if (_classname!="")
	{
		//Remove old
		std::map<std::string,CClass*>::iterator it=_classmap()[_lib].find(_classname);
		if (it!=_classmap()[_lib].end())
			_classmap()[_lib].erase(it);

		_classes()[_classid]=NULL;
	}
	_classname=classname;

	if (_classname!="")
	{
		if (_classes().size()==0)
			_classes().push_back(NULL);

		int i;
		for (i=1;i<_classes().size();i++)
		{
			if (!_classes()[i])
				break;
		}
		if (i>=_classes().size())
			_classes().push_back(NULL);
		_classid=i;
		_classes()[i]=this;

		assert(_classmap()[_lib].find(_classname)==_classmap()[_lib].end());
		_classmap()[_lib][_classname]=this;
	}
}

CClass::CClass()
{
	_classname="";
	_classid=ClassID_Null;

	_basename="";
	_baseid=ClassID_Null;

	_uid=0;

	_depth=-1;

	_lib=0;
	_flag=0;
}


void CClass::SetBaseName(const char *basename)	
{		
	_basename=basename;		
	_baseid=0;	
}


ClassID CClass::GetBaseID()
{
	if (_baseid==ClassID_Null)
	{
		if (_basename!="")
		{
			CClass *clss=Find(_basename.c_str(),_lib);
			if (!clss)
				_basename="";
			else
				_baseid=clss->GetID();
		}
	}
	return _baseid;
}

CClass *CClass::GetBase()
{
	return Find(GetBaseID());
}


DWORD CClass::GetDepth()
{
	if (_depth>=0)
		return _depth;

	//calculate inherit depth
	_depth=-1;
	CClass *p=this;
	while(p)
	{
		_depth++;
		p=Find(p->GetBaseID());
	}

	return _depth;
}
