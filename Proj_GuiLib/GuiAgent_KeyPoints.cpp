
#include "stdh.h"

#include "AgentCmdID.h"

#include "GuiAgent_KeyPoints.h"

#include "MapObjUtil.h"

class CGuiAgent_KeySelDraw :public CGuiAgent
{
public:
	CGuiAgent_KeySelDraw(CGuiAgent_KeyPoints * owner)
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
			for(;k<_owner->_selCPs.size()&&_owner->_selCPs[k]!=i;k++);
			if(k<_owner->_selCPs.size())
				col = 0xffffff00;

			rp->SimpleDrawMesh(_mesh,mat,col,FALSE,_mtrl,_light);
		}

		return TRUE;
	}

	BOOL _TouchRes()
	{
		IRenderPort * rp  = GetRP();
		assert(rp);

		IRenderSystem * pRS = rp->GetRS();

		if(!_mesh){
			_mesh = (IMesh *)pRS->GetMeshMgr()->ObtainRes("editor\\sphere.msh");
		}
		if(!_mtrl){
			_mtrl = (IMtrl *)pRS->GetMtrlMgr()->ObtainRes("editor\\sphere.mtl");
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
			_owner->_selCPs.clear();
			_owner->_RefreshSelBuf();
		}
		
		int sel = _owner->_HitSel(x,y);

		if(sel>=0){
			int i = 0;
			for(;i<_owner->_selCPs.size()&&_owner->_selCPs[i]!=sel;i++);
			if(i>=_owner->_selCPs.size())
				_owner->_selCPs.push_back(sel);
			else
				_owner->_selCPs.erase(_owner->_selCPs.begin()+i);

			_owner->_RefreshSelBuf();
			return FALSE;
		}

		return TRUE;
	}

private:
	IMesh * _mesh;
	IMtrl * _mtrl;
	ILight * _light;
	CGuiAgent_KeyPoints * _owner;
};

//////////////////////////////////////////////////////////////////////////
class CGuiAgent_KeyPointOp :public CGuiAgent
{

public:

	CGuiAgent_KeyPointOp(CGuiAgent_KeyPoints * owner)
	{
		_owner = owner;
	}

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag)
	{
		if (!_owner->_cpPack)
			return TRUE;

		_AddMenuSep();

		size_t nSel = _owner-> _selCPs.size();
		DWORD nKey = _owner->_cpPack->GetNumberOfCP();
		if(nSel>0&&(nKey - nSel>2))
			_AddMenu("删除点",ID_AGENT_PTDELETE);

		if(nSel==1){
			DWORD idx = (DWORD)_owner->_selCPs[0];
			if(idx!=0)
				_AddMenu("之前插入点",ID_AGENT_PTNEWPRE);
			if(idx<nKey-1)
				_AddMenu("之后插入点",ID_AGENT_PTNEWBACK);

			if(idx>1&&idx<nKey-2)
				_AddMenu("分裂",ID_AGENT_PTOBJSPLIT);

		}	

		if(_owner->_hObjEdit!=INVALID_HMAPOBJ){
			_AddMenu("反向",ID_AGENT_PTOBJINVERT);
		}

		return TRUE;
	}

	virtual BOOL OnCommand(DWORD idCmd)
	{
		if (_owner->_cpPack)
			return TRUE;
		std::vector<HMapObj> hObjsMod;
		DWORD nKey = _owner->_cpPack->GetNumberOfCP();
		
		BOOL bKeyPointChange = FALSE;
		switch(idCmd) {
			case ID_AGENT_PTDELETE:
				{
					std::vector<DWORD> idx;
					for(int i = 0;i<_owner->_selCPs.size();i++)
						idx.push_back(DWORD(_owner->_selCPs[i]));
					_owner->_cpPack->Remove(&(idx[0]),idx.size());
					bKeyPointChange = TRUE;
					break;
				}
			case ID_AGENT_PTNEWPRE:
				{
					DWORD idx = (DWORD)_owner->_selCPs[0];	
					DWORD sz = _owner->_cpPack->GetNumberOfCP();
					if(sz>=2){
						if(idx>0){
							CtrlPoint *p0 = _owner->_cpPack->At(idx);
							CtrlPoint *p1 = _owner->_cpPack->At(idx-1);
							CtrlPoint *pN = _owner->_cpPack->NewCP();
							pN->Lerp(p0,p1,0.5f);
							_owner->_cpPack->Insert(idx,pN);
							bKeyPointChange = TRUE;
						}
					}
					break;
				}
			case ID_AGENT_PTNEWBACK:
				{
					DWORD idx = (DWORD)_owner->_selCPs[0];					
					size_t sz = _owner->_cpPack->GetNumberOfCP();
					if(sz>=2){
						if(idx<sz-1){
							CtrlPoint *p0 = _owner->_cpPack->At(idx);
							CtrlPoint *p1 = _owner->_cpPack->At(idx+1);
							CtrlPoint *pN = _owner->_cpPack->NewCP();
							pN->Lerp(p0,p1,0.5f);
							_owner->_cpPack->Insert(idx+1,pN);
							bKeyPointChange = TRUE;
						}
					}

					break;
				}
			case ID_AGENT_PTOBJSPLIT: //分裂
				{	
					DWORD idx = (DWORD)_owner->_selCPs[0];					
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
			case ID_AGENT_PTOBJINVERT:
				{
					size_t i = 0;
					size_t j = _owner->_cpPack->GetNumberOfCP()-1;
					CtrlPoint * p0,*p1;
					while(i<j){
						p0 = _owner->_cpPack->At(i);
						p1 = _owner->_cpPack->At(j);
						_owner->_cpPack->Set(i,p1);
						_owner->_cpPack->Set(j,p0);
						i++;
						j--;
					}
					bKeyPointChange = TRUE;
				}
				default: break;
		}
		
		//当前对象节点发生了修改
		if(bKeyPointChange){
			HMapObj hObjNew = _owner->_SetKeyPos(_owner->_hObjEdit,_owner->_cpPack);
			hObjsMod.push_back(_owner->_hObjEdit);
			if(hObjNew!=_owner->_hObjEdit)
				hObjsMod.push_back(hObjNew);
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
	CGuiAgent_KeyPoints * _owner;
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

	CGuiAgent_KeyPointCreate(CGuiAgent_KeyPoints * owner);
	BOOL OnRButtonClick(int x,int y,DWORD flag);
	BOOL OnMouseMove(int x,int y,DWORD flag);
	BOOL OnDraw();
	BOOL _HitGroundPos(int x,int y,i_math::vector3df &pos);
	BOOL OnLButtonDown(int x,int y,DWORD flag);
	BOOL OnCommand(DWORD idCmd);
	void _GetLimitedPos(i_math::vector3df & cur);
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

private:
	ICtrlPointPack * _pack2create;
	CGuiAgent_KeyPoints * _owner;
	State _PTCState;
	i_math::vector3df _ptmove;
	
	i_math::vector3df _posStart;
	i_math::vector3df _posEnd;
	HMapObj _hObjCon2;
	DWORD _count;

	std::vector<i_math::vector3df> _linesRange;
};

CGuiAgent_KeyPointCreate::CGuiAgent_KeyPointCreate(CGuiAgent_KeyPoints * owner)
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
	CGuiAgent::OnDetachView(view,iLevel);
}
void CGuiAgent_KeyPointCreate::_GetLimitedPos(i_math::vector3df & cur)
{
	IObjMapEditor * editor = _owner->_GetEditor();
	if(!editor)
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

	if (!_owner->_cpPack)
		return TRUE;

	if(_PTCState==PTC_IDLE){
		_AddMenuSep();
		_AddMenu("新建",ID_AGENT_PTOBJNEW);
		if(_owner->_hObjEdit!=INVALID_HMAPOBJ){
			_AddMenu("向前增补",ID_AGENT_PTOBJAPPPRE);
			_AddMenu("向后增补",ID_AGENT_PTOBJAPPBACK);
			_AddMenu("连接",ID_AGENT_PTOBJCONNECT);
		}
	}
	else{
		std::vector<HMapObj> hObjsMod;
		switch(_PTCState)
		{
		case PTC_CREATE:
			{
				HMapObj hObj = _owner->_NewObj(_owner->_GetEditor(),_pack2create);
				_owner->_hObjEdit = hObj;
				hObjsMod.push_back(hObj);
				break;
			}
		case PTC_APPFRONT:
		case PTC_APPBACK:
			{
				HMapObj hObj = _owner->_SetKeyPos(_owner->_hObjEdit,_pack2create);
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

		return FALSE;
	}
	return TRUE;
}

BOOL CGuiAgent_KeyPointCreate::OnDraw()
{	
	IRenderPort * rp = GetRP();

	if(_PTCState!=PTC_IDLE){
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
			if(_PTCState==PTC_APPFRONT){
				lines.push_back(_pack2create->At(0)->pos);
				lines.push_back(_ptmove);
			}
			else{
				lines.push_back(_pack2create->Back()->pos);
				lines.push_back(_ptmove);
			}
		}

		rp->Lines(&(lines[0]),lines.size()/2,0xff00ff00);
	}

	if(_PTCState==PTC_CON){
		DWORD col[] = {0xffffff00,0xff00ff00};
		DWORD colSwitch = col[1];
		if(_hObjCon2!=INVALID_HMAPOBJ)
			colSwitch = col[(_count++/5)%2];
		rp->Line(_posStart,_posEnd,colSwitch);
	}
	
	if(_PTCState!=PTC_IDLE&&!_linesRange.empty())
		rp->Lines(&_linesRange[0],_linesRange.size()/2,0xffffff00);

	return TRUE;
}

BOOL CGuiAgent_KeyPointCreate::OnMouseMove(int x,int y,DWORD flag)
{
	IRenderPort * rp = GetRP();
	HitProbe hitProbe;
	rp->CalcHitProbe(x,y,hitProbe);
	i_math::vector3df pos;

	if(_PTCState==PTC_CON){
		_hObjCon2 = _owner->_HitTest(hitProbe);
		if(_hObjCon2!=_owner->_hObjEdit&&_hObjCon2!=INVALID_HMAPOBJ){
			ICtrlPointPack * pack_tmp = _owner->_NewCtrlPointPack();
			_owner->_GetKeyPos(_hObjCon2,pack_tmp);
			_posEnd = pack_tmp->Back()->pos;
			pack_tmp->DeleteMe();
		}
		else{
			_hObjCon2 = INVALID_HMAPOBJ;
			if(_owner->_HitGroundPos(hitProbe,pos))		
				_posEnd = pos;
		}
	}
	else if(_PTCState!=PTC_IDLE){
		if(_owner->_HitGroundPos(hitProbe,pos)){
			_GetLimitedPos(pos);
			_ptmove = pos;
			_ptmove.y += 0.15f;
		}
	}
	return TRUE;
}

BOOL CGuiAgent_KeyPointCreate::_HitGroundPos(int x,int y,i_math::vector3df &pos)
{	
	if (!_owner->_cpPack)
		return FALSE;
	int sel = _owner->_HitSel(x,y); //优先选中节点 方便与连接到已有的节点
	if(sel>=0){
		pos = _owner->_cpPack->At(sel)->pos;
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
	if (!_owner->_cpPack)
		return TRUE;
	if(_PTCState==PTC_CON){
		std::vector<HMapObj> hObjsMod;
		if(_hObjCon2!=INVALID_HMAPOBJ){
			ICtrlPointPack * pack_con = _owner->_NewCtrlPointPack();
			_owner->_GetKeyPos(_hObjCon2,pack_con);
			for(int i = 0;i<_owner->_cpPack->GetNumberOfCP();i++){
				pack_con->Push(_owner->_cpPack->At(i));
			}
			_owner->_cpPack->DeleteMe();
			_owner->_cpPack = pack_con;
			
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
	}
	else if(_PTCState!=PTC_IDLE){
		i_math::vector3df pos;			
		if(_HitGroundPos(x,y,pos)){
			CtrlPoint * p = _owner->_New(_ptmove);
			if(_PTCState==PTC_APPFRONT)
				_pack2create->Insert(0,p);
			else
				_pack2create->Push(p);
			_pack2create->DeleteCP(p);
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
				_hObjCon2 = INVALID_HMAPOBJ;
				break;
			}
		default :break;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
CGuiAgent_KeyPoints::CGuiAgent_KeyPoints(void)
:CGuiAgent_3DNodeMatEdit(EditMode_Move)
{
	_hObjEdit = INVALID_HMAPOBJ;
	_cpPack = NULL;
}

CGuiAgent_KeyPoints::~CGuiAgent_KeyPoints(void)
{
}

void * CGuiAgent_KeyPoints::_GetSelBuf()
{
	_UpdateBufCP();
	return &_selCPs;
}

void CGuiAgent_KeyPoints::_UpdateBufCP(void)
{
	HMapObj hObjSel = _GetSelObj();
	if(hObjSel!=_hObjEdit){
		_hObjEdit = hObjSel;
		_selCPs.clear();
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
			for(int i =0;i<_selCPs.size();i++)
				if(_selCPs[i]<pack_now->GetNumberOfCP())
					temp.push_back(_selCPs[i]);
			if(temp.size()!=_selCPs.size())
				temp.swap(_selCPs);
		}
		else{
			pack_now->DeleteMe();
		}
	}

	//将内部的选择状态更新到外部
	_RefreshSelBuf();
}

i_math::matrix43f * CGuiAgent_KeyPoints::_GetMat(H3DNode node)
{
	DWORD idx = DWORD(node);
	CtrlPoint * p = _cpPack->At(idx);
	if(p){
		_matTemp.setTranslation(_cpPack->At(idx)->pos);
		return &_matTemp;
	}
	return NULL;
}

BOOL CGuiAgent_KeyPoints::_GetObjBlock(const HMapObj &hObj,i_math::pos2di &ptBlk)
{
	IObjMapEditor *editor = _GetEditor();
	if(!editor)
		return FALSE;
	return editor->GetMapFileBlk(hObj,ptBlk);
}

i_math::pos2di * CGuiAgent_KeyPoints::_GetBlock(H3DNode node)
{
	if(_GetObjBlock(_hObjEdit,_ptBlk))
		return &_ptBlk;
	return NULL;
}

void CGuiAgent_KeyPoints::_Move(H3DNode &node,i_math::matrix43f &mat)
{
	HMapObj hObjNew = _Move(_hObjEdit,DWORD(node),mat);
	
	DWORD idx = DWORD(node);
	CtrlPoint * p = _cpPack->At(idx);
	if(p)
		p->pos = mat.getTranslation();

	if(hObjNew!=_hObjEdit)
		_hObjEdit = hObjNew;
}

CtrlPoint * CGuiAgent_KeyPoints::_New(i_math::vector3df &pos)
{
	CtrlPoint * p = _cpPack->NewCP();
	p->pos = pos;
	_OnNewCP(p);
	return p;
}

int CGuiAgent_KeyPoints::_HitSel(int x,int y)
{
	IRenderPort * rp = GetRP();
	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);

	float mind = 9999999.0f;
	int sel = -1;
	i_math::vector3df pos,posProj;

	for(int i = 0;i<_cpPack->GetNumberOfCP();i++){
		pos = _cpPack->At(i)->pos;
		probe.getProjectionPoint(pos,posProj);
		float dSQ = (float)(posProj - pos).getLengthSQ();
		if(dSQ <0.04){
			if(dSQ<mind){
				sel = i;
				mind = dSQ;
			}
		}
	}
	return sel;
}

void CGuiAgent_KeyPoints::_RefreshSelBuf()
{
	std::vector<DWORD> * buf = (std::vector<DWORD> *)_GetSelPointsBuf();
	if(buf){
		buf->resize(_selCPs.size());
		for(int i = 0;i<_selCPs.size();i++)
			(*buf)[i] = DWORD(_selCPs[i]);
	}
}

HMapObj CGuiAgent_KeyPoints::_HitTest(i_math::line3df &ray)
{
	HMapObj hObj = INVALID_HMAPOBJ;

	IObjMapEditor * editor = _GetEditor();
	if(editor)
		hObj = editor->HitTest(ray);
	
	return hObj;
}

BOOL CGuiAgent_KeyPoints::Respond(CtrlOp &co)
{
	if (!_cpPack)
		return TRUE;
	return CGuiAgent_3DNodeMatEdit::Respond(co);
}


void CGuiAgent_KeyPoints::OnAttachView(CGeView *view,DWORD iLevel)
{
	if(_GetEditor()){
		view->AddAgent(_iLevelInView,new CGuiAgent_KeySelDraw(this),_priority);
		view->AddAgent(_iLevelInView,new CGuiAgent_KeyPointOp(this),_priority);
		view->AddAgent(_iLevelInView,new CGuiAgent_KeyPointCreate(this),_priority);
	}
	
	CGuiAgent_3DNodeMatEdit::OnAttachView(view,iLevel);

	_cpPack = _NewCtrlPointPack();
}

void CGuiAgent_KeyPoints::OnDetachView(CGeView *view,DWORD iLevel)
{
	if(_cpPack)
		_cpPack->DeleteMe();
	CGuiAgent_3DNodeMatEdit::OnDetachView(view,iLevel);
}







