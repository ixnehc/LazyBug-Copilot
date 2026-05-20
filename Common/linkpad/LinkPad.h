#pragma once

#include "strlib/strlib.h"
#include <unordered_map>
#include <unordered_set>

#include "LinkPadDefines.h"

struct PadStub
{
	PadStub()
	{
		name="";
		sem="";
		bSingleLink=0;
		type=PadStub_In;
	}
	PadStub(const char *name_,PadStubType type_,BOOL bSingleLink_,const char *sem_="")
	{
		name=name_;
		sem=sem_;
		type=type_;
		bSingleLink=bSingleLink_;
	}
	const char *name;//注意,这个name的长度不要超过MAX_PADSTUB_NAME-1
	const char *sem;//语意,语意相同的stub之间才能连接
	PadStubType type;
	BYTE bSingleLink:1;
};

struct GObjBase;
class CClass;

class CLinkPad
{
public:
	CLinkPad()
	{
		_name=StringID_Invalid;
		_id=PadID_Null;
		_idFolder=PadID_Null;
		_bFolder=0;
	}
	~CLinkPad()
	{
		Clear();
	}

	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;
	virtual const char *GetTypeName()=0;
	virtual DWORD GetStubCount()=0;
	virtual PadStub GetStub(DWORD idx)=0;//注意返回的stub的名称长度不要超过MAX_PADSTUB_NAME
	virtual const char *GetShowName()	{		return "";	}
	virtual const char *GetFolderShowName()	{		return GetFolderName();	}

	void Clear();

	PadID GetID()	{		return _id;	}
	StringID GetName()	{		return _name;	}
	void SetName(StringID name)	{		_name=name;	}
	i_math::pos2di GetPos()	{		return i_math::pos2di(_pt.x,_pt.y);	};
	void SetPos(i_math::pos2di &pt)	{		_pt.set((short)pt.x,(short)pt.y);	};
	PadID GetFolder()	{		return _idFolder;	}
	void SetFolder(PadID idFolder)	{		_idFolder=idFolder;	}
	i_math::pos2di GetFolderPos()	{		return i_math::pos2di(_ptFolder.x,_ptFolder.y);	};
	void SetFolderPos(i_math::pos2di &pt)	{		_ptFolder.set((short)pt.x,(short)pt.y);	};
	const char *GetFolderName()	{		return _nameFolder.c_str();	}
	void SetFolderName(const char *name)	{		_nameFolder=name;	}
	BOOL IsFolder()	{		return _bFolder!=0;	}


public://Take it as protected
	StringID _name;
	PadID _id;//这个id用来在所属的CLinkPads里唯一标识自己
	i_math::pos2d_sh _pt;

	PadID _idFolder;//属于哪一个folder
	DWORD _bFolder;//自己是不是一个folder
	i_math::pos2d_sh _ptFolder;//如果自己是一个folder,_ptFolder记录了自己在自己这个folder中的位置
	std::string _nameFolder;

	void _BuildIdxCache();
	void _ClearIdxCache();
	std::unordered_map<std::string,short>_idxmap;

	friend class CLinkPads;
	friend class CLinkPad;
};


class CDataPacket;
class CLinkPads
{
public:
	CLinkPads()
	{
		_idSeed=PadID_Null+1;
		_clsses=NULL;
	}
	~CLinkPads()
	{
		Clear();
	}

	struct Link
	{
		short iPad[2];
		short iStub[2];
		BOOL operator==(Link &other)
		{
			return memcmp(this,&other,sizeof(other))==0;
		}
	};

	struct LinkPersist_Short//用于保存的Link
	{
		PadID_Short idPad[2];
		char nameStub[2][MAX_PADSTUB_NAME];
	};


	struct LinkPersist//用于保存的Link
	{
		void From(LinkPersist_Short &link)
		{
			idPad[0]=(PadID)link.idPad[0];
			idPad[1]=(PadID)link.idPad[1];
			memcpy(nameStub,link.nameStub,sizeof(nameStub));
		}
		BOOL Equals(LinkPersist &other)
		{
			if ((idPad[0]==other.idPad[0])&&
				(idPad[1]==other.idPad[1])&&
				(strcmp(nameStub[0],other.nameStub[0])==0)&&
				(strcmp(nameStub[1],other.nameStub[1])==0))
				return TRUE;
			if ((idPad[0]==other.idPad[1])&&
				(idPad[1]==other.idPad[0])&&
				(strcmp(nameStub[0],other.nameStub[1])==0)&&
				(strcmp(nameStub[1],other.nameStub[0])==0))
				return TRUE;
			return FALSE;
		}
		PadID idPad[2];
		char nameStub[2][MAX_PADSTUB_NAME];
	};

	struct FolderXfm_Short
	{
		PadID_Short id;
		i_math::pos2df off;
		i_math::pos2df scale;
	};

	struct FolderXfm
	{
		void From(FolderXfm_Short &xfm)
		{
			id=(PadID)xfm.id;
			off=xfm.off;
			scale=xfm.scale;
		}
		PadID id;
		i_math::pos2df off;
		i_math::pos2df scale;
	};

	virtual void Clear();

	void SetClasses(LinkPadClasses *clsses)	{		_clsses=clsses;	}
	LinkPadClasses *GetClasses()	{		return _clsses;	}

	void RefreshPadIDs();//Re-Generate all the id for each pad

	virtual PadID GenPadID();

	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp,BOOL &bLongPadID);

	void RemovePads(PadID *ids,DWORD count,BOOL bRemoveSubs=TRUE);
	CLinkPad *FindPad(PadID id);
	int FindPadIdx(PadID id);
	int FindStubIdx(int iPad,const char *stub);
	void RemoveLink(PadID id,const char *stub);
	BOOL AddLink(PadID idSrc,const char *stubSrc,PadID idTarget,const char *stubTarget);//注意:source 必须是out的stub,target必须是in的stub
	PadID NewPad(const char *clssname,i_math::pos2di &pt);

	DWORD GetPadCount()	{		return _pads.size();	}
	CLinkPad *GetPad(int idxPad);

	CLinkPad **GetPads(DWORD &n);
	Link *GetLinks(DWORD &n);
	LinkPersist*GetPersistLinks(DWORD &n);

	void PreModify(PadID id);
	void PostModify()	{		_FromPersist();	}

	BOOL CanFold(PadID id);//返回某个pad是否可以成为一个folder
	BOOL Fold(PadID id);
	BOOL UnFold(PadID id);
	BOOL IsFolder(PadID id);
	void PushFolder(PadID id);
	void PopFolder();
	void PopToFolder(PadID id);
	PadID GetCurFolder();
	PadID *GetInFolders(PadID idFolder,DWORD &count);//返回临时指针
	PadID *GetFolderStack(DWORD &count)	{		count=_stackFolder.size();		return _stackFolder.data();	}
	void ClearFolderStack()	{		_stackFolder.clear();	}
	BOOL GetFolderXfm(PadID id,i_math::pos2df &off,i_math::pos2df &scale);
	BOOL SetFolderXfm(PadID id,i_math::pos2df &off,i_math::pos2df &scale);
	void ClearFolderXfm();
	PadID *GetFolderSubs(PadID id,DWORD &count);
	BOOL IsInFolder(PadID idPad,PadID idFolder);
	int GetInFolderDepth(PadID idPad,PadID idFolder);//如果idPad等于idFolder,返回0,如果idPad直接在idFolder内,返回1,如果不在folder内,返回-1
	void ValidateFolders()	{		_ValidateFolders();	}

public://Take it as protected
	void _ToPersist();
	void _FromPersist();
protected:
	//这个函数的存在意义:
	//因为LinkPads在保存的时候,会将每个pad的class name保存下来,在load的时候,会根据这个name
	//来创建对应的pad对象,这就要求编译器把pad的CClass的全局对象link到可执行文件中,由于
	//只有当链接common库的dll/exe显式的引用到这些CClass时,编译器才会把它们链接进来,所以我们必须
	//写一点代码显式的引用到这些类._CalcClassCode()就是做这件事的,子类可以重载这个函数,用来显式的
	//引用那些需要被链接的pad的类
	virtual BYTE _CalcClassCode()	{		return 0;	}
	virtual BOOL _FoldLinkSrc()	{		return TRUE;	}


	PadID _NewPad(CLinkPad *pad,i_math::pos2di &pt);
	PadID _ReadShortPadID(CDataPacket &dp);

	LinkPadClasses *_clsses;

	std::vector<CLinkPad *>_pads;
	std::vector<Link> _links;

	void _BuildIdxCache();
	void _ClearIdxCache();
	std::unordered_map<PadID,short>_idxmap;

	static std::vector<LinkPersist>_links2;

	BOOL _GetLinkSubs(PadID id,std::unordered_set<int>&subs,std::unordered_set<int> *links);
	BOOL _GetFolderSubs(PadID id,std::vector<PadID> &subs);
	BOOL _IsFolderSub(PadID id,PadID idSub);

	void _ValidateFolders();//作Folder相关的Validation
	std::vector<PadID>_stackFolder;

	std::vector<FolderXfm> _xfmsFolder;//各个folder的xform

	PadID _idSeed;

	std::vector<PadID>_temp;//临时buffer
};