#include "Pipeline.h"
#include "Shader/RootSig.h"
#include "Shader/Shader.h"
#include "Bindable/InputLayout.h"
#include "Graphics.h"
#include "Debug/Exception.h"
#include "Pipeline.h"

Pipeline::Pipeline(Graphics& g, const RootSig& rootSig, const VertexShader& vs, const PixelShader& ps, const InputLayout& inputLayout)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.InputLayout = inputLayout.Layout(); // the structure describing our input layout
    psoDesc.pRootSignature = rootSig.Sig(); // the root signature that describes the input data this pso needs
    psoDesc.VS = vs.ByteCode(); // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = ps.ByteCode(); // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC{ D3D12_DEFAULT };
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc = { 1, 0 };
    psoDesc.SampleMask = 0xffffffff; 
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); 
    psoDesc.NumRenderTargets = 1; 
    /*psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = D3D12_CULL_MODE_BACK;*/
    HR g.Device().CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_State));
}

void Pipeline::Bind(Graphics& g)
{
    g.CL().SetPipelineState(m_State.Get());
}
