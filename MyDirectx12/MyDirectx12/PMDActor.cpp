#include"PMDActor.h"
#include"D3DResourceManage.h"
#include <sstream>
using namespace DirectX;

std::map<std::string, BasicModel*> BasicModel::s_modelTable;
std::map <unsigned long long, ModelInstance*> ModelInstance::s_uidModelTable;     //key is uid

//std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
//{
//	int pathIndex1 = modelPath.rfind('/');
//	int pathIndex2 = modelPath.rfind('\\');
//	auto pathIndex = max(pathIndex1, pathIndex2);
//	auto folderPath = modelPath.substr(0, pathIndex + 1);
//	return folderPath + texPath;
//}

ID3D12Resource* LoadTextureFromFile(std::string& texPath, ID3D12Device* _d3dDevive)
{
	auto pResTable = &D3DResourceManage::Instance().ResourceTable;
	auto iter = pResTable->find(texPath);
	if (iter != pResTable->end())
	{
		return iter->second;
	}

	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto wTexPath = GetWideStringFromString(texPath);
	auto ext = GetExtension(texPath);

	auto result = D3DResourceManage::Instance().
		LoadLambdaTable[ext](wTexPath, &metadata, scratchImg);

	if (FAILED(result))
	{
		wchar_t showError[80] = L"Load Texture File fault: ";
		wcscat_s(showError, wTexPath.c_str());
		PrintDebug(showError);
		return nullptr;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0);

	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.CreationNodeMask = 0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC texResDesc = {};
	texResDesc.Format = metadata.format;
	texResDesc.Width = metadata.width;
	texResDesc.Height = metadata.height;
	texResDesc.DepthOrArraySize = metadata.arraySize;
	texResDesc.SampleDesc.Count = 1;
	texResDesc.SampleDesc.Quality = 0;
	texResDesc.MipLevels = metadata.mipLevels;
	texResDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	texResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* texbuff = nullptr;
	result = _d3dDevive->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&texResDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(result))
	{
		PrintDebug(L"Create Texture Resource fault:");
		return nullptr;
	}

	//Transport the texture data to GPU. WriteToSubresource() is not the good function.
	//There is an better way - ID3D12Resource::Map(). Here not tell;
	result = texbuff->WriteToSubresource(
		0,
		nullptr,
		img->pixels,
		img->rowPitch,          //one row data size
		img->slicePitch  //one slice size, this time is all data size
	);
	if (FAILED(result))
	{
		PrintDebug(L"Transport Texture Resource to GPU fault. ");
		return nullptr;
	}

	pResTable->insert(std::pair<std::string, ID3D12Resource*>(texPath, texbuff));

	return texbuff;
}

ID3D12Resource* CreateGrayGradationTexture(ID3D12Device* _d3dDevive)
{
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;
	resDesc.Height = 256;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* grayGradationBuff = nullptr;

	auto result = _d3dDevive->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&grayGradationBuff));
	if (FAILED(result))
	{
		ShowMsgBox(NULL, "Create GrayGradationBuff texture buff Fault");
		PrintDebug(L"Create GrayGradationBuff texture buff Fault.");
		return nullptr;
	}

	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4)
	{
		auto col = (0xff << 24) | RGB(c, c, c);
		std::fill(it, it + 4, col);
		c--;
	}

	result = grayGradationBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * sizeof(unsigned int),
		sizeof(unsigned int) * data.size());


	return grayGradationBuff;
}

ID3D12Resource* CreateOneColorTexture(ID3D12Device* _d3dDevive, const uint32_t& color)
{
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;
	resDesc.Height = 4;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* oneColorBuff = nullptr;

	auto result = _d3dDevive->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&oneColorBuff));
	if (FAILED(result))
	{
		PrintDebug(L"Create OneColor texture buff Fault.");
		return nullptr;
	}

	uint8_t r = (color >> 24);
	uint8_t g = (color >> 16);
	uint8_t b = (color >> 8);
	uint8_t a = (color >> 0);
	uint32_t opposite = RGB(r, g, b) | (a << 24);
	std::vector<uint32_t> data(4 * 4);
	std::fill(data.begin(), data.end(), opposite);

	result = oneColorBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size() * sizeof(data[0]));
	if (FAILED(result))
	{
		PrintDebug(L"Write data to OneColor texture buff Fault.");
		return nullptr;
	}

	return oneColorBuff;
}

int PMDModel::SetVertices(D3DDevice* _cD3DDev, unsigned int _vertCount, unsigned char* _vertices,
	unsigned int _indCount, unsigned short* _indices)

{
	auto d3ddevice = _cD3DDev->pD3D12Device;

	constexpr unsigned int pmdvertex_size_inFIle = 38;
	constexpr unsigned int pmdvertex_size_inGPU = 40;

	//---rearrangement read data----
	std::vector<unsigned char> vertices(_vertCount * pmdvertex_size_inGPU);
	for (int i = 0; i < _vertCount; i++)
	{
		std::copy(&_vertices[i * pmdvertex_size_inFIle],
			&_vertices[i * pmdvertex_size_inFIle] + pmdvertex_size_inFIle,
			&vertices[i * pmdvertex_size_inGPU]);
	}



	//----------vertex buff------------------
	auto heapTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vertBuffDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size());
	//ID3D12Device::CreateHeap() and ID3D12Device::CreatePlaced Resource() similar, this book doesn't teach.
	HRESULT result = d3ddevice->CreateCommittedResource(
		&heapTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&vertBuffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vertBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"CreateCommittedResource PMDVertex fault.");
		return -1;
	}

	unsigned char* s_vertMap = nullptr;
	result = m_vertBuff->Map(0, nullptr, (void**)&s_vertMap);
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"vertMap fault.");
		return -1;
	}

	memcpy(s_vertMap, vertices.data(), vertices.size());
	//std::copy(vertices.begin(), vertices.end(), s_vertMap);

	m_vertBuff->Unmap(0, nullptr);

	//create vertex buffer view
	m_vbView.BufferLocation = m_vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = vertices.size();
	m_vbView.StrideInBytes = pmdvertex_size_inGPU;

	//----------------------index part----------------------------
	size_t indicesAllData_size = _indCount * sizeof(_indices[0]);
	auto indicesBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indicesAllData_size);
	result = d3ddevice->CreateCommittedResource(
		&heapTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&indicesBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_idxBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"CreateCommittedResource index fault.");
		return -1;
	}

	unsigned short* mappedIdx = nullptr;
	result = m_idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"index Map fault.");
		return -1;
	}
	std::copy(_indices, _indices + _indCount, mappedIdx);
	m_idxBuff->Unmap(0, nullptr);

	m_ibView.BufferLocation = m_idxBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = indicesAllData_size;

	return 1;
}



int PMDModel::SetBones(D3DDevice* _cD3DDev, unsigned short boneNum,
	unsigned short ikNum, const char* boneName[], unsigned short parentNo[], unsigned short nextNo[],
	unsigned char type[], unsigned short ikBoneNo[], DirectX::XMFLOAT3 pos[],
	uint16_t boneIdx[], uint16_t targetIdx[], uint16_t iterations[], float limit[],
	uint8_t chainLen[], uint16_t** nodeIdxes)
{
	//------------------bone data & aniamtion-----------------------
	m_pmdBones.resize(boneNum);
	for (int i = 0; i < boneNum; i++)
	{
		int nameLen = strlen(boneName[i]);
		nameLen = min(nameLen + 1, 20);
		for (int j = 0; j < nameLen; j++)
		{
			m_pmdBones[i].boneName[j] = boneName[i][j];
		}
		m_pmdBones[i].parentNo = parentNo[i];
		m_pmdBones[i].nextNo = nextNo[i];
		m_pmdBones[i].type = type[i];
		m_pmdBones[i].ikBoneNo = ikBoneNo[i];
		m_pmdBones[i].pos = pos[i];
	}

	m_ikData.resize(ikNum);
	for (int i = 0; i < ikNum; i++)
	{
		m_ikData[i].boneIdx = boneIdx[i];
		m_ikData[i].targetIdx = targetIdx[i];
		m_ikData[i].iterations = iterations[i];
		m_ikData[i].limit = limit[i];
		m_ikData[i].nodeIdxes.resize(chainLen[i]);
		for (int j = 0; j < chainLen[i]; j++)
		{
			m_ikData[i].nodeIdxes[j] = nodeIdxes[i][j];
		}
	}

	SetBone();

	//------Debug Ik bone info------------
	auto getNameFromIdx = [&](uint16_t idx)->std::string
	{
		auto it = std::find_if(m_boneNodeTable.begin(), m_boneNodeTable.end(),
			[idx](const std::pair<std::string, BoneNode>& obj)
			{
				return obj.second.boneIdx == idx;
			});
		if (it != m_boneNodeTable.end())
		{
			return it->first;
		}
		else
		{
			return "";
		}
	};
	for (auto& ik : m_ikData)
	{
		std::ostringstream oss;
		oss << "IK bone number = " << ik.boneIdx << ":"
			<< getNameFromIdx(ik.boneIdx) << std::endl;

		oss << "target number = " << ik.targetIdx << ":"
			<< getNameFromIdx(ik.targetIdx) << std::endl;

		for (auto& node : ik.nodeIdxes)
		{
			oss << "     node bone = " << node << ":"
				<< getNameFromIdx(node) << std::endl;
		}

		//ShowMsgBox(L"", ik.nodeIdxes.size());
		PrintDebug(oss.str().c_str());
	}

	return 1;
}


int PMDModel::SetBone()
{

	m_boneNameArr.resize(m_pmdBones.size());
	m_boneNodeAddressArr.resize(m_pmdBones.size());

	if (m_pmdBones.size() > 0)
	{
		m_rootNodeStr = m_pmdBones[0].boneName;
	}

	m_kneeIdxes.clear();
	for (int i = 0; i < m_pmdBones.size(); i++)
	{
		auto& pb = m_pmdBones[i];
		auto* node = &m_boneNodeTable[pb.boneName];
		node->boneIdx = i;
		node->startPos = pb.pos;

		m_boneNameArr[i] = pb.boneName;
		m_boneNodeAddressArr[i] = node;

		std::string boneName = pb.boneName;
		//the knee data is a special one, a common bone data struct may be not like this
		if (boneName.find("ひざ") != std::string::npos)
		{
			m_kneeIdxes.emplace_back(i);
		}
	}

	for (auto& pb : m_pmdBones)
	{
		if (pb.parentNo >= m_pmdBones.size())
		{
			continue;
		}

		auto parentName = m_boneNameArr[pb.parentNo];
		m_boneNodeTable[parentName].children.emplace_back(
			&m_boneNodeTable[pb.boneName]);
		//PrintDebug(parentName.c_str());
		//PrintDebug(pb.boneName);
	}

	return 1;
}

void PMDModelInstance::InitAnimation(D3DAnimation* animationRes)
{
	if (m_animationInstance == nullptr)
	{
		m_animationInstance = new D3DAnimationInstance();
	}
	m_animationInstance->m_animation = animationRes;

	m_animationInstance->m_owner = this;
}

int PMDModelInstance::CreateTransformView(D3DDevice* _cD3DDev, int boneSize)
{
	//m_boneMatrices.resize(Model()->m_pmdBones.size());
	//std::fill(m_boneMatrices.begin(), m_boneMatrices.end(), XMMatrixIdentity());
	auto buffSize = sizeof(Transform) * (1 + boneSize);
	buffSize = (buffSize + 0xff) & ~0xff;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = _cD3DDev->pD3D12Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_transformConstBuff));
	if (FAILED(result))
	{
		PrintDebug("Create Transform const buff fault.");
		return -1;
	}

	result = m_transformConstBuff->Map(0, nullptr, (void**)&m_mapMatrices);
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Map transform const buff fault.");
		return -1;
	}

	m_transform.world = XMMatrixIdentity();
	m_mapMatrices[0] = m_transform.world;

	std::fill(m_mapMatrices + 1, m_mapMatrices + 1 + boneSize, XMMatrixIdentity());

	return 1;
}

void PMDModelInstance::UpdateBoneMatrices(DirectX::XMMATRIX* boneMatrices, int size)
{
	std::copy(boneMatrices, boneMatrices + size, m_mapMatrices + 1);
}

int BasicModel::SetVertices(D3DDevice* _cD3DDev, unsigned int _vertCount, unsigned char* _vertices,
	unsigned int _indCount, unsigned short* _indices)
{
	//----------vertex buff------------------
	auto heapTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	constexpr unsigned int basicVertex_size = 32;
	UINT vertexAllData_size = _vertCount * basicVertex_size;
	UINT indicesAllData_size = _indCount * sizeof(unsigned short);
	auto vertBuffDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexAllData_size);
	//ID3D12Device::CreateHeap() and ID3D12Device::CreatePlaced Resource() similar, this book doesn't teach.
	HRESULT result = _cD3DDev->pD3D12Device->CreateCommittedResource(
		&heapTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&vertBuffDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vertBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		PrintDebug("CreateCommittedResource basic Vertex fault.");
		return -1;
	}
	unsigned char* s_vertMap = nullptr;
	result = m_vertBuff->Map(0, nullptr, (void**)&s_vertMap);
	if (FAILED(result))
	{
		PrintDebug(L"vertMap fault.");
		return -1;
	}

	memcpy(s_vertMap, _vertices, vertexAllData_size);

	m_vertBuff->Unmap(0, nullptr);

	//create vertex buffer view
	m_vbView.BufferLocation = m_vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = vertexAllData_size;
	m_vbView.StrideInBytes = basicVertex_size;

	//----------------------index part----------------------------

	auto indicesBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indicesAllData_size);
	result = _cD3DDev->pD3D12Device->CreateCommittedResource(
		&heapTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&indicesBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_idxBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		PrintDebug(L"CreateCommittedResource index fault.");
		return -1;
	}
	unsigned short* mappedIdx = nullptr;
	result = m_idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	if (FAILED(result))
	{
		PrintDebug(L"index Map fault.");
		return -1;
	}
	std::copy(_indices, _indices + _indCount, mappedIdx);
	m_idxBuff->Unmap(0, nullptr);

	m_ibView.BufferLocation = m_idxBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = indicesAllData_size;
	return 1;
}

int ModelInstance::CreateTransformView(D3DDevice* _cD3DDev)
{
	auto buffSize = sizeof(Transform);
	buffSize = (buffSize + 0xff) & ~0xff;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = _cD3DDev->pD3D12Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_transformConstBuff));
	if (FAILED(result))
	{
		PrintDebug("Create Transform const buff fault.");
		return -1;
	}

	result = m_transformConstBuff->Map(0, nullptr, (void**)&m_mapMatrices);
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Map transform const buff fault.");
		return -1;
	}

	m_transform.world = XMMatrixIdentity();
	*m_mapMatrices = m_transform.world;

	return 1;
}

void ModelInstance::CreateDescriptorsByPipeline(D3DPipeline* pipeline)
{
	ID3D12Device* d3ddevice = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;
	//basic Transform 
	m_pipelineRecordList.push_back(ModelInstance::PipelineRecord{});
	auto& record = m_pipelineRecordList[m_pipelineRecordList.size() - 1];
	record.transformDescOffset = pipeline->CreateConstantDescript(
		d3ddevice, m_transformConstBuff);
	record.pipeline = pipeline;

	////CustomizedResource
	//for (auto& iter : m_shaderResouceTable)
	//{
	//	auto& res = iter.second;
	//	
	//	res.descOffset = pipeline->CreateConstantDescript(
	//		d3ddevice, res.resource);
	//}
}

int BindPipelineToInstance(ModelInstance* pInstanse, const char* pipelineName)
{
	auto iter2 = D3DResourceManage::Instance().PipelineTable.find(pipelineName);
	if (iter2 == D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("BindPipeline fault, can't find pipeline: ");
		PrintDebug(pipelineName);
		return -1;
	}
	auto& pipeline = iter2->second;

	auto& map = pipeline->RenderModelTable[pInstanse->m_model];
	auto iter = map.find(pInstanse);
	if (iter == map.end())
	{
		map.insert(std::pair<ModelInstance*, int>(pInstanse, 1));
		pInstanse->CreateDescriptorsByPipeline(pipeline);
	}
	else
	{
		iter->second++;
	}



	return 1;
}

void RemovePipelineBind(ModelInstance* pInstanse, const char* pipelineName)
{
	auto iter2 = D3DResourceManage::Instance().PipelineTable.find(pipelineName);
	if (iter2 == D3DResourceManage::Instance().PipelineTable.end())
	{
		PrintDebug("RemovePipelineBind fault, can't find pipeline: ");
		PrintDebug(pipelineName);
		return;
	}
	auto& pipeline = iter2->second;

	auto& map = pipeline->RenderModelTable[pInstanse->m_model];
	auto iter = map.find(pInstanse);
	if (iter == map.end())
	{
		PrintDebug("RemovePipelineBind fault, pipeline not record it: ");
		PrintDebug(pipelineName);
	}
	else
	{
		iter->second--;

		if (iter->second <= 0)
		{
			map.erase(pInstanse);

			// release Descriptors logic
		}


	}
}

//int ModelInstance::BindMaterialControls(UINT matIds[], UINT count)
//{
//	if (count != m_materialControls.size())
//	{
//		ShowMsgBox(nullptr, "BindMaterialControl Error, the material count wrong!");
//		ShowMsgBox(nullptr, count);
//		ShowMsgBox(nullptr, m_materialControls.size());
//		return 1;
//	}
//
//	for (int i = 0; i < count; i++)
//	{
//		auto iter = D3DResourceManage::Instance().MaterialTable.find(matIds[i]);
//		if (iter == D3DResourceManage::Instance().MaterialTable.end())
//		{
//			PrintDebug("Bind Material Control fault, can't find matID: ");
//			PrintDebug((int)matIds[i]);
//			return -1;
//		}
//
//		
//		m_materialControls[i] = iter->second;
//	}
//	return 1;
//}

int ModelInstance::BindMaterialControl(UINT matId, UINT index)
{
	if (index >= m_materialControls.size())
	{
		ShowMsgBox(nullptr, "BindMaterialControl Error, the material index is over size!");
		ShowMsgBox(nullptr, index);
		ShowMsgBox(nullptr, m_materialControls.size());
		return -1;
	}


	auto iter = D3DResourceManage::Instance().MaterialTable.find(matId);
	if (iter == D3DResourceManage::Instance().MaterialTable.end())
	{
		PrintDebug("Bind Material Control fault, can't find matID: ");
		PrintDebug((int)matId);
		return -1;
	}

	if (m_materialControls[index] != nullptr)
	{
		RemovePipelineBind(this, iter->second->m_material.pipeLineName.c_str());
		m_materialControls[index] = nullptr;
	}

	m_materialControls[index] = iter->second;

	return BindPipelineToInstance(this, iter->second->m_material.pipeLineName.c_str());
}



//int ModelInstance::CreateCustomizedResource(D3DDevice* _cD3DDev, LPCSTR name, uint16_t datasize,
//	UINT rootParameterIndex)
//{
//	m_shaderResouceTable.insert(std::pair<std::string, ShaderResource>(name, ShaderResource{}));
//	auto& res = m_shaderResouceTable[name];
//	//res.shaderRegisterNum = shaderRegisterNum;
//	res.datasize = datasize;
//	res.RootParameterIndex = rootParameterIndex;
//
//	// Ensure buffer size is a multiple of 256
//	unsigned long long buffSize = (res.datasize + 0xff) & ~0xff;
//
//	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);
//
//	auto result = _cD3DDev->pD3D12Device->CreateCommittedResource(
//		&heapProp,
//		D3D12_HEAP_FLAG_NONE,
//		&resDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&res.resource));
//	if (FAILED(result))
//	{
//		PrintDebug("Create Customized const buff fault.");
//		return -1;
//	}
//
//	result = res.resource->Map(0, nullptr, (void**)&res.mapData);
//	if (FAILED(result))
//	{
//		ShowMsgBox(L"Error", L"Map Customized const buff fault.");
//		return -1;
//	}
//
//	memset(res.mapData, 0, buffSize);
//
//	return 1;
//}
//
//void ModelInstance::SetCustomizedResourceValue(LPCSTR name, unsigned char* data)
//{
//	auto iter = m_shaderResouceTable.find(name);
//	if (iter != m_shaderResouceTable.end())
//	{
//		std::memcpy(iter->second.mapData, data, iter->second.datasize);
//		PrintDebug("Set Customized Resource Value success!");
//	}
//}

ModelInstance::~ModelInstance()
{
	//m_transformDescHeap->Release();
	m_transformConstBuff->Release();

	//for (auto& iter : m_shaderResouceTable)
	//{
	//	auto& res = iter.second;
	//	res.resource->Release();
	//}
	//m_shaderResouceTable.clear();
}

int MaterialControl::SetMaterials(D3DDevice* _cD3DDev, unsigned int matCount, const char* shaderName[],
	DirectX::XMFLOAT3 diffuse[], float alpha[],
	float specularity[], DirectX::XMFLOAT3 specular[], DirectX::XMFLOAT3 ambient[], unsigned char edgeFlg[],
	const char* toonPath[], const char* texFilePath[], UINT MaterialControlIDs[])
{
	auto& matTable = D3DResourceManage::Instance().MaterialTable;
	auto d3ddevice = _cD3DDev->pD3D12Device;

	for (int i = 0; i < matCount; i++)
	{
		MaterialControl* matControl = new MaterialControl();
		matTable.insert(std::pair<UINT, MaterialControl*>(MaterialControlIDs[i], matControl));

		Material& matrial = matControl->m_material;

		std::string toonFilePath = toonPath[i];
		if (toonFilePath != "")
		{
			matrial.m_toonResource = LoadTextureFromFile(toonFilePath, d3ddevice);
		}

		matrial.pipeLineName = shaderName[i];
		matrial.material.diffuse = diffuse[i];
		matrial.material.alpha = alpha[i];
		matrial.material.specular = specular[i];
		matrial.material.specularity = specularity[i];
		matrial.material.ambient = ambient[i];

		if (strlen(texFilePath[i]) == 0)
		{
			matrial.m_textureResource = nullptr;
		}
		else
		{
			std::string texFileName = GetFileName(texFilePath[i]);
			std::string directoryName = GetDirectoryName(texFilePath[i]);
			std::string sphFileName = "";
			std::string spaFileName = "";

			if (std::count(texFileName.begin(), texFileName.end(), '*') > 0)
			{
				auto namepair = SplitFileName(texFileName);
				auto extension = GetExtension(namepair.first);
				if (extension == "sph")
				{
					texFileName = namepair.second;
					sphFileName = namepair.first;
				}
				else if (extension == "spa")
				{
					texFileName = namepair.second;
					spaFileName = namepair.first;
				}
				else
				{
					texFileName = namepair.first;
					extension = GetExtension(namepair.second);
					if (extension == "sph")
					{
						sphFileName = namepair.second;
					}
					else if (extension == "spa")
					{
						spaFileName = namepair.second;
					}
				}
			}
			else
			{
				auto extension = GetExtension(texFileName);
				if (extension == "sph")
				{
					sphFileName = texFileName;
					texFileName = "";
				}
				else if (extension == "spa")
				{
					spaFileName = texFileName;
					texFileName = "";
				}
			}

			if (texFileName != "")
			{
				auto texFilePathFull = directoryName + '/' + texFileName;
				matrial.m_textureResource = LoadTextureFromFile(texFilePathFull, d3ddevice);
			}
			if (sphFileName != "")
			{
				auto sphFilePath = directoryName + '/' + sphFileName;
				matrial.m_sphResource = LoadTextureFromFile(sphFilePath, d3ddevice);
			}
			if (spaFileName != "")
			{
				auto spaFilePath = directoryName + '/' + spaFileName;
				matrial.m_spaResource = LoadTextureFromFile(spaFilePath, d3ddevice);
			}
		}

		//-----------material buff---------------------
		auto materialBuffSize = sizeof(MaterialForHlsl);
		materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

		auto materialResDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize);
		auto heapTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto result = d3ddevice->CreateCommittedResource(
			&heapTypeUpload,
			D3D12_HEAP_FLAG_NONE,
			&materialResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(matControl->m_materialBuff.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			ShowMsgBox(L"error", L"Create material Resource fault.");
			return -1;
		}

		char* mapMaterial = nullptr;
		result = matControl->m_materialBuff->Map(0, nullptr, (void**)&mapMaterial);

		*((MaterialForHlsl*)mapMaterial) = matrial.material;

		matControl->m_materialBuff->Unmap(0, nullptr);
	}

	return 1;
}

int MaterialControl::CreateCustomizedResource(D3DDevice* _cD3DDev, LPCSTR name, uint16_t datasize,
	UINT rootParameterIndex)
{
	m_shaderResourceTable.insert(std::pair<std::string, ShaderResource>(name, ShaderResource{}));
	auto& res = m_shaderResourceTable[name];
	//res.shaderRegisterNum = shaderRegisterNum;
	res.datasize = datasize;
	res.RootParameterIndex = rootParameterIndex;

	// Ensure buffer size is a multiple of 256
	unsigned long long buffSize = (res.datasize + 0xff) & ~0xff;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = _cD3DDev->pD3D12Device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&res.resource));
	if (FAILED(result))
	{
		PrintDebug("Create Customized const buff fault.");
		return -1;
	}

	result = res.resource->Map(0, nullptr, (void**)&res.mapData);
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Map Customized const buff fault.");
		return -1;
	}

	memset(res.mapData, 0, buffSize);

	return 1;
}

void MaterialControl::CreateDescriptorsByPipeline(D3DPipeline* pipeline)
{
	ID3D12Device* d3ddevice = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

	//CustomizedResource
	for (auto& iter : m_shaderResourceTable)
	{
		auto& res = iter.second;

		res.descOffset = pipeline->CreateConstantDescript(
			d3ddevice, res.resource);
	}
}

void MaterialControl::SetCustomizedResourceValue(LPCSTR name, unsigned char* data)
{
	auto iter = m_shaderResourceTable.find(name);
	if (iter != m_shaderResourceTable.end())
	{
		std::memcpy(iter->second.mapData, data, iter->second.datasize);
		PrintDebug("Set Customized Resource Value success!");
	}
}

unsigned char* MaterialControl::GetCustomizedResourceValue(LPCSTR name, UINT16* size)
{
	auto iter = m_shaderResourceTable.find(name);
	if (iter != m_shaderResourceTable.end())
	{
		*size = iter->second.datasize;
		return iter->second.mapData;
	}
}

void MaterialControl::CreateDescriptor(D3DDevice* _cD3DDev, D3DPipeline* pipeline)
{
	auto d3ddevice = _cD3DDev->pD3D12Device;

	//------------cvb & srv view-------------

	auto whiteTex = D3DResourceManage::Instance().WhiteTexture;
	if (whiteTex == nullptr)
	{
		whiteTex = CreateOneColorTexture(d3ddevice, 0xffffffff);
		D3DResourceManage::Instance().WhiteTexture = whiteTex;
	}
	auto blackTex = D3DResourceManage::Instance().BlackTexture;
	if (blackTex == nullptr)
	{
		blackTex = CreateOneColorTexture(d3ddevice, 0x000000ff);
		D3DResourceManage::Instance().BlackTexture = blackTex;
	}
	auto gradTex = D3DResourceManage::Instance().GrayGradationTexture;
	if (gradTex == nullptr)
	{
		gradTex = CreateGrayGradationTexture(d3ddevice);
		D3DResourceManage::Instance().GrayGradationTexture = gradTex;
	}

	//------------srv desc--------------
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//------------cvb desc--------------
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCbvDesc = {};
	auto adress = m_materialBuff->GetGPUVirtualAddress();
	UINT sizeInBytes = m_materialBuff->GetDesc().Width;


	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;



	UINT matCount = 1;//m_materials.size();
	/*for (int i = 0; i < matCount; i++)
	{*/
	m_material.descOffset = pipeline->CreateConstantDescript(
		d3ddevice, adress, sizeInBytes);
	adress += materialBuffSize;

	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	if (m_material.m_textureResource != nullptr)
	{
		srvDesc.Format = m_material.m_textureResource->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, m_material.m_textureResource.Get());
	}
	else
	{
		srvDesc.Format = whiteTex->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, whiteTex);
	}

	if (m_material.m_sphResource != nullptr)
	{
		srvDesc.Format = m_material.m_sphResource->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, m_material.m_sphResource.Get());
	}
	else
	{
		srvDesc.Format = whiteTex->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, whiteTex);
	}

	if (m_material.m_spaResource != nullptr)
	{
		srvDesc.Format = m_material.m_spaResource->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, m_material.m_spaResource.Get());
	}
	else
	{
		srvDesc.Format = blackTex->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, blackTex);
	}

	if (m_material.m_toonResource != nullptr)
	{
		srvDesc.Format = m_material.m_toonResource->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, m_material.m_toonResource.Get());
	}
	else
	{
		srvDesc.Format = gradTex->GetDesc().Format;
		pipeline->CreateShaderResDescript(d3ddevice, gradTex);
	}
	//}
}

MaterialControl::~MaterialControl()
{
	for (auto& iter : m_shaderResourceTable)
	{
		auto& res = iter.second;
		res.resource->Release();
	}
	m_shaderResourceTable.clear();
}

//int MaterialControl::InitMaterial(int indicesNum)
//{
//	auto _cD3DDev = D3DResourceManage::Instance().pGraphicsCard;
//	auto d3ddevice = _cD3DDev->pD3D12Device;
//
//	unsigned int materialNum = 1;
//
//	m_textureResources.resize(materialNum);
//	m_sphResources.resize(materialNum);
//	m_spaResources.resize(materialNum);
//	m_toonResources.resize(materialNum);
//
//	m_materials.resize(materialNum);
//	for (int i = 0; i < materialNum; i++)
//	{
//		m_materials[i].indicesNum = indicesNum;
//		m_materials[i].material.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
//		m_materials[i].material.alpha = 1.0f;
//		m_materials[i].material.specular = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_materials[i].material.specularity = 0.5f;
//		m_materials[i].material.ambient = XMFLOAT3(0.2f, 0.2f, 0.2f);
//
//		m_textureResources[i] = nullptr;
//		m_sphResources[i] = nullptr;
//		m_spaResources[i] = nullptr;
//		m_toonResources[i] = nullptr;
//	}
//
//	//-----------material buff---------------------
//	auto materialBuffSize = sizeof(MaterialForHlsl);
//	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
//
//	auto materialResDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materialNum);
//	auto heapTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
//	HRESULT result = d3ddevice->CreateCommittedResource(
//		&heapTypeUpload,
//		D3D12_HEAP_FLAG_NONE,
//		&materialResDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(m_materialBuff.ReleaseAndGetAddressOf()));
//	if (FAILED(result))
//	{
//		ShowMsgBox(L"error", L"Create material Resource fault.");
//		return -1;
//	}
//
//	char* mapMaterial = nullptr;
//	result = m_materialBuff->Map(0, nullptr, (void**)&mapMaterial);
//	for (auto& m : m_materials)
//	{
//		*((MaterialForHlsl*)mapMaterial) = m.material;
//		mapMaterial += materialBuffSize;
//	}
//	m_materialBuff->Unmap(0, nullptr);
//
//
//}