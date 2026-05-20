#include "stdh.h"
#include ".\modbase.h"

BOOL _Mod_Add(CModManager *modmgr,CModBase *&mod,BOOL bNewGroup)
{
	BOOL bRet;
	if(modmgr)
	{
		if (bNewGroup)
			(modmgr)->NewModGroup();
		if (!mod->IsEmpty())
		{
			bRet=(modmgr)->PushBack(mod);
			mod=NULL;
		}
		else
		{
			bRet=FALSE;
			mod->Clear();
			mod->DeleteThis();
		}
	}
	else
	{
		bRet=mod->Do();
		mod->Clear();
		mod->DeleteThis();
	}
	mod=NULL;
	return bRet;
}


BOOL Mod_New(CModManager *modmgr,CModBase *&mod)
{
	return _Mod_Add(modmgr,mod,TRUE);
}

BOOL Mod_Append(CModManager *modmgr,CModBase *&mod)
{
	return _Mod_Add(modmgr,mod,FALSE);
}



CModBase::CModBase(void)
{
	_bInverse=FALSE;
}

CModBase::~CModBase(void)
{
}

//////////////////////////////////////////////////////////////////////////
//CModManager

CModManager::CModManager()
{
	_curgroup=0;
	_capacity=0;//0表示无限
}

void CModManager::_ClearUndo()
{
	CModBase *p;
	for (int i=0;i<_vecUndoBuffer.size();i++)
	{
		p=_vecUndoBuffer[i];
		p->Clear();
		p->DeleteThis();
	}
	_vecUndoBuffer.clear();
}
void CModManager::_ClearRedo()
{
	CModBase *p;
	for (int i=0;i<_vecRedoBuffer.size();i++)
	{
		p=_vecRedoBuffer[i];
		p->Clear();
		p->DeleteThis();
	}
	_vecRedoBuffer.clear();
}


void CModManager::Clear()
{
	_ClearUndo();
	_ClearRedo();

	_curgroup=0;
}

//new a group of mod
void CModManager::NewModGroup()
{
	_curgroup++;

}

//add the mod to the tail of current mod group
//NOTE:before call this function,you should call a NewModGroup() to start a new mod group
BOOL CModManager::PushBack(CModBase *pMod,BOOL bDoIt)
{
	BOOL bSomethingDone=FALSE;
	if (TRUE)
	{
		if (_vecUndoBuffer.size()>0)
		{
			if (_vecUndoBuffer[_vecUndoBuffer.size()-1]->_group==_curgroup)
				bSomethingDone=TRUE;
		}
	}
	if (bDoIt)
	{
		if (!pMod->TestDo())
		{
			if (!bSomethingDone)
				return FALSE;
			else
			{
				Clear();
				return FALSE;
			}
		}
		if (!pMod->Do())
		{
			Clear();//Cannot recover,clear all the mod
			return FALSE;
		}
	}

	//先清除太多的mod
	if (_capacity>0)
	{
		if (_vecUndoBuffer.size()>_capacity)
		{
			if (_vecUndoBuffer[0]->_group!=_curgroup)
			{
				DWORD group=_vecUndoBuffer[0]->_group;
				while(_vecUndoBuffer.size()>0)
				{
					if (_vecUndoBuffer[0]->_group==group)
					{
						CModBase *mod=_vecUndoBuffer[0];
						mod->Clear();
						mod->DeleteThis();
						_vecUndoBuffer.pop_front();
					}
					else
						break;
				}
			}
		}
	}

	pMod->_group=_curgroup;
	_vecUndoBuffer.push_back(pMod);
	_ClearRedo();
	return TRUE;
}


BOOL CModManager::CanUndo()
{
	if (_vecUndoBuffer.size()>0)
		return TRUE;
	return FALSE;
}
void CModManager::Undo()
{
	int sz;
	sz=_vecUndoBuffer.size();
	if (sz<=0)
		return;
	DWORD group;
	group=_vecUndoBuffer[sz-1]->_group;

	int iStart=sz-1;	
	while(_vecUndoBuffer[iStart]->_group==group)
	{
		iStart--;
		if (iStart<0)
			break;
	}
	iStart++;

	BOOL bSomethingDone=FALSE;
	for (int i=iStart;i<=sz-1;i++)
	{
		BOOL bOk=TRUE;
		if (!_vecUndoBuffer[iStart]->_bInverse)
		{
			if (!_vecUndoBuffer[iStart]->TestUndo())
			{
				if (!bSomethingDone)
					break;
				else
					bOk=FALSE;
			}
			if (bOk)
				bOk=_vecUndoBuffer[iStart]->Undo();
		}
		else
		{
			if (!_vecUndoBuffer[iStart]->TestRedo())
			{
				if (!bSomethingDone)
					break;
				else
					bOk=FALSE;
			}
			if (bOk)
				bOk=_vecUndoBuffer[iStart]->Redo();
		}

		if (!bOk)
		{
			Clear();//Cannot recover,clear all the mod
			return;
		}

		_vecRedoBuffer.push_back(_vecUndoBuffer[iStart]);
		_vecUndoBuffer.erase(_vecUndoBuffer.begin()+iStart);
		bSomethingDone=TRUE;
	}
	
}
BOOL CModManager::CanRedo()
{
	if (_vecRedoBuffer.size()>0)
		return TRUE;

	return FALSE;
}
void CModManager::Redo()
{
	int sz;
	sz=_vecRedoBuffer.size();
	if (sz<=0)
		return;

	DWORD group;
	group=_vecRedoBuffer[sz-1]->_group;

	int iStart=sz-1;	
	while(_vecRedoBuffer[iStart]->_group==group)
	{
		iStart--;
		if (iStart<0)
			break;
	}
	iStart++;

	BOOL bSomethingDone=FALSE;
	for (int i=iStart;i<=sz-1;i++)
	{
		BOOL bOk=TRUE;
		if (!_vecRedoBuffer[iStart]->_bInverse)
		{
			if (!_vecUndoBuffer[iStart]->TestRedo())
			{
				if (!bSomethingDone)
					break;
				else
					bOk=FALSE;
			}
			if (bOk)
				bOk=_vecRedoBuffer[iStart]->Redo();
		}
		else
		{
			if (!_vecUndoBuffer[iStart]->TestUndo())
			{
				if (!bSomethingDone)
					break;
				else
					bOk=FALSE;
			}
			if (bOk)
				bOk=_vecRedoBuffer[iStart]->Undo();
		}

		if (!bOk)
		{
			Clear();//Cannot recover,clear all the mod
			return;
		}

		_vecUndoBuffer.push_back(_vecRedoBuffer[iStart]);
		_vecRedoBuffer.erase(_vecRedoBuffer.begin()+iStart);
		bSomethingDone=TRUE;
	}
}
