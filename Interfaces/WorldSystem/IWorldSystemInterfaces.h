/********************************************************************
created:	2008/10/03   15:10
filename: 	e:\IxEngine\Interfaces\WorldSystem\IWorldSystemInterfaces.h
author:		cxi

purpose:	world system各个接口的预定义
*********************************************************************/

#pragma once

//interfaces from RenderSystem
class IRenderSystem;
class IRenderer;
class ITexture;

//interfaces from FileSystem
class IFileSystem;
class IFile;
class IFolder;

//interfaces from PhysicsSystem
class IPhysicsSystem;


class IGameSystem;
class IClient;

class IBspModel;
class IBspEditor;

class IWorldSystem;

class IMapFile;
class ITrrnBrushLibRT;
class ITrrnBrushLib;
class ITrrnMap;
class ITrrnMapEditor;

class IAssetPackage;

class IAssetSystem;
class IAsset;
class IAssetRenderer;
class IAssetEventer;

class IEntitySystem;
class IEntity;
class IEntityParam;
class IProtoNode;
class IProto;
class IProtoLib;
class IEntityMap;
class IEntityGlobal;

class ISceneBaker;
class ITrisBaker;
class IMiniMapBaker;
class IOutlineMapBaker;
class IGtiBaker;

class IEnvLight;

class ISptLib;
class ISptForest;

struct ProfilerMgr;

struct TrrnParam;

class IAnimNode;
class IAnimNodeMat;
class IAnimNodeSkeleton;
class IAnimNodeSkin;

class IMano;
