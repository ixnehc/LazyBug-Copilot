#pragma once

#include <vector>
#include "../math/iMath.h"
#include "../math/vector2d.h"
#include "../math/vector3d.h"
#include "../math/quaternion.h"
#include "../math/xform.h"
#include "../math/matrix43.h"

#include "animbase.h"

#include "datapacket/DataPacket.h"
#include <assert.h>

struct Key
{
	Key()	{		t=0;}
	AnimTick t;
};

struct KeySetHead
{
	KeySetHead()
	{
		memset(this,0,sizeof(*this));
	}
	KeySetHead(WORD tp,WORD sz)
	{
		keycount=0;
		type=tp;
		szKey=sz;
	}
	DWORD keycount;
	WORD szKey;
	WORD type;
};

struct KeySet
{
public:
	KeyType GetKeyType()	{		return (KeyType)_hd.type;	}

	BOOL IsEmpty()
	{
		if (_buf.size()<=0)
			return TRUE;
		return FALSE;
	}
	int GetKeySize()	{		return _hd.szKey;	}

	int GetKeyCount()		{				return (int)_hd.keycount;	}
	void SetKeyCount(int nKey)
	{
		_hd.keycount=nKey;
		_BuildBuffer();
	}
	Key* GetKey(DWORD iKey)//return NULL if fail
	{
		if (iKey<_hd.keycount)
			return (Key*)&_buf[iKey*_hd.szKey];
		return NULL;
	}
	void SetKey(DWORD iKey,const Key &key)
	{
		if (iKey<_hd.keycount)
			memcpy(&_buf[iKey*_hd.szKey],&key,_hd.szKey);
	}

	void InsertKey(DWORD iKey,const Key &key)
	{
		_buf.insert(_buf.begin()+iKey*_hd.szKey,_hd.szKey,0);
		memcpy(&_buf[iKey*_hd.szKey],&key,_hd.szKey);
		_hd.keycount++;
	}

	void AddKey(const Key &key)
	{
		InsertKey(GetKeyCount(),key);
	}

	//在末尾添加一个key,并返回它的指针,注意,新添加的Key的内容完全没有定义
	Key *NewKey()
	{
		int sz=_buf.size();
		_buf.resize(_buf.size()+_hd.szKey);
		_hd.keycount++;
		return (Key*)&_buf[sz];
	}

	void RemoveKey(DWORD iKey)
	{
		if (iKey<_hd.keycount)
		{
			_buf.erase(_buf.begin()+iKey*_hd.szKey,_buf.begin()+iKey*_hd.szKey+_hd.szKey);
			if (_hd.keycount>0)
				_hd.keycount--;
		}
	}

	//return a key index clamped to this keyset's range
	//if the keyset frame is empty,return 0xffffffff
	DWORD ClampKeyIdx(DWORD iKey)
	{
		if (iKey>=_hd.keycount)
			iKey=_hd.keycount-1; 
		return iKey;
	}

	Key* GetClampKey(DWORD iKey)//clamp the key index before get it. return NULL if fail
	{
		iKey=ClampKeyIdx(iKey);
		if (iKey==0xffffffff)
			return NULL;
		return GetKey(iKey);
	}
	AnimTick ClampTick(AnimTick t)//return a AnimTick clamped to this keyset's anim tick range
	{
		if (_hd.keycount<=0)
			return 0;

		Key *kStart,*kEnd;
		kStart=(Key *)_buf.data();
		kEnd=(Key *)&_buf[(_hd.keycount-1)*_hd.szKey];
		if (t<kStart->t)
			t=kStart->t;
		if (t>kEnd->t)
			t=kEnd->t;
		return t;
	}

	//set all the key's anim tick to an offset to the first key
	void ZeroOffset()
	{
		if (_hd.keycount<=0)
			return;
		BYTE *p=_buf.data();
		AnimTick t=((Key*)p)->t;
		for (int i=0;i<_hd.keycount;i++)
		{
			((Key*)p)->t-=t;
			p+=_hd.szKey;
		}
	}
	//reverse the key sequence
	void Reverse()
	{
		if (_hd.keycount<=0)
			return;

		//first reverse tick
		BYTE *p=_buf.data();
		AnimTick t1,t2;
		t1=((Key*)p)->t;
		t2=((Key*)(p+(_hd.keycount-1)*_hd.szKey))->t;
		for (int i=0;i<_hd.keycount;i++)
		{
			((Key*)p)->t=t2+t1-((Key*)p)->t;
			p+=_hd.szKey;
		}

		//now reverse the key

		std::vector<BYTE>temp;
		temp.resize(_hd.szKey);
		p=_buf.data();
		BYTE *q=p+(_hd.keycount-1)*_hd.szKey;

		for (int i=0;i<_hd.keycount/2;i++)
		{
			memcpy(temp.data(),p,_hd.szKey);
			memcpy(p,q,_hd.szKey);
			memcpy(q,temp.data(),_hd.szKey);
			p+=_hd.szKey;
			q-=_hd.szKey;
		}
	}
	AnimTick GetStartTick()
	{
		if (_hd.keycount<=0)
			return 0;
		return ((Key*)_buf.data())->t;
	}
	AnimTick GetEndTick()
	{
		if (_hd.keycount<=0)
			return 0;
		return ((Key*)&_buf[(_hd.keycount-1)*_hd.szKey])->t;
	}

	AnimTick GetRange()
	{
		if (_hd.keycount<=0)
			return 0;
		return ((Key*)&_buf[(_hd.keycount-1)*_hd.szKey])->t-((Key*)_buf.data())->t+1;
	}

#define FindKeyRet(k1,k2)															\
	r=(float)(t-k1->t)/(float)(k2->t-k1->t);									\
	if (t==k1->t)																			\
		iKey2=0xffffffff;																	\
	return TRUE;

	BOOL FindKeysInRange(AnimTick t,DWORD &iKey1,DWORD &iKey2,float &r)
	{
		if (_hd.keycount<=0)
			return FALSE;
		DWORD s,e;
		s=iKey1;
		e=iKey2;
		if (s>=_hd.keycount)
			s=_hd.keycount-1;
		if (e>=_hd.keycount)
			e=_hd.keycount-1;

		assert(s<=e);

		BYTE *p=_buf.data();

		Key *k1,*k2;
		k1=k2=NULL;
		iKey1=iKey2=0xffffffff;
		r=1.0f;

		iKey1=s;
		k1=(Key*)&p[s*_hd.szKey];
		if (t<=k1->t)
			return TRUE;
		k2=(Key*)&p[e*_hd.szKey];
		if (t>=k2->t)
		{
			iKey1=e;
			return TRUE;
		}

		if (s==e)
			return TRUE;
		while(s+1<e)
		{
			int mid=(s+e)/2;
			if (t<((Key*)&p[mid*_hd.szKey])->t)
				e=mid;
			else
				s=mid;
		}

		iKey1=s;
		iKey2=e;
		k1=(Key*)&p[s*_hd.szKey];
		k2=(Key*)&p[e*_hd.szKey];

		FindKeyRet(k1,k2);
	}

	//Find the keys neighboured to t,if there exists exactly a key at tick t,return this key in first 
	//value,and the second value is filled an invalid value(NULL or 0xffffffff).
	BOOL FindKeys(AnimTick t,DWORD &iKey1,DWORD &iKey2,float &r)//if iKey2 is 0xffffffff, r has no meaning
	{
		iKey1=0;iKey2=0xffffffff;
		return FindKeysInRange(t,iKey1,iKey2,r);
	}
	BOOL FindKeys(AnimTick t,Key *&k1,Key *&k2,float &r)//if k2 is NULL, r has no meaning
	{
		DWORD iKey1,iKey2;
		if (FALSE==FindKeys(t,iKey1,iKey2,r))
			return FALSE;

		BYTE *p=_buf.data();

		k1=(Key*)&p[iKey1*_hd.szKey];
		if (iKey2!=0xffffffff)
			k2=(Key*)&p[iKey2*_hd.szKey];
		else
			k2=NULL;
		return TRUE;
	}

	template<typename T_Key>
	BOOL CalcKey(AnimTick t,T_Key*key)
	{
		Key *k1,*k2;
		float r;
		if (FALSE==FindKeys(t,k1,k2,r))
			return FALSE;

		if (!k2)
			memcpy(key,k1,_hd.szKey);
		else
			return key->Lerp(k1,k2,r);

		return TRUE;
	}

	
	BOOL CheckTickAcsend()//Check whether the keys' ticks are ascending
	{
		BYTE *p=_buf.data();
		BYTE *q=p+_hd.szKey;

		for (int i=0;i<_hd.keycount-1;i++)
		{
			if (((Key*)p)->t>((Key*)q)->t)
				return FALSE;
			p=q;
			q+=_hd.szKey;
		}
		return TRUE;
	}


	virtual void Clean()
	{
		_buf.clear();
		_hd.keycount=0;
	}
	virtual BOOL CopyFrom(KeySet&src)
	{
		_hd=src._hd;
		_buf=src._buf;
		return TRUE;
	}
	virtual void Save(CDataPacket &dp)
	{
		DP_WriteVar(dp,_hd);
		DP_WriteVector(dp,_buf);
	}
	virtual void Load_(CDataPacket &dp)
	{
		DP_ReadVar(dp,_hd);
		DP_ReadVector(dp,_buf);
	}
	virtual void LoadOld(CDataPacket &dp)
	{
		_hd.type=dp.Data_NextWord();
		_hd.szKey=dp.Data_NextWord();
		DP_ReadVector(dp,_buf);
		_hd.keycount=dp.Data_NextDword();
	}

	//清除所有的大于或者等于这个时间的Key
	void DiscardKeysAfter(AnimTick t)
	{
		if (t>GetEndTick())
			return;
		DWORD iKey1,iKey2;
		float r;
		if (!FindKeys(t,iKey1,iKey2,r))
			return;
		if (iKey2==0xffffffff)
			iKey2=iKey1;

		//删除iKey2以及它之后的所有Keys
		SetKeyCount(iKey2);
	}

	//从keyset的头部删除尽量多的Key,但保证在t时刻的取值不受影响
	void DiscardInvalidKeysBefore(AnimTick t)
	{
		DWORD iKey1,iKey2;
		float r;
		if (FindKeys(t,iKey1,iKey2,r))
			DiscardHeadKeys(iKey1);
	}

	//从头开始删除若干个Keys
	void DiscardHeadKeys(DWORD nKey)
	{
		if (nKey>=GetKeyCount())
		{
			Clean();
			return;
		}

		DWORD c=GetKeyCount()-nKey;
		Key *k=GetKey(nKey);
		Key *k0=GetKey(0);
		memmove((void*)k0,(void*)k,c*_hd.szKey);
		SetKeyCount(c);
	}

protected:

friend void KeySet_Define(KeySet *,KeyType ,DWORD );
friend struct KeySetInfo;

	void _BuildBuffer()
	{
		_buf.resize(_hd.keycount*_hd.szKey);
	}
	void _Define(KeyType type,DWORD szKey)
	{
		_hd.szKey=(WORD)szKey;
		_hd.type=(WORD)type;
	}
	KeySetHead _hd;
	std::vector<BYTE> _buf;
};

struct KeySetInfo
{
	KeySet *keyset;
	DWORD iStartKey,iEndKey;
	void Build(KeySet *ks,AnimTick tStart,AnimTick tEnd)
	{
		keyset=ks;
		DWORD k1,k2;
		float r;
		ks->FindKeys(tStart,k1,k2,r);
		iStartKey=k1;
		ks->FindKeys(tEnd,k1,k2,r);
		if (k2==0xffffffff)
			iEndKey=k1;
		else
			iEndKey=k2;
	}
	BOOL FindKeys(AnimTick t,Key*&k1,Key *&k2,float &r)
	{
		DWORD iKey1=iStartKey,iKey2=iEndKey;
		if (FALSE==keyset->FindKeysInRange(t,iKey1,iKey2,r))
			return FALSE;
		k1=keyset->GetKey(iKey1);
		if (iKey2==0xffffffff)
			k2=NULL;
		else
			k2=keyset->GetKey(iKey2);

		return TRUE;
	}

	BOOL FindKeys(AnimTick t,DWORD&iKey1,DWORD&iKey2,float &r)
	{
		iKey1=iStartKey;
		iKey2=iEndKey;
		if (FALSE==keyset->FindKeysInRange(t,iKey1,iKey2,r))
			return FALSE;

		return TRUE;
	}

	template<typename T_Key>
	BOOL CalcKey(AnimTick t,T_Key*key)
	{
		Key *k1,*k2;
		float r;
		if (FALSE==FindKeys(t,k1,k2,r))
			return FALSE;

		if (!k2)
			memcpy(key,k1,keyset->_hd.szKey);
		else
			return key->Lerp(k1,k2,r);

		return TRUE;
	}


};


struct Key_f:public Key
{
	i_math::f32 v;
	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		v=i_math::lerp(((Key_f*)k1)->v,((Key_f*)k2)->v,r);
		return TRUE;
	}
};

struct Key_2f:public Key
{
	i_math::vector2df v;
	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		v=((Key_2f*)k2)->v.getInterpolated(((Key_2f*)k1)->v,r);
		return TRUE;
	}
};

struct Key_pos:public Key
{
	i_math::vector3df v;
	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		v=((Key_pos*)k2)->v.getInterpolated(((Key_pos*)k1)->v,r);
		return TRUE;
	}
};

struct Key_i:public Key
{
	i_math::s32 v;
	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		v=(i_math::s32)i_math::lerp((float)(((Key_i*)k1)->v),(float)(((Key_f*)k2)->v),r);
		return TRUE;
	}
};

struct Key_quat:public Key
{
	i_math::quatf v;
	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		v.slerp(((Key_quat*)k1)->v,((Key_quat*)k2)->v,r);
		return TRUE;
	}
};
struct Key_col:public Key
{
	union
	{
		DWORD color;
		BYTE  c[4];
	};

	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		for (int i=0;i<4;i++)
			c[i]=i_math::clamp_i((int)i_math::lerp((float)(((Key_col*)k1)->c[i]),(float)(((Key_col*)k2)->c[i]),r),0,255);
		return TRUE;
	}

};
struct Key_xform:public Key
{
	i_math::xformf v;
	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		v=((Key_xform*)k2)->v.getInterpolated(((Key_xform*)k1)->v,r);
		return TRUE;
	}
};



struct Key_mapcoord:public Key
{
	i_math::vector2df uvOff;
	i_math::vector2df uvTiling;
	i_math::vector3df uvRot;

	BOOL Lerp(Key *k1,Key *k2,float r)
	{
		uvOff=((Key_mapcoord*)k2)->uvOff.getInterpolated(((Key_mapcoord*)k1)->uvOff,r);
		uvTiling=((Key_mapcoord*)k2)->uvTiling.getInterpolated(((Key_mapcoord*)k1)->uvTiling,r);
		uvRot=((Key_mapcoord*)k2)->uvRot.getInterpolated(((Key_mapcoord*)k1)->uvRot,r);
		return TRUE;
	}

	void MakeMat(i_math::matrix43f &mat)
	{
		mat.setTranslation(uvOff.x,uvOff.y,0.0f);
		mat.addRotationXYZ(uvRot);
		mat.addScale(uvTiling.x,uvTiling.y,1.0f);
	}

};

struct Key_ref:public Key
{
	DWORD idx;
};


struct Key_s4:public Key
{
	i_math::vector4ds v;
};



//XXXXX:More KeyType

inline BOOL KeySet_LerpKey(KeySet *keyset,Key *k,Key *k1,Key *k2,float rate)
{
	if (!k2)
		memcpy(k,k1,keyset->GetKeySize());
	else
	{
		switch(keyset->GetKeyType())
		{
		case KT_Float:
			((Key_f*)k)->Lerp(((Key_f*)k1),((Key_f*)k2),rate);
			return TRUE;
		case KT_Int:
			((Key_i*)k)->Lerp(((Key_i*)k1),((Key_i*)k2),rate);
			return TRUE;
		case KT_Floatx2:
			((Key_2f*)k)->Lerp(((Key_2f*)k1),((Key_2f*)k2),rate);
			return TRUE;
		case KT_Pos:
			((Key_pos*)k)->Lerp(((Key_pos*)k1),((Key_pos*)k2),rate);
			return TRUE;
		case KT_Quat:
			((Key_quat*)k)->Lerp(((Key_quat*)k1),((Key_quat*)k2),rate);
			return TRUE;
		case KT_XForm:
			((Key_xform*)k)->Lerp(((Key_xform*)k1),((Key_xform*)k2),rate);
			return TRUE;
		case KT_Color:
			((Key_col*)k)->Lerp(((Key_col*)k1),((Key_col*)k2),rate);
			return TRUE;
		case KT_MapCoord:
			((Key_mapcoord*)k)->Lerp(((Key_mapcoord*)k1),((Key_mapcoord*)k2),rate);
			return TRUE;
		}
	}
	return FALSE;
}

inline BOOL KeySet_CalcKey(KeySet *keyset,Key *k,AnimTick t)
{
	Key *k1,*k2;
	float rate;
	keyset->FindKeys(t,k1,k2,rate);
	return KeySet_LerpKey(keyset,k,k1,k2,rate);
}

//在[start,end)范围内Find
inline BOOL KeySet_CalcKey(KeySet *keyset,Key *k,DWORD start,DWORD end,AnimTick t)
{
	end--;
	float rate;
	if (FALSE==keyset->FindKeysInRange(t,start,end,rate))
		return FALSE;

	Key *k1,*k2;
	k1=(Key*)keyset->GetKey(start);
	if (end!=0xffffffff)
		k2=(Key*)keyset->GetKey(end);
	else
		k2=NULL;
	return KeySet_LerpKey(keyset,k,k1,k2,rate);
}


inline BOOL KeySet_CalcAngleKey(KeySet *keyset,Key_f *k,AnimTick t)
{
	Key_f *k1,*k2;
	float r;
	if(keyset->FindKeys(t,(Key*&)k1,(Key*&)k2,r))
	{
		if (!k2)
			k->v=k1->v;
		else
		{
			k->v=k1->v;
			float gap=i_math::get_radian_dist(k1->v,k2->v);
			i_math::rotate_limited(k->v,k2->v,gap*r);
		}
		return TRUE;
	}
	return FALSE;
}




extern void KeySet_Define(KeySet *keyset,KeyType type,DWORD szKey=0);//if szKey is 0,automatically decide key size
extern void KeySet_Delete(KeySet *keyset);
extern KeySet *KeySet_New(KeyType type);
extern KeySet *KeySet_Clone(KeySet *src);
