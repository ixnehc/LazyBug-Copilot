
#pragma  once

#include "ResEditPanel.h"
#include "resdata/AnimData.h"

struct AnimData;

class IAnimCtrl;
class IMatrice43;

//Reps for ResEditPanelState
struct Reps_Anim:public ResEditPanelState
{
	Reps_Anim()
	{
		Zero();
	}
	virtual ~Reps_Anim()
	{

	}
	void Zero()
	{
		iSelAP=iSelEvent=-1;
		tCur=ANIMTICK_INFINITE;
		tEventAdjust=0;
		bAnimRangeDraging=FALSE;
	}
	virtual void SetData(ResData *data);
	virtual void CleanAndDelete();
	virtual void Copy(ResEditPanelState &src);
	BOOL ChangeSelAP(int iSel);

	int iSelAP;
	int iSelEvent;

	BOOL  bAnimRangeDraging;

	AnimTick tCur;
	int tEventAdjust;		//We need this value because that when we are dragging the 
	//anim piece range,the event values are not updated simoutaneously,
	//this value should be added to the event values to adjust them
	//to accurate values.
};

 inline AnimPiece *GetAP(Reps_Anim *state)
{
	if (!state)
		return NULL;
	if (!state->resdata)
		return NULL;
	if (state->iSelAP==-1)
		return NULL;

	return &((AnimData*)(state->resdata))->animpieces[state->iSelAP];
}
