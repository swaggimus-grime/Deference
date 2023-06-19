#include "RenderTarget.h"

RenderTarget::RenderTarget(Graphics& g, D3D12_CPU_DESCRIPTOR_HANDLE handle, ComPtr<ID3D12Resource> res)
    :Target(handle, D3D12_RESOURCE_STATE_PRESENT, res)
{
    g.Device().CreateRenderTargetView(m_Res.Get(), nullptr, m_Handle);
}

void RenderTarget::Clear(Graphics& g)
{
   FLOAT col[] = { 0.f, 0.f, 0.f, 1.f };
   g.CL().ClearRenderTargetView(m_Handle, col, 0, nullptr);
}

void RenderTarget::Bind(Graphics& g)
{
    g.CL().OMSetRenderTargets(1, &m_Handle, false, nullptr);
}

void RenderTarget::BindWithDepth(Graphics& g, DepthStencil* ds)
{
    g.CL().OMSetRenderTargets(1, &m_Handle, false, &ds->Handle());
}
