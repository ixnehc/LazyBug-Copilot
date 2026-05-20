
#include "stdh.h"

#include "assert.h"

#include "LevelObjSrc.h"
#include "LevelClasses.h"

#include "datapacket/DataPacket.h"

#include "LevelRecordAgent.h"

#include "gds/GObj.h"

//////////////////////////////////////////////////////////////////////////
//CLevelSrc

BOOL CLevelObjSrc::IsAttackable()
{
	if (_rec)
		return _rec->bAttackable;
	return FALSE;
}

void CLevelObjSrc::CopyFrom(CLevelObjSrc *src)
{
	GetGObj()->Copy(src->GetGObj());

	_mat=src->_mat;
	_guid=src->_guid;

	_rec=src->_rec;

	_bDisable=src->_bDisable;
}



//////////////////////////////////////////////////////////////////////////
//CLevelSources

void CLevelSources::Clear()
{
	_lookup.clear();
	_bLookUpBuilt=FALSE;
	for (int i=0;i<_buf.size();i++)
	{
		Safe_Class_Delete(_buf[i].src);
		Safe_Class_Delete(_buf[i].param);
	}
	_buf.clear();

}



BOOL CLevelSources::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=_buf.size();

	for (int i=0;i<_buf.size();i++)
	{
		if (TRUE)
		{
			CLevelObjSrc *p=_buf[i].src;
			ClassID clssid=p->GetClass()->GetUID();
			DP_WriteVar(dp,clssid);

			DP_PreSafeSave(dp);
			DP_WriteVar(dp,p->GetMat());
			if (TRUE)
			{
				LevelGUID guid=p->GetGUID();
				DP_WriteVar(dp,guid);
			}
			SaveGObj(dp,p->GetGObj());
			DP_PostSafeSave();
		}

		if (TRUE)
		{
			CLevelObjParam*p=_buf[i].param;
			ClassID clssid=ClassID_Null;
			if (p)
			{
				clssid=p->GetClass()->GetUID();
				assert(clssid!=ClassID_Null);
			}
			DP_WriteVar(dp,clssid);

			if (clssid!=ClassID_Null)
			{
				DP_PreSafeSave(dp);
				SaveGObj(dp,p->GetGObj());
				DP_PostSafeSave();
			}
		}

	}

	return TRUE;
}

BOOL CLevelSources::Load(CDataPacket &dp)
{
	Clear();

	DWORD sz=dp.Data_NextDword();
	_buf.reserve(sz);
	for (int i=0;i<sz;i++)
	{
		ClassID clssid;

		CLevelObjSrc *src=NULL;
		CLevelObjParam *param=NULL;
		LevelGUID guid=0;

		if (TRUE)
		{
			DP_ReadVar(dp,clssid);

			DP_PreSafeLoad(dp);
			CLevelObjSrc *p=NewLevelObjSrc(clssid);
			if (p)
			{
				DP_ReadVar(dp,p->GetMat());
				if (TRUE)
				{
					LevelGUID guid;
					DP_ReadVar(dp,guid);
					p->SetGUID(guid);
				}
				LoadGObj(dp,p->GetGObj(),NULL);

				src=p;
			}

			DP_PostSafeLoad();
		}

		if (TRUE)
		{
			DP_ReadVar(dp,clssid);

			if (clssid!=ClassID_Null)
			{
				DP_PreSafeLoad(dp);
				CLevelObjParam*p=NewLevelObjParam(clssid);
				if (p)
				{
					LoadGObj(dp,p->GetGObj(),NULL);
					param=p;
				}

				DP_PostSafeLoad();
			}
		}

		if (src||param)
		{
			Src s;
			s.src=src;
			s.param=param;
			_buf.push_back(s);
		}
	}


	return TRUE;
}

void CLevelSources::_BuildLookUp()
{
	if (_bLookUpBuilt)
		return;

	_lookup.clear();

	for (int i=0;i<_buf.size();i++)
	{
		Src *src=&_buf[i];
		if (src->src)
		{
			if (src->src->GetGUID()!=LevelGUID_Invalid)
				_lookup[src->src->GetGUID()]=src;
		}
	}
	_bLookUpBuilt=TRUE;
}


CLevelObjSrc *CLevelSources::FindLos(LevelGUID guid)
{
	_BuildLookUp();
	std::unordered_map<LevelGUID,Src*>::iterator it=_lookup.find(guid);
	if (it==_lookup.end())
		return NULL;
	return (*it).second->src;
}

CLevelObjParam *CLevelSources::FindLop(LevelGUID guid)
{
	_BuildLookUp();
	std::unordered_map<LevelGUID,Src*>::iterator it=_lookup.find(guid);
	if (it==_lookup.end())
		return NULL;
	return (*it).second->param;
}


CLevelObjSrc *CLevelSources::GetLos(DWORD idx)
{
	if (idx>=_buf.size())
		return NULL;
	return _buf[idx].src;
}

CLevelObjParam *CLevelSources::GetLop(DWORD idx)
{
	if (idx>=_buf.size())
		return NULL;
	return _buf[idx].param;
}

