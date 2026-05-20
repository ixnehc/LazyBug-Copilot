
/********************************************************************
	created:	2013/6/20 
	author:		cxi
	
	purpose:	对随从的命令
*********************************************************************/
#include "stdh.h"

#include "LevelRtnuCmds.h"

#include "Level.h"


BOOL CLevelRtnuCmds::Fetch(LevelObjID id,LevelRtnuCmd &cmd)
{
	std::unordered_map<LevelObjID,LevelRtnuCmdRT>::iterator it=_cmds.find(id);
	if (it==_cmds.end())
		return FALSE;
	cmd=(LevelRtnuCmd)(*it).second;
	_cmds.erase(it);
	return TRUE;
}

BOOL CLevelRtnuCmds::Fetch(LevelObjID id,LevelRtnuCmdType tp,LevelRtnuCmd &cmd)
{
	std::unordered_map<LevelObjID,LevelRtnuCmdRT>::iterator it=_cmds.find(id);
	if (it==_cmds.end())
		return FALSE;
	if ((*it).second.tp!=tp)
		return FALSE;
	cmd=(LevelRtnuCmd)(*it).second;
	_cmds.erase(it);
	return TRUE;

}


void CLevelRtnuCmds::Add(LevelRtnuCmd &cmd)
{
	if (!_level)
		return;
	LevelRtnuCmdRT cmdRT;
	((LevelRtnuCmd&)cmdRT)=cmd;
	cmdRT.tStart=_level->GetT_();
	_cmds[cmd.id]=cmdRT;
}



