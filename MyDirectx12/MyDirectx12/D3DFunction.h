#pragma once

#include"D3DAPI.h"
#include<vector>
#include"PMDActor.h"
#include<map>

struct SceneMatrix;

class D3DMulPassRender;
class ModelInstance;

class D3DDevice 
{
public:
	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceVal = 0;
	IDXGIFactory6* pDxgiFactory = nullptr;
	ID3D12Device* pD3D12Device = nullptr;
	ID3D12CommandAllocator* pCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* pCmdList = nullptr;
	ID3D12CommandQueue* pCmdQueue = nullptr;

	int	InitializeFence();
	int InitializeDXGIDevice();
	int InitializeCommand();
	
	void WaitForCommandQueue();
	
};

class D3DCamera
{
public:
	UINT64 Uid;

	float m_backColor[4] = { 0.0f,0.0f,1.0f,1.0f };

	IDXGISwapChain4* m_swapchain = nullptr;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	std::vector<ID3D12Resource*> m_backBuffers;
	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorrect = {};
	ID3D12DescriptorHeap* m_dsvHeap = nullptr;


	//D3DMulPassRender* m_mulPassRender;

	int CreateSwapChain(HWND hwnd, UINT width, UINT height);
	int CreateRenderTargetView();
	int CreateDepthStencilView(UINT width, UINT height);
	int Draw(D3DDevice* _cD3DDev);

	void SetViewPort(UINT width, UINT height);
	void Clear();
	void Barrier(ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void Flip();

	//void InitMulPassRender();

	void Release();
};

class BasicModel;

class D3DPipeline
{
private:
	std::string m_name;



public:

	std::map<BasicModel*, std::map<ModelInstance*,int>> RenderModelTable;

	D3DPipeline(const char* name);

	ID3D12RootSignature* m_rootsignature = nullptr;

	ID3D12PipelineState* m_pipelinestate = nullptr;

	ID3D12DescriptorHeap* m_descHeap = nullptr;
	ID3D12Resource* m_shaderBuff = nullptr;
	
	UINT64 m_curNumDescriptors;
	UINT64 m_nextDescIndex;

	SceneMatrix* m_mapSceneMatrix = nullptr;

	

	/*int SetPipeline(D3DDevice* _cD3DDev, D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT numElements,
		LPCWSTR vsShader, LPCWSTR psShader);*/
	int CreatePipeline(D3DDevice* _cD3DDev, D3D12_INPUT_ELEMENT_DESC inputLayout[], UINT numElements,
		LPCSTR vsCode, LPCSTR vsEntry, LPCSTR psCode, LPCSTR psEntry);

	int CreateSceneView(D3DDevice* _cD3DDev);
	int CreateDescriptHeap(D3DDevice* _cD3DDev, UINT64 instanceNums);
	UINT CreateConstantDescript(ID3D12Device* pDevice, ID3D12Resource* res);
	UINT CreateConstantDescript(ID3D12Device* pDevice, D3D12_GPU_VIRTUAL_ADDRESS address, UINT resSize);
	UINT CreateShaderResDescript(ID3D12Device* pDevice, ID3D12Resource* res);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDescHandle(UINT offset);

	void SetCameraTransform(DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up);
	//void SetCameraProjection(float FovAngleY, float AspectRatio, float NearZ, float Far);
	void Draw(ID3D12GraphicsCommandList*, ID3D12Device*);
};

