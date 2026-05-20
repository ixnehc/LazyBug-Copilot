
#pragma once

#include "commondefines/general_stl.h"

#include "stringparser/stringparser.h"

//T could be an integer type,such as int,DWORD,short,char and so on.
template <typename T>
class CRichGrid_ComboItem:public CXTPPropertyGridItem
{
public:
	CRichGrid_ComboItem(CString strCaption):CXTPPropertyGridItem(strCaption)
	{
		_str=NULL;
		_idx=NULL;
		SetFlags(xtpGridItemHasComboButton);
	}



	void Bind(std::string *s,std::vector<std::string>&constrains_)
	{
		std::vector<std::string>constrains=constrains_;
		_str=s;
		if (_str)
			SetValue(fromMBCS(_str->c_str()));

		_MakeCapHides(constrains);
		_ResetConstraint(constrains);
	}

	void Bind(T *idx,std::vector<std::string>&constrains_,std::vector<int>*remap)
	{
		std::vector<std::string>constrains=constrains_;
		_MakeCapHides(constrains);
		if (remap)
			_remap=*remap;
		else
		{// make a remap
			std::string s;
			int start=0;
			_remap.resize(constrains.size());
			for (int i=0;i<_remap.size();i++)
			{
				s=constrains[i];
				if (SeperateStringBy(":",s,constrains[i]))
					start=IntFromString(s.c_str());
				_remap[i]=start;
				start++;
			}
		}
		_idx=idx;
		if (_idx)
		{
			int v;
			VEC_FIND(_remap,*_idx,v);
			if (v!=-1)
			{
				SetValue(fromMBCS(constrains[v].c_str()));

				if (!_hides[v].empty())
				{
					static std::vector<std::string>temp;
					SplitStringBy("&",_hides[v],&temp);
					for (int j=0;j<temp.size();j++)
						GetRichGrid(this)->AddCaptionHide(temp[j].c_str());
				}
				if (!_shows[v].empty())
				{
					static std::vector<std::string>temp;
					SplitStringBy("&",_shows[v],&temp);
					for (int j=0;j<temp.size();j++)
						GetRichGrid(this)->AddCaptionShow(temp[j].c_str());
				}

			}
		}
		_ResetConstraint(constrains);
	}


	void OnValueChanged(CString strValue)
	{
		CXTPPropertyGridItem::OnValueChanged(strValue);
		if ((!_str)&&(!_idx))
			return;

		GetRichGrid(this)->OnBeginItemChange(this);
		if (_str)
			*_str = toMBCS((LPCTSTR)strValue);
		if (_idx)
		{
			DWORD v=GetConstraints()->FindConstraint(strValue);
			if (v<_remap.size())
				(*_idx)=_remap[v];
		}
		GetRichGrid(this)->OnItemChange(this);
		GetRichGrid(this)->OnEndItemChange(this);

	}
protected:

	void _MakeCapHides(std::vector<std::string>&constrains)
	{
		static std::vector<std::string>temp;

		for (int i=0;i<constrains.size();i++)
		{
			SplitStringBy("|",constrains[i],&temp);
			if (temp.size()>1)
			{
				constrains[i]=temp[0];
				_hides.push_back(temp[1]);
			}
			else
				_hides.push_back(std::string(""));

			SplitStringBy("$",constrains[i],&temp);
			if (temp.size()>1)
			{
				constrains[i]=temp[0];
				_shows.push_back(temp[1]);
			}
			else
				_shows.push_back(std::string(""));

		}
	}

	void _ResetConstraint(std::vector<std::string>&constrains)
	{
		CXTPPropertyGridItemConstraints* pList = GetConstraints();
		pList->RemoveAll();

		for (int i=0;i<constrains.size();i++)
			pList->AddConstraint(fromMBCS(constrains[i].c_str()));
	}


	std::string *_str;
	T *_idx;
	std::vector<int>_remap;
	std::vector<std::string>_hides;
	std::vector<std::string>_shows;

};




