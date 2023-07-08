#include "RootSig.h"
#include "RootSig.h"
#include <d3dx12.h>

RootSig::RootSig(Graphics& g, const RootParams& params, bool local)
{
    D3D12_ROOT_SIGNATURE_FLAGS flags = params.Flags();
    if (local)
        flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
    desc.Init_1_1(params.NumParams(), params.Params(), params.NumSamplers(), params.Samplers(), flags);

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;
    if (FAILED(D3DX12SerializeVersionedRootSignature(&desc,
        Graphics::ROOT_SIG_VERSION, &blob, &errorBlob)))
        throw std::runtime_error(static_cast<CHAR*>(errorBlob->GetBufferPointer()));

    HR g.Device().CreateRootSignature(0, blob->GetBufferPointer(),
        blob->GetBufferSize(), IID_PPV_ARGS(&m_Sig));
}

RootSig::RootSig(Graphics& g, SIZE_T numParams, const CD3DX12_ROOT_PARAMETER1* params)
{
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc;
    desc.Init_1_1(numParams, params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> errorBlob;
    if (FAILED(D3DX12SerializeVersionedRootSignature(&desc,
        Graphics::ROOT_SIG_VERSION, &blob, &errorBlob)))
        throw std::runtime_error(static_cast<CHAR*>(errorBlob->GetBufferPointer()));

    HR g.Device().CreateRootSignature(0, blob->GetBufferPointer(),
        blob->GetBufferSize(), IID_PPV_ARGS(&m_Sig));
}

void RootSig::Bind(Graphics& g)
{
   g.CL().SetGraphicsRootSignature(m_Sig.Get());
}

void DescTable::AddRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT slot, UINT space, UINT numDescs)
{
    D3D12_DESCRIPTOR_RANGE1 range = {};
    range.RangeType = type;
    range.NumDescriptors = numDescs; 
    range.BaseShaderRegister = slot;
    range.RegisterSpace = space;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    m_Ranges.push_back(std::move(range)); 
}

void RootParams::AddTable(const DescTable& table, D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_ROOT_DESCRIPTOR_TABLE1 desc = {};
    desc.NumDescriptorRanges = table.NumRanges();
    desc.pDescriptorRanges = table.Ranges();

    D3D12_ROOT_PARAMETER1 param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable = std::move(desc); 
    param.ShaderVisibility = visibility; 

    CheckFlags(visibility);

    m_Params.push_back(std::move(param));
}

void RootParams::CheckFlags(D3D12_SHADER_VISIBILITY visibility)
{
    switch (visibility) {
    case D3D12_SHADER_VISIBILITY_PIXEL:
        m_Flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
        break;
    case D3D12_SHADER_VISIBILITY_VERTEX:
        m_Flags &= ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
        break;
    }
}

void RootParams::AddInline(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility, UINT slot)
{
    D3D12_ROOT_DESCRIPTOR1 desc = {};
    desc.RegisterSpace = 0;
    desc.ShaderRegister = slot;

    D3D12_ROOT_PARAMETER1 param = {};
    param.ParameterType = type; 
    param.Descriptor = std::move(desc);
    param.ShaderVisibility = visibility;

    CheckFlags(visibility);

    m_Params.push_back(std::move(param));
}

void RootParams::AddSampler(SAMPLER_FILTER_MODE filter, SAMPLER_ADDRESS_MODE address, UINT slot)
{
    D3D12_STATIC_SAMPLER_DESC desc = {};
    switch (filter)
    {
    case SAMPLER_FILTER_MODE::NEAREST:
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        break;
    case SAMPLER_FILTER_MODE::BILINEAR:
        desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    }

    switch (address)
    {
    case SAMPLER_ADDRESS_MODE::BORDER:
        desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        break;
    }
    
    desc.MipLODBias = 0;
    desc.MaxAnisotropy = 0;
    desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    desc.MinLOD = 0.0f;
    desc.MaxLOD = D3D12_FLOAT32_MAX;
    desc.ShaderRegister = slot;
    desc.RegisterSpace = 0;
    desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    m_Samplers.push_back(std::move(desc));
}