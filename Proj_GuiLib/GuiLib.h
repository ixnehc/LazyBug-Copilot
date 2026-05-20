#pragma once


#ifdef GUILIB_EXPORTS
#define GuiLib_Api __declspec(dllexport)
#else
#ifdef GUILIB_IGNORE_IMPORT
#define GuiLib_Api
#else
#define GuiLib_Api __declspec(dllimport)
#endif
#endif

class IFileSystem;
class IRenderSystem;
class IUtilRS;
class IWorldSystem;
class IEntitySystem;
class IPhysicsSystem;
class CSscSystemWrapper;
class CConfig;
class CCurrentUserRegistry;

struct GuiLibSS
{
	GuiLibSS()
	{
		pFS = NULL;
		pRS = NULL;
		pWS = NULL;
		pES = NULL;
		pUtilRS = NULL;
		pPS = NULL;
		ssc = NULL;
		ssc2 = NULL;
		cfg = NULL;
		reg = NULL;
	}
	IFileSystem *pFS;
	IRenderSystem *pRS;
	IUtilRS *pUtilRS;
	IWorldSystem *pWS;
	IEntitySystem *pES;
	IPhysicsSystem* pPS;
	CSscSystemWrapper *ssc;
	CSscSystemWrapper *ssc2;
	CConfig *cfg;
	CCurrentUserRegistry *reg;
};

extern GuiLibSS GuiLib_Api g_ssGuiLib;
