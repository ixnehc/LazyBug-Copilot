
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"


#include "../IAssetSystem.h"
#include "../IEntitySystem.h"
#include "../ILuaMachine.h"

#include "editor/ctrlop.h"



#define AEL_GetProp(clss,prop,ael,name)											\
		clss*prop=(clss*)ael.GetProp(name);											\
		if (prop)																							\
		if (!prop->CheckClassName(#clss))												\
			prop=NULL;

#define AEL_Call(clss,prop,ael,name,param)											\
		clss*prop=(clss*)ael.Call(name,&param);										\
		if (prop)																								\
		if (!prop->CheckClassName(#clss))													\
			prop=NULL;

#define AEL_ASSERT_STUB(ael,name,prompt)											\
{																													\
	void *owner;																							\
	if (!ael.FindStub(name,owner))																\
		LuaDebugOutput("Warning",prompt,name);									\
}


//AEL 代表Asset/Entity/LuaObj
struct NodeAEL
{
	NodeAEL()
	{
		Zero();
	}
//	~NodeAEL();//注意,NodeAEL的析构函数里不调用Clear(),如果你使用NodeAEL,要手动调用它的Clear()函数
							//之所以不在析构函数里Clear(),是因为NodeAEL会存放在PropAEL这个Property里,而GProperty是不能在析构函数中
							//做清除工作的(因为stub使用GProperty作参数,并在stub内部实现时,使用了GProperty的静态成员变量,这些静态成员
							//变量只会在程序退出时被析构,而这时的清除工作可能会带来不好的结果)

	void Zero()
	{
		type=PN_Asset;
		ast=NULL;
	}
	void Clear();

	BOOL IsEmpty()	{		return (type==PN_Asset)&&(ast==NULL);	}

	void CopyFrom(NodeAEL &src);
	BOOL Equal(NodeAEL &other);

	void Set(IAsset *ast_)	{	Clear();	type=PN_Asset;		SAFE_REPLACE(ast,ast_);	}
	void Set(IEntity *entity_)	{	Clear();	type=PN_Entity;		SAFE_REPLACE(entity,entity_);	}
	void Set(ILuaObj *lo_)	{	Clear();		type=PN_LuaObj;		SAFE_REPLACE(lo,lo_);	}
	void Set(void *ptr_,ProtoNodeType type_)	
	{		
		switch(type_)
		{
			case PN_Asset:
				Set((IAsset*)ptr_);break;
			case PN_Entity:
				Set((IEntity*)ptr_);break;
			case PN_LuaObj:
				Set((ILuaObj*)ptr_);break;
			default:
				return;
		}
	}

	const char *GetDebugName();

	IAsset *GetAst()
	{
		if (type==PN_Asset)
			return ast;
		return NULL;
	}
	IAsset *GetAst(const char *name)
	{
		if (type==PN_Asset)
		{
			if (ast)
			{
				if (ast->GetClass()->CheckName(name))
					return ast;
			}
		}
		return NULL;

	}
	IEntity *GetEntity()
	{
		if (type==PN_Entity)
			return entity;
		return NULL;
	}
	ILuaObj *GetLO()
	{
		if (type==PN_LuaObj)
			return lo;
		return NULL;
	}

	BOOL SetProp(const char *name,GProperty *prop);
	GProperty *GetProp(const char *name);
	GProperty *Call(const char *name,GProperty *prop);

	BOOL IsAlive()
	{
		switch(type)
		{
			case PN_Asset:
				if(ast)
					return ast->IsAlive();
				return FALSE;
			case PN_Entity:
				return entity->IsAlive();
			case PN_LuaObj:
				return lo->IsAlive();
		}
		return FALSE;
	}

	GStubBase *FindStub(const char *name,void *&owner)
	{
		switch(type)
		{
			case PN_Asset:
				if(ast)
				{
					if (ast->IsAlive())
					{
						owner=ast->GetStubOwner();
						return ast->FindStub(name);
					}
				}
				break;
			case PN_Entity:
				if (entity->IsAlive())
					return entity->FindStub(name,owner);
				break;
			case PN_LuaObj:
				if (lo->IsAlive())
					return lo->FindStub(name,owner);
				break;
		}
		return NULL;
	}

	GStubConn*FindConn(const char *name)
	{
		switch(type)
		{
			case PN_Asset:
				if(ast)
				{
					if (ast->IsAlive())
					{
						return ast->FindConn(name);
					}
				}
				break;
			case PN_Entity:
				if (entity->IsAlive())
					return entity->FindConn(name);
				break;
			case PN_LuaObj:
				if (lo->IsAlive())
					return lo->FindConn(name);
				break;
		}
		return NULL;
	}


	BOOL DeferredDestroy()
	{
		switch(type)
		{
			case PN_Asset:
			{
				if (ast)
				{
					ast->AddRef();
					ast->DeferredDestroy();
				}
				return TRUE;
			}
		case PN_Entity:
			{
				if (entity)
				{
					entity->AddRef();
					entity->DeferredDestroy();
				}
				return TRUE;
			}
		}
		return FALSE;
	}

	CAssetCtrl *GetCtrl()
	{
		switch(type)
		{
			case PN_Asset:
			{
				if (ast)
					return ast->GetCtrl();
				break;
			}
		case PN_Entity:
			{
				if (entity)
					return entity->GetCtrl();
				break;
			}
		}

		return NULL;
	}

	ProtoNodeType type;
	union
	{
		IAsset *ast;
		IEntity *entity;
		ILuaObj *lo;
		void *ptr;
	};
};

struct PropAEL:public GProperty
{
	DEFINE_CLASS(PropAEL);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(PropAEL,1);
		GELEM_VAR(NodeAEL,v);
	END_GOBJ();    

	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_AEL;	}

	virtual GProperty *Clone();
	virtual BOOL Equals(GProperty *other);
	virtual void DeleteThis();


	NodeAEL v;

};

struct PropPN:public GProperty
{

	DEFINE_CLASS(PropPN);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(PropPN,1);
		GELEM_VAR_INIT(ProtoNodeID,id,0);//ProtoNodeID_Self
	END_GOBJ();    

	virtual GVarType GetGVT()	{		return (GVarType)GVTEx_ProtoNode;	}

	ProtoNodeID id;//ProtoNodeID_Self或者ProtoNodeID_Owner或者一个合法的ProtoNodeID
};


