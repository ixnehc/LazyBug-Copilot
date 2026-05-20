
#ifndef _RichGridParticleFeatureItem_H_
#define _RichGridParticleFeatureItem_H_

#include "GuiLib.h"
#include "WorldSystem/stubparams/param_sys.h"
#include "particleFeatureDialog.h"


class CRichGrid_ParticlePropertyItem : public CXTPPropertyGridItem
{
public:
	CRichGrid_ParticlePropertyItem( CString strCaption );
	
	void Bind( BrokenLineRef *p, const float fMinX, const float fMaxX, const float fMinY, const float fMaxY );
protected:
	virtual void OnInplaceButtonDown( CXTPPropertyGridInplaceButton* pButton );
	CParticleFeatureDialog dlg;
};




#endif
