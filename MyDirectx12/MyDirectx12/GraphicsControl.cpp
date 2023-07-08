#include"D3DFunction.h"
#include"PMDActor.h"
#include"D3DResourceManage.h"

using namespace std;

#pragma region InitD3dDevice

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall InitD3d(HWND);
#ifdef __cplusplus 
}
#endif


int __declspec(dllexport) __stdcall InitD3d(HWND hwnd)
{
	D3DDevice* p = new D3DDevice();
	p->InitializeDXGIDevice();
	p->InitializeCommand();
	p->InitializeFence();
	D3DResourceManage::Instance().pGraphicsCard = p;
	D3DResourceManage::Instance().Init();
	D3DResourceManage::Instance().InitializeLoadTable();
	

	return 1;
}

#pragma endregion

#pragma region ReleaseD3dDevice

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall ReleaseD3d(HWND);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall ReleaseD3d(HWND hwnd)
{
	return 0;
}

#pragma endregion

#pragma region Model

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall SetModel(unsigned long long _uid,const char* _FileFullName);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall SetModel(unsigned long long _uid,const char* _FileFullName)
{
	PMDModel* verRes = new PMDModel();
	int result = verRes->SetPMD(D3DResourceManage::Instance().pGraphicsCard, _FileFullName);
	if (result < 1)
	{
		return result;
	}

	if (result < 1)
	{
		return result;
	}
	
	auto iter = D3DResourceManage::Instance().PipelineModelTable->find("PmdStandard");

	iter->second->push_back(verRes);
	D3DResourceManage::Instance().UidModelTable->insert(
		pair<unsigned long long, PMDModel*>(_uid, verRes));

	return result;
}

//set Tansform
#ifdef __cplusplus 
extern"C"
{
#endif
void __declspec(dllexport) __stdcall SetModelTransform(unsigned long long _uid, DirectX::XMMATRIX matrix);
#ifdef __cplusplus 
}
#endif

void __declspec(dllexport) __stdcall SetModelTransform(unsigned long long _uid, DirectX::XMMATRIX matrix)
{
	auto iter = D3DResourceManage::Instance().UidModelTable->find(_uid);
	if (iter == D3DResourceManage::Instance().UidModelTable->end())
	{
		PrintDebug("SetModelTransform fault, can't find Entity.");
		return;
	}
	iter->second->m_transform.world = matrix;
}

//load Animation
#ifdef __cplusplus 
extern"C"
{
#endif
	void __declspec(dllexport) __stdcall LoadAnimation(unsigned long long _uid, const char* path);
#ifdef __cplusplus 
}
#endif

void __declspec(dllexport) __stdcall LoadAnimation(unsigned long long _uid, const char* path)
{
	auto iter = D3DResourceManage::Instance().UidModelTable->find(_uid);
	if (iter == D3DResourceManage::Instance().UidModelTable->end())
	{
		PrintDebug("LoadAnimation fault, can't find Entity.");
		return;
	}
	iter->second->LoadAnimation(path);
}

//Update Animation
#ifdef __cplusplus 
extern"C"
{
#endif
	void __declspec(dllexport) __stdcall UpdateAnimation(unsigned long long _uid);
#ifdef __cplusplus 
}
#endif

void __declspec(dllexport) __stdcall UpdateAnimation(unsigned long long _uid)
{
	auto iter = D3DResourceManage::Instance().UidModelTable->find(_uid);
	if (iter == D3DResourceManage::Instance().UidModelTable->end())
	{
		PrintDebug("LoadAnimation fault, can't find Entity.");
		return;
	}
	iter->second->UpdateAnimation();
}

#pragma endregion

#pragma region SetPmdStandardPipeline

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall SetPmdStandardPipeline();
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall SetPmdStandardPipeline()
{
	auto iter = D3DResourceManage::Instance().PipelineTable.find("PmdStandard");
	if (iter != D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("Already exist pipeline PmdStandard.");
		return -1;
	}
	D3DPipeline* pipeline = new D3DPipeline("PmdStandard");
	int result = pipeline->SetPipeline(D3DResourceManage::Instance().pGraphicsCard);
	pipeline->CreateSceneView(D3DResourceManage::Instance().pGraphicsCard);
	D3DResourceManage::Instance().PipelineTable.insert(
		pair< const char*, D3DPipeline*>("PmdStandard", pipeline));
	return result;
}

#pragma endregion

#pragma region Camera

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall CreateRenderTarget(HWND hwnd,unsigned long long uid, UINT width, UINT height);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall CreateRenderTarget(HWND hwnd,unsigned long long uid, UINT width, UINT height)
{
	D3DCamera* mainCamera = new D3DCamera();
	mainCamera->CreateSwapChain(hwnd,width,height);
	mainCamera->CreateRenderTargetView();
	mainCamera->CreateDepthStencilView(width, height);
	mainCamera->SetViewPort(width, height);
	mainCamera->Clear();

	D3DResourceManage::Instance().CameraTable.insert(
		pair<unsigned long long, D3DCamera*>(uid, mainCamera));

	return 1;
}

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall Render(unsigned long long uid);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall Render(unsigned long long uid)
{

	D3DResourceManage::Instance().CameraTable[uid]->
		Draw(D3DResourceManage::Instance().pGraphicsCard);

	return 1;
}

#ifdef __cplusplus 
extern"C"
{
#endif
	void __declspec(dllexport) __stdcall SetCameraTransform(
		DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up);
#ifdef __cplusplus 
}
#endif
void __declspec(dllexport) __stdcall SetCameraTransform(
	DirectX::XMFLOAT3 eye, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up)
{
	auto iter = D3DResourceManage::Instance().PipelineTable.find("PmdStandard");
	if (iter != D3DResourceManage::Instance().PipelineTable.end())
	{
		iter->second->SetCameraTransform(eye, target, up);
	}
}

#ifdef __cplusplus 
extern"C"
{
#endif
	void __declspec(dllexport) __stdcall SetCameraProjection(float FovAngleY,  //View angle
	float AspectRatio,      //width : height
	float NearZ,
	float Far);
#ifdef __cplusplus 
}
#endif
void __declspec(dllexport) __stdcall SetCameraProjection(float FovAngleY, float AspectRatio, float NearZ,float Far)
{
	auto iter = D3DResourceManage::Instance().PipelineTable.find("PmdStandard");
	if (iter != D3DResourceManage::Instance().PipelineTable.end())
	{
		iter->second->SetCameraProjection(FovAngleY, AspectRatio, NearZ, Far);
	}
}

#pragma endregion

