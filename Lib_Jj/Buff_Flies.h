#pragma once

#include "LevelBuff.h"


struct BuffParam_Flies:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Flies);

	BEGIN_GOBJ_PURE(BuffParam_Flies,1);

		GELEM_VAR_INIT(int,count,10);
			GELEM_EDITVAR("个数",GVT_S,GSem_Interger,"个数");
		GELEM_VAR_INIT(float,speed,4.0f);
			GELEM_EDITVAR("速度",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"速度");
		GELEM_VAR_INIT(float,radiusEnchant,0.25f);
			GELEM_EDITVAR("Enchant半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"Enchant半径");
		GELEM_VAR_INIT(float,radiusUnenchant,0.5f);
			GELEM_EDITVAR("取消Enchant半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"取消Enchant半径");

	END_GOBJ();

	int count;
	float speed;
	float radiusEnchant;
	float radiusUnenchant;


};

struct BuffArg_Flies:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Flies);
};



class Buff_Flies:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Flies,38)

	Buff_Flies()
	{
		_count=0;
		_bEnchanted=FALSE;
		_enchantedOverriden=0;
		_form=Form_Default;
	}

	enum Form
	{
		Form_Default,
		Form_Scattered,
	};

	BOOL IsEnchanted()	
	{		
		if (_enchantedOverriden==1)
			return TRUE;
		if (_enchantedOverriden==2)
			return FALSE;
		return _bEnchanted;	
	}

	virtual BOOL NeedSync()  override	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()  override	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Flies的Buff并存的情况

	virtual LevelBuffMask GetReplaceBuffs()  override;

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnUpdate(AnimTick dt)  override;
	virtual void _OnDestroy() override;

	virtual void _WriteData(CBitPacket *dp) override;

	void OverrideEnchanted(BOOL bEnchanted);
	void ClearOverrideEnchanted()	{		_enchantedOverriden=0;	}

	void SetForm(Form form)	{		_form=form;	}

protected:
	int _enchantedOverriden;//0表示没有override,1表示重载为enchanted,2表示重载为unenchanted
	BOOL _bEnchanted;
	Form _form;
	DWORD _count;

	LevelPos _pos;
	LevelPos _vel;

};

