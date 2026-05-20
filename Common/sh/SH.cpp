
#include "stdh.h"

#include "SH.h"

#include "assert.h"

const float pi = 3.1415926535f;

CSH::CSH(void)
{
	_InitFactorial();
}

CSH::~CSH(void)
{

}

void CSH::_InitFactorial()
{
	_factorials.resize(4*MAX_BANDSNUM+1);
	_factorials[0] = 1.0f;
	float f = 1.0f;
	for(size_t i = 1;i<_factorials.size();i++){
		f = f*i;	
		_factorials[i] = f;
	}	
}

float CSH::Factorial(int n) const // n!
{
	assert(size_t(n)<_factorials.size());
	return _factorials[n];
}

//Scale factor
float CSH::K(int l,int m) const	// K = sqrtf{ [(2*l+1)*((l-|m|)!)]/ [4*pi*((l+|m|)!)] }
{
	float temp = ((2*l+1.0f)*Factorial(l-m))/(4.0f*pi*Factorial(l+m));
	return sqrtf(temp);
}

//evaluate an associate legendre polynomial p(l,m,x) at x
float CSH::P(int l,int m,float x)  const
{
	float pmm = 1.0f;

	if(m>0){
		float somx2 = sqrtf((1.0f-x)*(1.0f+x));
		float fact = 1.0f;
		for(int i = 1;i<=m;i++){
			pmm = pmm*(-fact)*somx2;
			fact += 2.0f;
		}
	}

	if(m==l) return pmm;

	float pmmpl = x*(2.0f*m+1.0f)*pmm;
	if(l==m+1)		return pmmpl;

	float pll = 0.0f;
	for(int ll = m+2;ll<=l;++ll){
		pll = ((2.0f*ll-1.0f)*x*pmmpl-(ll+m-1.0f)*pmm ) /(ll - m);
		pmm = pmmpl;
		pmmpl = pll;
	}

	return pll;
}

float CSH::Y(int l,int m,float theta,float phi) const
{
	// return a point sample of a Spherical Harmonic basis function
	// l is the band ,range [0,N]
	// m in the range [-l,l]
	// theta in the range [0..pi]
	// phi in the range [0...2*pi]

	const float sqrt2 = sqrtf(2.0f);
	if(m==0) 
		return K(l,0)*P(l,m,cosf(theta));
	else if(m>0)
		return sqrt2*K(l,m)*cosf(m*phi)*P(l,m,cosf(theta));
	else
		return sqrt2*K(l,-m)*sinf(-m*phi)*P(l,-m,cosf(theta));
}

void CSH::Setup_Spherical_Samples(int sqrt_n_samples,int n_bands)
{
	_samples.resize(sqrt_n_samples*sqrt_n_samples);	//采样点的个数

	float oneOverN = 1.0f/sqrt_n_samples;  //  1/N

	int  i = 0;
	for(int a = 0;a<sqrt_n_samples;a++){
		for(int b = 0;b<sqrt_n_samples;b++){
			float x = (a + float(rand())/RAND_MAX ) * oneOverN;
			float y = (b + float(rand())/RAND_MAX ) * oneOverN;
			float theta = 2.0f * acosf(sqrtf(1.0f - x));
			float phi = 2.0f * pi * y;

			_samples[i].theta = theta;
			_samples[i].phi = phi;
			_samples[i].vec.set(sinf(theta)*cosf(phi), sinf(theta)*sinf(phi), cosf(theta));

			//precompute all SH coefficients for this sample
			int index = 0;
			for(int l = 0;l<n_bands;++l){
				for(int m = -l;m<=l;++m){
					_samples[i].coeff[index] = Y(l,m,theta,phi);	
					index++;
				}
			}
			i++;	//Next Sample
		}
	}
}

void CSH::Project_Spherical_Samples(Fun2SH fun,float *coeff,int n_coeff)
{
	const float weight = 4.0f*pi;	
	//init
	for(int i = 0;i<n_coeff;i++)
		coeff[i] = 0.0f;

	// for each sample
	for(size_t  i = 0;i<_samples.size();i++){
		float theta = _samples[i].theta;
		float phi = _samples[i].phi;
		float f = fun(theta,phi);
		for(int c = 0;c<n_coeff;++c)
			coeff[c] += f*_samples[i].coeff[c];
	}

	float factor = weight/_samples.size();
	for(int c = 0;c<n_coeff;++c)
		coeff[c] = coeff[c]*factor;
}

void CSH::SH(int sqrt_n_samples,int n_bands,Fun2SH fun,float *coeff,int n_coeff)
{
	assert(n_coeff == n_bands*n_bands);
	Setup_Spherical_Samples(sqrt_n_samples,n_bands);
	Project_Spherical_Samples(fun,coeff,n_coeff);
}




