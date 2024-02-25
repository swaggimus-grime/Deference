//#include"AccumPipeline.h"
//
//#include "Shader/Shader.h"
//#include "Bindable/Pipeline/InputLayout.h"
//#include "Shader/RootSig.h"
//
//AccumPipeline::AccumPipeline(Graphics& g)
//{
//    InputLayout layout(INPUT_LAYOUT_CONFIG::SCREEN);
//
//    VertexShader vs(L"shaders\\screen.vs.hlsl");
//    PixelShader ps(L"shaders\\accum.ps.hlsl");
//
//    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
//    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);
//
//    CD3DX12_ROOT_PARAMETER1 rootParameters[2];
//    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
//    rootParameters[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
//
//    Create(g, MakeShared<RootSig>(g, _countof(rootParameters), rootParameters), vs, ps, layout, 
//        {Swapchain::s_Format, Swapchain::s_Format });
//}
