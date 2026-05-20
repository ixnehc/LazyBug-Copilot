
#pragma once


struct dtVec3
{
	dtVec3(){x = 0;y = 0;z = 0;}

	dtVec3(float x_, float y_, float z_){x = x_; y = y_; z = z_;}

	dtVec3 operator +(const dtVec3 &oth) const
	{
		return dtVec3(x + oth.x,y + oth.y,z + oth.z);
	}

	dtVec3 operator -(const dtVec3 &oth) const
	{
		return dtVec3(x - oth.x,y - oth.y,z - oth.z);
	}

	friend  dtVec3 operator *(const dtVec3 &v,float s);
	friend  dtVec3 operator *(float s,const dtVec3 &v);

	dtVec3 &operator *=(float s)
	{
		x *= s; 
		y *= s; 
		z *= s;
		return *this;
	}

	dtVec3 &operator /=(float s)
	{
		x /= s; 
		y /= s; 
		z /= s;
		return *this;
	}
	
	dtVec3 &operator +=(const dtVec3 &oth)
	{
		x += oth.x; 
		y += oth.y; 
		z += oth.z;
		return *this;
	}

	float getLength() const
	{
		return sqrtf(x*x + y*y + z*z);
	}	
	
	float x,y,z;
};

struct dtVec2
{
	dtVec2(){x = 0;y = 0;}

	dtVec2(float x_,float y_){x = x_; y = y_;}

	float x,y;
};