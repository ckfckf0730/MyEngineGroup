﻿#include"PMDActor.h"
#include"D3DResourceManage.h"
#include <sstream>
using namespace DirectX;

std::map<std::string, BasicModel*> BasicModel::s_modelTable;
std::map <unsigned long long, ModelInstance*> ModelInstance::s_uidModelTable;     //key is uid

std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
{
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}

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

int PMDModel::SetPMD(D3DDevice* _cD3DDev, const char* _FileFullName)
{
	auto d3ddevice = _cD3DDev->pD3D12Device;
	struct PMDHeader
	{
		float version;
		char model_name[20];
		char comment[256];
	};

	char signature[3] = {};
	PMDHeader pmdheader = {};

	//read pmd file data
	FILE* fp;
	errno_t err = fopen_s(&fp, _FileFullName, "rb");
	if (err != 0)
	{
		ShowMsgBox(L"error", L"Load pmd file fault.");
		return -1;
	}

	fread(signature, sizeof(signature), 1, fp);  //At the beginning of file, there is PMD char 
	fread(&pmdheader, sizeof(pmdheader), 1, fp); //next PMDhear data

	constexpr unsigned int pmdvertex_size_inFIle = 38;
	constexpr unsigned int pmdvertex_size_inGPU = 40;

	fread(&m_vertNum, sizeof(m_vertNum), 1, fp); //next vertex number.
	std::vector<unsigned char> readFileVertices(m_vertNum * pmdvertex_size_inFIle);
	fread(readFileVertices.data(), readFileVertices.size(), 1, fp); //next PMDvertex data

	std::vector<unsigned short> indices;
	fread(&m_indicesNum, sizeof(m_indicesNum), 1, fp);	//next indices number
	indices.resize(m_indicesNum);
	size_t indicesAllData_size = m_indicesNum * sizeof(indices[0]);
	fread(indices.data(), indicesAllData_size, 1, fp); //next indices data

	unsigned int materialNum;
	fread(&materialNum, sizeof(materialNum), 1, fp);

	std::vector<PMDMaterial> pmdMaterials(materialNum);
	fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);

	//------------------bone data & aniamtion-----------------------
	unsigned short boneNum = 0;
	fread(&boneNum, sizeof(boneNum), 1,fp);
	m_pmdBones.resize(boneNum);
	fread(m_pmdBones.data(), sizeof(PMDBone), boneNum, fp);

	uint16_t ikNum = 0;
	fread(&ikNum, sizeof(ikNum), 1, fp);
	
	m_ikData.resize(ikNum);
	for (auto& ik : m_ikData)
	{
		fread(&ik.boneIdx, sizeof(ik.boneIdx), 1, fp);
		fread(&ik.targetIdx, sizeof(ik.targetIdx), 1, fp);

		uint8_t chainLen = 0;
		fread(&chainLen, sizeof(chainLen), 1, fp);
		ik.nodeIdxes.resize(chainLen);
		fread(&ik.iterations, sizeof(ik.iterations), 1, fp);
		fread(&ik.limit, sizeof(ik.limit), 1, fp);

		if (chainLen != 0)
		{
			fread(ik.nodeIdxes.data(),
				sizeof(ik.nodeIdxes[0]),
				chainLen, fp);
		}
	}


	fclose(fp);

	//---rearrangement read data----
	std::vector<unsigned char> vertices(m_vertNum * pmdvertex_size_inGPU);
	for (int i = 0; i < m_vertNum; i++)
	{
		std::copy(&readFileVertices[i * pmdvertex_size_inFIle],
			&readFileVertices[i * pmdvertex_size_inFIle] + pmdvertex_size_inFIle,
			&vertices[i * pmdvertex_size_inGPU]);
	}

	m_textureResources.resize(materialNum);
	m_sphResources.resize(materialNum);
	m_spaResources.resize(materialNum);
	m_toonResources.resize(materialNum);

	m_materials.resize(materialNum);
	for (int i = 0; i < materialNum; i++)
	{
		std::string toonFilePath = "toon/";
		char toonFileName[16];
		sprintf_s(toonFileName, "toon%02d.bmp",
			static_cast<unsigned char>(pmdMaterials[i].toonIdx + 1));
		toonFilePath += toonFileName;
		m_toonResources[i] = LoadTextureFromFile(toonFilePath, d3ddevice);

		m_materials[i].indicesNum = pmdMaterials[i].indicesNum;
		m_materials[i].material.diffuse = pmdMaterials[i].diffuse;
		m_materials[i].material.alpha = pmdMaterials[i].alpha;
		m_materials[i].material.specular = pmdMaterials[i].specular;
		m_materials[i].material.specularity = pmdMaterials[i].specularity;
		m_materials[i].material.ambient = pmdMaterials[i].ambient;

		if (strlen(pmdMaterials[i].texFilePath) == 0)
		{
			m_textureResources[i] = nullptr;
			continue;
		}

		std::string texFileName = pmdMaterials[i].texFilePath;
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
			auto texFilePath = GetTexturePathFromModelAndTexPath(
				_FileFullName, texFileName.c_str());
			m_textureResources[i] = LoadTextureFromFile(texFilePath, d3ddevice);
		}
		if (sphFileName != "")
		{
			auto sphFilePath = GetTexturePathFromModelAndTexPath(
				_FileFullName, sphFileName.c_str());
			m_sphResources[i] = LoadTextureFromFile(sphFilePath, d3ddevice);
		}
		if (spaFileName != "")
		{
			auto spaFilePath = GetTexturePathFromModelAndTexPath(
				_FileFullName, spaFileName.c_str());
			m_spaResources[i] = LoadTextureFromFile(spaFilePath, d3ddevice);
		}
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
	std::copy(indices.begin(), indices.end(), mappedIdx);
	m_idxBuff->Unmap(0, nullptr);

	m_ibView.BufferLocation = m_idxBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = indicesAllData_size;

	//-----------material buff---------------------
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto materialResDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materialNum);
	result = d3ddevice->CreateCommittedResource(
		&heapTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&materialResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_materialBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"Create material Resource fault.");
		return -1;
	}

	char* mapMaterial = nullptr;
	result = m_materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	for (auto& m : m_materials)
	{
		*((MaterialForHlsl*)mapMaterial) = m.material;
		mapMaterial += materialBuffSize;
	}
	m_materialBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC matHeapDesc = {};
	matHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matHeapDesc.NodeMask = 0;
	matHeapDesc.NumDescriptors = materialNum * 5;
	matHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = d3ddevice->CreateDescriptorHeap(
		&matHeapDesc, IID_PPV_ARGS(&m_materialDescHeap));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"Create material DescHeap fault.");
		return -1;
	}

	//------------srv desc--------------
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//------------cvb desc--------------
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCbvDesc = {};
	matCbvDesc.BufferLocation = m_materialBuff->GetGPUVirtualAddress();
	matCbvDesc.SizeInBytes = materialBuffSize;

	//------------cvb & srv view-------------
	auto matDescHeapH = m_materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = d3ddevice->
		GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	
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
	

	for (int i = 0; i < materialNum; i++)
	{
		d3ddevice->CreateConstantBufferView(&matCbvDesc, matDescHeapH);
		matDescHeapH.ptr += inc;
		matCbvDesc.BufferLocation += materialBuffSize;

		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		if (m_textureResources[i] != nullptr)
		{
			srvDesc.Format = m_textureResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_textureResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				whiteTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_sphResources[i] != nullptr)
		{
			srvDesc.Format = m_sphResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_sphResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				whiteTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_spaResources[i] != nullptr)
		{
			srvDesc.Format = m_spaResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_spaResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = blackTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				blackTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_toonResources[i] != nullptr)
		{
			srvDesc.Format = m_toonResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(m_toonResources[i], &srvDesc, matDescHeapH);
		}
		else
		{
			srvDesc.Format = gradTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(gradTex, &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += inc;
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

		PrintDebug(oss.str().c_str());
	}
	

	return 1;
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

int PMDModel::SetMaterials(D3DDevice* _cD3DDev, unsigned int matCount, DirectX::XMFLOAT3 diffuse[], float alpha[],
	float specularity[], DirectX::XMFLOAT3 specular[], DirectX::XMFLOAT3 ambient[], unsigned char edgeFlg[], 
	unsigned char toonIdx[], unsigned int indicesNum[], const char* texFilePath[], const char* _FileFullName)
{
	auto d3ddevice = _cD3DDev->pD3D12Device;

	m_textureResources.resize(matCount);
	m_sphResources.resize(matCount);
	m_spaResources.resize(matCount);
	m_toonResources.resize(matCount);

	m_materials.resize(matCount);
	for (int i = 0; i < matCount; i++)
	{
		std::string toonFilePath = "toon/";
		char toonFileName[16];
		sprintf_s(toonFileName, "toon%02d.bmp",
			static_cast<unsigned char>(toonIdx[i] + 1));
		toonFilePath += toonFileName;
		m_toonResources[i] = LoadTextureFromFile(toonFilePath, d3ddevice);

		m_materials[i].indicesNum = indicesNum[i];
		m_materials[i].material.diffuse = diffuse[i];
		m_materials[i].material.alpha = alpha[i];
		m_materials[i].material.specular = specular[i];
		m_materials[i].material.specularity =specularity[i];
		m_materials[i].material.ambient = ambient[i];

		if (strlen(texFilePath[i]) == 0)
		{
			m_textureResources[i] = nullptr;
			continue;
		}

		std::string texFileName = texFilePath[i];
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
			auto texFilePath = GetTexturePathFromModelAndTexPath(
				_FileFullName, texFileName.c_str());
			m_textureResources[i] = LoadTextureFromFile(texFilePath, d3ddevice);
		}
		if (sphFileName != "")
		{
			auto sphFilePath = GetTexturePathFromModelAndTexPath(
				_FileFullName, sphFileName.c_str());
			m_sphResources[i] = LoadTextureFromFile(sphFilePath, d3ddevice);
		}
		if (spaFileName != "")
		{
			auto spaFilePath = GetTexturePathFromModelAndTexPath(
				_FileFullName, spaFileName.c_str());
			m_spaResources[i] = LoadTextureFromFile(spaFilePath, d3ddevice);
		}
	}

	//-----------material buff---------------------
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto materialResDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * matCount);
	auto heapTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto result = d3ddevice->CreateCommittedResource(
		&heapTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&materialResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_materialBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"Create material Resource fault.");
		return -1;
	}

	char* mapMaterial = nullptr;
	result = m_materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	for (auto& m : m_materials)
	{
		*((MaterialForHlsl*)mapMaterial) = m.material;
		mapMaterial += materialBuffSize;
	}
	m_materialBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC matHeapDesc = {};
	matHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matHeapDesc.NodeMask = 0;
	matHeapDesc.NumDescriptors = matCount * 5;
	matHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = d3ddevice->CreateDescriptorHeap(
		&matHeapDesc, IID_PPV_ARGS(&m_materialDescHeap));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"Create material DescHeap fault.");
		return -1;
	}

	//------------srv desc--------------
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//------------cvb desc--------------
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCbvDesc = {};
	matCbvDesc.BufferLocation = m_materialBuff->GetGPUVirtualAddress();
	matCbvDesc.SizeInBytes = materialBuffSize;

	//------------cvb & srv view-------------
	auto matDescHeapH = m_materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = d3ddevice->
		GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


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


	for (int i = 0; i < matCount; i++)
	{
		d3ddevice->CreateConstantBufferView(&matCbvDesc, matDescHeapH);
		matDescHeapH.ptr += inc;
		matCbvDesc.BufferLocation += materialBuffSize;

		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		if (m_textureResources[i] != nullptr)
		{
			srvDesc.Format = m_textureResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_textureResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				whiteTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_sphResources[i] != nullptr)
		{
			srvDesc.Format = m_sphResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_sphResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				whiteTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_spaResources[i] != nullptr)
		{
			srvDesc.Format = m_spaResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_spaResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = blackTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				blackTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_toonResources[i] != nullptr)
		{
			srvDesc.Format = m_toonResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(m_toonResources[i], &srvDesc, matDescHeapH);
		}
		else
		{
			srvDesc.Format = gradTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(gradTex, &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += inc;
	}

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
		nameLen = min(nameLen, 20);
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

//int PMDModel::SetIKs()
//{
//
//}


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

int PMDModelInstance::CreateTransformView(D3DDevice* _cD3DDev)
{
	m_boneMatrices.resize(Model()->m_pmdBones.size());
	std::fill(m_boneMatrices.begin(), m_boneMatrices.end(), XMMatrixIdentity());
	auto buffSize = sizeof(Transform) * (1 + m_boneMatrices.size());
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

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = _cD3DDev->pD3D12Device->CreateDescriptorHeap(
		&heapDesc, IID_PPV_ARGS(&m_transformDescHeap));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Create transform const heap fault.");
		return -1;
	}

	auto heapHandle = m_transformDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_transformConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_transformConstBuff->GetDesc().Width;

	_cD3DDev->pD3D12Device->CreateConstantBufferView(&cbvDesc, heapHandle);


	m_transform.world = XMMatrixIdentity();
	m_mapMatrices[0] = m_transform.world;

	std::copy(m_boneMatrices.begin(),m_boneMatrices.end(), m_mapMatrices + 1);

	return 1;
}

int BasicModel::SetBasicModel(D3DDevice* _cD3DDev, const char* _FileFullName)
{
	auto d3ddevice = D3DResourceManage::Instance().pGraphicsCard->pD3D12Device;

	//read pmd file data
	FILE* fp;
	errno_t err = fopen_s(&fp, _FileFullName, "rb");
	if (err != 0)
	{
		PrintDebug(L"Load basic model file fault:");
		PrintDebug(_FileFullName);
		return -1;
	}

	fread(&m_vertNum, sizeof(m_vertNum), 1, fp);  //first is vertex count

	constexpr unsigned int basicVertex_size = 32;

	std::vector<unsigned char> vertices(m_vertNum * basicVertex_size);
	fread(vertices.data(), vertices.size(), 1, fp); //next vertex data

	std::vector<unsigned short> indices;
	fread(&m_indicesNum, sizeof(m_indicesNum), 1, fp);	//next indices number
	indices.resize(m_indicesNum);
	size_t indicesAllData_size = m_indicesNum * sizeof(indices[0]);
	fread(indices.data(), indicesAllData_size, 1, fp); //next indices data

	fclose(fp);

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

	memcpy(s_vertMap, vertices.data(), vertices.size());

	m_vertBuff->Unmap(0, nullptr);

	//create vertex buffer view
	m_vbView.BufferLocation = m_vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = vertices.size();
	m_vbView.StrideInBytes = basicVertex_size;

	//----------------------index part----------------------------

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
	std::copy(indices.begin(), indices.end(), mappedIdx);
	m_idxBuff->Unmap(0, nullptr);

	m_ibView.BufferLocation = m_idxBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = indicesAllData_size;
}
	
int BasicModel::InitMaterial()
{
	auto _cD3DDev = D3DResourceManage::Instance().pGraphicsCard;
	auto d3ddevice = _cD3DDev->pD3D12Device;

	unsigned int materialNum = 1;

	m_textureResources.resize(materialNum);
	m_sphResources.resize(materialNum);
	m_spaResources.resize(materialNum);
	m_toonResources.resize(materialNum);

	m_materials.resize(materialNum);
	for (int i = 0; i < materialNum; i++)
	{
		m_materials[i].indicesNum = m_indicesNum;
		m_materials[i].material.diffuse = XMFLOAT3(1.0f,1.0f,1.0f);
		m_materials[i].material.alpha = 1.0f;
		m_materials[i].material.specular = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_materials[i].material.specularity = 0.5f;
		m_materials[i].material.ambient = XMFLOAT3(0.2f, 0.2f, 0.2f);

		m_textureResources[i] = nullptr;
		m_sphResources[i] = nullptr;
		m_spaResources[i] = nullptr;
		m_toonResources[i] = nullptr;
	}

	//-----------material buff---------------------
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto materialResDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materialNum);
	auto heapTypeUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	HRESULT result = d3ddevice->CreateCommittedResource(
		&heapTypeUpload,
		D3D12_HEAP_FLAG_NONE,
		&materialResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_materialBuff.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"Create material Resource fault.");
		return -1;
	}

	char* mapMaterial = nullptr;
	result = m_materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	for (auto& m : m_materials)
	{
		*((MaterialForHlsl*)mapMaterial) = m.material;
		mapMaterial += materialBuffSize;
	}
	m_materialBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC matHeapDesc = {};
	matHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matHeapDesc.NodeMask = 0;
	matHeapDesc.NumDescriptors = materialNum * 5;
	matHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = d3ddevice->CreateDescriptorHeap(
		&matHeapDesc, IID_PPV_ARGS(&m_materialDescHeap));
	if (FAILED(result))
	{
		ShowMsgBox(L"error", L"Create material DescHeap fault.");
		return -1;
	}

	//------------srv desc--------------
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//------------cvb desc--------------
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCbvDesc = {};
	matCbvDesc.BufferLocation = m_materialBuff->GetGPUVirtualAddress();
	matCbvDesc.SizeInBytes = materialBuffSize;

	//------------cvb & srv view-------------
	auto matDescHeapH = m_materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto inc = d3ddevice->
		GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


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


	for (int i = 0; i < materialNum; i++)
	{
		d3ddevice->CreateConstantBufferView(&matCbvDesc, matDescHeapH);
		matDescHeapH.ptr += inc;
		matCbvDesc.BufferLocation += materialBuffSize;

		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		if (m_textureResources[i] != nullptr)
		{
			srvDesc.Format = m_textureResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_textureResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				whiteTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_sphResources[i] != nullptr)
		{
			srvDesc.Format = m_sphResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_sphResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = whiteTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				whiteTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_spaResources[i] != nullptr)
		{
			srvDesc.Format = m_spaResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				m_spaResources[i],
				&srvDesc,
				matDescHeapH);
		}
		else
		{
			srvDesc.Format = blackTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(
				blackTex,
				&srvDesc,
				matDescHeapH);
		}
		matDescHeapH.ptr += inc;

		if (m_toonResources[i] != nullptr)
		{
			srvDesc.Format = m_toonResources[i]->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(m_toonResources[i], &srvDesc, matDescHeapH);
		}
		else
		{
			srvDesc.Format = gradTex->GetDesc().Format;
			d3ddevice->CreateShaderResourceView(gradTex, &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += inc;
	}
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

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	result = _cD3DDev->pD3D12Device->CreateDescriptorHeap(
		&heapDesc, IID_PPV_ARGS(&m_transformDescHeap));
	if (FAILED(result))
	{
		ShowMsgBox(L"Error", L"Create transform const heap fault.");
		return -1;
	}

	auto heapHandle = m_transformDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_transformConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_transformConstBuff->GetDesc().Width;

	_cD3DDev->pD3D12Device->CreateConstantBufferView(&cbvDesc, heapHandle);


	m_transform.world = XMMatrixIdentity();
	*m_mapMatrices = m_transform.world;

	return 1;
}

ModelInstance::~ModelInstance()
{
	m_transformDescHeap->Release();
}