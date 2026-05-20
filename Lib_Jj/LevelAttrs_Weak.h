#pragma once

#include "class/class.h"

#include "gds/GObjUID.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

#include "LevelAttrs_DamageAttr.h"

struct LevelWeaksPack
{
	LevelWeaksPack()
	{
		Zero();
	}
	void Zero()
	{
		memset(weaks,0,sizeof(weaks));
	}

	void MergeFrom(LevelWeaksPack &wkpkOther)
	{
		for (int i=0;i<LevelWeakCategory_Count;i++)
			weaks[i]|=wkpkOther.weaks[i];
	}

	void MergeFrom(DamageAttrMask weaks_)
	{
		for (int i=0;i<LevelWeakCategory_Count;i++)
			weaks[i]|=weaks_;
	}
	void MergeFrom(DamageAttrType weak)
	{
		MergeFrom(1<<weak);
	}

	void Filter(LevelWeaksPack &wkpkOther)
	{
		for (int i=0;i<LevelWeakCategory_Count;i++)
			weaks[i]&=wkpkOther.weaks[i];
	}

	void Exclude(LevelWeaksPack &wkpkOther)
	{
		for (int i=0;i<LevelWeakCategory_Count;i++)
			weaks[i]&=~wkpkOther.weaks[i];
	}

	DamageAttrMask weaks[LevelWeakCategory_Count];

};



struct LevelAttr_Weaks
{
	DEFINE_CLASS(LevelAttr_Weaks);

	LevelAttr_Weaks()
	{
		Zero();
	}
	void Zero()
	{
		wkpks.clear();
	}

	void CopyFrom(LevelAttr_Weaks &src)
	{
		wkpks=src.wkpks;
	}

	LevelWeaksPack&Cur()
	{
		_EnsureContent();
		return wkpks[wkpks.size()-1];
	}

	void Push()
	{
		_EnsureContent();
		wkpks.push_back(wkpks[wkpks.size()-1]);
	}

	void Pop()
	{
		_EnsureContent();
		if (wkpks.size()>1)
			wkpks.pop_back();
	}

	void _EnsureContent()
	{
		if (wkpks.size()<=0)
			wkpks.resize(1);
	}

	std::deque<LevelWeaksPack> wkpks;

}; 

struct WeaksEx
{
	void ToAttr(LevelAttr_Weaks &attr)
	{
		ToWeakPack(attr.Cur());
	}

	void ToWeakPack(LevelWeaksPack &wkpk)
	{
		if (categories&1)
			wkpk.weaks[LevelWeakCategory_StunFwd]=stunFwd;
		if (categories&2)
			wkpk.weaks[LevelWeakCategory_StunSide]=stunSide;
		if (categories&4)
			wkpk.weaks[LevelWeakCategory_StunBack]=stunBack;
		if (categories&8)
			wkpk.weaks[LevelWeakCategory_KB]=kb;
		if (categories&16)
			wkpk.weaks[LevelWeakCategory_Jink]=jink;
	}

	void ToString(std::string &s);

	void CopyFrom(WeaksEx &other)
	{
		categories=other.categories;
		stunFwd=other.stunFwd;
		stunSide=other.stunSide;
		stunBack=other.stunBack;
		kb=other.kb;
		jink=other.jink;
		cache.bValid=FALSE;
	}

	LevelWeakCategoryMask categories;
	DamageAttrMask stunFwd;
	DamageAttrMask stunSide;
	DamageAttrMask stunBack;
	DamageAttrMask kb;
	DamageAttrMask jink;
	//XXXXX: more LevelWeakCategory

	BEGIN_GOBJ_PURE_UID(WeaksEx,1);

		GELEM_VAR_INIT(LevelWeakCategoryMask,categories,0)
			GELEM_EDITVAR("弱点类型",GVT_U,GSem(GSem_Flags,
			"硬直(前方)|硬直(前方):1,"
			"硬直(两侧)|硬直(两侧):2,"
			"硬直(背面)|硬直(背面):4,"
			"击退|击退:8,"
			"快闪|快闪:16"
			),"弱点类型");

		GELEM_VAR_INIT(DamageAttrMask,stunFwd,0);GELEM_UID(1);
			GELEM_EDITVAR("硬直(前方)",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"硬直弱点(前方)");
		GELEM_VAR_INIT(DamageAttrMask,stunSide,0);GELEM_UID(4);
			GELEM_EDITVAR("硬直(两侧)",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"硬直弱点(两侧)");
		GELEM_VAR_INIT(DamageAttrMask,stunBack,0);GELEM_UID(2);
			GELEM_EDITVAR("硬直(背面)",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"硬直弱点(背面)");
		GELEM_VAR_INIT(DamageAttrMask,kb,0);GELEM_UID(3);
			GELEM_EDITVAR("击退",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"击退");
		GELEM_VAR_INIT(DamageAttrMask,jink,0);GELEM_UID(5);
			GELEM_EDITVAR("快闪",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"快闪");
	END_GOBJ();

	void DiscardCache()
	{
		cache.bValid=FALSE;
	}


	LevelAttr_Weaks *Get()
	{
		if (!cache.bValid)
		{
			ToAttr(cache);
			cache.bValid=TRUE;
		}
		return &cache;
	}

	struct Cache:public LevelAttr_Weaks
	{
		Cache()
		{
			bValid=FALSE;
		}
		BOOL bValid;
	};

	Cache cache;


};


struct LevelAttr_WeaksMod
{
	LevelAttr_WeaksMod()
	{
		memset(this,0,sizeof(*this));
	}

	struct Override
	{
		Override()
		{
			Zero();
		}
		void Zero()
		{
			memset(this,0,sizeof(*this));
		}
		BOOL bValid;
		void * owner;
		LevelWeaksPack wkpk;
		BOOL bCanTakeOver;
		AnimTick tFinish;//截止时间
	};

	void Apply(LevelAttr_Weaks &attrWeaks)
	{
		if (overrideCur.bValid)
			attrWeaks.Cur()=overrideCur.wkpk;

		if (bFilter)
			attrWeaks.Cur().Filter(wkpkFilter);
	}

	void SetOverride(LevelWeaksPack &wkpk,void *owner,AnimTick t,BOOL bCanTakeOver)
	{
		overrideLast=overrideCur;
		overrideLast.tFinish=t;

		overrideCur.bValid=TRUE;
		overrideCur.wkpk=wkpk;
		overrideCur.owner=owner;
		overrideCur.bCanTakeOver=bCanTakeOver;
		overrideCur.tFinish=ANIMTICK_INFINITE;
	}

	void ClearOverride(void *owner,AnimTick t)
	{
		if (!overrideCur.bValid)
			return;

		if (owner!=overrideCur.owner)
			return;

		overrideLast=overrideCur;
		overrideLast.tFinish=t;

		overrideCur.Zero();
	}

	void SetFilter(LevelWeaksPack &wkpk,void *owner)
	{
		bFilter=TRUE;
		wkpkFilter=wkpk;
		ownerFilter=owner;
	}

	void ClearFilter(void *owner)
	{
		if (ownerFilter!=owner)
			return;
		if (!bFilter)
			return;

		bFilter=FALSE;
		wkpkFilter.Zero();
		ownerFilter=NULL;
	}

	Override overrideCur;
	Override overrideLast;

	BOOL bFilter;
	LevelWeaksPack wkpkFilter;
	void *ownerFilter;

};
