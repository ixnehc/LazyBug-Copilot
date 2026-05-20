/********************************************************************
	created:	2017/10/23 
	author:		cxi
	
	purpose:	GA功能:修改Shape
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelOSB.h"

#include "BgnGA_ModShape.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ModShape
BIND_BGN_CLASS(CBgnGA_ModShape,CBgpGA_ModShape);


void CBgnGA_ModShape::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ModShape*pad=_GetPad<CBgpGA_ModShape>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			if (ctx->lo)
			{
				if (IsClass2(ctx->lo,CLoGeneralAgent))
				{
					if (pad->op==CBgpGA_ModShape::Disable)
					{
						if (((CLoGeneralAgent*)ctx->lo)->IsShapeEnabled())
						{
							((CLoGeneralAgent*)ctx->lo)->DisableShape();
							decider->MakeShapeModify(LevelOSB(ctx->lo),ctx->lo,pad->op);
						}
					}
					if (pad->op==CBgpGA_ModShape::Enable)
					{
						if (!((CLoGeneralAgent*)ctx->lo)->IsShapeEnabled())
						{
							((CLoGeneralAgent*)ctx->lo)->EnableShape();
							decider->MakeShapeModify(LevelOSB(ctx->lo),ctx->lo,pad->op);
						}
					}
					if (pad->op==CBgpGA_ModShape::SetName)
					{
						if (((CLoGeneralAgent*)ctx->lo)->SetShapeName(pad->nmShape))
							decider->MakeShapeModify(LevelOSB(ctx->lo),ctx->lo,pad->op,pad->nmShape);
					}
				}
			}
		}
	}
	
	_OutputOk(outputs,1,"结束");
	return;
}
