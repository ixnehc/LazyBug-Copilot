#pragma once

#include "../math/iMath.h"


//////////////////////////////////////////////////////////////////////////
//Unit System defines

//16 bit HUS(Horizontal Unit System)
#define HUS16_HuPerMtr 2
#define HUS16_MtrFromHu(hu) (((float)(hu))/(float)HUS16_HuPerMtr)
#define HUS16_HuFromMtr(mtr) ((short)(FloatToNearestInt((mtr)*(float)HUS16_HuPerMtr)))

//16 bit VUS(Vertical Unit System)
#define VUS16_VuPerMtr 32
#define VUS16_MtrFromVu(vu) (((float)(vu))/(float)VUS16_VuPerMtr)
#define VUS16_VuFromMtr(mtr) ((short)(FloatToNearestInt((mtr)*(float)VUS16_VuPerMtr)))

//14 bit VUS(Vertical Unit System)
#define VUS14_BaseMtr (-256) //in meter
#define VUS14_MaxMeterRange (1000)//1024
#define VUS14_VuPerMtr 16
#define VUS14_Base (VUS14_BaseMtr*VUS14_VuPerMtr) //in VU
#define VUS14_MaxVu (VUS14_MaxMeterRange*VUS14_VuPerMtr)//in VU

#define VUS14_MtrFromVu(vu) (((float)(vu)/(float)VUS14_VuPerMtr)+(float)VUS14_BaseMtr)
#define VUS14_VuFromMtr(m) (i_math::clamp_i(FloatToNearestInt(((m)-(float)VUS14_BaseMtr)*(float)VUS14_VuPerMtr),0,VUS14_MaxVu))

#define VUS14_MtrFromVuOff(vu) (((float)(((int)(vu))-0x7f))/(float)VUS14_VuPerMtr)
#define VUS14_VuOffFromMtr(mtr) ((BYTE)(i_math::clamp_i(FloatToNearestInt((mtr)*(float)VUS14_VuPerMtr),-0x7f,0x7f)+0x7f))

//13 bit VUS(Vertical Unit System)
#define VUS13_BaseMtr (-32) //in meter
#define VUS13_MaxMeterRange (63)//63
#define VUS13_VuPerMtr 128
#define VUS13_Base (VUS13_BaseMtr*VUS13_VuPerMtr) //in VU
#define VUS13_MaxVu (VUS13_MaxMeterRange*VUS13_VuPerMtr)//in VU

#define VUS13_MtrFromVu(vu) (((float)(vu)/(float)VUS13_VuPerMtr)+(float)VUS13_BaseMtr)
#define VUS13_VuFromMtr(m) (i_math::clamp_i(FloatToNearestInt(((m)-(float)VUS13_BaseMtr)*(float)VUS13_VuPerMtr),0,VUS13_MaxVu))


//16 bit TUS(TexCoord Unit System)
#define TUS16_TuCount 32767.0f
#define TUS16_FromTu(tu) (((float)(tu))/(float)TUS16_TuCount)
#define TUS16_ToTu(uvvalue) ((short)(FloatToNearestInt((i_math::clamp_f(uvvalue,0,1))*(float)TUS16_TuCount)))

//6 bit TUS(TexCoord Unit System)
#define TUS6_TuCount 64.0f
#define TUS6_FromTu(tu) (((float)(tu))/(float)TUS6_TuCount)
#define TUS6_ToTu(uvvalue) ((short)(FloatToNearestInt((i_math::clamp_f(uvvalue,0,1))*(float)TUS6_TuCount)))

//6 bit NUS(Normal Unit System)
#define NUS6_NuCount 63.0f
#define NUS6_FromNu(nu) ((float)nu/(NUS6_NuCount/2.0f)-1.0f)
#define NUS6_ToNu(nv) ((DWORD)(FloatToNearestInt((nv+1.0f)*(NUS6_NuCount/2.0f))))

//5 bit NUS(Normal Unit System)
#define NUS5_NuCount 31.0f
#define NUS5_FromNu(nu) ((float)nu/(NUS5_NuCount/2.0f)-1.0f)
#define NUS5_ToNu(nv) ((DWORD)(FloatToNearestInt((nv+1.0f)*(NUS5_NuCount/2.0f))))

//8 bit NUS(Normal Unit System)
#define NUS8_NuCount 255.0f
#define NUS8_FromNu(nu) ((float)nu/(NUS8_NuCount/2.0f)-1.0f)
#define NUS8_ToNu(nv) (BYTE)(i_math::clamp_u((DWORD)(FloatToNearestInt((nv+1.0f)*(NUS8_NuCount/2.0f))),0,255))

//16 bit Signed NUS(Normal Unit System)
#define SNUS16_NuCount 32767.0f
#define SNUS16_FromNu(nu) (((float)(nu))/SNUS16_NuCount)
#define SNUS16_ToNu(nv) ((short)(i_math::clamp_i(FloatToNearestInt((nv)*NUS8_NuCount),-32767,32767)))

//8 bit unsigned LUS(Light Unit System)
#define LUS8_FromLu(lu) (((float)(lu))/255.0f)
#define LUS8_ToLu(lv) i_math::clamp_i(FloatToNearestInt((lv)*255.0f),0,255)


//8 bit signed LUS(Light Unit System)
#define SLUS8_FromLu(lu) ((((float)(lu))-128.0f)/128.0f)
#define SLUS8_ToLu(lv) ((BYTE)i_math::clamp_u((DWORD)(FloatToNearestInt(((lv)+1.0f)*128.0f)),0,255))


