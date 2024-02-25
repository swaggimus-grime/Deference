#include "Pipeline.h"
#include "RootSig.h"
#include "Shader.h"
#include "InputLayout.h"

namespace Def
{
    void Pipeline::Bind(Graphics& g)
    {
        g.CL().SetPipelineState(m_State.Get());
        m_Sig->Bind(g);
    }

    Pipeline::Pipeline(Graphics& g, Shared<RootSig> sig, const VertexShader& vs, const PixelShader& ps, const D3D12_INPUT_LAYOUT_DESC& layout,
        std::vector<DXGI_FORMAT> rtFormats)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = layout;
        psoDesc.pRootSignature = **sig;
        psoDesc.VS = vs.ByteCode();
        psoDesc.PS = ps.ByteCode();
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        for (UINT i = 0; i < rtFormats.size(); i++)
            psoDesc.RTVFormats[i] = rtFormats[i];
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC{ D3D12_DEFAULT };
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc = { 1, 0 };
        psoDesc.SampleMask = 0xffffffff;
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.NumRenderTargets = rtFormats.size();

        HR g.Device().CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_State));
        m_Sig = sig;
    }

}