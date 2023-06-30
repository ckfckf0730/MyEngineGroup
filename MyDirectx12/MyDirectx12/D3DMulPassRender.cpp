#include "D3DMulPassRender.h"
#include"D3DResourceManage.h"

using namespace DirectX;

void D3DMulPassRender::Init(D3DCamera* camera)
{
	auto heapDesc = camera->m_rtvHeap->GetDesc();

	auto& bbuff = camera->m_backBuffers[0];
	auto resDesc = bbuff->GetDesc();

	D3D12_HEAP_PROPERTIES heapProp = 
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	float clsClr[4] = { 0.5f,0.5f,0.5f,1.0f };
	D3D12_CLEAR_VALUE clearValue = 
		CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clsClr);

	auto resual = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device->
		CreateCommittedResource(
			&heapProp, D3D12_HEAP_FLAG_NONE,
			&resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue, IID_PPV_ARGS(m_peraResource.ReleaseAndGetAddressOf()));

}