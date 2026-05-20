/********************************************************************
	created:	2011/7/29   15:13
	file path:	e:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	得到GObj 内包含的路径
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include "GObjRefPath.h"
#include "WorldSystem/IAssetShell.h"
#include "stringparser/stringparser.h"
#include "WorldSystem/IWorldSystem.h"

#include "gds/GDefines.h"
#include "gds/GObj.h"


void GetVarRefPath(void *var,GVarType gvt,GSem &sem,std::vector<std::string>*bufRes,
				   std::vector<std::string>*bufTrrnBrLib,std::vector<std::string>*bufBrLib,std::vector<std::string>*bufMapFile,std::vector<unsigned __int64>*bufProto)
{
	if (gvt==GVT_Bx8)
	{
		unsigned __int64 id=*(unsigned __int64*)var;
		if (bufProto)
		{
			UNIQUE_VEC_ADD(*bufProto,id);
		}
	}
	if (gvt==GVT_String)
	{
		std::string &s=*(std::string*)var;
		std::vector<std::string>*buf=NULL;
		std::string path;
		switch(sem.code)
		{
			case GSem_TexturePartPath:
			{
				i_math::rect_sh rc;
				ParseShellImageStr(s,path,rc);
				buf=bufRes;
				break;
			}
			case GSem_TexturePath:
			case GSem_MeshPath:
			case GSem_MtrlPath:
			case GSem_BoneAnimPath:
			case GSem_XformAnimPath:
			case GSem_MtrlColorAnimPath:
			case GSem_UvAnimPath:
			case GSem_DummiesPath:
			case GSem_SptPath:
			case GSem_MoppPath:
			case GSem_SpgPath:
			case GSem_AnimTreePath:
			case GSem_BoneAnim2Path:
			case GSem_MtrlExtPath:
			case GSem_SoundPath:
			case GSem_RagdollPath:
			case GSem_DtrPath:
			case GSem_BehaviorGraphPath:
				path=s;
				buf=bufRes;
				break;
			case GSem_TrrnBrushLibPath:
			{
				path=g_ssGuiLib.pWS->GetPath(WSPath_TrrnBrushLib);
				path=path+"\\"+s;
				buf=bufTrrnBrLib;
				break;
			}
			case GSem_BrushLibPath:
			{
				path=g_ssGuiLib.pWS->GetPath(WSPath_BrushLib);
				path=path+"\\"+sem.constraint;
				path=path+"\\"+s;
				buf=bufBrLib;
				break;
			}
			case GSem_MapFilePath:
			{
				path=g_ssGuiLib.pWS->GetPath(WSPath_Map);
				path=path+"\\"+s;
				buf=bufMapFile;
				break;
			}

			default:
				return;
			//XXXXX:more res type
		}

		if (path=="")
			return;
		if (!buf)
			return;

		for (int i=0;i<buf->size();i++)
		{
			if (StringEqualNoCase((*buf)[i].c_str(),path.c_str()))
				return;
		}
		buf->push_back(path);
	}

}

//目前:
//bufRes里返回的是相对于ResRoot的相对路径
//bufTrrnBrLib和bufBrLib返回的是绝对路径
void GetGObjRefPath(GObjBase *obj,std::vector<std::string>*bufRes,std::vector<std::string>*bufTrrnBrLib,std::vector<std::string>*bufBrLib
					,std::vector<std::string>*bufMapFile,std::vector<unsigned __int64>*bufProto)
{
	if(!obj)
		return;

	if (obj->GetBase())
		GetGObjRefPath(obj->GetBase(),bufRes,bufTrrnBrLib,bufBrLib,bufMapFile,bufProto);

	void *owner=obj->GetOwner();
	GElemBase *elem=obj->GetElems();

	void *varSub;
	GObjBase *objSub;
	DWORD cSub;

	while(elem)
	{
		while(1)
		{
			GVarType vt=elem->GetVarType();
			GSem sem=elem->GetSem();

			if (elem->GetObj(owner,&objSub))
			{//It's a single obj
				GetGObjRefPath(objSub,bufRes,bufTrrnBrLib,bufBrLib,bufMapFile,bufProto);
				break;
			}
			if (elem->GetVar(owner,&varSub))
			{//It's a single var
				GetVarRefPath(varSub,vt,sem,bufRes,bufTrrnBrLib,bufBrLib,bufMapFile,bufProto);
				break;
			}

			if (elem->GetSubCount(owner,&cSub))
			{
				if (elem->GetSubVar(NULL,0,&varSub))
				{
					for (int i=0;i<cSub;i++)
					{
						elem->GetSubVar(owner,i,&varSub);

						GetVarRefPath(varSub,vt,sem,bufRes,bufTrrnBrLib,bufBrLib,bufMapFile,bufProto);
					}
				}
				if (elem->GetSubObj(NULL,0,&objSub))
				{
					for (int i=0;i<cSub;i++)
					{
						elem->GetSubObj(owner,i,&objSub);

						GetGObjRefPath(objSub,bufRes,bufTrrnBrLib,bufBrLib,bufMapFile,bufProto);
					}
				}

				break;
			}
			break;
		}
		elem=elem->next;
	}

}
