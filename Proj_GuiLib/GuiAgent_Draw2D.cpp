#include "stdh.h"

#include "RenderSystem/ITools.h"

#include ".\guiagent_draw2d.h"
#include "graphicsgraph.h"
#include "GuiData.h"
#include "GuiData_OverallMap.h"
#include "FileSystem/IMapFile.h"
#include "RenderSystem/IRenderSystem.h"

#pragma warning(disable:4244)

#define ID_MENU_GOHERE  1934

CGuiAgent_Draw2D::CGuiAgent_Draw2D(void)
{
	_modeSelcect = Select_Multi;
}

CGuiAgent_Draw2D::~CGuiAgent_Draw2D(void)
{
}
BOOL CGuiAgent_Draw2D::OnLButtonDown(int x,int y,DWORD flag)
{
	//备份初始的选中状态
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	if(dataMap)
		_oldSels.assign(dataMap->fldSels.begin(),dataMap->fldSels.end());

	_SelectFld(x,y,flag);
	
	CGuiAgent_Dragger<1,FALSE>::OnLButtonDown(x,y,flag);
    
	return TRUE;
}
void CGuiAgent_Draw2D::_SelectFld(int x,int y,DWORD flag,i_math::pos2di* fldSels/* = NULL*/)
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	GuiData_System * data = (GuiData_System *)FindData("system");

	IMapFile * pMF = data->mf;
	i_math::recti rcFld = pMF->GetFieldRect();
	i_math::recti rcMap = pMF->GetRect();

	const int lenGrid = (rcMap.getWidth()/rcFld.getWidth())*BLOCK_LENGTH*dataMap->scale;

	i_math::recti rc;
	int w,h;
	_GetFeildRc(rc,w,h);
	
	if((!fldSels&&!(flag&CtrlOpFlag_CtrlDown))&&(!(flag&CtrlOpFlag_ShiftDown))||(!fldSels&&_modeSelcect==Select_Sing))
		dataMap->fldSels.clear();

	i_math::pos2di orgFld = rcFld.UpperLeftCorner;

	if(rc.isPointInside(x,y))
	{
		i_math::pos2di pos;
		pos.x = (x - rc.UpperLeftCorner.x)/lenGrid + orgFld.x;
		pos.y = (h - 1 -(y - rc.UpperLeftCorner.y)/lenGrid) + orgFld.y;
		
		int k = 0;
		for(;k<dataMap->fldSels.size()&&pos!=dataMap->fldSels[k];k++);
		
		if(!fldSels&&k>=dataMap->fldSels.size())
			dataMap->fldSels.push_back(pos);
		
		if(fldSels)
			fldSels->set(pos.x,pos.y);
	}	
	
	if(_modeSelcect==Select_Sing&&dataMap->fldSels.size()>1)
		dataMap->fldSels.resize(1);

	_Redraw();
}

// get screen rectange by wolrd filed rc.
void CGuiAgent_Draw2D::_GetFeildRc(i_math::recti &rc,int &w,int &h)
{

	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	GuiData_System * data = (GuiData_System *)FindData("system");

	IMapFile * pMF = data->mf;
	i_math::recti rcFld = pMF->GetFieldRect();
	i_math::recti rcMap = pMF->GetRect();

	w = rcFld.getWidth();
	h = rcFld.getHeight();	

	const int lenGrid = (rcMap.getWidth()/rcFld.getWidth())*BLOCK_LENGTH;

	rcFld.UpperLeftCorner *= lenGrid;
	rcFld.LowerRightCorner *= lenGrid;

	// world space
	rcFld.LowerRightCorner.x *= dataMap->scale;
	rcFld.LowerRightCorner.y *= - dataMap->scale;
	
	rcFld.UpperLeftCorner.x *= dataMap->scale;
	rcFld.UpperLeftCorner.y *= - dataMap->scale;

	//
	int tp = rcFld.LowerRightCorner.y;
	rcFld.LowerRightCorner.y = rcFld.UpperLeftCorner.y;
	rcFld.UpperLeftCorner.y = tp;


	rcFld.LowerRightCorner.x += dataMap->ptOff.x;
	rcFld.LowerRightCorner.y += dataMap->ptOff.y;
	rcFld.UpperLeftCorner.x += dataMap->ptOff.x;
	rcFld.UpperLeftCorner.y += dataMap->ptOff.y;

	rc = rcFld;
}

BOOL CGuiAgent_Draw2D::OnDraw()
{
	GraphicsGraph * gg = GetGG();
	GuiData_System * data = (GuiData_System *) FindData("system");
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");

	IMapFile * pMF = data->mf;

	if(!gg)
		return TRUE;


	i_math::recti & rcMap = pMF->GetRect();
	i_math::recti & rcFld = pMF->GetFieldRect();
	

	int w = rcFld.getWidth();
	int h = rcFld.getHeight();

	const int lenGrid = (rcMap.getWidth()/rcFld.getWidth())*BLOCK_LENGTH;
	const int lenInterval = lenGrid*0.02f;

	i_math::recti rcArea;
	_GetClientRect(rcArea);
	
	char strName[256];

	// Draw Filed Rectangle
	for(int i = rcFld.Left();i<rcFld.Right();i++){
		int x = i*lenGrid;
		for(int j = rcFld.Top();j<rcFld.Bottom();j++)
		{
			i_math::recti rc;
			const int l = lenInterval/2;
			rc.UpperLeftCorner.x = x + l;
			rc.UpperLeftCorner.y = j*lenGrid + l;

			rc.LowerRightCorner.x = rc.UpperLeftCorner.x + lenGrid - l;
			rc.LowerRightCorner.y = rc.UpperLeftCorner.y + lenGrid - l;

			i_math::pos2di ptFld(i,j);
			if(pMF->FieldCheckedOut(ptFld)){//只有CheckOut并且没有简略图的时候才去绘制
				if(!dataMap->bDrawImage){
					DWORD w,h;
					if ((!dataMap->GetFieldRawMiniMap(ptFld,w,h))&&
						(!dataMap->GetFieldOutlineMap(ptFld,w,h)))
						gg->FillSolidRect(rc,0x00ffff,150);
				}
			}
			else
				gg->FillSolidRect(rc,0xff0000,150);

			sprintf(strName,"[%d,%d]",i,j);
			gg->DrawText(strName,rc,DT_CENTER,TRUE);
		}
	}
	
	//绘制栅格线
	if(TRUE){
		int x0 = rcFld.UpperLeftCorner.x*lenGrid;
		int y0 = rcFld.UpperLeftCorner.y*lenGrid;
		int x1 = rcFld.LowerRightCorner.x*lenGrid;
		int y1 = rcFld.LowerRightCorner.y*lenGrid;

		i_math::pos2di p0,p1;
		Pen pen(Color(255,255,255),1.0f);

		float dashVals[] = {2.0f,2.0f};
		pen.SetDashPattern(dashVals,2);
		pen.SetDashStyle(DashStyleDashDot);
		Graphics *grp = gg->GetGraphics();

		for(int i = x0;i<=x1;i+=lenGrid){
			p0.set(i,y0);
			p1.set(i,y1);
			gg->DrawLine(0xff000000,1,p0,p1);
			grp->DrawLine(&pen,PointF(p0.x,p0.y),PointF(p1.x,p1.y));
		}
		for(int i = y0;i<y1;i+=lenGrid){
			p0.set(x0,i);
			p1.set(x1,i);
			gg->DrawLine(0xff000000,1,p0,p1);
			grp->DrawLine(&pen,PointF(p0.x,p0.y),PointF(p1.x,p1.y));
		}
	}

	// Draw selected rectangle frame.
	if(TRUE)
	{
		for(int i = 0;i<dataMap->fldSels.size();i++)
		{
			int bx = dataMap->fldSels[i].x;
			int by = dataMap->fldSels[i].y;

			const int l = 2*lenInterval;
		
			i_math::recti rc;
			rc.UpperLeftCorner.x = bx*lenGrid + l;
			rc.UpperLeftCorner.y = by*lenGrid + l;

			rc.LowerRightCorner.x = rc.UpperLeftCorner.x + lenGrid - l*2;
			rc.LowerRightCorner.y = rc.UpperLeftCorner.y + lenGrid - l*2;
			
			gg->DrawFrameRect(rc,0x00ff00,l);
		}
	}
	
	if(_bInDrag)
		gg->DrawFrameRect(_rcSel,0x0000ff,1,255);
	
	if(TRUE)
	{
		_UpdateWorldCenter();
		gg->FillTri(_dirWalk,0x0000ff,100);
		sprintf(strName,"  %d,%d",_worldCenter.x,_worldCenter.y);
		i_math::recti rc;
		rc.UpperLeftCorner.x = _worldCenter.x;
		rc.UpperLeftCorner.y = _worldCenter.y;
		rc.LowerRightCorner.x = _worldCenter.x + 100;
		rc.LowerRightCorner.y = _worldCenter.y + 20;
		gg->DrawText(strName,rc,DT_LEFT,TRUE);
		
		rc.UpperLeftCorner.x = _worldCenter.x - 10;
		rc.UpperLeftCorner.y = _worldCenter.y - 10;	
		rc.LowerRightCorner.x = _worldCenter.x + 10;
		rc.LowerRightCorner.y = _worldCenter.y + 10;

		gg->GradientPie(rc,0x00ff00,0xffff00);
	}

	return TRUE;
}
BOOL CGuiAgent_Draw2D::OnBeginDrag(int x,int y,DWORD flag)
{
	_x = x;
	_y = y;
	return TRUE;
}
void CGuiAgent_Draw2D::OnEndDrag(int x,int y,DWORD flag)
{
	_UpdateSel(x,y,flag);
}
void CGuiAgent_Draw2D::_UpdateSel(int x,int y,DWORD flag)
{
	if(_modeSelcect == Select_Sing)
		return;

	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	
	GuiData_System * data = (GuiData_System *)FindData("system");

	IMapFile * pMF = data->mf;
	i_math::recti rcFld = pMF->GetFieldRect();
	i_math::recti rcMap = pMF->GetRect();
	
	// get grid len in world space.
	const int lenGrid = (rcMap.getWidth()/rcFld.getWidth())*BLOCK_LENGTH*dataMap->scale;

	i_math::pos2di orgFld;
	orgFld = rcFld.UpperLeftCorner;

	if(!dataMap)
		return;

	int x0 = _x,x1 = x,y0 = _y,y1 = y;
	//左选  与 右选的 差异
	
	BOOL bMultiSel = (flag&CtrlOpFlag_CtrlDown)||(flag&CtrlOpFlag_ShiftDown);
	dataMap->fldSels.clear();

	// get select areas.
	int minx = min(x1,x0);
	int miny = min(y1,y0);
	int maxx = max(x1,x0);
	int maxy = max(y1,y0);

	int w,h;
	
	// Get Screen rc.
	_GetFeildRc(rcFld,w,h);

	minx = (minx - rcFld.UpperLeftCorner.x)/lenGrid + orgFld.x;
	miny = (h-1-(miny - rcFld.UpperLeftCorner.y)/lenGrid) + orgFld.y;

	maxx = (maxx - rcFld.UpperLeftCorner.x)/lenGrid + orgFld.x;
	maxy = (h-1-(maxy - rcFld.UpperLeftCorner.y)/lenGrid) + orgFld.y;


	if(minx<orgFld.x) minx = orgFld.x;
	if(maxy<orgFld.y) maxy = orgFld.y;

	if(maxx>=(orgFld.x + w)) maxx = orgFld.x + w -1;
	if(miny>=(orgFld.y + h)) miny = orgFld.y + h -1;
	
	std::list<i_math::pos2di> fldsTemp;
	std::list<i_math::pos2di>::iterator itList;
	if(bMultiSel)
		fldsTemp.assign(_oldSels.begin(),_oldSels.end());

	for(int i = minx;i<=maxx;i++){
		for(int j = maxy;j<=miny;j++)
		{
			i_math::pos2di pos(i,j);
			if(bMultiSel){
				BOOL bSel = FALSE;
				for(itList=fldsTemp.begin();itList!=fldsTemp.end();itList++){ //已经存在将被从列表中删除
					if(pos==(*itList)){
						fldsTemp.erase(itList);
						bSel = TRUE;
						break;
					}
				}
				if(!bSel)
					dataMap->fldSels.push_back(pos);
			}
			else
				dataMap->fldSels.push_back(pos); 
		}
	}

	if(bMultiSel){
		for(itList=fldsTemp.begin();itList!=fldsTemp.end();itList++)
			dataMap->fldSels.push_back(*itList);
	}

	_Redraw();
}
void CGuiAgent_Draw2D::_ToWorld(int &x,int &y)
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	if(dataMap)
	{
		i_math::matrix43f mat;
		mat.setScale(dataMap->scale,- dataMap->scale,1.0f);
		mat.addTranslation(dataMap->ptOff.x,dataMap->ptOff.y,0.0f);
		mat.makeInverse();
		
		i_math::vector3df pos(x,y,0.0f);
		mat.transformVect(pos,pos);
		x = pos.x;
		y = pos.y;
	}
}
BOOL CGuiAgent_Draw2D::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_OverallMap  * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	
	i_math::pos2di pos;

	if(dataMap)
		_SelectFld(x,y,flag,&pos);
	
	int k = 0;
	for(;k<dataMap->fldSels.size()&&pos!=dataMap->fldSels[k];k++);
	
	if(k>=dataMap->fldSels.size())
	{
		dataMap->fldSels.clear();
		dataMap->fldSels.push_back(pos);
		_Redraw();
	}

	_AddMenu("go here",ID_MENU_GOHERE);
	return TRUE;
}
BOOL CGuiAgent_Draw2D::OnCommand(DWORD idCmd)
{
	GuiData_Camera * dataCameras = (GuiData_Camera *)FindData("cameras");
	GuiData_Trrn * dataTrrn = (GuiData_Trrn *)FindData("terrain");

	ICamera * camera = dataCameras->cams[Camera_Perspective];
	
	if(ID_MENU_GOHERE==idCmd)
	{
		ITrrnMapEditor * editor = dataTrrn->GetTrrnMapEditor();
		
		i_math::pos2di pt;
		_GetCursorPos(pt);
		_ToWorld(pt.x,pt.y);
		
		i_math::vector3df posAt,posEye;
		posAt.x = pt.x;
		posAt.z = pt.y;
		
		posEye = posAt;
		posEye.y = 50.0f;
		
		camera->SetPosTarget(posEye,posAt);
	}

	return TRUE;
}
void CGuiAgent_Draw2D::OnDrag(int x,int y,DWORD flag)
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	
	i_math::pos2di p0(_x,_y);
	i_math::pos2di p1(x,y);
	
	_ToWorld(p0.x,p0.y);
	_ToWorld(p1.x,p1.y);

	_rcSel.UpperLeftCorner = p0;
	_rcSel.LowerRightCorner = p1;

	_UpdateSel(x,y,flag);
}
void CGuiAgent_Draw2D::_UpdateWorldCenter()
{
	GuiData_Camera * dataCameras = (GuiData_Camera *)FindData("cameras");
	ICamera * camera = dataCameras->cams[Camera_Perspective];
	i_math::vector3df posEye;
	camera->GetEyePos(posEye);

	_worldCenter.x = posEye.x;
	_worldCenter.y = posEye.z;
	
	float fov;
	camera->GetFov(fov);
	
	i_math::vector3df pos[3];
	pos[1].x = 60*cosf(fov/2.0f);
	pos[2].x = pos[1].x;
	pos[1].z = 60*sinf(fov/2.0f);
	pos[2].z = -pos[1].z;
	
	i_math::vector3df dirEye;
	camera->GetEyeDir(dirEye);

	float a = acos(dirEye.x/sqrtf(dirEye.x*dirEye.x + dirEye.z*dirEye.z));
	if(dirEye.z>0)   a = -a;
	
	i_math::matrix43f mat;
	mat.addRotationY(a);
	mat.addTranslation(posEye);

	mat.transformVect(pos[0],pos[0]);
	mat.transformVect(pos[1],pos[1]);
	mat.transformVect(pos[2],pos[2]);

	_dirWalk[0].x = pos[0].x;
	_dirWalk[0].y = pos[0].z;

	_dirWalk[1].x = pos[1].x;
	_dirWalk[1].y = pos[1].z;

	_dirWalk[2].x = pos[2].x;
	_dirWalk[2].y = pos[2].z;
}

BOOL CGuiAgent_Draw2D::OnTimer(int dt,DWORD flag)
{
	GuiData_Camera * dataCameras = (GuiData_Camera *)FindData("cameras");
	ICamera * camera = dataCameras->cams[Camera_Perspective];
	i_math::vector3df posEye,dirEye;
	camera->GetEyePos(posEye);
	camera->GetEyeDir(dirEye);
	
	if(posEye.x!=_worldCenter.x||posEye.z!=_worldCenter.y||_oldEyeDir!=dirEye)
	{
		_oldEyeDir = dirEye;
		_worldCenter.set(posEye.x,posEye.z);
		_Redraw();
	}
	
	return TRUE;
}




