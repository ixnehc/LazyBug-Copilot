#include "stdh.h"
#include "DatabaseTreeCtrl.h"
#include "resource.h"

// 定义节点类型
#define NODETYPE_FOLDER 1  // 文件夹节点
#define NODETYPE_FILE   2  // 文件节点

BEGIN_MESSAGE_MAP(CDatabaseTreeCtrl, CNodeTreeCtrl)
    ON_WM_CREATE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

CDatabaseTreeCtrl::CDatabaseTreeCtrl()
{
    _pDB = nullptr;
}

CDatabaseTreeCtrl::~CDatabaseTreeCtrl()
{
    Clear();
}

void CDatabaseTreeCtrl::Clear()
{
    _pDB = nullptr;
    CNodeTree::ResetContent();
    CNodeTreeCtrl::SetNodeTree(nullptr);
}

void CDatabaseTreeCtrl::SetContent(CDatabase* pDB)
{
    if (!pDB)
    {
        Clear();
        return;
    }

    _pDB = pDB;
    
    // 初始化节点树
    CNodeTree::ResetContent();
    _OnInitType();
    
    // 添加根节点
    NodeHandle hRoot = NodeHandle_Root;
    
    // 获取所有条目
    const auto& entries = _pDB->GetEntries().GetAllEntries();
    
    // 遍历所有条目,构建树结构
    for (const auto& entry : entries)
    {
        // 创建虚拟路径节点
        std::string virtualPath = entry.pathVirtual;
        _OnNew(virtualPath.c_str(), NODETYPE_FILE, nullptr);
    }
    
    // 设置树控件显示
    CNodeTreeCtrl::SetNodeTree(this);
}

void CDatabaseTreeCtrl::Update()
{
    if (!_pDB)
    {
        return;
    }
    
    // 更新树控件显示
    CNodeTreeCtrl::UpdateNodeTree(this);
}

void CDatabaseTreeCtrl::_OnInitType()
{
    // 注册节点类型
    NodeTypeInfo info;
    
    // 文件夹类型
    info.type = NODETYPE_FOLDER;
    info.name = "Folder";
    info.category = "";
    RegisterType(info);
    
    // 文件类型
    info.type = NODETYPE_FILE; 
    info.name = "File";
    info.category = "";
    RegisterType(info);
}

BOOL CDatabaseTreeCtrl::_OnCheckTypeRelation(NodeType typeParent, NodeType typeChild)
{
    // 文件夹下可以包含文件夹和文件
    if (typeParent == NODETYPE_FOLDER)
        return TRUE;
        
    // 文件节点下不能包含子节点
    if (typeParent == NODETYPE_FILE)
        return FALSE;
        
    // 根节点下可以包含文件夹和文件
    if (typeParent == NodeType_Root)
        return TRUE;
        
    return FALSE;
}

const char* CDatabaseTreeCtrl::_GetSep()
{
    return "\\";  // 使用反斜杠作为路径分隔符
}

NodePtr CDatabaseTreeCtrl::_OnNew(const char* path, NodeType type, void* param)
{
    if (!path || !_pDB)
        return NodePtr_Null;
        
    // 创建节点
    NodePtr ptr = NodePtr_Null;
    
    if (type == NODETYPE_FILE)
    {
        // 查找对应的数据库条目
        const auto& entries = _pDB->GetEntries().GetAllEntries();
        for (const auto& entry : entries)
        {
            if (entry.pathVirtual == path)
            {
                ptr = (NodePtr)&entry;
                break;
            }
        }
    }
    else if (type == NODETYPE_FOLDER)
    {
        // 文件夹节点不需要特殊处理
        ptr = (NodePtr)1;
    }
    
    return ptr;
}

BOOL CDatabaseTreeCtrl::_OnDelete(NodeHandle hNode)
{
    // 暂不支持删除操作
    return FALSE;
}

BOOL CDatabaseTreeCtrl::_OnRename(NodeHandle hNode, const char* nameNew)
{
    // 暂不支持重命名操作
    return FALSE;
}

const char* CDatabaseTreeCtrl::_GetSscPath(NodeHandle hNode, BOOL& bFolder)
{
    // 不支持源代码管理
    return "";
}

const char* CDatabaseTreeCtrl::_GetShowName(NodeHandle hNode, const char* nameOrg)
{
    if (!nameOrg)
        return "";
        
    return nameOrg;
}

UINT CDatabaseTreeCtrl::_GetImageID()
{
    return IDB_RESTREEICON;  // 使用资源中的图标
}

DWORD CDatabaseTreeCtrl::_GetImageIdx(NodeHandle hNode, SscState state)
{
    NodeType type = GetType(hNode);
    
    if (type == NODETYPE_FOLDER)
        return 0;  // 文件夹图标
    else if (type == NODETYPE_FILE)
        return 1;  // 文件图标
        
    return 0;
}

BOOL CDatabaseTreeCtrl::_IsEditable()
{
    return FALSE;  // 暂不支持编辑
}

BOOL CDatabaseTreeCtrl::_CanRename(NodeType type)
{
    return FALSE;  // 暂不支持重命名
}

BOOL CDatabaseTreeCtrl::_CanNew(NodeType type)
{
    return FALSE;  // 暂不支持新建
}

BOOL CDatabaseTreeCtrl::_CanAutoGenUniqueName(NodeType type)
{
    return FALSE;  // 暂不支持自动生成名称
}

std::string CDatabaseTreeCtrl::_GenNewName(NodeType type, const char* nameType)
{
    return "";  // 暂不支持生成新名称
}

BOOL CDatabaseTreeCtrl::_GenUniqueName(NodeType type, std::string& name)
{
    return FALSE;  // 暂不支持生成唯一名称
}

int CDatabaseTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CNodeTreeCtrl::OnCreate(lpCreateStruct) == -1)
        return -1;
    return 0;
}

void CDatabaseTreeCtrl::OnDestroy()
{
    Clear();
    CNodeTreeCtrl::OnDestroy();
} 