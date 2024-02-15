#include<stdio.h>
#include <string.h>
#include <windows.h>
#include<iostream>
#include <vector>

#include<cmath>
using namespace std;


#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


const char* direction = "Models/";

void WriteModel(const char* dir, const char* fileName,
	unsigned int vertexCount, unsigned int indexCount,
	float(*vertices)[3], float(*uv)[2], float(*nomals)[3], int* indices);


void Cube();
void Sphere();

struct Vertex
{
	Vertex(float coord[3], float normal[3], float uv[2])
	{
		xCoord = coord[0];
		yCoord = coord[1];
		zCoord = coord[2];

		xNormal = normal[0];
		yNormal = normal[1];
		zNormal = normal[2];

		xUV = uv[0];
		yUV = uv[1];
	}

	float xCoord;
	float yCoord;
	float zCoord;
	
	float xNormal;
	float yNormal;
	float zNormal;

	float xUV;
	float yUV;
};

int main()
{
	int cmd;
	cout << "Please select create model Num,\n 1:Cube  2:Sphere" << endl;
	cin >> cmd;

	switch (cmd)
	{
	case 1:
		Cube();
		break;
	case 2:
		Sphere();
		break;
	default:
		break;
	}


}

void WriteModel(const char* dir, const char* fileName,
	unsigned int vertexCount, unsigned  int indexCount,
	float(*vertices)[3], float(*uv)[2], float(*nomals)[3], unsigned short* indices)
{
	char fullName[100] = "";
	strcat_s(fullName, dir);
	strcat_s(fullName, fileName);
	FILE* fp;
	errno_t err = fopen_s(&fp, fullName, "wb"); // "wb" for writing binary data in a new file
	if (fp == NULL)
	{
		CreateDirectoryA(dir, NULL);

		err = fopen_s(&fp, fullName, "wb");
		if (fp == NULL)
		{
			cout << "Cant openFile" << endl;
			return;
		}

	}

	fwrite(&vertexCount, sizeof(vertexCount), 1, fp);
	if (vertexCount > 0)
	{
		float(*verticesBuff)[8] = new float[vertexCount][8];
		for (int i = 0; i < vertexCount; i++)
		{
			verticesBuff[i][0] = vertices[i][0];
			verticesBuff[i][1] = vertices[i][1];
			verticesBuff[i][2] = vertices[i][2];

			verticesBuff[i][3] = nomals[i][0];
			verticesBuff[i][4] = nomals[i][1];
			verticesBuff[i][5] = nomals[i][2];

			verticesBuff[i][6] = uv[i][0];
			verticesBuff[i][7] = uv[i][1];
		}

		int size = sizeof(*vertices) + sizeof(*nomals) + sizeof(*uv);
		fwrite(verticesBuff, size, vertexCount, fp);
	}

	fwrite(&indexCount, sizeof(indexCount), 1, fp);
	if (indexCount > 0)
	{
		int size = sizeof(*indices);
		fwrite(indices, size, indexCount, fp);
	}


	fclose(fp);
}

void WriteModel(const char* dir, const char* fileName,
	std::vector<Vertex> vertices, std::vector<unsigned short> indices)
{
	int vertexCount = vertices.size();
	int indexCount = indices.size();
	float(*coords)[3] = new float[vertexCount][3];
	float(*uv)[2] = new float[vertexCount][2];
	float(*nomals)[3] = new float[vertexCount][3];
	unsigned short* indexArr = new unsigned short[indexCount];
	
	for (int i = 0; i < vertexCount; i++)
	{
		coords[i][0] = vertices[i].xCoord;
		coords[i][1] = vertices[i].yCoord;
		coords[i][2] = vertices[i].zCoord;

		uv[i][0] = vertices[i].xUV;
		uv[i][1] = vertices[i].yUV;

		nomals[i][0] = vertices[i].xNormal;
		nomals[i][1] = vertices[i].yNormal;
		nomals[i][2] = vertices[i].zNormal;
	}

	for (int i =0 ;i< indexCount;i++)
	{
		indexArr[i] = indices[i];
	}

	WriteModel(dir, fileName, vertexCount, indexCount,
		coords, uv, nomals, indexArr);
}

void Cube()
{

	float vertices[][3] =
	{
		//front
		{-0.5f, 0.5f, -0.5f},	//0		left	top		near
		{0.5f, 0.5f, -0.5f},	//1		right	top		near
		{0.5f, -0.5f, -0.5f},	//2		right	bottom	near
		{ -0.5f, -0.5f, -0.5f},	//3		left	bottom	near

		//back
		{0.5f, 0.5f, 0.5f},		//4		right	top		far
		{ -0.5f, 0.5f, 0.5f},	//5		left	top		far
		{-0.5f, -0.5f, 0.5f},	//6		left	bottom	far
		{ 0.5f, -0.5f, 0.5f},	//7		right	bottom	far

		//left
		{ -0.5f, 0.5f, 0.5f},	//8		left	top		far
		{-0.5f, 0.5f, -0.5f},	//9 	left	top		near
		{ -0.5f, -0.5f, -0.5f},	//10	left	bottom	near
		{-0.5f, -0.5f, 0.5f},	//11	left	bottom	far

		//right
		{0.5f, 0.5f, -0.5f},	//12	right	top		near
		{0.5f, 0.5f, 0.5f},		//13	right	top		far
		{ 0.5f, -0.5f, 0.5f},	//14	right	bottom	far
		{0.5f, -0.5f, -0.5f},	//15	right	bottom	near

		//top
		{ -0.5f, 0.5f, 0.5f},	//16	left	top		far
		{0.5f, 0.5f, 0.5f},		//17	right	top		far
		{0.5f, 0.5f, -0.5f},	//18	right	top		near
		{-0.5f, 0.5f, -0.5f},	//19	left	top		near

		//bottom
		{ -0.5f, -0.5f, -0.5f},	//20	left	bottom	near
		{0.5f, -0.5f, -0.5f},	//21	right	bottom	near
		{ 0.5f, -0.5f, 0.5f},	//22	right	bottom	far
		{-0.5f, -0.5f, 0.5f},	//23	left	bottom	far
	};

	float normals[][3] =
	{
		//front
		{0.0f, 0.0f, -1.0f},	//0		left	top		near
		{0.0f, 0.0f, -1.0f},	//1		right	top		near
		{0.0f, 0.0f, -1.0f},	//2		right	bottom	near
		{0.0f, 0.0f, -1.0f},	//3		left	bottom	near

		//back
		{0.0f, 0.0f, 1.0f},		//4		right	top		far
		{0.0f, 0.0f, 1.0f},		//5		left	top		far
		{0.0f, 0.0f, 1.0f},		//6		left	bottom	far
		{0.0f, 0.0f, 1.0f},		//7		right	bottom	far

		//left
		{-1.0f, 0.0f, 0.0f},	//8		left	top		far
		{-1.0f, 0.0f, 0.0f},	//9 	left	top		near
		{-1.0f, 0.0f, 0.0f},	//10	left	bottom	near
		{-1.0f, 0.0f, 0.0f},	//11	left	bottom	far

		//right
		{1.0f, 0.0f, 0.0f},		//12	right	top		near
		{1.0f, 0.0f, 0.0f},		//13	right	top		far
		{1.0f, 0.0f, 0.0f},		//14	right	bottom	far
		{1.0f, 0.0f, 0.0f},		//15	right	bottom	near

		//top
		{0.0f, 1.0f, 0.0f},		//16	left	top		far
		{0.0f, 1.0f, 0.0f},		//17	right	top		far
		{0.0f, 1.0f, 0.0f},		//18	right	top		near
		{0.0f, 1.0f, 0.0f},		//19	left	top		near

		//bottom
		{0.0f, -1.0f, 0.0f},	//20	left	bottom	near
		{0.0f, -1.0f, 0.0f},	//21	right	bottom	near
		{0.0f, -1.0f, 0.0f},	//22	right	bottom	far
		{0.0f, -1.0f, 0.0f},	//23	left	bottom	far
	};

	int verCount = _countof(vertices);

	float uv[][2] =
	{
		//front
		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1},

		//back
		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1},

		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1},

		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1},

		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1},

		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1},
	};

	int uvCount = _countof(uv);
	int nolCount = 0;

	unsigned short index[] =
	{
		//front
		0,1,2,
		2,3,0,

		//back
		4,5,6,
		6,7,4,

		//left
		8,9,10,
		10,11,8,

		//right
		12,13,14,
		14,15,12,

		//top
		16,17,18,
		18,19,16,

		//bottom
		20,21,22,
		22,23,20,
	};
	int indexCount = _countof(index);

	WriteModel(direction, "Cube.vd", verCount, indexCount,
		vertices, uv, normals, index);
}

void NormalizeVector(float* vector)
{
	double length = 0.0;
	for (int i = 0; i < 3; i++)
	{
		length += vector[i] * vector[i];
	}
	length = std::sqrt(length);

	for (int i = 0; i < 3; i++)
	{
		vector[i] = vector[i] / length;
	}
}

void Sphere()
{
	std::vector<Vertex> vertices;

	float r = 0.5f;
	float topPoint[3] = { 0.0f,0.5f,0.0f };
	float bottomPoint[3] = { 0.0f,-0.5f,0.0f };

	float topNormal[3] = { 0.0f,1.0f,0.0f };
	float bottomNormal[3] = { 0.0f,-1.0f,0.0f };

	float topUV[2] = { 0.5f,0.0f };
	float bottomUV[2] = { 0.5f,-1.0f };

	int xCount = 12;
	int yCount = 7;

	float pi = (float)M_PI;

	vertices.push_back(Vertex(topPoint, topNormal, topUV));
	

	for (int y = 1; y < yCount - 1; y++)
	{
		float yRadian = (pi / (yCount - 1)) * y;
		float yCoord = cos(yRadian) * r;
		float yUV = yRadian / pi;

		float xzRadius = sin(yRadian) * r;

		for (int x = 0; x < xCount + 1; x++)
		{
			float curPoint[3];
			float curNormal[3];
			float curUV[2];

			float xzRadian = (2 * pi / xCount) * x;
			float xCoord = sin(xzRadian) * xzRadius;
			float zCoord = cos(xzRadian) * xzRadius;

			curNormal[0] = curPoint[0] = xCoord;
			curNormal[1] = curPoint[1] = yCoord;
			curNormal[2] = curPoint[2] = zCoord;

			NormalizeVector(curNormal);

			float xUV = xzRadian / (2 * pi);

			curUV[0] = xUV;
			curUV[1] = yUV;

			vertices.push_back(Vertex(curPoint, curNormal, curUV));
		}
	}

	vertices.push_back(Vertex(bottomPoint, bottomNormal, bottomUV));


	std::vector<unsigned short> indices;
	unsigned short curIndex = 0;
	for (unsigned short i = 0; i < xCount; i++)
	{
		indices.push_back(curIndex);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
	}
	curIndex += 1;

	for (unsigned short y = 1; y < yCount - 2; y++)
	{
		for (unsigned short x = 0; x < xCount; x++)
		{
			indices.push_back(curIndex + x);
			indices.push_back(curIndex + x + 1);
			indices.push_back(curIndex + x + xCount + 1);

			indices.push_back(curIndex + x + xCount + 1);
			indices.push_back(curIndex + x + 1);
			indices.push_back(curIndex + x + xCount + 2);
		}
		curIndex += xCount + 1;
	}

	unsigned short lastInd = curIndex + xCount + 1;
	for (unsigned short i = 0; i < xCount; i++)
	{
		

		indices.push_back(curIndex + i);
		indices.push_back(curIndex + i + 1);
		indices.push_back(lastInd);
	}



	WriteModel(direction, "Sphere.vd",vertices, indices);
}