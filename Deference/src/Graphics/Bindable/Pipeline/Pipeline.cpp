#include "Pipeline.h"
#include "Shader/RootSig.h"
#include "Shader/Shader.h"
#include "Bindable/Pipeline/InputLayout.h"
#include "Graphics.h"
#include "Debug/Exception.h"
#include "Pipeline.h"

void Pipeline::Bind(Graphics& g)
{
    g.CL().SetPipelineState(m_State.Get());
    m_Sig->Bind(g);
}

void Pipeline::Create(Graphics& g, Shared<RootSig> sig, const VertexShader& vs, const PixelShader& ps, const InputLayout& layout, UINT numRTs)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; 
    psoDesc.InputLayout = layout.Layout();
    psoDesc.pRootSignature = **sig;
    psoDesc.VS = vs.ByteCode(); 
    psoDesc.PS = ps.ByteCode(); 
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    for(UINT i = 0; i < numRTs; i++)
        psoDesc.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC{ D3D12_DEFAULT };
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc = { 1, 0 };
    psoDesc.SampleMask = 0xffffffff;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.NumRenderTargets = numRTs;

    HR g.Device().CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_State));
    m_Sig = sig;
}
