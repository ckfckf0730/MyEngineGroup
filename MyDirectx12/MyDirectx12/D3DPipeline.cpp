#include"D3DFunction.h"
#include"D3DResourceManage.h"
using namespace DirectX;

D3DPipeline::D3DPipeline(const char* name)
{
	m_name = name;
}

int D3DPipeline::SetPipeline(D3DDevice* _cD3DDev,D3D12_INPUT_ELEMENT_DESC inputLayout[],UINT numElements,
	LPCWSTR vsShader, LPCWSTR psShader)
{
	//hlsl compile
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	HRESULT result = D3DCompileFromFile(
		vsShader,
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, vsBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"hlsl compile fault.");
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			ShowMsgBox(L"Error", L"Can't find hlsl file.");

		}
		else
		{

		}
		return -1;
	}

	result = D3DCompileFromFile(
		psShader,
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, psBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"hlsl compile fault.");
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			ShowMsgBox(L"Error", L"Can't find hlsl file.");
		}
		else
		{

		}
		return -1;
	}

	//create vertex layout (tell pipelien the vertex struct)
	//D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	//{
	//	{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	//	{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	//	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	//	{ "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	//	{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	//	/*{"EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}*/
	//};

	//create graphics pipeline
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipelineDesc = {};
	gpipelineDesc.pRootSignature = nullptr;
	gpipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	gpipelineDesc.HS.BytecodeLength = 0;
	gpipelineDesc.HS.pShaderBytecode = nullptr;
	gpipelineDesc.DS.BytecodeLength = 0;
	gpipelineDesc.DS.pShaderBytecode = nullptr;
	gpipelineDesc.GS.BytecodeLength = 0;
	gpipelineDesc.GS.pShaderBytecode = nullptr;

	gpipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//gpipelineDesc.SampleMask = 0xffffffff;//‘S•”‘ÎÛ

	gpipelineDesc.BlendState.AlphaToCoverageEnable = false;
	gpipelineDesc.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	renderTargetBlendDesc.LogicOpEnable = false;

	gpipelineDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	//gpipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipelineDesc.RasterizerState.MultisampleEnable = false;
	gpipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpipelineDesc.RasterizerState.DepthClipEnable = true;

	gpipelineDesc.RasterizerState.FrontCounterClockwise = false;
	gpipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipelineDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipelineDesc.RasterizerState.AntialiasedLineEnable = false;
	gpipelineDesc.RasterizerState.ForcedSampleCount = 0;
	gpipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//depth test
	gpipelineDesc.DepthStencilState.DepthEnable = true;
	gpipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	/*gpipelineDesc.BlendState.RenderTarget->BlendEnable = true;
	gpipelineDesc.BlendState.RenderTarget->SrcBlend = D3D12_BLEND_SRC_ALPHA;
	gpipelineDesc.BlendState.RenderTarget->DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	gpipelineDesc.BlendState.RenderTarget->BlendOp = D3D12_BLEND_OP_ADD;*/

	gpipelineDesc.DepthStencilState.StencilEnable = false;

	//gpipeline.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	gpipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	gpipelineDesc.InputLayout.NumElements = numElements;

	gpipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gpipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipelineDesc.NumRenderTargets = 1;
	gpipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	//gpipeline.NodeMask = 0;
	gpipelineDesc.SampleDesc.Count = 1;
	gpipelineDesc.SampleDesc.Quality = 0;


	//create root signature
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRange[4] = {};
	descTblRange[0].NumDescriptors = 1;
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[0].BaseShaderRegister = 0;
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descTblRange[1].NumDescriptors = 1;
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[1].BaseShaderRegister = 1;
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descTblRange[2].NumDescriptors = 1;
	descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[2].BaseShaderRegister = 2;
	descTblRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descTblRange[3].NumDescriptors = 4;
	descTblRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange[3].BaseShaderRegister = 0;
	descTblRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparam[3] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];
	rootparam[1].DescriptorTable.NumDescriptorRanges = 1;

	rootparam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootparam[2].DescriptorTable.pDescriptorRanges = &descTblRange[2];
	rootparam[2].DescriptorTable.NumDescriptorRanges = 2;

	rootSignatureDesc.pParameters = rootparam;
	rootSignatureDesc.NumParameters = 3;

	D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc[0].MinLOD = 0.0f;
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDesc[1] = samplerDesc[0];
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ShaderRegister = 1;

	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"create root signature Blob fault.");
		return -1;
	}

	result = _cD3DDev->pD3D12Device->CreateRootSignature(
		0, rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_rootsignature));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"create root signature fault.");
		return -1;
	}
	rootSigBlob->Release();

	gpipelineDesc.pRootSignature = m_rootsignature;

	result = _cD3DDev->pD3D12Device->CreateGraphicsPipelineState(
		&gpipelineDesc, IID_PPV_ARGS(&m_pipelinestate));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"create graphics pipeline fault. ");
		return -1;
	}


	

	return 1;
}

int D3DPipeline::CreateSceneView(D3DDevice* _cD3DDev)
{
	//create const buff
	XMFLOAT3 eye(0, -10, -10);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);

	XMMATRIX viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	XMMATRIX projMat = XMMatrixPerspectiveFovLH(XM_PIDIV2, //View is 90 degree angle
		static_cast<float>(800) / static_cast<float>(800),
		1.0f,  //nearest
		100.0f);  //farthest

	////2D screen coord transform
	//matrix.r[0].m128_f32[0] = 2.0f / window_width;
	//matrix.r[1].m128_f32[1] = -2.0f / window_height;
	//matrix.r[3].m128_f32[0] = -1.0f;
	//matrix.r[3].m128_f32[1] = 1.0f;

	auto constBuffHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto constBuffDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneMatrix) + 0xff) & ~0xff);
	auto result = _cD3DDev->pD3D12Device->CreateCommittedResource(
		&constBuffHeap,
		D3D12_HEAP_FLAG_NONE,
		&constBuffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_sceneConstBuff));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Create scene const buff fault.");
		return -1;
	}

	result = m_sceneConstBuff->Map(0, nullptr, (void**)&m_mapSceneMatrix);
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Map scene const buff fault.");
		return -1;
	}

	m_mapSceneMatrix->view = viewMat;
	m_mapSceneMatrix->proj = projMat;
	m_mapSceneMatrix->eye = eye;


	//Create basic(matrix) Buff View
	D3D12_DESCRIPTOR_HEAP_DESC basicHeapDesc = {};
	basicHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	basicHeapDesc.NodeMask = 0;
	basicHeapDesc.NumDescriptors = 1;//
	basicHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = _cD3DDev->pD3D12Device->CreateDescriptorHeap(
		&basicHeapDesc, IID_PPV_ARGS(&m_sceneDescHeap));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Create scene const heap fault.");
		return -1;
	}

	auto basicHeapHandle = m_sceneDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbcDesc = {};
	cbcDesc.BufferLocation = m_sceneConstBuff->GetGPUVirtualAddress();
	cbcDesc.SizeInBytes = m_sceneConstBuff->GetDesc().Width;

	_cD3DDev->pD3D12Device->CreateConstantBufferView(&cbcDesc, basicHeapHandle);

	return 1;
}

void D3DPipeline::SetCameraTransform(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up)
{
	XMMATRIX viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	m_mapSceneMatrix->view = viewMat;
	m_mapSceneMatrix->eye = eye;
}

void D3DPipeline::SetCameraProjection(float FovAngleY, float AspectRatio, float NearZ, float Far)
{
	XMMATRIX projMat = XMMatrixPerspectiveFovLH(FovAngleY,
		AspectRatio,
		NearZ,  //nearest
		Far);  //farthest
}

void D3DPipeline::Draw(ID3D12GraphicsCommandList* _cmdList, ID3D12Device* d3ddevice)
{
	_cmdList->SetPipelineState(m_pipelinestate);
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_cmdList->SetGraphicsRootSignature(m_rootsignature);

	auto models = (*D3DResourceManage::Instance().PipelineModelTable)[m_name];
	if (models == nullptr)
	{
		return;
	}

	for (auto& vertices : *models)
	{
		
		vertices->m_mapMatrices[0] = vertices->m_transform.world;

		_cmdList->IASetVertexBuffers(0, 1, &vertices->m_vbView);
		_cmdList->IASetIndexBuffer(&vertices->m_ibView);

		//--------------set const buff and texture buff heap-------
		_cmdList->SetDescriptorHeaps(1, &m_sceneDescHeap);
		_cmdList->SetGraphicsRootDescriptorTable(0,
			m_sceneDescHeap->GetGPUDescriptorHandleForHeapStart());

		_cmdList->SetDescriptorHeaps(1, &vertices->m_transformDescHeap);
		_cmdList->SetGraphicsRootDescriptorTable(1,
			vertices->m_transformDescHeap->GetGPUDescriptorHandleForHeapStart());

		_cmdList->SetDescriptorHeaps(1, &vertices->m_materialDescHeap);
		auto cbvsrvIncSize = d3ddevice->
			GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;
		auto materialH = vertices->m_materialDescHeap->GetGPUDescriptorHandleForHeapStart();
		unsigned int idxOffset = 0;

		for (auto& m : vertices->m_materials)
		{
			_cmdList->SetGraphicsRootDescriptorTable(2, materialH);

			_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);

			materialH.ptr += cbvsrvIncSize;
			idxOffset += m.indicesNum;
		}
	}


	

	
}