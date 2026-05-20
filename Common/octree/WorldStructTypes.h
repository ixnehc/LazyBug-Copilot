
#pragma  once
#include <vector>

//4 means 4 bytes
struct TriangleDesc4
{
	unsigned short m_idxTriangle;
	unsigned char m_xOctTree,m_yOctTree;
};

//4 means 4 bytes
struct OctTreeNodeDesc4
{
	unsigned short m_idxNode;
	unsigned char m_xOctTree,m_yOctTree;
	inline void Set(unsigned short idx,unsigned char x,unsigned char y)
	{
		m_xOctTree=x;
		m_yOctTree=y;
		m_idxNode=idx;
	}
	inline bool Equals(OctTreeNodeDesc4 &src)
	{
		return ((m_idxNode==src.m_idxNode)&&
					(m_xOctTree==src.m_xOctTree)&&
					(m_yOctTree==src.m_yOctTree));
	}
};


enum TriangleFlag
{
	TriFlag_Standable=0x01,
};
