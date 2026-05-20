#include "stdh.h"
#include ".\sptwindpropgrid.h"

CSptWindPropGrid::CSptWindPropGrid(void)
{
}

CSptWindPropGrid::~CSptWindPropGrid(void)
{
}
void CSptWindPropGrid::BindData(const SptWndCfg * cfg)
{
	ResetContent();
	if(NULL == cfg)
		return;
	
	_cfg = *cfg;
	
	LockPaint();
	BeginInsert();

	InsertCategory("branch","the wind parameter relative to branch.");
		PushInsert();
		InsertCategory("horizon","the wind parameter relative to branch horizon.");
			PushInsert();
			InsertFloatItem("low angle:","The low Horizontal wind angle.",&(_cfg.branchHor.fLowAngle),0,10.0f,0.01f);
			InsertFloatItem("high angle:","The high Horizontal wind angle.",&(_cfg.branchHor.fHiAngle),0.0f,10.0f,0.01f);
			InsertFloatItem("low speed:","The low Horizontal wind speed.",&(_cfg.branchHor.fLowSpeed),0.0f,20.0f,0.01f);
			InsertFloatItem("high speed:","The high Horizontal wind speed.",&(_cfg.branchHor.fHiSpeed),0.0f,20.0f,0.01f);
			PopInsert();
		InsertCategory("vertical","the wind parameter relative to branch vertical.");
			PushInsert();
			InsertFloatItem("low angle:","The low vertical wind angle.",&(_cfg.branchVer.fLowAngle),0.0f,20.0f,0.01f);
			InsertFloatItem("high angle:","The high vertical wind angle.",&(_cfg.branchVer.fHiAngle),0.0f,20.0f,0.01f);
			InsertFloatItem("low speed:","The low vertical wind speed.",&(_cfg.branchVer.fLowSpeed),0.0f,20.0f,0.01f);
			InsertFloatItem("high speed:","The high vertical wind speed.",&(_cfg.branchVer.fHiSpeed),0.0,20.0f,0.01f);
			PopInsert();

//		InsertCategory("exponent","exponent effect how strong winds need to be effect branch");
//			PushInsert();
//			InsertFloatItem("exponent:","exponent.",&(_cfg.fBranchExponents),0.0f,360.0f);
//			PopInsert();		
//		PopInsert();

	InsertCategory("leaf","the wind parameter relative to leaf.");
		PushInsert();
		InsertCategory("rock","the wind parameter relative to leaf rock.");
			PushInsert();
			InsertFloatItem("low angle:","The low rock angle.",&(_cfg.leafRocking.fLowAngle),0.0f,30.0f,0.01f);
			InsertFloatItem("high angle:","The high rock angle.",&(_cfg.leafRocking.fHiAngle),0.0f,30.0f,0.01f);
			InsertFloatItem("low speed:","The low rock speed.",&(_cfg.leafRocking.fLowSpeed),0.0f,20.0f,0.01f);
			InsertFloatItem("high speed:","The high rock speed.",&(_cfg.leafRocking.fHiSpeed),0.0f,20.0f,0.01f);
			PopInsert();
		InsertCategory("rustle","the wind parameter relative to leaf rustle.");
			PushInsert();
			InsertFloatItem("low angle:","The low rustle angle.",&(_cfg.leafRustling.fLowAngle),0.0f,30.0f,0.01f);
			InsertFloatItem("high angle:","The high rustle angle.",&(_cfg.leafRustling.fHiAngle),0.0f,30.0f,0.01f);
			InsertFloatItem("low speed:","The low rustle speed.",&(_cfg.leafRustling.fLowSpeed),0.0f,20.0f,0.01f);
			InsertFloatItem("high speed:","The high rustle speed.",&(_cfg.leafRustling.fHiSpeed),0.0f,20.0f,0.01f);
			PopInsert();

//		InsertCategory("exponent","exponent effect how strong winds need to be effect leaf");
//			PushInsert();
//			InsertFloatItem("exponent:","exponent.",&(_cfg.fBranchExponents),0.0f,360.0f);
//			PopInsert();	
//		PopInsert();

	InsertCategory("global","the wind parameter effect to the whole tree.");
		PushInsert();
		InsertFloatItem("Blend Max","The maximum bend angle for the whole tree in degrees.",&(_cfg.fMaxBend),0.0f,360.0f);
		InsertFloatItem("strength","Strength of the wind in the range ",&(_cfg.fStrength),0.0f,1.0f);
//		InsertIntItem<int>("angle number","Number of uniquely computed leaf angles.",&(_cfg.nLeafAngles),0,MAX_NUM_WINDANGLES);
//		InsertIntItem<int>("matrix number","Number of uniquely computed wind matrices.",&(_cfg.nMatrices),0,MAX_NUM_WINDMATRICES);
		InsertVec3Item("direction","The wind direction.",&(_cfg.direction));

		InsertCategory("gust","the gust effect to the whole tree.");
			PushInsert();
			InsertFloatItem("low strength","The low strength of the gust",&(_cfg.fGustStrengthMin),0.0f,360.0f);
			InsertFloatItem("high strength","The high strength of the gust",&(_cfg.fGustStrengthMax),0.0f,360.0f);
			InsertFloatItem("low duration","The low duration of the gust",&(_cfg.fGustDurationMin),0.0f,360.0f);
			InsertFloatItem("high duration","The high duration of the gust",&(_cfg.fGustDurationMax),0.0f,360.0f);
			InsertFloatItem("frequency","The frequency of the gust",&(_cfg.fGustFrequency),0.0f,360.0f);
			PopInsert();
		InsertCategory("response","the wind response and response limiter for the controller that handles smooth changes from one wind strength/direction to another.");
			PushInsert();
			InsertFloatItem("speed","Response speed of the controller that handles smooth changes.",&(_cfg.fResponse),0.0f,1.0f);
			InsertFloatItem("limiter","Limiter on the speed of the controller.",&(_cfg.fGustStrengthMax),0.0f,1.0f);
			PopInsert();
	PopInsert();

	EndInsert();
	UnLockPaint();
	
	ExpandAll();
}
void CSptWindPropGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	const char * className = "CSptWindPropGrid";
	NotifyEvent(Event_EndChange,0,(void *)className);
}






