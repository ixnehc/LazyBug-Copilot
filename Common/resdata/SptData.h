#pragma once

#include <string>
#include <vector>
#include "math/vector2d.h"

#include "ResData.h"
#include "math/capsule.h"

class CDataPacket;
//SptData存储一棵树的所有数据(对应于一个spt文件),用户有了SptData后,就可以不需要CSpeedTreeRT
//主要包括:
//1.树各部分的顶点数据
//2.需要用到的贴图路径名
//3.各个lod的数据(在0~1之间采样,比如说100个,记录下来)
//4.这棵树专用的风的信息,记录在SptWindCfg结构里,可以允许有多个

struct SptData:public ResData
{
	enum 
	{
		Res_Ver = 4,
	};

	DECLARE_CLASS(SptData);
	virtual 	ResType GetType()	{		return Res_Spt;	}
	virtual const char *GetTypeName()	{		return "SpeedTree";	}
	virtual const char *GetTypeSuffix()	{		return "spt";	}
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CollectRefs(std::vector<std::string>&buf);

	//Note:ResData could be loaded only the header data ,which is small and could be used
	//to access some brief information.
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);
	
	DWORD GetVtxNumberBr();
	DWORD GetVtxNumberFr();

public:

	std::vector<BYTE> branchVB;
	std::vector<BYTE> frondVB;
	std::vector<BYTE> leafCardVB;
	std::vector<BYTE> leafMeshVB;

	std::vector<WORD> sptIB;
	
	DWORD  branchLODs[MAX_LOD_LEVEL]; 
	DWORD  frondLODs[MAX_LOD_LEVEL];

	DWORD  leafCardLODs[MAX_LOD_LEVEL];//  index to vertex buffer.

	DWORD  leafMeshLODs[MAX_LOD_LEVEL];

	__int64 fvfBranch;
	__int64 fvfFrond;
	__int64 fvfLeafCard;
	__int64 fvfLeafMesh;

	float fLeafRockScale;
	float fLeafRusltScale;

	BYTE numLeafCardLods;
	BYTE numBranchLods;
	BYTE numFrondLods;
	BYTE numLeafMeshLods;

	std::string mapCompisiteDif;
	std::string mapCompisiteNor;
	std::string mapBranchDif;	
	std::string mapBranchNor;
	
	// for future use.
	std::vector<SptWndCfg>		cfgwinds;
	std::vector<std::string>	namewinds;

	// flat uv coordinate
	std::vector<i_math::vector2df> uvBranch;
	std::vector<i_math::vector2df> uvFrond;

	DWORD numLods;
	float transitionDists[MAX_LOD_LEVEL];
	float transitionPrecent[MAX_LOD_LEVEL];

	i_math::aabbox3df aabb;  	// bounding box of tree model.
	
	// for accurate collision detect. for perfect world.
	std::vector<i_math::capsulef>  capus;  
	std::vector<i_math::spheref>   sphs;
	std::vector<i_math::aabbox3df> obbs;

	//叶子的包围球
	i_math::spheref leafboundSph;
	
	//光照贴图相关 nPixel最小的光照贴图的大小
	float nPixel; 	
	i_math::size2df szBr,szFr;

	DWORD ver;
};

void CalcBoundBox(SptData * resData); //计算BoundBox

//封装如何根据面积因子 和 宽高 比计算 所需贴图的大小
struct LMSize
{
	LMSize(){
		_nPixel = 0;
		_szBr.set(0,0);
		_szFr.set(0,0);
	}
	LMSize(i_math::size2df szBr,i_math::size2df szFr,float nPixel){
		Set(szBr,szFr,nPixel);
	}
	void Set(i_math::size2df szBr,i_math::size2df szFr,float nPixel){
		_nPixel = nPixel;
		if(nPixel==0){
			_szBr.set(0,0);
			_szFr.set(0,0);
		}
		else{
			_szBr = szBr;
			_szFr = szFr;
		}
	}
	void GetMapSize(i_math::size2di &szBr,i_math::size2di &szFr,float rScale)
	{
		szBr.w = 4*((int(rScale*_szBr.w)+3)/4);
		szBr.h = 4*((int(rScale*_szBr.h)+3)/4);

		szFr.w = 4*((int(rScale*_szFr.w)+3)/4);
		szFr.h = 4*((int(rScale*_szFr.h)+3)/4);
	}

	float _nPixel;
	i_math::size2df _szBr,_szFr;
};

struct CSptLMUVGen
{
	//分类连通组
	//////////////////////////////////////////////////////////////////////////
	struct _Face{
		int vtx[3];
	};

	struct _Vtx{
		_Vtx(){flag = 0;grp = -1;idxRef = -1;}

		//描述邻接点的属性
		struct _NearVtx{
			_NearVtx(int i0){idx = i0;flag = 0;}
			_NearVtx(){flag = 0;}
			int idx;
			int flag; //描述与该点相连边的情况
		};

		std::vector<int> faces;			//含有该顶点的面
		std::vector<_NearVtx> nearVtxs; //与该顶点直接相连的顶点		
		int flag;						//顶点的类型	
		int grp;
		int idxRef;		

		//增加引用的面
		void addFace(int face,int i0,int i1){
			assert(i0>=0&&i1>=0);
			faces.push_back(face);
			for(int i = 0;i<nearVtxs.size();i++){
				_NearVtx & v = nearVtxs[i];
				if(v.idx==i0){
					i0 = -1;			
					v.flag++;
				}
				if(v.idx==i1){
					i1 = -1;
					v.flag++;
				}
			}
			if(i0<0||i1<0)
				flag++;

			if(i0>=0)
				nearVtxs.push_back(_NearVtx(i0));
			if(i1>=0)
				nearVtxs.push_back(_NearVtx(i1));
		}
	};

	struct _UniVtx{
		i_math::vector3df pos;
		_UniVtx(i_math::vector3df p){ pos = p;}
		bool operator <(const _UniVtx & oth) const{
			return (pos.x<oth.pos.x ||
				((pos.x==oth.pos.x)&&(pos.y<oth.pos.y))||
				((pos.x==oth.pos.x)&&(pos.y==oth.pos.y)&&(pos.z<oth.pos.z)));
		}
	};

	struct _GrpUV
	{
		_GrpUV(){w = 0;rcSrc.set(2.0f,2.0f,-2.0f,-2.0f);ulen = 1.0f;vlen = 1.0f;}
		bool operator < (const _GrpUV &oth)const {return w>oth.w;} //逆序 从大到小
		float w;				  // 该组顶点的面积因子，值越大表示该块占用的面积越大
		i_math::rectf     rcSrc;  // 原有的uv坐标
		i_math::rectf     rcDst;  // 变换后的uv坐标
		i_math::vector2df kb[2];  // u ,v 方向变换方程系数 y = k(x-b); .x:k  .y:b
		std::vector<int> idxs;
		float ulen;
		float vlen;
	};

	struct _Segment{
		_Segment(i_math::rectf &rc,int extype){ rcSeg = rc;extType = extype;idxExts.clear();}
		i_math::rectf rcSeg;
		std::vector<int> idxExts; //记录如果不能被填充时，需要用来延伸的矩形索引
		int extType;				 //延伸的类型  0：向左  1：向下
	};

	struct _TotalEdge  //用于记录边界引用的矩形索引
	{
		std::vector<int> leftEdges;
		std::vector<int> bottomEdges;
	};
	
public:
	CSptLMUVGen();
	//生成平铺的UV, nPixel 每米多少个象素，validPixel u或v向最少有效象素个数
	void Gen(SptData * data,int nPixel,int validPixel,int gapPixel);
	void GetNumberOfSegments(DWORD & nGrpBr,DWORD & nGrpFr); //得到Segment组的个数
	BOOL GetGrpInfoBr(int * grp,int nVtx); //得到顶点的组信息
	BOOL GetGrpInfoFr(int * grp,int nVtx); //得到顶点的组信息
	float GetBranchArea(); // 得到面积因子
	float GetFrondArea();  // 
private:
	
	BOOL _GetGrpInfo(int * grp,int nVtx,
					 std::vector<_Vtx> & vtxs);
	//按照连通性分类
	int _ConstructSeg(std::vector<i_math::vector3df> & posVtxs,
						std::vector<WORD> & indices,std::vector<_Face> &faces,
						std::vector<_GrpUV> &grps,std::vector<_Vtx> & vtxs);
	
	int _GrpID(std::vector<_Vtx> & vtxs,int idx);//得到组的索引
	//生成平铺的UV ,返回该段模型的表面积系数
	i_math::size2df _GenerateUV(SptData * data,std::vector<i_math::vector3df>&posVtxs,
					std::vector<i_math::vector2df>&uvVtxs,std::vector<_GrpUV> &grps,
					std::vector<_Vtx> & vtxs,std::vector<WORD>& indices);
	i_math::rectf _PackageQuad(std::vector<_GrpUV> &grps); //对uv矩形进行打包
	//已知一个矩形,从组列表中找到一个最贴合已知矩形的矩形区域,并返回切剩的矩形区域
	int _FindBestQuad(std::vector<_GrpUV> &grps,i_math::rectf & rcFix,_TotalEdge & idxSrc,std::vector<_Segment> &rcSegments); 
	//已知一个矩形找到一个能放入其中，且余下的面积最小的那一个
	int _FindQuadIn(std::vector<_GrpUV> &grps,i_math::rectf &rcCut,std::vector<_Segment> &rcSegments); 

	void _CalcGeomLenProjUV( std::vector<_GrpUV>&grps,
							 std::vector<i_math::vector3df> &posVtxs,
							 std::vector<i_math::vector3df> &norVtxs,
							 std::vector<i_math::vector3df> &tanVtxs,
							 std::vector<i_math::vector2df> &uvVtxs);
	//检查面的有效性
	BOOL _CheckFace(int * indices,int r); //
	float _AreaTri(i_math::vector3df * pos);	//计算三角形的面积
	
	
	//确保当前块最小要含有多少个象素
	void _InsurePixelWidth(i_math::rectf &rc);

	//得到trianglelist形式的索引并除去无效的面 (s到e段的三角形)
	void _FetchIndex(SptData * data,DWORD s,DWORD e,i_math::vector3df * pos,std::vector<WORD>&indices);
	void _FetchVtxBr(SptData *data);
	void _FetchVtxFr(SptData *data);
	
	void _DoChange(SptData * data); //将改变后的UV平铺加入到顶点结构体中

	//假定不同的顶点含有不同的位置
	std::vector<_Face> _facesBr;
	std::vector<_Face> _facesFr;
	std::vector<_Vtx>  _vtxsBr;
	std::vector<_Vtx>  _vtxsFr;
	
	//顶点的位置
	std::vector<i_math::vector3df> _posVtxsBr;
	std::vector<i_math::vector3df> _posVtxsFr;

	//顶点的纹理坐标值
	std::vector<i_math::vector2df> _posUVsBr;
	std::vector<i_math::vector2df> _posUVsFr;

	//法线
	std::vector<i_math::vector3df> _norVtxsBr;
	std::vector<i_math::vector3df> _norVtxsFr;

	//切线
	std::vector<i_math::vector3df> _tanVtxsBr;
	std::vector<i_math::vector3df> _tanVtxsFr;

	//组信息
	std::vector<_GrpUV> _grpsBr;
	std::vector<_GrpUV> _grpsFr;
	
	//顶点的索引值
	std::vector<WORD> _indicesBr;
	std::vector<WORD> _indicesFr;

	DWORD _nGrpsBr;
	DWORD _nGrpsFr;

	i_math::size2df _szBr,_szFr;
	
	float _nPixel;
	float _validPixel;
	float _gap;
};





