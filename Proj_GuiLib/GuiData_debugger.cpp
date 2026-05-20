
#include "stdh.h"

#include "GuiData_debugger.h"

#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetEventer.h"

#include "WorldSystem/Client/IClient.h"

#include "PhysicsSystem/IPhysicsSystem.h"

#include "WorldSystem/stubparams/stubparams.h"

#include "timer/profiler.h"

#include "stringparser/stringparser.h"




BOOL DebuggerContext::IsBreak()	
{		
	if (!bRunning)
		return FALSE;
	return dbgr->IsBreak();	
}

BOOL DebuggerContext::IsRunning()
{		
	return bRunning;	
}


void DebuggerContext::Run(BreakMode mode,ProtoID idProto,ProtoID idGE,ProtoID idGT,BOOL bAllowEditHelper)
{
	if (IsRunning())
		return;

	bRunning=TRUE;

	dbgr->Attach(mode);

	//设置Progress Callback
	if (TRUE)
	{
		ESProgressCallBack dlgt;
		dlgt.bind(this,&DebuggerContext::OnProgress);
		pES->SetProgressCallBack(dlgt);
	}

	bClient=(idProto==ProtoID_Null);
	
	if (FALSE==pES->SwitchEditMode(FALSE))
		assert(FALSE);

	if (bClient)
		pES->AttachClient();

	IClient *pClient=pES->GetSS()->pClient;
	if (bClient&&pClient)
	{
		IProtoLib *lib=pES->GetProtoLib();
		if (idGE==ProtoID_Null)
			idGE=lib->FindProto(pClient->GetGE());
		if (idGT==ProtoID_Null)
			idGT=lib->FindProto(pClient->GetGT());
	}

	pES->GetGlobal()->SetGT(idGT);
	pES->GetGlobal()->SetGE(idGE);

	pES->Locate(i_math::vector3df(0,0,0));
	//			dataProto->pES->GetMap()->ReloadAll();
 
	BOOL bEmbedded=FALSE;
	IEntity *enGlobal=pES->GetGlobal()->GetEntity();
	if (enGlobal)
	{
		void *ownerSrc;
		GStubBase *stbSrc=enGlobal->FindStub("SetTestee",ownerSrc);

		if (stbSrc)
		{
			StbParams t;
			t.Add(pES->GetProtoLib()->FindPath(idProto));
			stbSrc->SetProp(ownerSrc,&t);
			bEmbedded=TRUE;
		}
	}

	if ((!bEmbedded)&&(idProto!=ProtoID_Null))
		entity=pES->CreateEntity(i_math::matrix43f(),idProto,FALSE,bAllowEditHelper);

	pES->GetAS()->GetEventer()->FlushNewClocks();

	protoid=idProto;

	tLast=GetTickCount();

	if (pES->GetAS()->GetSS()->pPS)
		pES->GetAS()->GetSS()->pPS->BeginDebug();


}


void DebuggerContext::Continue(BreakMode mode)
{
	if (!IsRunning())
		return;
	if (IsBreak())
		dbgr->Continue(mode);
}

void DebuggerContext::RequestStop()
{
	Resume(0xffffffff);
	if (!IsRunning())
		return;
	if (dbgr)
	{
		bNeedStop=TRUE;
		Continue(Break_PointCheck);
		dbgr->Detach();
	}
}

BOOL DebuggerContext::IsPaused()
{
	IAssetSystem *pAS=pES->GetAS();
	if (pAS)
		return pAS->GetSS()->bPaused;
	return FALSE;
}


void DebuggerContext::Resume(DWORD nFrames)
{
	if (!IsRunning())
		return;

	IAssetSystem *pAS=pES->GetAS();
	if (pAS)
	{
		if (pAS->GetSS()->bPaused)
		{
			if(pAS->GetClient())
				pAS->GetClient()->Resume(nFrames);
			else
				pAS->Resume(nFrames);
		}
	}
}

void DebuggerContext::Pause()
{
	if (!IsRunning())
		return;

	IAssetSystem *pAS=pES->GetAS();
	if (pAS)
	{
		if (!pAS->GetSS()->bPaused)
		{
			if(pAS->GetClient())
				pAS->GetClient()->Pause();
			else
				pAS->Pause();
		}
	}
}


void DebuggerContext::TogglePause()
{
	if (IsPaused())
		Resume(0xffffffff);
	else
		Pause();
}

void DebuggerContext::StepPause()
{
	if (IsPaused())
		Resume(1);
}



void DebuggerContext::Attach(BreakMode mode)
{
	if (IsRunning())
		return;

	bRunning=TRUE;

	dbgr->Attach(mode);
}

void DebuggerContext::Detach()
{
	if (dbgr)
	{
		if (!IsRunning())
			return;
		bRunning=FALSE;
		bNeedStop=FALSE;
		dbgr->Detach();
	}

}


void DebuggerContext::Update()
{
	if (!IsRunning())
		return;
	if (IsBreak())
		return;

	DWORD t=GetTickCount();

	float vAcc=1.0f;
	if (acc>0)
		vAcc*=(float)(1<<acc);
	else
	{
		if (acc>=MIN_DEBUG_ACC)
			vAcc/=(float)(1<<(-acc));
		else
			vAcc=0.0f;
	}

	if (TRUE)
	{
		DWORD dt=(t-tLast);
		if (bRecentProgress)
		{
			if (dt>50)
				dt=50;
			bRecentProgress=FALSE;
		}
		if (dt>200)
			dt=200;
		input.SetDt(((float)dt)/1000.0f*vAcc);
	}

	if (TRUE)
	{
		CtrlOp op;
		op.op=CtrlOp::Op_KeysDown;
		int idx=0;
		for (int i=0x30;i<=0x5A;i++)//0,1,2,....9,A,B,C,....Z
		{
			if (GetKeyState(i)&0x80)
			{
				op.keysDown[idx]=(unsigned char)i;
				idx++;
				if (idx>=ARRAY_SIZE(op.keysDown))
					break;
			}
		}
		input.AddOp(op);
	}

	tLast=t;
	pES->Update(input);
	input.ClearOp();

	if (!pES->IsInProgress())//进度中不能Stop
	{
		if (bNeedStop)
		{
			bNeedStop=FALSE;

			//do the actual stop
			SAFE_DESTROY(entity);
			protoid=ProtoID_Null;

			if (bClient)
				pES->DetachClient();

			//时间归0
			if (FALSE==pES->SwitchEditMode(TRUE))
			{
				IProtoLib *lib=pES->GetProtoLib();
				DWORD c;
				IEntity **entities=lib->EnumEntities(c);
				if (c>0)
				{
					std::map<IProto*,int> tmap;
					for (int i=0;i<c;i++)
					{
						IProto *proto=entities[i]->GetProto();
						std::map<IProto*,int>::iterator it=tmap.find(proto);
						if (it==tmap.end())
							tmap[proto]=1;
						else
							((*it).second)++;
					}
					std::string s,ss;
					s="运行结束后发现系统中还剩余以下Proto的实例:\n";
					if (TRUE)
					{
						std::map<IProto*,int>::iterator it;
						for (it=tmap.begin();it!=tmap.end();it++)
						{
							IProto *proto=(*it).first;
							int c=(*it).second;

							s+=proto->GetFilePath();
							FormatString(ss,"(%d个)\n",c);
							s+=ss;
						}
					}

					AfxMessageBox(fromMBCS(s.c_str()));

					for (int i=0;i<c;i++)
					{
						IEntity *entity=entities[i];
						entity->AddRef();
						SAFE_DESTROY(entity);
					}

					//再尝试一次
					pES->SwitchEditMode(TRUE);
				}
				else
					assert(FALSE);
			}

			if (pES->GetAS()->GetSS()->pPS)
				pES->GetAS()->GetSS()->pPS->EndDebug();

			dbgr->Detach();

			bRunning=FALSE;
		}
	}
}

BOOL DebuggerContext::OnProgress()
{
	static DWORD tickLast=0;
	DWORD tick=GetTickCount();
	bRecentProgress=FALSE;
	if (tick-tickLast>5)
	{
		Update();
		if (dlgtProgressDraw)
			dlgtProgressDraw();
		tickLast=tick;
	}
	bRecentProgress=TRUE;
	return TRUE;
}
