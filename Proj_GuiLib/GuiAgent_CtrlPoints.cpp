
#include "stdh.h"

#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"


#include "AgentCmdID.h"

#include "GuiAgent_CtrlPoints.h"

#include "MapObjUtil.h"

class CGuiAgent_KeySelDraw :public CGuiAgent
{
public:
	CGuiAgent_KeySelDraw(CGuiAgent_CtrlPoints * owner)
	{
		_owner = owner;
		_mesh = NULL;
		_mtrl = NULL;
		_light = NULL;
	}
	~CGuiAgent_KeySelDraw()
	{
		SAFE_RELEASE(_mesh);
		SAFE_RELEASE(_mtrl);
		SAFE_RELEASE(_light);
	}

	virtual BOOL OnDraw()
	{
		//更新节点至最新状态
		_owner->_UpdateBufCP();

		if (!_owner->_cpPack)
			return TRUE;

		if(!_TouchRes())
			return TRUE;

		IRenderPort * rp = GetRP();
		ICamera * cam = rp->GetCamera();
		i_math::vector3df eyeDir;
		cam->GetEyeDir(eyeDir);
		_light->SetDirLight(eyeDir,0,0xffffffff,0xff555555);
		i_math::matrix43f mat;

		for(DWORD i = 0;i<_owner->_cpPack->GetNumberOfCP();i++){
			CtrlPoint * cp = _owner->_cpPack->At(i);
			mat.setScale(0.2f,0.2f,0.2f);
			mat.addTranslation(cp->pos);
			DWORD col = 0xff00ffff;

			// 检查该节点是否处于选中状态
			DWORD k = 0;
			for(;k<_owner->_GetSelCPs().size()&&_owner->_GetSelCPs()[k]!=i;k++);
			if(k<_owner->_GetSelCPs().size())
				col = 0xffffff00;

			rp->SimpleDrawMesh(_mesh,mat,col,FALSE,_mtrl,_light);

			ShaderState state;
			state.modeDepth=Depth_Disable;
			col=ColorAlpha(ColorAlpha_Color(col),128);
			rp->SimpleDrawMesh(_mesh,mat,col,FALSE,NULL,_light,&state);
		}

		return TRUE;
	}

	BOOL _TouchRes()
	{
		IRenderPort * rp  = GetRP();
		assert(rp);

		IRenderSystem * pRS = rp->GetRS();

		if(!_mesh){
			_mesh = (IMesh *)pRS->GetMeshMgr()->ObtainRes("_editor\\sphere.msh");
		}
		if(!_mtrl){
			_mtrl = (IMtrl *)pRS->GetMtrlMgr()->ObtainRes("_editor\\sphere.mtl");
		}
		if(!_light){
			_light = pRS->CreateLight();
		}

		if(A_Ok != _mesh->Touch())
			return FALSE;

		return TRUE;
	}
	BOOL OnLButtonDown(int x,int y,DWORD flag)
	{
		if(!(flag&CtrlOpFlag_CtrlDown)){
			_owner->_GetSelCPs().clear();
			_owner->_RefreshSelBuf();
		}
		
		int sel = _owner->_HitSel(x,y);

		if(sel>=0){
			int i = 0;
			for(;i<_owner->_GetSelCPs().size()&&_owner->_GetSelCPs()[i]!=sel;i++);
			if(i>=_owner->_GetSelCPs().size())
				_owner->_GetSelCPs().push_back(sel);
			else
				_owner->_GetSelCPs().erase(_owner->_GetSelCPs().begin()+i);

			_owner->_RefreshSelBuf();
			return FALSE;
		}

		return TRUE;
	}

private:
	IMesh * _mesh;
	IMtrl * _mtrl;
	ILight * _light;
	CGuiAgent_CtrlPoints * _owner;
};

//////////////////////////////////////////////////////////////////////////
class CGuiAgent_KeyPointOp :public CGuiAgent
{

public:

	CGuiAgent_KeyPointOp(CGuiAgent_CtrlPoints * owner)
	{
		_owner = owner;
	}

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag)
	{
		_AddMenuSep();

		size_t nSel = _owner-> _GetSelCPs().size();
		DWORD nKey = _owner->_cpPack->GetNumberOfCP();
		if(nSel>0&&(nKey - nSel>2))
			_AddMenu("删除点",ID_AGENT_PTDELETE);

		if(nSel==1){
			DWORD idx = (DWORD)_owner->_GetSelCPs()[0];
			if(idx!=0)
				_AddMenu("之前插入点",ID_AGENT_PTNEWPRE);
			if(idx<nKey-1)
				_AddMenu("之后插入点",ID_AGENT_PTNEWBACK);

			if(idx>1&&idx<nKey-2)
				_AddMenu("分裂",ID_AGENT_PTOBJSPLIT);
		}	

		if(!_owner->_cpPack->IsEmpty()){
			BOOL bFlag = 0;
			
			bFlag = _owner->_cpPack->IsClosed()?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("闭合",ID_AGENT_PTCLOSED,MF_ENABLED|MF_STRING|bFlag);

			bFlag = _owner->_cpPack->IsDoubleSide()?MF_CHECKED:MF_UNCHECKED;
			_AddMenu("双向",ID_AGENT_PTDOUBLESIDE,MF_ENABLED|MF_STRING|bFlag);
						
			_AddMenu("全选",ID_AGENT_PTSELECTALL);
		}

		return TRUE;
	}

	virtual BOOL OnCommand(DWORD idCmd)
	{
		std::vector<HMapObj> hObjsMod;
		DWORD nKey = _owner->_cpPack->GetNumberOfCP();
		
		BOOL bKeyPointChange = FALSE;
		switch(idCmd) {
			case ID_AGENT_PTDELETE:		//删除点
				{
					std::vector<DWORD> idx;
					for(int i = 0;i<_owner->_GetSelCPs().size();i++)
						idx.push_back(DWORD(_owner->_GetSelCPs()[i]));
					_owner->_cpPack->Remove(&(idx[0]),idx.size());
					bKeyPointChange = TRUE;
					break;
				}
			case ID_AGENT_PTNEWPRE:		//向前插入点
				{
					DWORD idx = (DWORD)_owner->_GetSelCPs()[0];	
					DWORD sz = _owner->_cpPack->GetNumberOfCP();
					if(sz>=2){
						if(idx>0){
							CtrlPoint *p0 = _owner->_cpPack->At(idx);
							CtrlPoint *p1 = _owner->_cpPack->At(idx-1);
							CtrlPoint *pN = _owner->_cpPack->NewCP();
							pN->Lerp(p0,p1,0.5f);
							_owner->_cpPack->Insert(idx,pN);
							_owner->_cpPack->DeleteCP(pN);
							bKeyPointChange = TRUE;
						}
					}
					break;
				}
			case ID_AGENT_PTNEWBACK:	//向后插入点
				{
					DWORD idx = (DWORD)_owner->_GetSelCPs()[0];					
					size_t sz = _owner->_cpPack->GetNumberOfCP();
					if(sz>=2){
						if(idx<sz-1){
							CtrlPoint *p0 = _owner->_cpPack->At(idx);
							CtrlPoint *p1 = _owner->_cpPack->At(idx+1);
							CtrlPoint *pN = _owner->_cpPack->NewCP();
							pN->Lerp(p0,p1,0.5f);
							_owner->_cpPack->Insert(idx+1,pN);
							_owner->_cpPack->DeleteCP(pN);
							bKeyPointChange = TRUE;
						}
					}

					break;
				}
			case ID_AGENT_PTOBJSPLIT: //分裂
				{	
					DWORD idx = (DWORD)_owner->_GetSelCPs()[0];					
					size_t sz = _owner->_cpPack->GetNumberOfCP();
					
					//增加一个对象
					ICtrlPointPack * pack_new = _owner->_NewCtrlPointPack();
					pack_new->Clone(_owner->_cpPack,0,idx+1);
					HMapObj hObj = _owner->_NewObj(_owner->_GetEditor(),pack_new);
					hObjsMod.push_back(hObj);
					pack_new->DeleteMe();
					
					//修改一个对象
					_owner->_cpPack->Clone(_owner->_cpPack,idx,_owner->_cpPack->GetNumberOfCP());
					bKeyPointChange = TRUE;
					break;
				}
			case ID_AGENT_PTSELECTALL: //全选
				{
					_owner->_GetSelCPs().clear();
					DWORD n = _owner->_cpPack->GetNumberOfCP();
					for(DWORD i = 0;i<n;i++)
						_owner->_GetSelCPs().push_back(H3DNode(i));
					break;
				}
			case ID_AGENT_PTCLOSED:
				{
					BOOL bClosed = _owner->_cpPack->IsClosed();
					_owner->_cpPack->SetClosed(!bClosed);
					bKeyPointChange = TRUE;
					break;
				}
			case ID_AGENT_PTDOUBLESIDE:
				{
					BOOL bDoubleSide = _owner->_cpPack->IsDoubleSide();
					_owner->_cpPack->SetDoubleSide(!bDoubleSide);
					bKeyPointChange = TRUE;
					break;
				}
				default: break;
		}
		
		//当前对象节点发生了修改
		if(bKeyPointChange){
			HMapObj hObjNew = _owner->_SetKeyPos(_owner->_hObjEdit,_owner->_cpPack);
			hObjsMod.push_back(_owner->_hObjEdit);
			if(hObjNew!=_owner->_hObjEdit){
				hObjsMod.push_back(hObjNew);
				//重新设置选中状态
				if(_owner->_GetSelObj()){
					*(_owner->_GetSelObj()) = hObjNew;
					_owner->_hObjEdit = hObjNew;
				}
			}
		}
		
		if(!hObjsMod.empty()){
			IObjMapEditor * editor = _owner->_GetEditor();
			if(editor)
				CommitMapObjMod(_GetModMgr(),GetView(),hObjsMod,editor);
			
			DWORD * ver = _owner->_GetVer();
			if(ver)
				(*ver)++;
		}
		return TRUE;
	}
	
private:
	CGuiAgent_CtrlPoints * _owner;
};
//////////////////////////////////////////////////////////////////////////

//创建由关键点构成的对象类型
class CGuiAgent_KeyPointCreate:public CGuiAgent
{
public:
	enum State
	{
		PTC_CREATE,
		PTC_APPFRONT,
		PTC_APPBACK,
		PTC_CON,
		PTC_IDLE,
	};

	CGuiAgent_KeyPointCreate(CGuiAgent_CtrlPoints * owner);
	BOOL OnRButtonClick(int x,int y,DWORD flag);
	BOOL OnMouseMove(int x,int y,DWORD flag);
	BOOL OnDraw();
	BOOL _HitGroundPos(HMapObj * hObjTempSel,int x,int y,i_math::vector3df &pos);
	BOOL OnLButtonDown(int x,int y,DWORD flag);
	BOOL OnCommand(DWORD idCmd);
	void _GetLimitedPos(i_math::vector3df & cur);
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	void BeginCreate();
	void EndCreate();

private:
	ICtrlPointPack * _pack2create;
	CGuiAgent_CtrlPoints * _owner;
	State _PTCState;
	i_math::vector3df _ptmove;
	
	i_math::vector3df _posStart;
	i_math::vector3df _posEnd;
	HMapObj _hObjCon2;
	DWORD _count;

	std::vector<i_math::vector3df> _linesRange;
};

CGuiAgent_KeyPointCreate::CGuiAgent_KeyPointCreate(CGuiAgent_CtrlPoints * owner)
{
	_owner = owner;
	_PTCState = PTC_IDLE;
	_hObjCon2 = INVALID_HMAPOBJ;
	_pack2create = NULL;
}

void CGuiAgent_KeyPointCreate::OnAttachView(CGeView *view,DWORD iLevel)
{
	_pack2create = _owner->_NewCtrlPointPack();
	CGuiAgent::OnAttachView(view,iLevel);
}

void CGuiAgent_KeyPointCreate::OnDetachView(CGeView *view,DWORD iLevel)
{
	if(_pack2create)
		_pack2create->DeleteMe();
	_pack2create=NULL;
	CGuiAgent::OnDetachView(view,iLevel);
}

void CGuiAgent_KeyPointCreate::_GetLimitedPos(i_math::vector3df & cur)
{
	IObjMapEditor * editor = _owner->_GetEditor();
	if(!editor||_pack2create->IsEmpty())
		return;

	i_math::aabbox3df abb = _pack2create->GetAABB();
	float len = editor->GetMapBlockLen();

	float rgl = 2.0f*len;

	float x0 = abb.MaxEdge.x - rgl;
	float z0 = abb.MaxEdge.z - rgl;

	float x1 = abb.MinEdge.x + rgl;
	float z1 = abb.MinEdge.z + rgl;

	if(TRUE){
		float h = abb.MaxEdge.y + 0.15f;
		i_math::vector3df p[4];
		p[0].set(x0,h,z0);
		p[1].set(x1,h,z0);
		p[2].set(x1,h,z1);
		p[3].set(x0,h,z1);

		_linesRange.clear();
		_linesRange.push_back(p[0]);
		_linesRange.push_back(p[1]);
		_linesRange.push_back(p[1]);
		_linesRange.push_back(p[2]);
		_linesRange.push_back(p[2]);
		_linesRange.push_back(p[3]);
		_linesRange.push_back(p[3]);
		_linesRange.push_back(p[0]);
	}

	// 如果溢出 返回最近的边界点
	if(cur.x<x0||cur.x>x1||cur.z<z0||cur.z>z1){

		i_math::line2df line[4];
		line[0].start.set(x1,z1); //上侧边
		line[0].end.set(x0,z1);

		line[1].start.set(x0,z1); //左测边
		line[1].end.set(x0,z0);

		line[2].start.set(x0,z0); //下侧边
		line[2].end.set(x1,z0);

		line[3].start.set(x1,z0); //右侧边
		line[3].end.set(x1,z1);

		i_math::vector2df out,vec0,vec1;
		i_math::vector2df cur2d(cur.x,cur.z);
		i_math::line2df ray;
		float mind = 999999.0f;
		i_math::vector2df minPos;

		//Up
		if(cur.z>z1){
			line[0].getProjectionPoint(cur2d,out);
			vec0 = line[0].end - cur2d; 
			vec1 = line[0].start - cur2d;
			if(vec0.dotProduct(vec1)<=0){ 
				float d = (float)(cur2d -out).getLengthSQ();
				if(d<mind){
					mind = d;
					minPos = out;
				}
			}
		}

		//Left 
		if(cur.x<x0){
			line[1].getProjectionPoint(cur2d,out);
			vec0 = line[1].end - cur2d; 
			vec1 = line[1].start - cur2d;
			if(vec0.dotProduct(vec1)<=0){ 
				float d = (float)(cur2d -out).getLengthSQ();
				if(d<mind){
					mind = d;
					minPos = out;
				}
			}
		}

		//Down
		if(cur.z<z0){
			line[2].getProjectionPoint(cur2d,out);
			vec0 = line[2].end - cur2d; 
			vec1 = line[2].start - cur2d;
			if(vec0.dotProduct(vec1)<=0){ 
				float d = (float)(cur2d -out).getLengthSQ();
				if(d<mind){
					mind = d;
					minPos = out;
				}
			}
		}

		//Right
		if(cur.x>x1){
			line[3].getProjectionPoint(cur2d,out);
			vec0 = line[3].end - cur2d; 
			vec1 = line[3].start - cur2d;
			if(vec0.dotProduct(vec1)<=0){ 
				float d = (float)(cur2d -out).getLengthSQ();
				if(d<mind){
					mind = d;
					minPos = out;
				}
			}
		}

		if(mind<999999.0f){
			cur.x = minPos.x;
			cur.z = minPos.y;
		}
		else{
			cur.x = (cur.x-x0<x1-cur.x)?x0:x1;
			cur.z = (cur.z-z0<z1-cur.z)?z0:z1;
		}
	}
}

BOOL CGuiAgent_KeyPointCreate::OnRButtonClick(int x,int y,DWORD flag)
{
	if(!_owner->_GetEditor())
		return TRUE;

	if(_PTCState==PTC_IDLE){
		_AddMenuSep();
		_AddMenu("新建",ID_AGENT_PTOBJNEW);
		if(_owner->_hObjEdit!=INVALID_HMAPOBJ){
			_AddMenu("向前增补",ID_AGENT_PTOBJAPPPRE);
			_AddMenu("向后增补",ID_AGENT_PTOBJAPPBACK);
			_AddMenu("连接",ID_AGENT_PTOBJCONNECT);
			_AddMenu("删除",ID_AGENT_PTOBJDELETE);
			_AddMenu("克隆",ID_AGENT_PTCLONE);
			_AddMenu("反向",ID_AGENT_PTOBJINVERT);
		}
	}
	else{
		EndCreate();
		_owner->_EndCreate();	//通知创建结束
		return FALSE;
	}

	return TRUE;
}

BOOL CGuiAgent_KeyPointCreate::OnDraw()
{	
	IRenderPort * rp = GetRP();

	if(_PTCState!=PTC_IDLE){
		BOOL bCon2Tail = (_PTCState!=PTC_APPFRONT);
		if(_owner->_DrawOnCreate(_pack2create,_ptmove,bCon2Tail)){
			std::vector<i_math::vector3df> lines;
			if(_pack2create->GetNumberOfCP()>1){
				for(int i = 0;i<_pack2create->GetNumberOfCP()-1;i++){
					CtrlPoint * p0 = _pack2create->At(i);
					CtrlPoint * p1 = _pack2create->At(i+1);
					lines.push_back(p0->pos);
					lines.push_back(p1->pos);
				}
			}
			
			if(!_pack2create->IsEmpty()){
				switch(_PTCState){
					case PTC_APPFRONT:
						lines.push_back(_pack2create->At(0)->pos);
						lines.push_back(_ptmove);
						break;
					case PTC_APPBACK:
					case PTC_CREATE:
						lines.push_back(_pack2create->Back()->pos);
						lines.push_back(_ptmove);
						break;
					default :break;
				}
			}

			rp->Lines(&(lines[0]),lines.size()/2,0xff00ff00);
		}
	}

	if(_PTCState==PTC_CON){
		DWORD col[] = {0xffffff00,0xff00ff00};
		DWORD colSwitch = col[1];
		if(_hObjCon2!=INVALID_HMAPOBJ)
			colSwitch = col[(_count++/5)%2];
		rp->Line(_posStart,_posEnd,colSwitch);
	}
	
	if(_PTCState!=PTC_IDLE&&!_linesRange.empty())
		rp->Lines(_linesRange.data(),_linesRange.size()/2,0xffffff00);

	return TRUE;
}

BOOL CGuiAgent_KeyPointCreate::OnMouseMove(int x,int y,DWORD flag)
{
	IRenderPort * rp = GetRP();
	HitProbe hitProbe;
	rp->CalcHitProbe(x,y,hitProbe);
	i_math::vector3df pos;

	if(_PTCState==PTC_CON){

		i_math::vector3df intersec;
		_hObjCon2 = _owner->_HitTest(hitProbe,&intersec);
		
		if(_owner->_HitGroundPos(hitProbe,pos))		
			_posEnd = pos;
		
		assert(!_owner->_cpPack->IsEmpty());
		if(_hObjCon2!=_owner->_hObjEdit&&_hObjCon2!=INVALID_HMAPOBJ){
			ICtrlPointPack * pack_tmp = _owner->_NewCtrlPointPack();
			_owner->_GetKeyPos(_hObjCon2,pack_tmp);

			i_math::vector3df p0 = pack_tmp->At(0)->pos;
			i_math::vector3df p1 = pack_tmp->Back()->pos;
			
			float d0 = p0.getDistanceFromSQ(intersec);
			float d1 = p1.getDistanceFromSQ(intersec);

			//得到最近的连接点
			_posEnd = (d0<d1)?p0:p1;

			pack_tmp->DeleteMe();
		}
		else{
			_hObjCon2 = INVALID_HMAPOBJ;
		}
	}
	else if(_PTCState!=PTC_IDLE){

		i_math::vector3df intersec;
		_hObjCon2 = _owner->_HitTest(hitProbe,&intersec);

		if(_HitGroundPos(&_hObjCon2,x,y,pos)){
			_GetLimitedPos(pos);
			_ptmove = pos;
			_ptmove.y += 0.15f;
		}
	}
	return TRUE;
}

BOOL CGuiAgent_KeyPointCreate::_HitGroundPos(HMapObj* hObjTempSel,int x,int y,i_math::vector3df &pos)
{		
	int sel = _owner->_HitSel(x,y,hObjTempSel,&pos); //优先选中节点 方便与连接到已有的节点
	if(sel>=0){
		_GetLimitedPos(pos);
		return TRUE;
	}
	//找到与世界的交点
	IRenderPort * rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	if(_owner->_HitGroundPos(probe,pos)){
		_GetLimitedPos(pos);
		return TRUE;
	}

	return FALSE;
}

BOOL CGuiAgent_KeyPointCreate::OnLButtonDown(int x,int y,DWORD flag)
{
	if(_PTCState==PTC_CON){
		std::vector<HMapObj> hObjsMod;
		if(_hObjCon2!=INVALID_HMAPOBJ){
			
			ICtrlPointPack * pack_con = _owner->_NewCtrlPointPack();
			_owner->_GetKeyPos(_hObjCon2,pack_con);
			
			//连接到头
			if(_posEnd==pack_con->At(0)->pos){
				for(int i = 0;i<pack_con->GetNumberOfCP();i++){
					_owner->_cpPack->Push(pack_con->At(i));
				}
			}
			else{ //连接到尾部
				for(int i = pack_con->GetNumberOfCP()-1;i>=0;i--){
					_owner->_cpPack->Push(pack_con->At(i));
				}
			}

			pack_con->DeleteMe();

			hObjsMod.push_back(_owner->_hObjEdit);
			HMapObj hObj = _owner->_SetKeyPos(_owner->_hObjEdit,_owner->_cpPack);
			if(hObj!=_owner->_hObjEdit)
				hObjsMod.push_back(hObj);

			IObjMapEditor * editor = _owner->_GetEditor();
			if(editor){
				editor->Delete(_hObjCon2);
				hObjsMod.push_back(_hObjCon2);
			}
		}

		if(!hObjsMod.empty())
			CommitMapObjMod(_GetModMgr(),GetView(),hObjsMod,_owner->_GetEditor());

		_PTCState = PTC_IDLE;
		_hObjCon2 = INVALID_HMAPOBJ;

		return FALSE;
	}
	else if(_PTCState!=PTC_IDLE){
		i_math::vector3df pos;			
		if(_HitGroundPos(NULL,x,y,pos)){
			CtrlPoint * p = _owner->_New(_ptmove);
			if(_PTCState==PTC_APPFRONT)
				_pack2create->Insert(0,p);
			else
				_pack2create->Push(p);
			_pack2create->DeleteCP(p);

			_owner->_OnCreate();	//通知状态的改变
		}
		return FALSE;
	}

	return TRUE;
}

BOOL CGuiAgent_KeyPointCreate::OnCommand(DWORD idCmd)
{
	switch(idCmd){
		case ID_AGENT_PTOBJNEW:
			{
				_pack2create->Clean();
				_PTCState = PTC_CREATE;
				break;
			}
		case ID_AGENT_PTOBJAPPPRE:
			{
				_pack2create->Clone(_owner->_cpPack);
				_PTCState = PTC_APPFRONT;
				break;
			}
		case ID_AGENT_PTOBJAPPBACK:
			{
				_pack2create->Clone(_owner->_cpPack);
				_PTCState = PTC_APPBACK;
				break;
			}
		case ID_AGENT_PTOBJCONNECT:  //连接
			{
				_PTCState = PTC_CON;
				_pack2create->Clone(_owner->_cpPack);
				_posStart = _pack2create->Back()->pos;
				_hObjCon2 = INVALID_HMAPOBJ;
				break;
			}
		case ID_AGENT_PTCLONE:
			{
				IObjMapEditor * editor = _owner->_GetEditor();
				if(editor&&!_owner->_cpPack->IsEmpty()){
					HMapObj hObj = _owner->_NewObj(editor,_owner->_cpPack);
					HMapObj * hObjSel = _owner->_GetSelObj();
					if(hObjSel){
						*hObjSel = hObj;
						_owner->_hObjEdit = hObj;
					}
					CommitMapObjMod(_GetModMgr(),GetView(),hObj,editor);
					//选中新建对象的所有节点
					_owner->_GetSelCPs().clear();
					for(DWORD i = 0;i<_owner->_cpPack->GetNumberOfCP();i++)
						_owner->_GetSelCPs().push_back(H3DNode(i));
				}
				break;
			}
		case ID_AGENT_PTOBJDELETE: //删除
			{
				IObjMapEditor * editor = _owner->_GetEditor();
				if(editor){
					if(editor->Delete(_owner->_hObjEdit)){
						CommitMapObjMod(_GetModMgr(),GetView(),_owner->_hObjEdit,editor);
						_owner->_hObjEdit = INVALID_HMAPOBJ;
					}
				}
				break;
			}
		case ID_AGENT_PTOBJINVERT:
			{
				size_t i = 0;
				size_t j = _owner->_cpPack->GetNumberOfCP()-1;
				CtrlPoint * p0,*p1,*pTemp;
				pTemp = _owner->_cpPack->NewCP();
				while(i<j){
					p0 = _owner->_cpPack->At(i);
					p1 = _owner->_cpPack->At(j);
					pTemp->Clone(p0);
					_owner->_cpPack->Set(i,p1);
					_owner->_cpPack->Set(j,pTemp);
					i++;
					j--;
				}
				IObjMapEditor * editor = _owner->_GetEditor();
				if(editor){
					HMapObj hObjNew = _owner->_SetKeyPos(_owner->_hObjEdit,_owner->_cpPack);
					CommitMapObjMod(_GetModMgr(),GetView(),_owner->_hObjEdit,editor);
				}
				_owner->_cpPack->DeleteCP(pTemp);
				break;
			}
		default :break;
	}

	//通知状态改变
	switch(_PTCState)
	{
		case PTC_APPFRONT:
		case PTC_APPBACK:
		case PTC_CREATE:
		case PTC_CON:
			_owner->_BeginCreate();
			break;
		default: break;
	}

	return TRUE;
}

void CGuiAgent_KeyPointCreate::BeginCreate()
{
	_pack2create->Clean();
	_PTCState = PTC_CREATE;
}

void CGuiAgent_KeyPointCreate::EndCreate()
{
	std::vector<HMapObj> hObjsMod;
	switch(_PTCState)
	{
	case PTC_CREATE:
		{
			HMapObj hObj = _owner->_NewObj(_owner->_GetEditor(),_pack2create);

			//将当前选中的对象 设为新建后的对象
			HMapObj * hObjSel = _owner->_GetSelObj();
			if(hObjSel)
				*hObjSel = hObj;
			_owner->_hObjEdit = hObj;

			hObjsMod.push_back(hObj);
			break;
		}
	case PTC_APPFRONT:
	case PTC_APPBACK:
		{
			HMapObj hObj = _owner->_SetKeyPos(_owner->_hObjEdit,_pack2create);

			//将当前选中的对象 设为新建后的对象
			HMapObj * hObjSel = _owner->_GetSelObj();
			if(hObjSel)
				*hObjSel = hObj;
			_owner->_hObjEdit = hObj;

			hObjsMod.push_back(_owner->_hObjEdit);
			if(hObj!=_owner->_hObjEdit)
				hObjsMod.push_back(hObj);
			break;
		}
	default:
		break;
	}
	CommitMapObjMod(_GetModMgr(),GetView(),hObjsMod,_owner->_GetEditor());
	_PTCState = PTC_IDLE;
	_pack2create->Clean();
}

//////////////////////////////////////////////////////////////////////////
class CGuiAgent_KeyObjSel :public CGuiAgent
{
public:
	CGuiAgent_KeyObjSel(CGuiAgent_CtrlPoints * owner){_owner = owner;}
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag)
	{
		IRenderPort * rp = GetRP();
		HitProbe rayHit;
		rp->CalcHitProbe(x,y,rayHit);

		HMapObj *hObj = _owner->_GetSelObj();
		if(hObj){
			HMapObj hObjSel = _owner->_HitTest(rayHit,NULL);
			_owner->OnSelected(hObjSel);
			if(*hObj!=hObjSel)
				*hObj = hObjSel;
		}
		return TRUE;
	}
private:
	CGuiAgent_CtrlPoints * _owner;
};
//////////////////////////////////////////////////////////////////////////
class CGuiAgent_KeyMove :public CGuiAgent_3DNodeMatEdit
{	
public:
	CGuiAgent_KeyMove(CGuiAgent_CtrlPoints * owner)
	:CGuiAgent_3DNodeMatEdit(EditMode_Move){
		_owner = owner;
	}
protected:
	virtual  void * _GetSelBuf();
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual i_math::pos2di *_GetBlock(H3DNode node);
	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);
private:
	CGuiAgent_CtrlPoints * _owner;
	i_math::matrix43f _matTemp;
	i_math::pos2di _ptBlk;
};
void * CGuiAgent_KeyMove::_GetSelBuf()
{
	_owner->_UpdateBufCP();
	return &_owner->_GetSelCPs();
}

i_math::matrix43f * CGuiAgent_KeyMove::_GetMat(H3DNode node)
{
	DWORD idx = DWORD(node);
	CtrlPoint * p = _owner->_cpPack->At(idx);
	if(p){
		_matTemp.setTranslation(_owner->_cpPack->At(idx)->pos);
		return &_matTemp;
	}
	return NULL;
}

void CGuiAgent_KeyMove::_Move(H3DNode &node,i_math::matrix43f &mat)
{
	DWORD idx = DWORD(node);
	CtrlPoint * p = _owner->_cpPack->At(idx);
	if(p)
		p->pos = mat.getTranslation();

	HMapObj hObjNew = _owner->_SetKeyPos(_owner->_hObjEdit,_owner->_cpPack);

	if(hObjNew!=_owner->_hObjEdit){
		HMapObj * hObj = _owner->_GetSelObj();
		if(hObj)
			*hObj = hObjNew;
		_owner->_hObjEdit = hObjNew;
	}
}

i_math::pos2di * CGuiAgent_KeyMove::_GetBlock(H3DNode node)
{
	if(_owner->_GetObjBlock(_owner->_hObjEdit,_ptBlk))
		return &_ptBlk;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CGuiAgent_CtrlPoints::CGuiAgent_CtrlPoints(void)
{
	_hObjEdit = INVALID_HMAPOBJ;
	_cpPack = NULL;
	_agentDraw		  = NULL;
	_agentCtrlPointOp = NULL;
	_agentCreate	  = NULL;
	_agentSel		  = NULL;
	_agentMove		  = NULL;
}

CGuiAgent_CtrlPoints::~CGuiAgent_CtrlPoints(void)
{
}

void CGuiAgent_CtrlPoints::_UpdateBufCP(void)
{
	HMapObj * phObj = _GetSelObj();
	if(!phObj){
		_hObjEdit = INVALID_HMAPOBJ;
		return;
	}

	HMapObj hObjSel = *phObj;
	if(hObjSel!=_hObjEdit){
		_hObjEdit = hObjSel;
		_GetSelCPs().clear();
	}
	
	//比较内容
	if(TRUE){
		ICtrlPointPack * pack_now = _NewCtrlPointPack();
		if (!pack_now)
			return;

		_GetKeyPos(_hObjEdit,pack_now);
		
		if(!pack_now->Equals(_cpPack)){
			_cpPack->DeleteMe();
			_cpPack = pack_now;
			
			//更新当前选中的节点列表
			std::vector<H3DNode> temp;
			for(int i =0;i<_GetSelCPs().size();i++)
				if(_GetSelCPs()[i]<pack_now->GetNumberOfCP())
					temp.push_back(_GetSelCPs()[i]);
			if(temp.size()!=_GetSelCPs().size())
				temp.swap(_GetSelCPs());
		}
		else{
			pack_now->DeleteMe();
		}
	}

	//将内部的选择状态更新到外部
	_RefreshSelBuf();
}

BOOL CGuiAgent_CtrlPoints::_GetObjBlock(const HMapObj &hObj,i_math::pos2di &ptBlk)
{
	IObjMapEditor *editor = _GetEditor();
	if(!editor)
		return FALSE;
	return editor->GetMapFileBlk(hObj,ptBlk);
}

CtrlPoint * CGuiAgent_CtrlPoints::_New(i_math::vector3df &pos)
{
	CtrlPoint * p = _cpPack->NewCP();
	p->pos = pos;
	_OnNewCP(p);
	return p;
}

int CGuiAgent_CtrlPoints::_HitSel(int x,int y,HMapObj * hObjTempSel /*= NULL*/,i_math::vector3df *posIntersec /*= NULL*/)
{
	IRenderPort * rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	float mind = 9999999.0f;
	int sel = -1;
	i_math::vector3df pos,posProj;

	ICtrlPointPack * packNew = NULL;
	if(hObjTempSel){
		packNew = _NewCtrlPointPack();
		_GetKeyPos(*hObjTempSel,packNew);
	}
	
	ICtrlPointPack * packTest = (packNew)?packNew:_cpPack;
	for(int i = 0;i<packTest->GetNumberOfCP();i++){
		pos = packTest->At(i)->pos;
		probe.getProjectionPoint(pos,posProj);
		float dSQ = (float)(posProj - pos).getLengthSQ();
		if(dSQ <0.04){
			if(dSQ<mind){
				sel = i;
				mind = dSQ;
			}
		}
	}
	
	if(posIntersec&&sel>=0){
		CtrlPoint * p = packTest->At(sel);
		*posIntersec = p->pos;
	}

	if(packNew)
		packNew->DeleteMe();

	return sel;
}

void CGuiAgent_CtrlPoints::_RefreshSelBuf()
{
	std::vector<DWORD> * buf = (std::vector<DWORD> *)_GetSelPointsBuf();
	if(buf){
		buf->resize(_GetSelCPs().size());
		for(int i = 0;i<_GetSelCPs().size();i++)
			(*buf)[i] = DWORD(_GetSelCPs()[i]);
	}
}

HMapObj CGuiAgent_CtrlPoints::_HitTest(i_math::line3df &ray,i_math::vector3df *intersec)
{
	HMapObj hObj = INVALID_HMAPOBJ;

	IObjMapEditor * editor = _GetEditor();
	if(editor)
		hObj = editor->HitTest(ray,intersec);
	
	return hObj;
}

BOOL CGuiAgent_CtrlPoints::Respond(CtrlOp &co)
{
	if (!_cpPack)
		return TRUE;
	return CGuiAgent::Respond(co);
}


void CGuiAgent_CtrlPoints::OnAttachView(CGeView *view,DWORD iLevel)
{
	if(_GetEditor()){
		_agentDraw		  = new CGuiAgent_KeySelDraw(this);
		_agentCtrlPointOp = new CGuiAgent_KeyPointOp(this);
		_agentCreate	  = new CGuiAgent_KeyPointCreate(this);
		_agentSel		  = new CGuiAgent_KeyObjSel(this);
		_agentMove		  = new CGuiAgent_KeyMove(this);
		
		view->AddAgent(_iLevelInView,_agentDraw,_priority);
		view->AddAgent(_iLevelInView,_agentCtrlPointOp,_priority);	
		view->AddAgent(_iLevelInView,_agentMove,_priority);
		view->AddAgent(_iLevelInView,_agentCreate,_priority);
		view->AddAgent(_iLevelInView,_agentSel,_priority-1);
	}
	
	CGuiAgent::OnAttachView(view,iLevel);

	_cpPack = _NewCtrlPointPack();
}

void CGuiAgent_CtrlPoints::OnDetachView(CGeView *view,DWORD iLevel)
{
	if(_cpPack)
		_cpPack->DeleteMe();
	_cpPack=NULL;
	CGuiAgent::OnDetachView(view,iLevel);

	_agentDraw		  = NULL;
	_agentCtrlPointOp = NULL;
	_agentCreate	  = NULL;
	_agentSel		  = NULL;
	_agentMove= NULL;
}

void CGuiAgent_CtrlPoints::Enable(BOOL bEnable)
{
	if(_agentDraw)
		_agentDraw->Enable(bEnable);
	
	if(_agentCtrlPointOp)
		_agentCtrlPointOp->Enable(bEnable);
	
	if(_agentCreate)
		_agentCreate->Enable(bEnable);
	
	if(_agentSel)
		_agentSel->Enable(bEnable);
	
	if(_agentMove)
		_agentMove->Enable(bEnable);

	return CGuiAgent::Enable(bEnable);
}

void CGuiAgent_CtrlPoints::UpdateBind()
{
	if(_agentMove)
		_agentMove->UpdateBind();
}

void CGuiAgent_CtrlPoints::BeginCreate()
{
	if(_agentCreate)
		_agentCreate->BeginCreate();	
}

void CGuiAgent_CtrlPoints::EndCreate()
{
	if(_agentCreate)
		_agentCreate->EndCreate();
}



