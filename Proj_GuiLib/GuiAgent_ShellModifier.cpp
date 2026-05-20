#include "stdh.h"

#include "RenderSystem/ITexture.h"

#include ".\guiagent_shellmodifier.h"

#include "GuiData_proto.h"

#include "WorldSystem/IEntitySystem.h"

#include "resource.h"

#include "GuiActor_proto.h"

#include "AgentCmdID.h"


#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"

#include "WorldSystem/IWorldSystem.h"

#include "WorldSystem/IAssetShell.h"

#include "GuiData.h"

#include "GuiData_debugger.h"

#include "Log/LogDump.h"

#include "align/rectalign.h"

#include "WorldSystem/assetcore/AssetCtrl/ShellImage.h"

#define VOID_RET

#define CHECK_RUNNING(ret)																												\
	GuiData_Debugger * dataDebugger = (GuiData_Debugger*)FindData("debugger");					\
	assert(dataDebugger);																														\
	if(dataDebugger->context->IsRunning())																						\
		return ret;

#define PRE_WORK()																																\
	CHECK_RUNNING(TRUE);																													\
	GuiData_Proto* data = (GuiData_Proto*)FindData("proto");															\
	assert(data);																																		\
	_Update();																																			

CGuiAgent_ShellModifier::CGuiAgent_ShellModifier(void)
{
	_iRc = -1;
	_iProp = -1;
	_iRcEdit = -1;
	_Op = Drag_None;
	_rcActive.set(0,0,0,0);
	_newMenu = NULL;
}

CGuiAgent_ShellModifier::~CGuiAgent_ShellModifier(void)
{
}

void CGuiAgent_ShellModifier::OnAttachView(CGeView *view,DWORD iLevel)
{
	_UpdateGuiList();
}

// enum shell node from a tree node.
// a tree node has at least one shell node or none.
void CGuiAgent_ShellModifier::_EnumNode(IProto * proto,CNodeTree * tree,NodeHandle handle,int nodeParent,i_math::recti * rcParent,std::vector<_ShellNode> &nodes)
{	
	IProtoNode * nodeProto = NULL;

	//get tree node path name.
	std::string path = tree->GetPath(handle);
	
	// find proto from path name.
	if(path.compare("")!=0)
		nodeProto = proto->FindNode(path.c_str());

	i_math::recti rcScreen(0,0,SHELL_STANDARD_WIDTH,SHELL_STANDARD_HEIGHT);

	RectAlign alignType = ALIGN_LEFTUP;
	// if found a proto node , enum all the properties with name "Rect" see it as a shell node.
	if(nodeProto)
	{
		if(nodeProto->IsDynamic()) //ingore the dynamic asset.
			return;

		DWORD n = nodeProto->GetPropCount();
		int p =0;		
		i_math::vector4di relativeArea;
		for(;p<n;p++)
		{
			std::string nameProp = nodeProto->GetPropName(p);
			if(nameProp.compare("Rect") == 0)
			{
				Prop_Sx4 * prop = (Prop_Sx4 *)nodeProto->GetPropData(p);
				relativeArea = prop->v;
			}
			if(nameProp.compare("Align")==0)
			{
				Prop_S * prop = (Prop_S *)nodeProto->GetPropData(p);
				alignType =(RectAlign) prop->v;
			}
		}

		if(rcParent)
		{
			rcScreen  = RelativeAreaToLocalRect(*rcParent,relativeArea,alignType);
			rcScreen += rcParent->UpperLeftCorner;
		}
		else
			rcScreen.set(relativeArea.x,relativeArea.y,relativeArea.z,relativeArea.w);
	}
	
	BOOL bTop = FALSE;
	int iParent = -1;

	if(nodeProto)
	{
		DWORD nProps = nodeProto->GetPropCount();
		
		// a proto node may has more than one shell node, but only one is main rect.
		// the property of main rect is always "Rect". 
		_ShellNode nodeShell;
		nodeShell.nodeProtoID = nodeProto->GetID();
		nodeShell.c = 0;

		_ShellNode nodeInner;
		int ip = 0;

		std::vector<std::string> images;
	
		// find out all the shell node contained.
		for(int i = 0;i<nProps;i++)
		{
			assert(ip<4);

			std::string nameProp = nodeProto->GetPropName(i);
			GStubBase * stub = nodeProto->FindStub(nameProp.c_str());
			// if the property stub is unlinked.
			if(NULL == stub)
				continue;
			
			// if stub semectic marker is "CSem_Rect" indicate that it is a shell node
			if(stub->sem.code == GSem(GSem_Rect).code)
			{
				// if property name is Rect indicate that is the main shell node
				if(nameProp.compare("Rect")==0) 
				{
					nodeShell.idxs[0] = i;		// current node property index
					nodeShell.c = 1;
				}
				else	// window element consider as child mian shell node.
				{
					nodeInner.idxs[ip++] = i;
					nodeInner.c = ip;			// current rect count
				}
			}
			else if(nameProp.compare("AlwaysOnTop")==0)
			{
				Prop_S * prop = (Prop_S *)nodeProto->GetPropData(i);
				assert(prop->GetClass()->CheckName("Prop_S"));
				bTop = (prop->v == TRUE);				
			}
			else if(stub->sem.code==GSem_TexturePartPath)
			{
				Prop_String * propImagePath = (Prop_String *)nodeProto->GetPropData(i);
				assert(propImagePath->CheckClassName("Prop_String"));
				if(!propImagePath->v.empty())
					images.push_back(propImagePath->v);
			}
			
			//Combo image ,only support one whole image
			if(TRUE){
				GProperty * prop = nodeProto->GetPropData(i);
				if(prop->CheckClassName("PropRef"))
				{
					PropRef *propRef=static_cast<PropRef*>(prop);
					if (propRef->clss->CheckName("ImageCombo"))
					{
						ImageCombo * image =(ImageCombo * )propRef->stuff;
						if(image&&image->combo==0&&(!image->path.empty()))
							images.push_back(image->path);
					}
				}
			}

		}
		
		if(TRUE)
		{
			if(nodeShell.c&&nodeInner.c)  // the window elelment node.
			{
				nodeInner.rcParent = rcScreen;
				nodeInner.alignType = alignType;
				nodeInner.nodeProtoID = nodeShell.nodeProtoID;
				nodeInner.images.assign(images.begin(),images.end());
			}

			if(nodeShell.c)		// window rect
			{
				nodeShell.images.assign(images.begin(),images.end());
				nodeShell.bMain = TRUE;
				nodeShell.alignType = alignType;
				nodeShell.iParent  = nodeParent;
				nodeShell.rcParent = *rcParent;
				if(bTop)
				{	
					int iParent = _topnodes.size();
					nodeShell.iParent = -1; //
					nodeInner.iParent = iParent;
					_topnodes.push_back(nodeShell);
					_topnodes.push_back(nodeInner);
				}
				else
				{
					iParent = nodes.size();
					nodeInner.iParent = iParent;
					nodes.push_back(nodeShell);
					nodes.push_back(nodeInner);
				}
			}
			// collect one node completely.
		}
	}
	
	// enum the child node.
	DWORD c = tree->GetChildCount(handle);

	for(int i = 0;i<c;i++)
	{
		NodeHandle child = tree->GetChild(handle,i);

		if(bTop)
			_EnumNode(proto,tree,child,iParent,&rcScreen,_topnodes);
		else
			_EnumNode(proto,tree,child,iParent,&rcScreen,nodes);
	}
	
}

BOOL CGuiAgent_ShellModifier::OnCommand(DWORD idCmd)
{
	PRE_WORK();

	if(ID_AGENT_ADJUST_SIZE_START<=idCmd&&ID_AGENT_ADJUST_SIZE_END>idCmd)
	{
		if(_iRcEdit>=0&&_iProp>=0)
		{
			_ShellNode & nodeShell = _nodes[_iRcEdit];

			IProto * proto = NULL;
			if(data->lib)
				proto = data->lib->ObtainProto(data->protoid);
			
			if(!proto)
				return TRUE;

			int idx = idCmd - ID_AGENT_ADJUST_SIZE_START;
			assert(idx<nodeShell.images.size());

			std::string & image = nodeShell.images[idx];
			std::string pathTex;
			i_math::rect_sh rc;
			ParseShellImageStr(image,pathTex,rc);
			
			if(rc.getArea()==0)
			{
				GuiData_Proto * dataSys =(GuiData_Proto *) FindData("proto");
				
				if(!dataSys)
					return TRUE;
				
				IRenderSystem * pRS = dataSys->pES->GetWS()->GetRS();

				ITexture * pTex = (ITexture *)pRS->GetTexMgr()->ObtainRes(pathTex.c_str());
				if(!pTex)
					return TRUE;

				rc.LowerRightCorner.x =(short) pTex->GetWidth();
				rc.LowerRightCorner.y = (short)pTex->GetHeight();
				
				SAFE_RELEASE(pTex);
			}

			IProtoNode * nodeProto = proto->GetNode(data->sels[0]);
			Prop_Sx4 * prop = (Prop_Sx4 *)nodeProto->GetPropData(_iProp);
			assert(prop->CheckClassName("Prop_Sx4"));
			
			CClass * cls = nodeProto->GetAssetClass();
			const char * nameCls = NULL;
			if(cls)
				nameCls = cls->GetName();
			BOOL bButtonCtrl = (nameCls&&(strcmp(nameCls,"AstButton")==0));

			prop->v.z = prop->v.x + rc.getWidth();
			if(bButtonCtrl)
				prop->v.w = prop->v.y + rc.getHeight()/4;
			else
				prop->v.w = prop->v.y + rc.getHeight();

			nodeProto->IncVer();
		}
	}
	else
	{
		CGuiActor_Proto * actor = (CGuiActor_Proto *)_GetActor();
		actor->SendProtoTreeCmd(idCmd);
	}
	
	return TRUE;
}

BOOL CGuiAgent_ShellModifier::OnTimer(int dt,DWORD flag)
{
	PRE_WORK();

	if (!_IsReadOnly())
	{
		BOOL keyState[5];
		// get Key state.
		keyState[0] = (GetAsyncKeyState(VK_LEFT)&0x8000);
		keyState[1] = (GetAsyncKeyState(VK_RIGHT)&0x8000);
		keyState[2] = (GetAsyncKeyState(VK_UP)&0x8000);
		keyState[3] = (GetAsyncKeyState(VK_DOWN)&0x8000);
		keyState[4] = (GetAsyncKeyState(VK_SHIFT)&0x8000);

		BOOL keyPressed = keyState[0]||keyState[1]||keyState[2]||keyState[3];
		if(FALSE==keyPressed)
			_time = 0;
		else
		{
			//moving speed change by time flowing.
			int unit = _GetUnit(_time);
			if(unit>0)
				_KeyMove(unit,keyState);

			_time++;
		}
	}

	_Update();
	
	return TRUE;
}

struct _AccelerateItem
{
	_AccelerateItem(float v0,float v1){fRange = v0;fRatio = v1;}
	float fRange;
	float fRatio;
};

int CGuiAgent_ShellModifier::_GetUnit(int t)
{
	static float fCur = 0;
	static float fSeg = 0;

	if(t==0){
		fCur = 0;
		fSeg = 0;
	}

	const _AccelerateItem accelerTable[] = {
		_AccelerateItem(2.0f,0.2f),
		_AccelerateItem(4.0f,0.5f),
		_AccelerateItem(4.0f,1.0f),
		_AccelerateItem(8.0f,2.0f),
		_AccelerateItem(8.0f,4.0f)
	};
	
	float total = 0;
	int i = 0;
	for(;i<5;i++)
	{
		total += accelerTable[i].fRange;
		if(fCur < total)
			break;
	}
	i = min(i,4);// climp to [0,2];

	fCur += accelerTable[i].fRatio;
	fSeg += accelerTable[i].fRatio;
	
	int unit = int(fSeg); 
	fSeg = fSeg - unit;
	
	return unit;
}

void CGuiAgent_ShellModifier::_Update()
{
// 	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
// 	assert(data);
// 
// 	IProto * proto = NULL;
// 	if(data->lib)
// 		proto = data->lib->ObtainProto(data->protoid);
// 
// 	if(!proto)
// 		return;
// 
// 	BOOL bUpdate = FALSE;
// 	
// 	GuiData_ProtoShellInfo * dataShellInfo = (GuiData_ProtoShellInfo *)FindData("protoshellinfo");
// 	assert(dataShellInfo);
// 	if(dataShellInfo->bDirty)
// 	{			
// 		_UpdateGuiList();		
// 		_ProtoNodes2Rects();	
// 		dataShellInfo->bDirty = FALSE;	
// 		bUpdate = TRUE;
// 	}
// 
// 	// 检测Proto是否发生变化，如果发生改变更新Rect 列表
// 	if(proto->GetVer()!=_ver)
// 	{
// 		_UpdateGuiList();
// 		_ProtoNodes2Rects();
// 		bUpdate = TRUE;
// 	}
// 	
// 	if(data->sels.size()!=_idprotos.size())
// 	{
// 		bUpdate = TRUE;
// 		_UpdateGuiList();
// 		_ProtoNodes2Rects();
// 	}
// 	else
// 	{
// 		for(int i = 0;i<data->sels.size();i++)
// 			if(data->sels[i	]!=_idprotos[i])
// 			{
// 				bUpdate = TRUE;
// 				_UpdateGuiList();
// 				_ProtoNodes2Rects();
// 				break;
// 			}
// 	}
// 	
// 	if(bUpdate)
// 	{
// 		_UpdateRects(proto);
// 		_Redraw(FALSE);
// 	}
}

Prop_Sx4 * CGuiAgent_ShellModifier::_CheckNode(IProto * proto,int idxNode,int idx)
{
	Prop_Sx4 * prop = NULL;

	if(NULL==proto||idxNode>=_nodes.size()||idxNode<0||idx<0)
		return NULL;

	_ShellNode &nodeSel = _nodes[idxNode];
	IProtoNode * nodeProto = proto->GetNode(nodeSel.nodeProtoID);
	if(nodeProto)
	{
		prop =(Prop_Sx4 *)nodeProto->GetPropData(idx);
		if(prop&&prop->CheckClassName("Prop_Sx4"))
			return prop;
	}

	return NULL;
}

BOOL CGuiAgent_ShellModifier::_GetPropRect(IProto * proto,int iNode,int iProp,i_math::recti & rc)
{
	Prop_Sx4 * prop = _CheckNode(proto,iNode,iProp);
	if(prop){
		_ShellNode & nodeShell = _nodes[iNode];

		rc = RelativeAreaToLocalRect(nodeShell.rcParent,prop->v,(RectAlign)nodeShell.alignType);
		rc += nodeShell.rcParent.UpperLeftCorner;
		
		return TRUE;
	}

	return FALSE;
}

BOOL CGuiAgent_ShellModifier::OnDraw()	
{
	PRE_WORK();

	IRenderPort * rp = GetRP();
	
	IProto * proto = NULL;
	if(data->lib)
		proto = data->lib->ObtainProto(data->protoid);

	if(!proto)
		return TRUE;

	BOOL bMain = FALSE;
	if(data->sels.size()>0)
	{
		_ShellNode &nodeSel = _nodes[_iRcEdit];
		if(NULL==_CheckNode(proto,_iRcEdit,_iProp))
			return TRUE;
		bMain = nodeSel.bMain;
	}
	
	Prop_Sx4 * prop = NULL;

	if(_iRc>=0)
		rp->FrameRect(_rcActive,0xffff00ff);
	
	for(int i = 0;i<_rects.size();i++)
	{
		_Rect &irc = _rects[i];
		i_math::recti rc;

		if(prop=_CheckNode(proto,irc.iNode,irc.iProp))
		{
			_ShellNode &nodeShell = _nodes[irc.iNode];
			
			_GetPropRect(proto,irc.iNode,irc.iProp,rc);
			
			rp->FrameRect(rc,0xffee8800);

			if(nodeShell.bMain)
			{
				rc.UpperLeftCorner.x += 2;
				rc.UpperLeftCorner.y += 2;
				rc.LowerRightCorner.x -= 2;
				rc.LowerRightCorner.y -= 2;
				rp->FrameRect(rc,0xffee8800);
			}
			// draw rect select.
		}
	}

	if(_CheckNode(proto,_iRcEdit,_iProp))
	{	
		rp->FrameRect(_rcEdit,0xffff0000);
		
		if(bMain)
		{
			i_math::recti rc = _rcEdit;
			rc.UpperLeftCorner.x += 2;
			rc.UpperLeftCorner.y += 2;
			rc.LowerRightCorner.x -= 2;
			rc.LowerRightCorner.y -= 2;
			rp->FrameRect(rc,0xffff0000);
		}
	}
	
	
	// draw shell areas
	if(TRUE)
	{
		i_math::recti rcScreen(0,0,SHELL_STANDARD_WIDTH,SHELL_STANDARD_HEIGHT);
		rp->FrameRect(rcScreen,0x55ffffff);
	}

	return TRUE;
}
BOOL CGuiAgent_ShellModifier::OnLButtonClick(int x,int y,DWORD flag)
{
	PRE_WORK();

	if(_Op == Drag_None||_Op == Drag_Move)
	{
		_SelActiveArea(x,y);

		int i = 0;
		for(;i<_rects.size();i++)
			if(_rects[i].iNode==_iRc&&_rects[i].iProp==_iRcProp)
				break;

		if(_rects.size()&&i<_rects.size()&&!(flag&CtrlOpFlag_ShiftDown))
			_SelNodeByUI(x,y,flag);

	}

	return (_iRc<0);
}
BOOL CGuiAgent_ShellModifier::OnLButtonDown(int x,int y,DWORD flag)
{
	PRE_WORK();

// 	if (_IsReadOnly())
// 		return TRUE;

	if(_Op == Drag_Move||_Op == Drag_None)
	{
		_SelActiveArea(x,y);
		
		if(_iRc<0||_iRcProp<0)
			return TRUE;
	
		int i = 0;
		for(;i<_rects.size();i++)
			if(_rects[i].iNode==_iRc&&_rects[i].iProp==_iRcProp)
				break;

		//更新鼠标图标 指示移动状态
		if(!_IsReadOnly())
			_IndicateMove(x,y);

		if(i<_rects.size()&&!(flag&(CtrlOpFlag_CtrlDown|CtrlOpFlag_ShiftDown)))
		{
			if(!_IsReadOnly())
				_Op = Drag_Move;
		}
		else
		{
			_SelNodeByUI(x,y,flag);	
		}
	}

	CGuiAgent_Dragger<TRUE,FALSE>::OnLButtonDown(x,y,flag);

	return FALSE;
}

//当检测到外部改变了选中状态时，由改函数处理
void CGuiAgent_ShellModifier::_ProtoNodes2Rects()
{
	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	assert(data);

	std::vector<_Rect> temp;

	//清除当前的Rect状态
	{
		temp.swap(_rects);
		_iRc = -1;
	}
	
	//从已有的状态中恢复选中状态
	std::vector<ProtoNodeID> selNode;
	for(int i = 0;i<data->sels.size();i++)
	{
		
		BOOL bNoHandle = TRUE;

		for(int j = 0;j<temp.size();j++)
		{
			if(data->sels[i]==temp[j].protoID)
			{
				_rects.push_back(temp[j]);
				bNoHandle = FALSE;
			}
		}
		// 未处理的选中状态
		if(bNoHandle)
			selNode.push_back(data->sels[i]);
	}

	// 从ProtoNode到Rect的映射
	for(int i = 0;i<selNode.size();i++)
	{
		ProtoNodeID & nodeId = selNode[i];
		
		int j=0;
		for(;j<_nodes.size();j++)
		{
			_ShellNode & nodeShell = _nodes[j];    //查找对应的Main Rect项
			if(nodeShell.bMain&&nodeShell.nodeProtoID==nodeId)
			{
				_Rect irc;
				irc.iNode = j;
				irc.iParent = nodeShell.iParent;
				irc.iProp = nodeShell.idxs[0];
				irc.protoID = nodeShell.nodeProtoID;

				_rects.push_back(irc);			
				break;
			}
		}
		//断言必定能找到对应的Rect.
	}

	// 更新最近选中项
	if(_rects.size()>0)
	{
		_Rect &irc = _rects.back();
	
		IProtoNode * nodeProto = NULL;

		_ShellNode & nodeShell = _nodes[_iRcEdit];
		
		IProto * proto = data->lib->ObtainProto(data->protoid);

		// validate property,since it may be changed under way.

		// 检测旧的选中项是否存在，如果存在选中原有项
		{
			int r = 0;
			for(;r<_rects.size()&&!(_rects[r].iNode==_iRcEdit&&_rects[r].iProp==_iRcProp&&_rects[r].protoID==_protoID);r++);
			
			if(_iRcEdit<0||r>=_rects.size())
			{
				_iProp = irc.iProp;
				_iRcEdit = irc.iNode;
				_protoID = irc.protoID;
			}
		}
	
		//更新选择区域 _rcEdit;
		_GetPropRect(proto,_iRcEdit,_iProp,_rcEdit);
	}
	else
	{
		_iRcEdit = -1;
	}

	_idprotos.assign(data->sels.begin(),data->sels.end());
}
void CGuiAgent_ShellModifier::_Reset()
{
	_iProp = -1;
	_iRc = -1;
	_rects.clear();
}

//judge which shellnode is hited.
void CGuiAgent_ShellModifier::_SelActiveArea(int x,int y)
{
	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	assert(data);

	IProto * proto = NULL;
	proto = data->lib->ObtainProto(data->protoid);
	if(!proto)
		return;
	
	std::vector<int> selNodes;
	std::vector<int> selProps;

	i_math::recti rc ;

	// 搜索所有的节点及其属性点，并判断是否在其内
	for(int i = _nodes.size()-1;i>=0;i--)
	{
		_ShellNode & nodeShell  = _nodes[i];
		IProtoNode * nodeProto = proto->GetNode(nodeShell.nodeProtoID);
		
		if(NULL==nodeProto)
			continue;

		int nProps = nodeProto->GetPropCount();
		int r = 0;

		for(;r<nodeShell.c&&nodeProto;r++)
		{
			// judge prop index is correct.
			int idxProp = nodeShell.idxs[r];
			if( idxProp>=nProps)
				continue;
			
			if(_GetPropRect(proto,i,idxProp,rc)&&rc.isPointInside(x,y)){
				_activeprotoID = nodeShell.nodeProtoID;
				selNodes.push_back(i);
				selProps.push_back(nodeShell.idxs[r]);		
			}
		}
	}
	

	// 从结果集中找到最合适的 节点
	if(selNodes.size()>0)
	{
		std::vector<int> curNodes;
		std::vector<int> curProps;

		_ShellNode &snode = _nodes[selNodes[0]];

		for(int i = 0;i<selNodes.size();i++)
		{
			int iNode = selNodes[i];
			int iProp = selProps[i];

			_ShellNode & node = _nodes[iNode];
	
			if(node.iParent==snode.iParent){  // 与第一个选中的节点 具有相同的父节点（bMain）
				curNodes.push_back(iNode);
				curProps.push_back(iProp);
			}
		}
		
		if(curNodes.size()==1){
			_iRc = curNodes[0];
			_iRcProp = curProps[0];
		}
		else{
			i_math::recti rcMin;
			int minNode = 0;
			_GetPropRect(proto,curNodes[0],curProps[0],rcMin);
			for(int i = 1;i<curNodes.size();i++)
			{
				i_math::recti rc;
				if(_GetPropRect(proto,curNodes[i],curProps[i],rc)&&rc.getArea()<rcMin.getArea()){
					minNode = i;
					rcMin = rc;
				}
			}
			_iRc = curNodes[minNode];
			_iRcProp = curProps[minNode];
		}
	}
	else
	{
		_iRc = -1;
		_iRcProp = -1;
	}

	_activeprotoID = _GetNodeProtoID(_iRc);

	_GetPropRect(proto,_iRc,_iRcProp,_rcActive);
}

ProtoNodeID CGuiAgent_ShellModifier::_GetNodeProtoID(int idx)
{
	if(idx<_nodes.size()&&idx>=0)
	{
		_ShellNode & nodeShell  = _nodes[idx];
		return nodeShell.nodeProtoID;
	}
	return ProtoNodeID_Null;
}
// 从UI编辑窗口选中对应的Rect
void CGuiAgent_ShellModifier::_SelNodeByUI(int x,int y,DWORD flag)
{

	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	assert(data);

	_SelActiveArea(x,y);

	int i = 0;
	for(;i<_rects.size();i++)
		if(_rects[i].iNode==_iRc&&_rects[i].iProp==_iRcProp)
			break;
	
	BOOL bIn = (_rects.size()!=0&&i<_rects.size());
	BOOL bShif = flag&CtrlOpFlag_ShiftDown;
	BOOL bCtrl = flag&CtrlOpFlag_CtrlDown;


	//如果并非多选将清除已有的选中项
	if((!bShif&&!bCtrl))
	{
		if(_iRc>=0&&_iRcProp>=0) // 清除选中状态
			data->sels.clear();
	
		_rects.clear();
		bIn = FALSE;
	}

	//存在被选中的Rect
	if(_iRc>=0) 
	{
		_ShellNode & nodeShell = _nodes[_iRc];
		_Rect irc;
		irc.iNode = _iRc;
		irc.iParent = nodeShell.iParent;
		irc.iProp = _iRcProp;
		irc.protoID = nodeShell.nodeProtoID;

		int i = 0;
		for(;i<_rects.size()&&!(_rects[i]==irc);i++);
		
		BOOL bRemove = FALSE;
		for(i = 0;i<_rects.size();i++)
		{
			_Rect & rc = _rects[i];
			if(irc==rc)
			{
				bRemove = TRUE;
				break;
			}
		}

		//该区域已经是选中状态
		if(i<_rects.size()&&bShif&&bRemove) 
		{	

			if(nodeShell.bMain)
			for(int n = 0;n<data->sels.size();n++)
			{
				if(data->sels[n]==nodeShell.nodeProtoID)
				{
					data->sels.erase(data->sels.begin()+n);
					break;
				}
			}

			_rects.erase(_rects.begin()+i);

			if(_iRc==_iRcEdit)
			{
				if(_rects.size()>0)
				{
					_Rect &rclst = _rects.back();
					_iRcEdit = rclst.iNode;
					_iProp = rclst.iProp;
					_protoID = rclst.protoID;
				}
				else
				{
					_iRcEdit = -1;
					_iProp = -1;
				}

				_UpdateRects(data->proto());
			}

		}
		else
		{

			_iRcEdit = _iRc;
			_iProp = _iRcProp;
			_protoID = _activeprotoID;
			
			if(!bIn)
				_rects.push_back(irc);
		
			int i = 0;
			for(;i<data->sels.size()&&data->sels[i]!=nodeShell.nodeProtoID;i++);

			//更新
			if(i>=data->sels.size())
				data->sels.push_back(nodeShell.nodeProtoID);
		}

		//更新同步状态
		_idprotos.assign(data->sels.begin(),data->sels.end());
	}

	_UpdateRects(data->proto());
}

void CGuiAgent_ShellModifier::_UpdateRects(IProto * proto)
{
	_GetPropRect(proto,_iRcEdit,_iProp,_rcEdit);
	_GetPropRect(proto,_iRc,_iRcProp,_rcActive);
}
BOOL CGuiAgent_ShellModifier::OnLButtonUp(int x,int y,DWORD flag)
{
	extern void ScaleLen(IRenderPort *rp,const i_math::vector3df& pos,float &len);
	i_math::vector3df pos(0,0,0);
	float len = 1.0f;
	ScaleLen(GetRP(),pos,len);

	PRE_WORK();

	OnEndDrag(x,y,flag);
	
	_bInDrag=FALSE;
	DiscardFocus(OpType_Mouse);
	
	return TRUE;
}
void CGuiAgent_ShellModifier::OnEndDrag(int x,int y,DWORD flag)
{
	CHECK_RUNNING(VOID_RET);

	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	assert(data);											

	IProto * proto = NULL;
	if(data->lib)
		proto = data->lib->ObtainProto(data->protoid);

	if((!proto)||data->sels.size()==0||_iProp<0)
		return;
	
	if(_Op != Drag_None){
		IProtoNode * nodeProto = proto->GetNode(data->sels[0]);
		if(nodeProto)
			nodeProto->IncVer();	
	}
	
	if(_Op==Drag_Move)
	{
		_Op = Drag_None;
		OnSetCursor(0,0,0);
	}

	data->bChanging = FALSE;
}
void CGuiAgent_ShellModifier::_IndicateMove(int x,int y)
{
	BOOL bShift = (GetAsyncKeyState(VK_SHIFT)&0x8000);

	if(_iRcEdit>=0&&_iProp>=0)
	{
		bool inside = _rcEdit.isPointInside(x,y);
		if(inside&&_Op==Drag_None&&_iRcEdit==_iRc&&!bShift)
		{
			_Op = Drag_Move;
			_SetCursor(IDC_CURSORMOVE);
		}
	}
}
BOOL CGuiAgent_ShellModifier::OnMouseMove(int x,int y,DWORD flag)
{
	PRE_WORK();
	
	if(!_bInDrag)
	{	
		_SelActiveArea(x,y);
		_Op = Drag_None;
		if(!_IsReadOnly())
			_DecideOperateType(x,y);
		_Redraw(FALSE);
	}
	
	//只读状态不响应
	if(!_IsReadOnly())
		_IndicateMove(x,y);

	
	CGuiAgent_Dragger<TRUE,FALSE>::OnMouseMove(x,y,flag);

	return TRUE;
}
BOOL CGuiAgent_ShellModifier::OnBeginDrag(int x,int y,DWORD flag)
{
	_x = x;
	_y = y;

	PRE_WORK();

	IProto * proto = NULL;
	if(data->lib)
		proto = data->lib->ObtainProto(data->protoid);

	if((!proto)||_iProp<0||_iRcEdit<0)
		return TRUE;

	_ShellNode & nodeShell = _nodes[_iRcEdit];

	IProtoNode * nodeProto = proto->GetNode(nodeShell.nodeProtoID);
	if(!nodeProto)
		return TRUE;

	Prop_Sx4 * prop = (Prop_Sx4*)nodeProto->GetPropData(_iProp);
	if(prop&&prop->CheckClassName("Prop_Sx4"))
		_rcInit = RelativeAreaToLocalRect(nodeShell.rcParent,prop->v,(RectAlign)nodeShell.alignType);
	
	_rectinits.resize(_rects.size());
	for(int i = 0;i<_rects.size();i++)
	{
		if(_rects[i].iNode>=_nodes.size())
			continue;
		
		_ShellNode & nodeSh = _nodes[_rects[i].iNode];
		
		nodeProto = proto->GetNode(nodeSh.nodeProtoID);
		prop = (Prop_Sx4*)nodeProto->GetPropData(_rects[i].iProp);
		if(prop&&prop->CheckClassName("Prop_Sx4"))
			_rectinits[i] = RelativeAreaToLocalRect(nodeSh.rcParent,prop->v,(RectAlign)nodeSh.alignType) ;
	}

	data->bChanging = TRUE;

	return TRUE;
}
void CGuiAgent_ShellModifier::OnDrag(int x,int y,DWORD flag)
{
	CHECK_RUNNING(VOID_RET);
	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	assert(data);											

	IProto * proto = NULL;
	if(data->lib)
		proto = data->lib->ObtainProto(data->protoid);

	if((!proto)||_iRcEdit<0||_iProp<0)
		return;
	

	int offX = x - _x;
	int offY = y - _y;
	
	if(_Op!=Drag_None&&_Op!=Drag_Move&&_iRcEdit>=0)
	{
		_ShellNode & nodeSh = _nodes[_iRcEdit];

		IProtoNode * nodeProto = proto->GetNode(nodeSh.nodeProtoID);
		if(!nodeProto)
			return;

		Prop_Sx4 * prop = (Prop_Sx4*)nodeProto->GetPropData(_iProp);
		assert(prop->CheckClassName("Prop_Sx4"));

		int vx = offX;
		int vy = offY;

		i_math::recti rc = RelativeAreaToLocalRect(nodeSh.rcParent,prop->v,(RectAlign)nodeSh.alignType);

		if(_Op==Drag_Right||_Op==Drag_UpRight||_Op==Drag_LowRight)
			rc.LowerRightCorner.x = _rcInit.LowerRightCorner.x + vx;
		if(_Op==Drag_Left||_Op==Drag_UpLeft||_Op==Drag_LowLeft)
			rc.UpperLeftCorner.x  = _rcInit.UpperLeftCorner.x + vx;

		if(_Op==Drag_Top||_Op==Drag_UpLeft||_Op==Drag_UpRight)
			rc.UpperLeftCorner.y = _rcInit.UpperLeftCorner.y + vy;
		if(_Op==Drag_Bottom||_Op==Drag_LowLeft||_Op==Drag_LowRight)
			rc.LowerRightCorner.y = _rcInit.LowerRightCorner.y + vy;

		i_math::recti rcAdj = rc;

		BOOL bIncreaseX = (prop->v.z - prop->v.x)<rc.getWidth();
		BOOL bIncreaseY = (prop->v.w - prop->v.y)<rc.getHeight();
		
		i_math::recti rcLocal = RelativeAreaToLocalRect(nodeSh.rcParent,prop->v,(RectAlign)nodeSh.alignType);

		switch(_Op)
		{
		case Drag_Left:
			CheckValid(rc,bIncreaseX,bIncreaseY,TRUE,FALSE,rcLocal,&rcAdj);
			break;
		case Drag_Right:
			CheckValid(rc,bIncreaseX,bIncreaseY,FALSE,FALSE,rcLocal,&rcAdj);
			break;
		case Drag_Top:
			CheckValid(rc,bIncreaseX,bIncreaseY,FALSE,TRUE,rcLocal,&rcAdj);
			break;
		case Drag_Bottom:
			CheckValid(rc,bIncreaseX,bIncreaseY,FALSE,FALSE,rcLocal,&rcAdj);
			break;
		case Drag_UpLeft:
			CheckValid(rc,bIncreaseX,bIncreaseY,TRUE,TRUE,rcLocal,&rcAdj);
			break;
		case Drag_UpRight:
			CheckValid(rc,bIncreaseX,bIncreaseY,FALSE,TRUE,rcLocal,&rcAdj);
			break;
		case Drag_LowLeft:
			CheckValid(rc,bIncreaseX,bIncreaseY,TRUE,FALSE,rcLocal,&rcAdj);
			break;
		case Drag_LowRight:
			CheckValid(rc,bIncreaseX,bIncreaseY,FALSE,FALSE,rcLocal,&rcAdj);
			break;
		default:
			break;
		}

		FORCE_TYPE(i_math::recti,prop->v) = LocalRectToRelativeArea(nodeSh.rcParent,rcAdj,(RectAlign)nodeSh.alignType);
			
		_rcEdit  = rcAdj;
		_rcEdit += nodeSh.rcParent.UpperLeftCorner;

		_rcActive = _rcEdit;
	}


		
	if(_Op==Drag_Move)
		_Move(proto,offX,offY,FALSE);

	_Redraw(FALSE);
}
void CGuiAgent_ShellModifier::_Move(IProto * proto,int offx,int offy,BOOL bAbsoulte /*= FALSE*/)
{
	std::vector<int> idxs;

	for(int i = 0;i<_rects.size();i++)
	{
		_Rect & rc = _rects[i];
		if(rc.iParent<0)
		{
			idxs.push_back(i);
			continue;
		}
	
		BOOL bParent = FALSE;
		
		int iParent = rc.iParent;
		
		while(iParent>=0)
		{
			int j = 0;
			for(;j<_rects.size()&&_rects[j].iNode!=iParent;j++);
			if(j<_rects.size())
			{
				bParent = TRUE;
				break;
			}
			iParent = _nodes[iParent].iParent;
		}

		if(!bParent)
			idxs.push_back(i);
	}

	for(int i = 0;i<idxs.size();i++)
	{
		int idx = idxs[i];

		_Rect &rc = _rects[idx];
		_ShellNode & nodeSh = _nodes[rc.iNode];

		IProtoNode * nodeProto = proto->GetNode(nodeSh.nodeProtoID);
		if(nodeProto)
		{
			Prop_Sx4 * prop = (Prop_Sx4 *)nodeProto->GetPropData(rc.iProp);
			assert(prop->CheckClassName("Prop_Sx4"));
			
			if(bAbsoulte)
			{
				i_math::recti rcLocal = RelativeAreaToLocalRect(nodeSh.rcParent,prop->v,(RectAlign)nodeSh.alignType);
				rcLocal += i_math::pos2di(offx,offy);
				FORCE_TYPE(i_math::recti,prop->v)= LocalRectToRelativeArea(nodeSh.rcParent,rcLocal,(RectAlign)nodeSh.alignType);

				int c = 0;
				c++;
			}
			else
			{
				i_math::recti rcLocal = _rectinits[idx];
				rcLocal += i_math::pos2di(offx,offy);
				FORCE_TYPE(i_math::recti,prop->v)= LocalRectToRelativeArea(nodeSh.rcParent,rcLocal,(RectAlign)nodeSh.alignType);

				int c = 0;
				c++;
			}
		}
	// 更新
	}

	_UpdateGuiList();

	_UpdateRects(proto);
}

BOOL CGuiAgent_ShellModifier::OnSetCursor(int x,int y,DWORD flag)
{
	PRE_WORK();

	switch(_Op)
	{
	case Drag_Left:
	case Drag_Right:
		_SetCursor(IDC_CURSORLRMOVE);
		break;
	case Drag_Top:
	case Drag_Bottom:
		_SetCursor(IDC_CURSORULMOVE);
		break;
	case Drag_UpLeft:
	case Drag_LowRight:
		_SetCursor(IDC_CURSORSL1MOVE); 
		break;
	case Drag_UpRight:
	case Drag_LowLeft:
		_SetCursor(IDC_CURSOR_SL0MOVE);
		break;
	case Drag_None: 
		_SetCursor(NULL);
		break;
	}
	
	if(_Op==Drag_Move)
		_SetCursor(IDC_CURSORMOVE);
	
	return TRUE;
}
void CGuiAgent_ShellModifier::_DecideOperateType(int x,int y)
{
	int rng = 10;
	
	BOOL bShift = (GetAsyncKeyState(VK_SHIFT)&0x8000);
	
	GuiData_Proto* data = (GuiData_Proto*)FindData("proto");
	IProto * proto = NULL;
	if(data&&data->lib)
		proto = data->lib->ObtainProto(data->protoid);
	
	_Op = Drag_None;

	if(_CheckNode(proto,_iRcEdit,_iProp))
	{
		i_math::recti rcLeft,rcRight,rcTop,rcBottom;
		
		// left
		rcLeft.UpperLeftCorner.x = _rcEdit.UpperLeftCorner.x;
		rcLeft.UpperLeftCorner.y = _rcEdit.UpperLeftCorner.y;
		rcLeft.LowerRightCorner.x = _rcEdit.UpperLeftCorner.x + rng;
		rcLeft.LowerRightCorner.y = _rcEdit.LowerRightCorner.y;
		
		// right
		rcRight.UpperLeftCorner.x = _rcEdit.LowerRightCorner.x - rng;
		rcRight.UpperLeftCorner.y = _rcEdit.UpperLeftCorner.y;
		rcRight.LowerRightCorner.x = _rcEdit.LowerRightCorner.x;
		rcRight.LowerRightCorner.y = _rcEdit.LowerRightCorner.y;
		
		//top
		rcTop.UpperLeftCorner.x = _rcEdit.UpperLeftCorner.x;
		rcTop.UpperLeftCorner.y = _rcEdit.UpperLeftCorner.y;
		rcTop.LowerRightCorner.x = _rcEdit.LowerRightCorner.x;
		rcTop.LowerRightCorner.y = _rcEdit.UpperLeftCorner.y + rng;

		// bottom
		rcBottom.UpperLeftCorner.x = _rcEdit.UpperLeftCorner.x;
		rcBottom.UpperLeftCorner.y = _rcEdit.LowerRightCorner.y - rng;
		rcBottom.LowerRightCorner.x = _rcEdit.LowerRightCorner.x;
		rcBottom.LowerRightCorner.y = _rcEdit.LowerRightCorner.y;

		BOOL bLeft,bRight,bTop,bBottom,bMid;
		
		bLeft = rcLeft.isPointInside(x,y);
		bRight = rcRight.isPointInside(x,y);
		bTop = rcTop.isPointInside(x,y);
		bBottom = rcBottom.isPointInside(x,y);
		bMid = _rcEdit.isPointInside(x,y);

		if(bRight&&bBottom)
			_Op = Drag_LowRight;
		else if(bLeft&&bTop)
			_Op = Drag_UpLeft;
		else if(bRight&&bTop)
			_Op = Drag_UpRight;
		else if(bLeft&&bBottom)
			_Op = Drag_LowLeft;
		else if(bLeft)
			_Op = Drag_Left;
		else if(bRight)
			_Op = Drag_Right;
		else if(bTop)
			_Op = Drag_Top;
		else if(bBottom)
			_Op = Drag_Bottom;
		else if(!bShift)
		{
			for (int i = 0;i<_rects.size();i++)
			{
				if(_rects[i].iNode==_iRc&&_rects[i].iProp==_iRcProp)
				{
					_Op = Drag_Move;
					break;
				}
			}			
		}
	}
}

void CGuiAgent_ShellModifier::_KeyMove(int unit,const BOOL * keyState)
{
	CGuiView * view =(CGuiView *) GetView();
	CWnd * pWnd = view->GetWnd();
	
	CWnd * pFocus = CWnd::GetFocus();

	if(pWnd!=pFocus) //insure be focused on me.
		return;
	
	// get Key state.
	BOOL bLeft = keyState[0];
	BOOL bRight = keyState[1];
	BOOL bUp = keyState[2];
	BOOL bDown = keyState[3];
	BOOL bShift = keyState[4];
	
	static BOOL bChange = FALSE;
	
	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	assert(data);
	IProto * proto = NULL;
	if(data->lib)
		proto = data->lib->ObtainProto(data->protoid);
	
	// if proto is not exist,then return
	if(!proto||_iRcEdit<0||_iProp<0)
		return ;

	IProtoNode * nodeProto = proto->GetNode(_nodes[_iRcEdit].nodeProtoID);
	
	// if no key has pressed,then return
	if(!(bLeft||bRight||bUp||bDown))
	{
		if(bChange&&(!bShift))
		{
			_Op = Drag_None;
			nodeProto->IncVer();
			bChange = FALSE;
		}
		return;
	}
	
	// if proto node is NULL then return.
	if(NULL==nodeProto)
		return;

	int vl = 0,vr = 0,vt = 0,vb = 0;

	if(bUp) vt = -unit;
	if(bDown) vb = unit;
	if(bLeft) vl = -unit;
	if(bRight) vr = unit;
	

	BOOL oldState = bChange;

	// if Shift Key is not pressed move the rect.
	if(FALSE==bShift)
	{
		_Op = Drag_Move;
		bChange = TRUE;
		_Move(proto,vl+vr,vt+vb,TRUE);
	}
	else
	{
		_ShellNode & nodeShell = _nodes[_iRcEdit];

		Prop_Sx4 * prop = (Prop_Sx4*)nodeProto->GetPropData(_iProp);
		assert(prop->CheckClassName("Prop_Sx4"));

		i_math::recti rc;
		memcpy(&rc,&(prop->v),sizeof(rc));

		
		// caculate new size.
		_Op = Drag_LowRight;
		rc.LowerRightCorner.x += vl + vr;
		rc.LowerRightCorner.y += vt + vb;
	
		// decide if prop data need updating.
		BOOL bX = FALSE,bY = FALSE;

		bX = (rc.LowerRightCorner.x > rc.UpperLeftCorner.x + 10);
		bY = (rc.LowerRightCorner.y > rc.UpperLeftCorner.y + 10);

		int w = prop->v.z - prop->v.x;
		int h = prop->v.w - prop->v.y;
		
		// prop data can be changed when size will be increased 
		// or width(height) of it is larger than 10 after changed.
		bX = bX||w<rc.getWidth();
		bY = bY||h<rc.getHeight();
		
		// update prop data
		if(bX){
			prop->v.x = rc.UpperLeftCorner.x;
			prop->v.z = rc.LowerRightCorner.x;	
		}
		if(bY)	{
			prop->v.y = rc.UpperLeftCorner.y;
			prop->v.w = rc.LowerRightCorner.y;
		}
		
		bChange = bX||bY;
		if(bChange){	
			_UpdateGuiList();
			i_math::pos2di pt;
			_GetCursorPos(pt);
			_UpdateRects(proto);
		}
	}

	// if change redraw view port.
	if(bChange){
		_Redraw(FALSE);
	}
}
BOOL CGuiAgent_ShellModifier::OnKeyDown(char c,DWORD flag)
{
	PRE_WORK();	
	
	BOOL bSel = (_rects.size()>0);
	
	return !bSel; //存在选中的Proto不再向外转发 消息
}
void CGuiAgent_ShellModifier::OnDetachView(CGeView *view,DWORD iLevel)
{
	SAFE_DELETE(_newMenu);
}
BOOL CGuiAgent_ShellModifier::OnRButtonClick(int x, int y,DWORD flag)
{
	PRE_WORK();

	if (_IsReadOnly())
		return TRUE;

	_SelActiveArea(x,y);

	// 
	int i = 0;
	for(;i<_rects.size();i++)
		if(_rects[i].iNode==_iRc&&_rects[i].iProp==_iRcProp)
			break;
	if(i>=_rects.size())
		_rects.clear();
//	if(i>=_rects.size())
//		_SelNodeByUI(x,y,0);
//	else
	_SelNodeByUI(x,y,CtrlOpFlag_CtrlDown);
	
	_Redraw(FALSE);

	 CGuiActor_Proto * actor = (CGuiActor_Proto *)_GetActor();	 
	 assert(actor);
	
	SAFE_DELETE(_newMenu);
	_newMenu = new CMenu();
	_newMenu->CreatePopupMenu();
	
	if(_iRcEdit>=0&&_iProp>=0)
	{
		_ShellNode & nodeShell = _nodes[_iRcEdit];

		for(int i = 0;i<nodeShell.images.size();i++)
		{
			if(ID_AGENT_ADJUST_SIZE_START + i>=ID_AGENT_ADJUST_SIZE_END) 
				continue;
			_newMenu->InsertMenu(0, MF_STRING, ID_AGENT_ADJUST_SIZE_START + i, fromMBCS(nodeShell.images[i].c_str()));
		}
	}


	 CMenu * menu = _GetMenu();
	 actor->BuildProtoTreeMenu(_GetMenu());
	 
//	 int n = 0;
//	 for(int i = 0;i<_rects.size();i++)
//	 {
//		 _ShellNode & node = _nodes[_rects[i].iNode];
//		 if(node.bMain)
//		 {
//			 n++;
//			 if(n>1)
//				 break;
//		 }
//	 }
//	 if(n==1)
	 {
		 _AddMenuSep();
		 menu->AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)_newMenu->GetSafeHmenu(), _T("Auto Size"));
		_AddMenuSep();
	 }

	return TRUE;
}

void CGuiAgent_ShellModifier::CheckValid(i_math::recti &rc,BOOL bX,BOOL bY,BOOL bLeft,BOOL bTop,i_math::recti & rcSrc,i_math::recti * rcAdj/* = NULL*/)
{
	BOOL bXIncrease = bX , bYIncrease  = bY;

	bX = (rc.LowerRightCorner.x > rc.UpperLeftCorner.x + 10);
	bY = (rc.LowerRightCorner.y > rc.UpperLeftCorner.y + 10);
	
	BOOL bSrcX,bSrcY;

	bSrcX = rcSrc.LowerRightCorner.x > rcSrc.UpperLeftCorner.x + 10;
	bSrcY = rcSrc.LowerRightCorner.y > rcSrc.UpperLeftCorner.y + 10;

	if(rcAdj)
	{
		*rcAdj = rc;

		if(!bX)
		{
			if(FALSE == bXIncrease&&bSrcX)
			{
				if(bLeft)
					rcAdj->UpperLeftCorner.x = rcSrc.LowerRightCorner.x - 10;
				else
					rcAdj->LowerRightCorner.x = rcSrc.UpperLeftCorner.x + 10;
			}
			else
			{
				if(bLeft)
					rcAdj->UpperLeftCorner.x = rcSrc.UpperLeftCorner.x;
				else 
					rcAdj->LowerRightCorner.x = rcSrc.LowerRightCorner.x;
			}
		}

		if(!bY)
		{
			if(FALSE == bYIncrease&&bSrcY)
			{
				if(bTop)
					rcAdj->UpperLeftCorner.y = rcSrc.LowerRightCorner.y - 10;
				else
					rcAdj->LowerRightCorner.y = rcSrc.UpperLeftCorner.y + 10;
			}
			else
			{
				if(bTop)
					rcAdj->UpperLeftCorner.y = rcSrc.UpperLeftCorner.y;
				else
					rcAdj->LowerRightCorner.y = rcSrc.LowerRightCorner.y;
			}
		}
	}
}
void CGuiAgent_ShellModifier::_UpdateGuiList()
{
	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	assert(data);
	IProto * proto = NULL;
	if(data->lib)
		proto = data->lib->ObtainProto(data->protoid);
	
	if(proto)
	{
		CNodeTree * tree = proto->GetNodeTree()->GetTree();
		_nodes.clear();
		_topnodes.clear();

		if(tree)
		{
			_EnumNode(proto,tree,NodeHandle_Root,-1,NULL,_nodes);
			
			int iOff = _topnodes.size();
			for(int i =0;i<_nodes.size();i++)
			{
				_ShellNode & nodeShell = _nodes[i];
				if(nodeShell.iParent!=-1)
					nodeShell.iParent += iOff;
				_topnodes.push_back(nodeShell);
			}
			_nodes.swap(_topnodes);
		}
		
		_ver = proto->GetVer();
	}
}


BOOL CGuiAgent_ShellModifier::_IsReadOnly()
{
	GuiData_Proto * data = (GuiData_Proto *)FindData("proto");
	if (data->IsReadOnly())
		return TRUE;
	GuiData_Debugger * dataDebugger = (GuiData_Debugger*)FindData("debugger");
	assert(dataDebugger);
	if(dataDebugger->context->IsRunning())
		return TRUE;
	return FALSE;
}
