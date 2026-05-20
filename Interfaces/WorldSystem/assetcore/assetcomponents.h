/********************************************************************
	created:	2007/7/4   15:22
	filename: 	e:\IxEngine\Proj_WorldSystem\assetcomponents.h
	author:		cxi
	
	purpose:	all the component macro for the asset
*********************************************************************/
#pragma once

#include "log/LogDump.h"



class CAsset;

//////////////////////////////////////////////////////////////////////////
//XForm
#define DECLARE_POSITION																							\
virtual BOOL GetPos(i_math::vector3df &v)																	\
{																																		\
	v=_pos;																														\
	return TRUE;																												\
}																																		\
virtual BOOL GetXForm(i_math::matrix43f &v)															\
{																																		\
	v.makeIdentity();																											\
	v.addTranslation(_pos);																								\
	return TRUE;																												\
}																																		\
virtual BOOL SetPos(i_math::vector3df &v)																	\
{																																		\
	_pos=v;																														\
	return TRUE;																												\
}																																		\
virtual BOOL SetXForm(i_math::matrix43f &mat,i_math::matrix43f*matLocal)			\
{																																		\
	_pos=*mat.getTranslationP();																					\
	return TRUE;																												\
}																																		\
BOOL prop_Xform(BOOL bSet,i_math::matrix43f&mat)												\
{																																		\
	if (bSet)																														\
		return SetXForm(mat,i_math::matrix43f::identity());											\
	else																																\
		return GetXForm(mat);																							\
}																																		\
i_math::vector3df _pos;


#define DECLARE_MATRIX																							\
virtual BOOL GetPos(i_math::vector3df &v)																	\
{																																		\
	v=*_mat.getTranslationP();																						\
	return TRUE;																												\
}																																		\
virtual BOOL GetXForm(i_math::matrix43f &mat)															\
{																																		\
	mat=_mat;																														\
	return TRUE;																												\
}																																		\
virtual BOOL SetPos(i_math::vector3df &v)																	\
{																																		\
	*(_mat.getTranslationP())=v;																						\
	return TRUE;																												\
}																																		\
virtual BOOL SetXForm(i_math::matrix43f&mat,i_math::matrix43f*matLocal)																\
{																																		\
	_mat=mat;																														\
	return TRUE;																												\
}																																		\
BOOL prop_Xform(BOOL bSet,i_math::matrix43f&mat)												\
{																																		\
	if (bSet)																														\
		return SetXForm(mat,i_math::matrix43f::identity());											\
	else																																\
		return GetXForm(mat);																							\
}																																		\
i_math::matrix43f _mat;

#define DECLARE_LOCAL_MATRIX(__matLocal)															\
virtual BOOL GetPos(i_math::vector3df &v)																	\
{																																		\
	return FALSE;																												\
}																																		\
virtual BOOL GetXForm(i_math::matrix43f &mat)														\
{																																		\
	return FALSE;																												\
}																																		\
virtual BOOL SetPos(i_math::vector3df &v)																	\
{																																		\
	return FALSE;																												\
}																																		\
virtual BOOL SetXForm(i_math::matrix43f&mat,i_math::matrix43f*matLocal)			\
{																																		\
	__matLocal=matLocal;																								\
	return TRUE;																												\
}																																		\
BOOL prop_Xform(BOOL bSet,i_math::matrix43f&mat)												\
{																																		\
	if (bSet)																														\
		return SetXForm(mat,i_math::matrix43f::identity());											\
	else																																\
		return GetXForm(mat);																							\
}																																		\
i_math::matrix43f *__matLocal;


#define DECLARE_MATRIX_PTR																					\
virtual BOOL SetPos(i_math::vector3df &v)																	\
{																																		\
	*(_mat.Obtain()->getTranslationP())=v;																	\
	return TRUE;																												\
}																																		\
virtual BOOL SetXForm(i_math::matrix43f&v,i_math::matrix43f*matLocal)					\
{																																		\
	*_mat.Obtain()=v;																										\
	return TRUE;																												\
}																																		\
BOOL prop_Xform(BOOL bSet,i_math::matrix43f&mat)												\
{																																		\
	if (bSet)																														\
		return SetXForm(mat,i_math::matrix43f::identity());											\
	else																																\
		return GetXForm(mat);																							\
}																																		\
GPropertyPtr<Prop_Fx12,i_math::matrix43f> _mat;


//注意,使用时别忘了在构造函数里为__site赋初值
#define DECLARE_SITE(__site)																						\
virtual BOOL GetPos(i_math::vector3df &v)																	\
{																																		\
	assert(__site);																												\
	i_math::matrix43f *mat=__site->GetMat(_ss->t);													\
	if (!mat)																														\
		return FALSE;																											\
	v=*mat->getTranslationP();																						\
	return TRUE;																												\
}																																		\
virtual BOOL GetXForm(i_math::matrix43f &v)															\
{																																		\
	assert(__site);																												\
	i_math::matrix43f *mat=__site->GetMat(_ss->t);													\
	if (!mat)																														\
		return FALSE;																											\
	v=*mat;																														\
	return TRUE;																												\
}																																		\
virtual BOOL SetPos(i_math::vector3df &v)																	\
{																																		\
	i_math::matrix43f t;																									\
	*(t.getTranslationP())=v;																							\
	return SetXForm(t,i_math::matrix43f::identity());														\
}																																		\
virtual BOOL SetXForm(i_math::matrix43f&mat,i_math::matrix43f*matLocal)			\
{																																		\
	assert(__site);																												\
	if (!__site->GetBase())																								\
		__site->SetFixed(mat);																								\
	__site->SetLocalMat(matLocal);																				\
	return TRUE;																												\
}																																		\
BOOL prop_Xform(BOOL bSet,i_math::matrix43f&mat)												\
{																																		\
	if (bSet)																														\
		return SetXForm(mat,i_math::matrix43f::identity());											\
	else																																\
		return GetXForm(mat);																							\
}																																		\
IAnimNodeSite *__site;




#define GPropXform()	GPropMat43(Xform,matrix43f(),GSem_World,"")
#define GStubXform()	GStubMat43(Xform,matrix43f(),GSem_World,"")



#define APPLY_MATOFFSET(an,matOff)																						\
if (matOff)																																		\
{																																						\
	IAnimNodeMatOffset *anOff=_ss->ans->CreateMatOffset();												\
	anOff->SetOffset(*matOff);																										\
	anOff->SetBase(an);																													\
	SAFE_RELEASE(an);																														\
	an=anOff;																																	\
}






//////////////////////////////////////////////////////////////////////////
//Tree Linkage
#define DECLARE_TREE_LINK()																								\
public:																																		\
virtual BOOL SupportTreeLink(){	return TRUE;}																	\
virtual IAsset *GetParent(){	return _parent;}																		\
virtual DWORD GetChildCount(){	return _childs.size();}														\
virtual IAsset *GetChild(DWORD idx)																					\
{																																				\
	if (idx>=_childs.size())																										\
		return NULL;																													\
	return _childs[idx];																												\
}																																				\
virtual IAsset **GetChilds(DWORD &n)																				\
{																																				\
	n=_childs.size();																													\
	if (n>0)																																\
		return _childs.data();																											\
	return NULL;																														\
}																																				\
virtual BOOL SetParent(IAsset *ast)																						\
{																																				\
	_parent=ast;																														\
	return TRUE;																														\
}																																				\
virtual BOOL AddChild(IAsset *ast)																						\
{																																				\
	ast->SetParent(this);																											\
	_childs.push_back(ast);																										\
	return TRUE;																														\
}																																				\
virtual BOOL RemoveChild(IAsset *ast)																				\
{																																				\
	int idx;																																	\
	VEC_FIND(_childs,ast,idx);																									\
	if (idx==-1)																															\
		return FALSE;																													\
	_childs.erase(_childs.begin()+idx);																					\
	ast->ClearChilds();																												\
	ast->SetParent(NULL);																										\
	return TRUE;																														\
}																																				\
virtual BOOL ClearChilds()																										\
{																																				\
	for (int i=0;i<_childs.size();i++)																							\
	{																																			\
		if (!_childs[i])																													\
			continue;																														\
		_childs[i]->ClearChilds();																								\
		_childs[i]->SetParent(NULL);																							\
	}																																			\
	_childs.clear();																														\
	return TRUE;																														\
}																																				\
virtual BOOL CheckDescendent(IAsset *ast)																		\
{																																				\
	IAsset *p=ast;																														\
	while(p)																																\
	{																																			\
		if (p==this)																														\
			return (p!=ast);																											\
		p=p->GetParent();																											\
	}																																			\
	return FALSE;																														\
}																																				\
protected:																																\
IAsset *_parent;																														\
std::vector<IAsset *>_childs;


//////////////////////////////////////////////////////////////////////////
//Control
#define DECLARE_CONTROL(classname)																				\
public:																																		\
	virtual CAssetCtrl *GetCtrl()	{		return _ctrl;	}																\
	virtual void DisableStub()																									\
	{																																			\
		if (_ctrl)																															\
			_ctrl->DisableStub();																									\
	}																																			\
	virtual GStubBase *FindStub(const char *name)																\
	{																																			\
		if (_ctrl)																															\
			return _ctrl->FindStub(name);																					\
		else																																	\
		{																																		\
			CAssetCtrl*ctrl=(CAssetCtrl*)CClass::New(#classname);										\
			if (!ctrl)																														\
				return NULL;																											\
			GStubBase *ret=ctrl->FindStub(name);																	\
			Class_Delete(ctrl);																										\
			return ret;																													\
		}																																		\
	}																																			\
	virtual DWORD GetStubCount()																						\
	{																																			\
		if (_ctrl)																															\
			return _ctrl->GetStubCount();																					\
		else																																	\
		{																																		\
			CAssetCtrl*ctrl=(CAssetCtrl*)CClass::New(#classname);										\
			if (!ctrl)																														\
				return 0;																													\
			DWORD ret=ctrl->GetStubCount();																			\
			Class_Delete(ctrl);																										\
			return ret;																													\
		}																																		\
	}																																			\
	virtual GStubBase *GetStub(DWORD idx)																		\
	{																																			\
		if (_ctrl)																															\
			return _ctrl->GetStub(idx);																						\
		else																																	\
		{																																		\
			CAssetCtrl*ctrl=(CAssetCtrl*)CClass::New(#classname);										\
			if (!ctrl)																														\
				return NULL;																											\
			GStubBase *ret=ctrl->GetStub(idx);																			\
			Class_Delete(ctrl);																										\
			return ret;																													\
		}																																		\
	}																																			\
	virtual void *GetStubOwner()	{		return _ctrl;	}															\
	virtual GStubConn *FindConn(const char *name)															\
	{																																			\
		if (_ctrl)																															\
			return _ctrl->FindConn(name);																					\
		return NULL;																													\
	}																																			\
protected:																																\
	virtual BOOL _CreateCtrl(IAsset*parent)																			\
	{																																			\
		_ctrl=(CAssetCtrl*)CClass::New(#classname);																\
		if (_ctrl)																															\
		{																																		\
			if (_ctrl->Create(_ss,this))																							\
			{																																	\
				if (parent)																												\
					_ctrl->SetParent(parent->GetCtrl());																\
				return TRUE;																											\
			}																																	\
			Class_Delete(_ctrl);																										\
			_ctrl=NULL;																													\
		}																																		\
		return FALSE;																													\
	}																																			\
	virtual void _DestroyCtrl()																									\
	{																																			\
		if (_ctrl)																															\
		{																																		\
			CAssetCtrl *ctrl=_ctrl;																								\
			_ctrl->Destroy();				/*这个函数里会把_ctrl置为空*/											\
			Class_Delete(ctrl);																										\
		}																																		\
	}																																			\
	virtual void _ClearCtrl()																										\
	{																																			\
		_ctrl=NULL;																														\
	}																																			\
	virtual void OnPostCreate()																								\
	{																																			\
		if (_ctrl)																															\
			_ctrl->OnPostCreate();																								\
	}																																			\
	CAssetCtrl *_ctrl;

#define DECLARE_CLOCKED_CONTROL(classname)															\
DECLARE_CONTROL(classname);																							\
virtual void OnClock()																											\
{																																				\
	if (_ctrl)																																\
		_ctrl->OnClock();																											\
}

#ifdef _DEBUG


#define CHECK_ASSET_FIRE_TYPE(name,data)																			\
{																																					\
	GStubConn *conn=FindConn(#name);																					\
	if (conn)																																	\
	{																																				\
		if (!conn->GetDataClass()->CheckName((data)->GetClass()->GetName()))				\
			LOG_DUMP_3P("Asset",Log_Error,																				\
					"%s 中发送的Signal数据类型(\"%s\")与Stub数据类型(\"%s\")不符!",						\
								GetClass()->GetName(),																			\
								(data)->GetClass()->GetName(),																\
								conn->GetDataClass()->GetName());														\
	}																																				\
}

#else

#define CHECK_ASSET_FIRE_TYPE(name,data)

#endif





#define AstStubTrigger(name)																								\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	GStubTrigger(name);																											\
	GStackPop();																														\
}

#define AstStubFire(name,data)																							\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&data)																			\
	GStubFire(name,data);																										\
	GStackPop();																														\
}


#define  AstStubFireVoid(name)																							\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Void())																\
	GStubFireVoid(name);																										\
	GStackPop();																														\
}

#define AstStubFireString(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_String())																\
	GStubFireString(name,value);																							\
	GStackPop();																														\
}

#define AstStubFireInt(name,value)																						\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_S())																		\
	GStubFireInt(name,value);																									\
	GStackPop();																														\
}

#define AstStubFireDword(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_U())																	\
	GStubFireDword(name,value);																							\
	GStackPop();																														\
}

#define AstStubFireFloat(name,value)																					\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_F())																		\
	GStubFireFloat(name,value);																								\
	GStackPop();																														\
}

#define AstStubFireVector2(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Fx2())																	\
	GStubFireVector2(name,value);																						\
	GStackPop();																														\
}

#define AstStubFireVector3(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Fx3())																	\
	GStubFireVector3(name,value);																						\
	GStackPop();																														\
}

#define AstStubFireVector4(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Fx4())																	\
	GStubFireVector4(name,value);																						\
	GStackPop();																														\
}


#define AstStubFireMat43(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Fx12())																\
	GStubFireMat43(name,value);																							\
	GStackPop();																														\
}

#define AstStubFireMat44(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Fx16())																\
	GStubFireMat44(name,value);																							\
	GStackPop();																														\
}

#define AstStubFireInt4(name,value)																					\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Sx4())																	\
	GStubFireInt4(name,value);																								\
	GStackPop();																														\
}

#define AstStubFireInt2(name,value)																					\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Sx2())																	\
	GStubFireInt2(name,value);																								\
	GStackPop();																														\
}

#define AstStubFireByte4(name,value)																				\
{																																				\
	GStackPush_General(#name,this,GetClass());																	\
	CHECK_ASSET_FIRE_TYPE(name,&Prop_Bx4())																	\
	GStubFireByte4(name,value);																							\
	GStackPop();																														\
}


#define AstStubFireLink(name,an__)																						\
{																																				\
	Prop_AnimNode t;																												\
	t.an=an__;																															\
	AstStubFire(name,t);																											\
}