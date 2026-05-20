#pragma once



enum OpType
{
	OpType_None=-1,
	OpType_Mouse,
	OpType_Keyboard,
	OpType_Timer,
	OpType_Command,
	OpType_SetCursor,
	OpType_Max,
};

#define CtrlOpFlag_CtrlDown 1
#define CtrlOpFlag_ShiftDown 2
#define CtrlOpFlag_AltDown 4


struct CtrlOp//keyboard ,mouse info
{
	CtrlOp()
	{
		op=Op_None;
		vk=0;
		ch=-1;
		flag=0;
		x=y=0;
		delta=0;
		memset(keysDown,0,sizeof(keysDown));
	}
	enum Op
	{
		Op_None,
		Op_Down,
		Op_Up,
		Op_DblClick,
		Op_Click,//for mouse only,when button's down & up is at the same position,that will be a click
		Op_Char,
		Op_Move,
		Op_Wheel,
		Op_Timer,
		Op_Cmd,
		Op_SetCursor,
		Op_KeysDown,
	};
	Op op;
	DWORD flag;//CtrlOpFlag_XXXX
	int vk;//Mouse Button or Key
	int x,y;//mouse position
	int delta;//valid for Op_MouseWheel
	int dt;//valid for Op_Timer
	DWORD idCmd;//valid for Op_Cmd
	WORD ch;//if -1,no char available
	unsigned char keysDown[16];//terminated by 0
	OpType GetType() const
	{
		if (op==Op_None)
			return OpType_None;
		if (op==Op_Cmd)
			return OpType_Command;
		if (op==Op_SetCursor)
			return OpType_SetCursor;
		switch(vk)
		{
		case VK_LBUTTON:
		case VK_RBUTTON:
		case VK_MBUTTON:
			return OpType_Mouse;
			break;
		default:
			if (op==Op_Timer)
				return OpType_Timer;
			if ((op==CtrlOp::Op_Move)||(op==CtrlOp::Op_Wheel))
				return OpType_Mouse;
			return OpType_Keyboard;
		}
	}
};

