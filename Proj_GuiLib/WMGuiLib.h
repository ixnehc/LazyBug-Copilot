#pragma once

#include "GuiLib.h"

#define GLM_Start WM_USER+312
#define GLM_ResTree_ContentChange (GLM_Start+0)
#define GLM_ResTree_DblClick (GLM_Start+1)

#define GLM_SlideSpin_Notify (GLM_Start+2) //wparam:notify code
#define GLM_ResAnchor_Change (GLM_Start+3) //wparam:ResAnchor CtrlID,lparam:path ptr(string)
#define GLM_PrlTree_DblClick (GLM_Start+4)	//wparam:proto的路径

#define GLM_Game_PlayHere (GLM_Start+5)//wparam:i_math::vector3df* pos

#define GLM_Proto_DragOver (GLM_Start+6)	//wparam:路径,lparam:0表示为proto路径,1表示为asset名称,返回1表示可以drop
#define GLM_Proto_DragDrop (GLM_Start+7)	//wparam:路径,lparam:0表示为proto路径,1表示为asset名称

#define GLM_DoFindCommand (GLM_Start+8)//wparam: FindCmd*cmd

#define GLM_ShaderTree_DblClick (GLM_Start+20)//wparam:路径,lparam表示双击的item的类型

#define GLM_DbTree_DblClick (GLM_Start+30)	//wparam:proto的路径


#define SSN_BeginChange 1
#define SSN_Changing 2
#define SSN_EndChange 3//lparam:SlideSpinValue *

// Cust control Message:
// lParam  : Custom control Pointer
// WParam  : Low WORD  control ID
#define PBN_BASE  GLM_Start + 1000
#define PBN_PRECHANGE			 PBN_BASE + 0  // 开始变化
#define PBN_ONCHANGE			 PBN_BASE + 1	 // 任何一次变化都会发出(包括首次 和 最后一次)
#define PBN_ENDCHANGE			 PBN_BASE + 2  // 最后一次    任何一次改变都会一次确保发出Begin On End

#define STN_ONITEMCHANGE  GLM_Start + 1120
