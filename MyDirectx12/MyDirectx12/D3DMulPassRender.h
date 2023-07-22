#pragma once

#include"D3DAPI.h"
//#include"D3DFunction.h"

class D3DCamera;

struct PeraVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};

class D3DMulPassRender
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_peraResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_peraRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_peraSRVHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_peraVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_peraVBV;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_peraRootSign;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_peraPipeline;

	D3DCamera* m_bindCamera;

public:

	void Init(D3DCamera* camera);


	void CreatePeraPolygon();

	void Draw();

	void SetPipeline();
};


