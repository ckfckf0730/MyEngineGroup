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
	uint32_t boneIdx;
	uint32_t boneType;
	uint32_t ikParentBone;
	DirectX::XMFLOAT3 startPos;
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

#pragma pack(1)
struct VMDMorph
{
	char name[15];
	uint32_t frameNo;
	float wight;
};
#pragma pack()

#pragma pack(1)
struct VMDCamera
{
	uint32_t frameNo;
	float distance;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 eulerAngle;
	uint8_t Interpolation[24];
	uint32_t fov;
	uint8_t persFlg;
};
#pragma pack()

struct VMDLight
{
	uint32_t frameNo;
	DirectX::XMFLOAT3 rgb;
	DirectX::XMFLOAT3 vec;
};

#pragma pack(1)
struct VMDSelfShadow
{
	uint32_t frameNo;
	uint8_t mode;
	float distance;
};
#pragma pack()

struct VMDIKEnable
{
	uint32_t frameNo;
	std::unordered_map<std::string, bool> ikEnableTable;
};

struct KeyFrame 
{
	unsigned int frameNo;
	DirectX::XMVECTOR quaternion;
	DirectX::XMFLOAT3 offset;
	DirectX::XMFLOAT2 p1,p2;
	KeyFrame(unsigned int fno, DirectX::XMVECTOR& q, DirectX::XMFLOAT3 ofst,
		const DirectX::XMFLOAT2& ip1, const DirectX::XMFLOAT2& ip2) :
		frameNo(fno),
		quaternion(q),
		offset(ofst),
		p1(ip1),
		p2(ip2) {}
};

class PMDModel;
class PMDModelInstance;
class ModelInstance;

struct PMDIK
{
	uint16_t boneIdx;
	uint16_t targetIdx;
	//uint8_t chainLen;  //the number of node between ...
	uint16_t iterations;
	float limit;
	std::vector<uint16_t> nodeIdxes;
};

class D3DAnimation
{
private:
	D3DAnimation() = default;

	unsigned int m_duration;

public:

	unsigned int Duration()
	{
		return m_duration;
	}

	std::vector<VMDMorph> m_morphs;
	std::vector<VMDCamera> m_cameraData;
	std::vector<VMDLight> m_lights;
	std::vector<VMDSelfShadow> m_selfShadowData;
	std::vector<VMDIKEnable> m_ikEnableData;
	std::unordered_map<std::string,std::vector<KeyFrame>> m_motionData;

	static D3DAnimation* LoadVMDFile(const char* fullFilePath);
	
	std::string m_fileName;

	

	void LoadAnimation(const char* path);
};

class D3DAnimationInstance
{
public:

	D3DAnimation* m_animation;

	PMDModelInstance* m_owner;

	DWORD m_startTime;


	void StartAnimation();
	void UpdateAnimation();

	void RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat);
	void IKSolve(int fremeNo);
	void SolveCCDIK(const PMDIK& ik);
	void SolveCosineIK(const PMDIK& ik);
	void SolveLookAt(const PMDIK& ik);
};

class ModelInstance;

class BasicModel
{
public:

	static std::map<std::string, BasicModel*> s_modelTable;

	D3D12_VERTEX_BUFFER_VIEW m_vbView = {};
	D3D12_INDEX_BUFFER_VIEW m_ibView = {};
	ID3D12DescriptorHeap* m_materialDescHeap = nullptr;
	//unsigned int m_vertNum;
	//unsigned int m_indicesNum;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertBuff;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_materialBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_idxBuff = nullptr;

	std::vector<Material> m_materials;

	std::vector<ID3D12Resource*> m_textureResources;
	std::vector<ID3D12Resource*> m_toonResources;
	std::vector<ID3D12Resource*> m_sphResources;
	std::vector<ID3D12Resource*> m_spaResources;

	std::vector<ModelInstance*> m_instances;

public:
	//int SetVertex(D3DDevice* _cD3DDev, Vertex* vertices, int verNum, unsigned short* indices, int indexNum);
	//int SetBasicModel(D3DDevice* _cD3DDev, const char* _FileFullName);
	virtual int SetVertices(D3DDevice* _cD3DDev, unsigned int _vertCount, unsigned char* _vertices,
		unsigned int _indCount, unsigned short* _indices);

public:
	int InitMaterial(int indicesNum);

};

class PMDModel : public  BasicModel
{
public:
	std::map<std::string, BoneNode> m_boneNodeTable;
	std::vector<PMDBone> m_pmdBones;
	std::vector<PMDIK> m_ikData;

	std::vector<std::string> m_boneNameArr;
	std::vector<BoneNode*> m_boneNodeAddressArr;
	std::string m_rootNodeStr;
	std::vector<uint32_t> m_kneeIdxes;

public:
	//int SetVertex(D3DDevice* _cD3DDev, Vertex* vertices, int verNum, unsigned short* indices, int indexNum);
	
	//int SetPMD(D3DDevice* _cD3DDev, const char* _FileFullName);
	
	int SetMaterials(D3DDevice* _cD3DDev, unsigned int matCount, DirectX::XMFLOAT3 diffuse[], float alpha[],
		float specularity[], DirectX::XMFLOAT3 specular[], DirectX::XMFLOAT3 ambient[], unsigned char edgeFlg[],
		unsigned char toonIdx[], unsigned int indicesNum[], const char* texFilePath[], const char* _FileFullName);
	int SetBones(D3DDevice* _cD3DDev, unsigned short boneNum,
		unsigned short ikNum, const char* boneName[], unsigned short parentNo[], unsigned short nextNo[],
		unsigned char type[], unsigned short ikBoneNo[], DirectX::XMFLOAT3 pos[],
		uint16_t boneIdx[], uint16_t targetIdx[], uint16_t iterations[], float limit[],
		uint8_t chainLen[], uint16_t** nodeIdxes);
	int SetBone();
	 int SetVertices(D3DDevice* _cD3DDev, unsigned int _vertCount, unsigned char* _vertices,
		unsigned int _indCount, unsigned short* _indices) override;
};

class D3DPipeline;

struct ShaderResource
{
	uint16_t datasize;

	ID3D12DescriptorHeap* heap;
	ID3D12Resource* buffer;
	void* mapData;

	UINT shaderRegisterNum;
};


class ModelInstance
{
public:
	BasicModel* m_model;

	static std::map <unsigned long long, ModelInstance*> s_uidModelTable;     //key is uid

	ID3D12DescriptorHeap* m_transformDescHeap = nullptr;
	ID3D12Resource* m_transformConstBuff = nullptr;
	Transform m_transform;
	DirectX::XMMATRIX* m_mapMatrices = nullptr;

	D3DPipeline* m_bindPipeline = nullptr;

	std::map<std::string, ShaderResource> m_shaderResouceTable;


	virtual int CreateTransformView(D3DDevice* _cD3DDev);

	~ModelInstance();
};

class PMDModelInstance : public ModelInstance
{
public:
	//std::vector<DirectX::XMMATRIX> m_boneMatrices;

	PMDModel* Model()
	{
		return static_cast<PMDModel*>(m_model);
	}

	D3DAnimationInstance* m_animationInstance;

	void InitAnimation(D3DAnimation* animationRes);

	//void BindAnimation(D3DAnimation* bindAnimation);
	int CreateTransformView(D3DDevice* _cD3DDev, int boneSize);
	void UpdateBoneMatrices(DirectX::XMMATRIX* boneMatrices, int size);
};

namespace
{
	enum class BoneType
	{
		Rotation,
		RotAndMove,
		IK,
		Undefined,
		IKChild,
		RotationChild,
		IKDestination,
		Invisible
	};
}

