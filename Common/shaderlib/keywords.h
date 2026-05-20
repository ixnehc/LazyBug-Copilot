#pragma once


#define SHADER_KEYWORDS																\
"asm1 asm_fragment bool column_major compile "\
"compile_fragment const discard decl1 do switch "\
"double else extern false "\
"for if  in inline "\
"inout matrix out pass1 "\
"pixelfragment return register row_major sampler "\
"sampler1D sampler2D sampler3D samplerCUBE sampler_state "\
"shared stateblock stateblock_state static string "\
"struct technique1 texture texture1D texture2D "\
"texture3D textureCUBE true typedef uniform "\
"vector vertexfragment void volatile while "\
"float float2 float3 float4 float1x1 float1x2 float1x3 float1x4 float2x1 float2x2 float2x3 float2x4 float3x1 float3x2 float3x3 float2x4 float4x1 float4x2 float4x3 float4x4 "\
"int int2 int3 int4 int1x1 int1x2 int1x3 int1x4 int2x1 int2x2 int2x3 int2x4 int3x1 int3x2 int3x3 int2x4 int4x1 int4x2 int4x3 int4x4 "\
"half half2 half3 half4 half1x1 half1x2 half1x3 half1x4 half2x1 half2x2 half2x3 half2x4 half3x1 half3x2 half3x3 half2x4 half4x1 half4x2 half4x3 half4x4 "\
"double double2 double3 double4 double1x1 double1x2 double1x3 double1x4 double2x1 double2x2 double2x3 double2x4 double3x1 double3x2 double3x3 double2x4 double4x1 double4x2 double4x3 double4x4 "\
"vs_1_1 vs_2_0 vs_2_a vs_3_0 "\
"ps_1_1 ps_1_2 ps_1_3 ps_1_4 ps_2_0 ps_2_a ps_3_0 "\
"abs acos all any asin atan atan2f ceil clamp clip cosf cosh cross D3DCOLORtoUBYTE4 ddx "\
"ddy degrees determinant distance dot exp exp2 faceforward floor fmod frac frexp "\
"fwidth isfinite isinf isnan ldexp length lerp lit log log10 log2 max min modf mul noise "\
"normalize pow radians reflect refract round rsqrt saturate sign sinf sincos sinh smoothstep "\
"sqrtf step tan tanh tex1D tex1D tex1Dproj tex1Dbias tex2D tex2D tex2Dproj tex2Dbias "\
"tex3D tex3D tex3Dproj tex3Dbias texCUBE texCUBE texCUBEproj texCUBEbias transpose "\
"technique pass"

#define SHADER_KEYWORDS_CG											\
"if else break return continue discard while for do void bool  bvec2 bvec3 bvec4 int ivec2 ivec3 ivec4 "\
"float vec2  vec3  vec4 mat2  mat3  mat4 mat2x2 mat2x3 mat2x4 mat3x2 mat3x3 mat3x4 mat4x2 mat4x3 mat4x4 "\
"sampler1D sampler2D sampler3D samplerCUBE "\
"sampler1DShadow sampler2DShadow struct const attribute varying uniform "\
"in out inout __LINE__ __FILE__ __VERSION__ true false "\
"radians degrees sinf cosf tan asin acos atan pow exp2 log2 sqrtf inversesqrt "\
"abs sign floor ceil fract mod min max clamp mix step smoothstep "\
"length distance dot cross normalize ftransform faceforward reflect "\
"matrixcompmult lessThan lessThanEqual greaterThan greaterThanEqual equal notEqual any all not "\
"texture1D texture1DProj texture1DLod texture1DProjLod "\
"texture2D texture2DProj texture2DLod texture2DProjLod "\
"texture3D texture3DProj texture3DLod texture3DProjLod "\
"textureCube textureCubeLod "\
"shadow1D shadow1DProj shadow1DLod shadow1DProjLod "\
"shadow2D shadow2DProj shadow2DLod shadow2DProjLod "\
"dFdx dFdy fwidth noise1 noise2 noise3 noise4 "\
"refract exp log "\
"vi_pos vi_pos2 vi_posRHW vi_xyzw vi_xyzw2 "\
"vi_normal vi_normal2 vi_binormal vi_tangent vi_psize vi_dif vi_spec "\
"vi_weightx2 vi_weightx3 vi_weightx4 vi_blendindice vi_blendindice2 "\
"vi_posTrrn vi_uvTrrn0 vi_uvTrrn1 vi_uvTrrn2 vi_uvTrrn3 vi_uvTrrn4 vi_uvTrrn5 vi_nmlTrrn "\
"vi_uv0 vi_uv1 vi_uv2 vi_uv3 vi_uv4 vi_uv5 vi_uv6 vi_uv7 vi_uvw0 vi_uvw1 vi_uvw2 vi_uvw3 vi_uvw4 vi_uvw5 vi_uvw6 vi_uvw7 "\
"vi_qux0 vi_qux1 vi_qux2 vi_qux3"\
"vi_posC vi_normalC vi_binormalC vi_uvVege vi_colVege vi_geomVege vi_parVege vi_colF "\
"gl_Position gl_FragColor"




#define SHADER_KEYWORDS2 "VertexIn VertexShader PixelIn PixelOut PixelShader EffectParam FeatureGroup state declare assign feature basefeature priority global vs_ver ps_ver cap unifeature "