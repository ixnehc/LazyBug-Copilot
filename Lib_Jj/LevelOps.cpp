/********************************************************************
	created:	2012/10/30 
	author:		cxi
	
	purpose:	记录一个obj里面所有的op
*********************************************************************/
#include "stdh.h"

#include "LevelOp.h"

#include "LevelOps.h"

#include "datapacket/BitPacket.h"


void CLevelOps::Clear()
{
	std::deque<CLevelOp*>::iterator it;
	for (it=_ops.begin();it!=_ops.end();it++)
	{
		CLevelOp *op=(*it);
		Class_Delete(op);
	}
	_ops.clear();
}


void CLevelOps::WriteSync(CBitPacket *bp,BOOL bFirstSync,BOOL &bContent)
{
	CLevelOp * buf[128];
	DWORD c=0;
	for (int i=0;i<_ops.size();i++)
	{
		CLevelOp *op=_ops[i];
		BOOL bIgnore=FALSE;
		if (bFirstSync)
		{
			//FirstSync的话Ignore所有Buff的增/删,因为它们会通过CLevelBuffs的WriteFirstSync来同步
			if (op->ToPtr<LevelOp_AddBuff>())
				bIgnore=TRUE;
			if (op->ToPtr<LevelOp_ModBuff>())
				bIgnore=TRUE;
		}
		if (!bIgnore)
		{
			if (c>=ARRAY_SIZE(buf))
			{
				assert(FALSE);
				bIgnore=TRUE;
			}
		}

		if (!bIgnore)
			buf[c++]=op;
	}

	if (c<=0)
		bp->Bit_Write_0();//没有skill op
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();//有skill op

		bp->Data_WriteSimple((BYTE)c);

		for (int i=0;i<c;i++)
		{
			CLevelOp*op=buf[i];
			op->GetDesc().Save(bp);
			op->Save(bp);
		}
	}
}

void CLevelOps::ReadSync(CBitPacket *bp)
{
	if (bp->Bit_Read())
	{
		BYTE sz=bp->Data_ReadSimple<BYTE>();

		for (int i=0;i<sz;i++)
		{
			LevelOpDesc desc;
			desc.Load(bp);

			extern CLevelOp *NewLevelOp(LevelOpDesc &desc);
			CLevelOp *op=NewLevelOp(desc);
			assert(op);

			op->Load(bp);
			_ops.push_back(op);
		}
	}
}

void CLevelOps::PostWriteSync()
{
	Clear();
}
