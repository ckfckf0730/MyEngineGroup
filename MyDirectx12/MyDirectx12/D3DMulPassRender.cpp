#include "D3DMulPassRender.h"
#include"D3DResourceManage.h"

using namespace DirectX;

void D3DMulPassRender::Init(D3DCamera* camera)
{
	//the resource created is a render target one, and at the same time, it's also as a shader resource.
	m_bindCamera = camera;
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

void D3DMulPassRender::Draw()
{
	auto rtvHeapPointer = m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	auto _cmdList = D3DResourceManage::Instance().pGraphicsCard->pCmdList;

	auto dsvHeapHandle = m_bindCamera->m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(
		1,
		&rtvHeapPointer,
		false,
		&dsvHeapHandle);

	D3D12_RESOURCE_BARRIER barrierDesc;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.pResource = m_peraResource.Get();
	_cmdList->ResourceBarrier(1, &barrierDesc);

	_cmdList->SetGraphicsRootSignature(m_peraRootSign.Get());
	_cmdList->SetPipelineState(m_peraPipeline.Get());
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_cmdList->IASetVertexBuffers(0, 1, &m_peraVBV);
	_cmdList->DrawInstanced(4, 1, 0, 0);
}

void D3DMulPassRender::CreatePeraPolygon()
{
	PeraVertex pv[4] =
	{
		{{-1.0f, -1.0f, 0.1f}, {0.0f, 1.0f}},	//left bottom
		{{-1.0f, 1.0f, 0.1f}, {0.0f, 0.0f}},	//left top
		{{1.0f, -1.0f, 0.1f}, {1.0f, 1.0f}},	//right bottom
		{{1.0f, 1.0f, 0.1f}, {1.0f, 0.0f}}		//right top
	};

	auto _dev = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

	auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv));
	auto result = _dev->CreateCommittedResource(
		&heap,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_peraVertexBuffer.ReleaseAndGetAddressOf()));

	m_peraVBV.BufferLocation = m_peraVertexBuffer->GetGPUVirtualAddress();
	m_peraVBV.SizeInBytes = sizeof(pv);
	m_peraVBV.StrideInBytes = sizeof(PeraVertex);
	PeraVertex* mappedPera = nullptr;
	m_peraVertexBuffer->Map(0, nullptr, (void**)&mappedPera);
	std::copy(std::begin(pv), std::end(pv), mappedPera);
	m_peraVertexBuffer->Unmap(0, nullptr);
}

void D3DMulPassRender::SetPipeline()
{
	auto _dev = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	Microsoft::WRL::ComPtr<ID3DBlob> vs;
	Microsoft::WRL::ComPtr<ID3DBlob> ps;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	auto result = D3DCompileFromFile(
		L"PeraVertex.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs", "vs_5_0", 0, 0,
		vs.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"create MulpassRender vs Blob fault.");
	}

	result = D3DCompileFromFile(
		L"PeraPixel.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps", "ps_5_0", 0, 0,
		ps.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"create MulpassRender ps Blob fault.");
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = 0;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	Microsoft::WRL::ComPtr<ID3DBlob> rsBlob;

	result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"create MulpassRender root signature blob fault.");
	}

	result = _dev->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(m_peraRootSign.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"create MulpassRender root signature fault.");
	}
	
	gpsDesc.pRootSignature = m_peraRootSign.Get();
	result = _dev->CreateGraphicsPipelineState(
		&gpsDesc,
		IID_PPV_ARGS(m_peraPipeline.ReleaseAndGetAddressOf()));

}