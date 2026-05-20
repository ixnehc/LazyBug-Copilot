#pragma once

#include "GuiLib.h"

#include "WorldSystem/INavMesh.h"

#include "editor/editor.h"

struct GuiLib_Api GuiData_NavMesh:public GeData
{
	virtual const char *GetName()	{		return "navmesh";	}

	enum OpState
	{
		OpNull,
		OpSetStartPos,
		OpSetEndPos,
		OpBuildNavMesh,
		OpSelectNavMesh
	};

	GuiData_NavMesh()
	{
		Zero();
	}

	~GuiData_NavMesh()
	{
		Clear();
	}

	void Zero()
	{
		pES = NULL;
		opState = OpNull;
	}
	
	void Clear()
	{
		Zero();
	}

	struct NavMeshEditParams
	{
		NavMeshEditParams()
		{
			wBlock = 1; 
			bSel=FALSE;
		}
		BOOL bSel;
		i_math::vector3df centerSel;
		int wBlock;
	};

	INavMeshEditor	* GetEditor();
	INavService		* GetNavService();
	IEntitySystem		*pES;
	NavMeshEditParams	editParams;
	NavMeshBuildParams  buildParams;
	OpState				opState;
};





