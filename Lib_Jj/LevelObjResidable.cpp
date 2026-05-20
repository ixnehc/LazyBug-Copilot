/********************************************************************
	created:	2012/10/30 
	author:		cxi
	
	purpose:	代表一个LevelObj里面的可Reside的位点集合
*********************************************************************/

#include "stdh.h"

#include "LoAgent.h"

#include "LevelObjResidable.h"

////////////////////////////////////////////////////////////////////////
//CLevelObjResidable_Single

void CLevelObjResidable_Single::Init(i_math::matrix43f &mat,LevelPos3D &posSeat)
{
	_bPreserve=0;
	_bOccupy=0;
	mat.transformVect(posSeat,_posSeat);

}


////////////////////////////////////////////////////////////////////////
//CLevelObjResidable_Infinite
void CLevelObjResidable_Infinite::Init(i_math::matrix43f &mat,LevelPos3D &posSeat)
{
	mat.transformVect(posSeat,_posSeat);
}
