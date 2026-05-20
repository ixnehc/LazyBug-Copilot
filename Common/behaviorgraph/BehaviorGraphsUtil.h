#pragma once

#include "../class/class.h"
#include "../gds/GObj.h"
#include "../mempool/mempool.h"

#include "../strlib/strlibdefines.h"

#include "../linkpad/LinkPadDefines.h"

#include "BehaviorMem.h"

#include <set>

class CBehaviorGraphPads;
struct FillDescAssist;
struct BgpClasses;
class CBehaviorGraphPad;
class CBehaviorGraphUtil
{
public:
	void Init(const char *pathRoot,BgpClasses *clsses)
	{
		_pathRoot=pathRoot;
		_clsses=clsses;
	}
	StringID *EnumNames(DWORD &count);
	BOOL LoadBGPads(StringID nm,CBehaviorGraphPads &pads);
	const char *FindBGPadsPath(StringID nm);

	BOOL ResolveBGPads(CBehaviorGraphPads &pads);
	BOOL VerifyBases(CBehaviorGraphPads &pads);
	BOOL VerifyNewBase(CBehaviorGraphPads &pads,StringID nmNewBase);
	BOOL UnresolveBGPads(CBehaviorGraphPads &pads);

	struct Node
	{
		Node()
		{
			pad=NULL;
			depth=0;
			parent=NULL;
			height=0.0f;
			bLockRecursive=FALSE;
			yAbs=-1.0f;
			yLocal=-1.0f;
		}
		BOOL IsRoot();
		CBehaviorGraphPad *pad;
		std::vector<Node *>candidatesParent;
		std::vector<Node*> childs;
		std::vector<int> indicesChilds;//各个child的顺序号(在第几个out stub上)
		Node *parent;
		int depth;
		float height;
		float yAbs;
		float yLocal;
		BOOL bLockRecursive;
	};
	void Repos(CBehaviorGraphPads &pads,FillDescAssist *assist);

protected:

	BOOL _VerifyBases(CBehaviorGraphPads &pads,std::set<StringID> &existing);
	BOOL _LoadBGPads(const char *path,StringID nm0,CBehaviorGraphPads &pads);
	BOOL _FindAndLoadBGPads(const char *pathRoot,StringID nm,CBehaviorGraphPads &pads);
	BOOL _FindBGPads(const char *pathRoot,StringID nm,std::string &path);
	BOOL _ResolveBGPads(CBehaviorGraphPads &pads);

	BOOL _MergeBGPads(CBehaviorGraphPads &pads,CBehaviorGraphPads &padsBase);
	void _CullBGPads(CBehaviorGraphPads &pads,CBehaviorGraphPads &padsBase);

	void _DispatchDepth(Node *node,int &depthMax);
	void _Deposit(Node *node,std::vector<std::vector<Node*> >&nodesDepth);
	void _LocateChilds(Node *node);

	float _CalcPadHeight(CBehaviorGraphPad *pad,FillDescAssist *assist);
	void _LocateParent(std::vector<Node*>&nodesChild);


	std::string _pathRoot;
	BgpClasses *_clsses;

	std::vector<BYTE>_temp;
	std::vector<StringID> _temp2;

	CMemPool<Node> _poolNode;
	std::vector<Node *>_nodes;


};
