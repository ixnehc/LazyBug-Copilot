
#pragma once

#include "commondefines/general_stl.h"

#include "stringparser/stringparser.h"

class CRichGrid_DynObjItem:public CXTPPropertyGridItem
{
public:
	CRichGrid_DynObjItem(CString strCaption):CXTPPropertyGridItem(strCaption)
	{
		_obj=NULL;
		SetFlags(xtpGridItemHasComboButton);
	}


	void Bind(void **obj,std::unordered_map<std::string,CClass*>&classes,std::unordered_map<std::string,std::string>&names)
	{
		_obj=obj;

		_ResetConstraint(classes,names);
	}


	void OnValueChanged(CString strValue)
	{
		CXTPPropertyGridItem::OnValueChanged(strValue);
		if (!_obj)
			return;

		std::unordered_map<std::string,CClass*>::iterator it;
		if (*_obj)//先把原来的指针释放掉
		{
			for (it=_classes.begin();it!=_classes.end();it++)
			{
				CClass *clss=(*it).second;
				if (clss->CheckInstance(*_obj))
				{
					clss->_del(*_obj);
					*_obj=NULL;
					break;
				}
			}
		}
		it = _classes.find(toMBCS((LPCTSTR)strValue));
		if (it!=_classes.end())
		{
			CClass *clss=(*it).second;
			GetRichGrid(this)->OnBeginItemChange(this);
			*_obj=clss->New();
			GetRichGrid(this)->OnItemChange(this);
			GetRichGrid(this)->OnEndItemChange(this);
		}
	}
protected:

	void _ResetConstraint(std::unordered_map<std::string,CClass*>&classes,std::unordered_map<std::string,std::string>&names)
	{
		CXTPPropertyGridItemConstraints* pList = GetConstraints();
		pList->RemoveAll();

		_classes.clear();

		std::unordered_map<std::string,CClass*>::iterator it;
		std::unordered_map<std::string,std::string>::iterator it2;
		for (it=classes.begin();it!=classes.end();it++)
		{
			it2=names.find((*it).first);
			assert(it2!=names.end());
			pList->AddConstraint(fromMBCS((*it2).second.c_str()));
			_classes[(*it2).second]=(*it).second;
			if ((*it).second->CheckInstance(*_obj))
				SetValue(fromMBCS((*it2).second.c_str()));
		}

		pList->Sort();
	}

	void **_obj;
	std::unordered_map<std::string,CClass*>_classes;
	std::unordered_map<std::string,CClass*>_names;

};




