
#include "stdh.h"

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITools.h"


#include "GuiData_Baffles.h"

#include "GuiAgent_BafflesDraw.h"

CGuiAgent_BafflesDraw::CGuiAgent_BafflesDraw()
{

}
CGuiAgent_BafflesDraw::~CGuiAgent_BafflesDraw()
{

}

BOOL CGuiAgent_BafflesDraw::OnDraw()
{	
	GuiData_Baffles * data = (GuiData_Baffles *)FindData("baffle");
	if(!data)
		return TRUE;
	
	IBafflesEditor * editor = data->GetEditor();
	if(!editor)
		return TRUE;
	
	IRenderPort * rp = GetRP();
	
	_Update();
	
	std::list<BafflesNCache*>::iterator it;
	for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++){
		BafflesNCache * n = (*it);

		ICtrlPointPack * pack = n->data.pack;
		if(!pack)
			continue;

		if(!n->lines.empty()&&n->hObj!=data->hObjSel)
			rp->Lines(&(n->lines[0]),n->lines.size()/2,0xff2030ff);

		if(!n->tris.empty()){
			ShaderState state;
			//绘制正障碍面
			state.modeFacing = Facing_Front;
			rp->Triangles(&(n->tris[0]),n->tris.size()/3,0xffff0000,&state);
			//绘制负障碍面
			DWORD colBack = (pack->IsDoubleSide())?0xffff0000:0xff008300;
			state.modeFacing = Facing_Back;
			rp->Triangles(&(n->tris[0]),n->tris.size()/3,colBack,&state);
		}
	}

	std::list<BafflesNCache*> &cache = data->nodeCache;
	for(it=cache.begin();it!=cache.end();it++){
		BafflesNCache * n = *it;
		for(int i = 0;i<n->keyPos.size();i++){
			// 检查该节点是否处于选中状态
			if(n->hObj==data->hObjSel){
				if(!n->lines.empty())
					rp->Lines(&(n->lines[0]),n->lines.size()/2,0xffffff00);
				break;
			}
		}
	}
	
	return TRUE;
}

void CGuiAgent_BafflesDraw::_Update()
{
	GuiData_Baffles * data = (GuiData_Baffles *)FindData("baffle");
	if(!data)
		return;

	_EnumNode();
	if(!_UpdateBuffer()){
		if(!data->nodeCache.empty()){
			DWORD count = 0;	
			BafflesNCache * nodeUpdate = NULL;
			std::list<BafflesNCache*>::iterator it,itUpdate;
			for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++){
				BafflesNCache * n = *it;
				n->updateRef++;
				if(n->updateRef>count){
					count = n->updateRef;
					nodeUpdate = n;
					itUpdate = it;
				}
			}
			
			if(nodeUpdate){
				_UpdateNode(nodeUpdate);    //更新一个节点
				nodeUpdate->bUpdate = true;
				nodeUpdate->updateRef = 0;
				data->nodeCache.erase(itUpdate);
				data->nodeCache.push_back(nodeUpdate);	
			}
		}
	}
}

BOOL CGuiAgent_BafflesDraw::_UpdateNode(BafflesNCache * node)
{
	GuiData_Baffles * data = (GuiData_Baffles *)FindData("baffle");
	assert(data);
	
	IBafflesEditor *editor = data->GetEditor();
	if(!editor)
		return FALSE;

	ICtrlPointPack * pack = editor->GetCtrlPointPack(node->hObj);
	if(!pack)
		return FALSE;
	
	DWORD count = 0;
	i_math::vector3df * pos = NULL;
	pos = editor->GetGroundPos(node->hObj,count);
	node->lines.clear();
	node->tris.clear();
	if(count>1){
		int pre = 0;
		int next = 0;
		for(int  i = 0;i<count;i++){
		
			if(i!=count-1){
				pre = i;
				next = i+1;
			}
			else{
				if(!pack->IsClosed())
					break;
				pre = count -1;
				next = 0;
			}

			node->lines.push_back(pos[pre]);
			node->lines.push_back(pos[next]);
			//tris
			i_math::vector3df p2(pos[pre].x,pos[pre].y-0.5f,pos[pre].z);
			i_math::vector3df p3(pos[next].x,pos[next].y-0.5f,pos[next].z);
			node->tris.push_back(pos[next]);
			node->tris.push_back(p2);	
			node->tris.push_back(pos[pre]);

			node->tris.push_back(pos[next]);
			node->tris.push_back(p3);
			node->tris.push_back(p2);
		}
	}

	for(int  i = 0;i<count;i++){
		i_math::vector3df low = pos[i];
		low.y = -1000.0f;
		i_math::vector3df p2(pos[i].x,pos[i].y-0.5f,pos[i].z);
		node->lines.push_back(p2);
		node->lines.push_back(low);
	}

	//初始化关键点位置
	node->keyPos.clear();
	i_math::vector3df keyPos;
	for(int i = 0;i<pack->GetNumberOfCP();i++){
		editor->GetGroundPos(pack->At(i)->pos.x,pack->At(i)->pos.z,keyPos);
		node->keyPos.push_back(keyPos);
	}
	
	node->updateRef = 0;
	node->bUpdate = true;

	return TRUE;
}

BOOL CGuiAgent_BafflesDraw::_UpdateBuffer()
{
	GuiData_Baffles * data = (GuiData_Baffles *)FindData("baffle");
	if(!data)
		return FALSE;

	if(data->nodeCache.empty())
		_EnumNode();

	BOOL bUpdate = FALSE;
	if(!data->nodeCache.empty()){
		BafflesNCache * n = data->nodeCache.front();
		if(!n->bUpdate){
			if(_UpdateNode(n)){ //更新无效的点将不再加入到对尾
				data->nodeCache.push_back(n);
				n->bUpdate = true;
			}
			else{
				n->Clean();
				Class_Delete(n);
			}
			data->nodeCache.pop_front();
			bUpdate = TRUE;
		}
	}

	return bUpdate;
}

bool Cmp_BafNCache(BafflesNCache * n0,BafflesNCache * n1)
{
	int v0 = n0->bUpdate?1:0; 
	int v1 = n1->bUpdate?1:0;
	return (v0<v1||v0==v1&&n0->d2Cam<n1->d2Cam);
}

void CGuiAgent_BafflesDraw::_EnumNode()
{
	GuiData_Baffles * data = (GuiData_Baffles *)FindData("baffle");
	if(!data)
		return;
	
	IBafflesEditor * editor = data->GetEditor();
	if(!editor)
		return;
	
	IRenderPort * rp = GetRP();
	ICamera * cam = rp->GetCamera();
	i_math::vector3df eyePos;
	cam->GetEyePos(eyePos);

	DWORD count = 0;
	HMapObj * hObjs = NULL;
	hObjs = editor->Enum(eyePos,500.0f,count);
		
	std::list<BafflesNCache*>::iterator it;
	for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++){
		BafflesNCache * n = (*it);
		n->bInView = false;
	}
	
	for(int i = 0;i<count;i++){
		ICtrlPointPack * pack = editor->GetCtrlPointPack(hObjs[i]);
		if(!pack)
			continue;
		
		HMapObj &hObj = hObjs[i];
		BafflesNCache * node = NULL;

		for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++){
			BafflesNCache * n = (*it);
			if(hObj==n->hObj){
				node = n;
				break;
			}
		}
		
		if(!node){
			node = Class_New2(BafflesNCache);
			node->hObj = hObj;
			data->nodeCache.push_front(node);
			node->bUpdate = false;
			node->updateRef = 0;
			
			//关键点数据
			ICtrlPointPack * pack = editor->GetCtrlPointPack(node->hObj);
			if(pack){
				if(!node->data.pack)
					node->data.pack = editor->NewCtrlPointPack();
				node->data.pack->Clone(pack);
			}
		}
		else{
			ICtrlPointPack * pack = editor->GetCtrlPointPack(node->hObj);
			assert(node->data.pack);
			if(!node->data.equals(pack)){
				if(pack)
					node->data.pack->Clone(pack);
				else
					node->data.pack = NULL;
				node->bUpdate = false;
			}
		}
		
		node->bInView = true;		//标志该节点在场景中
		node->d2Cam = eyePos.getDistanceFromSQ(pack->GetCenter());
	}

	//删除不在场景中的节点
	for(it=data->nodeCache.begin();it!=data->nodeCache.end();){
		BafflesNCache * n = (*it);
		if(!n->bInView){
			it = data->nodeCache.erase(it);
			n->Clean();
			Class_Delete(n);
		}
		else
			it++;
	}
	
	data->nodeCache.sort(Cmp_BafNCache);
}



