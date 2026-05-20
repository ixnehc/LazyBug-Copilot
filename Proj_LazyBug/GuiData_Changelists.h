#pragma once


#include "GuiLib.h"
#include "editor/editor.h"

#include "ref/ref.h"

#include "ruler/ruler.h"

#include <unordered_map>

//#include "Changelists.h"

#define ChangelistNode_Width 240
#define ChangelistNode_Height 160

#define ChangelistNode_HorPadding 60
#define ChangelistNode_VerPadding 40

// 文件列表布局相关常量
#define FileList_TitleHeight 20
#define FileList_LeftMargin 5
#define FileList_RightMargin 5
#define FileList_TopMargin 30
#define FileList_BottomMargin 5
#define FileList_LineSpacing 2

//用于绘制Changelists的信息,记录各个节点的位置,大小
//changelists中的changelist为树状结构
//Snapshot中每个节点对应一个changelist,这些节点以树状结构排列,从左向右生长
//Snapshot中每个节点包含一个changelist的uid
//Snapshot中每个节点为一个固定长宽的长方形,包含包含位置大小信息
struct ChangelistsSnapshot
{
	struct Node
	{
		Node()
		{
			_uid = FileChangeListUID_Invalid;
			_parent = FileChangeListUID_Invalid;
			_x = 0;
			_y = 0;
			_level = 0;    // 树的层级
			_index = 0;    // 同层中的索引
			_isCur = false;
			_isCommit = false;
			_fileListScrollLine = 0;  // 文件列表当前滚动到的行号
			_totalLines = 0;          // 文件列表总行数
			_lineHeight = 0;          // 文件列表行高
			_visibleLines = 0;        // 显示区域的行数
		}
		bool _isCur;
		bool _isCommit;
		FileChangeListUID _uid;       // changelist的UID
		FileChangeListUID _parent;    // 父节点的UID
		float _x;                     // 节点左上角x坐标
		float _y;                     // 节点左上角y坐标
		int _level;                   // 节点所在的层级（根为0）
		int _index;                   // 节点在同层中的索引
		std::string _desc;            // changelist描述
		
		// 文件列表相关
		int _fileListScrollLine;      // 文件列表当前滚动到的行号
		int _totalLines;              // 文件列表总行数
		int _lineHeight;              // 文件列表行高
		int _visibleLines;            // 显示区域的行数

		// 滚动文件列表
		// deltaLine > 0 向下滚动
		// deltaLine < 0 向上滚动
		void Scroll(int deltaLine)
		{
			if (_totalLines <= 0)
			{
				_fileListScrollLine = 0;
				return;
			}
			
			// 计算新的滚动位置
			int newScrollLine = _fileListScrollLine + deltaLine;
			
			// 确保不会滚动到负数位置
			if (newScrollLine < 0)
				newScrollLine = 0;
				
			// 确保不会滚动到超出总行数
			if (newScrollLine >= _totalLines)
				newScrollLine = _totalLines - 1;
				
			// 确保最后一行在可见区域内
			if (newScrollLine + _visibleLines > _totalLines)
				newScrollLine = _totalLines - _visibleLines;
				
			// 如果可见行数大于总行数,则不需要滚动
			if (_visibleLines >= _totalLines)
				newScrollLine = 0;
				
			_fileListScrollLine = newScrollLine;
		}

		// 根据鼠标位置计算选中的文件索引
		// 返回 -1 表示没有选中任何文件
		int FileIndexHitTest(float mouseX, float mouseY) const
		{
			// 计算文件列表区域
			float listX = _x + FileList_LeftMargin;
			float listY = _y + FileList_TopMargin;
			float listWidth = ChangelistNode_Width - FileList_LeftMargin - FileList_RightMargin;
			float listHeight = ChangelistNode_Height - FileList_TopMargin - FileList_BottomMargin;

			// 检查鼠标是否在文件列表区域内
			if (mouseX < listX || mouseX > listX + listWidth ||
				mouseY < listY || mouseY > listY + listHeight)
			{
				return -1;
			}

			// 计算相对于列表顶部的垂直位置
			float relativeY = mouseY - listY;

			// 计算行号
			int lineIndex = static_cast<int>(relativeY / _lineHeight);

			// 考虑滚动位置
			return lineIndex + _fileListScrollLine;
		}

		// 判断坐标是否在文件列表区域内
		bool IsInFileListArea(float mouseX, float mouseY) const
		{
			// 计算文件列表区域
			float listLeft = _x + FileList_LeftMargin;
			float listRight = _x + ChangelistNode_Width - FileList_RightMargin;
			float listTop = _y + FileList_TopMargin;
			float listBottom = _y + ChangelistNode_Height - FileList_BottomMargin;
			
			// 判断坐标是否在区域内
			return (mouseX >= listLeft && mouseX <= listRight && mouseY >= listTop && mouseY <= listBottom);
		}
	};

	ChangelistsSnapshot()
	{
		Zero();
		_dataVer = 0;
		_changelistsVer = 0;
	}

	void Zero()
	{
		_rootUID = 0;
		_maxLevel = 0;
		_nodes.clear();
		_hiliteNodeUID = FileChangeListUID_Invalid;
		_hiliteFileIndex = -1;
		_selectedNodeUID = FileChangeListUID_Invalid;
		_selectedFileIndex = -1;
	}

	void Clear()
	{
		Zero();
	}
	
	// 清除所有节点的高亮状态
	void ClearNodeHilights()
	{
		_hiliteNodeUID = FileChangeListUID_Invalid;
		_hiliteFileIndex = -1;
	}

	// 清除所有节点的高亮状态
	void ClearNodeSelected()
	{
		_selectedNodeUID = FileChangeListUID_Invalid;
		_selectedFileIndex = -1;
	}


	// 设置高亮状态,返回有无变化
	bool SetHilite(FileChangeListUID nodeUID, int fileIndex)
	{
		if ((_hiliteNodeUID == nodeUID) && (fileIndex == _hiliteFileIndex))
			return false;
		_hiliteNodeUID = nodeUID;
		_hiliteFileIndex = fileIndex;
		return true;
	}

	// 设置选中状态,返回有无变化
	bool SetSelected(FileChangeListUID nodeUID, int fileIndex)
	{
		if ((_selectedNodeUID == nodeUID) && (fileIndex == _selectedFileIndex))
			return false;
		_selectedNodeUID = nodeUID;
		_selectedFileIndex = fileIndex;
		return true;
	}

	// 根据UID查找节点
	Node* FindNode(FileChangeListUID uid)
	{
		auto it = _nodes.find(uid);
		if (it != _nodes.end())
			return &it->second;
		return nullptr;
	}

	Node* FindCur()
	{
		for (auto& pair : _nodes)
		{
			if (pair.second._isCur)
				return &pair.second;
		}
		return nullptr;
	}

	FileChangeListUID HitTest(float x, float y)
	{
		// 遍历所有节点
		for (const auto& pair : _nodes)
		{
			const Node& node = pair.second;
			
			// 检查 x 坐标是否在节点宽度范围内
			bool hitX = (x >= node._x) && (x <= node._x + ChangelistNode_Width);
			// 检查 y 坐标是否在节点高度范围内
			bool hitY = (y >= node._y) && (y <= node._y + ChangelistNode_Height);
			
			// 如果 x 和 y 都在范围内，则命中
			if (hitX && hitY)
			{
				return node._uid; // 返回命中节点的 UID
			}
		}
		
		// 如果没有命中任何节点，返回无效 UID
		return FileChangeListUID_Invalid;
	}

	// 节点命中测试,同时返回命中的节点和文件索引
	// 返回: 命中的节点指针,如果未命中则返回nullptr
	// 参数: 
	//   x,y - 测试坐标
	//   outFileIndex - 输出参数,命中的文件索引,如果未命中文件列表则返回-1
	Node* NodeHitTest(float x, float y, int& outFileIndex)
	{
		// 遍历所有节点
		for (auto& pair : _nodes)
		{
			Node& node = pair.second;
			
			// 检查 x 坐标是否在节点宽度范围内
			bool hitX = (x >= node._x) && (x <= node._x + ChangelistNode_Width);
			// 检查 y 坐标是否在节点高度范围内
			bool hitY = (y >= node._y) && (y <= node._y + ChangelistNode_Height);
			
			// 如果 x 和 y 都在范围内，则命中节点
			if (hitX && hitY)
			{
				// 检查是否命中文件列表
				if (node.IsInFileListArea(x, y))
				{
					// 计算命中的文件索引
					outFileIndex = node.FileIndexHitTest(x, y);
				}
				else
				{
					outFileIndex = -1;
				}
				return &node;
			}
		}
		
		// 如果没有命中任何节点
		outFileIndex = -1;
		return nullptr;
	}

	DWORD _changelistsVer;
	DWORD _dataVer;
	FileChangeListUID _rootUID;               // 根节点UID
	std::unordered_map<FileChangeListUID, Node> _nodes;   // 所有节点
	int _maxLevel;                           // 最大层级
	float _levelHeight;                      // 每层高度
	
	// 高亮状态
	FileChangeListUID _hiliteNodeUID;        // 高亮节点的UID
	int _hiliteFileIndex;                    // 高亮文件索引
	
	// 选中状态
	FileChangeListUID _selectedNodeUID;      // 选中节点的UID
	int _selectedFileIndex;                  // 选中文件索引
};




class CGuiData_Changelists : public GeData
{	
public:
	CGuiData_Changelists();
	~CGuiData_Changelists();
	virtual const char *GetName()	{		return "Changelists";	}
	void	Clear();

	void Set(CChangelists* changelists);
	DWORD GetVer()
	{
		return _ver;
	}

	void SetSel(FileChangeListUID uid)
	{
		_sels.clear();
		_sels.push_back(uid);
	}

	void RequestOpenSel()
	{
		_requestOpenSelTime = GetAbsTick();
	}
	AbsTick FetchOpenSelRequest()
	{
		AbsTick ret = _requestOpenSelTime;
		_requestOpenSelTime = 0;
		return ret;
	}

public:

	CChangelists* _changelists;
	DWORD _ver;

	ChangelistsSnapshot _snapshot;
	std::vector<FileChangeListUID> _sels;
	AbsTick _requestOpenSelTime;

	friend class CChangelistsDialog;
	friend class CGuiView_Changelists;
};


