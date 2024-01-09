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
	int __declspec(dllexport) __stdcall SetPMDModel(unsigned long long _uid,const char* _FileFullName);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall SetPMDModel(unsigned long long _uid,const char* _FileFullName)
{
	auto iter = BasicModel::s_modelTable.find(std::string(_FileFullName));
	PMDModel* verRes = nullptr;
	int result = -1;
	if (iter == BasicModel::s_modelTable.end())
	{
		//------------create model from file------------------------
		verRes = new PMDModel();
		result = verRes->SetPMD(D3DResourceManage::Instance().pGraphicsCard, _FileFullName);
		if (result < 1)
		{
			return result;
		}

		if (result < 1)
		{
			return result;
		}

		auto iter2 = D3DResourceManage::Instance().PipelineModelTable->find("PmdStandard");

		iter2->second->push_back(static_cast<BasicModel*>(verRes));
	}
	else
	{
		verRes = static_cast<PMDModel*>(iter->second);
		result = 1;
		ShowMsgBox(L"error", L"find exist model.");
	}

	

	//------------create instance------------
	PMDModelInstance* instance = new PMDModelInstance();
	instance->m_model = verRes;
	instance->BindAnimation(verRes->m_animation);
	instance->CreateTransformView(D3DResourceManage::Instance().pGraphicsCard);

	verRes->m_instances.push_back(static_cast<ModelInstance*>(instance));
	ModelInstance::s_uidModelTable.insert(
		pair<unsigned long long, ModelInstance*>(_uid, static_cast<ModelInstance*>(instance)));

	return result;
}

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall SetBasicModel(unsigned long long _uid, const char* _FileFullName);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall SetBasicModel(unsigned long long _uid, const char* _FileFullName)
{
	auto iter = BasicModel::s_modelTable.find(std::string(_FileFullName));
	BasicModel* verRes = nullptr;
	int result = -1;
	if (iter == BasicModel::s_modelTable.end())
	{
		//------------create model from file------------------------
		verRes = new BasicModel();
		int result = verRes->SetBasicModel(D3DResourceManage::Instance().pGraphicsCard, _FileFullName);
		verRes->InitMaterial();
		if (result < 1)
		{
			return result;
		}

		if (result < 1)
		{
			return result;
		}

		auto iter2 = D3DResourceManage::Instance().PipelineModelTable->find("NoboneStandard");
		iter2->second->push_back(verRes);
	}
	else
	{
		verRes = iter->second;
		result = 1;
		ShowMsgBox(L"error", L"find exist model.");
	}

	//------------create instance------------
	ModelInstance* instance = new ModelInstance();
	instance->m_model = verRes;
	instance->CreateTransformView(D3DResourceManage::Instance().pGraphicsCard);

	verRes->m_instances.push_back(instance);
	ModelInstance::s_uidModelTable.insert(
		pair<unsigned long long, ModelInstance*>(_uid, instance));

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
	auto iter = ModelInstance::s_uidModelTable.find(_uid);
	if (iter == ModelInstance::s_uidModelTable.end())
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
	auto iter = ModelInstance::s_uidModelTable.find(_uid);
	if (iter == ModelInstance::s_uidModelTable.end())
	{
		PrintDebug("LoadAnimation fault, can't find Entity.");
		return;
	}
	PMDModelInstance* pInstance = static_cast<PMDModelInstance*>(iter->second);
	pInstance->m_animation->LoadVMDFile(path, static_cast<PMDModel*>(pInstance->m_model));
	pInstance->m_animation->StartAnimation();
	//static_cast<PMDModel*>(iter->second)->LoadAnimation(path);
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
	auto iter = ModelInstance::s_uidModelTable.find(_uid);
	if (iter == ModelInstance::s_uidModelTable.end())
	{
		PrintDebug("LoadAnimation fault, can't find Entity.");
		return;
	}
	static_cast<PMDModelInstance*>(iter->second)->m_animation->UpdateAnimation();
	//static_cast<PMDModel*>(iter->second)->UpdateAnimation();
}

#pragma endregion

#pragma region SetStandardPipeline

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
	//-------------------create PmdStandard pipeline------------------------------------------------
	auto iter = D3DResourceManage::Instance().PipelineTable.find("PmdStandard");
	if (iter != D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("Already exist pipeline PmdStandard.");
		return -1;
	}
	D3DPipeline* pipeline = new D3DPipeline("PmdStandard");
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		/*{"EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}*/
	};
	UINT numElements = _countof(inputLayout);
	int result = pipeline->SetPipeline(D3DResourceManage::Instance().pGraphicsCard, inputLayout, numElements,
		L"BasicVertexShader.hlsl", L"BasicPixelShader.hlsl");
	pipeline->CreateSceneView(D3DResourceManage::Instance().pGraphicsCard);
	D3DResourceManage::Instance().PipelineTable.insert(
		pair< const char*, D3DPipeline*>("PmdStandard", pipeline));

	if (result != 1)
	{
		ShowMsgBox(L"Error", L"Create PmdStandard fault.");
		return result;
	}

	//-------------------create NoboneStandard pipeline------------------------------------------------
	iter = D3DResourceManage::Instance().PipelineTable.find("NoboneStandard");
	if (iter != D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("Already exist pipeline NoboneStandard.");
		return -1;
	}
	pipeline = new D3DPipeline("NoboneStandard");
	D3D12_INPUT_ELEMENT_DESC inputLayout2[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};
	numElements = _countof(inputLayout2);
	result = pipeline->SetPipeline(D3DResourceManage::Instance().pGraphicsCard, inputLayout2, numElements,
		L"NoBoneVertexShader.hlsl", L"BasicPixelShader.hlsl");
	pipeline->CreateSceneView(D3DResourceManage::Instance().pGraphicsCard);
	D3DResourceManage::Instance().PipelineTable.insert(
		pair< const char*, D3DPipeline*>("NoboneStandard", pipeline));

	if (result != 1)
	{
		ShowMsgBox(L"Error", L"Create NoboneStandard fault.");
		return result;
	}

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
	//mainCamera->InitMulPassRender();
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

