#include "stdh.h"
#include "GuiView_Changelists.h"
#include "GuiData_Changelists.h"
#include "graphicsgraph.h"
#include "Log/LogDump.h"
#include "stringparser/stringparser.h"

#include <GdiPlus.h>

#include <algorithm>

CGuiView_Changelists::CGuiView_Changelists()
{
	_bYInverse=TRUE;
	_bReadOnly=FALSE;
	_needReCenterCur = FALSE;
}

CGuiView_Changelists::~CGuiView_Changelists()
{

}

void CGuiView_Changelists::_RefreshSnapshot(GraphicsGraph* gg)
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
	if (!data)
		return;

	CChangelists* changelists = data->_changelists;
	if (!changelists)
		return;

	ChangelistsSnapshot& snapshot = data->_snapshot;

	if ((data->GetVer() == snapshot._dataVer) && (changelists->GetVer() == snapshot._changelistsVer))
		return;//没有变化

	// 更新版本号
	snapshot._dataVer = data->GetVer();
	snapshot._changelistsVer = changelists->GetVer();
	
	// 清空快照
	snapshot.Clear();
	
	// 计算文件列表行高和可见行数
	int lineHeight = 0;
	int visibleLines = 0;
	if (gg)
	{
		i_math::size2di textSize = gg->MessureText("Tg"); // 使用包含基线和上部字符的文本来测量
		lineHeight = textSize.h + FileList_LineSpacing; // 添加行间距
		
		// 计算可见行数
		int contentHeight = ChangelistNode_Height - FileList_TopMargin - FileList_BottomMargin;
		visibleLines = contentHeight / lineHeight;
	}
	
	// 第一步：收集所有changelist节点信息，构建节点映射
	if (TRUE)
	{
		// 首先建立所有节点及其父子关系
		for (const auto& briefPair : changelists->_briefs)
		{
			const FileChangelist& cl = briefPair.second;

			ChangelistsSnapshot::Node node;
			node._uid = cl.uid;
			node._parent = cl.parent;
			node._desc = cl.desc;
			if (node._uid == changelists->_curUID)
				node._isCur = true;
			node._isCommit = cl.isCommit;
			node._lineHeight = lineHeight; // 设置行高
			node._visibleLines = visibleLines; // 设置可见行数

			snapshot._nodes[node._uid] = node;
			if (node._parent == FileChangeListUID_Invalid)
				snapshot._rootUID = node._uid;
		}
	}

	if(TRUE)
	{
		// 第二步：计算每个节点的层级（通过统计祖先节点数量）
		// 初始化最大层级
		snapshot._maxLevel = 0;
	
		// 遍历所有节点，计算其层级
		for (auto& pair : snapshot._nodes)
		{
			FileChangeListUID uid = pair.first;
			int level = 0;
			FileChangeListUID currentUID = uid;

			// 向上追溯所有祖先节点，统计层级
			while (true)
			{
				auto nodeIt = snapshot._nodes.find(currentUID);
				if (nodeIt == snapshot._nodes.end())
					break;

				// 如果已经是根节点（没有父节点），则停止计数
				if (nodeIt->second._parent == 0)
					break;

				// 移动到父节点，层级加一
				currentUID = nodeIt->second._parent;
				level++;

				// 防止循环引用导致的无限循环
				if (level > 1000)
					break;
			}

			// 设置节点层级
			pair.second._level = level;

			// 更新最大层级
			if (level > snapshot._maxLevel)
			{
				snapshot._maxLevel = level;
			}
		}
	}
	
	// 第三步：按层级收集节点并计算每层的节点数
	std::vector<std::vector<FileChangeListUID>> levelNodes(snapshot._maxLevel + 1);
	for (const auto& pair : snapshot._nodes)
	{
		int level = pair.second._level;
		levelNodes[level].push_back(pair.first);
	}
	
	// 第四步：计算每个节点在同层中的索引并设置位置
	for (int level = 0; level <= snapshot._maxLevel; ++level)
	{
		auto& nodesInLevel = levelNodes[level];
		
		// 对每层的节点进行排序以确保布局稳定性
		// 首先按照parent uid排序，如果相同则按照自身uid排序
		struct NodeSorter 
		{
			const std::unordered_map<FileChangeListUID, ChangelistsSnapshot::Node>& nodes;
			
			NodeSorter(const std::unordered_map<FileChangeListUID, ChangelistsSnapshot::Node>& n) : nodes(n) {}
			
			bool operator()(const FileChangeListUID& a, const FileChangeListUID& b) const
			{
				// 获取两个节点
				auto nodeA = nodes.find(a);
				auto nodeB = nodes.find(b);
				
				// 如果任一节点不存在，则使用默认比较
				if (nodeA == nodes.end() || nodeB == nodes.end())
				{
					return a < b;
				}
				
				// 首先比较parent uid
				if (nodeA->second._parent != nodeB->second._parent)
				{
					return nodeA->second._parent < nodeB->second._parent;
				}
				
				// 如果parent uid相同，则比较自身uid
				return a < b;
			}
		};
		
		std::sort(nodesInLevel.begin(), nodesInLevel.end(), NodeSorter(snapshot._nodes));
		
		// 计算每个节点的水平位置（x坐标）
		float levelX = level * (ChangelistNode_Width + ChangelistNode_HorPadding); // 层级 * (节点宽度 + 间距)
		
		// 遍历当前层的所有节点
		for (size_t i = 0; i < nodesInLevel.size(); ++i)
		{
			FileChangeListUID uid = nodesInLevel[i];
			auto nodeIt = snapshot._nodes.find(uid);
			if (nodeIt != snapshot._nodes.end())
			{
				auto& node = nodeIt->second;
				node._index = static_cast<int>(i);
				
				// 设置节点坐标 (x坐标)
				node._x = levelX;
			}
		}
	}
	
	// 第六步：按层级和排序顺序从上到下依次排列节点
	float yPadding = ChangelistNode_VerPadding;
	
	// 从顶层开始依次处理每一层
	for (int level = 0; level <= snapshot._maxLevel; level++)
	{
		float yOffset = 0.0f;
		
		// 遍历当前层的所有节点，按照排序后的顺序依次放置
		for (size_t i = 0; i < levelNodes[level].size(); i++)
		{
			FileChangeListUID uid = levelNodes[level][i];
			auto nodeIt = snapshot._nodes.find(uid);
			if (nodeIt == snapshot._nodes.end())
				continue;
				
			auto& node = nodeIt->second;
			
			// 设置节点y坐标，严格按照排序顺序从上到下排列
			node._y = yOffset;
			
			// 更新下一个节点的y起始位置
			yOffset += ChangelistNode_Height + ChangelistNode_HorPadding;
		}
	}
}

void CGuiView_Changelists::_ReCenterCur(GraphicsGraph* gg)
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
	if (!data)
		return;

	ChangelistsSnapshot& snapshot = data->_snapshot;

	// 查找当前节点
	auto curNode = snapshot.FindCur();
	if (!curNode)
		return;
	
	// 获取窗口大小
	i_math::recti clientRect;
	_wnd->GetClientRect((CRect*)&clientRect);
	
	// 计算需要的偏移量,使当前节点居中
	float centerX = clientRect.getWidth() / 2.0f - curNode->_x - ChangelistNode_Width / 2.0f;
	float centerY = clientRect.getHeight() / 2.0f - curNode->_y - ChangelistNode_Height / 2.0f;
	
	// 应用变换
	i_math::pos2df off(centerX, centerY);
	i_math::pos2df scale(1.0f, 1.0f);
	gg->Transform(off, scale);
	SetGGYInverse(FALSE);
	SetTransformGG(off, scale);
	
	// 重置标志
	_needReCenterCur = FALSE;
}

//绘制这个changelist里的内容(文件路径列表),要求限制在rc中,
void CGuiView_Changelists::_DrawChangelistFilePathes(GraphicsGraph* gg, const i_math::recti &rc, const ChangelistsSnapshot& snapshot, const ChangelistsSnapshot::Node& node, CChangelists& changelists,bool isSelected)
{
	// 如果节点被选中，绘制额外的边框
	if (isSelected)
	{
		i_math::recti rcNode = rc;
		gg->FrameRoundCornerRect(rcNode, 5, 0xFFFFFF, 4);
	}

	// 查找对应的changelist
	const FileChangelist* cl = nullptr;
	if (true)
	{
		auto it = changelists._briefs.find(node._uid);
		if (it != changelists._briefs.end())
		{
			cl = &it->second;
		}
	}
	
	// 计算可显示区域
	i_math::recti rcContent = rc;
	rcContent.UpperLeftCorner.y += FileList_TopMargin; // 留出空间显示标题
	rcContent.UpperLeftCorner.x += FileList_LeftMargin;  // 左边留出边距
	rcContent.LowerRightCorner.x -= FileList_RightMargin; // 右边留出边距
	rcContent.LowerRightCorner.y -= FileList_BottomMargin; // 底部留出边距
	
	// 绘制标题
	i_math::recti rcTitle = rc;
	rcTitle.LowerRightCorner.y = rcContent.UpperLeftCorner.y;
	gg->DrawText("Changed Files:", rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE, FALSE, 0xFFFFFF);
	
	// 绘制标题下方的分隔线
	i_math::recti rcDivider(
		rcContent.UpperLeftCorner.x,
		rcContent.UpperLeftCorner.y - 10,
		rcContent.LowerRightCorner.x,
		rcContent.UpperLeftCorner.y-9
	);
	gg->FillSolidRect(rcDivider, 0x000000, 255);
	
	// 如果没有changelist或者changelist为空,显示<empty>
	if (!cl || cl->changes.empty())
	{
		i_math::recti rcEmpty = rcContent;
		rcEmpty.LowerRightCorner.y = rcEmpty.UpperLeftCorner.y + FileList_TitleHeight;
		gg->DrawText("<empty>", rcEmpty, DT_LEFT | DT_VCENTER | DT_SINGLELINE, FALSE, 0x808080);
		return;
	}
		
	// 计算可显示的最大行数
	int maxLines = (rcContent.LowerRightCorner.y - rcContent.UpperLeftCorner.y) / node._lineHeight;
	if (maxLines <= 0)
		return;
		
	// 更新总行数
	const_cast<ChangelistsSnapshot::Node&>(node)._totalLines = cl->changes.size();
		
	// 绘制文件列表
	int y = rcContent.UpperLeftCorner.y;
	int lineCount = 0;
	
	// 如果前面还有内容,绘制向上箭头
	if (node._fileListScrollLine > 0)
	{
		i_math::recti rcArrow(
			rcContent.UpperLeftCorner.x,
			y - 8,
			rcContent.LowerRightCorner.x,
			y
		);
		// 计算箭头的位置和大小
		int arrowWidth = 8;
		int arrowHeight = 8;
		i_math::recti rcPyramid(
			rcArrow.Left() + (rcArrow.getWidth() - arrowWidth) / 2,
			rcArrow.Top() + (rcArrow.getHeight() - arrowHeight) / 2,
			rcArrow.Left() + (rcArrow.getWidth() + arrowWidth) / 2,
			rcArrow.Top() + (rcArrow.getHeight() + arrowHeight) / 2
		);
		gg->GradientPyrimid(rcPyramid, 0x00FF00, 0x008000);
	}
	
	// 从滚动位置开始绘制
	for (int i = node._fileListScrollLine; i < cl->changes.size() && lineCount < maxLines; ++i)
	{
		const FileChange& change = cl->changes[i];
			
		// 根据变更类型选择颜色
		DWORD textColor;
		std::string prefix;
		switch (change.type)
		{
		case FileChange_Added:
			textColor = 0x00FF00; // 绿色表示新增
			prefix = "[+] ";
			break;
		case FileChange_Deleted:
			textColor = 0xFF0000; // 红色表示删除
			prefix = "[-] ";
			break;
		case FileChange_Modified:
			textColor = 0x00FFFF; // 黄色表示修改
			prefix = "[*] ";
			break;
		default:
			textColor = 0xFFFFFF; // 白色表示其他
			prefix = "";
			break;
		}
		
		// 构造显示文本
		std::string displayText = prefix + change.relativePath;
		
		// 绘制文本,使用 DT_END_ELLIPSIS 来处理超出宽度的文本
		i_math::recti rcText(
			rcContent.UpperLeftCorner.x,
			y,
			rcContent.LowerRightCorner.x,
			y + node._lineHeight
		);
		
		// 绘制背景
		if (node._uid == snapshot._selectedNodeUID && i == snapshot._selectedFileIndex)
		{
			// 选中的行使用深色背景
			gg->FillSolidRect(rcText, 0x3F3F3F, 255);
		}
		if (node._uid == snapshot._hiliteNodeUID && i == snapshot._hiliteFileIndex)
		{
			// 高亮的行使用浅色背景
			i_math::recti rcHilight = rcText;

			// 如果是高亮状态,测量文本实际宽度
			i_math::size2di textSize = gg->MessureText(displayText.c_str());
			rcText.LowerRightCorner.x = rcText.UpperLeftCorner.x + textSize.w+8;
			if (rcText.LowerRightCorner.x > rcHilight.LowerRightCorner.x)
				rcHilight.LowerRightCorner.x = rcText.LowerRightCorner.x;

			gg->FillSolidRect(rcHilight, 0x2F5F2F, 128);
		}
		
		gg->DrawText(displayText.c_str(), rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, FALSE, textColor);
		
		y += node._lineHeight;
		lineCount++;
	}
	
	// 如果后面还有内容,绘制向下箭头
	if (node._fileListScrollLine + maxLines < cl->changes.size())
	{
		i_math::recti rcArrow(
			rcContent.UpperLeftCorner.x,
			y,
			rcContent.LowerRightCorner.x,
			y + 8
		);
		// 计算箭头的位置和大小
		int arrowWidth = 8;
		int arrowHeight = 8;
		i_math::recti rcPyramid(
			rcArrow.Left() + (rcArrow.getWidth() - arrowWidth) / 2,
			rcArrow.Top() + (rcArrow.getHeight() - arrowHeight) / 2,
			rcArrow.Left() + (rcArrow.getWidth() + arrowWidth) / 2,
			rcArrow.Top() + (rcArrow.getHeight() + arrowHeight) / 2
		);
		gg->GradientPyrimidInv(rcPyramid, 0x00FF00, 0x008000);
	}
}

void CGuiView_Changelists::_DrawNode(GraphicsGraph* gg, const ChangelistsSnapshot::Node& node)
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
	CChangelists& changelists = *data->_changelists;
	ChangelistsSnapshot& snapshot = data->_snapshot;

	// 创建节点矩形
	i_math::recti rcNode(
		(int)node._x,
		(int)node._y,
		(int)(node._x + ChangelistNode_Width),
		(int)(node._y + ChangelistNode_Height)
	);

	if (node._isCur)
	{
		i_math::recti rcBg;
		rcBg = rcNode;
		rcBg.inflate(10, 10, 10, 10);
		gg->DrawRoundCornerRect(rcBg, 10, 0x00007f, 1);
	}

	// 绘制圆角矩形背景
	if (!node._isCommit)
	{
		// 当前节点使用不同的颜色
		gg->DrawRoundCornerRect(rcNode, 5, 0xBFBFBF, 0x8F8F8F);
		gg->FrameRoundCornerRect(rcNode, 5, 0x000000, 1, 255, TRUE);
	}
	else
	{
		gg->DrawRoundCornerRect(rcNode, 5, 0x808080, 0x606060);
		gg->FrameRoundCornerRect(rcNode, 5, 0x000000, 1);
	}


	// 检查节点是否被选中
	bool isSelected = false;
	if (TRUE)
	{
		for (FileChangeListUID selectedUID : data->_sels)
		{
			if (node._uid == selectedUID)
			{
				isSelected = true;
				break;
			}
		}
	}

	// 绘制文件列表
	_DrawChangelistFilePathes(gg, rcNode, snapshot, node, changelists, isSelected);
}


void CGuiView_Changelists::_DrawChangelists(GraphicsGraph* gg)
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
	if (!data)
		return;

	// 刷新快照
	_RefreshSnapshot(gg);

	// 如果需要将当前节点居中
	if (_needReCenterCur)
	{
		_ReCenterCur(gg);
	}


	CChangelists& changelists = *data->_changelists;
	ChangelistsSnapshot& snapshot = data->_snapshot;
	
	// 如果没有节点，直接返回
	if (snapshot._nodes.empty() || snapshot._rootUID == 0)
		return;
	
	// 定义绘制颜色
	DWORD clrNode = 0x6FAADCFF;      // 节点填充颜色
	DWORD clrNodeBorder = 0x325A96FF; // 节点边框颜色
	DWORD clrNodeText = 0xF0F0F0FF;   // 节点文本颜色
	DWORD clrLine = 0xFF9898FF;       // 连线颜色
	
	// 先画连线
	for (const auto& pair : snapshot._nodes)
	{
		const auto& node = pair.second;
		
		// 根节点没有父节点，跳过
		if (node._parent == 0)
			continue;
		
		// 查找父节点
		auto parentNode = snapshot.FindNode(node._parent);
		if (!parentNode)
			continue;
		
		// 计算连线的起点和终点
		float startX = parentNode->_x + ChangelistNode_Width;
		float startY = parentNode->_y + ChangelistNode_Height / 2;
		float endX = node._x;
		float endY = node._y + ChangelistNode_Height / 4;
		
		// 绘制三段折线 (父节点右侧中点 -> 中间点 -> 子节点左侧中点)
		float midX = (startX + endX) / 2;
		
		// 折线第一段
		gg->DrawLine(clrLine, 4.0f, 
			i_math::pos2df(startX, startY), 
			i_math::pos2df(midX, startY));
		// 折线第二段（垂直段）
		gg->DrawLine(clrLine, 4.0f,
			i_math::pos2df(midX, startY), 
			i_math::pos2df(midX, endY));
		// 折线第三段
		gg->DrawLine(clrLine, 4.0f,
			i_math::pos2df(midX, endY), 
			i_math::pos2df(endX, endY));
	}
	
	// 再画节点
	if (true)
	{
		const ChangelistsSnapshot::Node* hilightNode = nullptr;
		for (const auto& pair : snapshot._nodes)
		{
			const auto& node = pair.second;

			if (node._uid == snapshot._hiliteNodeUID)
			{
				hilightNode = &node;
				continue;
			}

			_DrawNode(gg, node);
		}
		if (hilightNode)
			_DrawNode(gg, *hilightNode);
	}
}


void CGuiView_Changelists::_OnDraw( GraphicsGraph *gg )
{	
	gg->ClearBg( ColorAlpha(0x6f6f6f,0xff));
// 	gg->ClearBg(ColorAlpha(0, 0xff));
	gg->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality );

	i_math::recti rc0;
	_wnd->GetClientRect((CRect*)&rc0);

	// 获取Changelists数据
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
	if (data && data->_changelists)
	{
		_DrawChangelists(gg);
	}
}


BOOL CGuiView_Changelists::Respond(CtrlOp &co)
{
	if (!CGuiView::Respond(co))
		return FALSE;
	
// 	// 如果在只读模式下，不处理任何交互
// 	if (_bReadOnly)
// 		return FALSE;
// 		
// 	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
// 	if (!data || !data->_changelists)
// 		return FALSE;
// 		
// 	CChangelists* changelists = data->_changelists;
// 	
// 	switch (co.op)
// 	{
// 	case CtrlOp::LBUTTONDOWN:
// 		{
// 			// 检查是否点击了某个节点
// 			i_math::pos2df mousePos(co.x, co.y);
// 			
// 			for (const auto& pair : snapshot.nodes)
// 			{
// 				const auto& node = pair.second;
// 				
// 				// 检查点击是否在节点矩形内
// 				if (mousePos.x >= node.x && mousePos.x <= node.x + node.width &&
// 					mousePos.y >= node.y && mousePos.y <= node.y + node.height)
// 				{
// 					// 点击了节点，处理选择
// 					changelists->SetCurrentUID(node.uid);
// 					data->FireDataChanged();
// 					_wnd->InvalidateRect(NULL);
// 					return TRUE;
// 				}
// 			}
// 		}
// 		break;
// 		
// 	case CtrlOp::MOUSEMOVE:
// 		if (co.dwData & MK_LBUTTON)
// 		{
// 			// 可以实现拖拽操作
// 			// TODO: 实现拖拽功能
// 		}
// 		break;
// 		
// 	case CtrlOp::CONTEXTMENU:
// 		{
// 			// 检查是否在某个节点上右键点击
// 			i_math::pos2df mousePos(co.x, co.y);
// 			FileChangeListUID clickedUID = 0;
// 			
// 			for (const auto& pair : snapshot.nodes)
// 			{
// 				const auto& node = pair.second;
// 				
// 				// 检查点击是否在节点矩形内
// 				if (mousePos.x >= node.x && mousePos.x <= node.x + node.width &&
// 					mousePos.y >= node.y && mousePos.y <= node.y + node.height)
// 				{
// 					clickedUID = node.uid;
// 					break;
// 				}
// 			}
// 			
// 			if (clickedUID != 0)
// 			{
// 				// 在节点上右键点击，显示上下文菜单
// 				// TODO: 实现上下文菜单
// 				return TRUE;
// 			}
// 		}
// 		break;
// 	}
	
	return FALSE;
}
