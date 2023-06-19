#include "ShaderResource.h"
#include "Graphics.h"

ShaderResource::ShaderResource(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res, D3D12_SRV_DIMENSION dimension)
	:ShaderAccessible(handle, D3D12_RESOURCE_STATE_GENERIC_READ, res)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.ViewDimension = dimension;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    if(dimension == D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE)
        desc.RaytracingAccelerationStructure.Location = m_Res->GetGPUVirtualAddress();
    g.Device().CreateShaderResourceView(nullptr, &desc, m_Handle);
}

TopLevelAS::TopLevelAS(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res)
    :ShaderResource(g, handle, res, D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE)
{
}