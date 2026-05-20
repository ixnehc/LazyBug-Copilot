
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"

#include "brokenLine/BrokenLineEx.h"


struct BrokenLineRef
{
	BrokenLineRef()
	{
		bl = NULL;
		def = 1.0f;
	}
	BrokenLineEx *bl;
	float def;
};

template< int T_count >
struct PropBrokenLineBase : public GProperty
{
	virtual void DeleteThis()
	{
		for ( int i = 0;i < T_count; ++i )
			SAFE_RELEASE( bls[i].bl );
		Class_Delete(this);
	}
	//-------------------------------------------------------------------------------------------------------
	// Trick:这个Property的Clone比较古怪,是因为以下原因:
	// 1. 这个Property内部包含了一些带引用计数的指针
	// 2. Property会被保留在Stub的静态成员变量里作为缺省值,我们不能在这个缺省值里保留指针,
	//	  因为静态成员变量会很晚才被析构(晚于mempool的析构函数),这样会造成mempool报内存泄漏的错)
	// 3. 所以我们在为Property设缺省值时直接设到一个数值变量里(def)
	// 4. 当这个Property被clone时,我们再new一个BrokenLine的指针,并根据def为它设初始值
	// 5. 我们可以这么干是基于,每个保存在stb缺省值中的Property是不会被直接使用的,它肯定会被Clone()一下再使用
		//-------------------------------------------------------------------------------------------------------
	virtual GProperty* Clone()
	{
		PropBrokenLineBase* ret = (PropBrokenLineBase*)GetClass()->New();
		for ( int i = 0; i < T_count; ++i )
		{
			if ( !bls[i].bl )
			{
				ret->bls[i].bl = Class_New2(BrokenLineEx);
				ret->bls[i].bl->m_vecPoint.push_back( i_math::pos2df( 0.0f, bls[i].def) );
			}
			else
			{
				ret->bls[i] = bls[i];
			}
			SAFE_ADDREF( ret->bls[i].bl );
		}
		return ret;
	}

	virtual void Save( CDataPacket &dp )
	{
		dp.Data_NextWord() = 0;//版本号
		for ( int i = 0; i < T_count; ++i )
		{
			BrokenLineRef *p = &bls[i];
			dp.Data_NextFloat() = p->def;
			if ( !p->bl )
			{
				dp.Data_NextByte() = 0;
			}
			else
			{
				dp.Data_NextByte() = 1;
				p->bl->Save( dp );
			}
		}
	}

	virtual BOOL Load( CDataPacket &dp, BOOL *bRepaired )
	{
		WORD ver = dp.Data_NextWord();

		for (int i = 0; i < T_count; ++i )
		{
			BrokenLineRef *p = &bls[i];
			p->def = dp.Data_NextFloat();
			if ( 0 == dp.Data_NextByte() )
			{
				SAFE_RELEASE( p->bl );
			}
			else
			{
				if ( NULL == p->bl )
				{
					p->bl= Class_New2( BrokenLineEx );
					SAFE_ADDREF( p->bl );
				}
				if ( FALSE == p->bl->Load( dp, bRepaired ) )
					return FALSE;
			}
		}
		return TRUE;
	}

	BrokenLineRef bls[T_count];
};

struct Prop_BrokenLine : public PropBrokenLineBase<1>
{
	DEFINE_CLASS( Prop_BrokenLine );
	BEGIN_GOBJ_PURE( Prop_BrokenLine, 1 );
		GELEM_VAR_ALIAS( BrokenLineRef, bls[0], "bls[0]" );
			GELEM_EDITVAR( "Base", GVT_None, GSem_BrokenLine, "Base控制线" );
	END_GOBJ();

	void SetDefault( float fDef )
	{
		bls[0].def = fDef;
	}
};

