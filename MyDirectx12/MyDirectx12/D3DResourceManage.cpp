#include"D3DResourceManage.h"
using namespace DirectX;

D3DResourceManage& D3DResourceManage::Instance()
{
    static D3DResourceManage instance;
    return instance;
}

void D3DResourceManage::Init()
{
    PipelineModelTable = new std::map<const char*, std::vector<D3DModel*>*>();
    UidModelTable = new std::map<unsigned long long, D3DModel*>();

    //creat dault pmd pipeline
    auto pModelTable = D3DResourceManage::Instance().PipelineModelTable;
    auto iter = pModelTable->find("PmdStandard");
    std::vector<D3DModel*>* pVector = nullptr;
    if (iter == pModelTable->end())
    {
        pVector = new std::vector<D3DModel*>();
        pModelTable->insert(std::pair<const char*, std::vector<D3DModel*>*>("PmdStandard", pVector));
    }
    else
    {
        pVector = iter->second;
    }
}

void D3DResourceManage::InitializeLoadTable()
{
    LoadLambdaTable["sph"]
        = LoadLambdaTable["spa"]
        = LoadLambdaTable["bmp"]
        = LoadLambdaTable["png"]
        = LoadLambdaTable["jpg"]
        = [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)
        ->HRESULT
    {
        return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, img);
    };

    LoadLambdaTable["tga"]=[](const std::wstring& path, TexMetadata* meta, ScratchImage& img)
        ->HRESULT
    {
        return LoadFromTGAFile(path.c_str(), meta, img);
    };

    LoadLambdaTable["dds"] = [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)
        ->HRESULT
    {
        return LoadFromDDSFile(path.c_str(),DDS_FLAGS_NONE, meta, img);
    };
}