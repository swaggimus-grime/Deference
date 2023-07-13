#include "HybridOutPipeline.h"
#include "Shader/Shader.h"
#include "Bindable/Pipeline/InputLayout.h"
#include "Shader/RootSig.h"

HybridOutPipeline::HybridOutPipeline(Graphics& g)
{
    InputLayout layout(INPUT_LAYOUT_CONFIG::SCREEN);

    VertexShader vs(L"shaders\\screen.vs.hlsl");
    PixelShader ps(L"shaders\\hybrid.ps.hlsl");

    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0);

    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

    auto sig = MakeShared<RootSig>(g, _countof(rootParameters), rootParameters);
    Create(g, std::move(sig), vs, ps, std::move(layout), 1);
}
