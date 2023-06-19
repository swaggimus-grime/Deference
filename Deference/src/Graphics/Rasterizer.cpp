#include "Rasterizer.h"
#include "RootSig.h"
#include "Pipeline.h"

Rasterizer::Rasterizer(Graphics& g)
{
    m_DSs = MakeUnique<Heap<DepthStencil>>(g, 1);
    m_DSs->AddResource<DepthStencil>(g);

    g.Flush();
}

void Rasterizer::Render(Graphics& g, Shared<RenderTarget> bb)
{
    bb->Transition(g, D3D12_RESOURCE_STATE_RENDER_TARGET);
    bb->BindWithDepth(g, (*m_DSs)[0].get());
    bb->Clear(g);
    (*m_DSs)[0]->Clear(g);

    g.GetCamera().Bind(g);
    g.CL().IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (UINT i = 0; i < g.Scene().m_VBs.size(); i++)
    {
        g.Scene().m_VBs[i]->Bind(g);
        g.Scene().m_IBs[i]->Bind(g);
        g.CL().DrawIndexedInstanced(g.Scene().m_IBs[i]->NumIndices(), 1, 0, 0, 0);
    }

    bb->Transition(g, D3D12_RESOURCE_STATE_PRESENT);
}
