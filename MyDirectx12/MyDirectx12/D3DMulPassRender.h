#pragma once

#include"D3DAPI.h"
#include"D3DFunction.h"

struct PeraVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};

class D3DMulPassRender
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_peraResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_peraRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_peraSRVHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_peraVB;

	D3DCamera* m_bindCamera;

public:

	void Init(D3DCamera* camera);
	void CreatePeraPolygon();


	void Draw();
};


