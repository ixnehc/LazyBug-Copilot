
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/IEntitySystemDefines.h"
#include "protograph.h"

#include "GuiAgent_2DTransform.h"

#include "GuiAgent_3dnodeedit.h"

class IMesh;
class IMtrl;
class ILight;

class CRichGrid_MatSetItem;
class CRichGrid;

struct RemoteItemData_MatSet
{
	RemoteItemData_MatSet()
	{
		Zero();
	}
	void Zero()
	{
		bMats=FALSE;
		bVecs=FALSE;
		bSphs=FALSE;
	}
	void Clear()
	{
		mats_.clear();
		vecs.clear();
		sphs.clear();
		Zero();
	}
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	BOOL bMats;
	BOOL bVecs;
	BOOL bSphs;
	std::vector<i_math::matrix43f> mats_;
	std::vector<i_math::vector3df> vecs;
	std::vector<i_math::spheref> sphs;
	std::string mode;
};

class CRemoteItem_MatSet
{
public:
	CRemoteItem_MatSet()
	{
		_ver=0;
		_uidItem=0;
	}
	void Reset();
	BOOL IsValid()	{		return GetItemUID()!=0;	}
	std::vector<i_math::matrix43f> *GetBindMats()	{		return _data.bMats?&_data.mats_:NULL;	};
	std::vector<i_math::vector3df> *GetBindVecs()	{		return _data.bVecs?&_data.vecs:NULL;	};
	std::vector<i_math::spheref> *GetBindSphs()	{		return _data.bSphs?&_data.sphs:NULL;	};
	const char *GetMode()	{		return _data.mode.c_str();	}
	BOOL IsLS()	{		return FALSE;	}

	void Update();

	DWORD GetItemUID()	{		return _uidItem;	}

	void OnBeginChange()	{	}
	void OnChange()	{	}
	void OnEndChange();

protected:

	void _IncVer()	{		_ver++;	}

	RemoteItemData_MatSet _data;

	DWORD _uidItem;
	DWORD _ver;

	std::vector<BYTE>_bufTemp;
};




class CGuiAgent_MatSet:public CGuiAgent_3DNodeMatEdit
{
public:
	CGuiAgent_MatSet()
	{
		_bWorking=FALSE;
		_bNewMat=FALSE;
		_hangHit=0.0f;

		_mesh=NULL;
		_mtrl=NULL;
		_lgt=NULL;

		_curRemoteUID=0;

		_bEnableRemote=TRUE;
	}

	struct Binding
	{
		Binding()
		{
			remote=NULL;
			grid=NULL;
			item=NULL;
		}
		BOOL IsValid();
		std::vector<i_math::matrix43f> *GetBindMats();
		std::vector<i_math::vector3df> *GetBindVecs();
		std::vector<i_math::spheref> *GetBindSphs();
		BOOL IsLS();
		const char *GetMode();

		void OnBeginChange();
		void OnChange();
		void OnEndChange();

		CRemoteItem_MatSet *remote;
		CRichGrid * grid;
		CRichGrid_MatSetItem *item;

	};

	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	virtual BOOL OnDraw();
	virtual BOOL OnTimer(int dt,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnMouseWheel(int delta,DWORD flag){return TRUE;}
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

	void EnableRemote(BOOL bEnable)	{		_bEnableRemote=bEnable;	}

protected:

	virtual void _BeginMatrixEdit(i_math::matrix43f *mat);
	virtual void _EndMatrixEdit(i_math::matrix43f *mat);

	virtual  void*_GetSelBuf()	{		return (std::vector<H3DNode>*)&_sels;	}
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual i_math::matrix43f *_GetLocalMat(H3DNode node,i_math::matrix43f &matParent);
	virtual i_math::pos2di *_GetBlock(H3DNode node)	{		return NULL;	}
	virtual BOOL _NeedBackup()	{		return FALSE;	}

	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);
	virtual void _MoveLocal(H3DNode &node,i_math::matrix43f &mat);

	std::vector<H3DNode>_sels;

	i_math::matrix43f _matTemp;

	void _UpdateCur();
	Binding _GetCurBinding();
	void _DrawCP(i_math::matrix43f &matBase,i_math::matrix43f &mat,DWORD col);
	void _DrawRadius(i_math::matrix43f &matBase,i_math::vector3df &pos,float radius);
	void _MakeHitPos(int x,int y);
	CRichGrid_MatSetItem *_GetCurItem();
	CRichGrid *_GetCurGrid();
	int _NodeHitTest(i_math::line3df &line);
	void _GetBaseMat(i_math::matrix43f &matBase);

	std::string _curGrid;
	std::string _curItem;
	DWORD _curRemoteUID;

	BOOL _bWorking;

	BOOL _bNewMat;//正在新建一个mat
	int _idxNewAfter;//新建的mat在哪个mat之后,如果为-1,在最前面,如果为0xffff,在最后面,只在_bNewMat为TRUE时有效
	BOOL _bHitPos;//_bNewMat为TRUE时有效,表示当前鼠标是否点中了一个有效位置
	i_math::vector3df _posHit;//_bNewMat为TRUE时有效,当前点中的位置
	float _hangHit;//

	IMesh *_mesh;
	IMesh *_meshRadius;
	IMtrl *_mtrl;
	IMtrl *_mtrlRadius;
	ILight *_lgt;

	BOOL _bEnableRemote;
	CRemoteItem_MatSet _itemRemote;

};

