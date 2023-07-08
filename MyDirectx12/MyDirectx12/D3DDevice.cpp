#include"D3DFunction.h"
#include <synchapi.h>
#include"D3DResourceManage.h"

//BOOL WINAPI DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved)
//{
//	return TRUE;
//}

using namespace std;

int D3DDevice::InitializeDXGIDevice() 
{
	D3D_FEATURE_LEVEL levels[] =
	{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	};
//#ifdef _DEBUG
//	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory))))
//	{
//		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory))))
//		{
//			PrintDebug(L"Create dxgiFactory fault.");
//			return -1;
//		}
//	}
//#else 
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory))))
	{
		return -1;
	}
//#endif // _DEBUG

	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; pDxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&pD3D12Device)) == S_OK)
		{
			featureLevel = l;
			break;
		}
	}
	return 1;
}

int D3DDevice::InitializeCommand()
{
	HRESULT result = S_OK;
	result = pD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(
		&pCmdAllocator));
	result = pD3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		pCmdAllocator, nullptr, IID_PPV_ARGS(&pCmdList));

	//_cmdList->Close();
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//プライオリティ特に指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//ここはコマンドリストと合わせてください
	result = pD3D12Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&pCmdQueue));//コマンドキュー生成

	return 1;
}



int	D3DDevice::InitializeFence()
{
	auto result = pD3D12Device->CreateFence(m_fenceVal,
		D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

	return 1;
}


void D3DDevice::WaitForCommandQueue()
{
	pCmdQueue->Signal(
		m_fence, ++m_fenceVal);

	if (m_fence->GetCompletedValue() <m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
}



