
#pragma  once

enum CompositorType
{
	Comp_HDR = 0x1,
	Comp_Gamma = 0x2,
	Comp_DOF = 0x4,
	Comp_MotionBlur = 0x8,
	Comp_Glow		= 0x10,
	Comp_Outline		= 0x20,
};

//中间处理所需的贴图类型,由于对处理过程的优化
enum CompTexType
{
	CTDown10Col,	
	CTDown10Lum,	
};

class IAnimTreeCtrl;
class ICompositorManager
{
public:
	virtual IAnimTreeCtrl * GetAnimTreeCtrl(CompositorType compType) = 0;
	virtual BOOL Tick(AnimTick t) = 0;
};


