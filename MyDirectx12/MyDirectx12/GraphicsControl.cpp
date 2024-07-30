#include"D3DFunction.h"
#include"PMDActor.h"
#include"D3DResourceManage.h"
#include <stdexcept>

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
	int __declspec(dllexport) __stdcall DeleteModelInstance(unsigned long long _uid);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall DeleteModelInstance(unsigned long long _uid)
{
	auto iter = ModelInstance::s_uidModelTable.find(_uid);
	if (iter != ModelInstance::s_uidModelTable.end())
	{
		auto instance = (*iter).second;

		auto& list = instance->m_model->m_instances;
		auto iter2 = std::find(list.begin(), list.end(), instance);

		list.erase(iter2);

		delete instance;
		ModelInstance::s_uidModelTable.erase(iter);
		return 1;
	}
	return -1;
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
	int __declspec(dllexport) __stdcall LoadAnimation(unsigned long long _uid, const char* path);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall LoadAnimation(unsigned long long _uid, const char* path)
{
	auto iter = ModelInstance::s_uidModelTable.find(_uid);
	if (iter == ModelInstance::s_uidModelTable.end())
	{
		PrintDebug("LoadAnimation fault, can't find Entity.");
		return -1;
	}
	PMDModelInstance* pInstance = static_cast<PMDModelInstance*>(iter->second);
	PrintDebug("Begin LoadVMDFile:");
	PrintDebug(path);
	auto animation = D3DAnimation::LoadVMDFile(path);
	if (animation != nullptr)
	{
		PrintDebug("LoadAnimation success:");
		PrintDebug(path);
		pInstance->InitAnimation(animation);
		pInstance->m_animationInstance->StartAnimation();
	}

	return 1;
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

	auto animationIns = static_cast<PMDModelInstance*>(iter->second)->m_animationInstance;
	if (animationIns != nullptr)
	{

		animationIns->UpdateAnimation();
	}
}

#pragma endregion

#pragma region SetStandardPipeline

extern"C"
{
	int __declspec(dllexport) __stdcall CreateBonePipeline(LPCSTR pipelineName,
		LPCSTR vsCode, LPCSTR vsEntry, LPCSTR psCode, LPCSTR psEntry);
}

int __declspec(dllexport) __stdcall CreateBonePipeline(LPCSTR pipelineName,
	LPCSTR vsCode, LPCSTR vsEntry, LPCSTR psCode, LPCSTR psEntry)
{
	auto iter = D3DResourceManage::Instance().PipelineTable.find(pipelineName);
	if (iter != D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("Already exist pipeline name:");
		PrintDebug(pipelineName);
		return -1;
	}
	D3DPipeline* pipeline = new D3DPipeline(pipelineName);
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};
	UINT numElements = _countof(inputLayout);
	int result = pipeline->CreatePipeline(D3DResourceManage::Instance().pGraphicsCard, inputLayout, numElements,
		vsCode, vsEntry, psCode, psEntry);
	D3DResourceManage::Instance().PipelineTable.insert(
		pair<std::string, D3DPipeline*>(pipelineName, pipeline));


	if (result != 1)
	{
		ShowMsgBox(L"Error", L"Create pipeline fault:");
		return result;
	}

	return result;
}

extern"C"
{
	int __declspec(dllexport) __stdcall CreateNoBonePipeline(LPCSTR pipelineName,
		LPCSTR vsCode, LPCSTR vsEntry, LPCSTR psCode, LPCSTR psEntry);
}

int __declspec(dllexport) __stdcall CreateNoBonePipeline(LPCSTR pipelineName,
	LPCSTR vsCode, LPCSTR vsEntry, LPCSTR psCode, LPCSTR psEntry)
{
	auto iter = D3DResourceManage::Instance().PipelineTable.find(pipelineName);
	if (iter != D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("Already exist pipeline name:");
		PrintDebug(pipelineName);
		return -1;
	}
	D3DPipeline* pipeline = new D3DPipeline(pipelineName);
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
				{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
				{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
				{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};
	UINT numElements = _countof(inputLayout);
	int result = pipeline->CreatePipeline(D3DResourceManage::Instance().pGraphicsCard, inputLayout, numElements,
		vsCode, vsEntry, psCode, psEntry);
	D3DResourceManage::Instance().PipelineTable.insert(
		pair<std::string, D3DPipeline*>(pipelineName, pipeline));

	if (result != 1)
	{
		ShowMsgBox(L"Error", L"Create pipeline fault:");
		return result;
	}

	return result;
}

extern"C"
{void __declspec(dllexport) __stdcall ClearRootSignatureSetting(); }

void __declspec(dllexport) __stdcall ClearRootSignatureSetting()
{
	D3DResourceManage::Instance().RootSignatureSetting.clear();
}

extern"C"
{
	void __declspec(dllexport) __stdcall SetRootSignature(LPCSTR name,
		D3D12_DESCRIPTOR_RANGE_TYPE type, int baseShaderRegister, D3D12_SHADER_VISIBILITY visibility);
}

void __declspec(dllexport) __stdcall SetRootSignature(LPCSTR name,
	D3D12_DESCRIPTOR_RANGE_TYPE type, int baseShaderRegister, D3D12_SHADER_VISIBILITY visibility)
{
	D3D12_DESCRIPTOR_RANGE desc = {};
	desc.NumDescriptors = 1;  //testing phase just 1 each time
	desc.RangeType = type;
	desc.BaseShaderRegister = baseShaderRegister;
	desc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3DResourceManage::Instance().RootSignatureSetting.push_back(RootSignatureSetting{ name, desc  , visibility });
}

//extern"C"
//{
//	int __declspec(dllexport) __stdcall SetPmdStandardPipeline();
//}
//
//int __declspec(dllexport) __stdcall SetPmdStandardPipeline()
//{
//	////-------------------create PmdStandard pipeline------------------------------------------------
//	//auto iter = D3DResourceManage::Instance().PipelineTable.find("PmdStandard");
//	//if (iter != D3DResourceManage::Instance().PipelineTable.end())
//	//{
//	//	PrintDebug("Already exist pipeline PmdStandard.");
//	//	return -1;
//	//}
//	//D3DPipeline* pipeline = new D3DPipeline("PmdStandard");
//	//D3D12_INPUT_ELEMENT_DESC inputLayout[] =
//	//{
//	//	{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//	//	{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//	//	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//	//	{ "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//	//	{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//	//	/*{"EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}*/
//	//};
//	//UINT numElements = _countof(inputLayout);
//	//int result = pipeline->SetPipeline(D3DResourceManage::Instance().pGraphicsCard, inputLayout, numElements,
//	//	L"BasicVertexShader.hlsl", L"BasicPixelShader.hlsl");
//	//pipeline->CreateSceneView(D3DResourceManage::Instance().pGraphicsCard);
//	//D3DResourceManage::Instance().PipelineTable.insert(
//	//	pair< const char*, D3DPipeline*>("PmdStandard", pipeline));
//
//	//if (result != 1)
//	//{
//	//	ShowMsgBox(L"Error", L"Create PmdStandard fault.");
//	//	return result;
//	//}
//
//	//-------------------create NoboneStandard pipeline------------------------------------------------
//	auto iter = D3DResourceManage::Instance().PipelineTable.find("NoboneStandard");
//	if (iter != D3DResourceManage::Instance().PipelineTable.end())
//	{
//		PrintDebug("Already exist pipeline NoboneStandard.");
//		return -1;
//	}
//
//	D3DPipeline* pipeline = new D3DPipeline("NoboneStandard");
//	D3D12_INPUT_ELEMENT_DESC inputLayout2[] =
//	{
//		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
//	};
//
//	UINT numElements = _countof(inputLayout2);
//	int result = pipeline->SetPipeline(D3DResourceManage::Instance().pGraphicsCard, inputLayout2, numElements,
//		L"NoBoneVertexShader.hlsl", L"BasicPixelShader.hlsl");
//	pipeline->CreateSceneView(D3DResourceManage::Instance().pGraphicsCard);
//	D3DResourceManage::Instance().PipelineTable.insert(
//		pair< std::string, D3DPipeline*>("NoboneStandard", pipeline));
//	if (result != 1)
//	{
//		ShowMsgBox(L"Error", L"Create NoboneStandard fault.");
//		return result;
//	}
//
//	return result;
//}

#pragma endregion

#pragma region Camera

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall CreateRenderTarget(HWND hwnd, UINT width, UINT height, UINT64& resPoint);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall CreateRenderTarget(HWND hwnd, UINT width, UINT height, UINT64& resPoint)
{
	/*auto iter = D3DResourceManage::Instance().CameraTable.find(uid);
	if (iter != D3DResourceManage::Instance().CameraTable.end())
	{
		iter->second->Release();
		delete (iter->second);
		D3DResourceManage::Instance().CameraTable.erase(iter);
	}*/

	D3DCamera* camera = new D3DCamera();
	resPoint = reinterpret_cast<UINT64>(camera);
	//mainCamera->Uid = uid;
	camera->CreateSwapChain(hwnd, width, height);
	camera->CreateRenderTargetView();
	camera->CreateDepthStencilView(width, height);
	camera->SetViewPort(width, height);
	//mainCamera->InitMulPassRender();
	camera->Clear();

	//D3DResourceManage::Instance().CameraTable.insert(
	//	pair<unsigned long long, D3DCamera*>(uid, mainCamera));

	return 1;
}

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall DeleteRenderTarget(UINT64 resPoint);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall DeleteRenderTarget(UINT64 resPoint)
{
	/*auto iter = D3DResourceManage::Instance().CameraTable.find(uid);
	if (iter == D3DResourceManage::Instance().CameraTable.end())
	{
		PrintDebug("DeleteRenderTarget Error, Cant find camera UID:");
		PrintDebug((int)uid);
		return -1;
	}*/

	//if (resPoint == 0)
	//{
	//	PrintDebug("DeleteRenderTarget Error, resPoint is NULL.");
	//	return -1;
	//}

	auto camera = reinterpret_cast<D3DCamera*>(resPoint);
	camera->Release();
	delete (camera);
	//D3DResourceManage::Instance().CameraTable.erase(iter);

	return 1;
}

#ifdef __cplusplus 
extern"C"
{
#endif
	int __declspec(dllexport) __stdcall Render(UINT64 resPoint);
#ifdef __cplusplus 
}
#endif

int __declspec(dllexport) __stdcall Render(UINT64 resPoint)
{
	//auto iter = D3DResourceManage::Instance().CameraTable.find(uid);
	//if (iter != D3DResourceManage::Instance().CameraTable.end())
	//{
	//	iter->second->Draw(D3DResourceManage::Instance().pGraphicsCard);
	//	return 1;
	//}
	auto camera = reinterpret_cast<D3DCamera*>(resPoint);
	camera->Draw(D3DResourceManage::Instance().pGraphicsCard);

	return 1;
	//PrintDebug("Can't find Camera UID:");
	//PrintDebug((int)uid);
	//return -1;
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
	//auto iter = D3DResourceManage::Instance().PipelineTable.find("PmdStandard");
	for (const auto& iter : D3DResourceManage::Instance().PipelineTable)
	{
		iter.second->SetCameraTransform(eye, target, up);
	}
}

#ifdef __cplusplus 
extern"C"
{
#endif
	void __declspec(dllexport) __stdcall SetRenderTargetBackColor(UINT64 resPoint, float* color);
#ifdef __cplusplus 
}
#endif
void __declspec(dllexport) __stdcall SetRenderTargetBackColor(UINT64 resPoint, float* color)
{
	//float* backColor = D3DResourceManage::Instance().CameraTable[uid]->m_backColor;

	auto camera = reinterpret_cast<D3DCamera*>(resPoint);
	float* backColor = camera->m_backColor;

	for (int i = 0; i < 4; i++)
	{
		backColor[i] = color[i];
	}

}
#pragma endregion



extern"C"
{
	int __declspec(dllexport) __stdcall SetPMDVertices(const char* _FileFullName, unsigned int _vertCount, unsigned char* _vertices,
		unsigned int _indCount, unsigned short* _indices, unsigned int materialCount, unsigned int* matIndCount);
}

int __declspec(dllexport) __stdcall SetPMDVertices(const char* _FileFullName, unsigned int _vertCount, unsigned char* _vertices,
	unsigned int _indCount, unsigned short* _indices, unsigned int materialCount, unsigned int* matIndCount)
{
	if (_FileFullName == nullptr)
	{
		return -1;
	}

	auto iter = BasicModel::s_modelTable.find(std::string(_FileFullName));
	PMDModel* verRes = nullptr;
	int result = -1;
	if (iter == BasicModel::s_modelTable.end())
	{
		//------------create model from file------------------------
		verRes = new PMDModel();
		verRes->m_materialCount = materialCount;
		UINT indexOff = 0;
		for (int i = 0; i < materialCount; i++)
		{
			verRes->m_matStartIndexList.push_back(indexOff);
			verRes->m_matIndexCountList.push_back(matIndCount[i]);
			indexOff += matIndCount[i];
		}

		BasicModel::s_modelTable.insert(
			std::pair<std::string, BasicModel*>(std::string(_FileFullName), verRes));
		result = verRes->SetVertices(D3DResourceManage::Instance().pGraphicsCard, _vertCount,
			_vertices, _indCount, _indices);
		if (result < 1)
		{
			PrintDebug("SetVertices fault");
		}
	}
	else
	{
		verRes = static_cast<PMDModel*>(iter->second);
		result = 1;
	}
	return result;
}


extern"C"
{
	int __declspec(dllexport) __stdcall SetBasicVertices(const char* _FileFullName, unsigned int _vertCount, unsigned char* _vertices,
		unsigned int _indCount, unsigned short* _indices, unsigned int materialCount, unsigned int* matIndCount);
}

int __declspec(dllexport) __stdcall SetBasicVertices(const char* _FileFullName, unsigned int _vertCount, unsigned char* _vertices,
	unsigned int _indCount, unsigned short* _indices, unsigned int materialCount, unsigned int* matIndCount)
{
	if (_FileFullName == nullptr)
	{
		return -1;
	}

	auto iter = BasicModel::s_modelTable.find(std::string(_FileFullName));
	BasicModel* verRes = nullptr;
	int result = -1;
	if (iter == BasicModel::s_modelTable.end())
	{
		//------------create model from file------------------------
		verRes = new BasicModel();
		verRes->m_materialCount = materialCount;
		UINT indexOff = 0;
		for (int i = 0; i < materialCount; i++)
		{
			verRes->m_matStartIndexList.push_back(indexOff);
			verRes->m_matIndexCountList.push_back(matIndCount[i]);
			indexOff += matIndCount[i];
		}

		BasicModel::s_modelTable.insert(
			std::pair<std::string, BasicModel*>(std::string(_FileFullName), verRes));
		result = verRes->SetVertices(D3DResourceManage::Instance().pGraphicsCard, _vertCount, _vertices, _indCount, _indices);
		//verRes->InitMaterial(_indCount);
		if (result < 1)
		{
			return result;
		}
	}
	else
	{
		verRes = iter->second;
		result = 1;
	}
	return result;
}

//extern"C"
//{
//	int __declspec(dllexport) __stdcall BindPipeline(unsigned long long _uid, const char* pipelineName);
//}
//
//int __declspec(dllexport) __stdcall BindPipeline(unsigned long long _uid, const char* pipelineName)
//{
//	auto iter = ModelInstance::s_uidModelTable.find(_uid);
//	if (iter == ModelInstance::s_uidModelTable.end())
//	{
//		PrintDebug("BindPipeline fault, can't find uid: ");
//		PrintDebug((int)_uid);
//		return -1;
//	}
//
//	auto iter2 = D3DResourceManage::Instance().PipelineTable.find(pipelineName);
//	if (iter2 == D3DResourceManage::Instance().PipelineTable.end())
//	{
//		PrintDebug("BindPipeline fault, can't find pipeline: ");
//		PrintDebug(pipelineName);
//		return -1;
//	}
//
//	auto instance = iter->second;
//	auto pipeline = iter2->second;
//
//	pipeline->RenderModelTable[instance->m_model].insert(pair<ModelInstance*,int> (instance,1));
//
//	//instance->m_bindPipeline = pipeline;
//	instance->CreateDescriptorsByPipeline(pipeline);
//
//	return 1;
//}

extern"C"
{
	int __declspec(dllexport) __stdcall SetMaterial(UINT MaterialControlIDs,
		const char* shaderName, DirectX::XMFLOAT3 diffuse, float alpha,
		float specularity, DirectX::XMFLOAT3 specular, DirectX::XMFLOAT3 ambient, unsigned char edgeFlg,
		const char* toonPath, const char* texFilePath);
}

int __declspec(dllexport) __stdcall SetMaterial(UINT MaterialControlID,
	const char* shaderName, DirectX::XMFLOAT3 diffuse, float alpha,
	float specularity, DirectX::XMFLOAT3 specular, DirectX::XMFLOAT3 ambient, unsigned char edgeFlg,
	const char* toonPath, const char* texFilePath)
{
	auto result = MaterialControl::SetMaterial(D3DResourceManage::Instance().pGraphicsCard, shaderName, diffuse, alpha,
		specularity, specular, ambient, edgeFlg, toonPath, texFilePath, MaterialControlID);

	if (result != 1)
	{
		return result;
	}


	auto iter = D3DResourceManage::Instance().PipelineTable.find(shaderName);
	if (iter == D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("Can't find pipeline when SetMaterial: ");
		PrintDebug(shaderName);
		return -1;
	}

	auto material = D3DResourceManage::Instance().MaterialTable[MaterialControlID];

	material->CreateDescriptor(D3DResourceManage::Instance().pGraphicsCard, iter->second);

	return 1;
}

extern"C"
{
	void __declspec(dllexport) __stdcall SetMaterialValue(UINT MaterialControlIDs,
		const char* shaderName, DirectX::XMFLOAT3 diffuse, float alpha,
		float specularity, DirectX::XMFLOAT3 specular, DirectX::XMFLOAT3 ambient, unsigned char edgeFlg,
		const char* toonPath, const char* texFilePath);
}

void __declspec(dllexport) __stdcall SetMaterialValue(UINT MaterialControlID,
	const char* shaderName, DirectX::XMFLOAT3 diffuse, float alpha,
	float specularity, DirectX::XMFLOAT3 specular, DirectX::XMFLOAT3 ambient, unsigned char edgeFlg,
	const char* toonPath, const char* texFilePath)
{
	MaterialControl::SetMaterialValue(D3DResourceManage::Instance().pGraphicsCard, shaderName, diffuse, alpha,
		specularity, specular, ambient, edgeFlg, toonPath, texFilePath, MaterialControlID);
}

extern"C"
{
	void __declspec(dllexport) __stdcall CreateCustomizedResource(UINT materialId,
		LPCSTR name, uint16_t datasize, UINT shaderRegisterNum);
}

void __declspec(dllexport) __stdcall CreateCustomizedResource(UINT materialId,
	LPCSTR name, uint16_t datasize, UINT rootParameterIndex)
{
	auto iter = D3DResourceManage::Instance().MaterialTable.find(materialId);
	if (iter == D3DResourceManage::Instance().MaterialTable.end())
	{
		PrintDebug("CreateCustomizedResource fault, can't find : ");
		PrintDebug((int)materialId);
		return;
	}
	auto materialControl = iter->second;
	materialControl->CreateCustomizedResource(D3DResourceManage::Instance().pGraphicsCard,
		name, datasize, rootParameterIndex);
}

extern"C"
{
	void __declspec(dllexport) __stdcall SetCustomizedResourceValue(UINT materialId,
		LPCSTR name, unsigned char* data);
}

void __declspec(dllexport) __stdcall SetCustomizedResourceValue(UINT materialId,
	LPCSTR name, unsigned char* data)
{
	auto iter = D3DResourceManage::Instance().MaterialTable.find(materialId);
	if (iter == D3DResourceManage::Instance().MaterialTable.end())
	{
		PrintDebug("SetCustomizedResourceValue fault, can't find: ");
		PrintDebug((int)materialId);
		return;
	}
	auto materialControl = iter->second;
	materialControl->SetCustomizedResourceValue(name, data);
}

extern"C"
{
	void __declspec(dllexport) __stdcall GetCustomizedResourceValue(
		UINT materialId, LPCSTR name, unsigned char** data, UINT16* size);
}

void __declspec(dllexport) __stdcall GetCustomizedResourceValue(
	UINT materialId, LPCSTR name, unsigned char** data, UINT16* size)
{
	auto iter = D3DResourceManage::Instance().MaterialTable.find(materialId);
	if (iter == D3DResourceManage::Instance().MaterialTable.end())
	{
		PrintDebug("SetCustomizedResourceValue fault, can't find: ");
		PrintDebug((int)materialId);
		return;
	}
	auto materialControl = iter->second;
	*data = materialControl->GetCustomizedResourceValue(name, size);
}

extern"C"
{
	int __declspec(dllexport) __stdcall CreateCustomizedDescriptors(UINT materialID, const char* pipelineName);
}

int __declspec(dllexport) __stdcall CreateCustomizedDescriptors(UINT materialID, const char* pipelineName)
{
	auto iter = D3DResourceManage::Instance().MaterialTable.find(materialID);
	if (iter == D3DResourceManage::Instance().MaterialTable.end())
	{
		PrintDebug("CreateCustomizedDescriptors fault, can't find materialID: ");
		PrintDebug((int)materialID);
		return -1;
	}

	auto iter2 = D3DResourceManage::Instance().PipelineTable.find(pipelineName);
	if (iter2 == D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("BindPipeline fault, can't find pipeline: ");
		PrintDebug(pipelineName);
		return -1;
	}

	auto materialControl = iter->second;
	auto pipeline = iter2->second;

	materialControl->CreateDescriptorsByPipeline(pipeline);

	return 1;
}

extern"C"
{
	int __declspec(dllexport) __stdcall BindMaterialControls(UINT64 UID, UINT MaterialControlIDs[], UINT materialCount);
}
int __declspec(dllexport) __stdcall BindMaterialControls(UINT64 UID, UINT MaterialControlIDs[], UINT materialCount)
{
	auto iter = ModelInstance::s_uidModelTable.find(UID);
	if (iter == ModelInstance::s_uidModelTable.end())
	{
		PrintDebug("BindMaterialControl fault, can't find uid: ");
		PrintDebug((int)UID);
		return -1;
	}

	for (int i = 0; i < materialCount; i++)
	{
		iter->second->BindMaterialControl(MaterialControlIDs[i], i);
	}


	return 1;
}

extern"C"
{
	int __declspec(dllexport) __stdcall BindMaterialControl(UINT64 UID, UINT MaterialControlID, UINT index);
}
int __declspec(dllexport) __stdcall BindMaterialControl(UINT64 UID, UINT MaterialControlID, UINT index)
{
	auto iter = ModelInstance::s_uidModelTable.find(UID);
	if (iter == ModelInstance::s_uidModelTable.end())
	{
		PrintDebug("BindMaterialControl fault, can't find uid: ");
		PrintDebug((int)UID);
		return -1;
	}

	iter->second->BindMaterialControl(MaterialControlID, index);

	return 1;
}

extern"C"
{
	int __declspec(dllexport) __stdcall UnBindMaterialControl(UINT64 UID, UINT MaterialControlID, UINT index);
}
int __declspec(dllexport) __stdcall UnBindMaterialControl(UINT64 UID, UINT MaterialControlID, UINT index)
{
	auto iter = ModelInstance::s_uidModelTable.find(UID);
	if (iter == ModelInstance::s_uidModelTable.end())
	{
		PrintDebug("UnBindMaterialControl fault, can't find uid: ");
		PrintDebug((int)UID);
		return -1;
	}

	iter->second->UnBindMaterialControl(MaterialControlID, index);
	PrintDebug("UnBindMaterialControl not realized, Pending...");

	return 1;
}

extern"C"
{
	int __declspec(dllexport) __stdcall SetPMDBoneIk(const char* _FileFullName, unsigned short boneNum,
		unsigned short ikNum, const char* boneName[], unsigned short parentNo[], unsigned short nextNo[],
		unsigned char type[], unsigned short ikBoneNo[], DirectX::XMFLOAT3 pos[],
		uint16_t boneIdx[], uint16_t targetIdx[], uint16_t iterations[], float limit[],
		uint8_t chainLen[], uint16_t nodeIdxes[]);
}

int __declspec(dllexport) __stdcall SetPMDBoneIk(const char* _FileFullName, unsigned short boneNum,
	unsigned short ikNum, const char* boneName[], unsigned short parentNo[], unsigned short nextNo[],
	unsigned char type[], unsigned short ikBoneNo[], DirectX::XMFLOAT3 pos[],
	uint16_t boneIdx[], uint16_t targetIdx[], uint16_t iterations[], float limit[],
	uint8_t chainLen[], uint16_t nodeIdxes[])
{
	if (_FileFullName == nullptr)
	{
		return -1;
	}

	auto iter = BasicModel::s_modelTable.find(std::string(_FileFullName));
	PMDModel* verRes = nullptr;
	int result = -1;
	if (iter == BasicModel::s_modelTable.end())
	{
		PrintDebug("can't find:");
		PrintDebug(_FileFullName);
		return -1;
	}
	else
	{
		uint16_t** nodeIdxesArr = new uint16_t * [ikNum];
		uint16_t* curPos = nodeIdxes;
		for (int i = 0; i < ikNum; i++)
		{
			int len = chainLen[i];
			nodeIdxesArr[i] = new uint16_t[len];
			std::copy(curPos, curPos + len, nodeIdxesArr[i]);
			curPos += len;
		}


		verRes = static_cast<PMDModel*>(iter->second);
		/*result = verRes->SetBones(D3DResourceManage::Instance().pGraphicsCard, boneNum,
			ikNum, boneName, parentNo, nextNo,
			type, ikBoneNo, pos,
			boneIdx,targetIdx, iterations, limit,
			chainLen, nodeIdxesArr);*/

		for (int i = 0; i < ikNum; i++)
		{
			delete[]nodeIdxesArr[i];
		}
		delete[]nodeIdxesArr;
	}
	return 1;
}

extern"C"
{
	void __declspec(dllexport) __stdcall UpdatePMDBoneMatrices(unsigned long long _uid, DirectX::XMMATRIX* boneMatrices, int size);
}

void __declspec(dllexport) __stdcall UpdatePMDBoneMatrices(unsigned long long _uid, DirectX::XMMATRIX* boneMatrices, int size)
{
	auto iter = ModelInstance::s_uidModelTable.find(_uid);
	if (iter == ModelInstance::s_uidModelTable.end())
	{
		PrintDebug("UpdatePMDBoneMatrices fault, can't find: ");
		PrintDebug((int)_uid);
		return;
	}
	PMDModelInstance* pInstance = static_cast<PMDModelInstance*>(iter->second);
	pInstance->UpdateBoneMatrices(boneMatrices, size);
}

extern"C"
{
	int __declspec(dllexport) __stdcall InstantiatePMDModel(unsigned long long _uid, const char* _FileFullName, int boneSize);
}

int __declspec(dllexport) __stdcall InstantiatePMDModel(unsigned long long _uid, const char* _FileFullName, int boneSize)
{
	if (_FileFullName == nullptr)
	{
		return -1;
	}

	auto iter = BasicModel::s_modelTable.find(std::string(_FileFullName));
	PMDModel* verRes = nullptr;
	int result = -1;
	if (iter == BasicModel::s_modelTable.end())
	{
		return -1;
	}
	else
	{
		verRes = static_cast<PMDModel*>(iter->second);
		result = 1;
	}

	//------------delete if exist instance-------------
	DeleteModelInstance(_uid);

	//------------create instance------------
	PMDModelInstance* instance = new PMDModelInstance();
	instance->m_model = verRes;
	instance->m_materialControls.resize(verRes->m_materialCount);
	//instance->BindAnimation(verRes->m_animation);
	instance->CreateTransformView(D3DResourceManage::Instance().pGraphicsCard, boneSize);

	verRes->m_instances.push_back(static_cast<ModelInstance*>(instance));
	ModelInstance::s_uidModelTable.insert(
		pair<unsigned long long, ModelInstance*>(_uid, static_cast<ModelInstance*>(instance)));

	return result;
}

extern"C"
{
	int __declspec(dllexport) __stdcall InstantiateBasicModel(unsigned long long _uid, const char* _FileFullName);
}

int __declspec(dllexport) __stdcall InstantiateBasicModel(unsigned long long _uid, const char* _FileFullName)
{
	if (_FileFullName == nullptr)
	{
		return -1;
	}

	auto iter = BasicModel::s_modelTable.find(std::string(_FileFullName));
	BasicModel* verRes = nullptr;
	int result = -1;
	if (iter == BasicModel::s_modelTable.end())
	{
		return -1;
	}
	else
	{
		verRes = iter->second;
		result = 1;
	}

	//------------delete if exist instance-------------
	DeleteModelInstance(_uid);

	//------------create instance------------
	ModelInstance* instance = new ModelInstance();
	instance->m_model = verRes;
	instance->m_materialControls.resize(verRes->m_materialCount);
	instance->CreateTransformView(D3DResourceManage::Instance().pGraphicsCard);

	verRes->m_instances.push_back(instance);
	ModelInstance::s_uidModelTable.insert(
		pair<unsigned long long, ModelInstance*>(_uid, instance));

	return result;
}