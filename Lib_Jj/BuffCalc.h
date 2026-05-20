#pragma once

#include "class/class.h"


typedef DWORD BuffFlag;

#define BuffFlag_Birth 1
#define BuffFlag_Dead 2
#define BuffFlag_LayDown 4
#define BuffFlag_Pausing 8
#define BuffFlag_PausingAnim 16
#define BuffFlag_Reside 32
#define BuffFlag_Flying 64
#define BuffFlag_Teleport 128
#define BuffFlag_NotAttackable 256
#define BuffFlag_GhostCollide 512//没有碰撞
#define BuffFlag_Dormant 1024//休眠
#define BuffFlag_SlideMove 2048//滑行移动,方向由外部控制,(而不是始终朝向移动方向)
#define BuffFlag_Mount 4096//骑在其它单位上面
#define BuffFlag_Invisible 8192//隐身
#define BuffFlag_DamageImmune 16384
#define BuffFlag_TrampleTarget 32768


#define BuffFlagsCheck_SemConstraint "没有需求,可攻击对象,休眠对象"

class CBuffFactor
{
public:
	virtual BOOL IsAlive()	{		return FALSE;	}

	virtual BuffFlag GetFlags()	{		return 0;	}
	virtual float GetSlow()	{		return 0.0f;	}//冷冻程度
	virtual float GetIMS()	{		return 0.0f;	} //Increased Moving Speed
	virtual float GetIAS()	{		return 0.0f;	} //Increased Attack Speed
	
};

class CBuffFormular
{
public:
	CBuffFormular()
	{
		_flags=0;
		_bFlagDirty=TRUE;
	}
	void SetBuffs(std::vector<CBuffFactor*>*buffs)
	{
		_buffs=buffs;
	}

	float GetIMS();//移动速度修正
	float GetIAS();//技能速度修正

	void MarkFlagsDirty()
	{
		_bFlagDirty=TRUE;
	}

	void AddFlag(BuffFlag flag)
	{
		if (!_bFlagDirty)
			_flags|=flag;
	}

	BOOL TestFlag(BuffFlag flag)
	{
		if (_bFlagDirty)
		{
			_flags=0;
			if (_buffs)
			{
				for (int i=0;i<_buffs->size();i++)
				{
					CBuffFactor *p=(*_buffs)[i];
					if (!p)
						continue;
					if (!p->IsAlive())
						continue;
					_flags|=p->GetFlags();
				}
			}
			_bFlagDirty=FALSE;
		}

		return (_flags&flag)!=0;
	}


protected:
	BuffFlag _flags;
	BOOL _bFlagDirty;

	std::vector<CBuffFactor*>*_buffs;


};