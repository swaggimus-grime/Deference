//#include "ToneMapPipeline.h"
//#include "Shader/Shader.h"
//#include "Bindable/Pipeline/InputLayout.h"
//#include "Shader/RootSig.h"
//
//ToneMapPipeline::ToneMapPipeline(Graphics& g)
//{
//    InputLayout layout(INPUT_LAYOUT_CONFIG::SCREEN);
//
//    VertexShader vs(L"shaders\\screen.vs.hlsl");
//    PixelShader ps(L"shaders\\tonemap.ps.hlsl");
//
//    CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
//    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
//    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
//    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);
//
//    CD3DX12_ROOT_PARAMETER1 rootParameters[2];
//    rootParameters[0].InitAsDescriptorTable(2, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
//    rootParameters[1].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
//
//    auto sig = MakeShared<RootSig>(g, _countof(rootParameters), rootParameters);
//    Create(g, std::move(sig), vs, ps, std::move(layout), { Swapchain::s_Format });
//}
