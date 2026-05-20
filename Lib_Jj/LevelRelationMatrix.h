#pragma once

#include "LevelDefines.h"

struct LevelRelationRow
{
	LevelRelationRow()
	{
		enemy=ally=0;
	}
	LevelPlayerMask enemy;
	LevelPlayerMask ally;
};

struct LevelRelationMatrix
{
	LevelRelationMatrix()
	{
		for (int i=0;i<LEVEL_MAX_PLAYER;i++)
		{
			SetEnemy(i,LevelPlayerID_Wild);
		}
		for (int i=0;i<LEVEL_MAX_PLAYER;i++)
		{
			SetAlly(i,LevelPlayerID_PlayerWild);
		}

		SetEnemy(LevelPlayerID_PlayerWild,LevelPlayerID_Wild);

	}
	void SetEnemy(LevelPlayerID idPlayer1,LevelPlayerID idPlayer2)
	{
		rows[idPlayer1].enemy|=(1<<idPlayer2);
		rows[idPlayer2].enemy|=(1<<idPlayer1);

		rows[idPlayer1].ally&=(~(1<<idPlayer2));
		rows[idPlayer2].ally&=(~(1<<idPlayer1));
	}
	void SetAlly(LevelPlayerID idPlayer1,LevelPlayerID idPlayer2)
	{
		rows[idPlayer1].ally|=(1<<idPlayer2);
		rows[idPlayer2].ally|=(1<<idPlayer1);

		rows[idPlayer1].enemy&=(~(1<<idPlayer2));
		rows[idPlayer2].enemy&=(~(1<<idPlayer1));
	}

	LevelPlayerMask GetEnemies(LevelPlayerID idPlayer)
	{
		return rows[idPlayer].enemy;
	}
	LevelPlayerMask GetAllies(LevelPlayerID idPlayer)
	{
		return rows[idPlayer].ally;
	}

	LevelRelationRow rows[16];

};