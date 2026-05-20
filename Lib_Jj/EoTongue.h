#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Tongue 55

#define MAX_TONGUE_BRANCH 6

struct EoParamTongue:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamTongue);

	BEGIN_GOBJ_PURE(EoParamTongue,1);

		GELEM_VAR_INIT(DWORD,nBranches,4);
			GELEM_EDITVAR("分支个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"有几个分支");
		GELEM_VAR_INIT(DWORD,nNodes,20);
			GELEM_EDITVAR("分支节点个数",GVT_U,GSem_Interger,"每个分支与几个节点");
		GELEM_VAR_INIT(float,lengthTotal,14.0f);
			GELEM_EDITVAR("总长度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"总长度");
		GELEM_VAR_INIT(float,lengthThrust,8.0f);
			GELEM_EDITVAR("冲刺长度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"冲刺长度");
		GELEM_VAR_INIT(float,speedThrust,10.0f);
			GELEM_EDITVAR("冲刺速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"冲刺速度");
		GELEM_VAR_INIT(float,speedTrace,4.0f);
			GELEM_EDITVAR("跟踪速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"跟踪速度");
		GELEM_VAR_INIT(float,radiusNode,0.2f);
			GELEM_EDITVAR("节点半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"节点半径");
		GELEM_VAR_INIT(float,radiusSense,1.0f);
			GELEM_EDITVAR("感知半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"末端的感知半径");
	END_GOBJ();

	DWORD nBranches;
	DWORD nNodes;
	float lengthTotal;
	float lengthThrust;
	float speedThrust;
	float speedTrace;
	float radiusNode;
	float radiusSense;
};

class EoTongue;
class CTongueBranch
{
public:
	CTongueBranch()
	{
		Zero();
	}
	void Init(EoTongue *tongue,LevelPos &pos,float face);
	void Clear()
	{
		_nodes.clear();
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
		_param=NULL;
		_stage=Stage_Ready;
		_faceRoot=0.0f;
		_length=0.0f;
	}
protected:

	enum Stage
	{
		Stage_Ready,
		Stage_Thrust,
		Stage_Trace,
		Stage_Withdraw,
	};

	EoTongue *_owner;
	EoParamTongue *_param;

	Stage _stage;
	LevelPos _posRoot;
	LevelFace _faceRoot;

	struct Node
	{
		Node()
		{
			yaw=0.0f;
		}
		LevelFaceYaw yaw;
	};
	std::deque<Node> _nodes;
	float _length;


};


class EoTongue:public CLoEffectObj
{
public:
	EoTongue()
	{
		_idTarget=LevelObjID_Invalid;
	}
	DEFINE_LEVELOBJ_CLASS(EoTongue,CLASSUID_Tongue);

	virtual const char *GetShowName()	{		return "舌头";	}

protected:

	virtual void _OnPostCreate();


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	void _OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _OnUpdate();
	virtual BOOL _NeedOps()	{		return TRUE;	}

	void _WriteState(CBitPacket *bp);

	DWORD _nBranches;
	CTongueBranch _branches[MAX_TONGUE_BRANCH];

	LevelObjID _idTarget;



};
