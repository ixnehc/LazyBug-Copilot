#pragma once



//Service
// enum LevelServiceType
// {
// 	LevelService_None=0,
// 	LevelService_CureHP,
// 	LevelService_蜈蚣节点地幔,
// 	LevelService_蜈蚣节点地幔喷射者,
// 	LevelService_蜈蚣节点,
// 
// 	LevelService_Max,
// 
// 	LevelService_ForceDword=0xffffffff,
// };


// #define LevelServiceType_SemConstraint "n/a,回复HP,蜈蚣节点地幔,蜈蚣节点地幔喷射者,蜈蚣节点"

extern const char *GetServiceName(LevelServiceType tp);


class CLevel;
class GameTileMap;
class CUnitMgrNavMesh;
class CLevelObj;
class CLevelService
{
public:
	DEFINE_CLASS(CLevelService);

	enum ClientPriorityMethod
	{
		ClientPriority_Equal,
		ClientPriority_Closer,//离server越近,Priority越高
	};
	struct ClientEntry
	{
		ClientEntry()
		{
			idServer=LevelObjID_Invalid;
			bValid=FALSE;
			tCDEnd=0;
		}
		LevelObjID idServer;
		BOOL bValid;
		AnimTick tCDEnd;//CoolDown的终止时间
	};

	struct ServerEntry
	{
		ServerEntry()
		{
			nQuota=0;
			nPreserved=0;
			tTimeOut=ANIMTICK_INFINITE;
		}
		DWORD nQuota;
		DWORD nPreserved;
		AnimTick tTimeOut;//这个时间之后,如果没有Preserve,则删除这个server entry
	};

	CLevelService()
	{
		Zero();
	}

	void Zero()
	{
		_level=NULL;
		_methodPriority=ClientPriority_Equal;
	}
	void Init(CLevel *level);
	void Clear();

	void SetPriorityMethod(ClientPriorityMethod method)	{		_methodPriority=method;	}

	BOOL AddServer(LevelObjID idServer,DWORD nQuota,AnimTick tTimeOut=ANIMTICK_INFINITE);
	void RemoveServer(LevelObjID idServer);
	BOOL AddClient(LevelObjID idClient);
	void RemoveClient(LevelObjID idClient);
	BOOL AddUniqueServer(LevelObjID idServer,DWORD nQuota);
	LevelObjID GetUniqueServer();

	DWORD GetServerCount();

	BOOL Preserve(LevelObjID idClient,LevelObjID idServer);
	BOOL CanPreserve(LevelObjID idClient,LevelObjID idServer);
	BOOL CheckPreserve(LevelObjID idClient,LevelObjID idServer);
	BOOL CheckPreserve(LevelObjID idClient);
	LevelObjID GetPreserve(LevelObjID idClient);

	LevelObjID FindClosestServer(LevelPos &pos,float radius,DWORD nMinAvailableQuota);

	void SetClientCD(LevelObjID idClient,AnimTick durCD);

	DWORD GetServerQuota(LevelObjID idServer);
	LevelObjID Get1stPreserveClient(LevelObjID idServer);

	LevelObjID QueryClient(LevelObjID idClient);//查看某个client是否拿到了某个Server的名额
	void Acknowledge(LevelObjID idClient);//确认client已经领取了它的名额

	void Update();

protected:

	virtual float _CalcPriority(CLevelObj *loServer,CLevelObj *loClient);

	void _GarbageCollect();

	void _RemoveServer(std::unordered_map<LevelObjID,ServerEntry>::iterator it);
	void _RemoveClient(std::unordered_map<LevelObjID,ClientEntry>::iterator it,BOOL bErase);


	CLevel *_level;

	std::unordered_map<LevelObjID,ClientEntry> _clients;
	std::unordered_map<LevelObjID,ServerEntry>_servers;

	ClientPriorityMethod _methodPriority;

};


class CLevelServiceCureHP:public CLevelService
{
public:
protected:
	virtual float _CalcPriority(CLevelObj *loServer,CLevelObj *loClient);

};