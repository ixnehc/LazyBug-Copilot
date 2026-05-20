#pragma once

#pragma warning(disable:4018)
#pragma warning(disable:4267)

//std::vector<T>vec, T v
#ifndef UNIQUE_VEC_ADD
#define UNIQUE_VEC_ADD(vec,v)\
{\
	int _i;\
	for (_i=0;_i<(vec).size();_i++)\
		if ((vec)[_i]==(v))\
			break;\
	if (_i>=(vec).size())\
		(vec).push_back(v);\
}
#endif 

#define VEC_REMOVE(vec,v)\
{\
	int _i;\
	for (_i=0;_i<(vec).size();_i++)\
		if ((vec)[_i]==(v))\
		{\
			(vec).erase((vec).begin()+_i);\
			break;\
		}\
}

//删除一个数组成员,并把数组中最后最后一个成员移到被删除成员的位置上
#define VEC_REMOVE_SWAP(vec,v)																						\
{																																				\
	int _i;																																	\
	for (_i=0;_i<(vec).size();_i++)																								\
		if ((vec)[_i]==(v))																												\
		{																																		\
			(vec)[_i]=(vec)[(vec).size()-1];																						\
			(vec).pop_back();																											\
			break;																															\
		}																																		\
}

#define VEC_COMPARE(vec1,vec2,bSame)																	\
{																																		\
	bSame=FALSE;																											\
	if ((vec1).size()==(vec2).size())																					\
	{																																	\
		if((vec1).size()==0)																									\
			bSame=TRUE;																									\
		else																															\
		{																																\
			if (0==memcmp(&(vec1)[0],&(vec2)[0],sizeof((vec1)[0])*(vec1).size()))			\
				bSame=TRUE;																								\
		}																																\
	}																																	\
}

//std::vector<T>vec,std::vector<T>vec2
//merge the 2 vector and store the result in vec
#ifndef VEC_MERGE
#define VEC_MERGE(vec,vec2)\
	{\
	if ((vec2).size()>0)\
	{\
		DWORD sz=(vec).size();\
		(vec).resize(sz+(vec2).size());\
		memcpy(&(vec)[sz],&(vec2)[0],sizeof((vec2)[0])*(vec2).size());\
	}\
	}
#endif

#define VEC_ASCEND(vec,vectype)\
{\
	for (int _i=0;_i<(vec).size();_i++)\
	for (int _j=_i+1;_j<(vec).size();_j++)\
	{\
		if ((vec)[_i]>(vec)[_j])\
		{\
			vectype t=(vec)[_j];\
			(vec)[_j]=(vec)[_i];\
			(vec)[_i]=t;\
		}\
	}\
}

#define VEC_ASCEND_BY_ELEMENT(vec,vectype,elem)\
{\
	for (int _i=0;_i<(vec).size();_i++)\
	for (int _j=_i+1;_j<(vec).size();_j++)\
	{\
		if ((vec)[_i].elem>(vec)[_j].elem)\
		{\
			vectype t=(vec)[_j];\
			(vec)[_j]=(vec)[_i];\
			(vec)[_i]=t;\
		}\
	}\
}

#define DEQUE_REMOVE(dq,type,v)												\
{																										\
	std::deque<type>::iterator it=(dq).begin();								\
	while(it!=(dq).end())																	\
	{																									\
		if ((*it)==(v))																			\
		{																								\
			(dq).erase(it);																		\
			break;																					\
		}																								\
		it++;																						\
	}																									\
}


//std::vector<T>vec, T v,int idx
#ifndef VEC_FIND
#define VEC_FIND(vec,v,idx)\
{\
	(idx)=-1;\
	for (int __i=0;__i<(vec).size();__i++)\
		if ((vec)[__i]==(v))\
		{\
			(idx)=__i;\
			break;\
		}\
}
#endif 

#ifndef VEC_FIND_BY_ELEMENT
#define VEC_FIND_BY_ELEMENT(vec,elem,v,idx)																\
{																																			\
	(idx)=-1;																															\
	for (int __i=0;__i<(vec).size();__i++)																				\
		if ((vec)[__i].elem==(v))																								\
		{																																	\
			(idx)=__i;																													\
			break;																														\
		}																																	\
}
#endif 

#define PVEC_FIND_BY_ELEMENT(vec,elem,v,idx)															\
{																																			\
	(idx)=-1;																															\
	for (int __i=0;__i<(vec).size();__i++)																				\
		if ((vec)[__i]->elem==(v))																							\
		{																																	\
			(idx)=__i;																													\
			break;																														\
		}																																	\
}



#define VEC_SET(vec,v)				\
if (!vec.empty())				\
memset(&(vec)[0],v,(vec).size()*sizeof((vec)[0]))

#define VEC_APPEND(vec,vec2)																	\
{																														\
	if ((vec2).size()>0)																						\
	{																													\
		DWORD sz=(vec).size();																			\
		(vec).resize(sz+(vec2).size());																	\
		memcpy(&(vec)[sz],&(vec2)[0],(vec2).size()*sizeof((vec2)[0]));			\
	}																													\
}

//注意:vec和buf必须类型一样
#define VEC_APPEND_BUFFER(vec,buf,count)												\
{																														\
	if (buf)																											\
	{																													\
		DWORD sz=(vec).size();																			\
		(vec).resize(sz+(count));																			\
		memcpy(&(vec)[sz],buf,(count)*sizeof((buf)[0]));									\
	}																													\
}

//注意:vec和buf必须类型一样
#define VEC_SET_BUFFER(vec,buf,count)														\
{																														\
	(vec).clear();																								\
	VEC_APPEND_BUFFER(vec,buf,count)														\
}

#define VEC_EMPTY(vectype,vec)																	\
{																														\
	std::vector<vectype> t;																				\
	t.swap(vec);																									\
}

