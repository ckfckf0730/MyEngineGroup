#pragma once

#include"D3DAPI.h"
#include"map"
#include"vector"

class D3DCamera;
class BasicModel;
class PMDModel;
class D3DDevice;
class D3DPipeline;


class D3DResourceManage
{
public:
    static D3DResourceManage& Instance();

private:
    D3DResourceManage() = default;
    ~D3DResourceManage() = default;
    D3DResourceManage(const D3DResourceManage&) = delete;
    D3DResourceManage& operator=(const D3DResourceManage&) = delete;

private:


public:
    D3DDevice* pGraphicsCard;

    std::map<unsigned long long, D3DCamera*> CameraTable;
    std::map<const char*, D3DPipeline*> PipelineTable;
    std::map<const char*,std::vector<BasicModel*>*>* PipelineModelTable;     //key is pipeline name
    std::map<std::string, ID3D12Resource*> ResourceTable;

    ID3D12Resource* WhiteTexture;
    ID3D12Resource* BlackTexture;
    ID3D12Resource* GrayGradationTexture;

    using LoadLambda_t = std::function<HRESULT(
        const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
    std::map<std::string, LoadLambda_t> LoadLambdaTable;

    void Init();
    void InitializeLoadTable();

    




};

