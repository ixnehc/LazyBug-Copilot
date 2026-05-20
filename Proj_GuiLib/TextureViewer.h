#ifndef __TextureViewer_H__
#define __TextureViewer_H__
#include "ResSelectorDefines.h"
#include "RenderSystem/IRenderSystem.h"
#include "ximage.h"
#include "CCompDCWnd.h"

const int TV_INFO_HEIGHT		= 16;
const BYTE TV_TRANSPAENT_ALPHA	= 10;

#define WM_TV_UPDATEINFO	WM_USER + 2000

class CTextureViewer : public CCompDCWnd<CWnd>, public CResourceViewer
{
public:
	CTextureViewer(IRenderSystem* rs);
	virtual ~CTextureViewer();

	enum OpMode
	{
		OP_NONE, 
		OP_MOVE, 
		OP_SELECT
	};

public:
	virtual void SetSelectMode(int mode);
	virtual void SetResource(const char* res);

	virtual BOOL IsCanView(const char* ext) const;

	virtual const RECT& GetSelectedRect() const;
	virtual void SetSelectedRect(RECT &rc);

	virtual void Update();

public:
	virtual void Draw(CDC* pDC);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

public:
	void DrawImageEdge(CDC* pDC);
	void DrawSelection(CDC* pDC);
	void DrawInformation(CDC* pDC);
	BOOL PointSelect(RECT& rcSelection, const POINT& pt);

	BOOL IsTransparent(const POINT& pt);
	
private:
	IRenderSystem* _pRender;
	int _selectMode;
	CxImage _image;	
	CFont _font;	
	OpMode _opMode;
	POINT _ptRef;
	POINT _ptMove;
	RECT _rcSelection;
	CBrush _brSelection;
	CPen _penSelection;
	CPen _penImage;
};
#endif