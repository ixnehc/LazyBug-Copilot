
#include "stdh.h"

#include "RenderSystem/IRenderPort.h"

#include "RenderSystem/ITools.h"

#include "GuiData_Ridge.h"

#include "GuiAgent_RidgeDraw.h"


CGuiAgent_RidgeDraw::CGuiAgent_RidgeDraw()
{

}

CGuiAgent_RidgeDraw::~CGuiAgent_RidgeDraw()
{

}

BOOL CGuiAgent_RidgeDraw::OnDraw()
{	
	GuiData_Ridge * data = (GuiData_Ridge *)FindData("ridge");
	if(!data)
		return TRUE;

	IRidgeEditor * editor = data->GetEditor();
	if(!editor)
		return TRUE;

	IRenderPort * rp = GetRP();
	_Update();

	std::list<RidgeNCache*>::iterator it;
	for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++){
		RidgeNCache * n = (*it);

		ICtrlPointPack * pack = n->data.pack;
		if(!pack)
			continue;
		
		if(!n->trisRidge.empty()){
			ShaderState state;
			state.modeFacing = Facing_Both;
			state.modeBlend = Blend_AlphaBlend;
			state.modeDepth = Depth_NoWrite;
			rp->Triangles(&(n->trisRidge[0]),n->trisRidge.size()/3,ColorAlpha(0x7f0000,0x3f),&state);
		}

		if(!n->tris.empty()){
			ShaderState state;
			state.modeFacing = Facing_Both;
			if(n->hObj==data->hObjSel)
				rp->Triangles(&(n->tris[0]),n->tris.size()/3,0xffffff00,&state);
			else
				rp->Triangles(&(n->tris[0]),n->tris.size()/3,0xfff75480,&state);
		}
	}
	
	for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++){
		RidgeNCache * n = (*it);
		if(n->lines.empty())
			continue;
		if(n->hObj==data->hObjSel)
			rp->Lines(&(n->lines[0]),n->lines.size()/2,0xffffff00);
		else
			rp->Lines(&(n->lines[0]),n->lines.size()/2,0xff00ff66);
	}

	return TRUE;
}

void CGuiAgent_RidgeDraw::_Update()
{
	GuiData_Ridge * data = (GuiData_Ridge *)FindData("ridge");
	if(!data)
		return;

	_EnumNode();
	if(!_UpdateBuffer())
	{
		if(!data->nodeCache.empty())
		{
			DWORD count = 0;	
			RidgeNCache * nodeUpdate = NULL;
			std::list<RidgeNCache*>::iterator it,itUpdate;
			for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++)
			{
				RidgeNCache * n = *it;
				n->updateRef++;
				if(n->updateRef>count)
				{
					count = n->updateRef;
					nodeUpdate = n;
					itUpdate = it;
				}
			}

			if(nodeUpdate)
			{
				_UpdateNode(nodeUpdate);    //更新一个节点
				nodeUpdate->bUpdate = true;
				nodeUpdate->updateRef = 0;
				data->nodeCache.erase(itUpdate);
				data->nodeCache.push_back(nodeUpdate);	
			}
		}
	}
}

BOOL CGuiAgent_RidgeDraw::_UpdateNode(RidgeNCache * node)
{
	GuiData_Ridge * data = (GuiData_Ridge *)FindData("ridge");
	assert(data);

	IRidgeEditor *editor = data->GetEditor();
	if(!editor)
		return FALSE;

	ICtrlPointPack * pack = NULL;
	
	//初始化关键点位置	
	pack = editor->GetRidge(node->hObj);
	i_math::vector3df keyPos;
	if(pack){	
		node->keyPos.clear();
		for(int i = 0;i<pack->GetNumberOfCP();i++){
			editor->GetGroundPos(pack->At(i)->pos.x,pack->At(i)->pos.z,keyPos);
			keyPos.y += 2.0f;
			node->keyPos.push_back(keyPos);
		}
	}
	
	pack = editor->GetRidge(node->hObj);
	if(pack){
		node->ridge.clear();
		i_math::vector3df ridgePos;
		for(int i = 0;i<pack->GetNumberOfCP();i++){
			ridgePos = pack->At(i)->pos;
			ridgePos.y += 2.0f;
			node->ridge.push_back(ridgePos);
		}
	}
	
	//绘制图形 初始化
	i_math::vector3df lowCur,lowBack;
	node->lines.clear();
	node->tris.clear();
	node->trisRidge.clear();
	i_math::vector3df posCur,posPre,posBack,vec,posLow,step;

	if(!node->keyPos.empty()){
		//条带
		posPre = node->keyPos[0];
		for(int i = 1;i<node->keyPos.size();++i){
			posCur = node->keyPos[i];
			vec = posCur - posPre;
			step = vec;
			if(vec.equalsZero())
				continue;
			step.setLength(0.8f);
			while(1){
				posBack = posPre + step;
				bool bOverflow = (posCur-posBack).dotProduct(vec)<0;
				if(bOverflow)
					posBack = posCur;
				
				editor->GetGroundPos(posBack.x,posBack.z,keyPos);
				posBack.y = keyPos.y + 2.0f;
				node->lines.push_back(posPre);
				node->lines.push_back(posBack);

				lowCur = posPre;
				lowCur.y -= 0.5f;
				lowBack = posBack;
				lowBack.y -= 0.5f;

				node->tris.push_back(posPre);
				node->tris.push_back(posBack);
				node->tris.push_back(lowCur);

				node->tris.push_back(posBack);
				node->tris.push_back(lowBack);
				node->tris.push_back(lowCur);

				if(bOverflow)
					break;
				posPre = posBack;
			}
			posPre = posCur;
		}
		
		//山脊转折分割线
		for(int i = 0;i<pack->GetNumberOfCP();i++){
			lowCur = pack->At(i)->pos;
			lowCur.y -= 50.0f;
			editor->GetGroundPos(lowCur.x,lowCur.z,keyPos);
			posCur = keyPos;
			posCur.y += 1.5f;

			node->lines.push_back(lowCur);
			node->lines.push_back(posCur);
		}
	}

	//遮挡面
	pack = editor->GetCollisionPoints(node->hObj);
	if(pack){
		for(int i = 0;i<pack->GetNumberOfCP()-1;i+=2){

			posCur = pack->At(i)->pos;
			posBack = pack->At(i+1)->pos;

			posLow = posCur;
			posLow.y -= 500.0f;
			node->trisRidge.push_back(posCur);
			node->trisRidge.push_back(posBack);
			node->trisRidge.push_back(posLow);

			posCur = posBack;
			posBack.y -= 500.0f;
			node->trisRidge.push_back(posCur);
			node->trisRidge.push_back(posBack);
			node->trisRidge.push_back(posLow);
		}
	}

	node->updateRef = 0;
	node->bUpdate = true;

	return TRUE;
}

BOOL CGuiAgent_RidgeDraw::_UpdateBuffer()
{
	GuiData_Ridge *data = (GuiData_Ridge*)FindData("ridge");
	if(!data)
		return FALSE;

	if(data->nodeCache.empty())
		_EnumNode();

	BOOL bUpdate = FALSE;
	if(!data->nodeCache.empty())
	{
		RidgeNCache * n = data->nodeCache.front();
		if(!n->bUpdate)
		{
			if(_UpdateNode(n))
			{ //更新无效的点将不再加入到对尾
				data->nodeCache.push_back(n);
				n->bUpdate = true;
			}
			else
			{
				n->Clean();
				Class_Delete(n);
			}
			data->nodeCache.pop_front();
			bUpdate = TRUE;
		}
	}

	return bUpdate;
}

bool Cmp_BafNCache(RidgeNCache * n0,RidgeNCache * n1)
{
	int v0 = n0->bUpdate?1:0; 
	int v1 = n1->bUpdate?1:0;
	return (v0<v1||v0==v1&&n0->d2Cam<n1->d2Cam);
}

void CGuiAgent_RidgeDraw::_EnumNode()
{
	GuiData_Ridge *data = (GuiData_Ridge*)FindData("ridge");
	if(!data)
		return;

	IRidgeEditor * editor = data->GetEditor();
	if(!editor)
		return;

	IRenderPort * rp = GetRP();
	ICamera * cam = rp->GetCamera();
	i_math::vector3df eyePos;
	cam->GetEyePos(eyePos);

	DWORD count = 0;
	HMapObj * hObjs = NULL;
	hObjs = editor->Enum(eyePos,500.0f,count);

	std::list<RidgeNCache*>::iterator it;
	for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++)
	{
		RidgeNCache * n = (*it);
		n->bInView = false;
	}

	for(int i = 0;i<count;i++){
		ICtrlPointPack * pack = editor->GetRidge(hObjs[i]);
		if(!pack)
			continue;

		HMapObj &hObj = hObjs[i];
		RidgeNCache * node = NULL;

		for(it=data->nodeCache.begin();it!=data->nodeCache.end();it++){
			RidgeNCache * n = (*it);
			if(hObj==n->hObj){
				node = n;
				break;
			}
		}

		if(!node){
			node = Class_New2(RidgeNCache);
			node->hObj = hObj;
			data->nodeCache.push_front(node);
			node->bUpdate = false;
			node->updateRef = 0;

			//关键点数据
			ICtrlPointPack * pack = editor->GetRidge(node->hObj);
			if(pack){
				if(!node->data.pack)
					node->data.pack = editor->NewCtrlPointPack();
				node->data.pack->Clone(pack);
			}
		}
		else{
			ICtrlPointPack * pack = editor->GetRidge(node->hObj);
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
		RidgeNCache * n = (*it);
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



