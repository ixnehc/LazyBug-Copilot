/********************************************************************
	created:	2008/4/9   14:25
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	entity system interfaces
*********************************************************************/

#pragma once
#include "IEntitySystemDefines.h"
#include "IAssetRendererDefines.h"

class IAssetSystem;
class IWorldSystem;
class IAssetPackage;
class IMapFile;

class IProtoLib;
class IProto;
class IEntityParam;
class IEntity;

class IEntityMap;
class IEntityGlobal;

class IDebugger;

class ILuaBase;

class ITrrnMap;
class ITrrnBrushLib;
class IForestEditor;
class IRenderPort;
class ISpgEditor;
class IEnvLight;

class IObjMapEditor;
class INavService;

struct Envelope;

class IClient;
class INael;
class IEntitySystem
{
public:
	INTERFACE_REFCOUNT;
	virtual IEntityParam *CreateEntityParam()=0;

	virtual EntitySystemState *GetSS()=0;

	virtual IAssetSystem *GetAS()=0;
	virtual IWorldSystem *GetWS()	=0;
	virtual IMapFile *GetMapFile()=0;
	virtual IProtoLib *GetProtoLib()=0;
	virtual IEntityMap *GetMap()=0;
	virtual IEntityGlobal*GetGlobal()=0;
	virtual IDebugger *GetDebugger()=0;

	virtual void Update(EntitySystemInput &input)=0;//input里的szRP传入(0,0)表示使用原来的大小
	virtual void Render(IRenderPort *rp,AssetRendererPart part)=0;

	virtual BOOL ChangeMapFile(const char *pathMapFile,BOOL bForceChange=FALSE)=0;//pathMapFile必须是一个完整路径,如果bForceChange为TRUE,即使map file的路径没有变化,仍然会强制change
	virtual BOOL Locate(i_math::vector3df &center)=0;

	virtual void SetProgressCallBack(ESProgressCallBack cb)=0;


	virtual BOOL IsEditMode()=0;
	virtual BOOL SwitchEditMode(BOOL bEditMode)=0;//切换编辑模式,只在系统中不存在alive 的entity的情况下才会成功

	virtual BOOL AttachClient()=0;
	virtual void DetachClient()=0;

	virtual BOOL IsInProgress()=0;

	virtual BOOL SaveToMap()=0;
	virtual BOOL ReloadMap(i_math::pos2di *blkpos,DWORD nBlks)=0;
	virtual BOOL ReloadAllMap()=0;

	virtual ITrrnMap *FindTrrn()=0;
	virtual ITrrnBrushLib *FindTrrnBrushLib()=0;
	virtual IEnvLight *FindEnvLight()=0;
	virtual IObjMapEditor * FindObjMapEditor(DWORD typeEditor)=0;//typeEditor为OBJMAP_TYPE类型
	virtual INavService * FindNavService()=0;
	

	virtual BOOL ReloadProtoLib()=0;

	virtual IEntity *CreateEntity(i_math::matrix43f &mat,IEntityParam *param)=0;
	virtual IEntity *CreateEntity(i_math::matrix43f &mat,ProtoID id,BOOL bDesc=FALSE,BOOL bAllowEditHelper=FALSE)=0;

	//返回的INael指针都带一个引用计数
	virtual INael*CreateNael(const char *pathProto,i_math::matrix43f *mat=NULL)=0;
	virtual BOOL BeginCreateNael(const char *pathProto,i_math::matrix43f *mat=NULL,IEntity *owner=NULL)=0;
	virtual INael*CreateNael(ProtoID idProto,i_math::matrix43f *mat=NULL)=0;
	virtual BOOL BeginCreateNael(ProtoID idProto,i_math::matrix43f *mat=NULL,IEntity *owner=NULL)=0;
	virtual BOOL AddCreateArg(const char *nameProp,double v)=0;
	virtual BOOL AddCreateArg(const char *nameProp,const char *str)=0;
	virtual INael *EndCreateNael()=0;
	virtual INael *NaelFromEntity(IEntity *en)=0;

	//根据一个ray在entity map上选一个entity,注意这个函数不是非常快,只适用在编辑时候使用
	virtual EntityAddress HitTestOnMap(i_math::line3df &ray)=0;//hit的点保存在ray.end里
	virtual EntityAddress *VolumeHitTestOnMap(i_math::volumeCvxf &vol,DWORD &count)=0;
	virtual EntityAddress FindAssetOwnerOnMap(IAsset *ast)=0;
	virtual void CollectEnvelopeOnMap(EntityAddress addr,Envelope &evlp)=0;

	virtual void ResetConfig()=0;


};

struct NodeTreeRef;
class ILuaMachine;
class IProtoLib
{
public:
	virtual void SetPath(const char *pathLib)=0;
	virtual const char *GetPath()=0;//相对路径
	virtual const char *GetLibFolderPath()=0;//get the lib folder full path
	virtual BOOL New()=0;
	virtual BOOL Load()=0;
	virtual void SetModified()=0;

	virtual ProtoID FindProto(const char *pathProto)=0;//pathProto为在nodetree里的path
	virtual IProto *ObtainProto(const char *pathProto)=0;//pathProto为在nodetree里的path
	virtual IProto *ObtainProto(ProtoID idProto)=0;
	virtual const char *BuildIDPath(ProtoID idProto,const char *path)=0;
	virtual ProtoID FindProto2(const char *pathProto)=0;//这个函数的参数和返回结果和FindProto()一样,但使用遍历法来寻找,会更慢,但节省内存
	virtual NodeTreeRef* GetNodeTree()=0;

	virtual ILuaMachine*GetLuaMachine()=0;

	virtual const char *FindPath(ProtoID idProto)=0;//得到这个proto在nodetree里的path

	virtual IEntity **EnumEntities(DWORD &c)=0;//return a temply ptr,只枚举alive的entities
	virtual IAsset **EnumAssets(DWORD &c)=0;//return a temply ptr,只枚举alive的assets

	virtual const char *MakeProtoPath(const char *path)=0;//把一个位于protolib根目录下的相对路径,转换成绝对路径

	virtual void UnloadAllProto()=0;

	//在编辑时,有可能Proto对应的文件被覆盖了(典型的情况是GetLatestVersion()),这时,这个Proto的ID可能和
	//原来不一样了,这个函数用来修复这种情况
	virtual BOOL RepairProtoID(const char *pathProto)=0;//返回是否发生了repair


	//这个函数比较proto与硬盘上的数据有没有不同(只比较除LuaSrc以外的信息),如果有不同,把硬盘上的数据读出来返回,
	//否则返回NULL
	virtual BYTE *MakeUndoData(IProto *proto,DWORD &cData)=0;

	virtual BOOL Reload(const char *pathRoot="")=0;

	virtual const char *GetClassHelp(const char *name)=0;
};

class CDataPacket;
struct GProperty;
struct GStubBase;
class CClass;
class IProtoNode
{
public:
	virtual ProtoNodeID GetID()=0;
	virtual ProtoNodeType GetType()=0;
	virtual const char *GetPath()=0;
	virtual const char *GetName()=0;
	virtual IProto *GetOwner()=0;
	virtual void IncVer()=0;
	virtual DWORD GetVer()=0;
	virtual DWORD GetPropCount()=0;
	virtual GProperty *GetPropData(DWORD idx)=0;
	virtual const char *GetPropName(DWORD idx)=0;
	virtual GProperty *FindPropData(const char *name)=0;

	virtual DWORD GetStubCount()=0;
	virtual GStubBase *FindStub(const char *name)=0;
	//这个函数用来得到一个stub和它的exposed name.
	//注意:如果这个ProtoNode为PN_Entity的话,得到的stub的exposed name
	//和返回的GStubBase的本身的名字不同的.
	virtual GStubBase *GetStub(DWORD idx,const char *&name)=0;

	//只对PN_LuaObj有效
	virtual BOOL AddStub(StubArg &arg)=0;
	virtual BOOL ChangeStub(const char *name,StubArg &arg)=0;
	virtual BOOL RemoveStub(const char *name)=0;
	virtual BOOL MoveStub(const char *name,BOOL bUp)=0;
	virtual BOOL SetLuaSrc(const char *src)=0;
	virtual const char *GetLuaSrc()=0;


	virtual BOOL NeedMat()=0;
	virtual i_math::matrix43f &GetLocalMat()=0;
	virtual void SetLocalMat(i_math::matrix43f &)=0;

	virtual BOOL IsDynamic()=0;
	virtual void SetDynamic(BOOL bDynamic)=0;
	virtual BOOL IsVirtual()=0;
	virtual void SetVirtual(BOOL bVirtual)=0;
	virtual BOOL IsLab()=0;
	virtual void SetLab(BOOL bLab)=0;
	virtual PNDeferGrp GetDeferGrp()=0;
	virtual void SetDeferGrp(PNDeferGrp grp)=0;
	virtual BOOL IsEditHelper()=0;
	virtual void SetEditHelper(BOOL bEditHelper)=0;

	virtual CClass *GetAssetClass()=0;
	virtual ProtoID GetProtoID()=0;

	virtual void SaveForCopy(CDataPacket &dp)=0;//存储用来copy/paste的数据

	//graph data
	virtual i_math::pos2di GetGraphPos()=0;
	virtual void SetGraphPos(i_math::pos2di&pos)=0;
	virtual const char *GetGraphStubHides()	=0;
	virtual BOOL ShrinkStub(BOOL bTest)=0;
	virtual BOOL ExpandStub(BOOL bTest)=0;
	virtual BOOL StubCanHide(const char *name)=0;
	virtual BOOL HideStub(const char *name,BOOL bHide)=0;
	virtual BOOL IsStubHide(const char *name)=0;

	virtual const char *GetShowName(const char *nameOrg,BOOL bFull=TRUE)=0;
};

struct GStubBase;
struct GProperty;
class IProto
{
public:
	virtual ProtoID GetID()=0;
	virtual void IncVer()=0;
	virtual DWORD GetVer()=0;
	virtual const char *GetFilePath()=0;
	virtual void ClearModified()=0;
	virtual void SetModified()=0;
	virtual BOOL GetModified()=0;
	virtual BOOL IsLoaded()=0;
	virtual BOOL Save()=0;
	virtual BOOL Load()=0;
	virtual BOOL Reload()=0;
	virtual void Unload()=0;
	virtual BOOL Save(CDataPacket &dp,BOOL bIgnoreVersion=FALSE)=0;
	virtual BOOL Load(CDataPacket &dp)=0;//return TRUE if already loaded

	virtual BOOL ContainGlobalAsset()=0;//是否包含有全局Asset

	virtual ProtoNodeID FindNodeID(const char *pathNode)=0;
	virtual IProtoNode *GetNode(ProtoNodeID id)=0;
	virtual IProtoNode *FindNode(const char *path)=0;
	virtual ProtoNodeID GetParentNodeID(ProtoNodeID id)=0;

	virtual NodeTreeRef* GetNodeTree()=0;

	virtual GStubBase *FindStub(const char *name)=0;//注意:返回的stub没有实际功用,只能用来得到一些
																						//stub的静态信息(比如sem,desc,data class)
	virtual DWORD GetStubCount()=0;
	virtual GStubBase *GetStub(DWORD idx,const char *&name)=0;

	virtual BOOL FindStubInfo(const char *name,ProtoStubInfo &info)=0;
	virtual BOOL GetStubInfo(DWORD idx,ProtoStubInfo &info)=0;
	virtual BOOL AddStub(ProtoStubInfo &info)=0;
	virtual BOOL RemoveStub(const char *name)=0;
	virtual GProperty *FindPropertyData(const char *name)=0;
	virtual GProperty *FindRawPropertyData(const char *nameStb)=0;

	virtual DWORD GetConnectCount()=0;
	virtual BOOL GetConnect(DWORD idx,PNConnect &ret)=0;
	virtual BOOL AddConnect(PNConnect &c)=0;
	virtual BOOL RemoveConnect(PNConnect &c)=0;

	virtual void ClearLuaSrc()=0;//清除所有的lua source code(把代码置成空串)

	virtual BOOL IsLuaOnly()=0;
	virtual BOOL GetLuaTblInfo(LuaTblInfo &info)=0;

	virtual ProtoID GetRunGE()=0;
	virtual void SetRunGE(ProtoID id)=0;
	virtual ProtoID GetRunGT()=0;
	virtual void SetRunGT(ProtoID id)=0;

	virtual void SetGraphTransform(i_math::pos2di&tranlate,float scale)=0;
	virtual void GetGraphTransform(i_math::pos2di&tranlate,float &scale)=0;
	virtual void SetNextNodePos(i_math::pos2di &pos)=0;//设置下一个新建的proto node的位置,如果为(-10000,-10000),表示这个新位置要自动生成
	virtual i_math::pos2di GetNextNodePos()=0;

	virtual BOOL ReplaceProtoNode(ProtoNodeID nodeid,BYTE *data,DWORD szData)=0;

	virtual void RepairReferring()=0;//修补所有引用到自己的其它proto的引用信息,注意这个函数不会增加任何proto的版本号

	virtual const char *GetDesc(ProtoNodeID nodeid,const char *nameStb,IEntity *en)=0;//en为使用这个proto创建出来的entity
	virtual GProperty **EnumProps(DWORD &c)=0;
	virtual GProperty **EnumAssetProps(DWORD &c,const char *nmAstClass)=0;
	virtual ProtoID *EnumReference(DWORD &c)=0;

};

class CDataPacket;
class IEntityParam
{
public:
	INTERFACE_REFCOUNT;

	virtual void SetProtoID(ProtoID protoid)=0;
	virtual ProtoID GetProtoID()=0;

	virtual BOOL IsEmpty()=0;
	virtual BOOL IsMissed()=0;
	virtual BOOL Save(CDataPacket &dp)=0;
	virtual BOOL Load(CDataPacket &dp,IProtoLib *lib,BOOL &bRepaired)=0;

	virtual BOOL Repair(IProtoLib *lib)=0;//返回是否有修补
	virtual BOOL AddProp(const char *name,GProperty *prop)=0;
	virtual GProperty *FindProp(const char *name)=0;
	virtual void ClearProp()=0;

};

class CAssetCtrl;
struct Envelope;
class INael;
struct GStubConn;
class IClientEntity;
class IEntity
{
public:
	INTERFACE_REFCOUNT;
	virtual ProtoID GetProtoID()=0;
	virtual IProto *GetProto()=0;
	//返回生成这个entity的ProtoNode(如果这个entity是包含在其它entity里面的话)
	virtual IProtoNode *GetOwnerProtoNode()=0;
	//寻找生成这个asset/entity/luaobj的proto node的id,如果这个asset/entity/luaobj是属于一个嵌套在本entity中的entity,
	//则返回那个entity在本entity中的proto node id
	virtual ProtoNodeID FindProtoNodeID(void *ael)=0;

	virtual void Destroy()=0;//注意这个函数会释放引用计数
	virtual void DeferredDestroy()=0;//注意这个函数会释放引用计数

	virtual BOOL IsAlive()=0;
	virtual void SetBit(EntityBit bit)=0;
	virtual void ClearBit(EntityBit bit)=0;
	virtual void ClearAllBits()=0;
	virtual BOOL TestBit(EntityBit bit)=0;

	virtual IAsset *GetLastAsset()=0;
	virtual CAssetCtrl*GetCtrl()=0;//得到这个entity里最顶级的那个Asset Control

	virtual BOOL AssignParam(IEntityParam *param)=0;//将param的数据内容作用到entity里
	virtual BOOL SaveAssetData(IEntityParam *param)=0;//将entity中asset的数据存入entity param 中,

	virtual DWORD GetStubCount()=0;
	virtual GStubBase *GetStub(DWORD idx,void *&owner)=0;
	virtual GStubBase *FindStub(const char *name,void *&owner)=0;
	virtual GStubConn *FindConn(const char *name)=0;

	virtual BOOL OwnAsset(IAsset *ast,BOOL bRecursiveFind=TRUE)=0;//检查是否拥有这个asset
	virtual DWORD FindAsset(const char *nameAstClass,IAsset**buf,DWORD szBuf)=0;

	virtual DWORD EnumAllAsset(IAsset**buf,DWORD szBuf)=0;


	virtual BOOL OwnEntity(IEntity *en,BOOL bRecursiveFind=TRUE)=0;//检查是否拥有这个entity,如果en就是this,返回FALSE

	//返回与ray相交的那个asset的protonode id
	//如果与ray相交的那个asset属于这个entity的子entity,则返回这个子entity的protonode id
	virtual ProtoNodeID ProtoNodeHitTest(i_math::line3df &ray)=0;

	virtual void CollectEnvelope(ProtoNodeID id,Envelope &evlp)=0;

	virtual IAsset *GetAsset(ProtoNodeID id)=0;//如果这个id没有对应的Asset,返回NULL
	virtual IEntity *GetEntity(ProtoNodeID id)=0;//如果这个id没有对应的Entity,返回NULL

	virtual INael *ObtainNael(const char *nm,BOOL bSilent=FALSE)=0;//nm是ProtoNode的名称(注意,不是路径),返回的指针会带一个引用计数
	virtual IClientEntity *GetClient()=0;
};


class IEntityMap
{
public:
	virtual EntityAddress Reside(i_math::matrix43f &mat,IEntityParam *param)=0;//mat是输入参数
	virtual IEntityParam *UnReside(i_math::matrix43f &mat,EntityAddress address)=0;//mat是输出参数
	virtual EntityAddress Clone(EntityAddress addr,i_math::vector3df &posOff)=0;
	virtual IEntity *ToEntity(EntityAddress address)=0;
	virtual IEntityParam *ToEntityParam(EntityAddress address)=0;
	virtual i_math::matrix43f *GetEntityMat(EntityAddress address)=0;//如果是一个无效的address,返回NULL
	virtual BOOL ResolveBlockPos(EntityAddress address,i_math::pos2di &ptBlk)=0;
	virtual EntityAddress FindAssetOwner(IAsset *ast)=0;
	virtual i_math::recti GetMapRect()=0;

	virtual DWORD EnumEntities(float x,float z,float range,IEntity **buf,DWORD szBuf)=0;//x,z为中心点,range为半径,都以米为单位,世界空间的值
	virtual DWORD EnumEntities(float x,float z,float range,EntityAddress*buf,DWORD szBuf)=0;//x,z为中心点,range为半径,都以米为单位,世界空间的值

	virtual void SetBlockModified(i_math::pos2di &ptBlk)=0;//ptBlk is of world coord
	virtual void SetModified(i_math::recti &rcBlk)=0;//将rcBlk范围内的block全部置为modified的
	virtual BOOL SaveModified()=0;

	virtual BOOL ReloadAll()=0;
	virtual BOOL UnloadAll()=0;

};

class IEntityGlobal
{
public:
	virtual ProtoID GetGE()=0;
	virtual ProtoID GetGT()=0;
	virtual BOOL SetGE(ProtoID protoid)=0;
	virtual BOOL SetGT(ProtoID protoid)=0;
	virtual IEntity *GetEntity()=0;//ge对应的entity对象
	virtual void ClearDynEntities()=0;//清除所有动态创建在global里的entity(一般是在脚本里通过调用CreateGlobal()创建的)
};

struct GStubConn;
struct GStubBase;
class ILuaObj
{
public:
	INTERFACE_REFCOUNT;
	virtual BOOL IsAlive()=0;

	virtual DWORD GetStubCount()=0;
	virtual const char *GetStubName(DWORD idx)=0;
	virtual GStubConn *FindConn(const char *name)=0;

	virtual void *GetStubOwner()=0;
	virtual GStubBase *GetStub(DWORD idx,void *&owner)=0;
	virtual GStubBase *FindStub(const char *name,void *&owner)=0;

};


