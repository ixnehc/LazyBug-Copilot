#include "stdh.h"

#include "RenderSystem/ISpeedTree.h"

#include ".\guiagent_treeoperate.h"
#include "GuiData_Forest.h"

#include "GuiActor_Forest.h"
#include "ModBlockBack.h"

#include "WorldSystem/IAssetRenderer.h"

extern BOOL DrawCapsule(IRenderPort * rp,i_math::capsulef & cap);
extern void DrawSphere(IRenderPort * rp,i_math::vector3df &center,float radius,DWORD col,float nStep=10,int nSeg =20);


// #define SAMPLE_TEST

#define ID_MENU_TREEDEL	1010
#define ID_MENU_SHADOW  1011
#define ID_MENU_SHADOW_BASE	1012    // 1012 ->1017  

BOOL CGuiAgent_treeOperate::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_Forest * data = (GuiData_Forest *)(FindData("forest"));
	
	IForestEditor * editor = NULL;
	IBrushLib * pSptLib = NULL;
	if(data){
		editor = data->GetEditor();
		pSptLib = data->GetLib();
	}

	if(!pSptLib)
		return TRUE;
	
	BOOL bRet = CGuiAgent_3DNodeOperate::OnRButtonClick(x,y,flag);
	
	//选中一棵树时才可以修改光照贴图的情况
	if(editor&&data->hTreeSels.size()==1)
	{	
		_AddMenuSep();
		
		HMapObj hTree = data->hTreeSels[0];
		const TreeInfo * inst = editor->GetTreeInfo(hTree);
		if(inst)
		{
			int lvl = editor->GetShadowMapSize(hTree);
			float scale[5] = {0};
			BOOL bBig[5] = {FALSE};
			const int szBig = 256;

			const IBrush * br = pSptLib->Get(inst->refModel);
			ISpt * pSpt = (ISpt *)pSptLib->ObtainRes(br);
			if(pSpt){
				for(int i = 0;i<5;i++){
					scale[i] = pSpt->GetMapScale(i);
					i_math::size2di szBr,szFr;
					if(i>0){
						pSpt->GetMapSize(szBr,szFr,i);
						bBig[i] = (szBr.w>szBig||szBr.h>szBig||szFr.w>szBig||szFr.h>szBig);
					}
				}
			}

			char temp[255];
			if(lvl>0){
				if(bBig[lvl])
					sprintf(temp,"静态光照图(%.1f倍)容量过大",scale[lvl]);
				else
					sprintf(temp,"静态光照图(%.1f倍)",scale[lvl]);

			}
			else
				sprintf(temp,"静态光照图(无)");

			_PushMenu(temp);
			
			for(DWORD i = 0 ;i<5;i++){
				DWORD flag = (i==lvl)?MF_CHECKED:MF_UNCHECKED;
				sprintf(temp,"%.1f",scale[i]);
				_AddMenu(temp,ID_MENU_SHADOW_BASE+i,MF_ENABLED|MF_STRING|flag);
			}

			_PopMenu();
		}
	}

	return bRet;
}

BOOL CGuiAgent_treeOperate::OnCommand(DWORD idCmd)
{
	GuiData_Forest *data = static_cast<GuiData_Forest *>(FindData("forest"));
	assert(data);
	IForestEditor *editor = data->GetEditor();

	if(!editor||data->hTreeSels.empty())
		return TRUE;
	
	//////////////////////////////////////////////////////////////////////////
	CModManager * mgr = _GetModMgr();
	CGuiPanel_Forest * actor =static_cast<CGuiPanel_Forest *>(_GetActor());
	assert(actor);
	
	HMapObj hTree = data->hTreeSels[0];

	i_math::pos2di block;
	if(!editor->GetMapFileBlk(hTree,block))
		return TRUE;
	
	int szPixel = 0;
	switch(idCmd)
	{
	case ID_MENU_SHADOW_BASE+0:
	case ID_MENU_SHADOW_BASE+1:
	case ID_MENU_SHADOW_BASE+2:
	case ID_MENU_SHADOW_BASE+3:
	case ID_MENU_SHADOW_BASE+4:
		{
			szPixel = idCmd - ID_MENU_SHADOW_BASE;
			
			CModManager *mgr = _GetModMgr();
			if(mgr){
				const TreeInfo * inst = editor->GetTreeInfo(hTree);
				if(!inst)
					return TRUE;

				CModBlockBack * mod = new CModBlockBack(GetView());
				mod->SetCallBack<CGuiPanel_Forest>(actor,&CGuiPanel_Forest::CommitStatusData,&CGuiPanel_Forest::RestorStatusData);

				mod->BackupBlocks(&block,1);
				editor->SetShadowLvl(hTree,szPixel);
				Mod_New(mgr,(CModBase*&)(mod));			
			}
			_Redraw();
			break;		
		}
	default:
		break;
	}
	
	return CGuiAgent_3DNodeOperate::OnCommand(idCmd);
}

// 3DNodeOperate
void* CGuiAgent_treeOperate::_GetSelBuf()
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(data)
		return &(data->hTreeSels);
	return NULL;
}

BOOL CGuiAgent_treeOperate::_NeedClone()
{
	return TRUE;
}

H3DNode CGuiAgent_treeOperate::_Clone(H3DNode node)
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(!data)
		return  NULL;
	
	HMapObj  hTree = INVALID_HMAPOBJ;
	IForestEditor * editor = data->GetEditor();
	if(editor){
		const TreeInfo * info = editor->GetTreeInfo(HMapObj(node));
		if(info)
			hTree = editor->AddTree(*info);
	}
	return hTree;
}

void CGuiAgent_treeOperate::_CollectEnvelope(H3DNode *node,DWORD nNodes,Envelope &evlp)
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(!data)
		return;

	HMapObj  hTree = INVALID_HMAPOBJ;
	IForestEditor * editor = data->GetEditor();
	
	if(!editor)
		return;

#define Fill_Line(i0,i1){			\
	lines.push_back(corner[i0]);	\
	lines.push_back(corner[i1]);	\
}

	for(int i = 0;i<nNodes;i++){
		HMapObj hObj = HMapObj(node[i]);
		const TreeInfo * info = editor->GetTreeInfo(hObj);
		if(info){
			i_math::vector3df corner[8];
			info->aabb.getCorners(corner);
			std::vector<i_math::vector3df> &lines = evlp.lines;
			Fill_Line(0,1)
			Fill_Line(0,4)
			Fill_Line(5,1)
			Fill_Line(5,4)

			Fill_Line(2,6)
			Fill_Line(2,3)
			Fill_Line(7,6)
			Fill_Line(7,3)

			Fill_Line(2,0)
			Fill_Line(3,1)
			Fill_Line(7,5)
			Fill_Line(6,4)
		}
	}	
}

DWORD * CGuiAgent_treeOperate::_GetVer()
{
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(!data)
		return  NULL;
	return &(data->ver);
}
#ifdef SAMPLE_TEST
	#include <fstream>
#endif

BOOL CGuiAgent_treeOperate::OnDraw()
{
	IForestEditor * editor = NULL;
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(data)
		editor = data->GetEditor();

	if(!editor)
		return TRUE;
	
	IBrushLib * pSptLib = editor->GetSptLib();

	std::vector<H3DNode> * buf = (std::vector<H3DNode> *)_GetSelBuf();
	if (buf&&data->colObjVisible&&pSptLib)
	{
		for(int i = 0;i<buf->size();i++){
			const TreeInfo * info = editor->GetTreeInfo(HMapObj((*buf)[i]));
			if(!info)
				continue;
			const IBrush * br = pSptLib->Get(info->refModel);
			ISpt * pSpt = (ISpt *)pSptLib->ObtainRes(br);
			if(pSpt)
				_DrawCapsule(pSpt,info);
		}
	}

#ifdef SAMPLE_TEST
	std::ifstream ifs("d:\\test.dat",std::ios_base::in|std::ios_base::binary);
	if(ifs.is_open()){
		int count = 0;
		ifs.read((char *)(&count),sizeof(int));
		std::vector<i_math::vector3df> fPos(count);
		std::vector<i_math::vector3df> fNor(count);
		for(int i = 0;i<count;i++){
			ifs.read((char *)(&(fPos[i])),sizeof(i_math::vector3df));
			ifs.read((char *)(&(fNor[i])),sizeof(i_math::vector3df));
		}
		ifs.close();

		std::vector<i_math::vector3df> lines(2*count);
		std::vector<DWORD> col(2*count);
		for(int i = 0;i<count;i++){
			lines[2*i+0] = fPos[i];
			lines[2*i+1] = fPos[i] + 0.5f*fNor[i];
			col[2*i+0] = 0xffff0000;
			col[2*i+1] = 0xff00ff00;
		}
		IRenderPort * rp = GetRP();
		rp->Lines(lines.data(),count,col.data());
	}
#endif

	return CGuiAgent_3DNodeOperate::OnDraw();
}

IObjMapEditor * CGuiAgent_treeOperate::_GetEditor()
{
	IForestEditor * editor = NULL;
	GuiData_Forest * data = (GuiData_Forest *)FindData("forest");
	if(data)
		editor = data->GetEditor();
	return editor;
}

void CGuiAgent_treeOperate::_DrawCapsule(ISpt * pSpt,const TreeInfo * info)
{
	IRenderPort * rp = GetRP();
	assert(pSpt&&rp);

	CollisionObjectType colType ;
	int nCollision = pSpt->GetNumberOfCollisionObjects();

	for(int i = 0;i<nCollision;i++)
	{
		void * p = pSpt->GetCollisionObject(i,colType);
		switch(colType)
		{
		case CO_CAPSULE:
			{
				i_math::capsulef cap = *((i_math::capsulef *)(p));

				i_math::matrix43f mat;
				mat.setRotationY(info->rotY);
				mat.transformVect(cap.start,cap.start);
				mat.transformVect(cap.end,cap.end);

				cap.start  *= info->scale;
				cap.end    *= info->scale;
				cap.radius *= info->scale;

				cap.start += info->pos;	
				cap.end += info->pos;

				DrawCapsule(rp,cap);
				break;
			}
		case CO_SPHERE:
			{
				i_math::spheref sph = *((i_math::spheref *)(p));
				sph.center *= info->scale;
				sph.radius *= info->scale;

				sph.center += info->pos;

				DrawSphere(rp,sph.center,sph.radius,0xffff6600);
				break;
			}
		default:
			break;
		}
	}
}


