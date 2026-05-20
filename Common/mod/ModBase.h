#pragma once

#include <deque>

class CModManager;
class CModBase;

extern BOOL Mod_New(CModManager *modmgr,CModBase *&mod);
extern BOOL Mod_Append(CModManager *modmgr,CModBase *&mod);


class CModBase
{
public:
	CModBase(void);
	virtual ~CModBase(void);

	virtual void DeleteThis()	{		delete this;	}

	virtual BOOL IsEmpty()=0;
	virtual void Clear()	{	}

	virtual BOOL TestUndo()	{		return TRUE;	}
	virtual BOOL TestRedo()	{		return TRUE;	}
	virtual BOOL Undo()=0;
	virtual BOOL Redo()=0;

	void SetInverse(BOOL bInverse)	{		_bInverse=bInverse;	}

	BOOL TestDo()
	{
		if (IsEmpty())
			return FALSE;
		if (_bInverse)
			return TestUndo();
		else
			return TestRedo();
	}

	BOOL Do()
	{
		if (_bInverse)
			return Undo();
		else
			return Redo();
	}


protected:

	//indicate whether this mod's operation is inversed,
	//for re-use of a same mod class in both the direction.
	BOOL _bInverse;

private:
	BOOL _group;//this is managed by the CModManager


	friend class CModManager;
};

class CModManager
{
public:
	CModManager();
	void Clear();
	void SetCapacity(DWORD nCap)	{		_capacity=nCap;	}
	void NewModGroup();//new a group of mod

	//add the mod to the tail of current mod group
	BOOL PushBack(CModBase *pMod,BOOL bDoIt=TRUE);


	BOOL CanUndo();
	void Undo();//undo a group of mod
	BOOL CanRedo();
	void Redo();//redo a group of mod

private:
	void _ClearUndo();
	void _ClearRedo();

	DWORD _capacity;

	std::deque<CModBase *>_vecUndoBuffer;
	std::deque<CModBase *>_vecRedoBuffer;

	DWORD _curgroup;

};