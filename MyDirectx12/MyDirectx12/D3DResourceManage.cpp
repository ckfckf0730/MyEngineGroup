#include"D3DResourceManage.h"
using namespace DirectX;

D3DResourceManage& D3DResourceManage::Instance()
{
    static D3DResourceManage instance;
    return instance;
}

void D3DResourceManage::Init()
{
    //PipelineModelTable = new std::map<std::string, std::vector<BasicModel*>*>();

    //creat default pmd pipelineTable
  /*  auto pModelTable = D3DResourceManage::Instance().PipelineModelTable;
    auto iter = pModelTable->find("PmdStandard");
    std::vector<BasicModel*>* pVector = nullptr;
    if (iter == pModelTable->end())
    {
        pVector = new std::vector<BasicModel*>();
        pModelTable->insert(std::pair<const char*, std::vector<BasicModel*>*>("PmdStandard", pVector));
    }
    
    iter = pModelTable->find("NoboneStandard");
    if (iter == pModelTable->end())
    {
        pVector = new std::vector<BasicModel*>();
        pModelTable->insert(std::pair<const char*, std::vector<BasicModel*>*>("NoboneStandard", pVector));
    }*/
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