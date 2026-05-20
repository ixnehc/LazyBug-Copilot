#include "stdh.h"
#include ".\brushutil.h"

const char * CBrushUtil::GetTypeName()
{
	return "TerrainBrush";
}
DWORD CBrushUtil::GetType()
{
	return TOOL_TERRAINBRUSH;
}
