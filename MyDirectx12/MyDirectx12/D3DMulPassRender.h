#pragma once

#include"D3DAPI.h"
#include"D3DFunction.h"

class D3DMulPassRender
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_peraResource;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_peraRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_peraSRVHeap;


public:

	void Init(D3DCamera* camera);
};