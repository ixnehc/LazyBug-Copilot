
#include "stdh.h"

#include "GuiData_NavMesh.h"

#include "WorldSystem/IEntitySystem.h"

INavMeshEditor * GuiData_NavMesh::GetEditor()
{
	INavMeshEditor * editor = NULL;

	IObjMapEditor *base = pES->FindObjMapEditor(OBJMAP_TYPE_NAVMESH);
	if(base)
		editor  = (INavMeshEditor *)(base);

	return editor;  
}

INavService	* GuiData_NavMesh::GetNavService()
{
	INavService * service = NULL;
	if(pES)
		service = pES->FindNavService();
	return service;
}



