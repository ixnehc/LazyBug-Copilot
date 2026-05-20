#pragma once

#include "LevelDefines.h"
#include "strlib/strlibdefines.h"

#define AgentBriefIcon_TexPath "界面\\小地图\\icons.dds"

class CDataPacket;
struct LevelAgentBrief
{
	enum Type
	{
		None=0,
		Default=1,

		ForceDword=0xffffffff,
	};

	LevelAgentBrief()
	{
	}

	void CopyFrom(LevelAgentBrief &other)
	{
		pos=other.pos;
		idAgent=other.idAgent;
		initial.CopyFrom(other.initial);
		cur.CopyFrom(other.cur);
		infosTreasure.CopyFrom(other.infosTreasure);
	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	void ClearCur();

	LevelPos3D pos;
	RecordID idAgent;

	struct Brief
	{
		Brief()
		{
			tp=None;
			tip=StringID_Invalid;
		}
		BOOL IsEmpty()
		{
			return tp==None;
		}
		void CopyFrom(Brief &other)
		{
			tp=other.tp;
			rcOnIconTex=other.rcOnIconTex;
			ptIconAnchor=other.ptIconAnchor;
			tip=other.tip;
		}
		void Save(CDataPacket &dp);
		void Load(CDataPacket &dp);
		BOOL Equals(Brief &other);

		Type tp;
		i_math::rect_sh rcOnIconTex;
		i_math::pos2db ptIconAnchor;//归一化坐标(x和y都是0..100之间的值)
		StringID tip;
	};

	Brief initial;
	Brief cur;

	struct TreasureInfos
	{
		TreasureInfos()
		{
			nEntries=0;
		}
		struct Entry
		{
			Entry()
			{
				memset(this,0,sizeof(*this));
			}
			BOOL CheckCompatible(Entry &other)
			{
				if (guidProvider!=other.guidProvider)
					return FALSE;
				if (tpRes!=other.tpRes)
					return FALSE;
				else
				{
					if (tpRes==LevelResource_None)
					{
						if (idItem!=other.idItem)
							return FALSE;
					}
				}
				return TRUE;
			}
			RecordID idItem;
			DWORD tpRes:4;
			DWORD bRevealed:1;
			DWORD count:16;
			DWORD possibility:7;//0~100
			LevelGUID guidProvider;
		};

		void CopyFrom(TreasureInfos &other)
		{
			nEntries=other.nEntries;
			memcpy(entries,other.entries,nEntries*sizeof(entries[0]));
		}
		void Save(CDataPacket &dp);
		void Load(CDataPacket &dp);

		Entry entries[32];
		DWORD nEntries;
	};

	TreasureInfos infosTreasure;

};

