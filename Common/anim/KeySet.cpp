#include "stdh.h"

#include "KeySet.h"

#include "../mempool/mempool.h"


#include <assert.h>

CMemPool<KeySet>*KeySet_GetPool()
{
	static CMemPool<KeySet>pool("KeySet Pool");
	return &pool;
}


//if szKey is 0,automatically decide key size
void KeySet_Define(KeySet *keyset,KeyType type,DWORD szKey)
{
	if (szKey==0)
	{
		switch(type)
		{
		case KT_Float:
			szKey=sizeof(Key_f);break;
		case KT_Int:
			szKey=sizeof(Key_i);break;
		case KT_Floatx2:
			szKey=sizeof(Key_2f);break;
		case KT_Pos:
			szKey=sizeof(Key_pos);break;
		case KT_Quat:
			szKey=sizeof(Key_quat);break;
		case KT_XForm:
			szKey=sizeof(Key_xform);break;
		case KT_Color:
			szKey=sizeof(Key_col);break;
		case KT_MapCoord:
			szKey=sizeof(Key_mapcoord);break;
		case KT_Ref:
			szKey=sizeof(Key_ref);break;
		case KT_Shortx4:
			szKey=sizeof(Key_s4);break;
		default:
			szKey=sizeof(Key);
		}
	}

	keyset->_Define(type,szKey);
}


void KeySet_Delete(KeySet *keyset)
{
	if (!keyset)
		return;
	keyset->Clean();
	KeySet_GetPool()->Free(keyset);
}

KeySet *KeySet_New(KeyType type)
{
	KeySet *ret=KeySet_GetPool()->Alloc();
	KeySet_Define(ret,type,0);
	return ret;
}

KeySet *KeySet_Clone(KeySet *src)
{
	if (!src)
		return NULL;
	KeySet *keyset=KeySet_GetPool()->Alloc();
	keyset->CopyFrom(*src);
	return keyset;
}

