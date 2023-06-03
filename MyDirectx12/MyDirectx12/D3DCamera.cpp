#include"D3DFunction.h"
#include"D3DResourceManage.h"

int D3DCamera::CreateSwapChain(HWND hwnd,UINT width , UINT height)
{
	auto dxgiFactory = D3DResourceManage::Instance().pGraphicsCard->pDxgiFactory;
	HRESULT result = S_OK;

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = width;
	swapchainDesc.Height = height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = dxgiFactory->CreateSwapChainForHwnd(
		D3DResourceManage::Instance().pGraphicsCard->pCmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&m_swapchain);

	return 1;
}

int D3DCamera::CreateFinalRenderTarget(UINT width, UINT height)
{
	auto device = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;
	auto graphicsCard = D3DResourceManage::Instance().pGraphicsCard;
	HRESULT result = S_OK;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//front and back 
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeaps));
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = m_swapchain->GetDesc(&swcDesc);
	for (int i = 0; i < swcDesc.BufferCount; i++)
	{
		m_backBuffers.push_back(nullptr);
	}

	//SRGB render target view setup
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < swcDesc.BufferCount; ++i)
	{
		result = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
		device->CreateRenderTargetView(m_backBuffers[i], &rtvDesc, handle);
		handle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//--depth stencil view setup----
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = width;
	depthResDesc.Height = height;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	ID3D12Resource* depthBuff = nullptr;

	result = device->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthBuff));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", "Create Depth Stencil resource fault");
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", "Create dsv HeapDesc fault");
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(
		depthBuff,
		&dsvDesc,
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart());


	//-------------create view port-------------- 
	m_viewport.Width = width;
	m_viewport.Height = height;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;


	m_scissorrect.top = 0;
	m_scissorrect.left = 0;
	m_scissorrect.right = m_scissorrect.left + width;
	m_scissorrect.bottom = m_scissorrect.top + height;


	auto cmdList = D3DResourceManage::Instance().pGraphicsCard->pCmdList;

	auto bbIdx = m_swapchain->GetCurrentBackBufferIndex();

	m_barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	m_barrierDesc.Transition.pResource = m_backBuffers[bbIdx];
	m_barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	m_barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmdList->ResourceBarrier(1, &m_barrierDesc);

	//appoint render target 
	auto rtvH = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	//clear screen
	float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//yellow
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	m_barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmdList->ResourceBarrier(1, &m_barrierDesc);

	cmdList->Close();

	//implement commond list
	ID3D12CommandList* cmdlists[] = { cmdList };
	D3DResourceManage::Instance().pGraphicsCard->pCmdQueue->ExecuteCommandLists(1, cmdlists);
	//wait
	D3DResourceManage::Instance().pGraphicsCard->pCmdQueue->Signal(graphicsCard->m_fence, ++graphicsCard->m_fenceVal);

	if (graphicsCard->m_fence->GetCompletedValue() != graphicsCard->m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		graphicsCard->m_fence->SetEventOnCompletion(graphicsCard->m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	D3DResourceManage::Instance().pGraphicsCard->pCmdAllocator->Reset();//キューをクリア
	cmdList->Reset(D3DResourceManage::Instance().pGraphicsCard->pCmdAllocator, nullptr);//再びコマンドリストをためる準備

	//flip
	m_swapchain->Present(1, 0);

	return 1;
}

using namespace DirectX;

int D3DCamera::Draw(D3DDevice* _cD3DDev)
{
	auto cmdList = D3DResourceManage::Instance().pGraphicsCard->pCmdList;
	auto d3ddevice = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

	//------------------set render target state------------------------
	//Get current back buffer
	auto bbIdx = m_swapchain->GetCurrentBackBufferIndex();

	m_barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	m_barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_barrierDesc.Transition.pResource = m_backBuffers[bbIdx];

	cmdList->ResourceBarrier(1, &m_barrierDesc);

	auto rtvH = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * d3ddevice->
		GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvh = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvh);

	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmdList->RSSetViewports(1, &m_viewport);
	cmdList->RSSetScissorRects(1, &m_scissorrect);

	//-------------render each pipeline------------------
	for (auto iter = D3DResourceManage::Instance().PipelineTable.begin(); 
		iter != D3DResourceManage::Instance().PipelineTable.end(); iter++) 
	{
		auto pipeline = iter->second;
		pipeline->Draw(cmdList, d3ddevice);
	}

	//------------------set render target state back------------------------
	m_barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	cmdList->ResourceBarrier(1, &m_barrierDesc);

	cmdList->Close();

	ID3D12CommandList* cmdlists[] = { cmdList };
	D3DResourceManage::Instance().pGraphicsCard->pCmdQueue->ExecuteCommandLists(1, cmdlists);
	//
	D3DResourceManage::Instance().pGraphicsCard->pCmdQueue->Signal(_cD3DDev->m_fence, ++_cD3DDev->m_fenceVal);

	if (_cD3DDev->m_fence->GetCompletedValue() != _cD3DDev->m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_cD3DDev->m_fence->SetEventOnCompletion(_cD3DDev->m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	D3DResourceManage::Instance().pGraphicsCard->pCmdAllocator->Reset();

	for (auto iter = D3DResourceManage::Instance().PipelineTable.begin();
		iter != D3DResourceManage::Instance().PipelineTable.end(); iter++)
	{
		auto pipeline = iter->second;
		cmdList->Reset(D3DResourceManage::Instance().pGraphicsCard->pCmdAllocator, pipeline->m_pipelinestate);
	}


	//
	m_swapchain->Present(1, 0);

	return 1;
}