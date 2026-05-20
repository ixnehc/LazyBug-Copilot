/********************************************************************
	created:	2006/8/3   16:15
	filename: 	e:\IxEngine\Common\resdata\AnimData.cpp
	author:		cxi
	
	purpose:	animation resource data:XFormData(transform anim),BoneData(bone transform anim)
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "AnimData.h"

#include "stringparser/stringparser.h"

#include "spline/CubicSpline.h"

#pragma warning (disable: 4018)

#pragma warning (disable: 4244)

void encodeQuat(const i_math::quatf & qSrc,i_math::vector4ds & qDst)
{
	qDst.x = (qSrc.array[0]*32767.0f);
	qDst.y = (qSrc.array[1]*32767.0f);
	qDst.z = (qSrc.array[2]*32767.0f);
	qDst.w = (qSrc.array[3]*32767.0f);
}

void decodeQuat(const i_math::vector4ds & qSrc,i_math::quatf & qDst)
{
	qDst.array[0] = qSrc.x/32767.0f;
	qDst.array[1] = qSrc.y/32767.0f;
	qDst.array[2] = qSrc.z/32767.0f;
	qDst.array[3] = qSrc.w/32767.0f;
}

void SortAnimEvent(std::vector<AnimEvent> &events)
{
	VEC_ASCEND_BY_ELEMENT(events,AnimEvent,tEvent);
}

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(BonesData2);

BonesData2::BonesData2(void)
{

}

BonesData2::~BonesData2(void)
{
}

void BonesData2::Save(CDataPacket &dp)
{
	dp.Data_NextByte() = GetVersion();

	DP_WriteVector(dp,skeleton);	
	dp.Data_NextDword() = animpieces.size();
	for(DWORD i = 0;i<animpieces.size();i++)
		animpieces[i].Save(dp);

	dp.Data_NextByte() = dataType; //数据类型
	switch(dataType){
	case 1:
		DP_WriteVector(dp,data1.bufPos);
		DP_WriteVector(dp,data1.bufScale);
		DP_WriteVector(dp,data1.bufRot);

		DP_WriteVector(dp,data1.ksPos);
		DP_WriteVector(dp,data1.ksScale);
		DP_WriteVector(dp,data1.ksRot);
		break;
	case 2:
		DP_WriteVector(dp,data2.keysPos);
		DP_WriteVector(dp,data2.keysScale);
		DP_WriteVector(dp,data2.keysRot);
		break;
	default:
		assert(false);
		break;
	}
}

void BonesData2::Load(CDataPacket &dp)
{
	DWORD ver = dp.Data_NextByte();
	DP_ReadVector(dp,skeleton);
	DWORD n = dp.Data_NextDword();
	animpieces.resize(n);
	for(DWORD i = 0;i<n;i++)
		animpieces[i].Load(dp,ver);

	dataType = dp.Data_NextByte();//数据类型
	switch(dataType){
	case 1:
		DP_ReadVector(dp,data1.bufPos);
		DP_ReadVector(dp,data1.bufScale);
		DP_ReadVector(dp,data1.bufRot);

		DP_ReadVector(dp,data1.ksPos);
		DP_ReadVector(dp,data1.ksScale);
		DP_ReadVector(dp,data1.ksRot);
		break;
	case 2:
		DP_ReadVector(dp,data2.keysPos);
		DP_ReadVector(dp,data2.keysScale);
		DP_ReadVector(dp,data2.keysRot);
		break;
	default:
		assert(false);
		break;
	}
}

void BonesData2::SaveHeader(CDataPacket &dp)
{

}

void BonesData2::LoadHeader(CDataPacket &dp)
{

}

void BonesData2::GetTickRange(AnimTick &tStart,AnimTick &tEnd)
{
	tStart = tEnd = 0;
	switch(dataType){
	case 1:
		if(!data1.ksRot.empty()){
			tStart = data1.ksRot[0].t;
			tEnd = data1.ksRot.back().t;
		}
	case 2:
		if(!data2.keysRot.empty()){
			tStart = data2.keysRot[0].t;
			tEnd = data2.keysRot.back().t;
		}
	default: break;
	}
}

//////////////////////////////////////////////////////////////////////////
//AnimData
void AnimData::GetAPTickRange(DWORD iAP,AnimTick &start,AnimTick &end)
{
	start=end=0;
	if (iAP>=animpieces.size())
		return;
	KeySet *keyset=GetKeySet();
	if (keyset->GetKeyCount()<=0)
		return;

	start=keyset->ClampTick(animpieces[iAP].tStart);
	end=keyset->ClampTick(animpieces[iAP].tEnd);
	if (start>end)
		start=end;
}

void AnimData::MakeDefaultAP()
{
	if (animpieces.size()<=0)
	{
		AnimPiece ap;
		animpieces.push_back(ap);
	}
}


void AnimData::Save(CDataPacket &dp)
{
	dp.Data_NextWord()=GetVer();//version
	dp.Data_NextWord()=(WORD)animpieces.size();
	for (int i=0;i<animpieces.size();i++)
		animpieces[i].Save(dp);
}

void AnimData::Load(CDataPacket &dp)
{
	WORD ver;
	Load(dp,ver);
}

void AnimData::Load(CDataPacket &dp,WORD &ver)
{
	ver=dp.Data_NextWord();

	WORD sz=dp.Data_NextWord();
	assert(sz<512);
	animpieces.resize(sz);
	for (int i=0;i<animpieces.size();i++)
		animpieces[i].Load(dp,ver);
}




//////////////////////////////////////////////////////////////////////////
//XFormData
IMPLEMENT_CLASS(XFormData);
void XFormData::Clean()
{
	keyset.Clean();
	cps.clear();
}

ResType XFormData::GetType()
{
	return ResA_XForm;
}
const char *XFormData::GetTypeName()
{
	return "XformAnim";
}

void XFormData::CalcContent(std::string &s)
{
	float tTotal=((float)keyset.GetEndTick())/(float)ANIMTICK_PER_SECOND;
	FormatString(s,"Tranform Animation data: \n%d 个keys,\n总长度为%.3f,\n总时间为%.3f秒,\n内存 %d 字节",
		keyset.GetKeyCount(),distTotal,tTotal,keyset.GetKeyCount()*keyset.GetKeySize());
}

void XFormData::Save(CDataPacket &dp)
{
	TAnimData<KT_XForm>::Save(dp);

	dp.Data_NextFloat()=distTotal;

	DP_WriteVector(dp,cps);
	dp.Data_NextDword()=flagsCP;
}

void XFormData::Load(CDataPacket &dp)
{
	TAnimData<KT_XForm>::Load(dp);

	distTotal=dp.Data_NextFloat();

	DP_ReadVector(dp,cps);
	flagsCP=dp.Data_NextDword();
}

//把一个旋转对齐到一个方向上,dir必须是normalize过的
void AlignQuat(i_math::quatf &q,i_math::vector3df &dir)
{
	i_math::vector3df za(0,0,1);
	za=q*za;

	//构建一个quaternion把za转到vel
	i_math::quatf qOff;
	qOff.from2Vector(za,dir);

	q=q*qOff;
	q.normalize();

}



void XFormData::CalcFromCPs(CtrlPointSampleInfo *info)
{
	if (cps.size()<=0)
	{//甭算了
		keyset.SetKeyCount(0);
		return;
	}

	std::vector<CtrlPoint> cpsW;//W 代表working

	if (TRUE)//根据cps,构建cpsW,主要是去掉位置重复的控制点
	{
		int c=0;
		cpsW.resize(cps.size());
		for(int i=0;i<cps.size();i++)
		{
			if (i>1)
			{
				if (cps[i].xfm.pos.equals(cps[i-1].xfm.pos))
					continue;//略过和前一个点重复的点
			}
			cpsW[c]=cps[i];
			c++;
		}
		cpsW.resize(c);

		if (bCircular)
		{//如果circular的话,最后一个点要和第一个点比较一下,看是否重复
			if(cpsW.size()>1)
			{
				if (cpsW[cpsW.size()-1].xfm.pos.equals(cpsW[0].xfm.pos))
					cpsW.pop_back();
			}
		}
	}

	BOOL bCircularW=bCircular;
	if (cpsW.size()<=1)//控制点少于等于一个的话,就不要搞回路了
		bCircularW=FALSE;

	//先根据ControlPoints构建一根曲线
	CCubicSpline spln(bCircularW);

	for (int i=0;i<cpsW.size();i++)
		spln.AddNode(cpsW[i].xfm.pos,cpsW[i].xfm.rot);

	spln.BuildSNS();
	int nodeCount=spln.GetNodeCount();

	//circular的话, 把头上的拷到尾巴上
	if (bCircularW)
		cpsW.push_back(cpsW[0]);
	assert(cpsW.size()==nodeCount);

	//如果某个node是velocitiy aligned的,我们要把它的旋转对齐到速度方向上
	for (int i=0;i<nodeCount;i++)
	{
		CCubicSpline::Node *node=spln.GetNode(i);
		i_math::vector3df vel=node->velocity;
		vel.normalize();

		if (cpsW[i].bVelocityAligned)
			AlignQuat(cpsW[i].xfm.rot,vel);
		spln.SetNodeRot(i,cpsW[i].xfm.rot);

	}

	spln.BuildRot();


	std::vector<CCubicSpline::Sample>samples;


	float distGap=0.2f;

	samples.resize((int)(spln.GetDistance()/distGap)+20);

	DWORD nSample=spln.GetSamples(distGap,samples.data());

	keyset.SetKeyCount(nSample);
	if (TRUE)
	{
		//先计算各个sample和控制点的曲线距离(所谓曲线距离是指它们与曲线起始点之间沿着曲线上的距离)
		std::vector<float>nodedists;
		std::vector<float>nodetimes;
		std::vector<float>nodeaccs;
		std::vector<float>sampledists;
		sampledists.resize(nSample);
		nodedists.resize(nodeCount);
		nodeaccs.resize(nodeCount);
		nodetimes.resize(nodeCount);
		i_math::vector3df *posCur=&cpsW[0].xfm.pos;
		nodedists[0]=0.0f;
		int iCurNode=0;
		float distCur=0.0f;

		for (int i=0;i<nSample;i++)
		{
			CCubicSpline::Sample *sample=&samples[i];
			int iNode=sample->iNode;

			if (iNode>iCurNode)
			{
				iCurNode=iNode;
				distCur+=(float)(cpsW[iNode].xfm.pos-*posCur).getLength();
				nodedists[iNode]=distCur;
				posCur=&cpsW[iNode].xfm.pos;
			}
			distCur+=(float)(sample->pos-*posCur).getLength();
			sampledists[i]=distCur;
			posCur=&sample->pos;
		}

		//计算每个控制点开始的这一段里的加速度,以及每个控制点的起始时间
		float tCur=0.0f;
		for (int i=0;i<nodeCount-1;i++)
		{
			//t=2*s/(v1+v0);
			float t=2*(nodedists[i+1]-nodedists[i])/(cpsW[i].vel+cpsW[i+1].vel);//这一段走完的时间

			nodetimes[i]=tCur;
			nodeaccs[i]=(cpsW[i+1].vel-cpsW[i].vel)/t;//加速度

			tCur+=t;
		}
		nodetimes[nodeCount-1]=tCur;

		//计算每个sample的时间
		for (int i=0;i<nSample-1;i++)
		{
			Key_xform *key=(Key_xform *)keyset.GetKey(i);

			CCubicSpline::Sample *sample=&samples[i];
			int iNode=sample->iNode;

			float s=sampledists[i]-nodedists[iNode];
			float v=cpsW[iNode].vel;
			float a=nodeaccs[iNode];
			float t;
			if (a==0)
				t=nodetimes[iNode]+s/v;
			else
				t=nodetimes[iNode]+(-v+sqrtf(v*v+2*a*s))/a;

			key->t=ANIMTICK_FROM_SECOND(t);
		}
		keyset.GetKey(nSample-1)->t=ANIMTICK_FROM_SECOND(nodetimes[nodeCount-1]);

		if (info)
		{
			info->cptimes.swap(nodetimes);
			info->tTotal=tCur;
			info->distTotal=distCur;
		}

		distTotal=distCur;
	}


	if (bCircularW)
		cpsW.pop_back();

	for (int i=0;i<nSample;i++)
	{
		Key_xform *key=(Key_xform *)keyset.GetKey(i);

		key->v.pos = samples[i].pos;
		key->v.rot = samples[i].rot;
		key->v.scale_=1;

	}

	Compress();
}

void XFormData::Compress()
{
	if (keyset.GetKeyCount()<3)
		return;

	Key_xform *kEnd=(Key_xform *)keyset.GetKey(keyset.GetKeyCount()-1);
	Key_xform *k1,*k2,*k3;
	k1=(Key_xform *)keyset.GetKey(0);
	k2=(Key_xform *)keyset.GetKey(1);
	k3=(Key_xform *)keyset.GetKey(2);

	DWORD nKey=0;

	i_math::xformf xfm;
	while(k3<=kEnd)
	{
		BOOL bCanDisard=FALSE;

		//根据k1,k3判断k2是不是可以discard
		if (k3->t-k1->t>0)
		{
			float r=((float)(k2->t-k1->t))/(float)(k3->t-k1->t);

			xfm.pos=k3->v.pos.getInterpolated(k1->v.pos,r);
			if (xfm.pos.equals(k2->v.pos,0.002f))
			{
				xfm.scale_=i_math::lerp(k1->v.scale_,k3->v.scale_,r);
				if (i_math::equals(xfm.scale_,k2->v.scale_,0.01f))
				{
					xfm.rot.slerp(k1->v.rot,k3->v.rot,r);
					xfm.rot.makeInverse();
					xfm.rot*=k2->v.rot;
					if (i_math::equals(xfm.rot.W,1.0f,0.00000002f))//0.002f
						bCanDisard=TRUE;
				}
			}
		}

		if (!bCanDisard)
		{
			keyset.SetKey(nKey,*k1);
			nKey++;
			k1=k2;
			k2=k3;
			k3++;

		}
		else
		{
			k2=k3;
			k3++;
		}
	}

	keyset.SetKey(nKey++,*k1);
	keyset.SetKey(nKey++,*k2);

	keyset.SetKeyCount(nKey);
}


void XFormData::Compress2()
{
	KeySet ksTemp;
	ksTemp.CopyFrom(keyset);
	ksTemp.SetKeyCount(0);

	for (int i=0;i<animpieces.size();i++)
	{
		AnimPiece *ap=&animpieces[i];

		if (ap->iEnd-ap->iStart<3)
		{
			int iStart=ksTemp.GetKeyCount();
			for (int j=ap->iStart;j<=ap->iEnd;j++)
				ksTemp.AddKey(*keyset.GetKey(j));
			ap->iStart=iStart;
			ap->iEnd=ksTemp.GetKeyCount();
		}
		else
		{
			int iStart=ksTemp.GetKeyCount();

			Key_xform *kEnd=(Key_xform *)keyset.GetKey(ap->iEnd-1);
			Key_xform *k1,*k2,*k3;
			k1=(Key_xform *)keyset.GetKey(ap->iStart);
			k2=(Key_xform *)keyset.GetKey(ap->iStart+1);
			k3=(Key_xform *)keyset.GetKey(ap->iStart+2);

			DWORD nKey=0;

			i_math::xformf xfm;
			while(k3<=kEnd)
			{
				BOOL bCanDisard=FALSE;

				//根据k1,k3判断k2是不是可以discard
				if (k3->t-k1->t>0)
				{
					float r=((float)(k2->t-k1->t))/(float)(k3->t-k1->t);

					xfm.pos=k3->v.pos.getInterpolated(k1->v.pos,r);
					if (xfm.pos.equals(k2->v.pos,0.002f))
					{
						xfm.scale_=i_math::lerp(k1->v.scale_,k3->v.scale_,r);
						if (i_math::equals(xfm.scale_,k2->v.scale_,0.01f))
						{
							xfm.rot.slerp(k1->v.rot,k3->v.rot,r);
							xfm.rot.makeInverse();
							xfm.rot*=k2->v.rot;
							if (i_math::equals(xfm.rot.W,1.0f,0.00002f))//0.002f
								bCanDisard=TRUE;
						}
					}
				}

				if (!bCanDisard)
				{
					ksTemp.AddKey(*k1);
					k1=k2;
					k2=k3;
					k3++;

				}
				else
				{
					k2=k3;
					k3++;
				}
			}

			ksTemp.AddKey(*k1);
			ksTemp.AddKey(*k2);

			ap->iStart=iStart;
			ap->iEnd=ksTemp.GetKeyCount();
		}

	}

	keyset.CopyFrom(ksTemp);
}


//////////////////////////////////////////////////////////////////////////
//MtrlColorData
IMPLEMENT_CLASS(MtrlColorData);
MtrlColorData::MtrlColorData()
{
	bDiffuse = FALSE;
	bOpacity = FALSE;
}
ResType MtrlColorData::GetType()
{
	return ResA_MtrlColor;
}

const char *MtrlColorData::GetTypeName()
{
	return "MtrlColor";
}

void MtrlColorData::CalcContent(std::string &s)
{
	s="<Not Available yet>";
}
void MtrlColorData::Load(CDataPacket &dp)
{
	DWORD ver = dp.Data_NextDword();
	
	TAnimData<KT_Color>::Load(dp); 

	bDiffuse = dp.Data_NextDword();
	bOpacity = dp.Data_NextDword();
}
void MtrlColorData::Save(CDataPacket &dp)
{
	dp.Data_NextDword() = GetVersion();
	
	TAnimData<KT_Color>::Save(dp);
	
	dp.Data_NextDword() = bDiffuse;
	dp.Data_NextDword() = bOpacity;
}
//////////////////////////////////////////////////////////////////////////
//MapCoordData
IMPLEMENT_CLASS(MapCoordData);

ResType MapCoordData::GetType()
{
	return ResA_MapCoord;
}

const char *MapCoordData::GetTypeName()
{
	return "MapCoord";
}

void MapCoordData::CalcContent(std::string &s)
{
	s="<Not Available yet>";
}

void MapCoordData::Load(CDataPacket &dp)
{
	DWORD ver = dp.Data_NextDword();
	TAnimData<KT_MapCoord>::Load(dp);
}

void MapCoordData::Save(CDataPacket &dp)
{
	dp.Data_NextDword() = GetVersion();
	TAnimData<KT_MapCoord>::Save(dp);
}

//////////////////////////////////////////////////////////////////////////





