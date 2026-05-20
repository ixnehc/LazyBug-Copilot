#pragma once

#include "GuiEditor.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/ISpt.h"

/*
	负责代理树的创建过程
	第一步： 在鼠标所指的位置创建一棵树，提交创建命令。
	第二步： 树跟随着鼠标移动。
	第三步： 鼠标单击结束移动，提交移动命令。并且将自身的状态置为非活动。

	<在移动过程中按下ESC键>：提交删除命令。并且将自身的状态置为非活动。
*/
class CGuiAgent_treeadd :public CGuiAgent
{
public:
	CGuiAgent_treeadd(void);
	virtual ~CGuiAgent_treeadd(void);
public:
	virtual BOOL OnKeyDown(char c,DWORD flag);//c is in upper case for 'a'~'z'
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnDraw();
	virtual BOOL OnCommand(DWORD idCmd);
protected:
	BOOL _HitTest(int x,int y);
private:
	i_math::vector3df _location;
	TreeInfo _info;
	ISptDrawer * _drawer;
};


