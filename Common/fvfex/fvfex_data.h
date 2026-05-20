#pragma once

struct FVFExData
{
	FVFExData(void){Clear();}

	int Add(const FVFEx &fvf){
		_fvfTotal |= fvf;
		int i = _fvfElments.size();
		_fvfElments.push_back(fvf);
		return i;
	}

	void Compute(void){
		_sz = fvfSize(_fvfTotal);
		size_t n = _fvfElments.size();
		_fvfSizes.resize(n);
		_fvfOffs.resize(n);
		for(int i = 0;i<n;i++){
			_fvfOffs[i] = fvfOffset(_fvfTotal,_fvfElments[i]);
			_fvfSizes[i] = fvfSize(_fvfElments[i]);
		}
	}

	void Clear(void)
	{
		_fvfElments.clear();
		_fvfOffs.clear();
		_fvfSizes.clear();
		_fvfTotal = 0;
		_ptr = NULL;
		_sz  = NULL;
	}

	FVFEx & GetFVF(){return _fvfTotal;}

	void * Get(int i) {
		assert(!_fvfOffs.empty());
		return _ptr + _fvfOffs[i];
	}

	void SetPtr(const void * ptr){_ptr = (BYTE *)ptr;}

	void Next(void)
	{
		assert(_ptr);
		_ptr += _sz;
	}

	void Set(int i,const void * data)
	{
		assert(i<_fvfOffs.size()&&data);
		void * p = Get(i);
		memcpy(p,data,_fvfSizes[i]);
	}

	void Set(int i ,const i_math::vector4df &v)
	{
		assert(_fvfSizes[i]==sizeof(v));
		Set(i,&v);
	}

	void Set(int i,const i_math::vector3df &v)
	{
		assert(_fvfSizes[i]==sizeof(v));
		Set(i,&v);
	}

	void Set(int i,const i_math::vector2df &v)
	{
		assert(_fvfSizes[i]==sizeof(v));
		Set(i,&v);
	}

	void Set(int i,DWORD v)
	{
		assert(_fvfSizes[i]==sizeof(v));
		Set(i,&v);
	}

private:
	std::vector<FVFEx> _fvfElments;
	std::vector<int> _fvfOffs;
	std::vector<int> _fvfSizes;
	FVFEx _fvfTotal;
	size_t _sz;
	BYTE * _ptr;
};

