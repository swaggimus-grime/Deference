#include "GeometryPipeline.h"
#include "Shader/Shader.h"
#include "Bindable/Pipeline/InputLayout.h"
#include "Shader/RootSig.h"

GeometryPipeline::GeometryPipeline(Graphics& g)
{
    InputLayout layout;

    VertexShader vs(L"shaders\\geometry.vs.hlsl");
    PixelShader ps(L"shaders\\geometry.ps.hlsl");

    CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 rootParameters[4];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[3].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    Create(g, MakeShared<RootSig>(g, _countof(rootParameters), rootParameters), vs, ps, layout, layout.NumElements());
}
