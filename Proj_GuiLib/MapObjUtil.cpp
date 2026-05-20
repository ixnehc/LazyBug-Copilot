
#include "stdh.h"

#include "MapObjUtil.h"

void CommitMapObjMod(CModManager * mgrMod,CGeView * view,std::vector<HMapObj>& hObjsMod,IObjMapEditor * editor)
{
	if(hObjsMod.empty())
		return;

	std::vector<pos2di> ptBlks;
	i_math::pos2di ptblk;
	for(int i = 0;i<hObjsMod.size();i++){
		if(editor->GetMapFileBlk(hObjsMod[i],ptblk)){
			ptBlks.push_back(ptblk);
		}
	}

	if(mgrMod){
		CModBlockBack * mod = new CModBlockBack(view);
		mod->BackupBlocks(&(ptBlks[0]),ptBlks.size());
		editor->Save();
		Mod_New(mgrMod,(CModBase *&)(mod));
	}
	else
		editor->Save();
}

void CommitMapObjMod(CModManager * mgrMod,CGeView * view,HMapObj &hObjMod,IObjMapEditor * editor)
{
	std::vector<HMapObj> hObjsMod;
	hObjsMod.push_back(hObjMod);
	CommitMapObjMod(mgrMod,view,hObjsMod,editor);
}