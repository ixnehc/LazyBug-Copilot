#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "FileSystem/IFileSystem.h"

#include "RenderSystem/ITools.h"

#include "WorldSystem/IWorldSystem.h"

#include "WorldSystem/IEntitySystem.h"
#include "RenderSystem/IRenderSystem.h"

#include <unordered_map>

#define DEFINE_GUIDATA_PROTO(v)															\
	GuiData_Proto *v=(GuiData_Proto*)FindData("proto");	




class ICamera;
struct GuiLib_Api GuiData_Proto:public GeData
{
	virtual const char *GetName()	{		return "proto";	}

	struct NodeMat
	{
		i_math::matrix43f base;
		i_math::matrix43f local;
	};

	GuiData_Proto()
	{
		Zero();
	}

	void Zero()
	{
		protoid=ProtoID_Null;

		entityView=NULL;

		cam=NULL;

		bChanging=FALSE;
		bShowProfile=FALSE;

		idGE=ProtoID_Null;
		idGT=ProtoID_Null;

		tView=0;

		scale=1.0f;
	}

	void Clear()
	{
		SAFE_RELEASE(cam);
		Zero();
	}

	IProto *proto()
	{
		if (!lib)
			return NULL;
		return lib->ObtainProto(protoid);
	}

	ProtoNodeID GetSelNodeID()
	{
		if (sels.size()==1&&proto())
			return sels[0];
		return ProtoNodeID_Null;
	}

	FileAttr GetFileAttr()
	{
		IProto *p=proto();
		if (!p)
			return File_Miss;
		const char *path=lib->MakeProtoPath(p->GetFilePath());
		FileAttr attr=pES->GetWS()->GetFS()->GetFileAttrAbs(path);
		return attr;
	}

	//检查proto对应的文件是否可写
	BOOL IsReadOnly()		{			return (GetFileAttr()==File_ReadOnly);	}
	//检查proto对应的文件是否存在
	BOOL IsExisting()	{		return (GetFileAttr()!=File_Miss);	}

	BOOL GetNodeMat(ProtoNodeID idPN,i_math::matrix43f &mat)
	{
		IProto *prt=proto();
		if (!prt)
			return FALSE;

		std::unordered_map<ProtoNodeID,NodeMat>::iterator it=nodemats.find(idPN);
		if (it!=nodemats.end())
		{
			NodeMat *p=&(*it).second;
			mat=p->local*p->base;
			return TRUE;
		}

		return FALSE;
	}

	BOOL SetNodeMat(ProtoNodeID idPN,i_math::matrix43f &mat)
	{
		IProto *prt=proto();
		if (!prt)
			return FALSE;

		IProtoNode *node=prt->GetNode(idPN);
		if (!node)
			return FALSE;


		std::unordered_map<ProtoNodeID,NodeMat>::iterator it=nodemats.find(idPN);
		if (it!=nodemats.end())
		{
			NodeMat *p=&(*it).second;
			i_math::matrix43f baseInv=p->base;
			baseInv.makeInverse();
			node->SetLocalMat(mat*baseInv);
			return TRUE;
		}

		return FALSE;
	}

	BOOL GetNodeLocalMat(ProtoNodeID idPN,i_math::matrix43f &mat,i_math::matrix43f &matParent)
	{
		IProto *prt=proto();
		if (!prt)
			return FALSE;

		std::unordered_map<ProtoNodeID,NodeMat>::iterator it=nodemats.find(idPN);
		if (it!=nodemats.end())
		{
			NodeMat *p=&(*it).second;
			mat=p->local;
			matParent=p->base;
			return TRUE;
		}

		return FALSE;
	}

	BOOL SetNodeLocalMat(ProtoNodeID idPN,i_math::matrix43f &mat)
	{
		IProto *prt=proto();
		if (!prt)
			return FALSE;

		IProtoNode *node=prt->GetNode(idPN);
		if (!node)
			return FALSE;


		std::unordered_map<ProtoNodeID,NodeMat>::iterator it=nodemats.find(idPN);
		if (it!=nodemats.end())
		{
			node->SetLocalMat(mat);
			return TRUE;
		}

		return FALSE;
	}


	
	IEntitySystem *pES;
	IProtoLib *lib;
	std::string pathProtoFile;
	ProtoID protoid;


	std::vector<ProtoNodeID> sels;

	BOOL bChanging;//指示是否在修改当前的proto

	BOOL bShowProfile;

	ICamera *cam;

	IEntity *entityView;//这个entity是为了显示而临时创建的,只用于显示(也只在显示过程中不为NULL)

	float tView;//当根据proto创建出用来显示的entity后,我们可以更新一段逻辑时间,再进行绘制,这样我们可以看到这个proto在运行中的画面

	//graph transform
	i_math::pos2di xlate;
	float scale;

	std::unordered_map<ProtoNodeID,NodeMat>nodemats;//这个hash map中记录了所有node的local矩阵以及它们的base矩阵

	//测试相关
	ProtoID idGE;
	ProtoID idGT;


};


