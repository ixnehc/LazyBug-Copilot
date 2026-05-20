
#include "stdh.h"

#include "PinboardEdit.h"
#include ".\pinboardedit.h"

CPinboardEdit::CPinboardEdit(void)
{
	_bLockSet = FALSE;
}

CPinboardEdit::~CPinboardEdit(void)
{
}
//////////////////////////////////////////////////////////////////////////
	
BEGIN_MESSAGE_MAP(CPinboardEdit,CEdit)
	ON_WM_KEYUP()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
float CPinboardEdit::GetFVal()
{
	TCHAR temp[255];
	memset(temp,0,sizeof(temp));	
	GetWindowText(temp,ARRAY_SIZE(temp)-1);
	float value = (float)atof(toMBCS(temp));
	return value;
}

void CPinboardEdit::OnKillFocus(CWnd* pNewWnd)
{
	CPinboardWndBase<CEdit>::OnKillFocus(pNewWnd);
	CString strCur;
	GetWindowText(strCur);
	if(_strContent!=strCur){
		NotifyEndChange();
	}
	_bLockSet = FALSE;
}

void CPinboardEdit::OnSetFocus(CWnd* pOldWnd)
{
	_bLockSet = TRUE;
	GetWindowText(_strContent);
	CPinboardWndBase<CEdit>::OnSetFocus(pOldWnd);
}

void CPinboardEdit::OnSetValue(int value,ChangeState state)
{	
	if(!_bLockSet){
		char temp[255];
		memset(temp,0,sizeof(temp));
		sprintf(temp,"%d",value);
		SetWindowText(fromMBCS(temp));
	}
}

void CPinboardEdit::OnSetValue(float value,ChangeState state)
{
	if(!_bLockSet){
		char temp[255];
		memset(temp,0,sizeof(temp));
		sprintf(temp,"%.2f",value);
		SetWindowText(fromMBCS(temp));
	}
}

void CPinboardEdit::OnEnable(BOOL bOnoff)
{
	CEdit::EnableWindow(bOnoff);
}

void CPinboardEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if(nChar==VK_RETURN)
		NotifyEndChange();

	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CPinboardEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CEdit::OnChar(nChar,nRepCnt,nFlags);
	_validate();
}

void CPinboardEdit::_validate()
{
	CString sTemp;
	GetWindowText(sTemp);
	char temp[500];
	strcpy(temp, toMBCS(sTemp));

	int len = strlen(temp);
	int po = false;
	bool isvalid = true;
	for(int i =0;i<len;i++)
	{
		char * p = temp+i;
		if(isdigit(*p))
			continue;
		else if(*p=='-'&&i==0)
			continue;
		else
		{
			if(IsFloat()&&!po&&*p=='.')
			{
				po = true;
				continue;
			}
			else if(i==len-1)
			{
				*p = '\0';
				isvalid = false;
			}
			else
			{
				char * c = temp;
				for(int j = i;j<len;j++)
					c[j] = c[j+1];

				len--;
				isvalid = false;
			}
		}
	}

	if(!isvalid)
	{
		SetWindowText(fromMBCS(temp));
	}
}

void CPinboardEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetSel(0,-1);
	CEdit::OnLButtonDown(nFlags, point);
}


