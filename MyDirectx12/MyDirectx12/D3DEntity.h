#pragma once

#include"D3DAPI.h"
#include<DirectXMath.h>
#include"D3DFunction.h"
#include<map>
#include<unordered_map>

class D3DDevice;

//struct PMDVertex  
//{
//	DirectX::XMFLOAT3 pos;
//	DirectX::XMFLOAT3 normal;
//	DirectX::XMFLOAT2 uv;
//	unsigned short boneNo[2];
//	unsigned char boneWeight;
//	unsigned char edgeFlg;
//};

#pragma pack(1)
struct PMDMaterial
{
	DirectX::XMFLOAT3 diffuse;
	float alpha;
	float specularity;
	DirectX::XMFLOAT3 specular;
	DirectX::XMFLOAT3 ambient;
	unsigned char toonIdx;
	unsigned char edgeFlg;
	unsigned int indicesNum;
	char texFilePath[20];
};//70byte but when alignment it's 72byte
#pragma pack()

struct SceneMatrix
{
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
	DirectX::XMFLOAT3 eye;
};

struct Transform
{
	DirectX::XMMATRIX world;

};

struct MaterialForHlsl
{
	DirectX::XMFLOAT3 diffuse;
	float alpha;
	DirectX::XMFLOAT3 specular;
	float specularity;
	DirectX::XMFLOAT3 ambient;
};

struct AdditionalMaterial
{
	std::string texPath;
	int toonIdx;
	bool edgeFlg;
};

struct Material
{
	unsigned int indicesNum;
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

#pragma pack(1)
struct PMDBone
{
	char boneName[20];
	unsigned short parentNo;
	unsigned short nextNo;
	unsigned char type;
	unsigned short ikBoneNo;
	DirectX::XMFLOAT3 pos;
};
#pragma pack(0)

struct BoneNode
{
	int boneIdx;
	DirectX::XMFLOAT3 startPos;
	DirectX::XMFLOAT3 endPos;
	std::vector<BoneNode*> children;
};

struct VMDMotion
{
	char boneName[15];
	unsigned int frameNo;
	DirectX::XMFLOAT3 location;
	DirectX::XMFLOAT4 quaternion;
	unsigned char bezier[64];
};

struct Motion
{
	unsigned int frameNo;
	DirectX::XMVECTOR quaternion;
	Motion(unsigned int fno, DirectX::XMVECTOR& q): frameNo(fno),quaternion(q)
	{
		
	}
};

struct KeyFrame 
{
	unsigned int frameNo;
	DirectX::XMVECTOR quaternion;
	//DirectX::XMFLOAT2 p1, p2;
	KeyFrame(unsigned int fno, DirectX::XMVECTOR& q/*, const DirectX::XMFLOAT2& ip1, const DirectX::XMFLOAT2& ip2*/) :
		frameNo(fno),
		quaternion(q)/*,
		p1(ip1),
		p2(ip2)*/ {}
};

class D3DAnimation
{
public:
	std::vector<VMDMotion> m_vmdMotionData;
	std::unordered_map<std::string,std::vector<KeyFrame>> m_motionData;

	int LoadVMDFile(const char* fullFilePath);

};

class D3DModel
{
public:
	
	D3D12_VERTEX_BUFFER_VIEW m_vbView = {};
	D3D12_INDEX_BUFFER_VIEW m_ibView = {};
	ID3D12DescriptorHeap* m_materialDescHeap = nullptr;
	unsigned int m_vertNum;
	unsigned int m_indicesNum;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertBuff;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_materialBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_idxBuff = nullptr;

	ID3D12DescriptorHeap* m_transformDescHeap = nullptr;
	ID3D12Resource* m_transformConstBuff = nullptr;
	Transform m_transform;
	DirectX::XMMATRIX* m_mapMatrices = nullptr;

	std::vector<Material> m_materials;
	std::vector<PMDBone> m_pmdBones;

	std::vector<DirectX::XMMATRIX> m_boneMatrices;
	std::map<std::string, BoneNode> m_boneNodeTable;

	std::vector<ID3D12Resource*> m_textureResources;
	std::vector<ID3D12Resource*> m_toonResources;
	std::vector<ID3D12Resource*> m_sphResources;
	std::vector<ID3D12Resource*> m_spaResources;

	D3DAnimation* m_animation;


public:
	//int SetVertex(D3DDevice* _cD3DDev, Vertex* vertices, int verNum, unsigned short* indices, int indexNum);
	int SetPMD(D3DDevice* _cD3DDev, const char* _FileFullName);
	int SetBone();
	int CreateTransformView(D3DDevice* _cD3DDev);

	void LoadAnimation(const char* path);


	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);
};

