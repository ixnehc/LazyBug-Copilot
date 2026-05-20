#include "stdh.h"

#include "RenderSystem/ISpeedGrass.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITexture.h"


#include "ModBlockBack.h"

#include "GuiAgent_grassOp.h"

#include "GuiData_vegetable.h"

#include "ModBlockBack.h"

#include "GuiActor_Vegetable.h"

#define GRID_SIZE  1.0f
#define ID_MENU_VEGPAINT    1220
#define ID_MENU_VEGREMOVE   1221
CGuiAgent_GrassOp::CGuiAgent_GrassOp(void)
{
}
CGuiAgent_GrassOp::~CGuiAgent_GrassOp(void)
{ 
}
BOOL CGuiAgent_GrassOp::OnMouseMove(int x,int y,DWORD flag)
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	
	if(!data||(data->op==GuiData_Vegetable::Op_Idle))
		return TRUE;

	IBrushLib * pspgLib = _GetLib();
	if(!pspgLib)
		return TRUE;

	ITrrnMapEditor * trrn = data->GetTrrnEditor();
	if(!trrn)
		return TRUE;

	i_math::vector3df center ,* pBuf = NULL;

	HitProbe hitProbe;
	IRenderPort * rp = GetRP();
	rp->CalcHitProbe(x,y,hitProbe);
	trrn->GetHitPos(hitProbe,TRUE,center);
	
	DWORD count;
	pBuf = trrn->CalcRoundBound(center,data->radius,100,count);
	
	if(pBuf){
		_posbound.resize(count);
		for(int i = 0;i<count;i++){
			_posbound[i] = pBuf[i];
			_posbound[i].y += 0.2f;
		}
	}
	return CGuiAgent_Dragger<TRUE,FALSE>::OnMouseMove(x,y,flag);	
}
BOOL CGuiAgent_GrassOp::OnBeginDrag(int x,int y,DWORD flag)
{
		return TRUE;
}
BOOL CGuiAgent_GrassOp::OnLButtonDown(int x,int y,DWORD flag)
{
	_InitOp(x,y);
	_Op(x,y);
	return CGuiAgent_Dragger<TRUE,FALSE>::OnLButtonDown(x,y,flag);
}
BOOL CGuiAgent_GrassOp::OnLButtonUp(int x,int y,DWORD flag)
{
	_EndOp();
	return CGuiAgent_Dragger<TRUE,FALSE>::OnLButtonUp(x,y,flag);
}
BOOL CGuiAgent_GrassOp::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	CGuiPanel_Vegetable * actor =  (CGuiPanel_Vegetable *)_GetActor();
	assert(actor);

	//结束涂抹操作 将状态置于空闲状态
	DWORD op = data->op;
	if(op==GuiData_Vegetable::Op_Idle){
		_AddMenu("Paint",ID_MENU_VEGPAINT);
		_AddMenu("Remove",ID_MENU_VEGREMOVE);
	}
	else{
		_EndOp();	
		data->op = GuiData_Vegetable::Op_Idle;
		actor->UpdateState();
		return FALSE;
	}
	
	return TRUE;
}
BOOL CGuiAgent_GrassOp::OnCommand(DWORD idCmd)
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	CGuiPanel_Vegetable * actor =  (CGuiPanel_Vegetable *)_GetActor();
	assert(actor);

	switch(idCmd){
		case ID_MENU_VEGPAINT:
			data->op = GuiData_Vegetable::Op_Paint;
			actor->UpdateState();
			break;
		case ID_MENU_VEGREMOVE:
			data->op = GuiData_Vegetable::Op_Remove;
			actor->UpdateState();
			break;
		default:
			break;
	}

	return TRUE;
}
void CGuiAgent_GrassOp::OnDrag(int x,int y,DWORD flag)
{

}
void CGuiAgent_GrassOp::_InitOp(int x,int y)
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");

	if(!data||(data->op==GuiData_Vegetable::Op_Idle))
		return;
	if(data->op == GuiData_Vegetable::Op_Paint)
	{
		IBrushLib *pspgLib = _GetLib();
		if(!pspgLib)
			return;

		BRUID * brIDs = NULL;
		DWORD count = 0;
		pspgLib->Enum(brIDs,count);

		for(DWORD i = 0;i<count;i++){
			if(pspgLib->IsChecked(brIDs[i])){
				const IBrush * br = pspgLib->Get(brIDs[i]);
				ISpg * pSpg = (ISpg *)pspgLib->ObtainRes(br);
				SpgBrushInfo * pBrInfo = (SpgBrushInfo *)pspgLib->GetInfo(brIDs[i]);
				if(pSpg&&pBrInfo){
					float r = pBrInfo->radius;
					float w = pBrInfo->weight;	
					float height = pSpg->GetHeight();
					_util.AddSeed(brIDs[i],r,w,height);
				}
			}
		}

		_util.SetRS(GetRS());
		_util.SetRP(GetRP());
		_util.SetScale(data->scaleMin,data->scaleMax);
		_util.Begin(GRID_SIZE,data->density);
	}
}
void CGuiAgent_GrassOp::_Op(int x,int y)
{
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");

	if(!data||(data->op==GuiData_Vegetable::Op_Idle))
		return;

	ITrrnMapEditor * trrn = data->GetTrrnEditor();
	if(!trrn)
		return;

	ISpgEditor * editor = data->GetEditor();
	if(!editor)
		return;

	IBrushLib * pspgLib = _GetLib();
	if(!pspgLib)
		return;

	HitProbe hitProbe;
	i_math::vector3df center;
	IRenderPort * rp = GetRP();
	rp->CalcHitProbe(x,y,hitProbe);
	extern BOOL FindStaticResidePos(i_math::line3df &line,i_math::vector3df &posHit,BOOL bGroundOnly);

	if (!FindStaticResidePos(hitProbe,center,TRUE))
		return;
	i_math::pos2df org(center.x,center.z);

	if(data->op == GuiData_Vegetable::Op_Paint)
	{
		DWORD nSz = 0;
		Spg * pSpgs = NULL;

		nSz = _util.Place(pSpgs,org,data->radius);	

		for(int i = 0;i<nSz;i++)
		{
			Spg & pSpg = pSpgs[i];
			trrn->GetHitPos(pSpg.info.pos.x,pSpg.info.pos.z,TRUE,pSpg.info.pos);
			// record the block modified
			HMapObj hObj = editor->Add(pSpg.ID,pSpg.info);
			if(hObj!=INVALID_HMAPOBJ){
				i_math::pos2di blk;
				editor->GetMapFileBlk(hObj,blk);
				_blkMods.insert(Blk(blk));
			}
		}
	}

	else if(data->op == GuiData_Vegetable::Op_Remove){
		_RemovOp(editor,org,data->radius);
	}

}
void CGuiAgent_GrassOp::_EndOp()
{
	_util.End();
	
	if(0==_blkMods.size())
		return;
	
	ISpgEditor * editor = NULL;
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(data)
		editor = data->GetEditor();
	
	if(NULL == editor)
		return;

	CModManager * modMgr = _GetModMgr();
	if(modMgr){
		
		std::vector<Blk> blks;
		blks.assign(_blkMods.begin(),_blkMods.end());

		CModBlockBack * mod = new CModBlockBack(GetView());
		mod->BackupBlocks(&(blks[0]),blks.size());
		editor->Save();	
		Mod_New(modMgr,(CModBase *&)(mod));
	}
	_blkMods.clear();
}
void CGuiAgent_GrassOp::OnEndDrag(int x,int y,DWORD flag)
{
	// do nothing
}
void CGuiAgent_GrassOp::_RemovOp(ISpgEditor * editor,i_math::pos2df& c,float radius)
{
	DWORD count = 0;
	HMapObj * hObjs = editor->Remove(c,radius,count);
	
	//记下改变的地块
	for(int i = 0;i<count;i++){
		i_math::pos2di ptBlk;
		editor->GetMapFileBlk(hObjs[i],ptBlk);
		_blkMods.insert(Blk(ptBlk));
	}
}
BOOL CGuiAgent_GrassOp::OnTimer(int dt,DWORD flag)
{
	if(_bInDrag)
	{
		POINT pt;
		GetCursorPos(&pt);
		CWnd * pWnd = GetWnd();
		if(pWnd){
			ScreenToClient(pWnd->GetSafeHwnd(),&pt);
			_Op(pt.x,pt.y);
		}
	}
	
	return TRUE;
}
BOOL CGuiAgent_GrassOp::OnKeyDown(char c,DWORD flag)
{
	_Redraw();
	return TRUE;
}

BOOL CGuiAgent_GrassOp::OnDraw()
{	
	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	
	IRenderPort * rp = GetRP();

	if(data&&data->op!=GuiData_Vegetable::Op_Idle)
		rp->Lines(&(_posbound[0]),_posbound.size()/2,0xffff0000);
	
	return TRUE;
}
IBrushLib * CGuiAgent_GrassOp::_GetLib()
{
	IBrushLib * pLib = NULL;

	GuiData_Vegetable * data = (GuiData_Vegetable *)FindData("vegetable");
	if(!data)
		return NULL;

	ISpgEditor * editor = data->GetEditor();
	if(editor)
		pLib = editor->GetSpgLib();

	return pLib;
}


//////////////////////////////////////////////////////////////////////////
BOOL DistributedUtil::Begin(float gridSize,float density)
{
	if(gridSize<=0.0f)
		return FALSE;

	_gridSz = gridSize;
	_density = density/100.0f;
	
	if(0 == _seeds.size())
		return FALSE;

	_ConstructWeightTable();
	_grid.Reset();

	return TRUE;
}

void DistributedUtil::AddSeed(BRUID ID,float radius,float w,float height)
{
	_Seed seed;
	seed.radius = radius;
	seed.w = w;
	seed.ID = ID;
	seed.height = height;
	_seeds.push_back(seed);
}

void DistributedUtil::_InitGrid(int xOrg,int yOrg,int w,i_math::pos2df &center,float radius)
{
	int c = 0;
	i_math::vector2df pos,cen;
	cen.set(center.x,center.y);
	float r = 0;

	for(int i = xOrg;i<xOrg+w;i++)
		for (int j = yOrg;j<yOrg+w;j++) {
			BYTE * p = _grid.Obtain(i,j);
			pos.x = i*_gridSz;
			pos.y = j*_gridSz;
			r = float(pos.getDistanceFrom(cen));
			if(r>radius&&*p==0)			 //将圆形区域外的格子初始化为128 表示该区域不能放置
				*p = 255;
			else
			{
				if(*p==255)
					*p = 0;
			}

			if(r<radius&&(*p==0||*p==255)) 
				c++;
		}

	r = float(c)/float(w*w);
	r++;
}
DWORD DistributedUtil::Place(Spg *& result,i_math::pos2df &center,float radius)
{   
	if(0==_seeds.size())
		return 0;
	
	_spgs.clear();

	int wUnit = int((2*radius+_gridSz-0.01f)/_gridSz);
	
	i_math::pos2df orgUnit,tmp;
	orgUnit.x = center.x - radius;
	orgUnit.y = center.y - radius;
	tmp = orgUnit;
	tmp.scale_signed(_gridSz);

	int orgX = int(tmp.x);
	int orgY = int(tmp.y);
	
	DWORD  sz = 0;
	i_math::pos2df org,pos;
	int idx = 0,iSpg = 0;
	int szSeed  =_seeds.size();
	
	_InitGrid(orgX,orgY,wUnit,center,radius);
	
//	_Dump(orgX-20,orgY-20,wUnit+40);

	Spg spg;
	i_math::pos2df posUnit;
	for(int i = orgX;i<wUnit+orgX;i++)
		for(int j = orgY;j<wUnit+orgY;j++)
		{
			BYTE  w = *(_grid.Obtain(i,j));
			if(w>0) 
				continue;

			posUnit.set(i*_gridSz,
						j*_gridSz);

			pos.x = posUnit.x + _RandomFloat(0,_gridSz);
			pos.y = posUnit.y + _RandomFloat(0,_gridSz);

			if(TRUE)
			{
				i_math::vector2df c0(pos.x,pos.y),c1(center.x,center.y);
				if(c0.getDistanceFrom(c1)>radius)
					continue;
			}

			spg.info.pos.set(pos.x,0,pos.y);
			spg.info.rot = _RandomFloat(0,6.28f);
			spg.info.scale = _RandomFloat(_scaleMin,_scaleMax);
			iSpg = _ChooseSeed();
			_Seed seed = _seeds[iSpg];
	
			//areas ralative
			float sz = (seed.height*spg.info.scale);
//			sz = pow(sz,0.5f);
			
			const float k = 5.0f;
			const float b = 0.9f/(exp(k) - 1.0f);
			const float a = 0.1f - b;
			float randValue = _RandomFloat(0.0f,1.0f);
			spg.info.fLod =  (a + b*exp(k*randValue))*sz;  //the lod ralate to seed size.

			float rSeed = spg.info.scale*seed.radius*(1.0f - _density);
			if(_CheckAndFill(pos,rSeed))
			{
				spg.ID = seed.ID;
				_spgs.push_back(spg);			
			}
		}
	
	sz = _spgs.size();
	if(sz>0)
		result = &(_spgs[0]);

//	_Dump(orgX-20,orgY-20,wUnit+40);

	return sz;
}
void  DistributedUtil::End()
{
	_grid.Reset();
	_seeds.clear();
}
void DistributedUtil::_Dump(int xOrg,int yOrg,int w)
{
	ITexture * tex = _pRS->GetWTexMgr2()->Create(w,w,D3DFMT_A8R8G8B8);
	DWORD pitch = 0;

	BYTE * p = (BYTE *)tex->Lock(pitch,TexLock_WriteOnly);
	
	for(int i = 0;i<w;i++)
		for (int j = 0;j<w;j++) {
			BYTE col = *(_grid.Obtain(i+xOrg,j+yOrg));
			p[0] = col;
			p[1] = col;
			p[2] = col;
			p[3] = col;
			p += 4;
		}

	tex->UnLock();

//	DrawTextureArg arg;
//	arg.bDestRect = TRUE;
//	arg.SetDest(0,0,200,200);
//	_pRP->DrawTexture(tex,arg);

	tex->DumpTga("d:\\test.tga");

	SAFE_RELEASE(tex);
}
BOOL DistributedUtil::_CheckAndFill(i_math::pos2df & center,float rSeed)
{
	i_math::pos2df p0,p1;
	p0.x = center.x - rSeed;
	p0.y = center.y - rSeed;

	p1.x = center.x + rSeed;
	p1.y = center.y + rSeed;

	p0.scale_signed(_gridSz);
	p1.scale_signed(_gridSz);

	int x0 = int(p0.x);
	int x1 = int(p1.x);

	int y0 = int(p0.y);
	int y1 = int(p1.y);

	for(int i = x0;i<x1;i++)
		for(int j = y0;j<y1;j++){
			BYTE * p = _grid.Obtain(i,j);
			BYTE w = _CalWeight(i,j,center,rSeed);//在该方格内占据的权重
			if(w>0){
				int r = *p + w;
				*p = (r>120)?120:int(r);//累加占据权重
			}
		}

	return TRUE;
}
BYTE DistributedUtil::_CalWeight(int x,int y,i_math::pos2df & center,float r)
{
	i_math::vector2df pos[4],c;

	c.x = center.x;
	c.y = center.y;

	pos[0].x = x*_gridSz ;
	pos[0].y = y*_gridSz;

	pos[1].x = (x+1)*_gridSz;
	pos[1].y = y*_gridSz;

	pos[2].x = x*_gridSz;
	pos[2].y = (y+1)*_gridSz;

	pos[3].x = (x+1)*_gridSz;
	pos[3].y = (y+1)*_gridSz;

	BYTE w = 0 ;

	for(int i = 0;i<4;i++)
	{
		float d = float(pos[i].getDistanceFrom(c));
		if(d<=r) 
			w += 30;
	}

	return w;
}

void DistributedUtil::_ConstructWeightTable()
{
	_weightTable.clear();
	int sz = _seeds.size();

	if(sz==0)
		return;

	float total = 0;
	for(int i = 0;i<sz;i++){
		_Seed &seed = _seeds[i];
		total += seed.w;
	}

	_weightTable.resize(sz);

	float weight = 0;
	for(int i = 0;i<sz;i++){
		_Seed &seed = _seeds[i];
		weight += seed.w/total;
		_weightTable[i] = weight;
	}
}
int DistributedUtil::_ChooseSeed()
{
	float w = _RandomFloat(0,1.0f);
	int i = 0;
	for(;i<_weightTable.size()&&w>_weightTable[i];i++);

	return i;
}

int DistributedUtil::_RandomInt(int vmax)
{
	return rand()%vmax;
}
float DistributedUtil::_RandomFloat(float vmin,float vmax)
{
	float  ratio = float(rand())/float(RAND_MAX);
	float v = vmin + (vmax - vmin)*ratio;
	return v;
}




