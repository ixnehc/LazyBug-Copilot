/********************************************************************
	created:	2013/6/20 
	author:		cxi
	
	purpose:	监控是否有随从命令
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "LevelRecords.h"
#include "LevelRecordSkill.h"

#include "BgnService.h"

extern CLevelService *LevelUtil_GetService(CLevelObj *lo,LevelServiceType tp);
extern CLevelService *LevelUtil_ObtainService(CLevelObj *lo,LevelServiceType tp);


////////////////////////////////////////////////////////////////////////
//CBgn_MonitorService
BIND_BGN_CLASS(CBgn_MonitorService,CBgp_MonitorService);
void CBgn_MonitorService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_MonitorService*pad=_GetPad<CBgp_MonitorService>();
	if (pad->_tp!=StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		CLevelService *service=LevelUtil_ObtainService(lo,pad->_tp);
		if (service)
		{
			service->AddClient(lo->GetID());
			_bMonitoring=TRUE;
			return;
		}
	}

	_SetResult(A_Fail);
}

void CBgn_MonitorService::Update(BGNOutputs &outputs)
{
	CBgp_MonitorService*pad=_GetPad<CBgp_MonitorService>();

	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
	{
		LevelObjID idServer=service->QueryClient(lo->GetID());
		if (idServer!=LevelObjID_Invalid)
		{
			if (pad->_nmVar!=StringID_Invalid)
				_SetID(pad->_nmVar,BehaviorMemType_ObjID,idServer);
			_OutputOk(outputs,1,"监控到");
		}
	}
}

void CBgn_MonitorService::Destroy()
{
	//放弃对服务的请求
	if (_bMonitoring)
	{
		CBgp_MonitorService*pad=_GetPad<CBgp_MonitorService>();
		if (pad->_tp!=StringID_Invalid)
		{
			CLevelObj *lo=_GetLo();
			CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
			if (service)
			{
				service->RemoveClient(lo->GetID());
				return;
			}
		}
		_bMonitoring=FALSE;
	}
}


void CBgn_MonitorService::Break(BGNOutputs &outputs)
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////
//CBgn_FindNearbyService
BIND_BGN_CLASS(CBgn_FindNearbyService,CBgp_FindNearbyService);
void CBgn_FindNearbyService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FindNearbyService*pad=_GetPad<CBgp_FindNearbyService>();
	if (pad->_tp!=StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
		if (service)
		{
			LevelObjID idServer=LevelObjID_Invalid;
			if (pad->_nmPos==StringID_Invalid)
			{
				LevelPos pos=lo->GetFramePos();
				idServer=service->FindClosestServer(pos,pad->_radius,pad->_nMinAvailableQuato);
			}
			else
			{
				LevelPos pos;
				if(_GetPos(pad->_nmPos,pos))
					idServer=service->FindClosestServer(pos,pad->_radius,pad->_nMinAvailableQuato);
			}

			if (idServer!=LevelObjID_Invalid)
			{
				if (pad->_nmServer!=StringID_Invalid)
					_SetID(pad->_nmServer,BehaviorMemType_ObjID,idServer);
				if (pad->_nmServerPos!=StringID_Invalid)
				{
					extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
					CLevelObj *loServer=LevelUtil_GetAliveLo(_GetLevel(),idServer);
					if (loServer)
						_SetPos(pad->_nmServerPos,loServer->GetFramePos());
				}

				_OutputOk(outputs,1,"成功");

				return;
			}
		}
	}

	_OutputFail(outputs,2,"失败");
	_SetResult(A_Fail);
}


////////////////////////////////////////////////////////////////////////
//CBgn_AddService
BIND_BGN_CLASS(CBgn_AddService,CBgp_AddService);
void CBgn_AddService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_AddService*pad=_GetPad<CBgp_AddService>();
	if (pad->_tp!=StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		CLevelService *service=LevelUtil_ObtainService(lo,pad->_tp);
		if (service)
		{
			if (!pad->_bGlobal)
				service->AddServer(lo->GetID(),pad->_nQuota);
			else
				service->AddUniqueServer(lo->GetID(),pad->_nQuota);
		}
	}

	_OutputOk(outputs,1,"结束");
}

////////////////////////////////////////////////////////////////////////
//CBgn_RemoveService
BIND_BGN_CLASS(CBgn_RemoveService,CBgp_RemoveService);
void CBgn_RemoveService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_RemoveService*pad=_GetPad<CBgp_RemoveService>();
	if (pad->_tp!=StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
		if (service)
			service->RemoveServer(lo->GetID());
	}

	_OutputOk(outputs,1,"结束");
}


////////////////////////////////////////////////////////////////////////
//CBgn_ClaimServiceWithSkill
BIND_BGN_CLASS(CBgn_ClaimServiceWithSkill,CBgp_ClaimServiceWithSkill);
void CBgn_ClaimServiceWithSkill::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ClaimServiceWithSkill*pad=_GetPad<CBgp_ClaimServiceWithSkill>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	_idTarget=LevelObjID_Invalid;
	if (pad->_nmVar!=StringID_Invalid)
		_GetID(pad->_nmVar,BehaviorMemType_ObjID,_idTarget);

	LevelBehaviorContext *ctx=_GetCtx();

	//立即开始攻击
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->_idSkill);
	if (recSkill)
	{
		if (_idTarget!=LevelObjID_Invalid)
		{
			LevelSkillTarget target;
			target.SetObjID(_idTarget);
			_verCasting=driver->GetCastVer();
			driver->Start(LevelSkillType(pad->_idSkill),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL);
		}
	}
}

AResult CBgn_ClaimServiceWithSkill::_UpdateClaim()
{
	CBgp_ClaimServiceWithSkill*pad=_GetPad<CBgp_ClaimServiceWithSkill>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return A_Fail;
	if (!driver->IsWorking())
	{
		CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
		if (service)
		{
			if (driver->GetCastVer()>_verCasting)
			{
				service->Acknowledge(lo->GetID());
				service->SetClientCD(lo->GetID(),pad->_durCDOnOK);
				return A_Ok;
			}
			else
			{
				service->RemoveClient(lo->GetID());
				service->SetClientCD(lo->GetID(),pad->_durCDOnFail);
				return A_Fail;
			}
		}

		return A_Fail;
	}

	return A_Pending;
}


void CBgn_ClaimServiceWithSkill::Update(BGNOutputs &outputs)
{
	AResult result=_UpdateClaim();
	if (result==A_Ok)
	{
		_OutputOk(outputs,1,"成功");
	}
	if (result==A_Fail)
	{
		_OutputFail(outputs,2,"失败");
	}
}

void CBgn_ClaimServiceWithSkill::Destroy()
{
	CLevelObj *lo=_GetLo();
	extern LevelSkillID LevelUtil_CancelSkill(CLevelObj *lo,BOOL bStopAct);
	LevelUtil_CancelSkill(lo,TRUE);

	_UpdateClaim();
}


void CBgn_ClaimServiceWithSkill::Break(BGNOutputs &outputs)
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////
//CBgn_ClaimService
BIND_BGN_CLASS(CBgn_ClaimService,CBgp_ClaimService);
void CBgn_ClaimService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ClaimService*pad=_GetPad<CBgp_ClaimService>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
	{
		service->Acknowledge(lo->GetID());
		service->SetClientCD(lo->GetID(),pad->_durCD);

		_OutputOk(outputs,1,"成功");
		return;
	}
	_OutputFail(outputs,2,"失败");
}


////////////////////////////////////////////////////////////////////////
//CBgn_ServiceQuota
BIND_BGN_CLASS(CBgn_ServiceQuota,CBgp_ServiceQuota);

void CBgn_ServiceQuota::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ServiceQuota*pad=_GetPad<CBgp_ServiceQuota>();
	if (pad->_tp!=StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
		if (service)
		{
			DWORD nQuota=service->GetServerQuota(lo->GetID());

			if (pad->_nmVar!=StringID_Invalid)
				_SetNumber(pad->_nmVar,(short)nQuota);
		}
	}

	_OutputOk(outputs,1,"结束");

}

//////////////////////////////////////////////////////////////////////////
//CBgn_PreserveService
BIND_BGN_CLASS(CBgn_PreserveService,CBgp_PreserveService);
void CBgn_PreserveService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_PreserveService*pad=_GetPad<CBgp_PreserveService>();

	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
	{
		LevelObjID idServer=LevelObjID_Invalid;
		if (pad->_nmServer!=StringID_Invalid)
			_GetID(pad->_nmServer,BehaviorMemType_ObjID,idServer);
		else
			idServer=service->GetUniqueServer();

		if (idServer!=LevelObjID_Invalid)
		{
			if (service->Preserve(lo->GetID(),idServer))
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"失败");
	return;

}

//////////////////////////////////////////////////////////////////////////
//CBgn_CanPreserveService
BIND_BGN_CLASS(CBgn_CanPreserveService,CBgp_CanPreserveService);
void CBgn_CanPreserveService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CanPreserveService*pad=_GetPad<CBgp_CanPreserveService>();

	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
	{
		LevelObjID idServer=LevelObjID_Invalid;
		if (pad->_nmServer!=StringID_Invalid)
			_GetID(pad->_nmServer,BehaviorMemType_ObjID,idServer);
		else
			idServer=service->GetUniqueServer();

		if (idServer!=LevelObjID_Invalid)
		{
			if (service->CanPreserve(lo->GetID(),idServer))
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"失败");
	return;

}

//////////////////////////////////////////////////////////////////////////
//CBgn_CheckPreserveService
BIND_BGN_CLASS(CBgn_CheckPreserveService,CBgp_CheckPreserveService);
void CBgn_CheckPreserveService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckPreserveService*pad=_GetPad<CBgp_CheckPreserveService>();

	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
	{
		if (pad->_nmServer!=StringID_Invalid)
		{
			LevelObjID idServer=LevelObjID_Invalid;
			_GetID(pad->_nmServer,BehaviorMemType_ObjID,idServer);

			if (idServer!=LevelObjID_Invalid)
			{
				if (service->CheckPreserve(lo->GetID(),idServer))
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
		else
		{
			if (service->CheckPreserve(lo->GetID()))
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"否");
	return;

}

//////////////////////////////////////////////////////////////////////////
//CBgn_GetPreserveServer
BIND_BGN_CLASS(CBgn_GetPreserveServer,CBgp_GetPreserveServer);
void CBgn_GetPreserveServer::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetPreserveServer*pad=_GetPad<CBgp_GetPreserveServer>();

	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
	{
		if (pad->_nmServer!=StringID_Invalid)
		{
			LevelObjID idServer=service->GetPreserve(lo->GetID());
			if (idServer!=LevelObjID_Invalid)
			{
				_SetID(pad->_nmServer,BehaviorMemType_ObjID,idServer);
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"否");
	return;

}


//////////////////////////////////////////////////////////////////////////
//CBgn_GetServicePreserveClient
BIND_BGN_CLASS(CBgn_GetServicePreserveClient,CBgp_GetServicePreserveClient);
void CBgn_GetServicePreserveClient::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetServicePreserveClient*pad=_GetPad<CBgp_GetServicePreserveClient>();

	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
	{
		if (pad->_nmClient!=StringID_Invalid)
		{
			LevelObjID idClient=service->Get1stPreserveClient(lo->GetID());
			if (idClient!=LevelObjID_Invalid)
			{
				_SetID(pad->_nmClient,BehaviorMemType_ObjID,idClient);
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"否");
	return;

}


//////////////////////////////////////////////////////////////////////////
//CBgn_CheckPreserveService
BIND_BGN_CLASS(CBgn_GetServiceServerCount,CBgp_GetServiceServerCount);
void CBgn_GetServiceServerCount::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetServiceServerCount*pad=_GetPad<CBgp_GetServiceServerCount>();

	DWORD count=0;
	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
		count=service->GetServerCount();

	if (pad->_nmVar!=StringID_Invalid)
		_SetNumber(pad->_nmVar,(short)count);

	_OutputOk(outputs,1,"结束");
	return;

}


//////////////////////////////////////////////////////////////////////////
//CBgn_DiscardService
BIND_BGN_CLASS(CBgn_DiscardService,CBgp_DiscardService);
void CBgn_DiscardService::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DiscardService*pad=_GetPad<CBgp_DiscardService>();

	CLevelObj *lo=_GetLo();
	CLevelService *service=LevelUtil_GetService(lo,pad->_tp);
	if (service)
		service->RemoveClient(lo->GetID());

	_OutputOk(outputs,1,"结束");
	return;
}
