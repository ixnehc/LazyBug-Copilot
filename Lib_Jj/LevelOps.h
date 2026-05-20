#pragma once

#include "class/class.h"
#include "LevelOp.h"

class CLevelOp;
class CLevelOps
{
public:
	void Clear();
	void WriteSync(CBitPacket *bp,BOOL bFirstSync,BOOL &bContent);
	void ReadSync(CBitPacket *bp);
	void PostWriteSync();
	void AddOp(CLevelOp *op)	
	{		
		_ops.push_back(op);	
	}
	CLevelOp *FetchOp()
	{
		if (_ops.size()<=0)
			return NULL;
		CLevelOp *p=_ops[0];  
		_ops.pop_front();
		return p;
	}
public:
	std::deque<CLevelOp*> _ops;

};