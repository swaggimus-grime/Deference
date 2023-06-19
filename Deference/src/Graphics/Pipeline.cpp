#include "Pipeline.h"
#include "RootSig.h"
#include "Shader.h"

Pipeline::Pipeline(Graphics& g, const RootSig& rootSig, const VertexShader& vs, const PixelShader& ps, const D3D12_INPUT_LAYOUT_DESC& inputLayout)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.InputLayout = inputLayout; // the structure describing our input layout
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
    HR g.Device().CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_State));
}
