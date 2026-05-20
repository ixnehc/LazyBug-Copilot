#include "stdh.h"
#include "GeomBox.h"



HRESULT GeomBox::GenerateVerticesIndices(VtxPosNormal *dataVb, DWORD sizeVb/*in bytes*/,  void *dataIb, DWORD sizeIb/*in bytes*/)
{
	if (!(dataVb&&dataIb))
	{
		return S_FALSE;
	}

#define INDEX_TYPE short

	if (sizeVb<NUMFACES*3*sizeof(VtxPosNormal)||sizeIb<NUMFACES*3*sizeof(INDEX_TYPE))
	{
		return S_FALSE;
	}


	//Vertices Generating

	VtxPosNormal* vertices_normal=(VtxPosNormal*)dataVb;

	int offset=0;


	//front face

	for (int iv=0;iv<6;iv++)
	{
		vertices_normal[offset+iv]._normal=i_math::vector3df(0.0f,0.0f,-1.0f);
	}

	vertices_normal[offset+0]._pos.x=-_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=-_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=-_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=-_height/2;	vertices_normal[offset+2]._pos.z=-_depth/2;

	offset+=3;

	vertices_normal[offset+0]._pos.x=_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=-_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=-_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=_height/2;	vertices_normal[offset+2]._pos.z=-_depth/2;

	offset+=3;


	//back face

	for (int iv=0;iv<6;iv++)
	{
		vertices_normal[offset+iv]._normal=i_math::vector3df(0.0f,0.0f,1.0f);
	}

	vertices_normal[offset+0]._pos.x=_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=_depth/2;
	vertices_normal[offset+1]._pos.x=_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=_depth/2;
	vertices_normal[offset+2]._pos.x=-_length/2;	vertices_normal[offset+2]._pos.y=-_height/2;	vertices_normal[offset+2]._pos.z=_depth/2;

	offset+=3;

	vertices_normal[offset+0]._pos.x=-_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=_depth/2;
	vertices_normal[offset+1]._pos.x=_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=_depth/2;
	vertices_normal[offset+2]._pos.x=-_length/2;	vertices_normal[offset+2]._pos.y=_height/2;	vertices_normal[offset+2]._pos.z=_depth/2;

	offset+=3;

	//top face

	for (int iv=0;iv<6;iv++)
	{
		vertices_normal[offset+iv]._normal=i_math::vector3df(0.0f,1.0f,0.0f);
	}


	vertices_normal[offset+0]._pos.x=-_length/2;	vertices_normal[offset+0]._pos.y=_height/2;	vertices_normal[offset+0]._pos.z=-_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=_height/2;	vertices_normal[offset+2]._pos.z=-_depth/2;

	offset+=3;

	vertices_normal[offset+0]._pos.x=_length/2;	vertices_normal[offset+0]._pos.y=_height/2;	vertices_normal[offset+0]._pos.z=-_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=_height/2;	vertices_normal[offset+2]._pos.z=_depth/2;

	offset+=3;

	//bottom face

	for (int iv=0;iv<6;iv++)
	{
		vertices_normal[offset+iv]._normal=i_math::vector3df(0.0f,-1.0f,0.0f);
	}


	vertices_normal[offset+0]._pos.x=-_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=-_height/2;	vertices_normal[offset+1]._pos.z=-_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=-_height/2;	vertices_normal[offset+2]._pos.z=_depth/2;

	offset+=3;

	vertices_normal[offset+0]._pos.x=_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=-_height/2;	vertices_normal[offset+1]._pos.z=-_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=-_height/2;	vertices_normal[offset+2]._pos.z=-_depth/2;

	offset+=3;

	//left face

	for (int iv=0;iv<6;iv++)
	{
		vertices_normal[offset+iv]._normal=i_math::vector3df(-1.0f,0.0f,0.0f);
	}


	vertices_normal[offset+0]._pos.x=-_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=_depth/2;
	vertices_normal[offset+2]._pos.x=-_length/2;	vertices_normal[offset+2]._pos.y=-_height/2;	vertices_normal[offset+2]._pos.z=-_depth/2;

	offset+=3;

	vertices_normal[offset+0]._pos.x=-_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=-_depth/2;
	vertices_normal[offset+1]._pos.x=-_length/2;	vertices_normal[offset+1]._pos.y=_height/2;		vertices_normal[offset+1]._pos.z=_depth/2;
	vertices_normal[offset+2]._pos.x=-_length/2;	vertices_normal[offset+2]._pos.y=_height/2;	vertices_normal[offset+2]._pos.z=-_depth/2;

	offset+=3;

	//right face

	for (int iv=0;iv<6;iv++)
	{
		vertices_normal[offset+iv]._normal=i_math::vector3df(1.0f,0.0f,0.0f);
	}


	vertices_normal[offset+0]._pos.x=_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=-_depth/2;
	vertices_normal[offset+1]._pos.x=_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=-_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=-_height/2;	vertices_normal[offset+2]._pos.z=_depth/2;

	offset+=3;

	vertices_normal[offset+0]._pos.x=_length/2;	vertices_normal[offset+0]._pos.y=-_height/2;	vertices_normal[offset+0]._pos.z=_depth/2;
	vertices_normal[offset+1]._pos.x=_length/2;	vertices_normal[offset+1]._pos.y=_height/2;	vertices_normal[offset+1]._pos.z=-_depth/2;
	vertices_normal[offset+2]._pos.x=_length/2;	vertices_normal[offset+2]._pos.y=_height/2;	vertices_normal[offset+2]._pos.z=_depth/2;

	offset+=3;



	//Indices Generating

	unsigned short* indices=(unsigned short*)dataIb;

	for (int index=0;index<NUMFACES*3;index++)
	{
		indices[index]=index;
	}


	return S_OK;
}



HRESULT GeomBox::GenerateVerticesIndices(VtxPos *dataVb, DWORD sizeVb, void *dataIb, DWORD sizeIb)
{

	if (!(dataVb&&dataIb))
	{
		return S_FALSE;
	}

#define INDEX_TYPE short

	if (sizeVb<NUMVTX*sizeof(VtxPos)||sizeIb<NUMFACES*3*sizeof(INDEX_TYPE))
	{
		return S_FALSE;
	}


	//Vertices Generating

	VtxPos* vertices=(VtxPos*)dataVb;



	vertices[0].x=-_length/2;	vertices[0].y=_height/2;	vertices[0].z=-_depth/2;
	vertices[1].x=_length/2;	vertices[1].y=_height/2;	vertices[1].z=-_depth/2;
	vertices[2].x=_length/2;	vertices[2].y=-_height/2;	vertices[2].z=-_depth/2;
	vertices[3].x=-_length/2;	vertices[3].y=-_height/2;	vertices[3].z=-_depth/2;


	vertices[4].x=-_length/2;	vertices[4].y=_height/2;	vertices[4].z=_depth/2;
	vertices[5].x=_length/2;	vertices[5].y=_height/2;	vertices[5].z=_depth/2;
	vertices[6].x=_length/2;	vertices[6].y=-_height/2;	vertices[6].z=_depth/2;
	vertices[7].x=-_length/2;	vertices[7].y=-_height/2;	vertices[7].z=_depth/2;


	//Indices Generating

	unsigned short* indices=(unsigned short*)dataIb;


	//front tris
	
	indices[0]=2; indices[1]=3; indices[2]=0;
	indices[3]=0; indices[4]=1; indices[5]=2;
	
	
	//back tris
	
	indices[6]=6; indices[7]=5; indices[8]=4;
	indices[9]=4; indices[10]=7; indices[11]=6;
	
	
	//top tris
	
	indices[12]=1; indices[13]=0; indices[14]=4;
	indices[15]=4; indices[16]=5; indices[17]=1;
	
	
	//bottom tris
	
	indices[18]=7; indices[19]=3; indices[20]=2;
	indices[21]=2; indices[22]=6; indices[23]=7;


	//left tris
	
	indices[24]=7; indices[25]=4; indices[26]=0;
	indices[27]=0; indices[28]=3; indices[29]=7;


	//right tris
	
	indices[30]=6; indices[31]=2; indices[32]=1;
	indices[33]=1; indices[34]=5; indices[35]=6;

	return S_OK;

} 