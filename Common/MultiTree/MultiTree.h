/********************************************************************
	created:	27:7:2006   20:32
	file path:	d:\IxEngine\Common\MultiTree
	file base:	MultiTree
	file ext:	h
	author:		cxi
	
	purpose:	a multi-branch tree
*********************************************************************/
 #pragma once

#include <map>
#include <vector>
#include <list>

#include <assert.h>

#pragma warning(disable:4312)


typedef unsigned __int64 MultiTreeKey;

typedef void * MultiTreeBanch;

#define MULTITREEBRANCH_NULL NULL
#define MULTITREEBRANCH_ROOT ((void *)(uintptr_t)0xffffffff)


template<class leaftype,class keytype,int nosortbranch=0>
class CMultiTreeBranch
{
public:
	CMultiTreeBranch()
	{
		Clear();
	}
	std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>m_mapSubBranch;

	BOOL m_bSubBranchVectorDirty;
	std::vector<CMultiTreeBranch<leaftype,keytype,nosortbranch>*>m_vecSubBranch;

	CMultiTreeBranch<leaftype,keytype,nosortbranch>*m_pParent;
	keytype m_key;
	void BuildSubBranchVector()
	{
		m_vecSubBranch.resize(m_mapSubBranch.size());
		std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
		int count;
		count=0;
		for (it=m_mapSubBranch.begin();it!=m_mapSubBranch.end();it++)
			m_vecSubBranch[count++]=(*it).second;

		m_bSubBranchVectorDirty=FALSE;
	}

	std::vector<leaftype>m_vecLeaves;

	void Clear()
	{
		std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
		for (it=m_mapSubBranch.begin();it!=m_mapSubBranch.end();it++)
		{
			(*it).second->Clear();
			delete (*it).second;
		}
		m_mapSubBranch.clear();
		if (nosortbranch)
			m_bSubBranchVectorDirty=FALSE;
		else
			m_bSubBranchVectorDirty=TRUE;
		m_vecSubBranch.clear();
		m_vecLeaves.clear();

		m_pParent=MULTITREEBRANCH_NULL;
	}
	DWORD NumLeaf()
	{
		DWORD count;
		count=0;
		std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
		for (it=m_mapSubBranch.begin();it!=m_mapSubBranch.end();it++)
			count+=it->second->NumLeaf();

		count+=m_vecLeaves.size();
		return count;
	}

	void FillLeaf(std::vector<std::vector<leaftype>*>*pBuffer)
	{
		std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
		for (it=m_mapSubBranch.begin();it!=m_mapSubBranch.end();it++)
			(*it).second->FillLeaf(pBuffer);

		if (m_vecLeaves.size()>0)
			pBuffer->push_back(&m_vecLeaves);
	}

	void AddLeaf(std::list<keytype>&aKeys,leaftype leaf)
	{
		if (aKeys.size()>0)
		{
			keytype key;
			if (TRUE)//Pop up one
			{
				std::list<keytype>::iterator it;
				it=aKeys.begin();
				key=*it;
				aKeys.erase(it);
			}
			std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
			it=m_mapSubBranch.find(key);
			if (it!=m_mapSubBranch.end())
			{
				(*it).second->AddLeaf(aKeys,leaf);
				aKeys.push_front(key);
				return;
			}

			CMultiTreeBranch<leaftype,keytype,nosortbranch>*p;
			p=new CMultiTreeBranch<leaftype,keytype,nosortbranch>;
			p->m_key=key;
			p->m_pParent=(CMultiTreeBranch<leaftype,keytype,nosortbranch>*)this;
			m_mapSubBranch[key]=p;
			p->AddLeaf(aKeys,leaf);
			aKeys.push_front(key);
			if (nosortbranch)
				m_vecSubBranch.push_back(p);
			else
				m_bSubBranchVectorDirty=TRUE;

			return;
		}
		m_vecLeaves.push_back(leaf);
	}

	MultiTreeBanch FindBranch(std::list<keytype>&aKeys)
	{
		if (aKeys.size()>0)
		{
			keytype key;
			if (TRUE)//Pop up one
			{
				std::list<keytype>::iterator it;
				it=aKeys.begin();
				key=*it;
				aKeys.erase(it);
			}
			MultiTreeBanch ret;
			ret=MULTITREEBRANCH_NULL;
			std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
			it=m_mapSubBranch.find(key);
			if (it!=m_mapSubBranch.end())
				ret=(*it).second->FindBranch(aKeys);
			aKeys.push_front(key);
			return ret;
		}
		else
			return this;
		return MULTITREEBRANCH_NULL;
	}

	//Find the first matched leaf and return the key list to the leaf
	//Return whether found,if found,aKeys will be filled
	BOOL FindLeaf(std::list<keytype>&aKeys,leaftype leaf)
	{
		aKeys.clear();
		return FindLeaf0(aKeys,leaf);
	}

private:
	BOOL FindLeaf0(std::list<keytype>&aKeys,leaftype leaf)
	{
		int i;
		for (i=0;i<m_vecLeaves.size();i++)
		{
			if (m_vecLeaves[i]==leaf)
				return TRUE;
		}

		std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
		for (it=m_mapSubBranch.begin();it!=m_mapSubBranch.end();it++)
		{
			aKeys.push_back((*it).first);
			if ((*it).second->FindLeaf0(aKeys,leaf))
				return TRUE;
			aKeys.pop_back();
		}

		return FALSE;
	}

};


template<class leaftype,class keytype=MultiTreeKey,int nosortbranch=0>
class CMultiTree  
{
	typedef CMultiTreeBranch<leaftype,keytype,nosortbranch> MyBranch;
public:
	BOOL Begin()
	{
		m_vecIteratedLeaves.clear();
		m_Root.FillLeaf(&m_vecIteratedLeaves);
		if (m_vecIteratedLeaves.size()<=0)
			return FALSE;
		m_iCurLeafVec=0;
		m_iCurLeaf=-1;
		return TRUE;
	}
	BOOL GetNext(leaftype &leaf)
	{
		DWORD t1,t2;
		t1=m_iCurLeaf;
		t2=m_iCurLeafVec;//Back up

		m_iCurLeaf++;
		if (m_iCurLeaf<m_vecIteratedLeaves[m_iCurLeafVec]->size())
		{
			leaf=(*m_vecIteratedLeaves[m_iCurLeafVec])[m_iCurLeaf];
			return TRUE;
		}

		m_iCurLeaf=0;
		m_iCurLeafVec++;
		if (m_iCurLeafVec>=m_vecIteratedLeaves.size())
		{
			//restore
			m_iCurLeaf=t1;
			m_iCurLeafVec=t2;
			return FALSE;
		}

		leaf=(*m_vecIteratedLeaves[m_iCurLeafVec])[m_iCurLeaf];
		return TRUE;
	}

	void AddLeaf(std::list<keytype>&aKeys,leaftype leaf)
	{
		m_Root.AddLeaf(aKeys,leaf);
	}

	MultiTreeBanch GetBranch(std::list<keytype>&aKeys)
	{
		MultiTreeBanch ret;
		ret=m_Root.FindBranch(aKeys);
		if (ret==&m_Root)
			return MULTITREEBRANCH_ROOT;
		return ret;
	}
	BOOL FindBranch(std::list<keytype>&aKeys,MultiTreeBanch branch)
	{
		aKeys.clear();
		while(branch!=MULTITREEBRANCH_ROOT)
		{
			aKeys.push_front(GetBranchKey(branch));
			branch=GetParentBranch(branch);
		}
		return TRUE;
	}

	BOOL FindLeaf(std::list<keytype>&aKeys,leaftype leaf)
	{
		return m_Root.FindLeaf(aKeys,leaf);
	}
	BOOL RemoveBranch(MultiTreeBanch branch)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		if (branch==MULTITREEBRANCH_ROOT)
			return FALSE;
		MyBranch *p=(MyBranch*)branch;
		MyBranch *parent=p->m_pParent;

		std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
		it=parent->m_mapSubBranch.find(p->m_key);
		if (it!=parent->m_mapSubBranch.end())
			parent->m_mapSubBranch.erase(it);

		int i;
		for (i=0;i<parent->m_vecSubBranch.size();i++)
		{
			if (parent->m_vecSubBranch[i]==p)
				break;
		}
		if (i<parent->m_vecSubBranch.size())
			parent->m_vecSubBranch.erase(parent->m_vecSubBranch.begin()+i);

		if (!nosortbranch)
			parent->m_bSubBranchVectorDirty=TRUE;

		p->Clear();
		delete p;

		return TRUE;
	}
	BOOL ChangeKey(MultiTreeBanch branch,keytype &key,BOOL bTest=FALSE)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		if (branch==MULTITREEBRANCH_ROOT)
			return FALSE;
		MyBranch *p=(MyBranch*)branch;

		if (p->m_key==key)
			return TRUE;

		std::map<keytype,CMultiTreeBranch<leaftype,keytype,nosortbranch>*>::iterator it;
		it=p->m_pParent->m_mapSubBranch.find(key);
		if (it!=p->m_pParent->m_mapSubBranch.end())
			return FALSE;//Not unique

		if (bTest)
			return TRUE;

		it=p->m_pParent->m_mapSubBranch.find(p->m_key);
		if (it!=p->m_pParent->m_mapSubBranch.end())
			p->m_pParent->m_mapSubBranch.erase(it);
		p->m_pParent->m_mapSubBranch[key]=p;

		p->m_key=key;

		if (!nosortbranch)
			p->m_pParent->m_bSubBranchVectorDirty=TRUE;

		return TRUE;
	}

	BOOL Move(MultiTreeBanch branch,BOOL bUp)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		if (branch==MULTITREEBRANCH_ROOT)
			return FALSE;
		if (!nosortbranch)
			return FALSE;

		MyBranch *p=(MyBranch*)branch;
		MyBranch *parent=p->m_pParent;

		int i;
		for (i=0;i<parent->m_vecSubBranch.size();i++)
		{
			if (parent->m_vecSubBranch[i]==p)
				break;
		}
		assert(i<parent->m_vecSubBranch.size());
		MyBranch *t;
		if (bUp)
		{
			if (i<=0)
				return FALSE;
			t=parent->m_vecSubBranch[i-1];
			parent->m_vecSubBranch[i-1]=p;
			parent->m_vecSubBranch[i]=t;
		}
		else
		{
			if (i>=parent->m_vecSubBranch.size()-1)
				return FALSE;
			t=parent->m_vecSubBranch[i+1];
			parent->m_vecSubBranch[i+1]=p;
			parent->m_vecSubBranch[i]=t;
		}
		return TRUE;
	}


	void Clear()
	{
		m_Root.Clear();
	}

	//Browsing the tree related functions:

	//Note the root branch has an undefined key value
	//so if MULTITREEBRANCH_ROOT is passed ,an undefined key value will be returned
	keytype &GetBranchKey(MultiTreeBanch branch)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		MyBranch *p;
		if (branch!=MULTITREEBRANCH_ROOT)
			p=(MyBranch*)branch;
		else
			p=&m_Root;

		return p->m_key;
	}

	MultiTreeBanch GetParentBranch(MultiTreeBanch branch)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		MyBranch *p;
		if (branch!=MULTITREEBRANCH_ROOT)
			p=(MyBranch*)branch;
		else
			p=&m_Root;

		if (p->m_pParent==&m_Root)
			return MULTITREEBRANCH_ROOT;
		return p->m_pParent;
	}

	DWORD GetSubBranchCount(MultiTreeBanch branch)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		MyBranch *p;
		if (branch!=MULTITREEBRANCH_ROOT)
			p=(MyBranch*)branch;
		else
			p=&m_Root;

		if (p->m_bSubBranchVectorDirty)
			p->BuildSubBranchVector();
		return p->m_vecSubBranch.size();
	}

	MultiTreeBanch GetSubBranch(MultiTreeBanch branch,DWORD idx)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		MyBranch *p;
		if (branch!=MULTITREEBRANCH_ROOT)
			p=(MyBranch*)branch;
		else
			p=&m_Root;

		if (p->m_bSubBranchVectorDirty)
			p->BuildSubBranchVector();

		if (idx>=p->m_mapSubBranch.size())
			return MULTITREEBRANCH_NULL;
		return p->m_vecSubBranch[idx];
	}

	DWORD GetLeafCount(MultiTreeBanch branch)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		MyBranch *p;
		if (branch!=MULTITREEBRANCH_ROOT)
			p=(MyBranch*)branch;
		else
			p=&m_Root;

		return (DWORD)(p->m_vecLeaves.size());
	}

	BOOL GetLeaf(MultiTreeBanch branch,DWORD idx,leaftype &leaf)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		MyBranch *p;
		if (branch!=MULTITREEBRANCH_ROOT)
			p=(MyBranch*)branch;
		else
			p=&m_Root;

		if (idx>=p->m_vecLeaves.size())
			return FALSE;

		leaf=p->m_vecLeaves[idx];
		return TRUE;
	}

	void ClearLeaf(MultiTreeBanch branch)
	{
		assert(branch!=MULTITREEBRANCH_NULL);
		MyBranch *p;
		if (branch!=MULTITREEBRANCH_ROOT)
			p=(MyBranch*)branch;
		else
			p=&m_Root;
		p->m_vecLeaves.clear();
	}




private:
	MyBranch m_Root;

	//for iterating
	std::vector<std::vector<leaftype>*>m_vecIteratedLeaves;
	DWORD m_iCurLeafVec;
	DWORD m_iCurLeaf;

};

#pragma warning(default:4312)

