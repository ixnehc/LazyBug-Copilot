#include "stdh.h"

#include "LevelAttrs_Weak.h"

static void Append(DamageAttrMask weaks,DamageAttrType attrDmg,const char *nmFlag,std::string &s)
{
	if (weaks&(1<<attrDmg))
	{
		if (s!="")
			s+=",";
		s+=nmFlag;
	}
}

static void ToString(DamageAttrMask weaks,std::string &s)
{
	s="";

	Append(weaks,DamageAttrType_Pierce,"穿刺",s);
	Append(weaks,DamageAttrType_Crush,"重击",s);
	Append(weaks,DamageAttrType_Fire,"火",s);
	Append(weaks,DamageAttrType_Lightning,"电",s);
	Append(weaks,DamageAttrType_Cold,"冰",s);
	Append(weaks,DamageAttrType_Poison,"毒",s);
	Append(weaks,DamageAttrType_Explosion,"爆炸",s);
	Append(weaks,DamageAttrType_CryticalBlocking,"Crytical格挡",s);
	Append(weaks,DamageAttrType_SpecialA,"特殊A",s);
	Append(weaks,DamageAttrType_Smash,"砸碎",s);
	//XXXXX: More DamageAttrType
}

static void Append(std::string &s,LevelWeakCategoryMask maskCategory,LevelWeakCategory category,DamageAttrMask weaks,const char *nmCategory)
{
	if (maskCategory&(1<<(category-1)))
	{
		static std::string ss;
		ToString(weaks,ss);
		if (ss!="")
		{
			if (s!="")
				s+="\n";
			s+=nmCategory;
			s+=';';
			s+=ss;
		}

	}
}

void WeaksEx::ToString(std::string &s)
{
	s="";
	Append(s,categories,LevelWeakCategory_StunFwd,stunFwd,"硬直(前方)");
	Append(s,categories,LevelWeakCategory_StunSide,stunSide,"硬直(两侧)");
	Append(s,categories,LevelWeakCategory_StunBack,stunBack,"硬直(背后)");
	Append(s,categories,LevelWeakCategory_KB,kb,"击倒");
	Append(s,categories,LevelWeakCategory_Jink,jink,"快闪");

	if (s=="")
		s="(无)";
}


