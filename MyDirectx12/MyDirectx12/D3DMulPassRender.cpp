#include "D3DMulPassRender.h"
#include"D3DResourceManage.h"

using namespace DirectX;

void D3DMulPassRender::Init(D3DCamera* camera)
{
	//the resource created is a render target one, and at the same time, it's also as a shader resource.

	auto _pDev = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

	auto& bbuff = camera->m_backBuffers[0];
	auto resDesc = bbuff->GetDesc();

	D3D12_HEAP_PROPERTIES heapProp = 
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	float clsClr[4] = { 0.5f,0.5f,0.5f,1.0f };
	D3D12_CLEAR_VALUE clearValue = 
		CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	auto result = _pDev->CreateCommittedResource(
			&heapProp, D3D12_HEAP_FLAG_NONE,
			&resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue, IID_PPV_ARGS(m_peraResource.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", "Create MulPass resource fault");
	}

	auto heapDesc = camera->m_rtvHeap->GetDesc();  //it's a copy
	heapDesc.NumDescriptors = 1;
	result = _pDev->CreateDescriptorHeap(&heapDesc,
		IID_PPV_ARGS(m_peraRTVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", "Create MulPass RTVHeap fault");
	}

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	_pDev->CreateRenderTargetView(
		m_peraResource.Get(),
		&rtvDesc,
		m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());

	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = _pDev->CreateDescriptorHeap(
		&heapDesc, 
		IID_PPV_ARGS(m_peraSRVHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", "Create MulPass SRVHeap fault");
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = rtvDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	_pDev->CreateShaderResourceView(
		m_peraResource.Get(),
		&srvDesc,
		m_peraSRVHeap->GetCPUDescriptorHandleForHeapStart());

}