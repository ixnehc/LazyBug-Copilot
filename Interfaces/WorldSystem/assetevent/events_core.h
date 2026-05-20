/********************************************************************
	created:	2008/1/18   10:54
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	declaration for the events used in worldsystem._dll(core package)
*********************************************************************/
#pragma once

//////////////////////////////////////////////////////////////////////////
//Hook Event

struct HkSetWorldCenter:public AstEvent
{
	i_math::vector3df center;

	DECLARE_EVENT(HkSetWorldCenter);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkSetWorldCenter,1);
		GELEM_VAR_INIT(i_math::vector3df,center,i_math::vector3df(0,0,0));
    END_GOBJ();    

};

struct HkGetWorldCenter:public AstEvent
{
	i_math::vector3df center;
	DECLARE_EVENT(HkGetWorldCenter);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkGetWorldCenter,1);
		GELEM_VAR_INIT(i_math::vector3df,center,i_math::vector3df(0,0,0));
	END_GOBJ();
};


struct HkSaveToMap:public AstEvent
{

	DECLARE_EVENT(HkSaveToMap);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkSaveToMap,1);
    END_GOBJ();    
};

struct HkReloadMap:public AstEvent
{
	BOOL bReloadAll;
	std::vector<i_math::pos2di>blks;

	DECLARE_EVENT(HkReloadMap);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkReloadMap,1);
		GELEM_VAR_INIT(BOOL,bReloadAll,FALSE);
		GELEM_VARVECTOR(i_math::pos2di,blks);
    END_GOBJ();    
};

class ITrrnMap;
struct HkGetTerrain:public AstEvent
{
	ITrrnMap *trrn;

	DECLARE_EVENT(HkGetTerrain);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(HkGetTerrain,1);
		GELEM_VAR_INIT(ITrrnMap*,trrn,NULL);
    END_GOBJ();    
};

class IForestEditor;
struct HkGetForestEditor :public AstEvent
{
	IForestEditor * forestEditor;
	DECLARE_EVENT(HkGetForestEditor);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkGetForestEditor,1);
		GELEM_VAR_INIT(IForestEditor *,forestEditor,NULL);
	END_GOBJ();
};
class IWaterEditor;
struct HKGetWaterEditor :public AstEvent
{
	IWaterEditor * editor;

	DECLARE_EVENT(HKGetWaterEditor);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HKGetWaterEditor,1);
	GELEM_VAR_INIT(IWaterEditor *,editor,NULL);
	END_GOBJ();
};
struct CtrlOp;
struct HkSetCtrlOp:public AstEvent
{
	CtrlOp *op;
	DECLARE_EVENT(HkSetCtrlOp);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkSetCtrlOp,1);
		GELEM_VAR_INIT(CtrlOp *,op,NULL);
	END_GOBJ();
};

class ICamera;
struct HkGetCamera:public AstEvent
{
	ICamera*cam;
	DECLARE_EVENT(HkGetCamera);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkGetCamera,1);
		GELEM_VAR_INIT(ICamera*,cam,NULL);
	END_GOBJ();

};

struct HkGetMapPath:public AstEvent
{
	std::string path;
	DECLARE_EVENT(HkGetMapPath);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkGetMapPath,1);
		GELEM_STRING_INIT(path,"");
	END_GOBJ();
};

struct HkChangeMap:public AstEvent
{
	IMapFile *mf;
	BOOL bBeginOrEnd;//开始change map或者结束change map

	DECLARE_EVENT(HkChangeMap);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkChangeMap,1);
		GELEM_VAR_INIT(IMapFile*,mf,NULL);
		GELEM_VAR_INIT(BOOL,bBeginOrEnd,TRUE);
	END_GOBJ();

};

struct HkPostChangeMap:public AstEvent
{
	IMapFile *mf;

	DECLARE_EVENT(HkPostChangeMap);

	//GObj Defination--------------------------
	BEGIN_GOBJ_PURE(HkPostChangeMap,1);
		GELEM_VAR_INIT(IMapFile*,mf,NULL);
	END_GOBJ();

};



//////////////////////////////////////////////////////////////////////////
//General Events

struct ETest:public AstEvent
{
	int t1;
	std::string t2;

	DECLARE_EVENT(ETest);

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(ETest,1);
		GELEM_VAR_INIT(int,t1,0);
		GELEM_STRING_INIT(t2,"ttt");
    END_GOBJ();    
};



