
#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridParticleFeatureItem.h"


CRichGrid_ParticlePropertyItem::CRichGrid_ParticlePropertyItem( CString strCaption )
		: CXTPPropertyGridItem( strCaption )
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;
}

void CRichGrid_ParticlePropertyItem::Bind( BrokenLineRef *p, 
										 const float fMinX, const float fMaxX, const float fMinY, const float fMaxY )
{
	dlg.BindProperty( p, fMinX, fMaxX, fMinY, fMaxY );
}


void CRichGrid_ParticlePropertyItem::OnInplaceButtonDown( CXTPPropertyGridInplaceButton* pButton )
{
	dlg.LoadData();
	if ( dlg.DoModal() == IDOK )
	{
		bool bBreak = true;
	}
}