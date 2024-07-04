#include"D3DFunction.h"
#include"D3DResourceManage.h"
#include"D3DMulPassRender.h"
#include <comdef.h>
#include <iostream>
#include <fstream>

using namespace DirectX;

int D3DCamera::CreateSwapChain(HWND hwnd, UINT width, UINT height)
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



int D3DCamera::CreateRenderTargetView()
{
	auto device = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;
	HRESULT result = S_OK;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;//front and back 
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = m_swapchain->GetDesc(&swcDesc);
	m_backBuffers.resize(swcDesc.BufferCount);

	//SRGB render target view setup
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->
		GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < swcDesc.BufferCount; ++i)
	{
		result = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
		device->CreateRenderTargetView(m_backBuffers[i], &rtvDesc, handle);
		handle.ptr += device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	return 1;
}

int D3DCamera::CreateDepthStencilView(UINT width, UINT height)
{
	auto device = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

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

	auto result = device->CreateCommittedResource(
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

	return 1;
}

void  D3DCamera::SetViewPort(UINT width, UINT height)
{
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
}

void D3DCamera::SetInitialRenderState(ID3D12GraphicsCommandList* cmdList)
{
	auto device = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;
	auto dsvh = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	//-------------render to normal BackBuffer-------------
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH;
	auto bbIdx = m_swapchain->GetCurrentBackBufferIndex();
	rtvH = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvh);

	cmdList->RSSetViewports(1, &m_viewport);
	cmdList->RSSetScissorRects(1, &m_scissorrect);
}

void D3DCamera::Clear()
{
	auto device = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;
	auto cmdList = D3DResourceManage::Instance().pGraphicsCard->pCmdList;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvH;
	auto dsvh = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	//if (m_mulPassRender == nullptr)
	{
		//-------------render to normal BackBuffer-------------
		auto bbIdx = m_swapchain->GetCurrentBackBufferIndex();
		Barrier(m_backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);

		rtvH = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvh);
	}
	//else
	//{
	//	//---------------render to mulpass render target-----------------
	//	rtvH = m_mulPassRender->m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	//	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvh);
	//	auto barrierRes = CD3DX12_RESOURCE_BARRIER::Transition(
	//		m_mulPassRender->m_peraResource.Get(),
	//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//		D3D12_RESOURCE_STATE_RENDER_TARGET);
	//	cmdList->ResourceBarrier(1, &barrierRes);
	//}

	cmdList->ClearRenderTargetView(rtvH, m_backColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvh, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	cmdList->RSSetViewports(1, &m_viewport);
	cmdList->RSSetScissorRects(1, &m_scissorrect);
}

void D3DCamera::Barrier(ID3D12Resource* resource,
	D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	auto cmdList = D3DResourceManage::Instance().pGraphicsCard->pCmdList;
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after, 0);
	cmdList->ResourceBarrier(1, &barrier);
}

void D3DCamera::Flip()
{
	auto cmdList = D3DResourceManage::Instance().pGraphicsCard->pCmdList;
	auto bbIdx = m_swapchain->GetCurrentBackBufferIndex();
	auto graphicsCard = D3DResourceManage::Instance().pGraphicsCard;

	Barrier(m_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	cmdList->Close();

	ID3D12CommandList* cmdlists[] = { cmdList };
	graphicsCard->pCmdQueue->ExecuteCommandLists(1, cmdlists);

	graphicsCard->WaitForCommandQueue();

	graphicsCard->pCmdAllocator->Reset();

	//cmdList->Reset(graphicsCard->pCmdAllocator, nullptr);

	for (auto iter = D3DResourceManage::Instance().PipelineTable.begin();
		iter != D3DResourceManage::Instance().PipelineTable.end(); iter++)
	{
		auto pipeline = iter->second;
		cmdList->Reset(D3DResourceManage::Instance().pGraphicsCard->pCmdAllocator, pipeline->m_pipelinestate);
	}

	//flip
	auto result = m_swapchain->Present(1, 0);
	if (FAILED(result))
	{
		_com_error err(result);
		//ShowMsgBox(nullptr, err.ErrorMessage());

		/*static int a = 0;
		if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
		{
			if (a == 0)
			{
				a = 1;
			}
			else
			{
				return;
			}

			HRESULT deviceRemovedReason = graphicsCard->pD3D12Device->GetDeviceRemovedReason();
			wchar_t  errorMessage[256];
			FormatMessageW(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				deviceRemovedReason,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				errorMessage,
				sizeof(errorMessage),
				NULL
			);
			
			ShowMsgBox(nullptr, errorMessage);
			std::ofstream file;
			file.open("abc.text", std::ios::out | std::ios::app);
			if (file.is_open())
			{
				file << errorMessage << std::endl;
				file.close();
			}
		}*/

		assert(SUCCEEDED(result));
	}
}

int D3DCamera::Draw(D3DDevice* _cD3DDev)
{
	auto cmdList = D3DResourceManage::Instance().pGraphicsCard->pCmdList;
	auto d3ddevice = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

	Clear();

	//-------------render each pipeline------------------
	for (auto iter = D3DResourceManage::Instance().PipelineTable.begin();
		iter != D3DResourceManage::Instance().PipelineTable.end(); iter++)
	{
		auto pipeline = iter->second;
		//SetInitialRenderState(cmdList);
		pipeline->Draw(cmdList, d3ddevice);
		//cmdList->Close();

	/*	ID3D12CommandList* cmdlists[] = { cmdList };
		_cD3DDev->pCmdQueue->ExecuteCommandLists(1, cmdlists);
		_cD3DDev->WaitForCommandQueue();

		cmdList->Reset(_cD3DDev->pCmdAllocator, nullptr);*/
	}

	Flip();

	return 1;
}

//void D3DCamera::InitMulPassRender()
//{
//	m_mulPassRender = new D3DMulPassRender();
//	m_mulPassRender->Init(this);
//	m_mulPassRender->CreatePeraPolygon();
//	m_mulPassRender->SetPipeline();
//}

void D3DCamera::Release()
{
	if (m_swapchain != nullptr)
	{
		m_swapchain->Release();
		m_swapchain = nullptr;
	}
	if (m_rtvHeap != nullptr)
	{
		m_rtvHeap->Release();
		m_rtvHeap = nullptr;
	}
	if (m_dsvHeap != nullptr)
	{
		m_dsvHeap->Release();
		m_dsvHeap = nullptr;
	}

	/*if (m_mulPassRender != nullptr)
	{
		delete(m_mulPassRender);
		m_mulPassRender = nullptr;
	}*/
}