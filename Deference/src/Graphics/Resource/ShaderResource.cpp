#include "ShaderResource.h"
#include "Graphics.h"
#include "Target.h"

ShaderResource::ShaderResource(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_SRV_DIMENSION dim, ComPtr<ID3D12Resource> res)
    :Resource(handle, D3D12_RESOURCE_STATE_GENERIC_READ, res)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = m_Res->GetDesc().Format;
    desc.ViewDimension = dim;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    auto pRes = m_Res.Get();
    switch (dim)
    {
    case D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE:
        desc.RaytracingAccelerationStructure.Location = m_Res->GetGPUVirtualAddress();
        pRes = nullptr;
        break;
    case D3D12_SRV_DIMENSION_TEXTURE2D:
        desc.Texture2D.MipLevels = 1;
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.PlaneSlice = 0;
    }
    g.Device().CreateShaderResourceView(pRes, &desc, m_Handle);
}

TargetSR::TargetSR(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, Shared<Target> target)
    :ShaderResource(g, handle, D3D12_SRV_DIMENSION_TEXTURE2D, target->Res())
{
}

TopLevelAS::TopLevelAS(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res)
    :ShaderResource(g, handle, D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE, res)
{
}