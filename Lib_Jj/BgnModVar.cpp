/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnModVar.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "LevelSkillDriver.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgn_ModVar_Obsolete
BIND_BGN_CLASS(CBgn_ModVar_Obsolete,CBgp_ModVar_Obsolete);

extern BehaviorMemType ResolveSimpleVarType(StringID nmVar);


void CBgn_ModVar_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ModVar_Obsolete*pad=_GetPad<CBgp_ModVar_Obsolete>();

	CBehavior *bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		CBehaviorGraph *bg=_GetBg();
		if (bg)
		{
			CBehaviorMemDesc *memdesc=_GetMemDesc();
			if (memdesc)
			{
				BehaviorMemType tp=ResolveSimpleVarType(pad->nm);
				if (tp==BehaviorMemType_None)
					tp=memdesc->GetVarType(pad->nm);

				switch(tp)
				{
					case BehaviorMemType_Integer:
					{
						short n;
						if (_GetNumber(pad->nm,n))
						{
							switch(pad->mode)
							{
								case 0:
									n+=(short)_GetBPR(pad->bprRef);
									break;
								case 1:
									n-=(short)_GetBPR(pad->bprRef);
									break;
								case 2:
									n=(short)_GetBPR(pad->bprRef);
									break;
								case 3:
									n=(short)(((int)n)*_GetBPR(pad->bprRef));
									break;
							}
							_SetNumber(pad->nm,n);
						}
						break;
					}
					case BehaviorMemType_Bit:
					{
						switch(pad->mode)
						{
							case 0:
							case 1:
							{
								BOOL b;
								if (_GetBit(pad->nm,b))
									_SetBit(pad->nm,!b);
								break;
							}
							case 2:
							{
								_SetBit(pad->nm,_GetBPR(pad->bprRef)!=0?1:0);
								break;
							}
						}
						break;
					}
					default:
						assert(FALSE);
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}





////////////////////////////////////////////////////////////////////////
//CBgn_ModVar
BIND_BGN_CLASS(CBgn_ModVar,CBgp_ModVar);

void CBgp_ModVar::FillDesc(std::string &s,FillDescAssist *assist)
{
	s="n/a";
	if (nm!=StringID_Invalid)
	{
		BehaviorMemType tp=assist->GetMemType(nm);
		std::string sTarget;
		switch(tp)
		{
			case BehaviorMemType_Bit:
				sTarget=GetBVRDesc_Bool(BVR_ARG(b),assist);
				break;
			case BehaviorMemType_Integer:
				sTarget=GetBVRDesc_Int(BVR_ARG(n),assist);
				break;
			case BehaviorMemType_Float:
				sTarget=GetBVRDesc_Float(BVR_ARG(f),assist);
				break;
			case BehaviorMemType_StringID:
				sTarget=GetBVRDesc_StringID(BVR_ARG(idStr),assist);
				break;
			case BehaviorMemType_SkillRecord:
			{
				sTarget=GetBVRDesc_SkillID(BVR_ARG(idSkill),assist);
				if (sTarget.empty())
					sTarget="[null]";
				break;
			}
			case BehaviorMemType_ItemRecord:
			{
				sTarget=GetBVRDesc_ItemID(BVR_ARG(idItem),assist);
				if (sTarget.empty())
					sTarget="[null]";
				break;
			}
			case BehaviorMemType_BuffRecord:
			{
				sTarget=GetBVRDesc_BuffID(BVR_ARG(idBuff),assist);
				if (sTarget.empty())
					sTarget="[null]";
				break;
			}
			case BehaviorMemType_UnitRecord:
			{
				sTarget=GetBVRDesc_UnitID(BVR_ARG(idUnit),assist);
				if (sTarget.empty())
					sTarget="[null]";
				break;
			}
			case BehaviorMemType_ResourceRecord:
			{
				sTarget=GetBVRDesc_ResourceID(BVR_ARG(idRes),assist);
				if (sTarget.empty())
					sTarget="[null]";
				break;
			}
				//XXXXX:more BehaviorMemType
		}
		if (!sTarget.empty())
		{
			if(mode==2)
				FormatString(s,"%s设为%s",assist->GetStr(nm),sTarget.c_str());
			if (mode==0)
				FormatString(s,"%s+=%s",assist->GetStr(nm),sTarget.c_str());
			if (mode==1)
				FormatString(s,"%s-=%s",assist->GetStr(nm),sTarget.c_str());
			if (mode==3)
				FormatString(s,"%s*=%s",assist->GetStr(nm),sTarget.c_str());
			if (mode==4)
				FormatString(s,"%s取反",assist->GetStr(nm));
		}
	}
}



void CBgn_ModVar::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ModVar*pad=_GetPad<CBgp_ModVar>();

	CBehavior *bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		CBehaviorGraph *bg=_GetBg();
		if (bg)
		{
			CBehaviorMemDesc *memdesc=_GetMemDesc();
			if (memdesc)
			{
				BehaviorMemType tp=ResolveSimpleVarType(pad->nm);
				if (tp==BehaviorMemType_None)
					tp=memdesc->GetVarType(pad->nm);

				switch(tp)
				{
					case BehaviorMemType_Integer:
					{
						short n;
						if (_GetNumber(pad->nm,n)) 
						{
							switch(pad->mode)
							{
								case 0:
									n+=(short)pad->n;
									break;
								case 1:
									n-=(short)pad->n;
									break;
								case 2:
									n=(short)pad->n;
									break;
								case 3:
									n=(short)(((int)n)*(int)pad->n);
									break;
							}
							_SetNumber(pad->nm,n);
						}
						break;
					}
					case BehaviorMemType_Float:
					{
						float f;
						if (_GetFloat(pad->nm,f))
						{
							switch(pad->mode)
							{
								case 0:
									f+=(float)pad->f;
									break;
								case 1:
									f-=(float)pad->f;
									break;
								case 2:
									f=(float)pad->f;
									break;
								case 3:
									f=(float)(((float)f)*(float)pad->f);
									break;
							}
							_SetFloat(pad->nm,f);
						}
						break;
					}
					case BehaviorMemType_Bit:
					{
						switch(pad->mode)
						{
							case 4:
							{
								BOOL b;
								if (_GetBit(pad->nm,b))
									_SetBit(pad->nm,!b);
								break;
							}
							case 2:
							{
								_SetBit(pad->nm,pad->b!=0?1:0);
								break;
							}
						}
						break;
					}
					case BehaviorMemType_StringID:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_StringID,pad->idStr);
						break;
					}
					case BehaviorMemType_BuffRecord:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_BuffRecord,pad->idBuff);
						break;
					}
					case BehaviorMemType_SkillRecord:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_SkillRecord,pad->idSkill);
						break;
					}
					case BehaviorMemType_ItemRecord:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_ItemRecord,pad->idItem);
						break;
					}
					case BehaviorMemType_UnitRecord:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_UnitRecord,pad->idUnit);
						break;
					}
					case BehaviorMemType_ResourceRecord:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_ResourceRecord,pad->idRes);
						break;
					}
					case BehaviorMemType_ObjID:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_ObjID,pad->idObj);
						break;
					}
					case BehaviorMemType_GUID:
					{
						if (pad->mode==2)
							_SetID(pad->nm,BehaviorMemType_GUID,pad->idGUID);
						break;
					}
					//XXXXX:more BehaviorMemType
					default:
						assert(FALSE);
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}

