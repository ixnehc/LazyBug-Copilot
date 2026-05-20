
#include "stdh.h"

#include <fstream>

#include "commondefines/general_stl.h"

#include "records/records.h"

#include "LevelResources.h"

#include "log/LogDump.h"

#include "resdata/ResDataDefines.h"
#include "resdata/ResData.h"

#include "resdata/AnimData.h"

#include "stringparser/stringparser.h"

#include "LevelRecords.h"
#include "LevelRecordRes.h"

//////////////////////////////////////////////////////////////////////////
//LevelPathes
void LevelPathes::Clear()
{
	if (TRUE)
	{
		std::unordered_map<StringID,LevelPath *>::iterator it;
		for (it=pathes.begin();it!=pathes.end();it++)
		{
			LevelPath *p=(*it).second;
			Safe_Class_Delete(p);
		}
		pathes.clear();
	}

	if (TRUE)
	{
		std::unordered_map<StringID,LevelLoResoPath *>::iterator it;
		for (it=pathesLoReso.begin();it!=pathesLoReso.end();it++)
		{
			LevelLoResoPath *p=(*it).second;
			Safe_Class_Delete(p);
		}
		pathesLoReso.clear();
	}

	Zero();

}

LevelPathesEvent *LevelPathes::FindEvent(StringID nmEvent)
{
	for (int i=0;i<events.size();i++)
	{
		if (events[i].name==nmEvent)
			return &events[i];
	}
	return NULL;
}



extern ResData *ResData_New(ResType tp);
extern void ResData_Delete(ResData *p);

void CLevelResources::Clear()
{
	if (TRUE)
	{
		std::unordered_map<RecordID,ResData *>::iterator it;
		for (it=_entries.begin();it!=_entries.end();it++)
		{
			ResData *data=(*it).second;
			ResData_Delete(data);
		}
		_entries.clear();
	}

	if (TRUE)
	{
		std::unordered_map<RecordID,LevelPathes *>::iterator it;
		for (it=_pathes.begin();it!=_pathes.end();it++)
		{
			LevelPathes *p=(*it).second;
			p->Clear();
			Safe_Class_Delete(p);
		}
		_pathes.clear();
	}

	Zero();
}


void CLevelResources::Init(const char *pathRoot,CLevelRecords *recs)
{
	_pathRoot=pathRoot;

	CRecords *records=recs->GetRecords_Resource();
	if (!records)
		return;

	DWORD c;
	RecordID*ids=records->GetRecords(c);

	for (int i=0;i<c;i++)
	{
		LevelRecordRes *rec=(LevelRecordRes *)records->GetRecord(ids[i]);
		if (!rec)
			continue;

		_LoadRes(ids[i],rec->path.c_str(),1.0f/rec->scalePathSpeed,rec->facePath);

	}

}

void CLevelResources::_LoadRes(RecordID id,const char *path_,float scaleTime,float facePath)
{
	if (FALSE)
	{
		float euler1=i_math::wrap_radian(LevelFaceToEuler(1.0f));
		euler1=i_math::wrap_radian(LevelFaceToEuler(1.2f));
		euler1=i_math::wrap_radian(LevelFaceToEuler(1.4f));
		euler1=i_math::wrap_radian(LevelFaceToEuler(1.5f));
		euler1=i_math::wrap_radian(LevelFaceToEuler(1.6f));
		euler1=i_math::wrap_radian(LevelFaceToEuler(1.7f));
		i_math::xformf xfm;
		xfm.pos.set(10.0f,20.0f,30.0f);
		i_math::vector3df euler;
		euler.x=1.35f;
		xfm.rot.fromEuler(euler);

		i_math::matrix43f mat;
		xfm.getMatrix(mat);
		xfm.fromMatrix(mat);

		xfm.rot.toEuler(euler);

	}

	if (_entries.find(id)!=_entries.end())
		return;//已经有了

	if (!path_[0])
		return;//路径无效

	std::vector<BYTE>buf;
	LevelResFileHeader header;
	if (TRUE)
	{
		std::string path;
		path=_pathRoot;
		path+="\\";
		path+=path_;

		std::ifstream ifs;
		ifs.open(path.c_str(),std::ios_base::in|std::ios_base::binary);
		if (!ifs.is_open())
			return;

		ifs.read((char *)&header,sizeof(header));
		ifs.seekg(header.off);

		DWORD sz;
		ifs.read((char *)&sz,sizeof(sz));
		buf.resize(sz);
		ifs.read((char *)buf.data(),sz);

		ifs.close();
	}

	ResData *data0=ResData_New((ResType)header.type);
	if (!data0)
		return;

	CDataPacket dp;
	dp.SetDataBufferPointer(buf.data());

	data0->Load(dp);

	_entries[id]=data0;

	if (data0->GetType()==ResA_XForm)
	{
		XFormData *data=(XFormData *)data0;
//		_LocalizeKeySet(data->keyset);

		LevelPathes *pathes=Class_New2(LevelPathes);

		_pathes[id]=pathes;

		for (int i=0;i<data->animpieces.size();i++)
		{
			KeySet ksAP;
			_CullAPKeySet(data0,i,ksAP);

			LevelPath *path=_MakePath(data,i,ksAP,scaleTime);
			if (path)
			{
				pathes->pathes[data->animpieces[i].name]=path;
				if (!pathes->def)
					pathes->def=path;

				AnimPiece &ap=data->animpieces[i];
				for (int i=0;i<ap.events.size();i++)
				{
					LevelPathesEvent e;
					(AnimEvent&)e=ap.events[i];
					AnimTick tEvent=ANIMTICK_SAFE_MINUS(e.tEvent,ap.tStart);
					Key_xform k;
					if (!ksAP.CalcKey(tEvent,&k))
						continue;
					e.tEvent=(AnimTick)(((float)tEvent)*scaleTime);
					e.xfm=k.v;

					AnimEventZone *zone=ap.FindZone(ap.events[i]);
					if (zone)
					{
						e.zone=*zone;

						e.zone.t=ANIMTICK_SAFE_MINUS(e.zone.t,ap.tStart);
						e.zone.t=(AnimTick)(((float)e.zone.t)*scaleTime);
						for (int m=0;m<e.zone.keysFan.size();m++)
						{
							AnimEventZone::KeyFan &k=e.zone.keysFan[m];
							k.t=ANIMTICK_SAFE_MINUS(k.t,ap.tStart);
							k.t=(AnimTick)(((float)k.t)*scaleTime);
						}

						if (e.zone.keysFan.size()>0)
							e.xfm=e.zone.keysFan[0].xfmCenter;
					}
					
// 					int idx=-1;
// 					VEC_FIND_BY_ELEMENT(pathes->events,name,e.name,idx);
// 					if (idx!=-1)
// 					{
// 						LOG_DUMP_2P("CLevelResources",Log_Error,"路径资源(%s)中发现重复的事件名称!",path_,StrLib_GetStr(e.name));
// 						continue;
// 					}

					pathes->events.push_back(e);
				}

				path->facePath=facePath;
			}

			LevelLoResoPath * pathLoReso=_MakeLoResoPath(ksAP,scaleTime);
			if (pathLoReso)
			{
				pathes->pathesLoReso[data->animpieces[i].name]=pathLoReso;
				if (!pathes->defLoReso)
					pathes->defLoReso=pathLoReso;
			}

		}

		VEC_ASCEND_BY_ELEMENT(pathes->events,LevelPathesEvent,tEvent);
	}

}

void CLevelResources::_LocalizeKeySet(KeySet &ks)
{
	DWORD c=ks.GetKeyCount();
	if (c>0)
	{
		i_math::matrix43f matBaseInv;
		Key_xform *k=((Key_xform*)ks.GetKey(0));
		k->v.getMatrix(matBaseInv);
		i_math::vector3df euler;
		k->v.rot.toEuler(euler);
		matBaseInv.makeInverse();

		i_math::matrix43f mat;
		for (int i=0;i<c;i++)
		{
			((Key_xform*)ks.GetKey(i))->v.getMatrix(mat);
			mat=mat*matBaseInv;
			((Key_xform*)ks.GetKey(i))->v.fromMatrix(mat);
		}
	}

}

void CLevelResources::_CullAPKeySet(ResData *data0,int idxAP,KeySet &ksAP)
{
	ksAP.Clean();

	if (data0->GetType()!=ResA_XForm)
		return;


	XFormData *data=(XFormData *)data0;

	KeySet_Define(&ksAP,KT_XForm);

	AnimPiece &ap=data->animpieces[idxAP];

	DWORD c=ap.iEnd-ap.iStart;
	for (int i=0;i<c;i++)
	{
		Key_xform *k=(Key_xform*)data->keyset.GetKey(ap.iStart+i);
		Key_xform kLocal=*k;
		kLocal.t=ANIMTICK_SAFE_MINUS(kLocal.t,ap.tStart);
		ksAP.AddKey(kLocal);
	}
}

LevelLoResoPath * CLevelResources::_MakeLoResoPath(KeySet &ksAP,float scaleTime)
{
	LevelLoResoPath *p=Class_New2(LevelLoResoPath);

	KeySet_Define(&p->ks,KT_Floatx2);

	DWORD c=ksAP.GetKeyCount();
	int iLastKey=0;
	for (int i=0;i<c;i++)
	{
		BOOL bKey=FALSE;
		if (i==0)
			bKey=TRUE;
		if (i+1==c)
			bKey=TRUE;

		if (!bKey)
		{
			//检查下一个点是否可以作为Key
			int iNext=i+1;
			i_math::line2df line;
			line.start=((Key_xform*)ksAP.GetKey(iLastKey))->v.pos.getXZ();
			line.end=((Key_xform*)ksAP.GetKey(iNext))->v.pos.getXZ();

			const float tol=0.1f;
			BOOL bKeyNext=TRUE;
			for (int j=iLastKey+1;j<iNext;j++)
			{
				i_math::vector2df pos=((Key_xform*)ksAP.GetKey(j))->v.pos.getXZ();

				float r;
				line.getProjection(pos,r);
				if ((r<0.0f)||(r>1.0f))
				{
					bKeyNext=FALSE;
					break;
				}

				if (line.getDistTo(pos)>tol)
				{
					bKeyNext=FALSE;
					break;
				}
			}

			if (!bKeyNext)
				bKey=TRUE;
		}

		if (bKey)
		{
			Key_2f k;
			k.t=ksAP.GetKey(i)->t;
			k.t=(AnimTick)(((float)k.t)*scaleTime);
			k.v=((Key_xform*)ksAP.GetKey(i))->v.pos.getXZ();
			p->ks.AddKey(k);
			iLastKey=i;
		}
	}

	c=p->ks.GetKeyCount();

	return p;
}

LevelPath * CLevelResources::_MakePath(ResData *data0,int idxAP,KeySet &ksAP,float scaleTime)
{
	if (data0->GetType()!=ResA_XForm)
		return NULL;

	XFormData *data=(XFormData *)data0;

	LevelPath *p=Class_New2(LevelPath);

	KeySet_Define(&p->ksPos,KT_Floatx2);
	KeySet_Define(&p->ksPos3D,KT_Pos);
	KeySet_Define(&p->ksFace,KT_Float);

	if (TRUE)
	{
		AnimTick start,end;
		data->GetAPTickRange(idxAP,start,end);
		start=(AnimTick)(((float)start)*scaleTime);
		end=(AnimTick)(((float)end)*scaleTime);
		p->dur=end-start;
	}

	DWORD c=ksAP.GetKeyCount();
	int iLastKey=0;
	for (int i=0;i<c;i++)
	{
		i_math::vector2df pos=((Key_xform*)ksAP.GetKey(i))->v.pos.getXZ();
		i_math::vector3df pos3D=((Key_xform*)ksAP.GetKey(i))->v.pos;
		i_math::quatf q=((Key_xform*)ksAP.GetKey(i))->v.rot;

		if (TRUE)
		{
			Key_2f k;
			k.t=ksAP.GetKey(i)->t;
			k.t=(AnimTick)(((float)k.t)*scaleTime);
			k.v=pos;
			p->ksPos.AddKey(k);
		}

		if (TRUE)
		{
			Key_pos k;
			k.t=ksAP.GetKey(i)->t;
			k.t=(AnimTick)(((float)k.t)*scaleTime);
			k.v=pos3D;
			p->ksPos3D.AddKey(k);
		}


		if (TRUE)
		{
			Key_f k;
			k.t=ksAP.GetKey(i)->t;
			k.t=(AnimTick)(((float)k.t)*scaleTime);

			i_math::vector3df dir(0,0,1);
			dir=q*dir;

			k.v=atan2f(dir.z,dir.x);
			p->ksFace.AddKey(k);
		}
	}

	float length=0.0f;
	if (p->ksPos.GetKeyCount()>1)
	{
		i_math::vector2df posPrev=((Key_2f*)p->ksPos.GetKey(0))->v;
		for (int i=1;i<p->ksPos.GetKeyCount();i++)
		{
			i_math::vector2df posCur=((Key_2f*)p->ksPos.GetKey(i))->v;
			length+=posCur.getDistanceFrom(posPrev);
			posPrev=posCur;
		}
	}

	p->length=length;

	return p;
}

LevelPathes *CLevelResources::FindPathes(RecordID idRec)
{
	std::unordered_map<RecordID,LevelPathes *>::iterator it=_pathes.find(idRec);
	if (it==_pathes.end())
		return NULL;
	return (*it).second;
}


LevelPath *CLevelResources::FindPath(RecordID idRec,StringID nmAP)
{
	std::unordered_map<RecordID,LevelPathes *>::iterator it=_pathes.find(idRec);
	if (it==_pathes.end())
		return NULL;

	if (nmAP==StringID_Invalid)
		return (*it).second->def;
	else
	{
		LevelPathes *pathes=((*it).second);
		std::unordered_map<StringID,LevelPath *>::iterator it=pathes->pathes.find(nmAP);
		if (it!=pathes->pathes.end())
			return (*it).second;
	}
	return NULL;
}

LevelLoResoPath *CLevelResources::FindLoResoPath(RecordID idRec,StringID nmAP)
{
	std::unordered_map<RecordID,LevelPathes *>::iterator it=_pathes.find(idRec);
	if (it==_pathes.end())
		return NULL;

	if (nmAP==StringID_Invalid)
		return (*it).second->defLoReso;
	else
	{
		LevelPathes *pathes=((*it).second);
		std::unordered_map<StringID,LevelLoResoPath*>::iterator it=pathes->pathesLoReso.find(nmAP);
		if (it!=pathes->pathesLoReso.end())
			return (*it).second;
	}
	return NULL;
}
