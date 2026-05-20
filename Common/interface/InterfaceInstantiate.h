#pragma once

#include "interface.h"


//II means Interface Instantiate

//Internal use
extern void *II_AcuireXS(const char *path,const char *name);
//

extern void II_Init(HMODULE hMod);
extern void II_UnInit();

#define II_AccuireFS01(pSys) \
	(pSys)=(IFileSystem*)II_AcuireXS("FileSystem._dll","FileSystem01")
#define II_AccuireFS02(pSys) \
	(pSys)=(IFileSystem*)II_AcuireXS("FileSystem._dll","FileSystem02")
#define II_AccuireFS03(pSys) \
	(pSys)=(IFileSystem*)II_AcuireXS("FileSystem._dll","FileSystem03")
#define II_AccuireRS(pSys) \
	(pSys)=(IRenderSystem*)II_AcuireXS("RenderSystem._dll","RenderSystem01")
#define II_AccuireUtilRS(pSys) \
	(pSys)=(IUtilRS*)II_AcuireXS("RenderSystem._dll","UtilRS01")
#define II_AccuireRSMT(pSys) \
	(pSys)=(IRenderSystem*)II_AcuireXS("RenderSystemMT._dll","RenderSystem01")
#define II_AccuireUtilRSMT(pSys) \
	(pSys)=(IUtilRS*)II_AcuireXS("RenderSystemMT._dll","UtilRS01")
#define II_AccuireWSMT(pSys) \
	(pSys)=(IWorldSystem*)II_AcuireXS("WorldSystemMT._dll","WorldSystem01")
#define II_AccuireSS01(pSys)	 \
	(pSys)=(ISscSystem*)II_AcuireXS("SscSystem._dll","SscSystem01")
#define II_AccuireSS02(pSys)	 \
	(pSys)=(ISscSystem*)II_AcuireXS("SscSystem._dll","SscSystem02")
#define II_AccuireMF(pMF)\
	(pMF)=(IMapFile*)II_AcuireXS("FileSystem._dll","MapFile01")
#define II_AccuireMF2(pMF2)\
	(pMF2)=(IMapFile*)II_AcuireXS("FileSystem._dll","MapFile2_01")
#define II_AccuirePS(pPS)\
	(pPS)=(IPhysicsSystem*)II_AcuireXS("PhysicsSystem._dll","PhysicsSystem01")
#define II_AccuireKD(pKD)\
	(pKD)=(IKinectDevice*)II_AcuireXS("KinectDevice._dll","KinectDevice01")


#define II_AccuireSVN(pSys)	 \
	(pSys)=(ISscSystem*)II_AcuireXS("SscSystem._dll","SscSystem_svn")