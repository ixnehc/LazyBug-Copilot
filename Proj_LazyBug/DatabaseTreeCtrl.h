#pragma once

#include "GuiLib.h"
#include "NodeTreeCtrl.h"
#include "Database.h"

// 数据库树控件类,用于显示数据库中的条目
class CDatabaseTreeCtrl : public CNodeTreeCtrl, public CNodeTree
{
public:
    CDatabaseTreeCtrl();
    ~CDatabaseTreeCtrl();

    // 清空树控件
    void Clear();

    // 设置要显示的数据库
    void SetContent(CDatabase* pDB);

    // 更新显示
    void Update();

protected:
    // CNodeTree重写
    virtual void _OnInitType() override;
    virtual BOOL _OnCheckTypeRelation(NodeType typeParent, NodeType typeChild) override;
    virtual const char* _GetSep() override;
    virtual NodePtr _OnNew(const char* path, NodeType type, void* param) override;
    virtual BOOL _OnDelete(NodeHandle hNode) override;
    virtual BOOL _OnRename(NodeHandle hNode, const char* nameNew) override;
    virtual BOOL _SupportSsc() override { return FALSE; }
    virtual const char* _GetSscPath(NodeHandle hNode, BOOL& bFolder) override;
    virtual const char* _GetShowName(NodeHandle hNode, const char* nameOrg) override;

    // CNodeTreeCtrl重写
    virtual UINT _GetImageID() override;
    virtual DWORD _GetImageIdx(NodeHandle hNode, SscState state) override;
    virtual BOOL _IsEditable() override;
    virtual BOOL _IsExchangable() override { return FALSE; }
    virtual BOOL _CanRename(NodeType type) override;
    virtual BOOL _CanNew(NodeType type) override;
    virtual BOOL _CanAutoGenUniqueName(NodeType type) override;
    virtual std::string _GenNewName(NodeType type, const char* nameType) override;
    virtual BOOL _GenUniqueName(NodeType type, std::string& name) override;

protected:
    CDatabase* _pDB;  // 数据库指针

public:
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
}; 