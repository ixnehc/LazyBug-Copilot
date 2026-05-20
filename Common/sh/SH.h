
#pragma once

#define MAX_BANDSNUM	8
#define MAX_COEFFNUM    MAX_BANDSNUM*MAX_BANDSNUM

typedef float( * Fun2SH)(float,float);

class CSH
{
public:
	CSH(void);
	~CSH(void);

	struct vector3d
	{
		float x,y,z;
		void set(float v0,float v1,float v2){ x = v0; y = v1; z = v2;}
	};

	struct SHSample
	{
		float theta;
		float phi;
		vector3d vec;	
		float coeff[MAX_COEFFNUM];	
	};

	void  Setup_Spherical_Samples(int sqrt_n_samples,int n_bands);
	void  Project_Spherical_Samples(Fun2SH fun,float *coeff,int n_coeff);
	float Y(int l,int m,float theta,float phi) const;
	float P(int l,int m,float x) const;
	float K(int l,int m) const;
	float Factorial(int n) const;

	void SH(int sqrt_n_samples,int n_band,Fun2SH fun,float *coeff,int n_coeff);

protected:
	void _InitFactorial();

private:
	std::vector<SHSample> _samples;
	std::vector<float> _factorials;
};





