

#include "stdh.h"
#include "GuiData_ValueSet.h"
#include "Log/LogDump.h"

#include "RichGrid.h"

#include "RichGridValueSetItem.h"

#include "WndBase.h"

//////////////////////////////////////////////////////////////////////////
//ValueSetEntry

CRichGrid_ValueSetItem*ValueSetEntry::GetItem()
{
	if (!ref)
		return NULL;
	return (CRichGrid_ValueSetItem*)ref->GetStuff();
}

ValueSet *ValueSetEntry::GetValueSet(std::string *path)
{
	if (path)
		*path="";
	CRichGrid_ValueSetItem* item=GetItem();
	if (!item)
		return NULL;
	ValueSet *vs=item->GetBind();
	if (!vs)
		return NULL;
	if (path)
		*path=((CRichGrid*)item->GetGrid()->GetPropertyGrid())->PathFromItem(item);
	return vs;
}



//////////////////////////////////////////////////////////////////////////
//ValueSetGroup
void ValueSetGroup::AddEntry(CRichGrid *grid,CXTPPropertyGridItem*item,const char *classname)
{
	ValueSetEntry entry;

	if (std::string("CRichGrid_ValueSetItem")==classname)
	{
		entry.ref=((CRichGrid_ValueSetItem*)item)->ObtainRef();
		entries.push_back(entry);
	}

	ver++;
}

void ValueSetGroup::ClearEntries()
{
	for (int i=0;i<entries.size();i++)
	{
		SAFE_RELEASE(entries[i].ref);
	}

	entries.clear();

	ver++;
}

i_math::rectf ValueSetGroup::CalcBound(const char *selentry,BOOL bSelOnly,BOOL bClose)
{
	i_math::rangef r;
	i_math::rectf rc;
	for (int i=0;i<entries.size();i++)
	{
		std::string s;
		ValueSet *vs=entries[i].GetValueSet(&s);
		if (!vs)
			continue;
		if (bSelOnly)
		{
			if (s!=selentry)
				continue;
		}
		else
		{
			if ((s!=selentry)&&(!vs->_bVisible))
				continue;//即没选中,也不可见
		}
		CRichGrid_ValueSetItem*item=entries[i].GetItem();

		if (vs->GetKeyType()==KT_Float)
		{
			if (vs->GetKeyCount()==0)
				continue;
			i_math::rectf rcSub;
			for (int i=0;i<vs->GetKeyCount();i++)
			{
				Key_f *k=(Key_f *)vs->GetKey(i);
				i_math::pos2df pt;
				pt.x=ANIMTICK_TO_SECOND(k->t);
				pt.y=k->v;
				rcSub.merge(pt);
			}

			rcSub.Right()-=1.0f;
			rcSub.Bottom()-=1.0f;

			i_math::rectf rcRange=item->GetRangeRect();

			if (rcSub.Right()<=rcSub.Left())
			{
				rcSub.Left()=rcRange.Left();
				rcSub.Right()=rcRange.Right();
			}

			if (rcSub.Bottom()-rcSub.Top()<0.001f)
			{
				if (!bClose)
				{
					rcSub.Bottom()=rcRange.Bottom();
					rcSub.Top()=rcRange.Top();
				}
				else
				{
					if (rcSub.Bottom()>=0.0f)
					{
						if (rcSub.Bottom()<0.2f)
							rcSub.Bottom()=0.2f;
						rcSub.Top()=-rcSub.Bottom();
						rcSub.Bottom()=2.0f*rcSub.Bottom();
					}
					else
					{
						if (rcSub.Bottom()>-0.2f)
							rcSub.Bottom()=-0.2f;

						rcSub.Top()=rcSub.Bottom()*2.0f;
						rcSub.Bottom()=-rcSub.Bottom();
					}
				}
			}
			else
			{
				float gap=rcSub.Bottom()-rcSub.Top();
				rcSub.Bottom()+=gap;
				rcSub.Top()-=gap;
			}


			rcSub.inflate(rcSub.getWidth()/16.0f,rcSub.getHeight()/8.0f,
								rcSub.getWidth()/8.0f,rcSub.getHeight()/8.0f);	

			//往外扩一点,以避免一个太小的rect
			if (TRUE)
			{
				float dx,dy;
				dx=(0.01f-rcSub.getWidth())/2.0f;
				dy=(0.01f-rcSub.getHeight())/2.0f;
				if (dx<0.0f)
					dx=0.0f;
				if (dy<0.0f)
					dy=0.0f;
				rcSub.inflate(dx,dy,dx,dy);
			}

			rc.merge(rcSub);
		}
		if (vs->GetKeyType()==KT_Color)
		{
			if (vs->GetKeyCount()==0)
				continue;
			i_math::rangef rSub;
			for (int i=0;i<vs->GetKeyCount();i++)
			{
				Key_col *k=(Key_col*)vs->GetKey(i);
				rSub.merge(ANIMTICK_TO_SECOND(k->t));
			}

			if (rSub.low>=rSub.hi)
			{
				i_math::rectf rc=item->GetRangeRect();
				rSub.low=rc.Left();
				rSub.hi=rc.Right();
			}

			rSub.inflate((rSub.hi-rSub.low)/8.0f,(rSub.hi-rSub.low)/8.0f);
			r.merge(rSub);
		}
	}

	if (r.hi>=r.low)
	{
		if (rc.isValid())
		{
			if (rc.Left()>r.low)
				rc.Left()=r.low;
			if (rc.Right()<r.hi)
				rc.Right()=r.hi;
		}
		else
		{
			rc.Left()=r.low;
			rc.Right()=r.hi;
			rc.Top()=0.0f;
			rc.Bottom()=1.0f;
		}
	}

	return rc;
}




//////////////////////////////////////////////////////////////////////////
//CGuiData_ValueSet

CGuiData_ValueSet::CGuiData_ValueSet()
{	
	_colpage=NULL;
}

CGuiData_ValueSet::~CGuiData_ValueSet()
{
}


void	CGuiData_ValueSet::Clear()
{
	std::unordered_map<std::string,ValueSetGroup*>::iterator it;
	for (it=_grps.begin();it!=_grps.end();it++)
	{
		ValueSetGroup*grp=(*it).second;
		grp->ClearEntries();
		Safe_Class_Delete(grp);
	}
	_grps.clear();
}

void CGuiData_ValueSet::SetGrpGrid(const char *grp,CRichGrid *grid)
{
	ValueSetGroup*pGrp=_EnsureGrp(grp);

	Ref *t=grid->ObtainRef();
	SAFE_RELEASE(pGrp->refGrid);
	pGrp->refGrid=t;
}

CRichGrid *CGuiData_ValueSet::GetGrid()
{
	ValueSetGroup *pGrp=GetSelGroup();
	if (!pGrp)
		return NULL;
	if (!pGrp->refGrid)
		return NULL;
	return (CRichGrid *)pGrp->refGrid->GetStuff();
}


ValueSetGroup * CGuiData_ValueSet::_EnsureGrp(const char *grp)
{
	std::unordered_map<std::string,ValueSetGroup*>::iterator it=_grps.find(std::string(grp));
	ValueSetGroup*pGrp;
	if (it!=_grps.end())
		pGrp=(*it).second;
	else
	{
		pGrp=Class_New2(ValueSetGroup);
		_grps[std::string(grp)]=pGrp;
	}
	return pGrp;
}


void CGuiData_ValueSet::AddEntry(const char *grp,CRichGrid *grid,CXTPPropertyGridItem*item,const char *classname)
{
	ValueSetGroup*pGrp=_EnsureGrp(grp);
	pGrp->AddEntry(grid,item,classname);
}



void CGuiData_ValueSet::ClearGroup(const char *grp)
{
	ValueSetGroup*pGrp=FindGroup(grp);
	if (pGrp)
		pGrp->ClearEntries();

}

ValueSetGroup *CGuiData_ValueSet::FindGroup(const char *grp)
{
	std::unordered_map<std::string,ValueSetGroup*>::iterator it=_grps.find(std::string(grp));
	if (it==_grps.end())
		return NULL;
	return (*it).second;
}

//根据所有RichGrid的可见与否/Focus与否,选择一个Grp作为当前选中的Grp
BOOL CGuiData_ValueSet::UpdateSelGrp()
{
	std::string s;
	std::string sel,sel2;//sel比sel2优先
	std::unordered_map<std::string,ValueSetGroup*>::iterator it;
	for (it=_grps.begin();it!=_grps.end();it++)
	{
		s=(*it).first;
		ValueSetGroup *pGrp=(*it).second;
		if (!pGrp->refGrid)
			continue;
		CRichGrid *wnd=(CRichGrid *)pGrp->refGrid->GetStuff();
		if (!wnd)
			continue;
		if (!wnd->IsWindowVisible())
			continue;

		CWnd *wndFocus=CWnd::GetFocus();
		if ((wndFocus==wnd)||CheckWndDescendant(wnd,wndFocus))
		{
			if (_selgrp==s)
				return FALSE;
			_selgrp=s;
			return TRUE;
		}
		if (s==_selgrp)//与原来选中的一样
			sel=s;
		sel2=s;
	}

	if (sel=="")
		sel=sel2;

	if (sel==_selgrp)
		return FALSE;
	_selgrp=sel;
	return TRUE;
}


ValueSetEntry*CGuiData_ValueSet::GetSelEntry()
{
	ValueSetGroup *pGrp=GetSelGroup();
	if (!pGrp)
		return NULL;
	for (int i=0;i<pGrp->entries.size();i++)
	{
		std::string s;
		ValueSet *vs=pGrp->entries[i].GetValueSet(&s);
		if (!vs)
			continue;
		if (s==_selentry)
			return &pGrp->entries[i];
	}
	return NULL;
}

void CGuiData_ValueSet::_UpdateSelKey()
{
	if (_selkey.grp==_selgrp)
	{
		if (_selkey.entry==_selentry)
		{
			ValueSetEntry *entry=GetSelEntry();
			if (entry)
			{
				ValueSet *vs=entry->GetValueSet(NULL);
				if (vs)
				{
					if (_selkey.idxKey>=vs->GetKeyCount())
						_selkey.idxKey=-1;
					return;
				}
			}
		}
	}
	_selkey.grp=_selgrp;
	_selkey.entry=_selentry;
	_selkey.idxKey=-1;
}

int CGuiData_ValueSet::GetSelKey()
{
	_UpdateSelKey();
	return _selkey.idxKey;
}

void CGuiData_ValueSet::SetSelKey(int idxKey)
{
	_UpdateSelKey();
	_selkey.idxKey=idxKey;
}
