/********************************************************************
	created:	2008/3/4   10:18
	file path:	d:\IxEngine\Common\misc
	author:		cxi
	
	purpose:	shell image description
*********************************************************************/

#include "stdh.h"

#include "shellimage.h"

#include "stringparser/stringparser.h"


BOOL ShellImageDesc::fromString(const char *str)
{
	std::vector<std::string>buf,buf2;
	std::string s=str;
	SplitStringBy("|",s,&buf);
	if (buf.size()<2)
		return FALSE;
	pathTex=buf[0];

	SplitStringBy(",",buf[1],&buf2);
	if (buf2.size()!=4)
		return FALSE;

	rc.Left()=IntFromString(buf2[0].c_str());
	rc.Top()=IntFromString(buf2[1].c_str());
	rc.Right()=IntFromString(buf2[2].c_str());
	rc.Bottom()=IntFromString(buf2[3].c_str());

	bBlending=FALSE;

	if (buf.size()>2)
	{
		if (buf[2]=="1")
			bBlending=TRUE;
	}

	return TRUE;
}

BOOL ShellImageDesc::toString(std::string &str)
{
	FormatString(str,"%s|%d,%d,%d,%d|%d",pathTex.c_str(),
					rc.Left(),rc.Top(),rc.Right(),rc.Bottom(),
					bBlending?1:0);

	return TRUE;
}
