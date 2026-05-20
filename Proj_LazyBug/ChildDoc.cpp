#include "stdh.h"

#include "ChildDoc.h"

#include "WorldSystem/IEntitySystem.h"

#include <assert.h>


/////////////////////////////////////////////////////////////////////////////
// CChildDoc

IMPLEMENT_DYNCREATE(CChildDoc, CDocument)

BEGIN_MESSAGE_MAP(CChildDoc, CDocument)
	//{{AFX_MSG_MAP(CChildDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildDoc construction/destruction

CChildDoc::CChildDoc()
{
	// TODO: add one-time construction code here
}

CChildDoc::~CChildDoc()
{
}

BOOL CChildDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	assert(FALSE);

	return TRUE;
}


BOOL CChildDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
// 	_protoid=*(ProtoID*)lpszPathName;
	return TRUE;
}


// void CChildDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
// {
// 	std::string path=GetProtoFilePath();
// 	if (path=="")
// 		path="<Proto>";
// 	
// 	CDocument::SetPathName(path.c_str(),FALSE);//阻止该path被加入菜单上的最近文档列表
// }
// 
// const char *CChildDoc::GetProtoFilePath()
// {
// 	if (!_lib)
// 		return "";
// 	IProto *proto=_lib->ObtainProto(_protoid);
// 	if (!proto)
// 		return "";
// 	return proto->GetFilePath();
// }