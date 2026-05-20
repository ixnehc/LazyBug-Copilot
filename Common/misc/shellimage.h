#pragma once

#include <string>


//ShellImageDesc string format:
//texturepath|left,top,right,bottom|bBlending
//note:bBlending could be missed,the default value is FALSE

struct ShellImageDesc
{
	BOOL fromString(const char *str);
	BOOL toString(std::string &str);
	std::string pathTex;
	i_math::recti rc;
	BOOL bBlending;
};
