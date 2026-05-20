#include "matrix43.h"

#include "matrix44.h"

namespace i_math
{

	template<class T>
	void matrix43<T>::set(const matrix44<T> &source)
	{
		set(source.m00, source.m01, source.m02,
		source.m10, source.m11, source.m12,
		source.m20, source.m21, source.m22,
		source.m30, source.m31, source.m32);
	}



}