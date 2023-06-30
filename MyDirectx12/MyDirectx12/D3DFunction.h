#pragma once

#include"D3DAPI.h"
#include<vector>
#include"PMDActor.h"

struct SceneMatrix;

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
	
};

class D3DCamera
{
public:

	IDXGISwapChain4* m_swapchain = nullptr;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	std::vector<ID3D12Resource*> m_backBuffers;
	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorrect = {};
	ID3D12DescriptorHeap* m_dsvHeap = nullptr;
	D3D12_RESOURCE_BARRIER m_barrierDesc = {};

	int CreateSwapChain(HWND hwnd, UINT width, UINT height);
	int CreateFinalRenderTarget(UINT width, UINT height);
	int Draw(D3DDevice* _cD3DDev);
};

class D3DPipeline
{
private:
	const char* m_name = nullptr;

public:
	D3DPipeline(const char* name);


	ID3D12RootSignature* m_rootsignature = nullptr;

	ID3D12PipelineState* m_pipelinestate = nullptr;

	ID3D12DescriptorHeap* m_sceneDescHeap = nullptr;
	ID3D12Resource* m_sceneConstBuff = nullptr;

	SceneMatrix* m_mapSceneMatrix = nullptr;

	

	int SetPipeline(D3DDevice* _cD3DDev);
	int CreateSceneView(D3DDevice* _cD3DDev);
	void SetCameraTransform(DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up);
	void SetCameraProjection(float FovAngleY, float AspectRatio, float NearZ, float Far);
	void Draw(ID3D12GraphicsCommandList*, ID3D12Device*);
};