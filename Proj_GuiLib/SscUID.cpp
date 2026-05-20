/********************************************************************
	created:	2012/12/23 
	author:		cxi
	
	purpose:	使用Ssc来创建一个32bit的UID
*********************************************************************/
#include "stdh.h"

#include "SscUID.h"

#include "RenderSystem/IRenderSystem.h"
#include "FileSystem/IFileSystem.h"
#include "GuiLib.h"
#include "SscBase.h"

#define SSCUIDFILE_PATH "_editor\\sscuid.dat"

const char *CSscUIDs::_GetPath()
{
	static std::string path;
	if (path=="")
	{
		path=g_ssGuiLib.pRS->GetPath(Path_Res);
		path=path+"\\"+SSCUIDFILE_PATH;
	}
	return path.c_str();
}

ISscSystem *CSscUIDs::_GetSS()
{
	ISscSystem *pSS=NULL;
	if (g_ssGuiLib.ssc)
	{
		pSS=g_ssGuiLib.ssc->GetSS();
		if (pSS)
		{
			if (!pSS->IsConnected())
				pSS = NULL;
		}
	}
	return pSS;

}


BOOL CSscUIDs::BeginGen()
{
	ISscSystem* pSS = _GetSS();
	if (!pSS)
	{
		_bLocked = TRUE;
		return TRUE;
	}

	const char *path=_GetPath();

	BOOL bExist=g_ssGuiLib.pFS->ExistFileAbs(path);

	BOOL bSscOk=FALSE;
	SscState state;
	if (pSS->GetState(path,state))
	{
		if (state==SSC_NOTCHECKEDOUT)
		{
			if (TRUE==pSS->CheckOut(path,128))//Replace
				bSscOk=TRUE;
		}
		if (state==SSC_CHECKEDOUT_ME)
		{
			if (!bExist)
			{
				if (TRUE==pSS->CheckOut(path,128))
					bSscOk=TRUE;
			}
			else
				bSscOk=TRUE;
		}
		if (state==SSC_NOTCONTROLLED)
		{
			if (!bExist)
			{//不存在,新建一个文件
				IFile *fl=g_ssGuiLib.pFS->OpenFileAbs(path,FileAccessMode_Write);
				if (fl)
				{
					DWORD v=32;
					fl->Write(&v,sizeof(v));
					fl->Close();
				}
			}
			if (g_ssGuiLib.pFS->ExistFileAbs(path))
			{
				if (pSS->CheckIn(path,0))
				{
					if (pSS->CheckOut(path,128))
						bSscOk=TRUE;
				}
			}
		}
	}

	if (!bSscOk)
		return FALSE;

	_bLocked=TRUE;

	return TRUE;
}

SscUID CSscUIDs::Gen()
{
	if (!_bLocked)
		return SscUID_Invalid;

	const char *path=_GetPath();

	IFile *fl=g_ssGuiLib.pFS->OpenFileAbs(path,FileAccessMode_Modify);
	if (!fl)
	{
		if(AfxMessageBox(_T("The uid file is read only! Do you want to make it writable and continue?"),MB_OKCANCEL)!=IDOK)
			return SscUID_Invalid;
		else
		{
			g_ssGuiLib.pFS->SetFileAttrAbs(path, File_Default);
			fl = g_ssGuiLib.pFS->OpenFileAbs(path, FileAccessMode_Modify);
			if (!fl)
				return SscUID_Invalid;
		}
	}

	SscUID uid=SscUID_Invalid;
	DWORD v;
	fl->Read(&v,sizeof(v));
	v++;
	uid=v;

	fl->Seek(0);
	fl->Write(&v,sizeof(v));
	fl->Close();

	return uid;
}

void CSscUIDs::EndGen()
{
	ISscSystem *pSS=_GetSS();
	if (!pSS)
		return;
	const char *path=_GetPath();
	pSS->CheckIn(path,0);
}

GuiLib_Api SscUID SscUID_SafeGen()
{
	SscUID uid;
	CSscUIDs uids;
	while(!uids.BeginGen())
		Sleep(0);
	uid=uids.Gen();
	uids.EndGen();
	return uid;
}